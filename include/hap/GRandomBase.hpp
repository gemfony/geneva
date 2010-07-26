/**
 * @file GRandom.hpp
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


#ifndef GRANDOMBASE_HPP_
#define GRANDOMBASE_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif


// Hap headers go here

/****************************************************************************/

namespace Gem {
namespace Hap {

/****************************************************************************/
/**
 * This class encapsulates the functions needed to create various different
 * types of random numbers (such as a gaussian distribution as the most important
 * use case. Ways of creating the "raw" input for these distributions -- double
 * values -- are defined in derived classes. GRandomBase is thus a purely virtual
 * class and cannot be instantiated.
 */
class GRandomBase
{
public:
	/****************************************************************************/
    /** @brief Helps to use this object as a generator for boost's PRNR distributions */
    typedef double result_type;
    /** @brief The minimum value returned by evenRandom() */
    const double min_value;
    /** @brief The maximum value returned by evenRandom() */
    const double max_value;

    /****************************************************************************/
	/** @brief The standard constructor */
	GRandomBase();
	/** @brief A standard destructor */
	virtual ~GRandomBase();

	/** @brief Emits a double value in the range [0,1[ */
	virtual double fpUniform() = 0;

	/** @brief Retrieves an fpUniform item */
	double GRandomBase::operator()();
	/** @brief Retrieves the minimum value returned by operator() */
	double min() const;
	/** @brief Retrieves the maximum value returned by operator() */
	double max() const;

	/** @brief Emits evenly distributed random numbers in the range [0,max[ */
	double fpUniform(const double&);
	/** @brief Produces evenly distributed random numbers in the range [min,max[ */
	double fpUniform(const double&, const double&);

	/** @brief Produces gaussian-distributed random numbers */
	double fpGaussian(const double&, const double&);
	/** @brief Distribution comprising two gaussians with defined distance */
	double fpDoubleGaussian(const double&, const double&, const double&);

	/****************************************************************************/
	/**
	 * This function produces integer random numbers in the range of [0, max[ .
	 *
	 * @param max The maximum (excluded) value of the range
	 * @return Discrete random numbers evenly distributed in the range [0,max[
	 */
	template <typename int_type>
	int_type intUniform (
			  const int_type& max
			, typename boost::enable_if<boost::is_integral<int_type> >::type* dummy = 0
	) {
	#ifdef DEBUG
		int_type result = boost::numeric_cast<int_type> (evenRandom(boost::numeric_cast<double> (max)));
		assert(result<max);
		return result;
	#else
		return static_cast<int_type>(evenRandom(static_cast<double>(max)));
	#endif
	}

	/*************************************************************************/
	/**
	 * This function produces integer random numbers in the range of [min, max[ .
	 * Note that max may also be < 0. .
	 *
	 * @param min The minimum value of the range
	 * @param max The maximum (excluded) value of the range
	 * @return Discrete random numbers evenly distributed in the range [min,max[
	 */
	template <typename int_type>
	int_type intUniform (
			  const int_type& min
			, const int_type& max
			, typename boost::enable_if<boost::is_integral<int_type> >::type* dummy = 0
	) {
	#ifdef DEBUG
		assert(min < max);
		int_type result = boost::numeric_cast<int_type>(discreteRandom(max - min) + min);
		assert(result>=min && result<max);
		return result;
	#else
		return discreteRandom(max - min) + min;
	#endif
	}

	/*************************************************************************/

	/** @brief Produces bool values with a 50% likelihood each for true and false. */
	bool boolUniform();
	/** @brief Returns true with a given probability, otherwise false. */
	bool boolWeighted(const double&);

	/** @brief Produces random ASCII characters */
	char charUniform(const bool& printable = true);

private:
	/** @brief Two gaussian random numbers are produced in one go. One number can be cached here */
	double gaussCache_;
	/** @brief Specifies whether a valid cached gaussian is available */
	bool gaussCacheAvailable_;
};

} /* namespace Hap */
} /* namespace Gem */

/****************************************************************************/

#endif /* GRANDOMBASE_HPP_ */
