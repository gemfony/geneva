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
 */
template <typename processable_type>
class GStdThreadConsumerT final : public GBaseConsumerT<processable_type> {
public:
    using item_ptr = typename GBaseConsumerT<processable_type>::item_ptr;

    /***************************************************************************/
    /** @brief Initialization with the number of worker threads (0 == hardware concurrency). */
    explicit GStdThreadConsumerT(unsigned int n_threads = 0)
        : pool_(n_threads == 0 ? default_threads() : n_threads)
    { /* nothing */ }

    ~GStdThreadConsumerT() override = default;

protected:
    /***************************************************************************/
    /**
     * Evaluates all items of one round concurrently. process() sets PROCESSED on success and
     * EXCEPTION_CAUGHT on a caught processing exception (which it also re-throws -- swallowed here).
     *
     * Waits on a PER-BATCH counter rather than GThreadPool::wait() (a global drain barrier): the pool
     * is shared, so several algorithms can submit concurrently (the fan-in case -- e.g. a
     * meta-optimization over a population of inner algorithms), and each must wait for ONLY its own
     * items, not the whole pool. The captured @c remaining (shared, kept alive by the tasks) and the
     * stack @c m / @c cv are valid throughout because dispatch_ blocks until every task has run.
     */
    void dispatch_(std::vector<item_ptr> &items) override {
        if(items.empty()) {
            return;
        }
        auto remaining = std::make_shared<std::atomic<std::size_t>>(items.size());
        std::mutex m;
        std::condition_variable cv;
        for(auto &it : items) {
            pool_.post([it, remaining, &m, &cv]() {
                try {
                    it->process();
                }
                catch(...) {
                    // The item's status already reflects the failure (EXCEPTION_CAUGHT); the
                    // re-thrown exception is intentionally swallowed so it never escapes the
                    // worker thread. Reconciliation reads the status, not an exception.
                }
                if(remaining->fetch_sub(1) == 1) { // this was the last item of THIS batch
                    std::lock_guard<std::mutex> lk(m);
                    cv.notify_one();
                }
            });
        }
        std::unique_lock<std::mutex> lk(m);
        cv.wait(lk, [&] { return remaining->load() == 0; });
    }

private:
    static unsigned int default_threads() {
        const unsigned int hc = std::thread::hardware_concurrency();
        return hc == 0 ? 1u : hc;
    }

    Gem::Common::GThreadPool pool_;
};

/******************************************************************************/

} /* namespace Gem::Courtier */
