/**
 * @file GSeedManager.hpp
 */

/* Copyright (C) 2009 Dr. Ruediger Berlich
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

#ifndef GSEEDMANAGER_HPP_
#define GSEEDMANAGER_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif


// GenEvA headers go here
#include "GBoundedBufferT.hpp"
#include "GEnums.hpp"
#include "GenevaExceptions.hpp"

/****************************************************************************/

namespace Gem {
namespace Util {

const boost::uint32_t DEFAULTSTARTSEED=1234;

/****************************************************************************/
/**
 * This class manages a set of seeds, making sure they are handed out in
 * random order themselves. The need for this class became clear when it
 * turned out that random number sequences with successive seeds can be
 * highly correlated. This can only be amended by handing out seeds in a
 * pseudo-random order themselves.
 */
class GSeedManager: private boost::noncopyable // prevents this class from being copied
{
public:
	/************************************************************************/
	/**
	 * Sets the internal queue to a given size
	 *
	 * @param minUniqueSeeds The number of unique seeds delivered by this class in succession
	 */
	GSeedManager(const boost::uint32_t minUniqueSeeds):
		minUniqueSeeds_(minUniqueSeeds),
		seedQueue_(minUniqueSeeds_),
		seedInitialized_(false),
		startSeed_(DEFAULTSTARTSEED)
	{ /* nothing */ }

	/************************************************************************/
	/**
	 * The destructor. Its main purpose is to make sure that the seed thread
	 * has terminated.
	 */
	~GSeedManager() {

	}

	/************************************************************************/
	/**
	 * Allows to set the initial seed of the sequence to a defined (i.e. not
	 * random) value. This function will only  have an effect if seeding hasn't
	 * started yet. It should thus be called before any random number consumers
	 * are started.
	 */
	void setStartSeed(boost::uint32_t startSeed) {
		if(!seedInitialized_) { // Faster when read before locking
			boost::mutex::scoped_lock lk(class_lock_);
			if(!seedInitialized_) { // could have been set by another thread already
				startSeed_ = startSeed;
			}
			seedInitialized_ = true;
		}
	}

	/************************************************************************/
	/**
	 * Allows different objects to retrieve seeds concurrently
	 *
	 * @return A seed that will not be followed by the same value in the next minUniqueSeeds_ calls
	 */
	boost::uint32_t getSeed() {
		// Double lock
		if(!seedInitialized_) { // Faster when read before locking
			boost::mutex::scoped_lock lk(class_lock_);
			if(!seedInitialized_) { // could have been set by another thread already
				// Set the local seed
				setStartSeed();

				// Start the seed thread. If the seed hasn't been
				// initialized yet, it is not yet running.

				// Make it known that seeding has started
				seedInitialized_ = true;
			}
		}

		boost::uint32_t seed;
		seedQueue_.pop_back(&seed);
		return seed;
	}

	/************************************************************************/
	/**
	 * Allows different objects to retrieve seeds concurrently, while observing
	 * a time-out. Note that this function will throw once the timeout is
	 * reached. See the GBoundedBufferT class for details.
	 *
	 * @param timeout duration until a timeout occurs
	 * @return A seed that will not be followed by the same value in the next minUniqueSeeds_ calls
	 */
	boost::uint32_t getSeed(const boost::posix_time::time_duration& timeout) {
		boost::uint32_t seed;
		seedQueue_.pop_back(&seed, timeout);
		return seed;
	}

	/************************************************************************/
	/**
	 * Allows to retrieve the minimum number of unique seeds delivered in
	 * succession. No need to lock the data structures - reading this variable
	 * should be atomic.
	 *
	 * @return The minimum number of unique seeds
	 */
	std::size_t getMinUniqueSeeds() const {
		return minUniqueSeeds_;
	}

private:
	/************************************************************************/
	/**
	 * We do not need a default constructor. Hence this function is
	 * intentionally private and undefined
	 */
	GSeedManager();

	/************************************************************************/
	/**
	 * Manages the production of seeds.
	 */
	void seedProducer() {
		try {
			// Instantiate a uniform random number generator with the start seed
			boost::mt19937 rng(startSeed_);

			// Add random seeds to the queue until the end of production has been signaled
			while (true) {
				if(boost::this_thread::interruption_requested()) break;
				seedQueue_.push_front_if_unique(boost::ref(rng));
			}
		}
		catch (boost::thread_interrupted&) { // Not an error
			return; // We're done
		}
		catch (std::bad_alloc& e) {
			std::cerr << "In GSeedManager::seedProducer(): Error!" << std::endl
			<< "Caught std::bad_alloc exception with message"
			<< std::endl << e.what() << std::endl;
			std::terminate();
		}
		catch (std::invalid_argument& e) {
			std::cerr << "In GSeedManager::seedProducer(): Error!" << std::endl
			<< "Caught std::invalid_argument exception with message"  << std::endl
			<< e.what() << std::endl;
			std::terminate();
		}
		catch (boost::thread_resource_error&) {
			std::cerr << "In GSeedManager::seedProducer(): Error!" << std::endl
			<< "Caught boost::thread_resource_error exception which"  << std::endl
			<< "likely indicates that a mutex could not be locked."  << std::endl;
			// Terminate the process
			std::terminate();
		}
		catch (...) {
			std::cerr << "In GSeedManager::seedProducer(): Error!" << std::endl
			<< "Caught unkown exception." << std::endl;
			// Terminate the process
			std::terminate();
		}
	}

	/************************************************************************/
	/**
	 * Creates an initial seed for the seeding random seed sequence to a random
	 * value. If no way of setting a random start value can be found, it leaves
	 * the startSeed_ variable untouched.
	 *
	 *
	 * @return An integer used as the start seed for a sequence of seeds
	 */
	void setStartSeed() {

	}

	/************************************************************************/
	// Variables and data structures

	/** @brief The minimum number of unique seeds to be delivered by this class */
	std::size_t minUniqueSeeds_;
	/** @brief Holds a predefined number of unique seeds */
	GBoundedBufferT<boost::uint32_t> seedQueue_;

	/** @brief Indicates whether the seed has already been initialized. Once
	 * this is done, no changes to this class'es settings are allowed anymore */
	bool seedInitialized_;

	/** The initial seed of the random seed sequence */
	boost::uint32_t startSeed_;

	/** @brief Locks access to all data structures of this class */
	boost::mutex class_lock_;
};

} /* namespace Util */
} /* namespace Gem */

#endif /* GSEEDMANAGER_HPP_ */
