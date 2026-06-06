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

/**
 * Tier-1 tests for the courtier Phase-5 machinery -- the adaptive timeout / death-detection (the
 * reclaim lease + the explicit put-back) and the clone-from-template refill. The networked timeout
 * logic lives in GNetworkedConsumerT independently of any actual socket, so it is exercised here
 * deterministically by a test subclass acting as a simulated transport: a driver thread plays the
 * role of clients (checkout / process / checkin), and can ABANDON an item (never check it back in,
 * = a client that died mid-evaluation) or explicitly REQUEUE one (= a websocket session-death
 * put-back). In both cases every item must still come back, via the reclaim lease / the put-back.
 */

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstddef>
#include <memory>
#include <set>
#include <thread>
#include <vector>

#include "courtier/GDemoProcessingContainers.hpp"
#include "courtier/GBrokerT.hpp"
#include "courtier/GExecutorT.hpp"
#include "courtier/GSubmissionPolicy.hpp"
#include "courtier/consumers/GNetworkedConsumerT.hpp"
#include "courtier/consumers/GStdThreadConsumerT.hpp"

using Gem::Courtier::fault_mode;
using Gem::Courtier::GFaultyContainer;
namespace c2 = Gem::Courtier;
using namespace std::chrono_literals;

namespace {

using item_ptr = std::shared_ptr<GFaultyContainer>;

std::vector<item_ptr> make_batch(std::size_t n, const std::vector<std::size_t> &faulty = {},
                                 fault_mode fm = fault_mode::THROW_PROCESSING) {
    std::vector<item_ptr> v;
    v.reserve(n);
    for(std::size_t i = 0; i < n; ++i) {
        const bool f = std::find(faulty.begin(), faulty.end(), i) != faulty.end();
        v.push_back(std::make_shared<GFaultyContainer>(i, f ? fm : fault_mode::NONE));
    }
    return v;
}

std::size_t count_processed(const std::vector<item_ptr> &v) {
    std::size_t c = 0;
    for(const auto &it : v) {
        if(it && it->is_processed()) {
            ++c;
        }
    }
    return c;
}

/** @brief A GNetworkedConsumerT with no real transport: it exposes the checkout/checkin/requeue
 *  endpoints so a test driver thread can play "clients" against the dispatch_ machinery. */
class SimNetConsumer : public c2::GNetworkedConsumerT<GFaultyContainer> {
public:
    using c2::GNetworkedConsumerT<GFaultyContainer>::checkout;
    using c2::GNetworkedConsumerT<GFaultyContainer>::checkin;
    using c2::GNetworkedConsumerT<GFaultyContainer>::requeue;
};

/** @brief How the driver mistreats a chosen item the first time it sees it. */
enum class misbehave { abandon, requeue_it };

/**
 * Runs @p batch through a SimNetConsumer with a driver thread that processes items, but the FIRST
 * time it sees an item selected by @p victim it abandons or requeues it (per @p mode) -- simulating
 * a client death (reclaimed by the lease) or a session-death put-back. Returns once every slot is
 * resolved.
 */
template <typename VictimPred>
void run_with_misbehaviour(std::vector<item_ptr> &batch, misbehave mode, VictimPred victim) {
    SimNetConsumer consumer;
    consumer.setLeaseBootstrap(30ms);  // reclaim an abandoned item quickly
    consumer.setLeaseBounds(10ms, 5s);
    consumer.setSweepTick(5ms);

    std::atomic<bool> finished{false};
    std::jthread worker([&] {
        consumer.processBatch(
            std::span<item_ptr>(batch.data(), batch.size()),
            c2::GSubmissionPolicy::full_success_or_fatal()
        );
        finished.store(true);
    });

    std::set<std::size_t> mistreated_once;
    while(not finished.load()) {
        auto p = consumer.checkout();
        if(not p) {
            std::this_thread::sleep_for(1ms);
            continue;
        }
        const std::size_t id = p->get_stored_number();
        if(victim(id) && mistreated_once.insert(id).second) {
            if(mode == misbehave::requeue_it) {
                consumer.requeue(p); // immediate put-back (as a websocket session-death would do)
            }
            // mode == abandon: simply drop it -- never check it in; the reclaim lease must recover it
            continue;
        }
        p->process();
        consumer.checkin(p);
    }
    worker.join();
}

} /* anonymous namespace */

/******************************************************************************/

TEST_CASE("courtier(timeout): the reclaim lease recovers abandoned (dead-client) items",
          "[courtier][timeout]") {
    auto batch = make_batch(40);
    run_with_misbehaviour(batch, misbehave::abandon, [](std::size_t id) { return id % 4 == 0; });
    CHECK(count_processed(batch) == 40); // every item came back despite the "deaths"
}

TEST_CASE("courtier(timeout): an explicit requeue (session-death put-back) loses nothing",
          "[courtier][timeout]") {
    auto batch = make_batch(40);
    run_with_misbehaviour(batch, misbehave::requeue_it, [](std::size_t id) { return id % 3 == 0; });
    CHECK(count_processed(batch) == 40);
}

TEST_CASE("courtier(timeout): every item abandoned once is still recovered",
          "[courtier][timeout]") {
    auto batch = make_batch(24);
    run_with_misbehaviour(batch, misbehave::abandon, [](std::size_t) { return true; });
    CHECK(count_processed(batch) == 24);
}

/******************************************************************************/

TEST_CASE("courtier(clone): unresolved slots are refilled from the supplied template",
          "[courtier][clone]") {
    // A representative, already-evaluated template the algorithm hands down.
    constexpr std::size_t TEMPLATE_ID = 9999;
    auto tmpl = std::make_shared<GFaultyContainer>(TEMPLATE_ID, fault_mode::NONE);
    tmpl->set_processing_status(Gem::Courtier::processingStatus::DO_PROCESS);
    tmpl->process(); // a valid, evaluated template

    auto broker = std::make_shared<c2::GBrokerT<GFaultyContainer>>();
    broker->registerConsumer(std::make_shared<c2::GStdThreadConsumerT<GFaultyContainer>>(4));
    c2::GExecutorT<GFaultyContainer> executor(broker);

    const std::vector<std::size_t> faulty{2, 5, 9};
    auto batch = make_batch(12, faulty, fault_mode::THROW_PROCESSING);

    executor.workOn(batch, c2::GSubmissionPolicy::clone_on_partial_return(), tmpl);

    CHECK(count_processed(batch) == 12); // no slot left unevaluated
    // The throwing slots must hold clones of the TEMPLATE (not of a surviving sibling).
    for(std::size_t i : faulty) {
        CHECK(batch[i]->get_stored_number() == TEMPLATE_ID);
        CHECK(batch[i]->is_processed());
    }
    // A non-faulty slot keeps its own identity.
    CHECK(batch[0]->get_stored_number() == 0);
}

/******************************************************************************/
