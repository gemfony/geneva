/**
 * @file GRandom.hpp
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

#include <cstdlib>
#include <iomanip>
#include <ctime>
#include <cmath>
#include <iostream>
#include <sstream>
#include <cassert>

// Boost headers go here

#include <boost/version.hpp>
#include "GGlobalDefines.hpp"
#if BOOST_VERSION < ALLOWED_BOOST_VERSION
#error "Error: Boost has incorrect version !"
#endif /* BOOST_VERSION */

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
#include <boost/exception.hpp>
#include <boost/cstdint.hpp>
#include <boost/cast.hpp>

/**
 * Check that we have support for threads. This collection of classes is useless
 * without this.
 */
#ifndef BOOST_HAS_THREADS
#error "Error: No support for Boost.threads available."
#endif

#ifndef GRANDOM_HPP_
#define GRANDOM_HPP_

// GenEvA headers go here

#include "GBoundedBufferT.hpp"
#include "GEnums.hpp"
#include "GThreadGroup.hpp"
#include "GRandomFactory.hpp"
#include "GenevaExceptions.hpp"

/****************************************************************************/

namespace Gem {
namespace Util {

/****************************************************************************/
/**
 * This class gives objects access to random numbers. It internally handles
 * retrieval of random numbers from the GRandomFactory class as needed. Random
 * distributions are calculated on the fly from these numbers. Usage is thus
 * transparent to the user.
 */
class GRandom
	:boost::noncopyable
{
public:
	/** @brief The standard constructor */
	GRandom() throw();
	/** @brief A standard destructor */
	~GRandom();

	/** @brief Emits evenly distributed random numbers in the range [0,1[ */
	double evenRandom();
	/** @brief Emits evenly distributed random numbers in the range [0,max[ */
	double evenRandom(const double&);
	/** @brief Produces evenly distributed random numbers in the range [min,max[ */
	double evenRandom(const double&, const double&);
	/** @brief Produces gaussian-distributed random numbers */
	double gaussRandom(const double&, const double&);
	/** @brief Produces two gaussians with defined distance */
	double doubleGaussRandom(const double&, const double&, const double&);

	/*************************************************************************/
	/**
	 * This function produces integer random numbers in the range of [0, max[ .
	 *
	 * @param max The maximum (excluded) value of the range
	 * @return Discrete random numbers evenly distributed in the range [0,max[
	 */
	template <typename int_type>
	int_type discreteRandom(const int_type& max) {
	#ifdef DEBUG
		int_type result = boost::numeric_cast<int_type> (this->evenRandom(boost::numeric_cast<double> (max)));
		assert(result<max);
		return result;
	#else
		return static_cast<int_type>(this->evenRandom(static_cast<double>(max)));
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
	int_type discreteRandom(const int_type& min, const int_type& max) {
	#ifdef DEBUG
		assert(min < max);
		int_type result = discreteRandom(max - min) + min;
		assert(result>=min && result<max);
		return result;
	#else
		return discreteRandom(max - min) + min;
	#endif
	}

	/*************************************************************************/

	/** @brief Produces bool values with a 50% likelihood each for true and false. */
	bool boolRandom();
	/** @brief Returns true with a given probability, otherwise false. */
	bool boolRandom(const double&);
	/** @brief Produces random ASCII characters */
	char charRandom(const bool& printable = true);

private:
	GRandom(const GRandom&); ///< Intentionally left undefined
	GRandom& operator=(const GRandom&); ///< Intentionally left undefined

	/** @brief Fills a random container if none could be retrieved from the factory */
	void fillContainer01();
	/** @brief (Re-)Initialization of p01_ */
	void getNewP01();

	boost::shared_array<double> p01_; ///< Holds the container of [0,1[ random numbers
	std::size_t current01_; ///< The current position in p01_
	double *p_raw_; ///< A pointer to the content of p01_ for faster access

	boost::shared_ptr<Gem::Util::GRandomFactory> grf_; ///< A local copy of the global GRandomFactory
};

/****************************************************************************/

template <> double GRandom::discreteRandom(const double&); ///< Specialization for double
template <>	double GRandom::discreteRandom(const double&, const double&); ///< Specialization for double
template <> float GRandom::discreteRandom(const float&); ///< Specialization for float
template <>	float GRandom::discreteRandom(const float&, const float&); ///< Specialization for float
template <> bool GRandom::discreteRandom(const bool&); ///< Specialization for bool
template <>	bool GRandom::discreteRandom(const bool&, const bool&); ///< Specialization for bool

} /* namespace Util */
} /* namespace Gem */

#endif /* GRANDOM_HPP_ */
