/**
 * @file GRandomBase.hpp
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt) and Karlsruhe
 * Institute of Technology (University of the State of Baden-Wuerttemberg
 * and National Laboratory of the Helmholtz Association).
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) com
 *
 * This file is part of the Geneva library collection
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

#include <cstdlib>
#include <iomanip>
#include <ctime>
#include <cmath>
#include <iostream>
#include <sstream>
#include <cassert>

// Includes check for correct Boost version(s)
#include "common/GGlobalDefines.hpp"

// Boost headers go here
#include <boost/cstdint.hpp>
#include <boost/random.hpp>
#include <boost/date_time.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>
#include <boost/utility.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/shared_array.hpp>
#include <boost/pool/detail/singleton.hpp>
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

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif


// Hap headers go here
#include "common/GMathHelperFunctions.hpp"
#include "GHapEnums.hpp"
#include "GRandomDefines.hpp"
#include "GRandomFactory.hpp"

/****************************************************************************/

namespace Gem {
namespace Hap {

/****************************************************************************/
/**
 * This class defines ways of obtaining different random number distributions
 * from "raw" random numbers, which can be obtained in derived classes using
 * various different ways.
 */
class GRandomBase
	: private boost::noncopyable
{
public:
	/****************************************************************************/
	/** @brief Helps to use this object as a generator for boost's PRNR distributions */
	typedef double result_type;
	/** @brief The minimum value returned by evenRandom() */
	const result_type min_value;
	/** @brief The maximum value returned by evenRandom() */
	const result_type max_value;

	/************************************************************************/
	/** @brief The standard constructor */
	GRandomBase()
		: min_value(result_type(0.))
		, max_value(result_type(1.))
		, fltGaussCache_(float(0.))
		, dblGaussCache_(double(0.))
		, ldblGaussCache_((long double)0.)
		, fltGaussCacheAvailable_(false)
    	, dblGaussCacheAvailable_(false)
		, ldblGaussCacheAvailable_(false)
	{ /* nothing */ }

	/************************************************************************/
	/** @brief A standard destructor */
	virtual ~GRandomBase()
	{ /* nothing */ }

	/************************************************************************/
	/** @brief Uniformly distributed random numbers in the range [0,1[ */
	template <typename fp_type>
	fp_type uniform_01(
			typename boost::enable_if<boost::is_floating_point<fp_type> >::type* dummy = 0
	) {
		return static_cast<float>(uniform_01<double>());
	}

	/************************************************************************/
	/**
	 * Retrieves an uniform_01 item. Thus function, together with the min() and
	 * max() functions make it possible to use GRandomBase as a generator for
	 * boost's random distributions.
	 *
	 * @return A random number taken from the evenRandom function
	 */
	result_type operator()() {
		return uniform_01<result_type>();
	}

	/************************************************************************/
	/**
	 * Returns the minimum value returned by evenRandom()
	 *
	 * @return The minimum value returned by evenRandom()
	 */
	result_type min() const {
		return min_value;
	}

	/************************************************************************/
	/**
	 * Returns the maximum value returned by evenRandom()
	 *
	 * @return The maximum value returned by evenRandom()
	 */
	result_type max() const {
		return max_value;
	}

	/************************************************************************/
	/**
	 * Emits evenly distributed random numbers in the range [0,max[
	 *
	 * @param max The maximum (excluded) value of the range
	 * @return Random numbers evenly distributed in the range [0,max[
	 */
	template <typename fp_type>
	fp_type uniform_real(
		  const fp_type& max
		, typename boost::enable_if<boost::is_floating_point<fp_type> >::type* dummy = 0
	) {
#ifdef DEBUG
		// Check that min and max have appropriate values
		assert(max>=(fp_type)0.);
#endif
		return uniform_01<fp_type>() * max;
	}

	/************************************************************************/
	/**
	 * Produces evenly distributed random numbers in the range [min,max[
	 *
	 * @param min The minimum value of the range
	 * @param max The maximum (excluded) value of the range
	 * @return Random numbers evenly distributed in the range [min,max[
	 */
	template <typename fp_type>
	fp_type uniform_real(
			const fp_type& min
		  , const fp_type& max
		  , typename boost::enable_if<boost::is_floating_point<fp_type> >::type* dummy = 0
	) {
#ifdef DEBUG
		// Check that min and max have appropriate values
		assert(min<=max);
#endif

		if(min >= (fp_type)0. || max <= (fp_type)0.) { // (max-min) is valid
			return uniform_01<fp_type>() * (max - min) + min;
		}
		else { // Some values (e.g. max=std::numeric_limits<T>::max(), min=-std::numeric_limits<fp_type>::max()) will fail (max-min)
			// We know: min<0., max>0.
			assert(min<(fp_type)0.);
			assert(max>(fp_type)0.);

			// Calculate a random number in the range [0,1[
			fp_type fraction = uniform_01<fp_type>();

			// Calculate the fraction of the distance of min from 0.
			volatile fp_type minFraction = -fraction*min; // < std::numeric_limits<T>::max(), thus valid
			// Calculate the fraction of the distance of max from 0.
			volatile fp_type maxFraction = fraction*max; // < std::numeric_limits<T>::max(), thus valid

			// The start of the scale
			volatile fp_type result = min + minFraction;

			// Add maxFraction to result. Possible problem: Can compiler-optimization amalgamate this to "return min+minFraction+maxFraction" ?
			result += maxFraction;

			return result;
		}
	}

	/************************************************************************/
	/**
	 * Produces gaussian-distributed floating point random numbers with sigma 1
	 * and mean 0. This function is a trap. See the corresponding specializations
	 * for the actual implementation.
	 *
	 * @return floating point random numbers with a gaussian distribution
	 */
	template<typename fp_type>
	fp_type normal_distribution() {
		raiseException(
			"In GRandomBase::normal_distribution<fp_type>(): Error!" << std::endl
			<< "function called with incorrect type"
		);
	}

	/************************************************************************/
	/**
	 * Produces gaussian-distributed random numbers with mean 0 and sigma "sigma"
	 *
	 * @param mean The mean value of the Gaussian
	 * @param sigma The sigma of the Gaussian
	 * @return floating point random numbers with a gaussian distribution
	 */
	template <typename fp_type>
	fp_type normal_distribution(
			const fp_type& sigma
		  , typename boost::enable_if<boost::is_floating_point<fp_type> >::type* dummy = 0
	) {
		return sigma * normal_distribution<fp_type>();
	}

	/************************************************************************/
	/**
	 * Produces gaussian-distributed random numbers with
	 * mean "mean" and sigma "sigma"
	 *
	 * @param mean The mean value of the Gaussian
	 * @param sigma The sigma of the Gaussian
	 * @return floating point random numbers with a gaussian distribution
	 */
	template <typename fp_type>
	fp_type normal_distribution(
			const fp_type& mean
		  , const fp_type& sigma
		  , typename boost::enable_if<boost::is_floating_point<fp_type> >::type* dummy = 0
	) {
		return sigma * normal_distribution<fp_type>() + mean;
	}

	/************************************************************************/
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
	template <typename fp_type>
	fp_type bi_normal_distribution(
			const fp_type& mean
		  , const fp_type& sigma
		  , const fp_type& distance
		  , typename boost::enable_if<boost::is_floating_point<fp_type> >::type* dummy = 0
	) {
		if (uniform_bool()) {
			return normal_distribution<fp_type>(mean - GFabs(distance / 2.), sigma);
		}
		else {
			return normal_distribution<fp_type>(mean + GFabs(distance / 2.), sigma);
		}
	}

	/************************************************************************/
	/**
	 * This function returns true with a probability "probability", otherwise false.
	 *
	 * @param p The probability for the value "true" to be returned
	 * @return A boolean value, which will be true with a user-defined likelihood
	 */
	bool weighted_bool(
			const double& probability
	) {
	#ifdef DEBUG
		assert(probability>=0. && probability<=1.);
	#endif
		return ((uniform_01<double>()<probability)?true:false);
	}

	/************************************************************************/
	/**
	 * This function produces boolean values with a 50% likelihood each for
	 * true and false.
	 *
	 * @return Boolean values with a 50% likelihood for true/false respectively
	 */
	bool uniform_bool() {
		return weighted_bool(0.5);
	}

	/*************************************************************************/
	/**
	 * This function produces integer random numbers in the range of [min, max] .
	 * Note that max may also be < 0. .
	 *
	 * @param min The minimum value of the range
	 * @param max The maximum (excluded) value of the range
	 * @return Discrete random numbers evenly distributed in the range [min,max]
	 */
	template <typename int_type>
	int_type uniform_int (
			  const int_type& min
			, const int_type& max
			, typename boost::enable_if<boost::is_integral<int_type> >::type* dummy = 0
	) {
#ifdef DEBUG
		assert(max >= min);
#endif /* DEBUG */

		// A uniform distribution in the desired range. Note that boost::uniform_int produces
		// random numbers up to and including its upper limit.
		boost::uniform_int<int_type> ui(min, max);

		// A generator that binds together our own random number generator and a uniform_int distribution
		boost::variate_generator<Gem::Hap::GRandomBase&, boost::uniform_int<int_type> > boost_uniform_int(*this, ui);

		return boost_uniform_int();
	}

	/****************************************************************************/
	/**
	 * This function produces integer random numbers in the range of [0, max] .
	 *
	 * @param max The maximum (excluded) value of the range
	 * @return Discrete random numbers evenly distributed in the range [0,max]
	 */
	template <typename int_type>
	int_type uniform_int (
			  const int_type& max
			, typename boost::enable_if<boost::is_integral<int_type> >::type* dummy = 0
	) {
		return uniform_int(0, max);
	}

protected:
	 /** @brief Uniformly distributed double random numbers in the range [0,1[ */
	virtual double dbl_random01() = 0;

private:
	/************************************************************************/
	/** @brief Two float gaussian random numbers are produced in one go. One number can be cached here */
	float fltGaussCache_;
	/** @brief Two double gaussian random numbers are produced in one go. One  number can be cached here */
	double dblGaussCache_;
	/** @brief Two long double gaussian random numbers are produced in one go. One  number can be cached here */
	long double ldblGaussCache_;
	/** @brief Specifies whether a valid cached float gaussian is available */
	bool fltGaussCacheAvailable_;
	/** @brief Specifies whether a valid cached double gaussian is available */
	bool dblGaussCacheAvailable_;
	/** @brief Specifies whether a valid cached long double gaussian is available */
	bool ldblGaussCacheAvailable_;
};

/****************************************************************************/

/**
 * Produces gaussian-distributed float random numbers with sigma 1 and mean 0
 *
 * @return float random numbers with a gaussian distribution
 */
template<>
float GRandomBase::normal_distribution<float>() {
	using namespace Gem::Common;

	if(fltGaussCacheAvailable_) {
		fltGaussCacheAvailable_ = false;
		return fltGaussCache_;
	}
	else {
#ifdef USEBOXMULLER
		float rnr1 = uniform_01<float>();
		float rnr2 = uniform_01<float>();
		dblGaussCache_ = GSqrt(GFabs(-2.f * GLog(1.f - rnr1))) * GCos(2.f * (float)M_PI	* rnr2);
		dblGaussCacheAvailable_ = true;
		return GSqrt(GFabs(-2.f * GLog(1.f - rnr1))) * GSin(2.f * (float)M_PI	* rnr2);
#else // USEBOXMULLERPOLAR, see here: http://de.wikipedia.org/wiki/Normalverteilung#Polar-Methode ; faster than USEBOXMULLER
		float q, u1, u2;
		do {
			u1 = 2.f* uniform_01<float>() - 1.f;
			u2 = 2.f* uniform_01<float>() - 1.f;
			q = u1*u1 + u2*u2;
		} while (q > 1.f);
		q = GSqrt((-2.f*GLog(q))/q);
		fltGaussCache_ = u2 * q;
		fltGaussCacheAvailable_ = true;
		return u1 * q;
#endif
	}
}

/****************************************************************************/
/**
 * Produces gaussian-distributed double random numbers with sigma 1 and mean 0
 *
 * @return double random numbers with a gaussian distribution
 */
template<>
double GRandomBase::normal_distribution<double>() {
	using namespace Gem::Common;

	if(dblGaussCacheAvailable_) {
		dblGaussCacheAvailable_ = false;
		return dblGaussCache_;
	}
	else {
#ifdef USEBOXMULLER
		double rnr1 = uniform_01<double>();
		double rnr2 = uniform_01<double>();
		dblGaussCache_ = GSqrt(GFabs(-2. * GLog(1. - rnr1))) * GCos(2. * M_PI	* rnr2);
		dblGaussCacheAvailable_ = true;
		return GSqrt(GFabs(-2. * GLog(1. - rnr1))) * GSin(2. * M_PI	* rnr2);
#else // USEBOXMULLERPOLAR, see here: http://de.wikipedia.org/wiki/Normalverteilung#Polar-Methode ; faster than USEBOXMULLER
		double q, u1, u2;
		do {
			u1 = 2.* uniform_01<double>() - 1.;
			u2 = 2.* uniform_01<double>() - 1.;
			q = u1*u1 + u2*u2;
		} while (q > 1.0);
		q = GSqrt((-2.*GLog(q))/q);
		dblGaussCache_ = u2 * q;
		dblGaussCacheAvailable_ = true;
		return u1 * q;
#endif
	}
}

/****************************************************************************/
/**
 * Produces gaussian-distributed long double random numbers with sigma 1 and mean 0
 *
 * @return double random numbers with a gaussian distribution
 */
template<>
long double GRandomBase::normal_distribution<long double>() {
	using namespace Gem::Common;

	if(ldblGaussCacheAvailable_) {
		ldblGaussCacheAvailable_ = false;
		return ldblGaussCache_;
	}
	else {
#ifdef USEBOXMULLER
		long double rnr1 = uniform_01<long double>();
		long double rnr2 = uniform_01<long double>();
		ldblGaussCache_ = GSqrt(GFabs(-2. * GLog(1.l - rnr1))) * GCos(2.l * (long double)M_PI	* rnr2);
		ldblGaussCacheAvailable_ = true;
		return GSqrt(GFabs(-2.l * GLog(1.l - rnr1))) * GSin(2. * (long double)M_PI	* rnr2);
#else // USEBOXMULLERPOLAR, see here: http://de.wikipedia.org/wiki/Normalverteilung#Polar-Methode ; faster than USEBOXMULLER
		long double q, u1, u2;
		do {
			u1 = 2.l* uniform_01<long double>() - 1.l;
			u2 = 2.l* uniform_01<long double>() - 1.l;
			q = u1*u1 + u2*u2;
		} while (q > 1.0l);
		q = GSqrt((-2.l*GLog(q))/q);
		ldblGaussCache_ = u2 * q;
		ldblGaussCacheAvailable_ = true;
		return u1 * q;
#endif
	}
}

/****************************************************************************/

} /* namespace Hap */
} /* namespace Gem */

#endif /* GRANDOMBASE_HPP_ */
