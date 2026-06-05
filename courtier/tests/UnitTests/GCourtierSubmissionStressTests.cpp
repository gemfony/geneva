/**
 * @file GCourtierSubmissionStressTests.cpp
 */

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

// This is the LOCAL (no-network) tier of the courtier submission-path stress harness.
// It drives the full broker + threaded-consumer + worker + GBrokerExecutorT path with the
// lightweight, fault-injecting GFaultyContainer (no dependency on the geneva optimization
// library), and asserts the two invariants the audit (prompts/2026-06-04-submission-path-audit.md)
// cares about: (1) item conservation under load, and (2) a single misbehaving work item must
// not take down the consumer / the process.
//
// A separate, networked tier (loopback ASIO/websocket consumer + in-process client with
// fault-injection seams) will be added alongside the T1 fixes.

// Standard headers
#include <atomic>
#include <chrono>
#include <cstddef>
#include <memory>
#include <vector>

// Catch2
#include <catch2/catch_test_macros.hpp>

// Geneva / courtier
#include "courtier/GBrokerT.hpp"
#include "courtier/GDemoProcessingContainers.hpp"
#include "courtier/GExecutorT.hpp"
#include "courtier/consumers/GStdThreadConsumerT.hpp"

using namespace Gem::Courtier;
using Gem::Courtier::Consumers::GStdThreadConsumerT;

namespace {

/******************************************************************************/
/**
 * RAII wiring for one isolated submission-path test: a fresh broker singleton with a
 * multi-threaded consumer enrolled. The broker is a per-type singleton, so each test must
 * reset it on the way in and out to stay independent.
 */
struct ThreadedBrokerFixture {
    std::shared_ptr<GStdThreadConsumerT<GFaultyContainer>> consumer;

    explicit ThreadedBrokerFixture(std::size_t n_threads) {
        resetBroker<GFaultyContainer>(); // start from a clean singleton
        consumer = std::make_shared<GStdThreadConsumerT<GFaultyContainer>>(n_threads);
        broker<GFaultyContainer>()->init();
        broker<GFaultyContainer>()->enrol_consumer(consumer);
    }

    ~ThreadedBrokerFixture() {
        broker<GFaultyContainer>()->finalize(); // shuts the consumer down + joins workers
        resetBroker<GFaultyContainer>();
    }

    ThreadedBrokerFixture(const ThreadedBrokerFixture &) = delete;
    ThreadedBrokerFixture &operator=(const ThreadedBrokerFixture &) = delete;
    ThreadedBrokerFixture(ThreadedBrokerFixture &&) = delete;
    ThreadedBrokerFixture &operator=(ThreadedBrokerFixture &&) = delete;
};

/** @brief Builds a work item with the given id + fault mode, already due for processing. */
std::shared_ptr<GFaultyContainer> make_item(
    std::size_t id,
    fault_mode fm = fault_mode::NONE,
    unsigned int sleep_ms = 0
) {
    auto item = std::make_shared<GFaultyContainer>(id, fm, sleep_ms);
    item->set_processing_status(processingStatus::DO_PROCESS);
    return item;
}

} // namespace

/******************************************************************************/

TEST_CASE("submission(local): all items are conserved under load", "[courtier][submission][local]") {
    ThreadedBrokerFixture fx(8);
    GBrokerExecutorT<GFaultyContainer> exec;
    exec.init();

    constexpr std::size_t N = 5000;
    std::vector<std::shared_ptr<GFaultyContainer>> items;
    items.reserve(N);
    for(std::size_t i = 0; i < N; ++i) {
        items.push_back(make_item(i, fault_mode::NONE));
    }

    auto status = exec.workOn(items);
    exec.finalize();

    REQUIRE(status.is_complete);
    REQUIRE_FALSE(status.has_errors);
    REQUIRE(items.size() == N); // no item vanished from the vector

    std::size_t processed = 0;
    for(auto const &it : items) {
        if(it && it->is_processed()) {
            ++processed;
        }
    }
    REQUIRE(processed == N); // every single item came back processed
}

/******************************************************************************/

TEST_CASE("submission(local): slow workers still return every item", "[courtier][submission][local]") {
    ThreadedBrokerFixture fx(4);
    GBrokerExecutorT<GFaultyContainer> exec;
    exec.init();

    constexpr std::size_t N = 400;
    std::vector<std::shared_ptr<GFaultyContainer>> items;
    items.reserve(N);
    for(std::size_t i = 0; i < N; ++i) {
        // Every fourth item sleeps briefly to force the queue to drain out of order.
        const bool slow = (i % 4 == 0);
        items.push_back(make_item(i, slow ? fault_mode::SLEEP : fault_mode::NONE, slow ? 5u : 0u));
    }

    auto status = exec.workOn(items);
    exec.finalize();

    REQUIRE(status.is_complete);
    REQUIRE_FALSE(status.has_errors);
    for(auto const &it : items) {
        REQUIRE(it->is_processed());
    }
}

/******************************************************************************/

TEST_CASE(
    "submission(local): a misbehaving work item is flagged, not fatal",
    "[courtier][submission][local]"
) {
    // CHARACTERISATION TEST. GProcessingContainerT::process() funnels every exception thrown by
    // process_() into a g_processing_exception, which GWorkerT::process() catches. So a work item
    // that throws -- even a plain std::runtime_error -- must come back flagged with an error while
    // the consumer keeps serving the rest of the batch and the process stays alive. (This narrows
    // the audit's P0.1: a throwing fitnessCalculation does NOT terminate the worker; only an
    // exception escaping retrieve()/submit() does.) If this assumption were wrong, the run would
    // std::terminate here instead of failing an assertion.
    ThreadedBrokerFixture fx(4);
    GBrokerExecutorT<GFaultyContainer> exec;
    exec.init();

    constexpr std::size_t N_GOOD = 200;
    std::vector<std::shared_ptr<GFaultyContainer>> items;

    // Interleave good items with each kind of misbehaviour.
    for(std::size_t i = 0; i < N_GOOD; ++i) {
        items.push_back(make_item(i, fault_mode::NONE));
        if(i % 50 == 0) {
            items.push_back(make_item(1000 + i, fault_mode::THROW_PROCESSING));
            items.push_back(make_item(2000 + i, fault_mode::THROW_FATAL));
            items.push_back(make_item(3000 + i, fault_mode::FLAG_ERROR));
        }
    }

    auto status = exec.workOn(items);
    exec.finalize();

    // The headline assertion: we got here at all -> no std::terminate from a throwing item.
    SUCCEED("the consumer survived misbehaving work items");

    // Every well-behaved item must still have been processed.
    std::size_t good_processed = 0;
    for(auto const &it : items) {
        if(it->get_fault_mode() == fault_mode::NONE && it->is_processed()) {
            ++good_processed;
        }
    }
    REQUIRE(good_processed == N_GOOD);

    // And the batch must report that something went wrong.
    REQUIRE(status.has_errors);
}
