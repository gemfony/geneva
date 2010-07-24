/**
 * @file GSeedManager.hpp
 */

/* Copyright (C) Dr. Ruediger Berlich and Karlsruhe Institute of Technology
 * (University of the State of Baden-Wuerttemberg and National Laboratory
 * of the Helmholtz Association)
 *
 * Contact: info [at] gemfony (dot) com
 *
 * This file is part of the Hap library, Gemfony scientific's library of
 * random number routines.
 *
 * Hap is free software: you can redistribute it and/or modify
 * it under the terms of version 3 of the GNU Affero General Public License
 * as published by the Free Software Foundation.
 *
 * Hap is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with the Hap library.  If not, see <http://www.gnu.org/licenses/>.
 *
 * For further information on Gemfony scientific and Hap, visit
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
#include "random/GRandomDefines.hpp"

/****************************************************************************/

namespace Gem {
namespace Hap {

/****************************************************************************/
/**
 * This class manages a set of seeds, making sure they are handed out in
 * pseudo random order themselves. The need for this class became clear when it
 * turned out that random number sequences with successive seeds can be
 * highly correlated. This can only be amended by handing out seeds randomly
 * themselves. A start seed for the seeding sequence is either taken from a
 * non deterministic generator, or provided by the user.
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
	/*************************************************************************/
	/**
	 * Wrapper function that performs a one-time creation of a start seed
	 * for the seeding sequence. It uses a non-deterministic random number
	 * generator, as provided by the boost library collection (see e.g.
	 * here: http://www.boost.org/doc/libs/1_43_0/doc/html/boost/random_device.html).
	 *
	 * @return A start seed for the seeding sequence
	 */
	static initial_seed_type createStartSeed() {
		initial_seed_type startSeed;
		nondet_rng rng;
		while(!(startSeed=rng())); // Prevent a start seed of 0

		std::cout << "Obtained start seed of " << startSeed << std::endl;

		return startSeed;
	}

	/************************************************************************/
	/** @brief Manages the production of seeds */
	void seedProducer();

	/************************************************************************/
	// Variables and data structures

	/** @brief Holds a predefined number of unique seeds */
	Gem::Common::GBoundedBufferT<seed_type> seedQueue_;

	/** @brief Stores the initial start seed */
	initial_seed_type startSeed_;

	/** @brief Holds the producer thread */
	thread_ptr seedThread_;
};

} /* namespace Hap */
} /* namespace Gem */

#endif /* GSEEDMANAGER_HPP_ */
