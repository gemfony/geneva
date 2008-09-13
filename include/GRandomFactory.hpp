/**
 * @file GRandomFactory.hpp
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

#ifndef GRANDOMFACTORY_HPP_
#define GRANDOMFACTORY_HPP_

// GenEvA headers go here

#include "GBoundedBufferT.hpp"
#include "GEnums.hpp"
#include "GLogger.hpp"
#include "GThreadGroup.hpp"

/****************************************************************************/

namespace Gem {
namespace Util {

/****************************************************************************/
// Some constants needed for the random number generation

const std::size_t DEFAULTARRAYSIZE = 1000; ///< Default size of the random number array
const std::size_t DEFAULTFACTORYBUFFERSIZE = 1000; ///< Default size of the underlying buffer
const boost::uint16_t DEFAULTFACTORYPUTWAIT = 10; ///< waiting time in milliseconds
const boost::uint16_t DEFAULTFACTORYGETWAIT = 10; ///< waiting time in milliseconds

/****************************************************************************/
/**
 * The number of threads that simultaneously produce [0,1[ random numbers
 */
const boost::uint16_t DEFAULT01PRODUCERTHREADS = 4;

/****************************************************************************/
/**
 * Synchronization of access to boost::date_time functions.
 */
boost::mutex randomseed_mutex;

/****************************************************************************/
/**
 * Past implementations of random numbers for the Geneva library showed a
 * particular bottle neck in the random number generation. Every GObject
 * had its own random number generator, and seeding was very expensive.
 * We thus now produce floating point numbers in the range [0,1[ in a separate
 * thread in this class and calculate other numbers from this in the GRandom class.
 * A second thread is responsible for the creation of gaussian random numbers.
 * This circumvents the necessity to seed the generator over and over again and
 * allows us to get rid of a dependency on the MersenneTwister library. We are now
 * using a generator from the boost library instead, so users need to download fewer
 * libraries to use the GenEvA library.
 *
 * This class produces packets of random numbers and stores them in bounded buffers.
 * Clients can retrieve packets of random numbers, while a separate thread keeps
 * filling the buffer up.
 *
 * The implementation currently uses the lagged fibonacci generator. According to
 * http://www.boost.org/doc/libs/1_35_0/libs/random/random-performance.html this is
 * the fastest generator amongst all of Boost's generators. It is the author's belief that
 * the "quality" of random numbers is of less concern in evolutionary algorithms, as the
 * geometry of the quality surface adds to the randomness.
 */
class GRandomFactory {
public:
	/** @brief The default constructor */
	GRandomFactory() throw();
	/** @brief A constructor that creates a user-specified number of producer threads*/
	GRandomFactory(const boost::uint16_t&) throw();
	/** @brief The destructor */
	~GRandomFactory() throw();

	/** @brief Sets the number of producer threads for this factory. */
	void setNProducerThreads(const boost::uint16_t&);
	/** @brief Delivers a new [0,1[ random number container to clients */
	boost::shared_array<double> new01Container();

	/*************************************************************************/
	/**
	 * This static function returns a seed based on the current time.
	 *
	 * Comments on some boost mailing lists (late 2005) seem to indicate that
	 * the date_time library's functions are not re-entrant when using gcc and
	 * its libraries. It was not possible to determine whether this is still
	 * the case, hence we protect calls to date_time with a mutex.
	 *
	 * @return A seed based on the current time
	 */
	static boost::uint32_t GSeed(){
		boost::mutex::scoped_lock lk(randomseed_mutex);
		boost::posix_time::ptime t1 = boost::posix_time::microsec_clock::local_time();
	    return (uint32_t)t1.time_of_day().total_milliseconds();
	}

	/*************************************************************************/

private:
	/** @brief Starts the threads needed for the production of random numbers */
	void startProducerThreads() throw();
	/** @brief The production of [0,1[ random numbers takes place here */
	void producer01(const boost::uint32_t& seed) throw();

	/** @brief A bounded buffer holding the [0,1[ random number packages */
	Gem::Util::GBoundedBufferT<boost::shared_array<double> > g01_;

	boost::uint32_t seed_; ///< The seed for the random number generators
	boost::uint16_t n01Threads_; ///< The number of threads used to produce [0,1[ random numbers
	GThreadGroup producer_threads_01_; ///< A thread group that holds [0,1[ producer threads
};

} /* namespace Util */
} /* namespace Gem */

#endif /* GRANDOM_HPP_ */

/**
 * A single, global random number factory is created as a singleton.
 */
typedef boost::details::pool::singleton_default<Gem::Util::GRandomFactory> grfactory;
#define GRANDOMFACTORY grfactory::instance() ///< Define that makes the singleton more easily accessible
