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
#include <deque>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>

// Geneva headers
#include "courtier/GCourtierEnums.hpp" // BUFFERPORT_ID_TYPE, processingStatus
#include "courtier2/GBaseConsumerT.hpp"

namespace Gem::Courtier2 {

/******************************************************************************/
/**
 * Base class for networked courtier2 consumers (ASIO, websocket, MPI). It owns the per-batch work
 * machinery that turns the synchronous reconcile-the-span contract of GBaseConsumerT into the
 * asynchronous, checkout/return world of remote clients:
 *
 *  - dispatch_(items) loads the round's items into a pending queue (each tagged with a unique id
 *    reusing the otherwise-unused bufferport id field), then blocks until every item has come back
 *    or a bounded timeout elapses. Items that never return are left DO_PROCESS == MISSING, so the
 *    inherited reconciliation loop resubmits/clones/fails them per the policy.
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
    /** @brief Hands the next pending item to a calling session, or null if none is pending. */
    item_ptr checkout() {
        std::lock_guard<std::mutex> lk(mtx_);
        if(pending_.empty()) {
            return nullptr;
        }
        auto p = pending_.front();
        pending_.pop_front();
        outstanding_[p->getBufferId()] = p;
        return p;
    }

    /***************************************************************************/
    /** @brief Accepts a returned result and matches it to its slot by id. A result whose id is not
     *  currently outstanding (late/duplicate) is dropped. */
    void checkin(item_ptr p) {
        if(not p) {
            return;
        }
        std::lock_guard<std::mutex> lk(mtx_);
        const Gem::Courtier::BUFFERPORT_ID_TYPE id = p->getBufferId();
        auto it = outstanding_.find(id);
        if(it == outstanding_.end()) {
            return; // stale or duplicate -- safe to ignore
        }
        outstanding_.erase(it);
        results_[id] = p;
        if(results_.size() == n_pending_) {
            cv_done_.notify_one();
        }
    }

    /***************************************************************************/
    /** @brief Whether the server is being torn down (sessions use this to stop accepting work). */
    [[nodiscard]] bool stopped() const noexcept { return stop_.load(); }

    /** @brief Requests teardown -- subclasses call this from their stop path. */
    void requestStop() noexcept { stop_.store(true); }

    /***************************************************************************/
    /**
     * Loads one round of items into the pending queue, waits for them to return (or time out), and
     * writes the returned results back into @p items. Unresolved items are left DO_PROCESS.
     */
    void dispatch_(std::vector<item_ptr> &items) override {
        const std::size_t n = items.size();
        if(n == 0) {
            return;
        }

        std::unordered_map<Gem::Courtier::BUFFERPORT_ID_TYPE, std::size_t> slot_of_id;
        {
            std::lock_guard<std::mutex> lk(mtx_);
            pending_.clear();
            outstanding_.clear();
            results_.clear();
            n_pending_ = n;
            for(std::size_t k = 0; k < n; ++k) {
                const Gem::Courtier::BUFFERPORT_ID_TYPE id = next_id_++;
                if(next_id_ == 0) { // never hand out 0; keeps ids stable across the unlikely wrap
                    next_id_ = 1;
                }
                items[k]->setBufferId(id);
                slot_of_id[id] = k;
                pending_.push_back(items[k]);
            }
        }

        // Bounded wait: a first-item budget plus a per-item budget. The predicate returns as soon as
        // every item is back, so the happy path does not wait for the deadline.
        const auto deadline = clock::now() + this->batchDeadline(n);
        {
            std::unique_lock<std::mutex> lk(mtx_);
            cv_done_.wait_until(lk, deadline, [this] {
                return results_.size() == n_pending_ || stop_.load();
            });

            for(const auto &[id, res] : results_) {
                auto sit = slot_of_id.find(id);
                if(sit != slot_of_id.end()) {
                    items[sit->second] = res; // replace the slot with the processed copy
                }
            }
            // Items not in results_ stay DO_PROCESS (== MISSING) for the reconciliation loop.
            pending_.clear();
            outstanding_.clear();
            results_.clear();
            n_pending_ = 0;
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
    mutable std::mutex mtx_;
    std::condition_variable cv_done_;

    std::deque<item_ptr> pending_;                                              ///< Items awaiting a client
    std::unordered_map<Gem::Courtier::BUFFERPORT_ID_TYPE, item_ptr> outstanding_; ///< Checked out, not back
    std::unordered_map<Gem::Courtier::BUFFERPORT_ID_TYPE, item_ptr> results_;     ///< Returned this round
    std::size_t n_pending_ = 0;
    Gem::Courtier::BUFFERPORT_ID_TYPE next_id_ = 1;

    std::atomic<bool> stop_{false};

    std::chrono::milliseconds first_item_max_wait_{60'000}; ///< Max wait for the first result of a batch
    std::chrono::milliseconds per_item_wait_{1'000};        ///< Added per item to the batch deadline
};

/******************************************************************************/

} /* namespace Gem::Courtier2 */
