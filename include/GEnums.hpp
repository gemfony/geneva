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
 * The two const variables MAXIMIZE and MINIMIZE determine, whether the library
 * should work in maximization or minimization mode.
 */
const bool MAXIMIZE = true;
const bool MINIMIZE = false;

/**********************************************************************************************/
/**
 * The number of iterations after which information should be
 * emitted about the inner state of the optimization algorithm.
 */
const boost::uint32_t DEFAULTREPORTGEN = 10; // deprecated
const boost::uint32_t DEFAULTREPORTITER = 1;

/**********************************************************************************************/
/**
 * The number of iterations after which a checkpoint should be written.
 * 0 means that no checkpoints are written at all.
 */
const boost::uint32_t DEFAULTCHECKPOINTGEN = 0; // deprecated
const boost::uint32_t DEFAULTCHECKPOINTIT = 0;

/**********************************************************************************************/
/**
 * The default maximum number of iterations
 */
const boost::uint32_t DEFAULTMAXGEN = 1000; // deprecated
const boost::uint32_t DEFAULTMAXIT = 1000;

/**********************************************************************************************/
/**
 * The default maximum number of iterations without improvement. 0 means: ignore
 */
const boost::uint32_t DEFAULMAXTSTALLGEN = 0; // deprecated
const boost::uint32_t DEFAULMAXTSTALLIT = 0;

/**********************************************************************************************/
/**
 * The default maximization mode
 */
const bool DEFAULTMAXMODE = false; // means: "minimization"

/**********************************************************************************************/
/**
 * A 0 time period . timedHalt will not trigger if this duration is set
 */
const std::string EMPTYDURATION = "00:00:00.000"; // 0 - no duration

/**********************************************************************************************/
/**
 * The default maximum duration of the calculation.
 */
const std::string DEFAULTDURATION = EMPTYDURATION;

/**********************************************************************************************/
/**
 * The default quality threshold
 */
const double DEFAULTQUALITYTHRESHOLD=0.;

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
/**
 * Currently three types of recombination schemes are supported:
 * - DEFAULTRECOMBINE defaults to RANDOMRECOMBINE
 * - RANDOMRECOMBINE chooses the parents to be replicated randomly from all parents
 * - VALUERECOMBINE prefers parents with a higher fitness
 */
enum recoScheme {
	  DEFAULTRECOMBINE
	, RANDOMRECOMBINE
	, VALUERECOMBINE
};

/**********************************************************************************************/
/**
 * The info function can be called in these three modes
 */
enum infoMode {
	  INFOINIT
	, INFOPROCESSING
	, INFOEND
};

/**********************************************************************************************/
/**
 * Ids that are assigned to adaptors and which should (by convention!) be unique for these
 */
enum adaptorId {
	  GIDENTITYADAPTOR
	, GSWARMADAPTOR
	, GDOUBLEGAUSSADAPTOR
	, GINT32GAUSSADAPTOR
	, GBOOLEANADAPTOR
	, GINT32FLIPADAPTOR
};

/**********************************************************************************************/
/**
 * The selection mode in populations. MUPLUSNU means that new parents are selected from old
 * parents and their children. MUCOMMNU means that new parents are selected from children only.
 * MUNU1PRETAIN means that the best parent of the last generation will also become a new parent
 * (unless a better child was found). All other parents are selected from children only.
 */
enum sortingMode {
	  MUPLUSNU
	, MUCOMMANU
	, MUNU1PRETAIN
};

/**********************************************************************************************/
/**
 * Different optimization algorithms need to assign different information to individuals. They
 * can thus assume different personalities, resulting in different data structures to be stored
 * in them (see the GPersonalityTraits classes).
 */
enum personality {
	  NONE
	, EA
	, GD
	, SWARM
};

/**********************************************************************************************/

const double DEFAULTSIGMA = 1; ///< Default start value for sigma_
const double DEFAULTSIGMASIGMA = 0.001; ///< Default width of the gaussian used for sigma adaption
const double DEFAULTMINSIGMA = 0.0000001; ///< Default minimum allowed value for sigma_
const double DEFAULTMAXSIGMA = 5; ///< Default maximum allowed value for sigma_
const double DEFAULTBITADPROB = 0.05; // 5 percent adaption probability for bits
const double DEFAULTADPROB = 1.0; // 100 percent adaption probability for all other cases

/**********************************************************************************************/

const double DEFAULTCLOCAL = 2.; ///< Default multiplier for local distances (swarm)
const double DEFAULTCGLOBAL = 2.; ///< Default multiplier for global distances (swarm)
const double DEFAULTCDELTA = 2.; ///< Default multiplier for deltas (swarm)
const double CLOCALRANGEDISABLED = -1.; ///< A value < 0 means that the local range is disabled
const double CGLOBALRANGEDISABLED = -1.; ///< A value < 0 means that the global range is disabled
const double CDELTARANGEDISABLED = -1.; ///< A value < 0 means that the delta range is disabled

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
