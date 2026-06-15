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
#include <map>
#include <set>
#include <memory>
#include <mutex>
#include <vector>

// Geneva headers
#include "courtier/GCourtierEnums.hpp" // CORRELATION_ID_TYPE, dispatchState
#include "courtier/GBaseConsumerT.hpp"

namespace Gem::Courtier {

/******************************************************************************/
/**
 * Base class for networked courtier consumers (ASIO, websocket, MPI). It owns the work machinery
 * that turns the synchronous reconcile-the-span contract of GBaseConsumerT into the asynchronous,
 * checkout/return world of remote clients -- and it does so for MANY concurrent submitters at once
 * (the fan-in design), so a population of optimization algorithms can each submit to the SAME shared
 * client pool simultaneously (e.g. a meta-optimization over a population of inner algorithms).
 *
 *  - dispatch_(items) registers the round's items as a new BATCH (the scheduling state lives ON each
 *    item -- its dispatchState -- so there are no per-item side queues; each call BORROWS the caller's
 *    round vector, it never owns/copies the items). It then blocks until every slot of ITS batch
 *    reaches DONE or that batch's adaptive give-up window elapses; items left DO_PROCESS == MISSING are
 *    resubmitted/cloned/failed by the inherited reconciliation loop.
 *  - checkout()/checkin() are the queue endpoints a transport's session calls. checkout() serves the
 *    next pending slot, round-robin INTERLEAVED across all currently-active batches (so concurrent
 *    submitters make progress together, not one whole batch after another). checkin() matches a
 *    returned (deserialized) result to its slot by id and writes it back.
 *
 * The wire correlation token is `(batch_id, slot)` -- the reincarnation of the old broker's
 * bufferport routing: batch_id selects WHICH submitter's batch a result belongs to (so two concurrent
 * batches' slot 0s never collide), slot the index within that batch. A result whose batch_id is no
 * longer active (a late arrival from a batch that already timed out / finished) is silently dropped --
 * this is what makes resubmission and lease-reclaim safe, and replaces the former per-round generation.
 *
 * Borrow contract (also what makes the later shared_ptr->unique_ptr migration a localized change): the
 * batch's vector is BORROWED for the duration of dispatch_; the consumer reads/serializes/schedules
 * through it but never takes ownership. The only write is checkin() swapping a slot's pointer for the
 * deserialized result -- the population stays the sole owner of its individuals.
 *
 * Timeout / death-detection (unchanged in spirit, now per batch): each dispatch_ waits until its batch
 * is DONE or progress stalls past an ADAPTIVE give-up window (a multiple of the running mean return
 * time, shared across batches). The give-up window only applies ONCE at least one result has ever been
 * received: before that, dispatch_ waits INDEFINITELY, because under a batch-scheduling system the first
 * worker may not enter the pool for minutes or hours. While waiting, where the transport has no
 * client-liveness signal
 * (usesTimeLease(), default true: ASIO/MPI), stuck in-flight items of that batch are reclaimed once
 * they exceed an adaptive LEASE. A transport that detects client death directly (the websocket
 * consumer, via its persistent session + CheckoutLease) reclaims immediately on disconnect and
 * disables the time lease.
 */
template <typename processable_type>
class GNetworkedConsumerT : public GBaseConsumerT<processable_type> {
public:
    using item_ptr = typename GBaseConsumerT<processable_type>::item_ptr;
    using clock = std::chrono::steady_clock;

    GNetworkedConsumerT() = default;
    ~GNetworkedConsumerT() override = default;

    /***************************************************************************/
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
     * consumer, whose client keeps an open connection while it evaluates). A session holds one of these
     * and records EVERY item it currently has in flight (a prefetching client may hold several at once);
     * if the session is destroyed while still holding unreturned items -- the client disconnected
     * mid-evaluation -- the lease requeues all of them for other clients immediately, a liveness-driven
     * put-back that does not wait out the time lease. Transports with one-shot, per-request connections
     * (ASIO) must NOT use this (their sessions end normally after every exchange); they rely on the time
     * lease instead -- see usesTimeLease().
     *
     * Items are keyed by their (batch_id, slot) correlation id, so add()/remove() pair up regardless of
     * the order results come back in. Guarded by an internal mutex: although a single session drives its
     * add()/remove() on one strand, the destructor may run on whichever io thread releases the last
     * reference, so the two must not race.
     */
    struct CheckoutLease {
        std::mutex mtx;
        // Items travel by unique_ptr, so the lease cannot co-own them: it tracks only the in-flight
        // correlation ids (a borrow). The owning copy stays in the consumer's batch; on abandon the
        // lease asks the consumer to requeue those ids.
        std::set<Gem::Courtier::CORRELATION_ID_TYPE> in_flight;
        std::function<void(Gem::Courtier::CORRELATION_ID_TYPE)> on_abandon;

        CheckoutLease() = default;
        CheckoutLease(const CheckoutLease &) = delete;
        CheckoutLease &operator=(const CheckoutLease &) = delete;

        /** @brief Records the id of an item just handed to the session. */
        void add(const item_ptr &p) {
            if(not p) {
                return;
            }
            std::lock_guard<std::mutex> lk(mtx);
            in_flight.insert(p->getCorrelationId());
        }
        /** @brief Drops an item the session returned normally (nothing left for the lease to reclaim). */
        void remove(const item_ptr &p) {
            if(not p) {
                return;
            }
            std::lock_guard<std::mutex> lk(mtx);
            in_flight.erase(p->getCorrelationId());
        }
        ~CheckoutLease() {
            std::set<Gem::Courtier::CORRELATION_ID_TYPE> remaining;
            {
                std::lock_guard<std::mutex> lk(mtx);
                remaining.swap(in_flight);
            }
            if(on_abandon) {
                for(auto id : remaining) {
                    on_abandon(id);
                }
            }
        }
    };

    /***************************************************************************/
    /** @brief Hands the next pending slot's item to a calling session, or null if none is pending.
     *  Round-robin across all active batches; the item's dispatch state flips PENDING -> IN_FLIGHT and
     *  correlation rides its (batch_id, slot) id. */
    item_ptr checkout() {
        std::lock_guard<std::mutex> lk(mtx_);
        return checkout_locked();
    }

    /***************************************************************************/
    /** @brief Like checkout(), but blocks up to @p wait for a slot to become available before
     *  giving up and returning null. Keeps a transport from churning "no data" responses during the
     *  gaps between batches. */
    item_ptr checkoutWait(std::chrono::milliseconds wait) {
        std::unique_lock<std::mutex> lk(mtx_);
        if(total_pending_ == 0) {
            cv_work_.wait_for(lk, wait, [this] { return total_pending_ > 0 || stop_.load(); });
        }
        return checkout_locked();
    }

    /***************************************************************************/
    /** @brief Accepts a returned result and writes it into its slot. The result's (batch_id, slot) is
     *  decoded from its correlation id; a result whose batch is no longer active (timed out / finished)
     *  or whose slot is no longer IN_FLIGHT (a duplicate) is dropped, which is what makes resubmission
     *  safe. */
    void checkin(item_ptr p) {
        if(not p) {
            return;
        }
        std::lock_guard<std::mutex> lk(mtx_);
        const Gem::Courtier::CORRELATION_ID_TYPE id = p->getCorrelationId();
        auto it = batches_.find(decodeBatch(id));
        if(it == batches_.end()) {
            return; // batch no longer active: a late arrival from a timed-out/finished batch
        }
        BatchState &b = it->second;
        const std::size_t slot = decodeSlot(id);
        if(slot >= b.items->size() ||
           (*b.items)[slot]->getDispatchState() != Gem::Courtier::dispatchState::IN_FLIGHT) {
            return; // out of range, or duplicate / not currently in flight
        }
        // Feed the (shared) adaptive timeout: how long this item took from checkout to return.
        const auto now = clock::now();
        recordReturnTime_(now - b.checked_out_at[slot]);
        b.last_progress = now;

        // Replace the work item in this broker slot with the returned result. The broker deals in bare
        // individuals; all OA scratch (the personality object) now lives on the population's
        // GIndividualSlot, NOT on the individual, so replacing the individual is lossless -- the
        // optimization algorithm swaps the reconciled individual back into its slot, which still holds
        // its own personality.
        p->setDispatchState(Gem::Courtier::dispatchState::DONE);
        (*b.items)[slot] = std::move(p);
        ++b.done;
        if(b.done == b.target) {
            cv_done_.notify_all(); // each waiting dispatch_ re-checks its own batch
        }
    }

    /***************************************************************************/
    /** @brief Returns a slot's still-in-flight item to PENDING so another client picks it up
     *  immediately (the RAII put-back on a client disconnect). Caller passes the item it checked out;
     *  the batch+slot are located via its correlation id. A no-op if the batch moved on or the slot is
     *  no longer in flight. */
    void requeue(Gem::Courtier::CORRELATION_ID_TYPE id) {
        std::lock_guard<std::mutex> lk(mtx_);
        auto it = batches_.find(decodeBatch(id));
        if(it == batches_.end()) {
            return;
        }
        if(requeueSlot_locked(it->second, decodeSlot(id))) {
            cv_work_.notify_one();
        }
    }

    /***************************************************************************/
    /** @brief Whether the server is being torn down (sessions use this to stop accepting work). */
    [[nodiscard]] bool stopped() const noexcept { return stop_.load(); }

    /** @brief Requests teardown -- subclasses call this from their stop path. Wakes every waiter. */
    void requestStop() noexcept {
        stop_.store(true);
        cv_work_.notify_all();
        cv_done_.notify_all();
    }

    /***************************************************************************/
    /** @brief Whether this consumer reclaims stuck in-flight items via the TIME lease. True for
     *  transports with no per-client liveness signal (ASIO, MPI): an item that has been IN_FLIGHT
     *  longer than the adaptive lease is presumed lost and requeued. A transport that detects client
     *  death directly (the websocket consumer, via CheckoutLease on its persistent session) overrides
     *  this to false, so a live-but-slow client is never wrongly reclaimed. */
    virtual bool usesTimeLease() const { return true; }

    /***************************************************************************/
    /**
     * Registers @p items as a fresh batch (state lives on the items themselves, the vector is merely
     * BORROWED), waits for them to return (or time out), and -- because checkin() writes results
     * straight into their slots -- simply deregisters and returns. Items that never reached DONE are
     * left DO_PROCESS == MISSING for the reconciliation loop. Safe to call concurrently from many
     * threads: each call owns a distinct batch_id; the shared client pool is served round-robin.
     */
    void dispatch_(std::vector<item_ptr> &items) override {
        const std::size_t n = items.size();
        if(n == 0) {
            return;
        }

        const auto start = clock::now();
        typename std::map<batch_key_t, BatchState>::iterator my_it;
        {
            std::lock_guard<std::mutex> lk(mtx_);
            const batch_key_t key = (next_batch_id_++ & BATCH_MASK);
            BatchState b;
            b.items = &items;
            b.target = n;
            b.pending = n;
            b.cursor = 0;
            b.checked_out_at.assign(n, start);
            b.last_progress = start;
            for(std::size_t k = 0; k < n; ++k) {
                // (batch_id, slot) correlation token; slot is the index into this round's batch.
                items[k]->setCorrelationId(encodeId(key, k));
                items[k]->setDispatchState(Gem::Courtier::dispatchState::PENDING);
            }
            my_it = batches_.emplace(key, std::move(b)).first;
            total_pending_ += n;
        }
        cv_work_.notify_all(); // wake any session blocked in checkoutWait()

        // Wait until every slot of THIS batch is DONE, or its progress stalls past the (adaptive)
        // give-up window. The happy path wakes on the final checkin() and exits at once. While
        // waiting, where the transport has no liveness signal, periodically reclaim this batch's items
        // that have been in flight longer than the adaptive lease.
        {
            std::unique_lock<std::mutex> lk(mtx_);
            BatchState &b = my_it->second; // stable across wait_for: std::map refs survive other ins/erase
            while(true) {
                if(b.done == b.target || stop_.load()) {
                    break;
                }
                const auto now = clock::now();
                // Never give up while NO result has EVER been received: under a batch-scheduling system
                // the first worker may enter the pool arbitrarily later (minutes, even hours), so we
                // wait indefinitely for the first return. The give-up window only bounds how long we
                // keep waiting once workers ARE returning (it reclaims items handed to workers that
                // have since gone slow/dead) -- it must not bound the wait for workers to appear.
                if(n_return_samples_ > 0 && now - b.last_progress > currentStallWindow()) {
                    break; // progress stalled with live workers -> give up; unresolved slots MISSING
                }
                if(this->usesTimeLease()) {
                    leaseSweep_locked(b, now);
                }
                cv_done_.wait_for(lk, sweep_tick_);
            }
            // Slots are already updated in place by checkin(); slots never reached DONE keep their
            // original DO_PROCESS item == MISSING for the policy loop. Drop the still-PENDING slots
            // from the global count and deregister the batch (its borrowed vector is about to leave
            // dispatch_'s scope), so any very late checkin() for it becomes a no-op.
            total_pending_ -= b.pending;
            batches_.erase(my_it);
        }
    }

private:
    /***************************************************************************/
    // The work item's correlation id (a uint32) carries the wire correlation token: the high 16 bits
    // hold the batch_id (which active submitter's batch), the low 16 the slot index within it. NB:
    // this bounds a single batch to 2^16 items (ample for any Geneva population) and the batch_id
    // wraps every 2^16 dispatch rounds -- a harmless, astronomically unlikely aliasing, since a stale
    // return only matters within a timeout window of its dispatch.
    using batch_key_t = Gem::Courtier::CORRELATION_ID_TYPE;
    static constexpr Gem::Courtier::CORRELATION_ID_TYPE SLOT_BITS = 16;
    static constexpr Gem::Courtier::CORRELATION_ID_TYPE SLOT_MASK = (1u << SLOT_BITS) - 1u;
    static constexpr Gem::Courtier::CORRELATION_ID_TYPE BATCH_MASK = 0xFFFFu;

    static Gem::Courtier::CORRELATION_ID_TYPE encodeId(batch_key_t batch, std::size_t slot) {
        return ((batch & BATCH_MASK) << SLOT_BITS)
               | (static_cast<Gem::Courtier::CORRELATION_ID_TYPE>(slot) & SLOT_MASK);
    }
    static batch_key_t decodeBatch(Gem::Courtier::CORRELATION_ID_TYPE id) {
        return (id >> SLOT_BITS) & BATCH_MASK;
    }
    static std::size_t decodeSlot(Gem::Courtier::CORRELATION_ID_TYPE id) {
        return static_cast<std::size_t>(id & SLOT_MASK);
    }

    /***************************************************************************/
    /** @brief Per-batch scheduling state. `items` is a BORROWED pointer to the caller's round vector
     *  (valid only while that call's dispatch_ blocks); everything else is this batch's bookkeeping. */
    struct BatchState {
        std::vector<item_ptr> *items = nullptr; ///< Borrowed round vector (not owned)
        std::size_t target = 0;                 ///< Number of slots in this batch
        std::size_t done = 0;                   ///< Slots that have reached DONE
        std::size_t pending = 0;                ///< Slots currently PENDING (awaiting a client)
        std::size_t cursor = 0;                 ///< Next slot index to consider in checkout
        std::vector<clock::time_point> checked_out_at; ///< Per-slot checkout time (lease + stats)
        clock::time_point last_progress{};      ///< Time of the most recent checkin (stall basis)
    };

    /***************************************************************************/
    /** @brief Serves the next PENDING slot, round-robin across active batches for fairness (so no
     *  single submitter starves the others). Caller holds mtx_. */
    item_ptr checkout_locked() {
        if(total_pending_ == 0 || batches_.empty()) {
            return nullptr;
        }
        // Start just past the last-served batch and walk the (ordered) map once, wrapping around.
        auto it = batches_.upper_bound(last_served_batch_);
        for(std::size_t scanned = 0; scanned < batches_.size(); ++scanned) {
            if(it == batches_.end()) {
                it = batches_.begin();
            }
            BatchState &b = it->second;
            while(b.cursor < b.items->size() &&
                  (*b.items)[b.cursor]->getDispatchState() != Gem::Courtier::dispatchState::PENDING) {
                ++b.cursor;
            }
            if(b.cursor < b.items->size()) {
                const std::size_t k = b.cursor++;
                (*b.items)[k]->setDispatchState(Gem::Courtier::dispatchState::IN_FLIGHT);
                b.checked_out_at[k] = clock::now();
                --b.pending;
                --total_pending_;
                last_served_batch_ = it->first;
                // Hand the session a clone to serialize and ship; the owning copy stays in the slot,
                // marked IN_FLIGHT. Its correlation id rides the clone, so checkin() finds the slot again.
                // Clone via the consumer's type-generic helper (polymorphic functor or copy-construct).
                return this->clone_item_((*b.items)[k]);
            }
            ++it;
        }
        return nullptr;
    }

    /***************************************************************************/
    /** @brief Flips a slot IN_FLIGHT -> PENDING (a put-back) within batch @p b and rewinds its cursor
     *  so checkout re-finds it. Returns whether anything was flipped. Caller holds mtx_. */
    bool requeueSlot_locked(BatchState &b, std::size_t slot) {
        if(slot >= b.items->size() ||
           (*b.items)[slot]->getDispatchState() != Gem::Courtier::dispatchState::IN_FLIGHT) {
            return false;
        }
        (*b.items)[slot]->setDispatchState(Gem::Courtier::dispatchState::PENDING);
        ++b.pending;
        ++total_pending_;
        if(slot < b.cursor) {
            b.cursor = slot;
        }
        return true;
    }

    /***************************************************************************/
    /** @brief Reclaims every slot of batch @p b that has been IN_FLIGHT longer than the adaptive
     *  lease, returning it to PENDING for re-service. Caller holds mtx_. */
    void leaseSweep_locked(BatchState &b, clock::time_point now) {
        const auto lease = currentLease();
        bool any = false;
        for(std::size_t k = 0; k < b.items->size(); ++k) {
            if((*b.items)[k]->getDispatchState() == Gem::Courtier::dispatchState::IN_FLIGHT &&
               (now - b.checked_out_at[k]) > lease) {
                any = requeueSlot_locked(b, k) || any;
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
    /** @brief The current give-up window: how long a batch's dispatch_ tolerates NO progress before
     *  declaring the rest of that batch MISSING. A generous multiple of the running mean return time,
     *  clamped. Only ever consulted once n_return_samples_ > 0 (dispatch_ waits indefinitely before
     *  the first return ever arrives), so there is no pre-sample fallback here. */
    std::chrono::milliseconds currentStallWindow() const {
        const auto v = std::chrono::milliseconds(static_cast<long long>(stall_factor_ * mean_return_ms_));
        return std::clamp(v, min_stall_, max_stall_);
    }

    mutable std::mutex mtx_;
    std::condition_variable cv_done_; ///< Signalled when a batch's last slot reaches DONE
    std::condition_variable cv_work_; ///< Signalled when a slot becomes available

    std::map<batch_key_t, BatchState> batches_; ///< All currently-active batches, keyed by batch_id
    batch_key_t next_batch_id_ = 0;             ///< Monotonic batch_id source (masked to 16 bits)
    batch_key_t last_served_batch_ = 0;         ///< Round-robin cursor across batches (for fairness)
    std::size_t total_pending_ = 0;             ///< Slots PENDING across ALL batches (cv predicate)

    double mean_return_ms_ = 0.0;       ///< Running (EMA) mean checkout->return time (consumer-wide)
    std::size_t n_return_samples_ = 0;  ///< Returns observed so far (across all batches)

    std::atomic<bool> stop_{false};

    // --- adaptive-timeout configuration (sane defaults; tunable via the setters) ---
    double ema_alpha_ = 0.25;   ///< EMA weight for new return-time samples
    double lease_factor_ = 4.0; ///< Reclaim lease = lease_factor_ * mean return time (clamped)
    double stall_factor_ = 8.0; ///< Give-up window = stall_factor_ * mean return time (clamped)
    std::chrono::milliseconds lease_bootstrap_{10'000};     ///< Reclaim lease before any return is seen
    std::chrono::milliseconds min_lease_{200};              ///< Lower clamp on the reclaim lease
    std::chrono::milliseconds max_lease_{300'000};          ///< Upper clamp on the reclaim lease
    std::chrono::milliseconds min_stall_{2'000};            ///< Lower clamp on the give-up window
    std::chrono::milliseconds max_stall_{600'000};          ///< Upper clamp on the give-up window
    std::chrono::milliseconds sweep_tick_{50};              ///< dispatch_ re-evaluation poll interval
};

/******************************************************************************/

} /* namespace Gem::Courtier */
