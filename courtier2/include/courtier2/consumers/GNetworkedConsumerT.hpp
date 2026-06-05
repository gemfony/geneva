/********************************************************************************
 *
 * This file is part of the Geneva library collection. The following license
 * applies to this file:
 *
 * ------------------------------------------------------------------------------
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * ------------------------------------------------------------------------------
 *
 * Note that other files in the Geneva library collection may use a different
 * license. Please see the licensing information in each file.
 *
 ********************************************************************************
 *
 * See the NOTICE file in the top-level directory of the Geneva library
 * collection for a list of contributors and copyright information.
 *
 ********************************************************************************/

#pragma once

#include "common/GGlobalDefines.hpp"

// Standard headers
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <memory>
#include <mutex>
#include <vector>

// Geneva headers
#include "courtier/GCourtierEnums.hpp" // BUFFERPORT_ID_TYPE, dispatchState
#include "courtier2/GBaseConsumerT.hpp"

namespace Gem::Courtier2 {

/******************************************************************************/
/**
 * Base class for networked courtier2 consumers (ASIO, websocket, MPI). It owns the per-batch work
 * machinery that turns the synchronous reconcile-the-span contract of GBaseConsumerT into the
 * asynchronous, checkout/return world of remote clients:
 *
 *  - dispatch_(items) marks the round's items PENDING -- the scheduling state lives ON each item
 *    (its dispatchState), so there are no side queues; the batch itself is the queue. It then blocks
 *    until every slot reaches DONE or a bounded timeout elapses. Items that never return are left
 *    DO_PROCESS == MISSING, so the inherited reconciliation loop resubmits/clones/fails them.
 *  - checkout()/checkin() are the queue endpoints a transport's session calls: checkout() hands the
 *    next pending item to a client (or null -> the client backs off), checkin() matches a returned
 *    (deserialized) result to its slot by id and replaces the slot's pointer with the result.
 *
 * A result whose id is no longer outstanding (a late arrival from a previous, timed-out round, or a
 * duplicate) is silently dropped -- this is what makes resubmission safe. Concrete transports
 * implement only the server lifecycle (accept connections, run sessions wired to checkout/checkin).
 *
 * The timeout here is intentionally simple (a bounded first-item + per-item budget); liveness-aware
 * and adaptive timeouts are the subject of the deliberately-late Phase 5.
 */
template <typename processable_type>
class GNetworkedConsumerT : public GBaseConsumerT<processable_type> {
public:
    using item_ptr = typename GBaseConsumerT<processable_type>::item_ptr;
    using clock = std::chrono::steady_clock;

    GNetworkedConsumerT() = default;
    ~GNetworkedConsumerT() override = default;

    /***************************************************************************/
    /** @brief Sets the maximum time to wait for the FIRST item of a batch to return. A value of
     *  zero means "wait forever for the first item" (only the per-item budget then bounds the wait). */
    void setFirstItemMaxWait(std::chrono::milliseconds w) { first_item_max_wait_ = w; }
    /** @brief Sets the per-item time budget added to the batch deadline. */
    void setPerItemWait(std::chrono::milliseconds w) { per_item_wait_ = w; }

protected:
    /***************************************************************************/
    /** @brief Hands the next pending slot's item to a calling session, or null if none is pending.
     *  The item's dispatch state flips PENDING -> IN_FLIGHT; correlation rides its bufferport id. */
    item_ptr checkout() {
        std::lock_guard<std::mutex> lk(mtx_);
        return checkout_locked();
    }

    /***************************************************************************/
    /** @brief Like checkout(), but blocks up to @p wait for a slot to become available before
     *  giving up and returning null. Mirrors the brief blocking get() the broker offered, which
     *  keeps a transport from churning "no data" responses during the gaps between batches. */
    item_ptr checkoutWait(std::chrono::milliseconds wait) {
        std::unique_lock<std::mutex> lk(mtx_);
        if(pending_count_ == 0) {
            cv_work_.wait_for(lk, wait, [this] { return pending_count_ > 0 || stop_.load(); });
        }
        return checkout_locked();
    }

    /***************************************************************************/
    /** @brief Accepts a returned result and writes it into its slot. The result's (generation, slot)
     *  is decoded from its bufferport id; a result from a previous (timed-out) round or a duplicate
     *  -- i.e. one whose generation is stale or whose slot is no longer IN_FLIGHT -- is dropped,
     *  which is what makes resubmission safe. */
    void checkin(item_ptr p) {
        if(not p) {
            return;
        }
        std::lock_guard<std::mutex> lk(mtx_);
        if(current_batch_ == nullptr) {
            return; // no active round (e.g. a very late arrival after dispatch_ returned)
        }
        const Gem::Courtier::BUFFERPORT_ID_TYPE id = p->getBufferId();
        if(decodeGeneration(id) != (generation_ & GENERATION_MASK)) {
            return; // stale: from a previous round
        }
        const std::size_t slot = decodeSlot(id);
        auto &batch = *current_batch_;
        if(slot >= batch.size() ||
           batch[slot]->getDispatchState() != Gem::Courtier::dispatchState::IN_FLIGHT) {
            return; // out of range, or duplicate / not currently in flight
        }
        p->setDispatchState(Gem::Courtier::dispatchState::DONE);
        batch[slot] = p; // swap the processed copy into the slot in place
        ++done_;
        if(done_ == target_) {
            cv_done_.notify_one();
        }
    }

    /***************************************************************************/
    /** @brief Returns a slot's still-in-flight item to PENDING so another client picks it up
     *  immediately (the RAII put-back on a client disconnect). Caller passes the item it checked
     *  out; the slot is located via its bufferport id. A no-op if the round moved on or the slot is
     *  no longer in flight. */
    void requeue(const item_ptr &p) {
        if(not p) {
            return;
        }
        std::lock_guard<std::mutex> lk(mtx_);
        if(current_batch_ == nullptr) {
            return;
        }
        const Gem::Courtier::BUFFERPORT_ID_TYPE id = p->getBufferId();
        if(decodeGeneration(id) != (generation_ & GENERATION_MASK)) {
            return;
        }
        const std::size_t slot = decodeSlot(id);
        auto &batch = *current_batch_;
        if(slot >= batch.size() ||
           batch[slot]->getDispatchState() != Gem::Courtier::dispatchState::IN_FLIGHT) {
            return;
        }
        batch[slot]->setDispatchState(Gem::Courtier::dispatchState::PENDING);
        ++pending_count_;
        if(slot < cursor_) {
            cursor_ = slot; // let checkout re-find it
        }
        cv_work_.notify_one();
    }

    /***************************************************************************/
    /** @brief Whether the server is being torn down (sessions use this to stop accepting work). */
    [[nodiscard]] bool stopped() const noexcept { return stop_.load(); }

    /** @brief Requests teardown -- subclasses call this from their stop path. */
    void requestStop() noexcept { stop_.store(true); }

    /***************************************************************************/
    /**
     * Marks one round of items PENDING (state lives on the item itself, no side queues), waits for
     * them to return (or time out), and -- because checkin() writes results straight into their
     * slots -- simply returns. Items that never reached DONE are left DO_PROCESS == MISSING for the
     * reconciliation loop.
     */
    void dispatch_(std::vector<item_ptr> &items) override {
        const std::size_t n = items.size();
        if(n == 0) {
            return;
        }

        {
            std::lock_guard<std::mutex> lk(mtx_);
            ++generation_; // a fresh round: stale returns from prior rounds will now be rejected
            current_batch_ = &items;
            target_ = n;
            done_ = 0;
            cursor_ = 0;
            pending_count_ = n;
            for(std::size_t k = 0; k < n; ++k) {
                // (generation, slot) correlation token -- slot is the index into this round's batch.
                items[k]->setBufferId(encodeId(generation_, k));
                items[k]->setDispatchState(Gem::Courtier::dispatchState::PENDING);
            }
        }
        cv_work_.notify_all(); // wake any session blocked in checkoutWait()

        // Bounded wait: a first-item budget plus a per-item budget. The predicate returns as soon as
        // every slot is DONE, so the happy path does not wait for the deadline.
        const auto deadline = clock::now() + this->batchDeadline(n);
        {
            std::unique_lock<std::mutex> lk(mtx_);
            cv_done_.wait_until(lk, deadline, [this] {
                return done_ == target_ || stop_.load();
            });
            // Slots are already updated in place by checkin(); slots never reached DONE keep their
            // original DO_PROCESS item == MISSING for the policy loop. Detach from the batch (whose
            // storage is about to leave dispatch_'s scope) so any very late checkin() is a no-op.
            current_batch_ = nullptr;
            target_ = 0;
            done_ = 0;
            pending_count_ = 0;
        }
    }

    /***************************************************************************/
    /** @brief The overall deadline budget for a batch of @p n items. */
    std::chrono::milliseconds batchDeadline(std::size_t n) const {
        // first-item budget (0 == effectively unbounded) plus a per-item budget.
        const auto first = first_item_max_wait_.count() == 0
            ? std::chrono::milliseconds(24 * 60 * 60 * 1000) // a day -- "wait forever" in practice
            : first_item_max_wait_;
        return first + per_item_wait_ * static_cast<long long>(n);
    }

private:
    /***************************************************************************/
    // The bufferport id (a uint32) is reused as the wire correlation token: the high bits hold a
    // round generation (so a stale return from a timed-out round is rejected), the low bits the slot
    // index within the round. NB: this bounds a single batch to 2^24 items -- vastly beyond any real
    // Geneva population -- and the generation wraps every 2^8 rounds (a harmless, astronomically
    // unlikely aliasing). When the old courtier (the real buffer-port user) is removed in Phase 6,
    // the field should be renamed to correlation_id_.
    static constexpr Gem::Courtier::BUFFERPORT_ID_TYPE SLOT_BITS = 24;
    static constexpr Gem::Courtier::BUFFERPORT_ID_TYPE SLOT_MASK = (1u << SLOT_BITS) - 1u;
    static constexpr Gem::Courtier::BUFFERPORT_ID_TYPE GENERATION_MASK = 0xFFu;

    static Gem::Courtier::BUFFERPORT_ID_TYPE encodeId(std::size_t generation, std::size_t slot) {
        return ((static_cast<Gem::Courtier::BUFFERPORT_ID_TYPE>(generation) & GENERATION_MASK) << SLOT_BITS)
               | (static_cast<Gem::Courtier::BUFFERPORT_ID_TYPE>(slot) & SLOT_MASK);
    }
    static Gem::Courtier::BUFFERPORT_ID_TYPE decodeGeneration(Gem::Courtier::BUFFERPORT_ID_TYPE id) {
        return (id >> SLOT_BITS) & GENERATION_MASK;
    }
    static std::size_t decodeSlot(Gem::Courtier::BUFFERPORT_ID_TYPE id) {
        return static_cast<std::size_t>(id & SLOT_MASK);
    }

    /***************************************************************************/
    /** @brief Scans from the cursor for the next PENDING slot, flips it IN_FLIGHT and returns its
     *  item. Caller holds mtx_. */
    item_ptr checkout_locked() {
        if(current_batch_ == nullptr || pending_count_ == 0) {
            return nullptr;
        }
        auto &batch = *current_batch_;
        while(cursor_ < batch.size() &&
              batch[cursor_]->getDispatchState() != Gem::Courtier::dispatchState::PENDING) {
            ++cursor_;
        }
        if(cursor_ >= batch.size()) {
            return nullptr; // none pending from here (a put-back would have reset the cursor)
        }
        const std::size_t k = cursor_++;
        batch[k]->setDispatchState(Gem::Courtier::dispatchState::IN_FLIGHT);
        --pending_count_;
        return batch[k];
    }

    mutable std::mutex mtx_;
    std::condition_variable cv_done_; ///< Signalled when the last slot of a batch reaches DONE
    std::condition_variable cv_work_; ///< Signalled when a batch's slots become available

    std::vector<item_ptr> *current_batch_ = nullptr; ///< The round's batch (valid while dispatch_ blocks)
    std::size_t generation_ = 0;    ///< Bumped each round; encoded into the correlation token
    std::size_t target_ = 0;        ///< Number of slots in the current round
    std::size_t done_ = 0;          ///< Slots that have reached DONE this round
    std::size_t pending_count_ = 0; ///< Slots currently PENDING (awaiting a client)
    std::size_t cursor_ = 0;        ///< Next slot index to consider in checkout

    std::atomic<bool> stop_{false};

    std::chrono::milliseconds first_item_max_wait_{60'000}; ///< Max wait for the first result of a batch
    std::chrono::milliseconds per_item_wait_{1'000};        ///< Added per item to the batch deadline
};

/******************************************************************************/

} /* namespace Gem::Courtier2 */
