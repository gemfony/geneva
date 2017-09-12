/**
 * @file GRandomBase.hpp
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

#include <cstdlib>
#include <iomanip>
#include <ctime>
#include <cmath>
#include <iostream>
#include <sstream>
#include <cassert>
#include <algorithm>
#include <random>

// Boost headers go here
#include <boost/math/constants/constants.hpp>
#include <boost/utility.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/cast.hpp>

#ifndef GRANDOMBASE_HPP_
#define GRANDOMBASE_HPP_

// Hap headers go here
#include "common/GMathHelperFunctions.hpp"
#include "hap/GHapEnums.hpp"
#include "hap/GRandomDefines.hpp"
#include "hap/GRandomFactory.hpp"

/******************************************************************************/

namespace Gem {
namespace Hap {

/******************************************************************************/
/**
 * This class defines ways of obtaining different random number distributions
 * from "raw" random numbers, which can be obtained in derived classes using
 * various different ways.
 */
class GRandomBase
{
public:
	 /** @brief Helps to use this object as a generator for C++11 std::distributions */
	 using result_type = G_BASE_GENERATOR::result_type;

	 /***************************************************************************/
	 /** @brief The standard constructor */
	 G_API_HAP GRandomBase();
	 /** @brief A standard destructor */
	 virtual G_API_HAP ~GRandomBase();
	 /** @brief Retrieves a "raw" random item item */
	 G_API_HAP GRandomBase::result_type operator()();

	 /***************************************************************************/
	 /**
	  * This function is part of the standard interface of C++11 random number
	  * engines. It returns the minimum value returned by the generator. Since
	  * this class acts as a proxy for a wrapped generator or a generator running
	  * as a factory, we simply return the base generators min()-Value.
	  */
	 static constexpr G_API_HAP result_type (min)() {
		 return (G_BASE_GENERATOR::min)();
	 }

	 /***************************************************************************/
	 /**
	  * This function is part of the standard interface of C++11 random number
	  * engines. It returns the maximum value returned by the generator. Since
	  * this class acts as a proxy for a wrapped generator or a generator running
	  * as a factory, we simply return the base generators max()-Value.
	  */
	 static constexpr G_API_HAP result_type (max)() {
		 return (G_BASE_GENERATOR::max)();
	 }

protected:
	 /***************************************************************************/
	 // Prevent copying
	 GRandomBase(const GRandomBase&) = delete;
	 GRandomBase(const GRandomBase&&) = delete;
	 const GRandomBase& operator=(const GRandomBase&) = delete;
	 const GRandomBase& operator=(const GRandomBase&&) = delete;

	 /** @brief Uniformly distributed integer numbers in the range min/max */
	 virtual G_API_HAP result_type int_random() = 0;
};

/******************************************************************************/

} /* namespace Hap */
} /* namespace Gem */

#endif /* GRANDOMBASE_HPP_ */
