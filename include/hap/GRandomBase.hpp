/**
 * @file GRandomBase.hpp
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt)
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) com
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
#include <algorithm>

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
	/***************************************************************************/
	/** @brief Helps to use this object as a generator for boost's PRNR distributions */
	typedef double result_type;

	/***************************************************************************/
	/** @brief The standard constructor */
	GRandomBase();
	/** @brief A standard destructor */
	virtual ~GRandomBase();

	/** @brief Retrieves an uniform_01 item */
	result_type operator()();
	/** @brief Returns the minimum value returned by evenRandom() */
	result_type min() const;
	/** @brief Returns the maximum value returned by evenRandom() */
	result_type max() const;

	/***************************************************************************/
	/**
	 * Returns the precision of the values returned by operator(). This
	 * makes it possible to use GRandomBase as a generator for Boost's random
	 * distributions.
	 *
	 * @return The precision of the values returned by GRandomBase::operator()
	 */
	static std::size_t precision() {
		return std::numeric_limits<GRandomBase::result_type>::digits;
	}

	/***************************************************************************/

	/** @brief This function returns true with a probability "probability", otherwise false */
	bool weighted_bool(const double&);
	/** @brief This function produces boolean values with a 50% likelihood each for true and false */
	bool uniform_bool();

	/***************************************************************************/
	/** @brief Uniformly distributed random numbers in the range [0,1[ */
	template <typename fp_type>
	fp_type uniform_01(
			typename boost::enable_if<boost::is_floating_point<fp_type> >::type* dummy = 0
	) {
		return static_cast<fp_type>(dbl_random01());
	}

	/***************************************************************************/
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

	/***************************************************************************/
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
	template <typename fp_type>
	fp_type normal_distribution(
			const fp_type& sigma
		  , typename boost::enable_if<boost::is_floating_point<fp_type> >::type* dummy = 0
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
	template <typename fp_type>
	fp_type normal_distribution(
			const fp_type& mean
		  , const fp_type& sigma
		  , typename boost::enable_if<boost::is_floating_point<fp_type> >::type* dummy = 0
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
	template <typename fp_type>
	fp_type bi_normal_distribution(
			const fp_type& mean
		  , const fp_type& sigma
		  , const fp_type& distance
		  , typename boost::enable_if<boost::is_floating_point<fp_type> >::type* dummy = 0
	) {
		if (uniform_bool()) {
			return normal_distribution<fp_type>(mean - Gem::Common::GFabs(distance / 2.), sigma);
		}
		else {
			return normal_distribution<fp_type>(mean + Gem::Common::GFabs(distance / 2.), sigma);
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
	template <typename fp_type>
	fp_type bi_normal_distribution(
			const fp_type& mean
		  , const fp_type& sigma1
		  , const fp_type& sigma2
		  , const fp_type& distance
		  , typename boost::enable_if<boost::is_floating_point<fp_type> >::type* dummy = 0
	) {
		if (uniform_bool()) {
			return normal_distribution<fp_type>(mean - Gem::Common::GFabs(distance / 2.), sigma1);
		}
		else {
			return normal_distribution<fp_type>(mean + Gem::Common::GFabs(distance / 2.), sigma2);
		}
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
		// random numbers up to and including its upper limit. Note that ui is a distribution
		// only. The actual generator is provided by this class (see variate_generator).
		boost::uniform_int<int_type> ui(min, max);

		// A generator that binds together our own random number generator and a uniform_int distribution
		boost::variate_generator<Gem::Hap::GRandomBase&, boost::uniform_int<int_type> > boost_uniform_int(*this, ui);

		return boost_uniform_int();
	}

	/***************************************************************************/
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
		return this->uniform_int<int_type>(0, max);
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
	template <typename int_type>
	int_type uniform_smallint (
			  const int_type& min
			, const int_type& max
			, typename boost::enable_if<boost::is_integral<int_type> >::type* dummy = 0
	) {
#ifdef DEBUG
		assert(max >= min);
#endif /* DEBUG */

		// A uniform distribution in the desired range. Note that boost::uniform_int produces
		// random numbers up to and including its upper limit. Note that ui is a distribution
		// only. The actual generator is provided by this class (see variate_generator).
		boost::uniform_smallint<int_type> ui(min, max);

		// A generator that binds together our own random number generator and a uniform_smallint distribution
		boost::variate_generator<Gem::Hap::GRandomBase&, boost::uniform_smallint<int_type> > boost_uniform_smallint(*this, ui);

		return boost_uniform_smallint();
	}

	/***************************************************************************/
	/**
	 * This function produces integer random numbers in the range of [0, max] .
	 * The size of the integers is assumed to be small compared to int_type's value
	 * range.
	 *
	 * @param max The maximum (excluded) value of the range
	 * @return Discrete random numbers evenly distributed in the range [0,max]
	 */
	template <typename int_type>
	int_type uniform_smallint (
			  const int_type& max
			, typename boost::enable_if<boost::is_integral<int_type> >::type* dummy = 0
	) {
#ifdef DEBUG
		assert(max >= 0);
#endif /* DEBUG */

		return this->uniform_smallint<int_type>(0, max);
	}

protected:
	/***************************************************************************/
	/** @brief Uniformly distributed double random numbers in the range [0,1[ */
	virtual double dbl_random01() = 0;

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

/** @brief  Produces gaussian-distributed float random numbers with sigma 1 and mean 0 */
template<> float GRandomBase::normal_distribution<float>();
/** @brief Produces gaussian-distributed double random numbers with sigma 1 and mean 0 */
template<> double GRandomBase::normal_distribution<double>();
/** @brief Avoid the cast for native double type */
template <> double GRandomBase::uniform_01<double>(boost::enable_if<boost::is_floating_point<double> >::type*);

/******************************************************************************/

} /* namespace Hap */
} /* namespace Gem */

#endif /* GRANDOMBASE_HPP_ */
