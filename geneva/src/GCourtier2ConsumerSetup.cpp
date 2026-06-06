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

// The concrete courtier2 consumers -- known ONLY here.
#include "courtier2/consumers/GAsioConsumerT.hpp"
#include "courtier2/consumers/GMPIConsumerT.hpp" // self-guarded by GENEVA_BUILD_WITH_MPI_CONSUMER
#include "courtier2/consumers/GSerialConsumerT.hpp"
#include "courtier2/consumers/GStdThreadConsumerT.hpp"
#include "courtier2/consumers/GWebsocketConsumerT.hpp"

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

} /* namespace Gem::Geneva */
