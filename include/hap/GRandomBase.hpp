/**
 * @file
 */

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

// Hap headers go here
#include "common/GCommonMathHelperFunctions.hpp"
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
	// Prevent copying
	GRandomBase(const GRandomBase&) = delete;
	GRandomBase(const GRandomBase&&) = delete;
	GRandomBase& operator=(const GRandomBase&) = delete;
	GRandomBase& operator=(const GRandomBase&&) = delete;

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

private:
	 /***************************************************************************/
	 /** @brief Uniformly distributed integer numbers in the range min/max */
	 virtual G_API_HAP result_type int_random() = 0;
};

/******************************************************************************/

} /* namespace Hap */
} /* namespace Gem */

