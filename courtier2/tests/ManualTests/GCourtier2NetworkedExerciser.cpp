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
 * Standalone, multi-PROCESS exerciser for the courtier2 socket consumers. Unlike the in-process
 * loopback unit tests, this runs the server and the clients as separate OS processes (typically
 * launched together by startCourtier2Jobs.sh), so it validates the real cross-process behaviour.
 *
 *   server : GCourtier2NetworkedExerciser [--server] -c asio|beast --port P [--n N] [--faultEvery K]
 *   client : GCourtier2NetworkedExerciser  --client  -c asio|beast --ip HOST --port P
 *
 * The server submits a batch through the courtier2 span+policy executor and prints OK/FAIL plus the
 * processed count; with --faultEvery K, every K-th item throws and the clone-on-partial-return
 * policy is used (so every slot must still end up processed).
 */

#include <chrono>
#include <cstddef>
#include <initializer_list>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "courtier/GDemoProcessingContainers.hpp"
#include "courtier/consumers/GAsioConsumerT.hpp"
#include "courtier/consumers/GWebsocketConsumerT.hpp"
#include "courtier2/GBrokerT.hpp"
#include "courtier2/GExecutorT.hpp"
#include "courtier2/GSubmissionPolicy.hpp"
#include "courtier2/consumers/GAsioConsumerT.hpp"
#include "courtier2/consumers/GWebsocketConsumerT.hpp"

using Gem::Courtier::fault_mode;
using Gem::Courtier::GFaultyContainer;
namespace c2 = Gem::Courtier2;
namespace ccons = Gem::Courtier::Consumers;

namespace {

constexpr auto BIN = Gem::Common::serializationMode::BINARY;

std::string arg_value(int argc, char **argv, const std::string &key, const std::string &dflt) {
    for(int i = 1; i < argc; ++i) {
        const std::string a = argv[i];
        if(a == key && i + 1 < argc) {
            return argv[i + 1];
        }
        const std::string eq = key + "=";
        if(a.rfind(eq, 0) == 0) {
            return a.substr(eq.size());
        }
    }
    return dflt;
}

bool has_flag(int argc, char **argv, const std::string &key) {
    for(int i = 1; i < argc; ++i) {
        if(std::string(argv[i]) == key) {
            return true;
        }
    }
    return false;
}

/** @brief First non-empty value among several keys (so the same binary accepts both the generic
 *  --port/--ip and the Geneva-convention --asio_port/--asio_ip / --beast_port/--beast_ip that
 *  startLocalJobs.sh passes). */
std::string first_arg(int argc, char **argv, std::initializer_list<const char *> keys,
                      const std::string &dflt) {
    for(const char *k : keys) {
        const std::string v = arg_value(argc, argv, k, "");
        if(not v.empty()) {
            return v;
        }
    }
    return dflt;
}

int run_server(const std::string &consumer, unsigned short port, std::size_t n, std::size_t fault_every) {
    auto broker = std::make_shared<c2::GBrokerT<GFaultyContainer>>();

    // Build the chosen courtier2 consumer behind the common base type.
    std::shared_ptr<c2::GBaseConsumerT<GFaultyContainer>> base;
    std::shared_ptr<c2::GAsioConsumerT<GFaultyContainer>> asio;
    std::shared_ptr<c2::GWebsocketConsumerT<GFaultyContainer>> beast;
    if(consumer == "beast") {
        beast = std::make_shared<c2::GWebsocketConsumerT<GFaultyContainer>>(port, 0, BIN);
        base = beast;
    }
    else {
        asio = std::make_shared<c2::GAsioConsumerT<GFaultyContainer>>(port, 0, BIN);
        base = asio;
    }
    broker->registerConsumer(base);

    if(asio) {
        asio->startServer();
        port = asio->getPort();
    }
    else {
        beast->startServer();
        port = beast->getPort();
    }
    std::cout << "[server] courtier2 " << consumer << " consumer listening on port " << port
              << "; submitting " << n << " items (faultEvery=" << fault_every << ")\n"
              << std::flush;

    std::vector<std::shared_ptr<GFaultyContainer>> items;
    items.reserve(n);
    for(std::size_t i = 0; i < n; ++i) {
        const bool faulty = fault_every > 0 && (i % fault_every == 0);
        items.push_back(std::make_shared<GFaultyContainer>(
            i, faulty ? fault_mode::THROW_PROCESSING : fault_mode::NONE
        ));
    }

    const auto policy = fault_every > 0
        ? c2::GSubmissionPolicy::clone_on_partial_return()
        : c2::GSubmissionPolicy::full_success_or_fatal();

    c2::GExecutorT<GFaultyContainer> executor(broker);
    executor.workOn(items, policy);

    if(asio) {
        asio->stopServer();
    }
    else {
        beast->stopServer();
    }

    std::size_t processed = 0;
    for(const auto &it : items) {
        if(it && it->is_processed()) {
            ++processed;
        }
    }

    if(processed == n) {
        std::cout << "[server] OK: " << processed << "/" << n << " items processed\n" << std::flush;
        return 0;
    }
    std::cout << "[server] FAIL: only " << processed << "/" << n << " items processed\n" << std::flush;
    return 1;
}

int run_client(const std::string &consumer, const std::string &ip, unsigned short port) {
    std::cout << "[client] connecting to " << ip << ":" << port << " (" << consumer << ")\n"
              << std::flush;
    if(consumer == "beast") {
        auto client =
            std::make_shared<ccons::GWebsocketClientT<GFaultyContainer>>(ip, port, BIN, false);
        client->run();
    }
    else {
        auto client =
            std::make_shared<ccons::GAsioConsumerClientT<GFaultyContainer>>(ip, port, BIN, 50);
        client->run();
    }
    return 0;
}

} /* anonymous namespace */

int main(int argc, char **argv) {
    const std::string consumer = arg_value(argc, argv, "-c", arg_value(argc, argv, "--consumer", "asio"));
    // Accept both the generic --port/--ip and the Geneva-convention names that
    // startLocalJobs.sh passes (--asio_port/--asio_ip, --beast_port/--beast_ip).
    const auto port = static_cast<unsigned short>(
        std::stoi(first_arg(argc, argv, {"--port", "--asio_port", "--beast_port"}, "10000"))
    );
    const bool is_client = has_flag(argc, argv, "--client");

    if(is_client) {
        const std::string ip = first_arg(argc, argv, {"--ip", "--asio_ip", "--beast_ip"}, "127.0.0.1");
        return run_client(consumer, ip, port);
    }

    const auto n = static_cast<std::size_t>(std::stoul(arg_value(argc, argv, "--n", "200")));
    const auto fault_every =
        static_cast<std::size_t>(std::stoul(arg_value(argc, argv, "--faultEvery", "0")));
    return run_server(consumer, port, n, fault_every);
}
