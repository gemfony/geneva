/**
 * @file GCommonEnums.hpp
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


// Standard headers go here
#include <string>
#include <istream>
#include <ostream>

// Includes check for correct Boost version(s)
#include "common/GGlobalDefines.hpp"

// Boost headers go here

#include <boost/cstdint.hpp>
#include <boost/cast.hpp>

#ifndef GCOMMONENUMS_HPP_
#define GCOMMONENUMS_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif

// Geneva headers go here

namespace Gem {
namespace Common {

/******************************************************************************/
/**
 * Different log and exception types
 */
enum logType {
   EXCEPTION = 0
   , TERMINATION = 1
   , WARNING = 2
   , LOGGING = 3
   , FILE = 4
   , STDOUT = 5
   , STDERR = 6
};

/******************************************************************************/
/** @brief Puts a Gem::Common::logType into a stream. Needed also for boost::lexical_cast<> */
std::ostream& operator<<(std::ostream&, const Gem::Common::logType&);
/** @brief Reads a Gem::Common::logType from a stream. Needed also for boost::lexical_cast<> */
std::istream& operator>>(std::istream&, Gem::Common::logType&);

/******************************************************************************/
/**
 * The default number of bins in histograms, used in GPlotDesigner
 */
const std::size_t DEFAULTNBINSGPD = 100;

/******************************************************************************/
/**
 * Used in parameter definitions (GParserBuilder)
 */
const bool VAR_IS_ESSENTIAL = true;
const bool VAR_IS_SECONDARY = false;

/******************************************************************************/
/**
 * Needed for the serialization of boost::logic::tribool
 */
enum triboolStates {
	  TBS_FALSE
	, TBS_INDETERMINATE
	, TBS_TRUE
};

/******************************************************************************/
/** @brief Puts a Gem::Common::triboolStates into a stream. Needed also for boost::lexical_cast<> */
std::ostream& operator<<(std::ostream&, const Gem::Common::triboolStates&);
/** @brief Reads a Gem::Common::triboolStates from a stream. Needed also for boost::lexical_cast<> */
std::istream& operator>>(std::istream&, Gem::Common::triboolStates&);

/******************************************************************************/
/**
 * The serialization modes that are currently allowed
 */
enum serializationMode {
	  SERIALIZATIONMODE_TEXT = 0
	, SERIALIZATIONMODE_XML = 1
	, SERIALIZATIONMODE_BINARY = 2
};

/******************************************************************************/

/** @brief Puts a Gem::Common::serializationMode into a stream. Needed also for boost::lexical_cast<> */
std::ostream& operator<<(std::ostream&, const Gem::Common::serializationMode&);
/** @brief Reads a Gem::Common::serializationMode item from a stream. Needed also for boost::lexical_cast<> */
std::istream& operator>>(std::istream&, Gem::Common::serializationMode&);

/******************************************************************************/
/**
 * Specification of whether checkExpectation should emit messages
 */
const bool CE_SILENT = false;
const bool CE_WITH_MESSAGES = true;

/******************************************************************************/
/**
 * Needed to express expectations in testing framework. CE stands for "Check expectation".
 */
enum expectation {
	  CE_EQUALITY // bitwise equality of all checked components
	, CE_INEQUALITY // at least one checked component differs
	, CE_FP_SIMILARITY // equality for non-floating point components, similarity for floating point
};

/******************************************************************************/

/** @brief Puts a Gem::Common::expectation into a stream. Needed also for boost::lexical_cast<> */
std::ostream& operator<<(std::ostream&, const Gem::Common::expectation&);
/** @brief Reads a Gem::Common::expectation item from a stream. Needed also for boost::lexical_cast<> */
std::istream& operator>>(std::istream&, Gem::Common::expectation&);

/******************************************************************************/
/**
 * Specification of the default number of threads, when no information about hardware
 * concurrency can be determined.
 */
const unsigned int DEFAULTNHARDWARETHREADS = 2;

/******************************************************************************/

} /* namespace Common */
} /* namespace Gem */

#endif /* GCOMMONENUMS_HPP_ */
