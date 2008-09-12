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

#if BOOST_VERSION < 103600
#error "Error: Boost should at least have version 1.36 !"
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
#include "GLogger.hpp"
#include "GThreadGroup.hpp"
#include "GRandomFactory.hpp"

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
	:boost::noncopyable {
public:
	/** @brief The standard constructor */
	GRandom() throw();

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
	/** @brief Produces integer random numbers in the range of [0, max[ */
	boost::uint16_t discreteRandom(const boost::uint16_t&);
	/** @brief Produces integer random numbers in the range of [min, max[ */
	boost::int16_t discreteRandom(const boost::int16_t&, const boost::int16_t&);
	/** @brief Produces boolean values with a 50% likelihood each for true and false. */
	GenEvA::bit bitRandom();
	/** @brief Returns true with a given probability, otherwise false. */
	GenEvA::bit bitRandom(const double&);
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
	double *p_raw; ///< A pointer to the content of p01_ for faster access
};

/****************************************************************************/

} /* namespace Util */
} /* namespace Gem */

#endif /* GRANDOM_HPP_ */
