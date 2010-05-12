/**
 * @file GSeedManager.hpp
 */

/* Copyright (C) Dr. Ruediger Berlich and Karlsruhe Institute of Technology
 * (University of the State of Baden-Wuerttemberg and National Laboratory
 * of the Helmholtz Association)
 *
 * Contact: info [at] gemfony (dot) com
 *
 * This file is part of the Geneva library, Gemfony scientific's optimization
 * library.
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

/**
 * This seed will be used as the global setting if the seed hasn't
 * been set manually and could not be determined in a random way (e.g.
 * by reading from /dev/urandom). The chosen value follows a setting
 * in boost's mersenne twister library.
 */
const boost::uint32_t DEFAULTSTARTSEED=5489;

/**
 * This value specifies the guaranteed number of unique seeds that will
 * follow when retrieving a seed from this class.
 */
const std::size_t DEFAULTSEEDQUEUESIZE=5000;

/****************************************************************************/
/**
 * This class manages a set of seeds, making sure they are handed out in
 * random order themselves. The need for this class became clear when it
 * turned out that random number sequences with successive seeds can be
 * highly correlated. This can only be amended by handing out seeds in a
 * pseudo-random order themselves.
 */
class GSeedManager:
	private boost::noncopyable // prevents this class from being copied
{
	typedef boost::shared_ptr<boost::thread> thread_ptr;
	typedef boost::mt19937 mersenne_twister;

public:
	/************************************************************************/
	/**
	 * The default constructor.
	 */
	GSeedManager():
		queueSize_(DEFAULTSEEDQUEUESIZE),
		seedQueue_(queueSize_),
		startSeed_(0) // means: "unset"
	{ /* nothing */ }

	/************************************************************************/
	/**
	 * The destructor. Its main purpose is to make sure that the seed thread
	 * has terminated.
	 */
	~GSeedManager() {
		if(seedThread_) {
			seedThread_->interrupt();
			seedThread_->join();
		}
	}

	/************************************************************************/
	/**
	 * Allows to set the initial seed of the sequence to a defined (i.e. not
	 * random) value. This function will only  have an effect if seeding hasn't
	 * started yet. It should thus be called before any random number consumers
	 * are started.
	 */
	bool setStartSeed(boost::uint32_t startSeed) {
		// Check that startSeed is != 0
		if(startSeed == 0) {
			std::cerr << "In GSeedManager::setStartSeed : Error!" << std::endl
					  << "Tried to set the start seed to 0. This value" << std::endl
					  << "has a special meaning in the class" << std::endl;
			exit(1);
		}

		// We only act if seeding hasn't started yet
		if(!seedThread_) { // Faster when read before locking
			boost::mutex::scoped_lock lk(class_lock_);
			if(!seedThread_) { // could have been set by another thread already
				startSeed_ = startSeed;
				return true;
			}
			else {
				return false;
			}
		}
		else {
			return false;
		}
	}

	/************************************************************************/
	/**
	 * Allows to retrieve the current value of the start seed
	 *
	 * @return The current value of the start seed
	 */
	boost::uint32_t getStartSeed() const {
		return startSeed_;
	}

	/************************************************************************/
	/**
	 * Allows different objects to retrieve seeds concurrently. Note that
	 * this function will block if the queue is empty and will only wake
	 * up again once seed items have again become available.
	 *
	 * @return A seed that will not be followed by the same value in the next _ calls
	 */
	boost::uint32_t getSeed() {
		checkSeedAndThread();
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
	 * @return A seed that will not be followed by the same value in the next _ calls
	 */
	boost::uint32_t getSeed(const boost::posix_time::time_duration& timeout) {
		checkSeedAndThread();
		boost::uint32_t seed;
		seedQueue_.pop_back(&seed, timeout);
		return seed;
	}

	/*************************************************************************/
	/**
	 * Checks whether the global seeding has already started. This is the case
	 * if the seedThread_ sharerd_ptr points to an actual object
	 *
	 * @return A boolean indicating whether the seed has already been initialized
	 */
	bool checkSeedingIsInitialized() const {
		return (seedThread_?true:false);
	}

	/*************************************************************************/
	/**
	 * Retrieve the size of the seeding queue
	 */
	std::size_t getQueueSize() const {
		return queueSize_;
	}

private:
	/*************************************************************************/
	/**
	 * A private member function that allows to set the global
	 * seed to a fixed value.
	 *
	 * @param seed The desired initial value of the global seed
	 */
	void setSeedFixed(const boost::uint32_t& seed) {
		startSeed_ = seed; // Set the seed as requested;
	}

	/*************************************************************************/
	/**
	 * A private member function that allows to set the global
	 * seed once, using a random number taken from /dev/urandom. This function
	 * should be called only once.
	 *
	 * @return A boolean indicating whether retrieval was successful
	 */
	bool setSeedURandom() {
		// Check if /dev/urandom exists (this might not be a Linux system after all)
		if(!boost::filesystem::exists("/dev/urandom")) return false;
		// Open the device
		std::ifstream urandom("/dev/urandom");
		if(!urandom) return false;
		// Read in the data
		boost::uint32_t seed;
		urandom.read(reinterpret_cast<char*>(&seed), sizeof(seed));
		urandom.close();
		startSeed_ = seed; // Set the seed as requested;
		return true;
	}

	/************************************************************************/
	/**
	 * Checks whether the seed has already been set and, if necessary, initializes
	 * it and starts the seeding thread.
	 */
	void checkSeedAndThread() {
		// Double lock
		if(!seedThread_) { // Faster when read before locking
			boost::mutex::scoped_lock lk(class_lock_);
			if(!seedThread_) { // could have been set by another thread already
				// Set the local seed, unless it as been set already
				if(startSeed_==0) {
					if(!setSeedURandom()) {
						std::cerr << "Using /dev/urandom to set global seed failed." << std::endl
							      << "Setting the seed to the default value " << DEFAULTSTARTSEED << "instead" << std::endl;
						setSeedFixed(DEFAULTSTARTSEED);
					}
					else {
						std::cout << "Used /dev/urandom to set the start seed to " << startSeed_ << std::endl;
					}
				}
				else {
					std::cout << "Using pre-set seed of " << startSeed_ << std::endl;
				}

				// Start the seed thread.
				seedThread_ = thread_ptr(new boost::thread(boost::bind(&GSeedManager::seedProducer, this)));
			}
		}
	}

	/************************************************************************/
	/**
	 * Manages the production of seeds.
	 */
	void seedProducer() {
		try {
			// Instantiate a uniform random number generator with the start seed
			mersenne_twister mt(startSeed_);

			// Add random seeds to the queue until the end of production has been signaled
			while (true) {
				if(boost::this_thread::interruption_requested()) break;
				seedQueue_.push_front(mt());
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
	// Variables and data structures

	/** @brief The minimum number of unique seeds to be delivered by this class */
	std::size_t queueSize_;

	/** @brief Holds a predefined number of unique seeds */
	GBoundedBufferT<boost::uint32_t> seedQueue_;

	/** The initial seed of the random seed sequence */
	boost::uint32_t startSeed_;

	/** @brief Locks access to all data structures of this class */
	boost::mutex class_lock_;

	/** @brief Holds the producer thread */
	thread_ptr seedThread_;
};

} /* namespace Util */
} /* namespace Gem */

#endif /* GSEEDMANAGER_HPP_ */
