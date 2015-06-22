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

// Boost headers go here
#include <boost/cstdint.hpp>
#include <boost/random.hpp>
#include <boost/date_time.hpp>
#include <boost/math/constants/constants.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include <boost/utility.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/cstdint.hpp>
#include <boost/cast.hpp>
#include <boost/function.hpp>
#include <boost/random/linear_congruential.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/type_traits.hpp>
#include <boost/static_assert.hpp>
#include <boost/concept_check.hpp>

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
	: private boost::noncopyable {
public:
	/***************************************************************************/
	/** @brief Helps to use this object as a generator for boost's PRNR distributions */
	typedef double result_type;

	/***************************************************************************/
	/** @brief The standard constructor */
	G_API_HAP GRandomBase();

	/** @brief A standard destructor */
	virtual G_API_HAP ~GRandomBase();

	/** @brief Retrieves an uniform_01 item */
	G_API_HAP result_type operator()();
	/** @brief Returns the minimum value returned by evenRandom() */
	G_API_HAP result_type (min)() const;
	/** @brief Returns the maximum value returned by evenRandom() */
	G_API_HAP result_type (max)() const;

	/***************************************************************************/
	/**
	 * Returns the precision of the values returned by operator(). This
	 * makes it possible to use GRandomBase as a generator for Boost's random
	 * distributions.
	 *
	 * @return The precision of the values returned by GRandomBase::operator()
	 */
	static G_API_HAP std::size_t precision() {
		return std::numeric_limits<GRandomBase::result_type>::digits;
	}

	/***************************************************************************/

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
		return static_cast<fp_type>(dbl_random01());
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
		return uniform_01<fp_type>() * maxVal;
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
#ifdef DEBUG
		// Check that minVal and maxVal have appropriate values
		assert(minVal<=maxVal);
#endif

		if (minVal >= (fp_type) 0. || maxVal <= (fp_type) 0.) { // (maxVal-minVal) is valid
			return uniform_01<fp_type>() * (maxVal - minVal) + minVal;
		}
		else { // Some values (e.g. maxVal=std::numeric_limits<T>::max(), minVal=-std::numeric_limits<fp_type>::max()) will fail (maxVal-minVal)
			// We know: minVal<0., maxVal>0.
			assert(minVal < (fp_type) 0.);
			assert(maxVal > (fp_type) 0.);

			// Calculate a random number in the range [0,1[
			fp_type fraction = uniform_01<fp_type>();

			// Calculate the fraction of the distance of minVal from 0.
			volatile fp_type minFraction = -fraction * minVal; // < std::numeric_limits<T>::max(), thus valid
			// Calculate the fraction of the distance of max from 0.
			volatile fp_type maxFraction = fraction * maxVal; // < std::numeric_limits<T>::max(), thus valid

			// The start of the scale
			volatile fp_type result = minVal + minFraction;

			// Add maxFraction to result. Possible problem: Can compiler-optimization amalgamate this to "return minVal+minFraction+maxFraction" ?
			result += maxFraction;

			return result;
		}
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
		const fp_type &sigma, typename std::enable_if<std::is_floating_point<fp_type>::value>::type *dummy = 0
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
		const fp_type &mean, const fp_type &sigma,
		typename std::enable_if<std::is_floating_point<fp_type>::value>::type *dummy = 0
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
		}
		else {
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
		const int_type &minVal, const int_type &maxVal,
		typename std::enable_if<std::is_integral<int_type>::value>::type *dummy = 0
	) {
#ifdef DEBUG
		assert(maxVal >= minVal);
#endif /* DEBUG */

		// A uniform distribution in the desired range. Note that boost::uniform_int produces
		// random numbers up to and including its upper limit. Note that ui is a distribution
		// only. The actual generator is provided by this class (see variate_generator).
		boost::uniform_int<int_type> ui(minVal, maxVal);

		// A generator that binds together our own random number generator and a uniform_int distribution
		boost::variate_generator<Gem::Hap::GRandomBase &, boost::uniform_int<int_type>> boost_uniform_int(*this, ui);

		return boost_uniform_int();
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
		const int_type &maxVal, typename std::enable_if<std::is_integral<int_type>::value>::type *dummy = 0
	) {
		return this->uniform_int<int_type>(0, maxVal);
	}

	/*************************************************************************/
	/**
	 * This function produces integer random numbers in the range of [minVal, maxVal] .
	 * Note that maxVal may also be < 0. . The size of the integers is assumed to be
	 * small compared to int_type's value range.
	 *
	 * @param minVal The minimum value of the range
	 * @param maxVal The maximum (excluded) value of the range
	 * @return Discrete random numbers evenly distributed in the range [minVal,maxVal]
	 */
	template<typename int_type>
	int_type uniform_smallint(
		const int_type &minVal, const int_type &maxVal,
		typename std::enable_if<std::is_integral<int_type>::value>::type *dummy = 0
	) {
#ifdef DEBUG
		assert(maxVal >= minVal);
#endif /* DEBUG */

		// A uniform distribution in the desired range. Note that boost::uniform_int produces
		// random numbers up to and including its upper limit. Note that ui is a distribution
		// only. The actual generator is provided by this class (see variate_generator).
		boost::uniform_smallint<int_type> ui(minVal, maxVal);

		// A generator that binds together our own random number generator and a uniform_smallint distribution
		boost::variate_generator<Gem::Hap::GRandomBase &, boost::uniform_smallint<int_type>> boost_uniform_smallint(
			*this, ui);

		return boost_uniform_smallint();
	}

	/***************************************************************************/
	/**
	 * This function produces integer random numbers in the range of [0, maxVal] .
	 * The size of the integers is assumed to be small compared to int_type's value
	 * range.
	 *
	 * @param maxVal The maximum (excluded) value of the range
	 * @return Discrete random numbers evenly distributed in the range [0,maxVal]
	 */
	template<typename int_type>
	int_type uniform_smallint(
		const int_type &maxVal, typename std::enable_if<std::is_integral<int_type>::value>::type *dummy = 0
	) {
#ifdef DEBUG
		assert(maxVal >= 0);
#endif /* DEBUG */

		return this->uniform_smallint<int_type>(0, maxVal);
	}

protected:
	/***************************************************************************/
	/** @brief Uniformly distributed double random numbers in the range [0,1[ */
	virtual G_API_HAP double dbl_random01() = 0;

public:
	/***************************************************************************/
	/** @brief The minimum value returned by evenRandom() */
	const result_type min_value;
	/** @brief The maximum value returned by evenRandom() */
	const result_type max_value;

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
