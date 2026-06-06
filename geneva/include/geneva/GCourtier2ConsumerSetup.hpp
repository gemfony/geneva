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
#include <functional>
#include <memory>
#include <string>

// Boost headers
#include <boost/program_options/variables_map.hpp>

// Geneva headers
#include "common/GCommonEnums.hpp" // serializationMode
#include "courtier2/GBrokerT.hpp"
#include "geneva/par/GParameterSet.hpp"

namespace Gem::Geneva {

/******************************************************************************/
/**
 * A transport-agnostic description of the courtier2 consumer to build, assembled from configuration
 * / command-line parameters by the caller (Go2 or a standalone example). Keeps the caller free of the
 * concrete courtier2 consumer types -- those are known only to buildCourtier2Setup().
 */
struct Courtier2ConsumerSpec {
    std::string mnemonic;            ///< "sc" | "stc" | "asio" | "beast" | "mpi"
    unsigned int n_threads = 0;      ///< local thread-pool / networked IO-thread count (0 == hardware concurrency)
    unsigned short port = 0;         ///< listening port (networked socket consumers)
    Gem::Common::serializationMode serialization_mode =
        Gem::Common::serializationMode::BINARY; ///< wire serialization (networked)
};

/******************************************************************************/
/**
 * The result of building a courtier2 setup for the current process.
 */
struct Courtier2Setup {
    /** @brief A ready broker (consumer registered, clone function set, server started for networked
     *  consumers) to inject into the algorithms via GBase::setCourtier2Broker(). Null when this
     *  process is not a submitter -- e.g. an MPI worker rank. */
    std::shared_ptr<Gem::Courtier2::GBrokerT<gpar::GParameterSet>> broker;
    /** @brief When this process must serve as a worker (an MPI worker rank), the loop to run; null
     *  otherwise. The caller invokes it instead of submitting. */
    std::function<void()> run_worker;
};

/******************************************************************************/
/**
 * Builds a courtier2 setup from @p spec: constructs the matching courtier2 consumer, sets the
 * polymorphic GParameterSet clone function (required by clone-on-partial-return), registers it with a
 * fresh single-consumer broker, and -- for networked consumers -- starts the server (for MPI only on
 * the master rank; a worker rank yields a run_worker loop and a null broker instead).
 *
 * This is the SINGLE place that knows the concrete courtier2 consumer types, so callers (Go2 and the
 * standalone examples) share one construction path and stay free of consumer specifics. An unknown
 * mnemonic yields an empty setup (both fields null).
 */
Courtier2Setup buildCourtier2Setup(const Courtier2ConsumerSpec &spec);

/******************************************************************************/
/**
 * Builds a Courtier2ConsumerSpec for @p mnemonic from the already-parsed command line @p vm.
 *
 * This is the single place that maps the consumer command-line options (asio_port,
 * beast_serializationMode, nWorkerThreads, ...) onto the transport-agnostic spec, keeping callers
 * (Go2, the standalone examples) free of per-consumer option knowledge. Options absent from @p vm
 * fall back to the spec's defaults; an unknown mnemonic yields a spec carrying only the mnemonic.
 */
Courtier2ConsumerSpec specFromCommandLine(
    const std::string &mnemonic, const boost::program_options::variables_map &vm);

/******************************************************************************/

} /* namespace Gem::Geneva */
