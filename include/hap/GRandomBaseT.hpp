/**
 * @file GRandomBaseT.hpp
 */

/* Copyright (C) Dr. Ruediger Berlich and Karlsruhe Institute of Technology
 * (University of the State of Baden-Wuerttemberg and National Laboratory
 * of the Helmholtz Association)
 *
 * Contact: info [at] gemfony (dot) com
 *
 * This file is part of the Hap library, Gemfony scientific's library of
 * random number routines.
 *
 * Hap is free software: you can redistribute it and/or modify
 * it under the terms of version 3 of the GNU Affero General Public License
 * as published by the Free Software Foundation.
 *
 * Hap is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with the Hap library.  If not, see <http://www.gnu.org/licenses/>.
 *
 * For further information on Gemfony scientific and Hap, visit
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

#ifndef GRANDOMBASET_HPP_
#define GRANDOMBASET_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif


// Hap headers go here
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
 *
 * TODO: Change value ranges of uniform_*int in all of Geneva so that the upper boundary is inclusive
 */
template <
	  typename fp_type = double
	, typename int_type = boost::int32_t
>
class GRandomBaseT
	: private boost::noncopyable
{
	BOOST_CONCEPT_ASSERT((boost::SignedInteger<int_type>));

public:
	/****************************************************************************/
	/** @brief Helps to use this object as a generator for boost's PRNR distributions */
	typedef fp_type result_type;
	/** @brief The minimum value returned by evenRandom() */
	const result_type min_value;
	/** @brief The maximum value returned by evenRandom() */
	const result_type max_value;

	/************************************************************************/
	/** @brief The standard constructor */
	GRandomBaseT()
		: min_value(0.)
		, max_value(1.)
		, initialSeed_(GRANDOMFACTORY->getSeed())
		, gaussCache_(0.)
	    , gaussCacheAvailable_(false)
	{ /* nothing */ }

	/************************************************************************/
	/** @brief Initialization by seed */
	explicit GRandomBaseT(const seed_type& initialSeed)
		: min_value(0.)
		, max_value(1.)
		, initialSeed_(initialSeed)
		, gaussCache_(0.)
	    , gaussCacheAvailable_(false)
	{ /* nothing */ }

	/************************************************************************/
	/** @brief A standard destructor */
	virtual ~GRandomBaseT()
	{ /* nothing */ }

	/************************************************************************/
	/** @brief Production of uniformly distributed floating point numbers in [0,1[ */
	virtual fp_type uniform_01() = 0;

	/************************************************************************/
	/**
	 * Retrieves an uniform_01 item. Thus function, together with the min() and
	 * max() functions make it possible to use GRandomBase as a generator for
	 * boost's random distributions.
	 *
	 * @return A random number taken from the evenRandom function
	 */
	result_type operator()() {
		return (result_type)uniform_01();
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
	fp_type uniform_real(
		  const fp_type& max
		, typename boost::enable_if<boost::is_floating_point<fp_type> >::type* dummy = 0
	) {
#ifdef DEBUG
		// Check that min and max have appropriate values
		assert(max>=0.);
#endif
		return uniform_01() * max;
	}

	/************************************************************************/
	/**
	 * Produces evenly distributed random numbers in the range [min,max[
	 *
	 * @param min The minimum value of the range
	 * @param max The maximum (excluded) value of the range
	 * @return Random numbers evenly distributed in the range [min,max[
	 */
	fp_type uniform_real(
			const fp_type& min
		  , const fp_type& max
		  , typename boost::enable_if<boost::is_floating_point<fp_type> >::type* dummy = 0
	) {
#ifdef DEBUG
		// Check that min and max have appropriate values
		assert(min<=max);
#endif
		return uniform_01() * (max - min) + min;
	}

	/************************************************************************/
	/**
	 * Produces gaussian-distributed random numbers with sigma 1 and mean 0
	 *
	 * TODO: This function needs to be specialized for long double and float types
	 *
	 * @return floating point random numbers with a gaussian distribution
	 */
	fp_type normal_distribution(
			typename boost::enable_if<boost::is_floating_point<fp_type> >::type* dummy = 0
	) {
		if(gaussCacheAvailable_) {
			gaussCacheAvailable_ = false;
			return gaussCache_;
		}
		else {
#ifdef USEBOXMULLER
			fp_type rnr1 = uniform_01();
			fp_type rnr2 = uniform_01();
			gaussCache_ = sqrt(fabs(-2. * std::log(1. - rnr1))) * cos(2. * M_PI	* rnr2);
			gaussCacheAvailable_ = true;
			return sqrt(fabs(-2. * std::log(1. - rnr1))) * sin(2. * M_PI	* rnr2);
#else // USEBOXMULLERPOLAR, see here: http://de.wikipedia.org/wiki/Normalverteilung#Polar-Methode ; faster than USEBOXMULLER
			fp_type q, u1, u2;
	        do {
	        	u1 = 2.* uniform_01() - 1.;
	        	u2 = 2.* uniform_01() - 1.;
	        	q = u1*u1 + u2*u2;
	        } while (q > 1.0);
	        q = sqrt((-2.*std::log(q))/q);
	        gaussCache_ = u2 * q;
	        gaussCacheAvailable_ = true;
			return u1 * q;
#endif
		}
	}

	/************************************************************************/
	/**
	 * Produces gaussian-distributed random numbers with mean 0 and sigma "sigma"
	 *
	 * @param mean The mean value of the Gaussian
	 * @param sigma The sigma of the Gaussian
	 * @return floating point random numbers with a gaussian distribution
	 */
	fp_type normal_distribution(
			const fp_type& sigma
		  , typename boost::enable_if<boost::is_floating_point<fp_type> >::type* dummy = 0
	) {
		return sigma * normal_distribution();
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
	fp_type normal_distribution(
			const fp_type& mean
		  , const fp_type& sigma
		  , typename boost::enable_if<boost::is_floating_point<fp_type> >::type* dummy = 0
	) {
		return sigma * normal_distribution() + mean;
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
	fp_type bi_normal_distribution(
			const fp_type& mean
		  , const fp_type& sigma
		  , const fp_type& distance
		  , typename boost::enable_if<boost::is_floating_point<fp_type> >::type* dummy = 0
	) {
		if (uniform_bool()) {
			return normal_distribution(mean - fabs(distance / 2.), sigma);
		}
		else {
			return normal_distribution(mean + fabs(distance / 2.), sigma);
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
			const fp_type& probability
		  , typename boost::enable_if<boost::is_floating_point<fp_type> >::type* dummy = 0
	) {
	#ifdef DEBUG
		assert(probability>=0. && probability<=1.);
	#endif
		return ((uniform_01()<probability)?true:false);
	}

	/************************************************************************/
	/**
	 * This function produces boolean values with a 50% likelihood each for
	 * true and false.
	 *
	 * @return Boolean values with a 50% likelihood for true/false respectively
	 */
	bool uniform_bool() {
		return weighted_bool(fp_type(0.5));
	}

	/*************************************************************************/
	/**
	 * This function produces integer random numbers in the range of [min, max] .
	 * Note that max may also be < 0. .
	 *
	 * @param min The minimum value of the range
	 * @param max The maximum (excluded) value of the range
	 * @return Discrete random numbers evenly distributed in the range [min,max[
	 */
	int_type uniform_int (
			  const int_type& min
			, const int_type& max
			, typename boost::enable_if<boost::is_integral<int_type> >::type* dummy = 0
	) {
#ifdef DEBUG
		assert(max > min);
#endif /* DEBUG */

		// A uniform distribution in the desired range. Note that boost::uniform_int produces
		// random numbers up to and including its upper limit.
		boost::uniform_int<int_type> ui(min, max-1);

		// A generator that binds together our own random number generator and a uniform_int distribution
		boost::variate_generator<Gem::Hap::GRandomBaseT<fp_type, int_type>&, boost::uniform_int<int_type> > boost_uniform_int(*this, ui);

		return boost_uniform_int();
	}

	/****************************************************************************/
	/**
	 * This function produces integer random numbers in the range of [0, max] .
	 *
	 * @param max The maximum (excluded) value of the range
	 * @return Discrete random numbers evenly distributed in the range [0,max[
	 */
	int_type uniform_int (
			  const int_type& max
			, typename boost::enable_if<boost::is_integral<int_type> >::type* dummy = 0
	) {
		return uniform_int(0, max);
	}

	/*************************************************************************/
	/**
	 * This function produces integer random numbers in the range of [min, max] .
	 * Note that max may also be < 0. . The size of the integers is assumed to be
	 * small compared to int_type's value range.
	 *
	 * @param min The minimum value of the range
	 * @param max The maximum (excluded) value of the range
	 * @return Discrete random numbers evenly distributed in the range [min,max]
	 */
	int_type uniform_smallint (
			  const int_type& min
			, const int_type& max
			, typename boost::enable_if<boost::is_integral<int_type> >::type* dummy = 0
	) {
#ifdef DEBUG
		assert(max > min);
#endif /* DEBUG */

		// A uniform distribution in the desired range. Note that boost::uniform_int produces
		// random numbers up to and including its upper limit.
		boost::uniform_smallint<int_type> ui(min, max-1);

		// A generator that binds together our own random number generator and a uniform_int distribution
		boost::variate_generator<Gem::Hap::GRandomBaseT<fp_type, int_type>&, boost::uniform_smallint<int_type> > boost_uniform_smallint(*this, ui);

		return boost_uniform_smallint();
	}

	/****************************************************************************/
	/**
	 * This function produces integer random numbers in the range of [0, max] .
	 * The size of the integers is assumed to be small compared to int_type's value
	 * range.
	 *
	 * @param max The maximum (excluded) value of the range
	 * @return Discrete random numbers evenly distributed in the range [0,max]
	 */
	int_type uniform_smallint (
			  const int_type& max
			, typename boost::enable_if<boost::is_integral<int_type> >::type* dummy = 0
	) {
		return uniform_smallint(0, max);
	}


protected:
	/************************************************************************/
	/** @brief Holds the initial seed used in a local random number generator */
	seed_type initialSeed_;

private:
	/************************************************************************/
	/** @brief Two gaussian random numbers are produced in one go. One number can be cached here */
	fp_type gaussCache_;
	/** @brief Specifies whether a valid cached gaussian is available */
	bool gaussCacheAvailable_;
};

/****************************************************************************/

} /* namespace Hap */
} /* namespace Gem */

#endif /* GRANDOMBASET_HPP_ */
