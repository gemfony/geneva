/**
 * @file GSeedManager.hpp
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
#include <boost/nondet_random.hpp>
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
#include <boost/date_time.hpp>
#include <boost/cast.hpp>
#include <boost/static_assert.hpp>

/**
 * Check that we have support for threads. This class is useless without this.
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


// Gemfony headers go here
#include "common/GBoundedBufferT.hpp"
#include "common/GCommonEnums.hpp"
#include "common/GExceptions.hpp"
#include "hap/GRandomDefines.hpp"

/******************************************************************************/

namespace Gem {
namespace Hap {

/******************************************************************************/
/**
 * This class manages a set of seeds, making sure they are handed out in
 * pseudo random order themselves. The need for this class results from the fact
 * that random number sequences with successive seeds can be highly correlated.
 * This can only be amended by handing out seeds in a pseudo random fashion themselves.
 * A start seed for the seeding sequence is either taken from a non deterministic
 * generator, or can be provided by the user.
 */
class GSeedManager:
	private boost::noncopyable // prevents this class from being copied
{
public:
	/** @brief The default constructor. */
	GSeedManager();
	/** @brief Initialization with a start seed */
	GSeedManager(const initial_seed_type& startSeed, const std::size_t& seedQueueSize = DEFAULTSEEDQUEUESIZE);
	/** @brief The destructor */
	~GSeedManager();

	/** @brief Allows different objects to retrieve seeds concurrently */
	seed_type getSeed();

	/** @brief Allows different objects to retrieve seeds concurrently, observing a time-out. */
	seed_type getSeed(const boost::posix_time::time_duration&);

	/** @brief Checks whether the global seeding has already started */
	bool checkSeedingIsInitialized() const;

	/** @brief Retrieves the value of the initial start seed */
	initial_seed_type getStartSeed() const;

	/** @brief Retrieves the maximum size of the seed queue */
	std::size_t getQueueSize() const;

private:
	/***************************************************************************/
	/**
	 * Wrapper function that attempts to create a start seed using different methods
	 *
	 * TODO: Add a method that allows to retrieve a good start seed on Windows systems
	 *
	 * @return A start seed for the random seed sequence
	 */
	static initial_seed_type createStartSeed() {
		initial_seed_type startSeed = 0;

		startSeed = GSeedManager::createStartSeedDevURandom();
		if(0 == startSeed) {
			std::cerr
			<< "Warning: Using /dev/urandom to retrieve global seed failed." << std::endl
			<< "Using the current time instead: ";
			startSeed = GSeedManager::createStartSeedCurrentTime();
	 	}

		return startSeed;
	}

	/***************************************************************************/
	/**
	 * A private member function that allows to set the global
	 * seed once, using a random number taken from /dev/urandom. As enough entropy
	 * needs to be available and reads from /dev/random will block (on Linux) if this
	 * is not the case, this function should be called only rarely and is meant for
	 * the initialization of the random seed sequence only.
	 *
	 * @return A random number obtained from /dev/urandom
	 */
	static initial_seed_type createStartSeedDevURandom() {
		// Check if /dev/urandom exists (this might not be a Linux system after all)
		if(!boost::filesystem::exists("/dev/urandom")) return 0;

		// Open the device
		std::ifstream devurandom("/dev/urandom");
		if(!devurandom) return 0;

		// Read in the data
		initial_seed_type startSeed;
		devurandom.read(reinterpret_cast<char*>(&startSeed), sizeof(initial_seed_type));

		// Clean up
		devurandom.close();

		return startSeed;
	}

	/***************************************************************************/
	/**
	 * Allows to derive a seed from the current time. Note that, although we try to
	 * add randomness, this might not give good results if many seeds are requested by
	 * different entities in short succession. It should be sufficient for a one-time
	 * retrieval of a seed for the seed random sequence, though. Remote components of
	 * an application needing unique seeds should retrieve them out of the random
	 * sequence instead of this function.
	 */
	static initial_seed_type createStartSeedCurrentTime() {
		using namespace boost::posix_time;
		using namespace boost::gregorian;

		// There might be strange systems where this is not the case
		BOOST_STATIC_ASSERT(sizeof(long) >= sizeof(initial_seed_type));

		// Retrieve the time passed since January 1, 1970 in microseconds
		long currentMS = (microsec_clock::local_time() - ptime(date(1970, Jan, 1))).total_microseconds();

		// Attach the char pointer to the beginning of the long value
		char *currentMSStart = reinterpret_cast<char*>(&currentMS);

		// Attach the seed to this point
		initial_seed_type *seed_ptr = reinterpret_cast<initial_seed_type *>(currentMSStart);

		// Let the audience know
		return *seed_ptr;
	}

	/***************************************************************************/
	/** @brief Manages the production of seeds */
	void seedProducer();

	/***************************************************************************/
	// Variables and data structures

	/** @brief Holds a predefined number of unique seeds */
	Gem::Common::GBoundedBufferT<seed_type> seedQueue_;

	/** @brief Stores the initial start seed */
	initial_seed_type startSeed_;

	/** @brief Holds the producer thread */
	thread_ptr seedThread_;
};

/******************************************************************************/

} /* namespace Hap */
} /* namespace Gem */

#endif /* GSEEDMANAGER_HPP_ */
