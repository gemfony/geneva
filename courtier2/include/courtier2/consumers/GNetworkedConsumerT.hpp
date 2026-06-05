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
#include <algorithm>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <functional>
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
 * Timeout / death-detection strategy: dispatch_ waits until every slot is DONE or progress stalls
 * past an ADAPTIVE give-up window (a multiple of the running mean return time, bootstrapped by a
 * first-return window). While waiting, stuck in-flight items are reclaimed (requeued for another
 * client) once they exceed an adaptive LEASE -- but only for transports without a client-liveness
 * signal (usesTimeLease(), default true: ASIO/MPI). A transport that knows when a client died
 * (the websocket consumer, via its persistent session + CheckoutLease) reclaims immediately on
 * disconnect and disables the time lease, so a live-but-slow client is never wrongly reclaimed.
 */
template <typename processable_type>
class GNetworkedConsumerT : public GBaseConsumerT<processable_type> {
public:
    using item_ptr = typename GBaseConsumerT<processable_type>::item_ptr;
    using clock = std::chrono::steady_clock;

    GNetworkedConsumerT() = default;
    ~GNetworkedConsumerT() override = default;

    /***************************************************************************/
    /** @brief Sets how long dispatch_ waits for the FIRST result of the very first batch before
     *  giving up (no client ever connected). Once results have been seen, the give-up window becomes
     *  adaptive (a multiple of the running mean return time). */
    void setFirstItemMaxWait(std::chrono::milliseconds w) { first_return_window_ = w; }
    /** @brief Sets the lease used to reclaim a stuck in-flight item before the running mean is known. */
    void setLeaseBootstrap(std::chrono::milliseconds w) { lease_bootstrap_ = w; }
    /** @brief Sets the multiple of the running mean return time used as the reclaim lease. */
    void setLeaseFactor(double f) { lease_factor_ = f; }
    /** @brief Sets the lower/upper clamp on the adaptive reclaim lease. */
    void setLeaseBounds(std::chrono::milliseconds lo, std::chrono::milliseconds hi) {
        min_lease_ = lo;
        max_lease_ = hi;
    }
    /** @brief Sets the poll interval at which dispatch_ re-evaluates the lease/stall while waiting. */
    void setSweepTick(std::chrono::milliseconds w) { sweep_tick_ = w; }

protected:
    /***************************************************************************/
    /**
     * A RAII checkout lease for transports with PERSISTENT, per-client sessions (e.g. the websocket
     * consumer, whose client computes inline on an open connection). A session holds one of these and
     * records the item it currently has in flight; if the session is destroyed while still holding an
     * unreturned item -- the client disconnected mid-evaluation -- the lease requeues that item for
     * another client immediately, a liveness-driven put-back that does not wait out the time lease.
     * Transports with one-shot, per-request connections (ASIO) must NOT use this (their sessions end
     * normally after every exchange); they rely on the time lease instead -- see usesTimeLease().
     */
    struct CheckoutLease {
        item_ptr current;
        std::function<void(const item_ptr &)> on_abandon;

        CheckoutLease() = default;
        CheckoutLease(const CheckoutLease &) = delete;
        CheckoutLease &operator=(const CheckoutLease &) = delete;
        ~CheckoutLease() {
            if(current && on_abandon) {
                on_abandon(current);
            }
        }
    };

    /***************************************************************************/
    /** @brief Hands the next pending slot's item to a calling session, or null if none is pending.
     *  The item's dispatch state flips PENDING -> IN_FLIGHT; correlation rides its correlation id. */
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
     *  is decoded from its correlation id; a result from a previous (timed-out) round or a duplicate
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
        const Gem::Courtier::BUFFERPORT_ID_TYPE id = p->getCorrelationId();
        if(decodeGeneration(id) != (generation_ & GENERATION_MASK)) {
            return; // stale: from a previous round
        }
        const std::size_t slot = decodeSlot(id);
        auto &batch = *current_batch_;
        if(slot >= batch.size() ||
           batch[slot]->getDispatchState() != Gem::Courtier::dispatchState::IN_FLIGHT) {
            return; // out of range, or duplicate / not currently in flight
        }
        // Feed the adaptive timeout: how long this item took from checkout to return.
        const auto now = clock::now();
        recordReturnTime_(now - checked_out_at_[slot]);
        last_progress_ = now;

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
     *  out; the slot is located via its correlation id. A no-op if the round moved on or the slot is
     *  no longer in flight. */
    void requeue(const item_ptr &p) {
        if(not p) {
            return;
        }
        std::lock_guard<std::mutex> lk(mtx_);
        if(current_batch_ == nullptr) {
            return;
        }
        const Gem::Courtier::BUFFERPORT_ID_TYPE id = p->getCorrelationId();
        if(decodeGeneration(id) != (generation_ & GENERATION_MASK)) {
            return;
        }
        if(requeueSlot_locked(decodeSlot(id))) {
            cv_work_.notify_one();
        }
    }

    /***************************************************************************/
    /** @brief Whether the server is being torn down (sessions use this to stop accepting work). */
    [[nodiscard]] bool stopped() const noexcept { return stop_.load(); }

    /** @brief Requests teardown -- subclasses call this from their stop path. */
    void requestStop() noexcept { stop_.store(true); }

    /***************************************************************************/
    /** @brief Whether this consumer reclaims stuck in-flight items via the TIME lease. True for
     *  transports with no per-client liveness signal (ASIO, MPI): an item that has been IN_FLIGHT
     *  longer than the adaptive lease is presumed lost and requeued. A transport that detects client
     *  death directly (the websocket consumer, via CheckoutLease on its persistent session) overrides
     *  this to false, so a live-but-slow client is never wrongly reclaimed. */
    virtual bool usesTimeLease() const { return true; }

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

        const auto start = clock::now();
        {
            std::lock_guard<std::mutex> lk(mtx_);
            ++generation_; // a fresh round: stale returns from prior rounds will now be rejected
            current_batch_ = &items;
            target_ = n;
            done_ = 0;
            cursor_ = 0;
            pending_count_ = n;
            checked_out_at_.assign(n, start); // real checkout times are stamped in checkout_locked()
            last_progress_ = start;
            for(std::size_t k = 0; k < n; ++k) {
                // (generation, slot) correlation token -- slot is the index into this round's batch.
                items[k]->setCorrelationId(encodeId(generation_, k));
                items[k]->setDispatchState(Gem::Courtier::dispatchState::PENDING);
            }
        }
        cv_work_.notify_all(); // wake any session blocked in checkoutWait()

        // Wait until every slot is DONE, or progress stalls past the (adaptive) give-up window. The
        // happy path wakes on the final checkin() and exits at once. While waiting, where the
        // transport has no liveness signal, we periodically reclaim items that have been in flight
        // longer than the adaptive lease, so a dead client's slot is re-served to a live one.
        {
            std::unique_lock<std::mutex> lk(mtx_);
            while(true) {
                if(done_ == target_ || stop_.load()) {
                    break;
                }
                const auto now = clock::now();
                if(now - last_progress_ > currentStallWindow()) {
                    break; // no progress for too long -> give up; unresolved slots become MISSING
                }
                if(this->usesTimeLease()) {
                    leaseSweep_locked(now);
                }
                cv_done_.wait_for(lk, sweep_tick_);
            }
            // Slots are already updated in place by checkin(); slots never reached DONE keep their
            // original DO_PROCESS item == MISSING for the policy loop. Detach from the batch (whose
            // storage is about to leave dispatch_'s scope) so any very late checkin() is a no-op.
            current_batch_ = nullptr;
            target_ = 0;
            done_ = 0;
            pending_count_ = 0;
            checked_out_at_.clear();
        }
    }

private:
    /***************************************************************************/
    // The work item's correlation id (a uint32) carries the wire correlation token: the high bits
    // hold a round generation (so a stale return from a timed-out round is rejected), the low bits
    // the slot index within the round. NB: this bounds a single batch to 2^24 items -- vastly beyond
    // any real Geneva population -- and the generation wraps every 2^8 rounds (a harmless,
    // astronomically unlikely aliasing).
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
        checked_out_at_[k] = clock::now(); // start of this slot's lease / return-time measurement
        --pending_count_;
        return batch[k];
    }

    /***************************************************************************/
    /** @brief Flips a slot IN_FLIGHT -> PENDING (a put-back) and rewinds the cursor so checkout
     *  re-finds it. Returns whether anything was flipped. Caller holds mtx_. */
    bool requeueSlot_locked(std::size_t slot) {
        if(current_batch_ == nullptr) {
            return false;
        }
        auto &batch = *current_batch_;
        if(slot >= batch.size() ||
           batch[slot]->getDispatchState() != Gem::Courtier::dispatchState::IN_FLIGHT) {
            return false;
        }
        batch[slot]->setDispatchState(Gem::Courtier::dispatchState::PENDING);
        ++pending_count_;
        if(slot < cursor_) {
            cursor_ = slot;
        }
        return true;
    }

    /***************************************************************************/
    /** @brief Reclaims every slot that has been IN_FLIGHT longer than the adaptive lease, returning
     *  it to PENDING for re-service. Caller holds mtx_. */
    void leaseSweep_locked(clock::time_point now) {
        if(current_batch_ == nullptr) {
            return;
        }
        const auto lease = currentLease();
        bool any = false;
        auto &batch = *current_batch_;
        for(std::size_t k = 0; k < batch.size(); ++k) {
            if(batch[k]->getDispatchState() == Gem::Courtier::dispatchState::IN_FLIGHT &&
               (now - checked_out_at_[k]) > lease) {
                any = requeueSlot_locked(k) || any;
            }
        }
        if(any) {
            cv_work_.notify_all();
        }
    }

    /***************************************************************************/
    /** @brief Folds one observed checkout->return duration into the running mean (an EMA). */
    void recordReturnTime_(clock::duration d) {
        const double ms = std::chrono::duration<double, std::milli>(d).count();
        if(n_return_samples_ == 0) {
            mean_return_ms_ = ms;
        }
        else {
            mean_return_ms_ = ema_alpha_ * ms + (1.0 - ema_alpha_) * mean_return_ms_;
        }
        ++n_return_samples_;
    }

    /***************************************************************************/
    /** @brief The current reclaim lease: a multiple of the running mean return time (clamped), or a
     *  bootstrap value until the first return has been observed. */
    std::chrono::milliseconds currentLease() const {
        if(n_return_samples_ == 0) {
            return lease_bootstrap_;
        }
        const auto v = std::chrono::milliseconds(static_cast<long long>(lease_factor_ * mean_return_ms_));
        return std::clamp(v, min_lease_, max_lease_);
    }

    /***************************************************************************/
    /** @brief The current give-up window: how long dispatch_ tolerates NO progress before declaring
     *  the rest of the batch MISSING. A generous multiple of the running mean once returns have been
     *  seen; the first-return window until then. */
    std::chrono::milliseconds currentStallWindow() const {
        if(n_return_samples_ == 0) {
            return first_return_window_;
        }
        const auto v = std::chrono::milliseconds(static_cast<long long>(stall_factor_ * mean_return_ms_));
        return std::clamp(v, min_stall_, max_stall_);
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

    std::vector<clock::time_point> checked_out_at_;     ///< Per-slot checkout time (lease + stats)
    clock::time_point last_progress_{};                 ///< Time of the most recent checkin (stall basis)
    double mean_return_ms_ = 0.0;                       ///< Running (EMA) mean checkout->return time
    std::size_t n_return_samples_ = 0;                  ///< Returns observed so far (across rounds)

    std::atomic<bool> stop_{false};

    // --- adaptive-timeout configuration (sane defaults; tunable via the setters) ---
    double ema_alpha_ = 0.25;   ///< EMA weight for new return-time samples
    double lease_factor_ = 4.0; ///< Reclaim lease = lease_factor_ * mean return time (clamped)
    double stall_factor_ = 8.0; ///< Give-up window = stall_factor_ * mean return time (clamped)
    std::chrono::milliseconds first_return_window_{60'000}; ///< Wait for the very first return
    std::chrono::milliseconds lease_bootstrap_{10'000};     ///< Reclaim lease before any return is seen
    std::chrono::milliseconds min_lease_{200};              ///< Lower clamp on the reclaim lease
    std::chrono::milliseconds max_lease_{300'000};          ///< Upper clamp on the reclaim lease
    std::chrono::milliseconds min_stall_{2'000};            ///< Lower clamp on the give-up window
    std::chrono::milliseconds max_stall_{600'000};          ///< Upper clamp on the give-up window
    std::chrono::milliseconds sweep_tick_{50};              ///< dispatch_ re-evaluation poll interval
};

/******************************************************************************/

} /* namespace Gem::Courtier2 */
