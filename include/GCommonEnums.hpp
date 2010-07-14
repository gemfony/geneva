/**
 * @file GCommonEnums.hpp
 */

/* Copyright (C) Dr. Ruediger Berlich and Karlsruhe Institute of Technology
 * (University of the State of Baden-Wuerttemberg and National Laboratory
 * of the Helmholtz Association)
 *
 * Contact: info [at] gemfony (dot) com
 *
 * This file is part of the Geneva library, Gemfony scientific's optimization
 * library.
 *
 * Geneva is free software: you can redistribute it and/or modify
 * it under the terms of version 3 of the GNU Affero General Public License
 * as published by the Free Software Foundation.
 *
 * Geneva is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with the Geneva library.  If not, see <http://www.gnu.org/licenses/>.
 *
 * For further information on Gemfony scientific and Geneva, visit
 * http://www.gemfony.com .
 */

// Standard headers go here
#include <string>
#include <istream>
#include <ostream>

// Includes check for correct Boost version(s)
#include "GGlobalDefines.hpp"

// Boost headers go here

#include <boost/cstdint.hpp>

#ifndef GENUMS_HPP_
#define GENUMS_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif

// GenEvA headers go here

namespace Gem {
namespace Common {

/**********************************************************************************************/
/**
 * Needed for the serialization of boost::logic::tribool
 */
enum triboolStates {
	  TBS_FALSE
	, TBS_INDETERMINATE
	, TBS_TRUE
};

/**********************************************************************************************/
/** @brief Puts a Gem::Util::triboolStates into a stream. Needed also for boost::lexical_cast<> */
std::ostream& operator<<(std::ostream&, const Gem::Common::triboolStates&);
/** @brief Reads a Gem::Util::triboolStates from a stream. Needed also for boost::lexical_cast<> */
std::istream& operator>>(std::istream&, Gem::Common::triboolStates&);

/**********************************************************************************************/
/**
 * The serialization modes that are currently allowed
 */
enum serializationMode {
	  SERIALIZATIONMODE_TEXT = 0
	, SERIALIZATIONMODE_XML = 1
	, SERIALIZATIONMODE_BINARY = 2
};

/**********************************************************************************************/

/** @brief Puts a Gem::Common::serializationMode into a stream. Needed also for boost::lexical_cast<> */
std::ostream& operator<<(std::ostream&, const Gem::Common::serializationMode&);
/** @brief Reads a Gem::Common::serializationMode item from a stream. Needed also for boost::lexical_cast<> */
std::istream& operator>>(std::istream&, Gem::Common::serializationMode&);

/**********************************************************************************************/
/**
 * Specification of whether checkExpectation should emit messages
 */
const bool CE_SILENT = false;
const bool CE_WITH_MESSAGES = true;

/**********************************************************************************************/
/**
 * Needed to express expectations in testing framework. CE stands for "Check expectation".
 */
enum expectation {
	  CE_EQUALITY // bitwise equality of all checked components
	, CE_INEQUALITY // at least one checked component differs
	, CE_FP_SIMILARITY // equality for non-floating point components, similarity for floating point
};

/**********************************************************************************************/

/** @brief Puts a Gem::Common::expectation into a stream. Needed also for boost::lexical_cast<> */
std::ostream& operator<<(std::ostream&, const Gem::Common::expectation&);
/** @brief Reads a Gem::Common::expectation item from a stream. Needed also for boost::lexical_cast<> */
std::istream& operator>>(std::istream&, Gem::Common::expectation&);

/**********************************************************************************************/

} /* namespace Common */
} /* namespace Gem */

#endif /* GENUMS_HPP_ */
