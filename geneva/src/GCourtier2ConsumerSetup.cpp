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

#include "geneva/GCourtier2ConsumerSetup.hpp"

// Standard headers
#include <cstddef>

// The concrete courtier2 consumers -- known ONLY here.
#include "courtier2/consumers/GAsioConsumerT.hpp"
#include "courtier2/consumers/GMPIConsumerT.hpp" // self-guarded by GENEVA_BUILD_WITH_MPI_CONSUMER
#include "courtier2/consumers/GSerialConsumerT.hpp"
#include "courtier2/consumers/GStdThreadConsumerT.hpp"
#include "courtier2/consumers/GWebsocketConsumerT.hpp"

// The networked clients are wire-compatible with the courtier2 socket servers and are reused as-is;
// like the consumers, the concrete client types are known ONLY here.
#include "courtier/consumers/GAsioConsumerT.hpp"     // GAsioConsumerClientT
#include "courtier/consumers/GWebsocketConsumerT.hpp" // GWebsocketClientT

namespace Gem::Geneva {

namespace {

/** @brief The polymorphic clone for GParameterSet (copy-construction would slice the held individual). */
std::function<std::shared_ptr<gpar::GParameterSet>(const std::shared_ptr<gpar::GParameterSet> &)>
parameterSetCloneFunction() {
    return [](const std::shared_ptr<gpar::GParameterSet> &p) { return p->clone<gpar::GParameterSet>(); };
}

/** @brief Wraps a ready consumer (clone function already set) in a fresh single-consumer broker. */
std::shared_ptr<Gem::Courtier2::GBrokerT<gpar::GParameterSet>>
brokerFor(std::shared_ptr<Gem::Courtier2::GBaseConsumerT<gpar::GParameterSet>> consumer) {
    auto broker = std::make_shared<Gem::Courtier2::GBrokerT<gpar::GParameterSet>>();
    broker->registerConsumer(std::move(consumer));
    return broker;
}

} /* anonymous namespace */

/******************************************************************************/

Courtier2Setup buildCourtier2Setup(const Courtier2ConsumerSpec &spec) {
    namespace c2 = Gem::Courtier2;
    Courtier2Setup setup;

    if(spec.mnemonic == "sc") {
        auto consumer = std::make_shared<c2::GSerialConsumerT<gpar::GParameterSet>>();
        consumer->setCloneFunction(parameterSetCloneFunction());
        setup.broker = brokerFor(consumer);
    }
    else if(spec.mnemonic == "stc") {
        auto consumer = std::make_shared<c2::GStdThreadConsumerT<gpar::GParameterSet>>(spec.n_threads);
        consumer->setCloneFunction(parameterSetCloneFunction());
        setup.broker = brokerFor(consumer);
    }
    else if(spec.mnemonic == "asio") {
        auto consumer = std::make_shared<c2::GAsioConsumerT<gpar::GParameterSet>>(
            spec.port, spec.n_threads, spec.serialization_mode);
        consumer->setCloneFunction(parameterSetCloneFunction());
        consumer->startServer();
        setup.broker = brokerFor(consumer);
    }
    else if(spec.mnemonic == "beast") {
        auto consumer = std::make_shared<c2::GWebsocketConsumerT<gpar::GParameterSet>>(
            spec.port, spec.n_threads, spec.serialization_mode);
        consumer->setCloneFunction(parameterSetCloneFunction());
        consumer->startServer();
        setup.broker = brokerFor(consumer);
    }
#ifdef GENEVA_BUILD_WITH_MPI_CONSUMER
    else if(spec.mnemonic == "mpi") {
        // MPI fixes the master/worker split by rank; the consumer is built on every rank and branches.
        auto consumer = std::make_shared<c2::GMPIConsumerT<gpar::GParameterSet>>();
        if(consumer->isMasterNode()) {
            consumer->setCloneFunction(parameterSetCloneFunction());
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

Courtier2ConsumerSpec specFromCommandLine(
    const std::string &mnemonic, const boost::program_options::variables_map &vm) {
    Courtier2ConsumerSpec spec;
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
        spec.n_threads = 0; // networked IO threads: hardware concurrency
    }
    else if(mnemonic == "beast") {
        readPort("beast_port", spec.port);
        readSerMode("beast_serializationMode", spec.serialization_mode);
        readString("beast_ip", spec.ip);
        if(vm.count("beast_verboseControlFrames") != 0u) {
            spec.verbose_control_frames = vm["beast_verboseControlFrames"].as<bool>();
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

std::shared_ptr<Gem::Courtier::GBaseClientT<gpar::GParameterSet>>
buildCourtier2Client(const Courtier2ConsumerSpec &spec) {
    namespace cons = Gem::Courtier::Consumers;

    if(spec.mnemonic == "asio") {
        return std::make_shared<cons::GAsioConsumerClientT<gpar::GParameterSet>>(
            spec.ip, spec.port, spec.serialization_mode, spec.max_reconnects);
    }
    if(spec.mnemonic == "beast") {
        return std::make_shared<cons::GWebsocketClientT<gpar::GParameterSet>>(
            spec.ip, spec.port, spec.serialization_mode, spec.verbose_control_frames);
    }

    // sc/stc are local-only; the mpi worker loop comes from buildCourtier2Setup().run_worker.
    return nullptr;
}

/******************************************************************************/

} /* namespace Gem::Geneva */
