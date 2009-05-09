/**
 * @file GEnum.hpp
 */

/* Copyright (C) 2004-2008 Dr. Ruediger Berlich
 * Copyright (C) 2007-2008 Forschungszentrum Karlsruhe GmbH
 *
 * This file is part of Geneva, Gemfony scientific's optimization library.
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
 */

// Standard headers go here

// Boost headers go here

#include <boost/version.hpp>
#include "GGlobalDefines.hpp"
#if BOOST_VERSION < ALLOWED_BOOST_VERSION
#error "Error: Boost has incorrect version !"
#endif /* BOOST_VERSION */

#include <boost/cstdint.hpp>

#ifndef GENUMS_HPP_
#define GENUMS_HPP_

// GenEvA headers go here

namespace Gem {
namespace GenEvA {

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

  const double DEFAULTSIGMA = 1; ///< Default start value for sigma_
  const double DEFAULTSIGMASIGMA = 0.001; ///< Default width of the gaussian used for sigma adaption
  const double DEFAULTMINSIGMA = 0.0000001; ///< Default minimum allowed value for sigma_
  const double DEFAULTMAXSIGMA = 5; ///< Default maximum allowed value for sigma_
  const double DEFAULTMUTPROB = 0.05; // 5 percent mutation probability

  /**********************************************************************************************/

} /* namespace GenEvA */

//-----------------------------------------------------------------------------------------------

namespace Util {
/**********************************************************************************************/
/**
 * Random number generation can happen in three modes
 */
enum rnrGenerationMode { RNRFACTORY=0, RNRLOCAL=1, RNRPROXY=2};

/**********************************************************************************************/
} /* namespace Util */

} /* namespace Gem */



#endif /* GENUMS_HPP_ */
