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
#include <chrono>
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
 * Specification of different consumer types of the broker
 */
enum class consumerType : Gem::Common::ENUMBASETYPE {
    SERIAL = 0,
    MULTITHREADED = 1,
    NETWORKED = 2,
    LAST = consumerType::NETWORKED
};

/**
 * The default parallelization mode of optimization algorithms
 */
const consumerType DEFAULT_BROKER_MODE = consumerType::MULTITHREADED;

/******************************************************************************/
/**
 * Global variables for failed transfers and connection attempts.
 */
constexpr std::uint32_t GASIOCONSUMERMAXSTALLS = 0; // infinite number of stalls
constexpr std::uint32_t GASIOCONSUMERMAXCONNECTIONATTEMPTS = 10;
constexpr unsigned short GCONSUMERDEFAULTPORT = 10000;
const std::string GCONSUMERDEFAULTSERVER = "localhost"; // NOLINT
constexpr std::uint16_t GCONSUMERLISTENERTHREADS = 4;
const Gem::Common::serializationMode GCONSUMERSERIALIZATIONMODE =
    Gem::Common::serializationMode::BINARY;
constexpr std::int32_t GASIOMAXOPENPINGS =
    100; // The maximum number of pings without matching pong from the server
const std::chrono::milliseconds GASIOPINGINTERVAL = std::chrono::milliseconds(1000); // NOLINT
constexpr std::size_t GBEASTCONSUMERPINGINTERVAL = 15;
constexpr std::size_t GBEASTMSTIMEOUT = 50;

/******************************************************************************
 * Constants specifically for the GMPIConsumerT:
 */
/**
 * The timer to use for retrieving new work items from the broker and putting processed work items into the broker
 */
constexpr std::size_t GMPICONSUMERBROKERACCESSBROKERTIMEOUT = 50;
/**
 * When GMPIConsumerWorkerNodeT does retrieve a NODATA response from GMPIConsumerMasterNodeT it waits for a random number
 * of milliseconds which is distributed between GMPICONSUMERWORKERNODERETRYINTERVALLOWERBOUNDARYMSEC and
 * GMPICONSUMERWORKERNODERETRYINTERVALUPPERBOUNDARYMSEC.
 */
constexpr std::uint32_t GMPICONSUMERWORKERNODERETRYINTERVALLOWERBOUNDARYMSEC = 50;
constexpr std::uint32_t GMPICONSUMERWORKERNODERETRYINTERVALUPPERBOUNDARYMSEC = 200;

/******************************************************************************/
/** @brief The default number of threads for parallelization with threads */
constexpr std::uint16_t DEFAULTNSTDTHREADS = 2;

/******************************************************************************/
/**
 * The size of input and output buffers of the GBufferPortT class
 */

const std::size_t DEFAULTRAWBUFFERSIZE = Gem::Common::DEFAULTBUFFERSIZE;
const std::size_t DEFAULTPROCESSEDBUFFERSIZE = Gem::Common::DEFAULTBUFFERSIZE;

/******************************************************************************/
/**
 * Needed by the executor to distinguish between successfully processed items,
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
 * Determines how many items contribute to the rolling average and max calculation
 * of return times. This is calcultated as a multiple of the expected number of
 * return items from the first iteration.
 */
constexpr std::size_t NEXPECTEDITEMSMULTIPLE = 2;

/******************************************************************************/
/**
 * Indicates processed or unprocessed work items
 */
constexpr bool GBC_UNPROCESSED = true;
constexpr bool GBC_PROCESSED = false;

/******************************************************************************/
/**
 * Indicates whether a client wants to continue or terminate
 */
constexpr bool CLIENT_CONTINUE = true;
constexpr bool CLIENT_TERMINATE = false;

/******************************************************************************/
/**
 * Needed so that server and client agree about the size of the headers and commands.
 * Currently our longest command has 7 characters. As we read commands synchronously,
 * we want to keep the command length as small as possible. Note that, as the size
 * of the data body is submitted as a "command", data bodies may not have more than
 * 16 digits describing the number of bytes to expect. This should however suffice for
 * every practical purpose.
 */
constexpr std::size_t COMMANDLENGTH = 36;

/******************************************************************************/
/**
 * The default factor applied to the turn-around time
 * of the first item in the current iteration. Used to
 * find a suitable timeout-value for following individuals.
 * Used in conjunction with optimization algorithms that
 * communicate via the "courtier" broker infrastructure.
 */
constexpr double DEFAULTMINBROKERWAITFACTOR = 1.;
constexpr double DEFAULTMAXBROKERWAITFACTOR = 10.;
const double DEFAULTBROKERWAITFACTOR = DEFAULTMAXBROKERWAITFACTOR;
constexpr double DEFAULTBROKERWAITFACTORINCREMENT = 0.1;
constexpr double DEFAULTMINPERCENTAGEOFTIMEOUT = 0.7;

constexpr double DEFAULTBROKERWAITFACTOR2 = 1.1; // For GBrokerExecutorT
constexpr double DEFAULTINITIALBROKERWAITFACTOR2 = 1.;

constexpr std::uint16_t DEFAULTEXECUTORPARTIALRETURNPERCENTAGE =
    0; ///< The minimum percentage of returned items in an iteration after which execution will continue

constexpr double DEFAULTEXECUTORFIRSTITEMMAXWAITSECONDS =
    0.; ///< Max seconds to wait for the very first item of a run (0 == wait indefinitely)

/******************************************************************************/
/**
 * A 0 time period . timedHalt will not trigger if this duration is set
 */
const std::string EMPTYDURATION = "00:00:00.000"; // 0 - no duration  NOLINT

/******************************************************************************/
/**
 * The default allowed time in seconds for the first individual
 * in generation 0 to return. Set it to 0 to disable this timeout.
 * Used in conjunction with optimization algorithms that
 * communicate via the "courtier" broker infrastructure.
 */
const std::string DEFAULTBROKERFIRSTTIMEOUT = EMPTYDURATION; // NOLINT

/******************************************************************************/
/**
 * The default maximum duration of the calculation.
 */
const std::string DEFAULTDURATION = EMPTYDURATION; // NOLINT

/******************************************************************************/
/**
 * Needed by the broker connector
 */
enum class submissionReturnMode : Gem::Common::ENUMBASETYPE {
    INCOMPLETERETURN = 0,
    RESUBMISSIONAFTERTIMEOUT = 1,
    EXPECTFULLRETURN = 2
};

const submissionReturnMode DEFAULTSRM = submissionReturnMode::EXPECTFULLRETURN;
constexpr std::size_t DEFAULTMAXRESUBMISSIONS = 5;

/******************************************************************************/
/**
 * These typedefs allow to steer the types of ids assigned to objects submitted to the broker
 */
using ITERATION_COUNTER_TYPE = std::uint64_t;
using RESUBMISSION_COUNTER_TYPE = std::size_t;
using COLLECTION_POSITION_TYPE = std::size_t;
// 64-bit so the networked consumer can pack a 48-bit, process-unique, never-wrapping batch id with a
// 16-bit slot index (GNetworkedConsumerT). A wide batch id is what makes late-return routing safe when
// many algorithms submit through one shared consumer: a stale return cannot alias a freshly-minted batch.
using CORRELATION_ID_TYPE = std::uint64_t;

/******************************************************************************/

/** @brief Puts a Gem::Courtier::networked_consumer_payload_command into a stream. Needed for streaming / Gem::Common::fromString<> */
std::ostream &
operator<<(std::ostream &o, const Gem::Courtier::networked_consumer_payload_command &ps);
/** @brief Reads a Gem::Courtier::networked_consumer_payload_command item from a stream. Needed for streaming / Gem::Common::fromString<> */
std::istream &
operator>>(std::istream &i, Gem::Courtier::networked_consumer_payload_command &ps);

/** @brief Puts a Gem::Courtier::beast_ping_state into a stream. Needed for streaming / Gem::Common::fromString<> */
std::ostream &operator<<(std::ostream &o, const Gem::Courtier::beast_ping_state &ps);
/** @brief Reads a Gem::Courtier::beast_ping_state item from a stream. Needed for streaming / Gem::Common::fromString<> */
std::istream &operator>>(std::istream &i, Gem::Courtier::beast_ping_state &ps);

/** @brief Puts a Gem::Courtier::submissionReturnMode into a stream. Needed for streaming / Gem::Common::fromString<> */
std::ostream &
operator<<(std::ostream &o, const Gem::Courtier::submissionReturnMode &srm);
/** @brief Reads a Gem::Courtier::submissionReturnMode item from a stream. Needed for streaming / Gem::Common::fromString<> */
std::istream &operator>>(std::istream &i, Gem::Courtier::submissionReturnMode &srm);

/** @brief Puts a Gem::Courtier::processingStatus into a stream. Needed for streaming / Gem::Common::fromString<> */
std::ostream &operator<<(std::ostream &o, const Gem::Courtier::processingStatus &srm);
/** @brief Reads a Gem::Courtier::processingStatus item from a stream. Needed for streaming / Gem::Common::fromString<> */
std::istream &operator>>(std::istream &i, Gem::Courtier::processingStatus &srm);

/** @brief Puts a Gem::Courtier::consumerType into a stream. Needed for streaming / Gem::Common::fromString<> */
std::ostream &operator<<(std::ostream &o, const Gem::Courtier::consumerType &bm);
/** @brief Reads a Gem::Courtier::consumerType item from a stream. Needed for streaming / Gem::Common::fromString<> */
std::istream &operator>>(std::istream &i, Gem::Courtier::consumerType &bm);

/******************************************************************************/

} /* namespace Gem::Courtier */
