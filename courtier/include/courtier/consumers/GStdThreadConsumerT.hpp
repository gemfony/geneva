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
#include <condition_variable>
#include <cstddef>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

// Geneva headers (reused from the common library)
#include "common/GThreadPool.hpp"
#include "courtier/GBaseConsumerT.hpp"

namespace Gem::Courtier {

/******************************************************************************/
/**
 * A local, multi-threaded consumer. Each round's items are evaluated concurrently on a
 * Gem::Common::GThreadPool. A local evaluation never goes MISSING -- it either succeeds
 * (PROCESSED) or, when the user's fitnessCalculation() throws, is funnelled by
 * GProcessingContainerT::process() into the item's EXCEPTION_CAUGHT status (the throw is caught
 * here so it never escapes the worker thread). Reconciliation against the policy is inherited
 * from GBaseConsumerT.
 *
 * @tparam processable_type The work-item type evaluated by the worker threads.
 */
template <typename processable_type>
class GStdThreadConsumerT final : public GBaseConsumerT<processable_type> {
public:
    using item_ptr = typename GBaseConsumerT<processable_type>::item_ptr;

    /***************************************************************************/
    /** @brief Initialization with the number of worker threads (0 == hardware concurrency).
     *  @param n_threads Number of worker threads in the pool; 0 means use the hardware concurrency. */
    explicit GStdThreadConsumerT(unsigned int n_threads = 0)
        : pool_(n_threads == 0 ? default_threads() : n_threads)
    {
        instances_constructed().fetch_add(1, std::memory_order_relaxed);
    }

    ~GStdThreadConsumerT() override = default;

    /***************************************************************************/
    /** @brief Process-wide count of how many thread-pool consumers (of this work-item type) have ever
     *  been constructed. Each instance owns its own GThreadPool, so this also counts the worker pools
     *  built. The single-shared-consumer goal is that every un-injected algorithm needing this KIND
     *  converges on ONE shared instance, so a whole process (even a nested EA-in-EA) builds exactly one;
     *  the count is the observable for that invariant. Lightweight (one relaxed atomic add per ctor).
     *  @return A reference to the process-wide construction counter. */
    static std::atomic<std::size_t> &instances_constructed() {
        static std::atomic<std::size_t> counter{0};
        return counter;
    }

protected:
    /***************************************************************************/
    /**
     * @brief Evaluates all items of one round concurrently. process() sets PROCESSED on success and
     * EXCEPTION_CAUGHT on a caught processing exception (which it also re-throws -- swallowed here).
     *
     * Waits on a PER-BATCH counter rather than GThreadPool::wait() (a global drain barrier): the pool
     * is shared, so several algorithms can submit concurrently (the fan-in case -- e.g. a
     * meta-optimization over a population of inner algorithms), and each must wait for ONLY its own
     * items, not the whole pool.
     *
     * The synchronisation state (@c remaining, @c m, @c cv) is heap-allocated and captured by the tasks
     * AS WELL AS held by the waiter, so it lives until the last party drops its reference. It must NOT be
     * stack-allocated: the waiter's predicate (remaining == 0) is satisfied by the atomic decrement, which
     * happens BEFORE the last worker takes @c m to notify. So the waiter can wake and return -- destroying
     * stack-allocated m/cv -- while that worker is still about to lock m, locking freed memory (a
     * use-after-scope: glibc aborts with "mutex->__data.__owner == 0"). Keeping m/cv alive via shared_ptr
     * makes the at-most-redundant late notify harmless instead of fatal.
     *
     * @param items The work items of one round; each is evaluated on the shared pool and the call blocks until all have finished
     */
    void dispatch_(std::vector<item_ptr> &items) override {
        if(items.empty()) {
            return;
        }
        auto remaining = std::make_shared<std::atomic<std::size_t>>(items.size());
        auto m = std::make_shared<std::mutex>();
        auto cv = std::make_shared<std::condition_variable>();
        for(auto &it : items) {
            // Items travel by unique_ptr; the task borrows a raw pointer rather than copying the owner.
            // The batch (items) outlives every task because dispatch_ blocks until cv fires below.
            processable_type *raw = it.get();
            pool_.post([raw, remaining, m, cv]() {
                try {
                    raw->process();
                }
                catch(...) {
                    // The item's status already reflects the failure (EXCEPTION_CAUGHT); the
                    // re-thrown exception is intentionally swallowed so it never escapes the
                    // worker thread. Reconciliation reads the status, not an exception.
                }
                if(remaining->fetch_sub(1) == 1) { // this was the last item of THIS batch
                    std::scoped_lock lk(*m);
                    cv->notify_one();
                }
            });
        }
        std::unique_lock<std::mutex> lk(*m);
        cv->wait(lk, [&] { return remaining->load() == 0; });
    }

private:
    /** @brief Default number of worker threads when none was requested.
     *  @return The hardware concurrency, or 1 if it cannot be determined. */
    static unsigned int default_threads() {
        const unsigned int hc = std::thread::hardware_concurrency();
        return hc == 0 ? 1u : hc;
    }

    Gem::Common::GThreadPool pool_;
};

/******************************************************************************/

} /* namespace Gem::Courtier */
