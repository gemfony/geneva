/**
 * @file GCourtierEnums.hpp
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt)
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) eu
 *
 * This file is part of the Geneva library collection.
 *
 * Geneva was developed with kind support from Karlsruhe Institute of
 * Technology (KIT) and Steinbuch Centre for Computing (SCC). Further
 * information about KIT and SCC can be found at http://www.kit.edu/english
 * and http://scc.kit.edu .
 *
 * Geneva is free software: you can redistribute and/or modify it under
 * the terms of version 3 of the GNU Affero General Public License
 * as published by the Free Software Foundation.
 *
 * Geneva is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with the Geneva library. If not, see <http://www.gnu.org/licenses/>.
 *
 * For further information on Gemfony scientific and Geneva, visit
 * http://www.gemfony.eu .
 */

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard headers go here
#include <string>
#include <ostream>
#include <istream>
#include <chrono>

// Boost headers go here
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_serialize.hpp>

#ifndef GCOURTIERENUMS_HPP_
#define GCOURTIERENUMS_HPP_

// Geneva headers go here
#include "common/GCommonEnums.hpp"

namespace Gem {
namespace Courtier {

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
const std::uint32_t GASIOTCPCONSUMERMAXSTALLS = 0; // infinite number of stalls
const std::uint32_t GASIOTCPCONSUMERMAXCONNECTIONATTEMPTS = 10;
const unsigned short GASIOTCPCONSUMERDEFAULTPORT = 10000;
const std::string GASIOTCPCONSUMERDEFAULTSERVER = "localhost";
const std::uint16_t GASIOTCPCONSUMERTHREADS = 4;
const Gem::Common::serializationMode GASIOTCPCONSUMERSERIALIZATIONMODE = Gem::Common::serializationMode::SERIALIZATIONMODE_BINARY;
const std::int32_t GASIOMAXOPENPINGS = 100; // The maximum number of pings without matching pong from the server
const std::chrono::milliseconds GASIOPINGINTERVAL = std::chrono::milliseconds(1000);

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
	 IGNORE = 0 // Set by the submitter when an item does not need to be processed
	 , DO_PROCESS = 1 // Marks a GProcessingContainerT as "needs to be processed"
	 , PROCESSED = 2 // Set when processing has taken place
	 , EXCEPTION_CAUGHT = 3 // Set by GProcessingContainerT when an exception was caught
	 , ERROR_FLAGGED = 4 // Set by an external entity when errors have occurred during processing
};

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
const std::size_t COMMANDLENGTH = 36; // must accomodate a boost::uuids::uuid of size 36

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
const std::string EMPTYDURATION = "00:00:00.000"; // 0 - no duration

/******************************************************************************/
/**
 * The default allowed time in seconds for the first individual
 * in generation 0 to return. Set it to 0 to disable this timeout.
 * Used in conjunction with optimization algorithms that
 * communicate via the "courtier" broker infrastructure.
 */
const std::string DEFAULTBROKERFIRSTTIMEOUT = EMPTYDURATION;

/******************************************************************************/
/**
 * The default maximum duration of the calculation.
 */
const std::string DEFAULTDURATION = EMPTYDURATION;

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
using BUFFERPORT_ID_TYPE = boost::uuids::uuid;

/******************************************************************************/

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

} /* namespace Courtier */
} /* namespace Gem */

#endif /* GCOURTIERENUMS_HPP_ */
