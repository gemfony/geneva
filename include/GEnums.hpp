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
 * The allowed modes during data exchange with external programs
 */
enum dataExchangeMode {BINARYEXCHANGE=0, TEXTEXCHANGE=1};

/**********************************************************************************************/
/**
 * The serialization modes that are currently allowed
 */
enum serializationMode {TEXTSERIALIZATION=0, XMLSERIALIZATION=1, BINARYSERIALIZATION=2};

/**********************************************************************************************/
/**
 * Currently three types of recombination schemes are supported:
 * - DEFAULTRECOMBINE defaults to RANDOMRECOMBINE
 * - RANDOMRECOMBINE chooses the parents to be replicated randomly from all parents
 * - VALUERECOMBINE prefers parents with a higher fitness
 */
enum recoScheme {DEFAULTRECOMBINE=0, RANDOMRECOMBINE=1, VALUERECOMBINE=2};

/**********************************************************************************************/
/**
 * The info function can be called in these three modes
 */
enum infoMode {INFOINIT=0, INFOPROCESSING=1, INFOEND=2};

/**********************************************************************************************/
/**
 * Ids that are assigned to adaptors and which should (by convention!) be unique for these
 */
enum adaptorId {
	GDOUBLEGAUSSADAPTOR=0,
	GINT32GAUSSADAPTOR=1,
	GBOOLEANADAPTOR=2,
	GINT32FLIPADAPTOR=3,
	GCHARFLIPADAPTOR=4
};

/**********************************************************************************************/
/**
 * The selection mode in populations. MUPLUSNU means that new parents are selected from old
 * parents and their children. MUCOMMNU means that new parents are selected from children only.
 * MUNU1PRETAIN means that the best parent of the last generation will also become a new parent
 * (unless a better child was found). All other parents are selected from children only.
 */
enum sortingMode {MUPLUSNU=0, MUCOMMANU=1, MUNU1PRETAIN=2};

/**********************************************************************************************/
/**
 * Different optimization algorithms need to assign different information to individuals. They
 * can thus assume different personalities, resulting in different data structures to be stored
 * in them (see the GPersonalityTraits classes).
 */
enum personality {NONE=0, EA=1, GD=2, SWARM=3};

/**********************************************************************************************/

const double DEFAULTSIGMA = 1; ///< Default start value for sigma_
const double DEFAULTSIGMASIGMA = 0.001; ///< Default width of the gaussian used for sigma adaption
const double DEFAULTMINSIGMA = 0.0000001; ///< Default minimum allowed value for sigma_
const double DEFAULTMAXSIGMA = 5; ///< Default maximum allowed value for sigma_
const double DEFAULTBITMUTPROB = 0.05; // 5 percent mutation probability for bits
const double DEFAULTMUTPROB = 1.0; // 100 percent mutation probability for all other cases

/**********************************************************************************************/

} /* namespace GenEvA */

//-----------------------------------------------------------------------------------------------

namespace Util {

/**********************************************************************************************/
/**
 * Random number generation can happen in two modes
 */
enum rnrGenerationMode { RNRFACTORY=0, RNRLOCAL=1 };

/**
 * The default random number generation mode
 */
const rnrGenerationMode DEFAULTRNRGENMODE=RNRFACTORY;

/**********************************************************************************************/

} /* namespace Util */

} /* namespace Gem */



#endif /* GENUMS_HPP_ */
