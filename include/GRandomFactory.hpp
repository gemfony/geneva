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
#include <fstream>
#include <cassert>

// Includes check for correct Boost version(s)
#include "GGlobalDefines.hpp"

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
#include <boost/exception.hpp>
#include <boost/cstdint.hpp>
#include <boost/limits.hpp>
#include <boost/filesystem.hpp>

/**
 * Check that we have support for threads. This collection of classes is useless
 * without this.
 */
#ifndef BOOST_HAS_THREADS
#error "Error: No support for Boost.threads available."
#endif

#ifndef GRANDOMFACTORY_HPP_
#define GRANDOMFACTORY_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif


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

const std::size_t DEFAULTARRAYSIZE = 1000; ///< Default size of the random number array
const std::size_t DEFAULTFACTORYBUFFERSIZE = 200; ///< Default size of the underlying buffer
const boost::uint16_t DEFAULTFACTORYPUTWAIT = 5; ///< waiting time in milliseconds
const boost::uint16_t DEFAULTFACTORYGETWAIT = 5; ///< waiting time in milliseconds
const boost::uint32_t DEFAULTSEED = 1234; ///< The starting value of the global seed

/****************************************************************************/
/**
 * The number of threads that simultaneously produce [0,1[ random numbers
 */
const boost::uint16_t DEFAULT01PRODUCERTHREADS = 4;

/****************************************************************************/
/**
 * Increment of the global seed
 */
const boost::uint32_t GLOBALSEEDINCREMENT = 3;

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
	/** @brief Delivers a new [0,1[ random number container with the current standard size to clients */
	boost::shared_array<double> new01Container();

	/** @brief Allows to set the size of random number arrays */
	void setArraySize(const std::size_t&);
	/** @brief Allows to retrieve the current value of the arraySize_ variable */
	std::size_t getCurrentArraySize() const;

	/** @brief Allows to retrieve the size of the buffer */
	std::size_t getBufferSize() const;

	/** @brief Setting of an initial seed for random number generators */
	bool setSeed(const boost::uint32_t&) const;
	/** @brief Retrieval of the current value of the global seed */
	boost::uint32_t getSeed() const;
	/** @brief Checks whether the global seed has already been initialized */
	bool checkSeedIsInitialized() const;

	/** @brief Calculation of a seed for the random numbers */
	static boost::uint32_t GSeed();

	/*************************************************************************/

private:
	GRandomFactory(const GRandomFactory&); ///< Intentionally left undefined
	const GRandomFactory& operator=(const GRandomFactory&);  ///< Intentionally left undefined

	/** @brief Setting of an initial seed for random numbers. Will be incremented for each instantiation */
	static bool setSeed_(const boost::uint32_t&);
	/** @brief Setting of an initial seed for random numbers, as taken from /dev/urandom */
	static bool setSeedURandom_();
	/** @brief Retrieval of the current value of the global seed */
	static  boost::uint32_t getSeed_();

	/** @brief Starts the threads needed for the production of random numbers */
	void startProducerThreads();
	/** @brief The production of [0,1[ random numbers takes place here */
	void producer01(boost::uint32_t seed);

	std::size_t arraySize_;
	bool threadsHaveBeenStarted_;
	boost::uint16_t n01Threads_; ///< The number of threads used to produce [0,1[ random numbers
	GThreadGroup producer_threads_01_; ///< A thread group that holds [0,1[ producer threads

	/** @brief A bounded buffer holding the [0,1[ random number packages */
	boost::shared_ptr<Gem::Util::GBoundedBufferT<boost::shared_array<double> > > g01_; // Note: Absolutely needs to be defined after the thread group !!!

	static boost::uint16_t multiple_call_trap_; ///< Trap to catch multiple instantiations of this class
	static boost::mutex factory_creation_mutex_; ///< Synchronization of access to multiple_call_trap in constructor

	boost::mutex thread_creation_mutex_; ///< Synchronization of access to the threadsHaveBeenStarted_ variable
	mutable boost::mutex arraySizeMutex_; ///< Regulates access to the arraySize_ variable
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

