/**
 * @file GSeedManager.cpp
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

#include "hap/GSeedManager.hpp"

namespace Gem {
namespace Hap {

/******************************************************************************/
/**
 * The default constructor.
 */
GSeedManager::GSeedManager()
	: seedQueue_(DEFAULTSEEDQUEUESIZE)
	, startSeed_(GSeedManager::createStartSeed())
{
	// Start the seed thread.
	seedThread_ = thread_ptr(new boost::thread(boost::bind(&GSeedManager::seedProducer, this)));
}

/******************************************************************************/
/**
 * Initialization with a start seed. Setting the seed to 0 will result in
 * the seed being obtained from a non-determinsitic source, as provided by
 * the Boost library collection. You can also set the queue size with this
 * function. By default it will use the DEFAULTQUEUESIZE.
 *
 * @param startSeed The initial seed for the pseudo-random generation of seeds
 * @param seedQueueSize The maximum number of seeds that can be stored in the queue
 */
GSeedManager::GSeedManager(const initial_seed_type& startSeed, const std::size_t& seedQueueSize)
	: seedQueue_(seedQueueSize)
	, startSeed_(startSeed>0?startSeed:GSeedManager::createStartSeed())
{
	// Cross-check the provided size of the seed queue
	if(seedQueueSize == 0) {
		std::cerr
		<< "In GSeedManager::GSeedManager(const seed_type&, const std::size_t&): Error!" << std::endl
      << "Received seedQueueSize of 0." << std::endl;
		std::terminate();
	}

	// Start the seed thread.
	seedThread_ = thread_ptr(new boost::thread(boost::bind(&GSeedManager::seedProducer, this)));
}

/******************************************************************************/
/**
 * The destructor. Its main purpose is to make sure that the seed thread
 * has terminated.
 */
GSeedManager::~GSeedManager() {
	if(seedThread_) {
		seedThread_->interrupt();
		seedThread_->join();
	}
}

/******************************************************************************/
/**
 * Allows different objects to retrieve seeds concurrently. Note that
 * this function will block if the queue is empty and will only wake
 * up again once seed items have again become available.
 *
 * @return A seed that is taken from a single random sequence
 */
seed_type GSeedManager::getSeed() {
	seed_type seed;
	seedQueue_.pop_back(seed);
	return seed;
}

/******************************************************************************/
/**
 * Allows different objects to retrieve seeds concurrently, while observing
 * a time-out. Note that this function will throw once the timeout is
 * reached. See the GBoundedBufferT class for details.
 *
 * @param timeout duration until a timeout occurs
 * @return A seed that will not be followed by the same value in the next _ calls
 */
seed_type GSeedManager::getSeed(const boost::posix_time::time_duration& timeout) {
	seed_type seed;
	seedQueue_.pop_back(seed, timeout);
	return seed;
}

/*************************************************************************/
/**
 * Retrieves the (fixed) value of the initial start seed
 *
 * @return The value of the initial start seed
 */
initial_seed_type GSeedManager::getStartSeed() const {
	return startSeed_;
}

/*************************************************************************/
/**
 * Retrieves the maximum size of the seed queue
 *
 * @return The size of the seed queue
 */
std::size_t GSeedManager::getQueueSize() const {
	return seedQueue_.getCapacity();
}

/*************************************************************************/
/**
 * Checks whether the global seeding has already started. This is the case
 * if the seedThread_ sharerd_ptr points to an actual object
 *
 * @return A boolean indicating whether the seed has already been initialized
 */
bool GSeedManager::checkSeedingIsInitialized() const {
	return (seedThread_?true:false);
}

/******************************************************************************/
/**
 * Manages the production of seeds from within its own thread. Only one
 * thread is started.
 */
void GSeedManager::seedProducer() {
	try {
		// Create a random number generator
		mersenne_twister mt(boost::numeric_cast<seed_type>(startSeed_));

		// Add random seeds to the queue until the end of production has been signaled
		while (true) {
			if(boost::this_thread::interruption_requested()) break;

			try {
				seedQueue_.push_front(mt(), boost::posix_time::milliseconds(DEFAULTSEEDQUEUEPUTWAIT));
			}
			catch(Gem::Common::condition_time_out&){
			   continue;
			} // queue is full: just try again. This will not keep the machine busy, as we use a time-out
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

/******************************************************************************/

} /* namespace Hap */
} /* namespace Gem */
