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
class GRandomBase
	: private boost::noncopyable
{
public:
	 /** @brief Helps to use this object as a generator for C++11 std::distributions */
	typedef G_BASE_GENERATOR::result_type result_type;

	 /***************************************************************************/
	 /** @brief The standard constructor */
	 G_API_HAP GRandomBase();
	 /** @brief A standard destructor */
	 virtual G_API_HAP ~GRandomBase();
	 /** @brief Retrieves an uniform_01 item */
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
	 // TODO: May be modelled by bernoulli-distribution instead

	 /** @brief This function returns true with a probability "probability", otherwise false */
	 G_API_HAP bool weighted_bool(const double &);
	 /** @brief This function produces boolean values with a 50% likelihood each for true and false */
	 G_API_HAP bool uniform_bool();

	 /***************************************************************************/
	 /** @brief Uniformly distributed random numbers in the range [0,1[ */
	 template<typename fp_type>
	 fp_type uniform_01(
		 typename std::enable_if<std::is_floating_point<fp_type>::value>::type *dummy = 0
	 ) {
		 return std::uniform_real_distribution<fp_type>(static_cast<fp_type>(0.), static_cast<fp_type>(1.))(*this);
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
		 const fp_type &maxVal, typename std::enable_if<std::is_floating_point<fp_type>::value>::type *dummy = 0
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
		 typename std::enable_if<std::is_floating_point<fp_type>::value>::type *dummy = 0
	 ) {
		 return std::uniform_real_distribution<fp_type>(static_cast<fp_type>(minVal), static_cast<fp_type>(maxVal))(*this);
	 }

	 /***************************************************************************/
	 /**
	  * Produces gaussian-distributed floating point random numbers with sigma 1
	  * and mean 0. This function is a trap. See the corresponding specializations
	  * for the actual implementation.
	  *
	  * @return floating point random numbers with a gaussian distribution
	  */
	 template<typename fp_type>
	 fp_type normal_distribution() {
		 glogger
		 << "In GRandomBase::normal_distribution<fp_type>(): Error!" << std::endl
		 << "function called with incorrect type" << std::endl
		 << GEXCEPTION;
	 }

	 /***************************************************************************/
	 /**
	  * Produces gaussian-distributed random numbers with mean 0 and sigma "sigma"
	  *
	  * @param mean The mean value of the Gaussian
	  * @param sigma The sigma of the Gaussian
	  * @return floating point random numbers with a gaussian distribution
	  */
	 template<typename fp_type>
	 fp_type normal_distribution(
		 const fp_type &sigma
		 , typename std::enable_if<std::is_floating_point<fp_type>::value>::type *dummy = 0
	 ) {
		 return sigma * normal_distribution<fp_type>();
	 }

	 /***************************************************************************/
	 /**
	  * Produces gaussian-distributed random numbers with
	  * mean "mean" and sigma "sigma"
	  *
	  * @param mean The mean value of the Gaussian
	  * @param sigma The sigma of the Gaussian
	  * @return floating point random numbers with a gaussian distribution
	  */
	 template<typename fp_type>
	 fp_type normal_distribution(
		 const fp_type &mean
		 , const fp_type &sigma
		 , typename std::enable_if<std::is_floating_point<fp_type>::value>::type *dummy = 0
	 ) {
		 return sigma * normal_distribution<fp_type>() + mean;
	 }

	 /***************************************************************************/
	 /**
	  * This function adds two gaussians with sigma "sigma" and a distance
	  * "distance" from each other, centered around mean. The idea is to use
	  * this function in conjunction with evolutionary strategies, so we avoid
	  * searching with the highest likelihood at a location where we already
	  * know a good value exists. Rather we want to shift the highest likelihood
	  * for probes a bit further away from the candidate solution.
	  *
	  * @param mean The mean value of the entire distribution
	  * @param sigma The (equal) sigma of both gaussians
	  * @param distance The distance between both peaks
	  * @return Random numbers with a bi-gaussian shape
	  */
	 template<typename fp_type>
	 fp_type bi_normal_distribution(
		 const fp_type &mean, const fp_type &sigma, const fp_type &distance,
		 typename std::enable_if<std::is_floating_point<fp_type>::value>::type *dummy = 0
	 ) {
		 if (uniform_bool()) {
			 return normal_distribution<fp_type>(mean - Gem::Common::gfabs(distance / 2.), sigma);
		 }
		 else {
			 return normal_distribution<fp_type>(mean + Gem::Common::gfabs(distance / 2.), sigma);
		 }
	 }

	 /***************************************************************************/
	 /**
	  * This function adds two gaussians with sigmas "sigma1" and "sigma2" and a
	  * distance "distance" from each other, centered around mean. The idea is to use
	  * this function in conjunction with evolutionary strategies, so we avoid
	  * searching with the highest likelihood at a location where we already
	  * know a good value exists. Rather we want to shift the highest likelihood
	  * for probes a bit further away from the candidate solution.
	  *
	  * @param mean The mean value of the entire distribution
	  * @param sigma1 The sigma of the first gaussian
	  * @param sigma2 The sigma of the second gaussian
	  * @param distance The distance between both peaks
	  * @return Random numbers with a bi-gaussian shape
	  */
	 template<typename fp_type>
	 fp_type bi_normal_distribution(
		 const fp_type &mean, const fp_type &sigma1, const fp_type &sigma2, const fp_type &distance,
		 typename std::enable_if<std::is_floating_point<fp_type>::value>::type *dummy = 0
	 ) {
		 if (uniform_bool()) {
			 return normal_distribution<fp_type>(mean - Gem::Common::gfabs(distance / 2.), sigma1);
		 } else {
			 return normal_distribution<fp_type>(mean + Gem::Common::gfabs(distance / 2.), sigma2);
		 }
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
		 , typename std::enable_if<std::is_integral<int_type>::value>::type *dummy = 0
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
		 , typename std::enable_if<std::is_integral<int_type>::value>::type *dummy = 0
	 ) {
		 return this->uniform_int<int_type>(0, maxVal);
	 }

protected:
	 /***************************************************************************/
	 /** @brief Uniformly distributed integer numbers in the range */
	 virtual G_API_HAP result_type int_random() = 0;

private:
	 /***************************************************************************/
	 /** @brief Two float gaussian random numbers are produced in one go. One number can be cached here */
	 float fltGaussCache_;
	 /** @brief Two double gaussian random numbers are produced in one go. One  number can be cached here */
	 double dblGaussCache_;
	 /** @brief Specifies whether a valid cached float gaussian is available */
	 bool fltGaussCacheAvailable_;
	 /** @brief Specifies whether a valid cached double gaussian is available */
	 bool dblGaussCacheAvailable_;
};

/******************************************************************************/
// Specializations of a number of local functions

/******************************************************************************/
/**
 * Produces gaussian-distributed float random numbers with sigma 1 and mean 0
 *
 * @return float random numbers with a gaussian distribution
 */
template<>
inline float GRandomBase::normal_distribution<float>() {
	using namespace Gem::Common;

	if (fltGaussCacheAvailable_) {
		fltGaussCacheAvailable_ = false;
		return fltGaussCache_;
	}
	else {
#ifdef GEM_HAP_USE_BOXMULLER
		float rnr1 = uniform_01<float>();
      float rnr2 = uniform_01<float>();
      dblGaussCache_ = gsqrt(GFabs(-2.f * glog(1.f - rnr1))) * gcos(2.f * boost::math::constants::pi<float>()  * rnr2);
      dblGaussCacheAvailable_ = true;
      return gsqrt(GFabs(-2.f * glog(1.f - rnr1))) * gsin(2.f * boost::math::constants::pi<float>()   * rnr2);
#else // GEM_HAP_USE_BOXMULLERPOLAR, see here: http://de.wikipedia.org/wiki/Normalverteilung#Polar-Methode ; faster than GEM_HAP_USE_BOXMULLER
		float q, u1, u2;
		do {
			u1 = 2.f * uniform_01<float>() - 1.f;
			u2 = 2.f * uniform_01<float>() - 1.f;
			q = u1 * u1 + u2 * u2;
		} while (q > 1.f);
		q = gsqrt((-2.f * glog(q)) / q);
		fltGaussCache_ = u2 * q;
		fltGaussCacheAvailable_ = true;
		return u1 * q;
#endif
	}
}

/******************************************************************************/
/**
 * Produces gaussian-distributed double random numbers with sigma 1 and mean 0
 *
 * @return double random numbers with a gaussian distribution
 */
template<>
inline double GRandomBase::normal_distribution<double>() {
	using namespace Gem::Common;

	if (dblGaussCacheAvailable_) {
		dblGaussCacheAvailable_ = false;
		return dblGaussCache_;
	}
	else {
#ifdef GEM_HAP_USE_BOXMULLER
		double rnr1 = uniform_01<double>();
        double rnr2 = uniform_01<double>();
        dblGaussCache_ = gsqrt(GFabs(-2. * glog(1. - rnr1))) * gcos(2. * boost::math::constants::pi<double>() * rnr2);
        dblGaussCacheAvailable_ = true;
        return gsqrt(GFabs(-2. * glog(1. - rnr1))) * gsin(2. * boost::math::constants::pi<double>()  * rnr2);
#else // GEM_HAP_USE_BOXMULLERPOLAR, see here: http://de.wikipedia.org/wiki/Normalverteilung#Polar-Methode ; faster than GEM_HAP_USE_BOXMULLER
		double q, u1, u2;
		do {
			u1 = 2. * uniform_01<double>() - 1.;
			u2 = 2. * uniform_01<double>() - 1.;
			q = u1 * u1 + u2 * u2;
		} while (q > 1.0);
		q = gsqrt((-2. * glog(q)) / q);
		dblGaussCache_ = u2 * q;
		dblGaussCacheAvailable_ = true;
		return u1 * q;
#endif
	}
}

/******************************************************************************/

} /* namespace Hap */
} /* namespace Gem */

#endif /* GRANDOMBASE_HPP_ */
