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
#include "GThreadGroup.hpp"
#include "GSingletonT.hpp"
#include "GenevaExceptions.hpp"

/****************************************************************************/

namespace Gem {
namespace Util {

/****************************************************************************/
// Some constants needed for the random number generation

const std::size_t DEFAULTARRAYSIZE = 10000; ///< Default size of the random number array
const std::size_t DEFAULTFACTORYBUFFERSIZE = 400; ///< Default size of the underlying buffer
const boost::uint16_t DEFAULTFACTORYPUTWAIT = 10; ///< waiting time in milliseconds
const boost::uint16_t DEFAULTFACTORYGETWAIT = 10; ///< waiting time in milliseconds

/****************************************************************************/
/**
 * The number of threads that simultaneously produce [0,1[ random numbers
 */
const boost::uint16_t DEFAULT01PRODUCERTHREADS = 4;

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
	GRandomFactory();
	/** @brief The destructor */
	~GRandomFactory();

	/** @brief Sets the number of producer threads for this factory. */
	void setNProducerThreads(const boost::uint16_t&);
	/** @brief Delivers a new [0,1[ random number container to clients */
	boost::shared_array<double> new01Container();

	/** @brief Calculation of a seed for the random numbers */
	static boost::uint32_t GSeed();

	/*************************************************************************/

private:
	GRandomFactory(const GRandomFactory&); ///< Intentionally left undefined
	const GRandomFactory& operator=(const GRandomFactory&);  ///< Intentionally left undefined

	/** @brief Starts the threads needed for the production of random numbers */
	void startProducerThreads();
	/** @brief The production of [0,1[ random numbers takes place here */
	void producer01(const boost::uint32_t& seed);

	boost::uint32_t seed_; ///< The seed for the random number generators
	boost::uint16_t n01Threads_; ///< The number of threads used to produce [0,1[ random numbers
	GThreadGroup producer_threads_01_; ///< A thread group that holds [0,1[ producer threads

	/** @brief A bounded buffer holding the [0,1[ random number packages */
	boost::shared_ptr<Gem::Util::GBoundedBufferT<boost::shared_array<double> > > g01_; // Note: Absolutely needs to be defined after the thread group !!!

	static boost::uint16_t multiple_call_trap_; ///< Trap to catch multiple instantiations of this class
	static boost::mutex thread_creation_mutex_; ///< Synchronization of access tp multiple_call_trap and thread creation
};

} /* namespace Util */
} /* namespace Gem */

/****************************************************************************/
/**
 * A single, global random number factory is created as a singleton.
 */
typedef Gem::Util::GSingletonT<Gem::Util::GRandomFactory> grfactory;
#define GRANDOMFACTORY grfactory::getInstance()

#endif /* GRANDOMFACTORY_HPP_ */

