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
#include <cstddef>
#include <functional>
#include <memory>
#include <string>

// Boost headers
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>

// Geneva headers
#include "common/GCommonEnums.hpp" // serializationMode
#include "courtier/GBrokerT.hpp"
#include "geneva/ind/GOptimizableEntity.hpp"

namespace Gem::Courtier {
template <typename processable_type>
class GBaseClientT; // the networked client base (wire-compatible with the courtier socket servers)
} /* namespace Gem::Courtier */

namespace Gem::Geneva {

/******************************************************************************/
/**
 * A transport-agnostic description of the courtier consumer to build, assembled from configuration
 * / command-line parameters by the caller (Go2 or a standalone example). Keeps the caller free of the
 * concrete courtier consumer types -- those are known only to buildConsumerSetup().
 */
struct ConsumerSpec {
    std::string mnemonic;            ///< "sc" | "stc" | "asio" | "beast" | "mpi"
    unsigned int n_threads = 0;      ///< local thread-pool / networked IO-thread count (0 == hardware concurrency)
    unsigned short port = 0;         ///< listening / target port (networked socket consumers)
    Gem::Common::serializationMode serialization_mode =
        Gem::Common::serializationMode::BINARY; ///< wire serialization (networked)
    // --- client-side fields (networked socket consumers; ignored by the server build) ---
    std::string ip = "localhost";        ///< server address the client connects to
    std::size_t max_reconnects = 0;      ///< [asio] client reconnect attempts before giving up
    bool verbose_control_frames = false; ///< [beast] client: log ping/pong/close frames
    std::size_t client_prefetch_depth = 1; ///< [asio/beast] client: max work items held concurrently
                                           ///< (1 == serial; >1 overlaps fetch/compute/return)
};

/******************************************************************************/
/**
 * The result of building a courtier setup for the current process.
 */
struct ConsumerSetup {
    /** @brief A ready broker (consumer registered, clone function set, server started for networked
     *  consumers) to inject into the algorithms via GOptimizationAlgorithmBase::setBroker(). Null when this
     *  process is not a submitter -- e.g. an MPI worker rank. */
    std::shared_ptr<Gem::Courtier::GBrokerT<gen::GOptimizableEntity>> broker;
    /** @brief When this process must serve as a worker (an MPI worker rank), the loop to run; null
     *  otherwise. The caller invokes it instead of submitting. */
    std::function<void()> run_worker;
};

/******************************************************************************/
/**
 * Builds a courtier setup from @p spec: constructs the matching courtier consumer, sets the
 * polymorphic GOptimizableEntity clone function (required by clone-on-partial-return), registers it with a
 * fresh single-consumer broker, and -- for networked consumers -- starts the server (for MPI only on
 * the master rank; a worker rank yields a run_worker loop and a null broker instead).
 *
 * This is the SINGLE place that knows the concrete courtier consumer types, so callers (Go2 and the
 * standalone examples) share one construction path and stay free of consumer specifics. An unknown
 * mnemonic yields an empty setup (both fields null).
 */
ConsumerSetup buildConsumerSetup(const ConsumerSpec &spec);

/******************************************************************************/
/**
 * Builds a ConsumerSpec for @p mnemonic from the already-parsed command line @p vm.
 *
 * This is the single place that maps the consumer command-line options (asio_port,
 * beast_serializationMode, nWorkerThreads, ...) onto the transport-agnostic spec, keeping callers
 * (Go2, the standalone examples) free of per-consumer option knowledge. Options absent from @p vm
 * fall back to the spec's defaults; an unknown mnemonic yields a spec carrying only the mnemonic.
 */
ConsumerSpec specFromCommandLine(
    const std::string &mnemonic, const boost::program_options::variables_map &vm);

/******************************************************************************/
/**
 * Builds the networked client for @p spec, for a process running in client mode. The socket servers
 * built by buildConsumerSetup() are wire-compatible with the existing client classes, so this is the
 * single place that maps a mnemonic onto the matching client (asio/beast). The caller sets the maximum
 * runtime and invokes run() on the returned client.
 *
 * Returns null for mnemonics that have no socket client (sc/stc are local; the mpi worker loop is
 * obtained from buildConsumerSetup().run_worker instead).
 */
std::shared_ptr<Gem::Courtier::GBaseClientT<gen::GOptimizableEntity>>
buildConsumerClient(const ConsumerSpec &spec);

/******************************************************************************/
/**
 * Registers the command-line options for every supported courtier consumer (asio/beast/stc, and mpi
 * when built) into @p visible / @p hidden. This is the single place that owns the consumer option
 * surface, so callers (Go2) register them without iterating a consumer store. specFromCommandLine()
 * reads the matching values back out of the parsed map.
 */
void addConsumerOptions(
    boost::program_options::options_description &visible,
    boost::program_options::options_description &hidden);

/******************************************************************************/
/** @brief Whether @p mnemonic names a consumer this layer can build (sc/stc/asio/beast/mpi). */
bool isKnownConsumer(const std::string &mnemonic);

/******************************************************************************/
/** @brief Whether a process selecting @p mnemonic can run as a networked client (asio/beast/mpi). */
bool consumerNeedsClient(const std::string &mnemonic);

/******************************************************************************/
/** @brief A "mnemonic:  human-readable-name" listing of the supported consumers, for help text. */
std::string consumerListing();

/******************************************************************************/
/** @brief The number of supported consumers (for help text). */
std::size_t consumerCount();

/******************************************************************************/

} /* namespace Gem::Geneva */
