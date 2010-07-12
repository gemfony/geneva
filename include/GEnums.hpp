/**
 * @file GEnums.hpp
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

namespace GenEvA {

/**********************************************************************************************/
/**
 * Needed so that server and client agree about the size of the headers and commands
 */
const std::size_t COMMANDLENGTH=64;


/**********************************************************************************************/
/**
 * The allowed modes during data exchange with external programs
 */
enum dataExchangeMode {
	  BINARYEXCHANGE
	, TEXTEXCHANGE
};

/**********************************************************************************************/
/**
 * The serialization modes that are currently allowed
 */
enum serializationMode {
	  TEXTSERIALIZATION = 0
	, XMLSERIALIZATION = 1
	, BINARYSERIALIZATION = 2
};

/**********************************************************************************************/

/** @brief Puts a Gem::GenEvA::serializationMode into a stream. Needed also for boost::lexical_cast<> */
std::ostream& operator<<(std::ostream&, const Gem::GenEvA::serializationMode&);

/** @brief Reads a Gem::GenEvA::serializationMode item from a stream. Needed also for boost::lexical_cast<> */
std::istream& operator>>(std::istream&, Gem::GenEvA::serializationMode&);

/** @brief Puts a Gem::GenEvA::dataExchangeMode into a stream. Needed also for boost::lexical_cast<> */
std::ostream& operator<<(std::ostream&, const Gem::GenEvA::dataExchangeMode&);

/** @brief Reads a Gem::GenEvA::dataExchangeMode item from a stream. Needed also for boost::lexical_cast<> */
std::istream& operator>>(std::istream&, Gem::GenEvA::dataExchangeMode&);

/**********************************************************************************************/

} /* namespace GenEvA */

//-----------------------------------------------------------------------------------------------

namespace Util {

/**********************************************************************************************/
/**
 * Needed for the serialization of boost::logic::tribool
 */
enum triboolStates {
	  FALSE
	, INDETERMINATE
	, TRUE
};

/**********************************************************************************************/
/** @brief Puts a Gem::Util::triboolStates into a stream. Needed also for boost::lexical_cast<> */
std::ostream& operator<<(std::ostream&, const Gem::Util::triboolStates&);

/** @brief Reads a Gem::Util::triboolStates from a stream. Needed also for boost::lexical_cast<> */
std::istream& operator>>(std::istream&, Gem::Util::triboolStates&);

/**********************************************************************************************/
/**
 * Specification of whether checkExpectation should emit messages
 */
const bool CE_SILENT = false;
const bool CE_WITH_MESSAGES = true;

/**********************************************************************************************/
/**
 * Needed to express expectations in testing framework
 */
enum expectation {
	  CE_EQUALITY // bitwise equality of all checked components
	, CE_INEQUALITY // at least one checked component differs
	, CE_FP_SIMILARITY // equality for non-floating point components, similarity for floating point
};

/**********************************************************************************************/

} /* namespace Util */

} /* namespace Gem */



#endif /* GENUMS_HPP_ */
