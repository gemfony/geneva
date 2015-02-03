/**
 * @file GRandomFactory.hpp
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

// For Microsoft-compatible compilers
#include "common/GWindowsDefines.hpp"

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
#include <boost/limits.hpp>
#include <boost/filesystem.hpp>
#include <boost/cast.hpp>

/**
 * Check that we have support for threads. This collection of classes is useless
 * without this.
 */
#ifndef BOOST_HAS_THREADS
#error "Error: No support for Boost.threads available."
#endif

#ifndef GRANDOMFACTORY_HPP_
#define GRANDOMFACTORY_HPP_


// Geneva headers go here

#include "common/GBoundedBufferT.hpp"
#include "common/GExceptions.hpp"
#include "common/GSingletonT.hpp"
#include "common/GThreadGroup.hpp"
#include "common/GHelperFunctions.hpp"
#include "hap/GSeedManager.hpp"

/******************************************************************************/

namespace Gem {
namespace Hap {

/******************************************************************************/
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
 * libraries to use the Geneva library.
 *
 * This class produces packets of random numbers and stores them in bounded buffers.
 * Clients can retrieve packets of random numbers, while separate threads keep
 * filling the buffer up.
 *
 * The implementation currently uses the lagged fibonacci generator. According to
 * http://www.boost.org/doc/libs/1_35_0/libs/random/random-performance.html this is
 * the fastest generator amongst all of Boost's generators. It is the author's belief that
 * the "quality" of random numbers is of less concern in evolutionary algorithms, as the
 * geometry of the quality surface adds to the randomness.
 */
class GRandomFactory {
	typedef boost::lagged_fibonacci19937 lagged_fibonacci;

public:
	/** @brief The default constructor */
	GRandomFactory();
	/** @brief The destructor */
	~GRandomFactory();

	/** @brief Initialization code for the GRandomFactory */
	void init();
	/** @brief Finalization code for the GRandomFactory */
	void finalize();

	/** @brief Sets the number of producer threads for this factory. */
	void setNProducerThreads(const boost::uint16_t&);
	/** @brief Delivers a new [0,1[ random number container with the current standard size to clients */
	boost::shared_array<double> new01Container();

	/** @brief Allows to retrieve the size of the array */
	std::size_t getCurrentArraySize() const;

	/** @brief Allows to retrieve the size of the buffer */
	std::size_t getBufferSize() const;

	/** @brief Setting of an initial seed for random number generators */
	bool setStartSeed(const initial_seed_type&);
	/** @brief Retrieval of the start-value of the global seed */
	initial_seed_type getStartSeed() const;
	/** @brief Checks whether seeding has already started*/
	bool checkSeedingIsInitialized() const;

	/** @brief Retrieval of a new seed for external or internal random number generators */
	seed_type getSeed();

	/** @brief Allows to retrieve the size of the seeding queue */
	std::size_t getSeedingQueueSize() const;

private:
	/***************************************************************************/

	GRandomFactory(const GRandomFactory&); ///< Intentionally left undefined
	const GRandomFactory& operator=(const GRandomFactory&);  ///< Intentionally left undefined

	/** @brief Starts the threads needed for the production of random numbers */
	void startProducerThreads();
	/** @brief The production of [0,1[ random numbers takes place here */
	void producer01(boost::uint32_t seed);

	bool finalized_;
	bool threadsHaveBeenStarted_;
	boost::uint16_t n01Threads_; ///< The number of threads used to produce [0,1[ random numbers
	Gem::Common::GThreadGroup producer_threads_01_; ///< A thread group that holds [0,1[ producer threads

	/** @brief A bounded buffer holding the [0,1[ random number packages */
	Gem::Common::GBoundedBufferT<boost::shared_array<double> > g01_; // Note: Absolutely needs to be defined after the thread group !!!

	static boost::uint16_t multiple_call_trap_; ///< Trap to catch multiple instantiations of this class
	static boost::mutex factory_creation_mutex_; ///< Synchronization of access to multiple_call_trap in constructor

	mutable boost::mutex thread_creation_mutex_; ///< Synchronization of access to the threadsHaveBeenStarted_ variable
	mutable boost::shared_mutex arraySizeMutex_; ///< Regulates access to the arraySize_ variable
	mutable boost::mutex seedingMutex_; ///< Regulates start-up of the seeding process

	mutable boost::shared_ptr<GSeedManager> seedManager_ptr_; ///< Manages seed creation
};

} /* namespace Hap */
} /* namespace Gem */

/******************************************************************************/
/**
 * A single, global random number factory is created as a singleton.
 */
#define GRANDOMFACTORY      Gem::Common::GSingletonT<Gem::Hap::GRandomFactory>::Instance(0)
#define RESETGRANDOMFACTORY Gem::Common::GSingletonT<Gem::Hap::GRandomFactory>::Instance(1)


#endif /* GRANDOMFACTORY_HPP_ */
