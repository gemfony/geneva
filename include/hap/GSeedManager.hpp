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

// Boost headers go here

#include <boost/cstdint.hpp>
#include <boost/random.hpp>
#include <boost/random/random_device.hpp>
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
class G_API GSeedManager:
	private boost::noncopyable // prevents this class from being copied
{
public:
	/** @brief The default constructor. */
	GSeedManager();
	/** @brief Initialization with a start seed */
	explicit GSeedManager(const initial_seed_type& startSeed, const std::size_t& seedQueueSize = DEFAULTSEEDQUEUESIZE);
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
	/** @brief Manages the production of seeds */
	void seedProducer();

	/***************************************************************************/
	// Variables and data structures

   boost::random::random_device nondet_rng; ///< Source of non-deterministic random numbers

	Gem::Common::GBoundedBufferT<seed_type> seedQueue_; ///< Holds a predefined number of unique seeds

	initial_seed_type startSeed_; ///< Stores the initial start seed
	thread_ptr seedThread_; ///< Holds the producer thread
};

/******************************************************************************/

} /* namespace Hap */
} /* namespace Gem */

#endif /* GSEEDMANAGER_HPP_ */
