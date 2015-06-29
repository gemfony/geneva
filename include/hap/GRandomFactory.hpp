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

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard headers go here

#include <cstdlib>
#include <iomanip>
#include <ctime>
#include <cmath>
#include <iostream>
#include <sstream>
#include <fstream>
#include <cassert>
#include <limits>

// Boost headers go here

#include <boost/atomic.hpp>
#include <boost/random.hpp>
#include <boost/date_time.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include <boost/random.hpp>
#include <boost/random/random_device.hpp>
#include <boost/utility.hpp>
#include <boost/lexical_cast.hpp>
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
#include "hap/GRandomDefines.hpp"

/******************************************************************************/

namespace Gem {
namespace Hap {

/**
 * In a redesign of this class we want:
 * - Round-robin creation and retrieval --> Whenever a package is handed out, a thread is submitted to a pool to create a new thread
 * - Recycling of unused random numbers in a packet through memcpy
 * - "Live-resizing of the thread-pool
 */

typedef boost::lagged_fibonacci19937 lagged_fibonacci;

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This struct holds and creates random number containers to be transmitted to GRandomT
 * via a buffer. It does minimal error checking as it is meant for internal usage only,
 * and excessive error checking on the code might have strong performance implications.
 * None of the functions in this class is thread-safe (in the sense of being usable
 * concurrently from multiple threads).
 */
struct random_container
	: private boost::noncopyable {
public:
	/** @brief Initialization with the number of entries in the buffer */
	random_container(const std::size_t &, lagged_fibonacci &);

	/** @brief The destructor -- gets rid of the random buffer r_ */
	~random_container();

	/** @brief Returns the size of the buffer */
	std::size_t size() const;

	/** @brief Returns the current position */
	std::size_t getCurrentPosition() const;

	/** @brief Replaces "used" random numbers */
	void refresh(lagged_fibonacci &);

	/***************************************************************************/
	/**
	 * Allows to check whether the buffer has run empty
	 */
	inline bool empty() {
		return (current_pos_ >= binSize_);
	}

	/***************************************************************************/
	/**
	 * Returns the next double number
	 */
	inline double next() {
#ifdef DEBUG
      if(empty()) {
         glogger
         << "In random_container::next(): Error!" << std::endl
         << "Invalid current_pos_ / binSize_: " << current_pos_ << " / " << binSize_ << std::endl
         << GEXCEPTION;
      }
#endif

		return r_[current_pos_++];
	}

private:
	/***************************************************************************/

	random_container() = delete; ///< The default constructor -- intentionally private and undefined
	random_container(const random_container &) = delete; ///< The copy constructor -- intentionally private and undefined
	const random_container &operator=(const random_container &) = delete; ///< intentionally private and undefined

	std::size_t current_pos_; ///< The current position in the array
	const std::size_t binSize_;     ///< The size of the buffer

	double *r_; ///< Holds the actual random numbers
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
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
class GRandomFactory
	: private boost::noncopyable {
public:
	/** @brief The default constructor */
	G_API_HAP GRandomFactory();
	/** @brief The destructor */
	G_API_HAP ~GRandomFactory();

	/** @brief Initialization code for the GRandomFactory */
	G_API_HAP void init();
	/** @brief Finalization code for the GRandomFactory */
	G_API_HAP void finalize();

	/** @brief Sets the number of producer threads for this factory. */
	G_API_HAP void setNProducerThreads(const std::uint16_t &);

	/** @brief Allows to retrieve the size of the array */
	G_API_HAP std::size_t getCurrentArraySize() const;

	/** @brief Allows to retrieve the size of the buffer */
	G_API_HAP std::size_t getBufferSize() const;

	/** @brief Setting of an initial seed for random number generators */
	G_API_HAP bool setStartSeed(const initial_seed_type &);
	/** @brief Retrieval of the start-value of the global seed */
	G_API_HAP initial_seed_type getStartSeed() const;
	/** @brief Checks whether seeding has already started*/
	G_API_HAP bool checkSeedingIsInitialized() const;

	/** @brief Delivers a new [0,1[ random number container with the current standard size to clients */
	G_API_HAP std::shared_ptr <random_container> new01Container();
	/** @brief Retrieval of a new seed for external or internal random number generators */
	G_API_HAP seed_type getSeed();

	/** @brief Allows recycling of partially used packages */
	G_API_HAP void returnUsedPackage(std::shared_ptr <random_container>);

private:
	/***************************************************************************/
	GRandomFactory(const GRandomFactory &); ///< Intentionally left undefined
	const GRandomFactory &operator=(const GRandomFactory &);  ///< Intentionally left undefined

	/** @brief The production of [0,1[ random numbers takes place here */
	void producer01(std::uint32_t seed);

	bool finalized_;
	boost::atomic<bool> threadsHaveBeenStarted_;
	boost::atomic<std::uint16_t> n01Threads_; ///< The number of threads used to produce [0,1[ random numbers
	Gem::Common::GThreadGroup producer_threads_01_; ///< A thread group that holds [0,1[ producer threads

	/** @brief A bounded buffer holding the [0,1[ random number packages */
	Gem::Common::GBoundedBufferT<std::shared_ptr < random_container>>
	p01_; // Note: Absolutely needs to be defined after the thread group !!!
	/** @brief A bounded buffer holding [0,1[ random number packages ready for recycling */
	Gem::Common::GBoundedBufferT<std::shared_ptr < random_container>>
	r01_;

	static std::uint16_t multiple_call_trap_; ///< Trap to catch multiple instantiations of this class
	static boost::mutex factory_creation_mutex_; ///< Synchronization of access to multiple_call_trap in constructor

	mutable boost::mutex thread_creation_mutex_; ///< Synchronization of access to the threadsHaveBeenStarted_ variable

	boost::random::random_device nondet_rng; ///< Source of non-deterministic random numbers
	initial_seed_type startSeed_; ///< Stores the initial start seed
	std::shared_ptr <mersenne_twister> mt_ptr_;
	mutable boost::shared_mutex seedingMutex_; ///< Regulates start-up of the seeding process
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
