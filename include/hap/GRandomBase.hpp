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
#include <boost/date_time.hpp>
#include <boost/math/constants/constants.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include <boost/utility.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/cast.hpp>
#include <boost/function.hpp>

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
class GRandomBase : private boost::noncopyable
{
public:
	 /** @brief Helps to use this object as a generator for C++11 std::distributions */
	typedef G_BASE_GENERATOR::result_type result_type;

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
		 return G_BASE_GENERATOR::min();
	 }

	 /***************************************************************************/
	 /**
	  * This function is part of the standard interface of C++11 random number
	  * engines. It returns the maximum value returned by the generator. Since
	  * this class acts as a proxy for a wrapped generator or a generator running
	  * as a factory, we simply return the base generators max()-Value.
	  */
	 static constexpr G_API_HAP result_type (max)() {
		 return G_BASE_GENERATOR::max();
	 }

	 /***************************************************************************/
	 /**
	  * Emits evenly distributed random numbers in the range [0,maxVal[
	  *
	  * @param maxVal The maximum (excluded) value of the range
	  * @return Random numbers evenly distributed in the range [0,maxVal[
	  */
	 template<typename fp_type>
	 fp_type uniform_real(
		 const fp_type &maxVal, typename std::enable_if<std::is_floating_point<fp_type>::value>::type *dummy = nullptr
	 ) {
#ifdef DEBUG
		 // Check that maxVal has an appropriate value
		assert(maxVal>=(fp_type)0.);
#endif
		 return std::uniform_real_distribution<fp_type>(static_cast<fp_type>(0.), static_cast<fp_type>(maxVal))(*this);
	 }

	 /***************************************************************************/
	 /**
	  * Produces evenly distributed random numbers in the range [minVal,maxVal[
	  *
	  * @param minVal The minimum value of the range
	  * @param maxVal The maximum (excluded) value of the range
	  * @return Random numbers evenly distributed in the range [minVal,maxVal[
	  */
	 template<typename fp_type>
	 fp_type uniform_real(
		 const fp_type &minVal, const fp_type &maxVal,
		 typename std::enable_if<std::is_floating_point<fp_type>::value>::type *dummy = nullptr
	 ) {
		 return std::uniform_real_distribution<fp_type>(static_cast<fp_type>(minVal), static_cast<fp_type>(maxVal))(*this);
	 }

	 /*************************************************************************/
	 /**
	  * This function produces integer random numbers in the range of [minVal, maxVal] .
	  * Note that maxVal may also be < 0. .
	  *
	  * @param minVal The minimum value of the range
	  * @param maxVal The maximum (excluded) value of the range
	  * @return Discrete random numbers evenly distributed in the range [minVal,maxVal]
	  */
	 template<typename int_type>
	 int_type uniform_int(
		 const int_type &minVal
		 , const int_type &maxVal
		 , typename std::enable_if<std::is_integral<int_type>::value>::type *dummy = nullptr
	 ) {
#ifdef DEBUG
		 assert(maxVal >= minVal);
#endif /* DEBUG */

		 // A uniform distribution in the desired range. Note that boost::uniform_int produces
		 // random numbers up to and including its upper limit. Note that ui is a distribution
		 // only. The actual generator is provided by this class (see variate_generator).
		 return std::uniform_int_distribution<int_type>(minVal, maxVal)(*this);
	 }

	 /***************************************************************************/
	 /**
	  * This function produces integer random numbers in the range of [0, maxVal] .
	  *
	  * @param maxVal The maximum (excluded) value of the range
	  * @return Discrete random numbers evenly distributed in the range [0,maxVal]
	  */
	 template<typename int_type>
	 int_type uniform_int(
		 const int_type &maxVal
		 , typename std::enable_if<std::is_integral<int_type>::value>::type *dummy = nullptr
	 ) {
		 return this->uniform_int<int_type>(0, maxVal);
	 }

protected:
	 /***************************************************************************/
	 /** @brief Uniformly distributed integer numbers in the range */
	 virtual G_API_HAP result_type int_random() = 0;
};

/******************************************************************************/

} /* namespace Hap */
} /* namespace Gem */

#endif /* GRANDOMBASE_HPP_ */
