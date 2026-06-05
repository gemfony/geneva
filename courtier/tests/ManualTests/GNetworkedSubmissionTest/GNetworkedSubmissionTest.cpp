/**
 * @file GNetworkedSubmissionTest.cpp
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

// A standalone, MULTI-PROCESS exerciser for the courtier submission path that does NOT depend on
// the geneva optimization library. It is the process-level counterpart to the in-process
// GCourtierNetworkedStressTests: one invocation is the server (runs a broker + a networked
// consumer and submits a batch of work items via GBrokerExecutorT), other invocations are clients
// connecting over TCP. It accepts the same command-line flags a Go2 networked program does, so it
// can be driven directly by scripts/startLocalJobs.sh and scripts/startClients.sh:
//
//   server : GNetworkedSubmissionTest -c asio  --asio_port=PORT
//   client : GNetworkedSubmissionTest -c asio  --client --asio_ip=HOST --asio_port=PORT
//   (use "-c beast" + --beast_ip/--beast_port for the websocket consumer)
//
// The payload is the lightweight, fault-injecting GFaultyContainer, so the same binary can also
// exercise the error paths (--faultEvery N injects a throwing work item every N-th item).
//
// The server prints a one-line PASS/FAIL summary and returns 0 only if every submitted item came
// back. This makes the program usable as an automated multi-process integration check (e.g. via
// GENEVA_RUN_TIMEOUT in startLocalJobs.sh).

// Standard headers
#include <cstddef>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

// Boost
#include <boost/program_options.hpp>

// Geneva / courtier
#include "common/GCommonEnums.hpp"
#include "courtier/GBaseClientT.hpp"
#include "courtier/GBrokerT.hpp"
#include "courtier/GDemoProcessingContainers.hpp"
#include "courtier/GExecutorT.hpp"
#include "courtier/consumers/GAsioConsumerT.hpp"
#include "courtier/consumers/GWebsocketConsumerT.hpp"

namespace po = boost::program_options;
using namespace Gem::Courtier;

namespace {

using WL = GFaultyContainer; // the work-item type travelling over the wire

/** @brief Runs the server: broker + consumer + a batch submitted through GBrokerExecutorT. */
int run_server(
    const std::string &consumer,
    unsigned short port,
    std::size_t n_items,
    std::size_t fault_every
) {
    resetBroker<WL>();

    std::shared_ptr<Consumers::GBaseConsumerT<WL>> consumer_ptr;
    if(consumer == "asio") {
        auto c = std::make_shared<Consumers::GAsioConsumerT<WL>>();
        c->setPort(port);
        c->setSerializationMode(Gem::Common::serializationMode::BINARY);
        consumer_ptr = c;
    }
    else {
        auto c = std::make_shared<Consumers::GWebsocketConsumerT<WL>>();
        c->setPort(port);
        c->setSerializationMode(Gem::Common::serializationMode::BINARY);
        consumer_ptr = c;
    }

    broker<WL>()->init();
    broker<WL>()->enrol_consumer(consumer_ptr);

    std::cout << "[server] " << consumer << " consumer listening on port " << port
              << ", submitting " << n_items << " work item(s)"
              << (fault_every > 0 ? " (a throwing item every " + std::to_string(fault_every) + ")" : "")
              << ". Waiting for clients ..." << std::endl;

    GBrokerExecutorT<WL> exec;
    exec.init();

    std::vector<std::shared_ptr<WL>> items;
    items.reserve(n_items);
    for(std::size_t i = 0; i < n_items; ++i) {
        const bool faulty = (fault_every > 0 && (i % fault_every == 0));
        auto item =
            std::make_shared<WL>(i, faulty ? fault_mode::THROW_PROCESSING : fault_mode::NONE);
        item->set_processing_status(processingStatus::DO_PROCESS);
        items.push_back(std::move(item));
    }

    auto status = exec.workOn(items);
    exec.finalize();

    std::size_t returned = 0;
    for(auto const &it : items) {
        if(it && it->getProcessingStatus() != processingStatus::DO_PROCESS) {
            ++returned; // processed OR flagged -- i.e. it made the full round-trip
        }
    }

    broker<WL>()->finalize();
    resetBroker<WL>();

    const bool ok = status.is_complete && (returned == n_items);
    std::cout << "[server] " << (ok ? "PASS" : "FAIL") << ": " << returned << "/" << n_items
              << " item(s) returned, complete=" << status.is_complete
              << ", errors=" << status.has_errors << std::endl;
    return ok ? 0 : 1;
}

/** @brief Runs a client connecting to the server over TCP. Blocks until done. */
int run_client(const std::string &consumer, const std::string &ip, unsigned short port) {
    std::shared_ptr<GBaseClientT<WL>> client;
    if(consumer == "asio") {
        client = std::make_shared<Consumers::GAsioConsumerClientT<WL>>(
            ip, port, Gem::Common::serializationMode::BINARY, /*max_reconnects=*/10
        );
    }
    else {
        client = std::make_shared<Consumers::GWebsocketClientT<WL>>(
            ip, port, Gem::Common::serializationMode::BINARY, /*verbose_control_frames=*/false
        );
    }

    std::cout << "[client] connecting to " << ip << ":" << port << " (" << consumer << ") ..."
              << std::endl;
    client->run(); // returns when the server is gone or a halt condition is reached
    std::cout << "[client] done." << std::endl;
    return 0;
}

} // namespace

int main(int argc, char **argv) {
    std::string consumer;
    bool is_client = false;
    std::string asio_ip;
    std::string beast_ip;
    unsigned short asio_port = 0;
    unsigned short beast_port = 0;
    std::size_t n_items = 0;
    std::size_t fault_every = 0;

    po::options_description desc("GNetworkedSubmissionTest options");
    // clang-format off
    desc.add_options()
        ("help,h", "Show this help message")
        ("consumer,c", po::value<std::string>(&consumer)->default_value("asio"),
            "Consumer to use: asio | beast")
        ("client", po::bool_switch(&is_client), "Run as a client (default: server)")
        ("asio_ip", po::value<std::string>(&asio_ip)->default_value("localhost"),
            "[asio] server ip/hostname (client)")
        ("asio_port", po::value<unsigned short>(&asio_port)->default_value(10000),
            "[asio] server port")
        ("beast_ip", po::value<std::string>(&beast_ip)->default_value("localhost"),
            "[beast] server ip/hostname (client)")
        ("beast_port", po::value<unsigned short>(&beast_port)->default_value(10000),
            "[beast] server port")
        ("nWorkItems", po::value<std::size_t>(&n_items)->default_value(200),
            "[server] number of work items to submit")
        ("faultEvery", po::value<std::size_t>(&fault_every)->default_value(0),
            "[server] inject a throwing work item every N items (0 = never)");
    // clang-format on

    po::variables_map vm;
    try {
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);
    }
    catch(const std::exception &e) {
        std::cerr << "Error parsing the command line: " << e.what() << '\n' << desc << std::endl;
        return 2;
    }

    if(vm.count("help") != 0u) {
        std::cout << desc << std::endl;
        return 0;
    }
    if(consumer != "asio" && consumer != "beast") {
        std::cerr << "Unknown consumer '" << consumer << "'. Use 'asio' or 'beast'." << std::endl;
        return 2;
    }

    const bool asio = (consumer == "asio");
    const std::string ip = asio ? asio_ip : beast_ip;
    const unsigned short port = asio ? asio_port : beast_port;

    if(is_client) {
        return run_client(consumer, ip, port);
    }
    return run_server(consumer, port, n_items, fault_every);
}
