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

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard headers go here
#include <array>
#include <cstdint>
#include <istream>
#include <ostream>
#include <string>

// Boost headers go here

// Geneva headers go here
#include "common/GCommonEnums.hpp"

namespace Gem::Courtier {

/******************************************************************************/
/** @brief Indicates whether the client executes the init-, run- or finally()-function */
enum class run_state : Gem::Common::ENUMBASETYPE {
    INIT = 0,
    RUN = 1,
    FINALLY = 2
};

/******************************************************************************/
/** @brief Ids of the allowed commands for the communication of networked consumers.
 *  REQUEST_LAYOUT / SEND_LAYOUT form the layout-send-once cache-miss fetch: a worker that
 *  receives a work item referencing a layout id it does not hold asks the server for it
 *  (REQUEST_LAYOUT), and the server replies with the serialized layout blob (SEND_LAYOUT). */
enum class networked_consumer_payload_command : Gem::Common::ENUMBASETYPE {
    NONE = 0,
    GETDATA = 1,
    NODATA = 2,
    COMPUTE = 3,
    RESULT = 4,
    STOP = 5,
    REQUEST_LAYOUT = 6,
    SEND_LAYOUT = 7
};

/******************************************************************************/
/** @brief Indicates in what state of the ping submission we are */
enum class beast_ping_state : Gem::Common::ENUMBASETYPE {
    CONNECTION_IS_ALIVE = 0,
    SENDING_PING = 1,
    CONNECTION_IS_STALE = 2
};

/******************************************************************************/
/**
 * Specification of the different consumer types held in the GConsumerRegistry
 */
enum class consumerType : Gem::Common::ENUMBASETYPE {
    SERIAL = 0,
    MULTITHREADED = 1,
    NETWORKED = 2,
    LAST = consumerType::NETWORKED
};

/******************************************************************************/
/**
 * Global variables for failed transfers and connection attempts.
 */
constexpr std::uint32_t GASIOCONSUMERMAXCONNECTIONATTEMPTS = 10;
constexpr unsigned short GCONSUMERDEFAULTPORT = 10000;
const std::string GCONSUMERDEFAULTSERVER = "localhost"; // NOLINT
constexpr std::uint16_t GCONSUMERLISTENERTHREADS = 4;
const Gem::Common::serializationMode GCONSUMERSERIALIZATIONMODE =
    Gem::Common::serializationMode::BINARY;
constexpr std::size_t GBEASTCONSUMERPINGINTERVAL = 15;

/******************************************************************************
 * Constants specifically for the GMPIConsumerT:
 */
/**
 * When GMPIConsumerWorkerNodeT does retrieve a NODATA response from GMPIConsumerMasterNodeT it waits for a random number
 * of milliseconds which is distributed between GMPICONSUMERWORKERNODERETRYINTERVALLOWERBOUNDARYMSEC and
 * GMPICONSUMERWORKERNODERETRYINTERVALUPPERBOUNDARYMSEC.
 */
constexpr std::uint32_t GMPICONSUMERWORKERNODERETRYINTERVALLOWERBOUNDARYMSEC = 50;
constexpr std::uint32_t GMPICONSUMERWORKERNODERETRYINTERVALUPPERBOUNDARYMSEC = 200;


/******************************************************************************/
/**
 * Needed by the submission machinery (workOn / processBatch) to distinguish successfully processed items,
 * items that have not returned (for unknown reasons, e.g. network failure)
 * and items for which an exception was thrown during processing.
 */
enum class processingStatus : Gem::Common::ENUMBASETYPE {
    UNPROCESSED =
        0 // The default/initial (and post-reset) state: the item has not been selected for processing
          // and carries no valid result yet. No defined object value.
        ,
    DO_PROCESS =
        1 // Marks a GProcessingContainerT as "needs to be processed". No defined object value.
        ,
    PROCESSED = 2 // Set when processing has taken place. Object value is well defined.
        ,
    EXCEPTION_CAUGHT =
        3 // Set by GProcessingContainerT when an exception was caught. No defined object value.
        ,
    ERROR_FLAGGED =
        4 // Set by an external entity when errors have occurred during processing. No defined object value.
};

/******************************************************************************/
/**
 * The per-batch SCHEDULING state of a work item, used by the courtier networked consumers. It is
 * distinct from processingStatus (the item's domain/processing truth) and is NOT serialized: it is
 * transient, server-side-only bookkeeping that lets the consumer track, within one dispatch round,
 * which slots are awaiting a client, in flight, or done -- directly on the item, so no side queues
 * are needed.
 */
enum class dispatchState : Gem::Common::ENUMBASETYPE {
    NONE = 0,      ///< Default / not part of an active networked dispatch
    PENDING = 1,   ///< Awaiting a client
    IN_FLIGHT = 2, ///< Handed to a client, result not yet back
    DONE = 3       ///< Result has been received and written into the slot
};

/******************************************************************************/
/**
 * These typedefs steer the types of the routing / lineage ids carried by submitted work items
 */
// 64-bit so the networked consumer can pack a 48-bit, process-unique, never-wrapping batch id with a
// 16-bit slot index (GNetworkedConsumerT). This is the per-DISPATCH ROUTING token: it is minted fresh each
// dispatch and is unique per in-flight slot, so a return is matched back to exactly the slot it was served
// from -- correct even when an individual is submitted more than once within a single OA iteration (large
// populations chunked into several spans, in-iteration re-dispatch). It deliberately does NOT identify the
// individual across re-dispatch; that LINEAGE identity is SUBMISSION_UUID_TYPE below.
using CORRELATION_ID_TYPE = std::uint64_t;

// The stable, per-individual LINEAGE identity of a work item (D11). Minted ONCE, at construction, and
// carried IMMUTABLY across every (re-)dispatch and serialized round-trip (fresh only on clone() = a new
// individual). Distinct from the per-dispatch CORRELATION_ID_TYPE above: the correlation id routes a single
// return to its slot, while the uuid lets a LATE return be reunited with -- and de-duplicated against -- the
// live individual it belongs to, even after that individual has been resubmitted under a new correlation id.
// 128-bit (process salt + monotonic counter) so ids never collide within or across runs / checkpoint resumes.
using SUBMISSION_UUID_TYPE = std::array<std::uint64_t, 2>;

/******************************************************************************/
// networked_consumer_payload_command and consumerType stream as their underlying
// numeric values through the shared machinery in GCommonEnums.hpp (marker
// specializations after the namespace end). Re-export the operators so ADL
// finds them for this namespace's enums. processingStatus keeps a hand-written
// pair: its insertion operator deliberately prints the enumerator NAME for
// diagnostics.
using Gem::Common::operator<<;
using Gem::Common::operator>>;

/** @brief Puts a Gem::Courtier::processingStatus into a stream. Needed for streaming / Gem::Common::fromString<> */
std::ostream &operator<<(std::ostream &o, const Gem::Courtier::processingStatus &srm);
/** @brief Reads a Gem::Courtier::processingStatus item from a stream. Needed for streaming / Gem::Common::fromString<> */
std::istream &operator>>(std::istream &i, Gem::Courtier::processingStatus &srm);

/******************************************************************************/

} /* namespace Gem::Courtier */

/******************************************************************************/
// Numeric streaming opt-in (see numeric_enum_io_v in GCommonEnums.hpp)
namespace Gem::Common {
template <> inline constexpr bool numeric_enum_io_v<Gem::Courtier::networked_consumer_payload_command> = true;
template <> inline constexpr bool numeric_enum_io_v<Gem::Courtier::consumerType> = true;
} /* namespace Gem::Common */
