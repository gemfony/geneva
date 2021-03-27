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
 * Geneva was started by Dr. Rüdiger Berlich and was later maintained together
 * with Dr. Ariel Garcia under the auspices of Gemfony scientific. For further
 * information on Gemfony scientific, see http://www.gemfomy.eu .
 *
 * The majority of files in Geneva was released under the Apache license v2.0
 * in February 2020.
 *
 * See the NOTICE file in the top-level directory of the Geneva library
 * collection for a list of contributors and copyright information.
 *
 ********************************************************************************/

#pragma once

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard headers go here
#include <string>
#include <ostream>
#include <istream>
#include <chrono>

// Boost headers go here

// Geneva headers go here
#include "common/GCommonEnums.hpp"

namespace Gem::Courtier {

/******************************************************************************/
/**
 * The default value for the socket's reuse_address option
 */
const bool DEFAULTREUSEADDRESS = true;

/**
 * Default CPU core thread pinning
 */
const bool DEFAULTUSECOREPINNING = true;

/**
 * Whether to use one IO-context ber thread or one io_context object with multiple
 * run() calls, one for each thread
 */
const bool DEFAULTMULTIPLEIOCONTEXTS = true;

/**
 * Whether to activate the socket's no_delay option
 */
const bool DEFAULTUSENODELAY = false;

/******************************************************************************/
/** @brief Indicates whether the client executes the init-, run- or finally()-function */
enum class run_state : Gem::Common::ENUMBASETYPE {
	 INIT=0
	 , RUN=1
	 , FINALLY=2
};

/******************************************************************************/
/** @brief Ids of the allowed commands for the communication of networked consumers */
enum class networked_consumer_payload_command : Gem::Common::ENUMBASETYPE {
	 NONE = 0
	 , GETDATA = 1
	 , NODATA = 2
	 , COMPUTE = 3
	 , RESULT = 4
};

/******************************************************************************/
/** @brief Indicates in what state of the ping submission we are */
enum class beast_ping_state : Gem::Common::ENUMBASETYPE {
	 CONNECTION_IS_ALIVE = 0
	 , SENDING_PING = 1
	 , CONNECTION_IS_STALE = 2
};


/******************************************************************************/
/**
 * Specification of different consumer types of the broker
 */
enum class consumerType : Gem::Common::ENUMBASETYPE {
	 SERIAL = 0
	 , MULTITHREADED = 1
	 , NETWORKED = 2
	 , LAST = static_cast<Gem::Common::ENUMBASETYPE>(consumerType::NETWORKED)
};

/**
 * The default parallelization mode of optimization algorithms
 */
const consumerType DEFAULT_BROKER_MODE = consumerType::MULTITHREADED;

/******************************************************************************/
/**
 * Global variables for failed transfers and connection attempts.
 */
const std::uint32_t GASIOCONSUMERMAXSTALLS = 0; // infinite number of stalls
const std::uint32_t GASIOCONSUMERMAXCONNECTIONATTEMPTS = 10;
const unsigned short GCONSUMERDEFAULTPORT = 10000;
const std::string GCONSUMERDEFAULTSERVER = "localhost"; // NOLINT
const std::uint16_t GCONSUMERLISTENERTHREADS = 4;
const Gem::Common::serializationMode GCONSUMERSERIALIZATIONMODE = Gem::Common::serializationMode::BINARY;
const std::int32_t GASIOMAXOPENPINGS = 100; // The maximum number of pings without matching pong from the server
const std::chrono::milliseconds GASIOPINGINTERVAL = std::chrono::milliseconds(1000); // NOLINT
const std::size_t GBEASTCONSUMERPINGINTERVAL = 15;
const std::size_t GBEASTMSTIMEOUT = 50;

/******************************************************************************/
/** @brief The default number of threads for parallelization with threads */
const std::uint16_t DEFAULTNSTDTHREADS = 2;
// TODO: Unify with Geneva-namespace constant of same name

/******************************************************************************/
/**
 * The size of input and output buffers of the GBufferPortT class
 */

const std::size_t DEFAULTRAWBUFFERSIZE=Gem::Common::DEFAULTBUFFERSIZE;
const std::size_t DEFAULTPROCESSEDBUFFERSIZE=Gem::Common::DEFAULTBUFFERSIZE;

/******************************************************************************/
/**
 * Needed by the executor to distinguish between successfully processed items,
 * items that have not returned (for unknown reasons, e.g. network failure)
 * and items for which an exception was thrown during processing.
 */
enum class processingStatus : Gem::Common::ENUMBASETYPE {
	 DO_IGNORE = 0 // Set by the submitter when an item does not need to be processed. No defined object value.
	 , DO_PROCESS = 1 // Marks a GProcessingContainerT as "needs to be processed". No defined object value.
	 , PROCESSED = 2 // Set when processing has taken place. Object value is well defined.
	 , EXCEPTION_CAUGHT = 3 // Set by GProcessingContainerT when an exception was caught. No defined object value.
	 , ERROR_FLAGGED = 4 // Set by an external entity when errors have occurred during processing. No defined object value.
};

/******************************************************************************/
/**
 * Determines how many items contribute to the rolling average and max calculation
 * of return times. This is calcultated as a multiple of the expected number of
 * return items from the first iteration.
 */
const std::size_t NEXPECTEDITEMSMULTIPLE=2;

/******************************************************************************/
/**
 * Indicates processed or unprocessed work items
 */
const bool GBC_UNPROCESSED = true;
const bool GBC_PROCESSED = false;

/******************************************************************************/
/**
 * Indicates whether a client wants to continue or terminate
 */
const bool CLIENT_CONTINUE = true;
const bool CLIENT_TERMINATE = false;

/******************************************************************************/
/**
 * Needed so that server and client agree about the size of the headers and commands.
 * Currently our longest command has 7 characters. As we read commands synchronously,
 * we want to keep the command length as small as possible. Note that, as the size
 * of the data body is submitted as a "command", data bodies may not have more than
 * 16 digits describing the number of bytes to expect. This should however suffice for
 * every practical purpose.
 */
const std::size_t COMMANDLENGTH = 36;

/******************************************************************************/
/**
 * The default factor applied to the turn-around time
 * of the first item in the current iteration. Used to
 * find a suitable timeout-value for following individuals.
 * Used in conjunction with optimization algorithms that
 * communicate via the "courtier" broker infrastructure.
 */
const double DEFAULTMINBROKERWAITFACTOR = 1.;
const double DEFAULTMAXBROKERWAITFACTOR = 10.;
const double DEFAULTBROKERWAITFACTOR = DEFAULTMAXBROKERWAITFACTOR;
const double DEFAULTBROKERWAITFACTORINCREMENT = 0.1;
const double DEFAULTMINPERCENTAGEOFTIMEOUT = 0.7;

const double DEFAULTBROKERWAITFACTOR2 = 1.1; // For GBrokerExecutorT
const double DEFAULTINITIALBROKERWAITFACTOR2 = 1.;

const std::uint16_t DEFAULTEXECUTORPARTIALRETURNPERCENTAGE = 0; ///< The minimum percentage of returned items in an iteration after which execution will continue

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
	INCOMPLETERETURN = 0
	, RESUBMISSIONAFTERTIMEOUT = 1
	, EXPECTFULLRETURN = 2
};

const submissionReturnMode DEFAULTSRM = submissionReturnMode::EXPECTFULLRETURN;
const std::size_t DEFAULTMAXRESUBMISSIONS = 5;

/******************************************************************************/
/**
 * These typedefs allow to steer the types of ids assigned to objects submitted to the broker
 */
using ITERATION_COUNTER_TYPE = std::uint64_t;
using RESUBMISSION_COUNTER_TYPE = std::size_t;
using COLLECTION_POSITION_TYPE = std::size_t;
using BUFFERPORT_ID_TYPE = std::uint32_t;

/******************************************************************************/

const BUFFERPORT_ID_TYPE MAXREGISTEREDBUFFERPORTS = 1000; ///< The maximum number of registered buffer ports in the broker

/******************************************************************************/

/** @brief Puts a Gem::Courtier::beast_payload_command into a stream. Needed also for boost::lexical_cast<> */
G_API_COURTIER std::ostream &operator<<(std::ostream &, const Gem::Courtier::networked_consumer_payload_command &);
/** @brief Reads a Gem::Courtier::beast_payload_command item from a stream. Needed also for boost::lexical_cast<> */
G_API_COURTIER std::istream &operator>>(std::istream &, Gem::Courtier::networked_consumer_payload_command &);

/** @brief Puts a Gem::Courtier::beast_ping_state into a stream. Needed also for boost::lexical_cast<> */
G_API_COURTIER std::ostream &operator<<(std::ostream &, const Gem::Courtier::beast_ping_state &);
/** @brief Reads a Gem::Courtier::beast_ping_state item from a stream. Needed also for boost::lexical_cast<> */
G_API_COURTIER std::istream &operator>>(std::istream &, Gem::Courtier::beast_ping_state &);

/** @brief Puts a Gem::Courtier::submissionReturnMode into a stream. Needed also for boost::lexical_cast<> */
G_API_COURTIER std::ostream &operator<<(std::ostream &, const Gem::Courtier::submissionReturnMode &);
/** @brief Reads a Gem::Courtier::submissionReturnMode item from a stream. Needed also for boost::lexical_cast<> */
G_API_COURTIER std::istream &operator>>(std::istream &, Gem::Courtier::submissionReturnMode &);

/** @brief Puts a Gem::Courtier::processingStatus into a stream. Needed also for boost::lexical_cast<> */
G_API_COURTIER std::ostream &operator<<(std::ostream &, const Gem::Courtier::processingStatus &);
/** @brief Reads a Gem::Courtier::processingStatus item from a stream. Needed also for boost::lexical_cast<> */
G_API_COURTIER std::istream &operator>>(std::istream &, Gem::Courtier::processingStatus &);

/** @brief Puts a Gem::Courtier::brokerMode into a stream. Needed also for boost::lexical_cast<> */
G_API_COURTIER std::ostream& operator<<(std::ostream&, const Gem::Courtier::consumerType&);
/** @brief Reads a Gem::Courtier::brokerMode item from a stream. Needed also for boost::lexical_cast<> */
G_API_COURTIER std::istream& operator>>(std::istream&, Gem::Courtier::consumerType&);

/******************************************************************************/

} // namespace Gem::Geneva

