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

#include "geneva/GConsumerSetup.hpp"

// Standard headers
#include <cstddef>
#include <cstdint>
#include <string>

// Default values for the consumer command-line options.
#include "courtier/GCourtierEnums.hpp"

// The concrete courtier consumers -- known ONLY here.
#include "courtier/consumers/GAsioConsumerT.hpp"
#include "courtier/consumers/GMPIConsumerT.hpp" // self-guarded by GENEVA_BUILD_WITH_MPI_CONSUMER
#include "courtier/consumers/GSerialConsumerT.hpp"
#include "courtier/consumers/GStdThreadConsumerT.hpp"
#include "courtier/consumers/GWebsocketConsumerT.hpp"

// The networked clients are wire-compatible with the courtier socket servers and are reused as-is;
// like the consumers, the concrete client types are known ONLY here.
#include "courtier/transport/GAsioTransportT.hpp"     // GAsioConsumerClientT
#include "courtier/transport/GWebsocketTransportT.hpp" // GWebsocketClientT

namespace Gem::Geneva {

namespace {

/** @brief The polymorphic clone for GOptimizableEntity (copy-construction would slice the held individual). */
std::function<std::unique_ptr<gen::GOptimizableEntity>(const std::unique_ptr<gen::GOptimizableEntity> &)>
individualCloneFunction() {
    return [](const std::unique_ptr<gen::GOptimizableEntity> &p) { return p->clone_unique(); };
}

/** @brief Wraps a ready consumer (clone function already set) in a fresh single-consumer broker. */
std::shared_ptr<Gem::Courtier::GBrokerT<gen::GOptimizableEntity>>
brokerFor(std::shared_ptr<Gem::Courtier::GBaseConsumerT<gen::GOptimizableEntity>> consumer) {
    auto broker = std::make_shared<Gem::Courtier::GBrokerT<gen::GOptimizableEntity>>();
    broker->registerConsumer(std::move(consumer));
    return broker;
}

} /* anonymous namespace */

/******************************************************************************/

ConsumerSetup buildConsumerSetup(const ConsumerSpec &spec) {
    namespace c2 = Gem::Courtier;
    ConsumerSetup setup;

    if(spec.mnemonic == "sc") {
        auto consumer = std::make_shared<c2::GSerialConsumerT<gen::GOptimizableEntity>>();
        consumer->setCloneFunction(individualCloneFunction());
        setup.broker = brokerFor(consumer);
    }
    else if(spec.mnemonic == "stc") {
        auto consumer = std::make_shared<c2::GStdThreadConsumerT<gen::GOptimizableEntity>>(spec.n_threads);
        consumer->setCloneFunction(individualCloneFunction());
        setup.broker = brokerFor(consumer);
    }
    else if(spec.mnemonic == "asio") {
        auto consumer = std::make_shared<c2::GAsioConsumerT<gen::GOptimizableEntity>>(
            spec.port, spec.n_threads, spec.serialization_mode);
        consumer->setCloneFunction(individualCloneFunction());
        consumer->startServer();
        setup.broker = brokerFor(consumer);
    }
    else if(spec.mnemonic == "beast") {
        auto consumer = std::make_shared<c2::GWebsocketConsumerT<gen::GOptimizableEntity>>(
            spec.port, spec.n_threads, spec.serialization_mode);
        consumer->setCloneFunction(individualCloneFunction());
        consumer->startServer();
        setup.broker = brokerFor(consumer);
    }
#ifdef GENEVA_BUILD_WITH_MPI_CONSUMER
    else if(spec.mnemonic == "mpi") {
        // MPI fixes the master/worker split by rank; the consumer is built on every rank and branches.
        auto consumer = std::make_shared<c2::GMPIConsumerT<gen::GOptimizableEntity>>();
        if(consumer->isMasterNode()) {
            consumer->setCloneFunction(individualCloneFunction());
            consumer->startServer();
            setup.broker = brokerFor(consumer);
        }
        else {
            // Worker rank: serve through the consumer (kept alive by the capture); no broker to inject.
            setup.run_worker = [consumer]() { consumer->runWorker(); };
        }
    }
#endif /* GENEVA_BUILD_WITH_MPI_CONSUMER */

    return setup;
}

/******************************************************************************/

ConsumerSpec specFromCommandLine(
    const std::string &mnemonic, const boost::program_options::variables_map &vm) {
    ConsumerSpec spec;
    spec.mnemonic = mnemonic;

    // The consumer command-line options are registered by the consumers' addCLOptions() during
    // command-line parsing; here we read the ones the spec needs back out of the parsed map. Each
    // read is guarded so an absent option leaves the spec default in place.
    auto readPort = [&vm](const char *key, unsigned short &dst) {
        if(vm.count(key) != 0u) {
            dst = vm[key].as<unsigned short>();
        }
    };
    auto readSerMode = [&vm](const char *key, Gem::Common::serializationMode &dst) {
        if(vm.count(key) != 0u) {
            dst = vm[key].as<Gem::Common::serializationMode>();
        }
    };
    auto readString = [&vm](const char *key, std::string &dst) {
        if(vm.count(key) != 0u) {
            dst = vm[key].as<std::string>();
        }
    };

    if(mnemonic == "asio") {
        readPort("asio_port", spec.port);
        readSerMode("asio_serializationMode", spec.serialization_mode);
        readString("asio_ip", spec.ip);
        if(vm.count("asio_maxReconnects") != 0u) {
            spec.max_reconnects = vm["asio_maxReconnects"].as<std::size_t>();
        }
        if(vm.count("asio_prefetchDepth") != 0u) {
            spec.client_prefetch_depth = vm["asio_prefetchDepth"].as<std::size_t>();
        }
        spec.n_threads = 0; // networked IO threads: hardware concurrency
    }
    else if(mnemonic == "beast") {
        readPort("beast_port", spec.port);
        readSerMode("beast_serializationMode", spec.serialization_mode);
        readString("beast_ip", spec.ip);
        if(vm.count("beast_verboseControlFrames") != 0u) {
            spec.verbose_control_frames = vm["beast_verboseControlFrames"].as<bool>();
        }
        if(vm.count("beast_prefetchDepth") != 0u) {
            spec.client_prefetch_depth = vm["beast_prefetchDepth"].as<std::size_t>();
        }
        spec.n_threads = 0;
    }
    else if(mnemonic == "stc") {
        if(vm.count("nWorkerThreads") != 0u) {
            spec.n_threads = static_cast<unsigned int>(vm["nWorkerThreads"].as<std::size_t>());
        }
    }
    // "sc" and "mpi" carry no networked spec fields: the defaults suffice.

    return spec;
}

/******************************************************************************/

std::shared_ptr<Gem::Courtier::GBaseClientT<gen::GOptimizableEntity>>
buildConsumerClient(const ConsumerSpec &spec) {
    namespace cons = Gem::Courtier::Consumers;

    if(spec.mnemonic == "asio") {
        return std::make_shared<cons::GAsioConsumerClientT<gen::GOptimizableEntity>>(
            spec.ip, spec.port, spec.serialization_mode, spec.max_reconnects,
            spec.client_prefetch_depth);
    }
    if(spec.mnemonic == "beast") {
        return std::make_shared<cons::GWebsocketClientT<gen::GOptimizableEntity>>(
            spec.ip, spec.port, spec.serialization_mode, spec.verbose_control_frames,
            spec.client_prefetch_depth);
    }

    // sc/stc are local-only; the mpi worker loop comes from buildConsumerSetup().run_worker.
    return nullptr;
}

/******************************************************************************/

namespace {

/** @brief One supported consumer: its mnemonic, human-readable name and whether it can have a client. */
struct C2ConsumerInfo {
    const char *mnemonic;
    const char *name;
    bool needs_client;
};

constexpr C2ConsumerInfo kC2Consumers[] = {
    {"sc", "GSerialConsumerT", false},
    {"stc", "GStdThreadConsumerT", false},
    {"asio", "GAsioConsumerT", true},
    {"beast", "GWebsocketConsumerT", true},
#ifdef GENEVA_BUILD_WITH_MPI_CONSUMER
    {"mpi", "GMPIConsumerT", true},
#endif /* GENEVA_BUILD_WITH_MPI_CONSUMER */
};

const C2ConsumerInfo *findC2Consumer(const std::string &mnemonic) {
    for(const auto &info : kC2Consumers) {
        if(mnemonic == info.mnemonic) {
            return &info;
        }
    }
    return nullptr;
}

} /* anonymous namespace */

/******************************************************************************/

void addConsumerOptions(
    boost::program_options::options_description &visible,
    boost::program_options::options_description &hidden) {
    namespace po = boost::program_options;
    using Gem::Common::serializationMode;

    // [asio]
    visible.add_options()(
        "asio_ip", po::value<std::string>()->default_value(Gem::Courtier::GCONSUMERDEFAULTSERVER),
        "\t[asio] The name or ip of the server")(
        "asio_port", po::value<unsigned short>()->default_value(Gem::Courtier::GCONSUMERDEFAULTPORT),
        "\t[asio] The port of the server");
    hidden.add_options()(
        "asio_serializationMode",
        po::value<serializationMode>()->default_value(Gem::Courtier::GCONSUMERSERIALIZATIONMODE),
        "\t[asio] Serialization in TEXTMODE (0), XMLMODE (1) or BINARYMODE (2)")(
        "asio_nProcessingThreads",
        po::value<std::size_t>()->default_value(Gem::Courtier::GCONSUMERLISTENERTHREADS),
        "\t[asio] The number of threads used to process incoming connections")(
        "asio_maxReconnects",
        po::value<std::size_t>()->default_value(Gem::Courtier::GASIOCONSUMERMAXCONNECTIONATTEMPTS),
        "\t[asio] The maximum number of client reconnection attempts")(
        "asio_prefetchDepth",
        po::value<std::size_t>()->default_value(1),
        "\t[asio] Max work items a client holds at once (1 == serial; >1 overlaps fetch/compute/return)");

    // [beast]
    visible.add_options()(
        "beast_ip", po::value<std::string>()->default_value(Gem::Courtier::GCONSUMERDEFAULTSERVER),
        "\t[beast] The name or ip of the server")(
        "beast_port", po::value<unsigned short>()->default_value(Gem::Courtier::GCONSUMERDEFAULTPORT),
        "\t[beast] The port of the server");
    hidden.add_options()(
        "beast_serializationMode",
        po::value<serializationMode>()->default_value(Gem::Courtier::GCONSUMERSERIALIZATIONMODE),
        "\t[beast] Serialization in TEXTMODE (0), XMLMODE (1) or BINARYMODE (2)")(
        "beast_nListenerThreads",
        po::value<std::size_t>()->default_value(Gem::Courtier::GCONSUMERLISTENERTHREADS),
        "\t[beast] The number of threads used to listen for incoming connections")(
        "beast_pingInterval",
        po::value<std::size_t>()->default_value(Gem::Courtier::GBEASTCONSUMERPINGINTERVAL),
        "\t[beast] The number of seconds between two consecutive pings")(
        "beast_verboseControlFrames",
        po::value<bool>()->default_value(false)->implicit_value(true),
        "\t[beast] Announce ping/pong/close frames")(
        "beast_prefetchDepth",
        po::value<std::size_t>()->default_value(1),
        "\t[beast] Max work items a client holds at once (1 == serial; >1 overlaps fetch/compute/return)");

    // [stc] -- 0 means "the consumer's own default" (hardware concurrency).
    hidden.add_options()(
        "nWorkerThreads", po::value<std::size_t>()->default_value(0),
        "\t[stc] The number of worker threads (0 == hardware concurrency)")(
        "stcCapableOfFullReturn", po::value<bool>()->default_value(true),
        "\t[stc] A debugging option toggling timeouts in the executor");

#ifdef GENEVA_BUILD_WITH_MPI_CONSUMER
    // [mpi] -- accepted for command-line compatibility; the courtier mpi path currently uses its own
    // defaults for these (per-option passthrough is a planned refinement).
    visible.add_options()(
        "mpi_asyncReq", po::value<bool>()->default_value(true),
        "\t[mpi] Whether clients prefetch the next work item")(
        "mpi_nHandlerThreads", po::value<std::uint32_t>()->default_value(0),
        "\t[mpi] The number of request-handler threads (0 == hardware concurrency)");
    hidden.add_options()(
        "mpi_cleanSessInterval", po::value<std::uint32_t>()->default_value(100),
        "\t[mpi] Interval in ms between master session-completion checks")(
        "mpi_serializationMode",
        po::value<serializationMode>()->default_value(Gem::Courtier::GCONSUMERSERIALIZATIONMODE),
        "\t[mpi] Serialization in TEXTMODE (0), XMLMODE (1) or BINARYMODE (2)");
#endif /* GENEVA_BUILD_WITH_MPI_CONSUMER */
}

/******************************************************************************/

bool isKnownConsumer(const std::string &mnemonic) {
    return findC2Consumer(mnemonic) != nullptr;
}

/******************************************************************************/

bool consumerNeedsClient(const std::string &mnemonic) {
    const C2ConsumerInfo *info = findC2Consumer(mnemonic);
    return info != nullptr && info->needs_client;
}

/******************************************************************************/

std::string consumerListing() {
    std::string result;
    for(const auto &info : kC2Consumers) {
        result += std::string(info.mnemonic) + ":  " + info.name + "\n";
    }
    return result;
}

/******************************************************************************/

std::size_t consumerCount() {
    return std::size(kC2Consumers);
}

/******************************************************************************/

} /* namespace Gem::Geneva */
