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
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <string>

// Default values for the consumer command-line options.
#include "courtier/GCourtierEnums.hpp"

#include "courtier/GConsumerRegistry.hpp"

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

/**
 * @brief Builds the polymorphic clone functor for GOptimizableEntity.
 *
 * Copy-construction would slice the held individual, so a virtual clone_unique() is used instead.
 *
 * @return A functor that deep-copies a GOptimizableEntity via its virtual clone_unique()
 */
std::function<std::unique_ptr<gen::GOptimizableEntity>(const std::unique_ptr<gen::GOptimizableEntity> &)>
individualCloneFunction() {
    return [](const std::unique_ptr<gen::GOptimizableEntity> &p) { return p->clone_unique(); };
}

} /* anonymous namespace */

/******************************************************************************/
/**
 * @brief Builds a consumer (and, for MPI workers, a worker loop) from a consumer specification.
 *
 * Dispatches on spec.mnemonic to instantiate the matching consumer, sets its clone function, starts
 * any required server, and registers it as the process's single consumer. For the MPI mnemonic the master rank
 * yields a consumer while a worker rank yields a run_worker callable instead (and no consumer).
 *
 * @param spec The consumer specification (mnemonic plus port/threads/serialization settings)
 * @return A ConsumerSetup holding the consumer and/or worker loop; empty for an unknown mnemonic
 */
ConsumerSetup buildConsumerSetup(const ConsumerSpec &spec) {
    namespace c2 = Gem::Courtier;
    ConsumerSetup setup;

    auto &registry = c2::GConsumerRegistryT<gen::GOptimizableEntity>::instance();

    // Idempotent networked build: a socket server binds a port, so if the process already has a consumer,
    // reuse it rather than binding again -- one listening endpoint per process. (MPI is excluded: its
    // consumer is built on every rank and self-determines master/worker, so it is constructed normally
    // and registered below; a sub-algorithm reuses it via the registry, not by re-entering this builder.)
    if(spec.mnemonic == "asio" || spec.mnemonic == "beast") {
        if(auto existing = registry.consumer()) {
            setup.consumer = existing;
            return setup;
        }
    }

    if(spec.mnemonic == "sc") {
        auto consumer = std::make_shared<c2::GSerialConsumerT<gen::GOptimizableEntity>>();
        consumer->setCloneFunction(individualCloneFunction());
        setup.consumer = consumer;
    }
    else if(spec.mnemonic == "stc") {
        auto consumer = std::make_shared<c2::GStdThreadConsumerT<gen::GOptimizableEntity>>(spec.n_threads);
        consumer->setCloneFunction(individualCloneFunction());
        setup.consumer = consumer;
    }
    else if(spec.mnemonic == "asio") {
        auto consumer = std::make_shared<c2::GAsioConsumerT<gen::GOptimizableEntity>>(
            spec.port, spec.n_threads, spec.serialization_mode);
        consumer->setCloneFunction(individualCloneFunction());
        consumer->startServer();
        setup.consumer = consumer;
    }
    else if(spec.mnemonic == "beast") {
        auto consumer = std::make_shared<c2::GWebsocketConsumerT<gen::GOptimizableEntity>>(
            spec.port, spec.n_threads, spec.serialization_mode);
        consumer->setCloneFunction(individualCloneFunction());
        consumer->startServer();
        setup.consumer = consumer;
    }
#ifdef GENEVA_BUILD_WITH_MPI_CONSUMER
    else if(spec.mnemonic == "mpi") {
        // MPI fixes the master/worker split by rank; the consumer is built on every rank and branches.
        // Forward the [mpi] command-line options into the consumer's config.
        Gem::Courtier::Consumers::MPIConsumerConfig mpi_config;
        mpi_config.useAsyncReq = spec.mpi_async_req;
        // 0 == hardware concurrency: leave the default-constructed config's recommendation in place.
        if(spec.mpi_n_handler_threads != 0) {
            mpi_config.nHandlerThreads = spec.mpi_n_handler_threads;
        }
        mpi_config.serializationMode = spec.serialization_mode;
        mpi_config.masterCleanSessIntervalMSec = spec.mpi_clean_sess_interval;
        auto consumer = std::make_shared<c2::GMPIConsumerT<gen::GOptimizableEntity>>(
            nullptr, nullptr, mpi_config);
        if(consumer->isMasterNode()) {
            consumer->setCloneFunction(individualCloneFunction());
            consumer->startServer();
            setup.consumer = consumer;
        }
        else {
            // Worker rank: serve through the consumer (kept alive by the capture); no consumer to register.
            setup.run_worker = [consumer]() { consumer->runWorker(); };
        }
    }
#endif /* GENEVA_BUILD_WITH_MPI_CONSUMER */

    // Register the freshly-built consumer as the process's single consumer so every algorithm submits
    // through it. An MPI worker rank has no consumer (run_worker only), so nothing is registered there.
    if(setup.consumer) {
        registry.setConsumer(setup.consumer);
    }

    return setup;
}

/******************************************************************************/
/**
 * @brief Reads a consumer specification out of the parsed command-line options.
 *
 * Each option read is guarded so an absent option leaves the corresponding spec default in place.
 * The "sc" and "mpi" mnemonics carry no networked spec fields and rely entirely on the defaults.
 *
 * @param mnemonic The selected consumer mnemonic (e.g. "asio", "beast", "stc")
 * @param vm The parsed program-options map produced during command-line parsing
 * @return A ConsumerSpec populated from vm for the given mnemonic
 */
ConsumerSpec specFromCommandLine(
    const std::string &mnemonic, const boost::program_options::variables_map &vm) {
    ConsumerSpec spec;
    spec.mnemonic = mnemonic;

    // The consumer command-line options are registered by the consumers' addCLOptions() during
    // command-line parsing; here we read the ones the spec needs back out of the parsed map. Each
    // read is guarded so an absent option leaves the spec default in place.
    auto readPort = [&vm](const char *key, unsigned short &dst) {
        if(vm.contains(key)) {
            dst = vm[key].as<unsigned short>();
        }
    };
    auto readSerMode = [&vm](const char *key, Gem::Common::serializationMode &dst) {
        if(vm.contains(key)) {
            dst = vm[key].as<Gem::Common::serializationMode>();
        }
    };
    auto readString = [&vm](const char *key, std::string &dst) {
        if(vm.contains(key)) {
            dst = vm[key].as<std::string>();
        }
    };

    if(mnemonic == "asio") {
        readPort("asio_port", spec.port);
        readSerMode("asio_serializationMode", spec.serialization_mode);
        readString("asio_ip", spec.ip);
        if(vm.contains("asio_maxReconnects")) {
            spec.max_reconnects = vm["asio_maxReconnects"].as<std::size_t>();
        }
        if(vm.contains("asio_prefetchDepth")) {
            spec.client_prefetch_depth = vm["asio_prefetchDepth"].as<std::size_t>();
        }
        spec.n_threads = 0; // networked IO threads: hardware concurrency
    }
    else if(mnemonic == "beast") {
        readPort("beast_port", spec.port);
        readSerMode("beast_serializationMode", spec.serialization_mode);
        readString("beast_ip", spec.ip);
        if(vm.contains("beast_verboseControlFrames")) {
            spec.verbose_control_frames = vm["beast_verboseControlFrames"].as<bool>();
        }
        if(vm.contains("beast_prefetchDepth")) {
            spec.client_prefetch_depth = vm["beast_prefetchDepth"].as<std::size_t>();
        }
        spec.n_threads = 0;
    }
    else if(mnemonic == "stc") {
        if(vm.contains("nWorkerThreads")) {
            spec.n_threads = static_cast<unsigned int>(vm["nWorkerThreads"].as<std::size_t>());
        }
    }
#ifdef GENEVA_BUILD_WITH_MPI_CONSUMER
    else if(mnemonic == "mpi") {
        if(vm.contains("mpi_asyncReq")) {
            spec.mpi_async_req = vm["mpi_asyncReq"].as<bool>();
        }
        if(vm.contains("mpi_nHandlerThreads")) {
            spec.mpi_n_handler_threads = vm["mpi_nHandlerThreads"].as<std::uint32_t>();
        }
        if(vm.contains("mpi_cleanSessInterval")) {
            spec.mpi_clean_sess_interval = vm["mpi_cleanSessInterval"].as<std::uint32_t>();
        }
        readSerMode("mpi_serializationMode", spec.serialization_mode);
    }
#endif /* GENEVA_BUILD_WITH_MPI_CONSUMER */
    // "sc" carries no networked spec fields: the defaults suffice.

    return spec;
}

/******************************************************************************/

/**
 * @brief Builds the networked client matching a consumer specification.
 *
 * The asio and beast mnemonics produce a transport client wired to the spec's connection settings.
 * Local-only consumers (sc/stc) need no client; the MPI worker loop is provided by
 * buildConsumerSetup().run_worker instead.
 *
 * @param spec The consumer specification (mnemonic plus connection/serialization settings)
 * @return A client for asio/beast; nullptr for local-only or non-client consumers
 */
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

/** @brief One supported consumer: its mnemonic, human-readable name and whether it needs a client. */
struct C2ConsumerInfo {
    const char *mnemonic;
    const char *name;
    bool needs_client;
};

constexpr C2ConsumerInfo kC2Consumers[] = {
    {.mnemonic="sc", .name="GSerialConsumerT", .needs_client=false},
    {.mnemonic="stc", .name="GStdThreadConsumerT", .needs_client=false},
    {.mnemonic="asio", .name="GAsioConsumerT", .needs_client=true},
    {.mnemonic="beast", .name="GWebsocketConsumerT", .needs_client=true},
#ifdef GENEVA_BUILD_WITH_MPI_CONSUMER
    {.mnemonic="mpi", .name="GMPIConsumerT", .needs_client=true},
#endif /* GENEVA_BUILD_WITH_MPI_CONSUMER */
#ifdef GENEVA_BUILD_WITH_GPU_CONSUMER
    // The GPU consumer is built by Go2 from a problem-registered builder closure (it needs a device
    // marshaller Go2 cannot supply), NOT by buildConsumerSetup(). It is listed here only so the mnemonic
    // is recognised/validated/printed; selecting it without a registered builder is a clear Go2 error.
    // Gated, like "mpi", on GENEVA_BUILD_WITH_GPU_CONSUMER: when the GPU consumer is not built into
    // courtier, "gpu" is simply not a selectable mnemonic.
    {.mnemonic="gpu", .name="GGPUConsumerT", .needs_client=false},
#endif /* GENEVA_BUILD_WITH_GPU_CONSUMER */
};

/**
 * @brief Looks up the consumer-info table entry for a mnemonic.
 *
 * @param mnemonic The consumer mnemonic to search for
 * @return Pointer to the matching table entry, or nullptr if the mnemonic is unknown
 */
const C2ConsumerInfo *findC2Consumer(const std::string &mnemonic) {
    auto it = std::ranges::find_if(kC2Consumers, [&](const auto &info) { return mnemonic == info.mnemonic; });
    return it != std::ranges::end(kC2Consumers) ? &*it : nullptr;
}

} /* anonymous namespace */

/******************************************************************************/

/**
 * @brief Registers all consumer-related command-line options.
 *
 * Adds the per-consumer options (asio, beast, stc, and, when built, mpi) to the supplied
 * program-options descriptions, splitting user-facing options from rarely-used/debug ones.
 *
 * @param visible The options description for user-facing options (shown in --help)
 * @param hidden The options description for hidden/advanced options
 */
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
    // [mpi] -- forwarded into the MPI consumer's config by specFromCommandLine()/buildConsumerSetup().
    // The defaults mirror MPIConsumerConfig so an unset option leaves the consumer's own default in place.
    visible.add_options()(
        "mpi_asyncReq", po::value<bool>()->default_value(true),
        "\t[mpi] Whether clients prefetch the next work item")(
        "mpi_nHandlerThreads", po::value<std::uint32_t>()->default_value(0),
        "\t[mpi] The number of request-handler threads (0 == hardware concurrency)");
    hidden.add_options()(
        "mpi_cleanSessInterval", po::value<std::uint32_t>()->default_value(1000),
        "\t[mpi] Interval in ms between master session-completion checks")(
        "mpi_serializationMode",
        po::value<serializationMode>()->default_value(Gem::Courtier::GCONSUMERSERIALIZATIONMODE),
        "\t[mpi] Serialization in TEXTMODE (0), XMLMODE (1) or BINARYMODE (2)");
#endif /* GENEVA_BUILD_WITH_MPI_CONSUMER */
}

/******************************************************************************/

/**
 * @brief Reports whether a mnemonic names a supported consumer.
 *
 * @param mnemonic The consumer mnemonic to test
 * @return true if the mnemonic is a known/supported consumer, false otherwise
 */
bool isKnownConsumer(const std::string &mnemonic) {
    return findC2Consumer(mnemonic) != nullptr;
}

/******************************************************************************/

/**
 * @brief Reports whether a consumer requires a separate networked client process.
 *
 * @param mnemonic The consumer mnemonic to test
 * @return true if the consumer can have a client (networked consumers), false otherwise (incl. unknown mnemonics)
 */
bool consumerNeedsClient(const std::string &mnemonic) {
    const C2ConsumerInfo *info = findC2Consumer(mnemonic);
    return info != nullptr && info->needs_client;
}

/******************************************************************************/

/**
 * @brief Produces a human-readable listing of all supported consumers.
 *
 * @return A newline-separated string mapping each mnemonic to its class name
 */
std::string consumerListing() {
    std::string result;
    for(const auto &info : kC2Consumers) {
        result += std::string(info.mnemonic) + ":  " + info.name + "\n";
    }
    return result;
}

/******************************************************************************/

/**
 * @brief Returns the number of supported consumers.
 *
 * @return The count of entries in the supported-consumer table (build-dependent, e.g. +1 with MPI)
 */
std::size_t consumerCount() {
    return std::size(kC2Consumers);
}

/******************************************************************************/

} /* namespace Gem::Geneva */
