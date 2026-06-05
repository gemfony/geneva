/**
 * @file GCourtierNetworkedStressTests.cpp
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

// This is the NETWORKED (loopback) tier of the courtier submission-path stress harness. It runs a
// real consumer (server) AND its client in the same process over 127.0.0.1, pushing the
// fault-injecting GFaultyContainer through the broker + GBrokerExecutorT, and asserts the two
// invariants from prompts/2026-06-04-submission-path-audit.md: item conservation, and "a single
// misbehaving client must not lose items or take itself down".
//
// The client's run() is wrapped in a try/catch here: GBaseClientT::run() RE-THROWS on any error,
// and an exception escaping the client thread would otherwise std::terminate the whole test
// binary. Catching it lets a client-death bug surface as a failed assertion instead of a crash.
//
// Tests tagged [.t1] are hidden by default (they encode behaviour that the T1 fixes must
// establish and currently fail / may hang); run them explicitly with "[t1]".

// Standard headers
#include <atomic>
#include <cstddef>
#include <memory>
#include <thread>
#include <vector>

// Boost (only to grab a free ephemeral port for the loopback server)
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>

// Catch2
#include <catch2/catch_test_macros.hpp>

// Geneva / courtier
#include "common/GCommonEnums.hpp"
#include "courtier/GBaseClientT.hpp"
#include "courtier/GBrokerT.hpp"
#include "courtier/GCommandContainerT.hpp"
#include "courtier/GCourtierEnums.hpp"
#include "courtier/GDemoProcessingContainers.hpp"
#include "courtier/GExecutorT.hpp"
#include "courtier/consumers/GAsioConsumerT.hpp"
#include "courtier/consumers/GWebsocketConsumerT.hpp"

using namespace Gem::Courtier;

namespace {

/** @brief Asks the OS for a currently-free TCP port (closed again immediately). */
unsigned short free_tcp_port() {
    boost::asio::io_context ioc;
    boost::asio::ip::tcp::acceptor acceptor(ioc, {boost::asio::ip::tcp::v4(), 0});
    const auto port = acceptor.local_endpoint().port();
    acceptor.close();
    return port;
}

std::shared_ptr<GFaultyContainer> make_item(
    std::size_t id,
    fault_mode fm = fault_mode::NONE,
    unsigned int sleep_ms = 0
) {
    auto item = std::make_shared<GFaultyContainer>(id, fm, sleep_ms);
    item->set_processing_status(processingStatus::DO_PROCESS);
    return item;
}

/** @brief Outcome of one loopback round. */
struct RunResult {
    executor_status_t status{false, false};
    std::size_t processed = 0;
    bool client_threw = false;
};

/**
 * Wires a consumer (server) + client over loopback, drives the items through a
 * GBrokerExecutorT, then tears everything down cleanly. The broker is a per-type singleton, so
 * we reset it on both ends to keep the tests independent.
 */
RunResult run_loopback(
    std::shared_ptr<Gem::Courtier::Consumers::GBaseConsumerT<GFaultyContainer>> consumer,
    std::shared_ptr<GBaseClientT<GFaultyContainer>> client,
    std::vector<std::shared_ptr<GFaultyContainer>> &items
) {
    // NB: the consumer captured its broker pointer at construction (GAsioConsumerT::broker_ptr_
    // is initialised from broker<>() in the member initialiser). So the broker must NOT be reset
    // here -- that would strand the consumer on a dropped instance while the executor/enrol use a
    // new one, and the consumer would see no work (endless NODATA). The previous run cleans up by
    // resetting at the end, so the consumer constructed by the caller already holds the live broker.
    broker<GFaultyContainer>()->init();
    broker<GFaultyContainer>()->enrol_consumer(consumer); // starts the server

    std::atomic<bool> threw{false};
    std::jthread client_thread([client, &threw]() {
        // GBaseClientT::run() re-throws; catch here so a client-death bug is observable
        // as a failed assertion rather than std::terminate-ing the whole binary.
        try {
            client->run();
        }
        catch(...) {
            threw.store(true);
        }
    });

    GBrokerExecutorT<GFaultyContainer> exec;
    exec.init();
    auto status = exec.workOn(items);
    exec.finalize();

    // Tear down: stop the client (it checks halt() at the top of each cycle), then the server.
    client->flagCloseRequested();
    if(client_thread.joinable()) {
        client_thread.join();
    }
    broker<GFaultyContainer>()->finalize(); // shuts the consumer/server down
    resetBroker<GFaultyContainer>();        // drop the singleton so the next test starts clean

    RunResult r;
    r.status = status;
    r.client_threw = threw.load();
    for(auto const &it : items) {
        if(it && it->is_processed()) {
            ++r.processed;
        }
    }
    return r;
}

} // namespace

/******************************************************************************/

TEST_CASE("serialization: GFaultyContainer survives the wire round-trip", "[courtier][serial][net]") {
    using cmd_t = networked_consumer_payload_command;

    auto payload = make_item(7, fault_mode::THROW_PROCESSING);
    GCommandContainerT<GFaultyContainer, cmd_t> out(cmd_t::COMPUTE, payload);

    std::string wire;
    REQUIRE_NOTHROW(wire = container_to_string(out, Gem::Common::serializationMode::BINARY));

    GCommandContainerT<GFaultyContainer, cmd_t> in(cmd_t::NONE);
    REQUIRE_NOTHROW(container_from_string(wire, in, Gem::Common::serializationMode::BINARY));

    REQUIRE(in.get_command() == cmd_t::COMPUTE);
    REQUIRE(in.get_payload());
    REQUIRE(in.get_payload()->get_fault_mode() == fault_mode::THROW_PROCESSING);
}

/******************************************************************************/

TEST_CASE("submission(asio-loopback): all items conserved over real sockets", "[courtier][submission][net][asio]") {
    const unsigned short port = free_tcp_port();

    auto consumer = std::make_shared<Consumers::GAsioConsumerT<GFaultyContainer>>();
    consumer->setPort(port);
    consumer->setSerializationMode(Gem::Common::serializationMode::BINARY);

    auto client = std::make_shared<Consumers::GAsioConsumerClientT<GFaultyContainer>>(
        "127.0.0.1", port, Gem::Common::serializationMode::BINARY, /*max_reconnects=*/10
    );

    constexpr std::size_t N = 300;
    std::vector<std::shared_ptr<GFaultyContainer>> items;
    items.reserve(N);
    for(std::size_t i = 0; i < N; ++i) {
        items.push_back(make_item(i, fault_mode::NONE));
    }

    auto r = run_loopback(consumer, client, items);

    REQUIRE_FALSE(r.client_threw);
    REQUIRE(r.status.is_complete);
    REQUIRE_FALSE(r.status.has_errors);
    REQUIRE(r.processed == N); // every item made the full server->client->server round-trip
}

/******************************************************************************/

TEST_CASE("submission(asio-loopback): slow client still returns every item", "[courtier][submission][net][asio]") {
    const unsigned short port = free_tcp_port();

    auto consumer = std::make_shared<Consumers::GAsioConsumerT<GFaultyContainer>>();
    consumer->setPort(port);
    consumer->setSerializationMode(Gem::Common::serializationMode::BINARY);

    auto client = std::make_shared<Consumers::GAsioConsumerClientT<GFaultyContainer>>(
        "127.0.0.1", port, Gem::Common::serializationMode::BINARY, 10
    );

    constexpr std::size_t N = 60;
    std::vector<std::shared_ptr<GFaultyContainer>> items;
    items.reserve(N);
    for(std::size_t i = 0; i < N; ++i) {
        const bool slow = (i % 5 == 0);
        items.push_back(make_item(i, slow ? fault_mode::SLEEP : fault_mode::NONE, slow ? 5u : 0u));
    }

    auto r = run_loopback(consumer, client, items);

    REQUIRE_FALSE(r.client_threw);
    REQUIRE(r.status.is_complete);
    REQUIRE(r.processed == N);
}

/******************************************************************************/

TEST_CASE("submission(websocket-loopback): all items conserved over real sockets", "[courtier][submission][net][websocket]") {
    const unsigned short port = free_tcp_port();

    auto consumer = std::make_shared<Consumers::GWebsocketConsumerT<GFaultyContainer>>();
    consumer->setPort(port);
    consumer->setSerializationMode(Gem::Common::serializationMode::BINARY);

    auto client = std::make_shared<Consumers::GWebsocketClientT<GFaultyContainer>>(
        "127.0.0.1", port, Gem::Common::serializationMode::BINARY, /*verbose_control_frames=*/false
    );

    constexpr std::size_t N = 200;
    std::vector<std::shared_ptr<GFaultyContainer>> items;
    items.reserve(N);
    for(std::size_t i = 0; i < N; ++i) {
        items.push_back(make_item(i, fault_mode::NONE));
    }

    auto r = run_loopback(consumer, client, items);

    REQUIRE_FALSE(r.client_threw);
    REQUIRE(r.status.is_complete);
    REQUIRE_FALSE(r.status.has_errors);
    REQUIRE(r.processed == N);
}

/******************************************************************************/

TEST_CASE(
    "submission(asio-loopback): client survives a throwing work item",
    "[courtier][submission][net][asio]"
) {
    // Regression for T1 (P0.2/P1.x). A work item whose process() throws is sent to the client.
    // The client handler now catches the (normalised) g_processing_exception and returns the item
    // flagged instead of letting it unwind the io thread, so: the client does not die, every
    // well-behaved item still returns, and the faulty item is accounted for (not lost). The faulty
    // item is placed last so the well-behaved items return first.
    const unsigned short port = free_tcp_port();

    auto consumer = std::make_shared<Consumers::GAsioConsumerT<GFaultyContainer>>();
    consumer->setPort(port);
    consumer->setSerializationMode(Gem::Common::serializationMode::BINARY);

    auto client = std::make_shared<Consumers::GAsioConsumerClientT<GFaultyContainer>>(
        "127.0.0.1", port, Gem::Common::serializationMode::BINARY, 10
    );

    constexpr std::size_t N_GOOD = 20;
    std::vector<std::shared_ptr<GFaultyContainer>> items;
    for(std::size_t i = 0; i < N_GOOD; ++i) {
        items.push_back(make_item(i, fault_mode::NONE));
    }
    items.push_back(make_item(9999, fault_mode::THROW_PROCESSING)); // the poison item, last

    auto r = run_loopback(consumer, client, items);

    CHECK_FALSE(r.client_threw);          // the client must not die on a bad item
    CHECK(r.processed == N_GOOD);          // every well-behaved item must still come back
    CHECK(r.status.is_complete);           // nothing left stranded (faulty item returns flagged)
}
