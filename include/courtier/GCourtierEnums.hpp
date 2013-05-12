/**
 * @file GCourtierEnums.hpp
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt)
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) com
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
 * http://www.gemfony.com .
 */

// Standard headers go here
#include <string>
#include <ostream>
#include <istream>

// Includes check for correct Boost version(s)
#include "common/GGlobalDefines.hpp"

// Boost headers go here

#include <boost/cstdint.hpp>

#ifndef GCOURTIERENUMS_HPP_
#define GCOURTIERENUMS_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif

// Geneva headers go here
#include "common/GCommonEnums.hpp"

namespace Gem {
namespace Courtier {

/******************************************************************************/
/**
 * Needed so that server and client agree about the size of the headers and commands
 */
const std::size_t COMMANDLENGTH=64;

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
enum submissionReturnMode {
	INCOMPLETERETURN = 0
	, RESUBMISSIONAFTERTIMEOUT = 1
	, EXPECTFULLRETURN = 2
};

const std::size_t DEFAULTMAXRESUBMISSIONS = 5;

/******************************************************************************/
/**
 * These two typedefs allow to steer the types of ids assigned to objects submitted to the broker
 */
typedef boost::uint64_t ID_TYPE_1;
typedef std::size_t     ID_TYPE_2;

typedef ID_TYPE_1 SUBMISSIONCOUNTERTYPE;
typedef ID_TYPE_2 POSITIONTYPE;

/******************************************************************************/

/** @brief Puts a Gem::Courtier::submissionReturnMode into a stream. Needed also for boost::lexical_cast<> */
std::ostream& operator<<(std::ostream&, const Gem::Courtier::submissionReturnMode&);

/** @brief Reads a Gem::Courtier::submissionReturnMode item from a stream. Needed also for boost::lexical_cast<> */
std::istream& operator>>(std::istream&, Gem::Courtier::submissionReturnMode&);

/******************************************************************************/

} /* namespace Courtier */
} /* namespace Gem */

#endif /* GCOURTIERENUMS_HPP_ */
