/**
 * @file GRandomFactory.cpp
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

#include "hap/GRandomFactory.hpp"

namespace Gem {
namespace Hap {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * Initialization of static data members
 */
std::uint16_t Gem::Hap::GRandomFactory::multiple_call_trap_ = 0;
boost::mutex Gem::Hap::GRandomFactory::factory_creation_mutex_;

/******************************************************************************/
/**
 * The standard constructor, which seeds the random number generator and checks
 * that this class is instantiated only once.
 */
GRandomFactory::GRandomFactory()
  	: threads_started_(ATOMIC_FLAG_INIT)
	, nProducerThreads_(boost::numeric_cast<std::uint16_t>(Gem::Common::getNHardwareThreads(DEFAULT01PRODUCERTHREADS)))
  	, p_fresh_bfr_(DEFAULTFACTORYBUFFERSIZE)
  	, p_ret_bfr_(DEFAULTFACTORYBUFFERSIZE)
	, seedCollection_(DEFAULTSEEDVECTORSIZE)
	, seed_cit_(seedCollection_.begin())
{
	/*
	 * Apparently the entropy() call currently always returns 0 with g++ and clang,
	 * as this call is not fully implemented.
	 *
	// Check whether enough entropy is available. Warn, if this is not the case
	if (0. == nondet_rng_.entropy()) {
		glogger
		<< "In GSeedManager::GSeedManager(): Error!" << std::endl
		<< "Source of non-deterministic random numbers" << std::endl
		<< "has entropy 0." << std::endl
		<< GWARNING;
	}
	*/

	boost::mutex::scoped_lock lk(factory_creation_mutex_);
	if (multiple_call_trap_ > 0) {
		glogger
		<< "Error in GRandomFactory::GRandomFactory():" << std::endl
		<< "Class has been instantiated before." << std::endl
		<< "and may be instantiated only once" << std::endl
		<< GTERMINATION;
	} else {
		multiple_call_trap_++;
	}
}

/******************************************************************************/
/**
 * The destructor. All work is done in the finalize() function.
 */
GRandomFactory::~GRandomFactory() {
	// Make sure the finalization code is executed
	// (if this hasn't happened already). Calling
	// finalize() multiple times is safe.
	finalize();
}

/******************************************************************************/
/**
 * Initializes the factory. This function does nothing at this time. Its
 * only purpose is to control initialization of the factory in the singleton.
 */
void GRandomFactory::init() { /* nothing */ }

/******************************************************************************/
/**
 * Finalization code for the GRandomFactory. All threads are given the
 * interrupt signal. Then we wait for them to join us. This function will
 * only once perform useful work and will return immediately when called a
 * second time. It can thus be called as often as you wish.
 */
void GRandomFactory::finalize() {
	// Only allow one finalization action to be carried out
	if (finalized_) return;

	producer_threads_.interrupt_all(); // doesn't throw
	producer_threads_.join_all();
	finalized_ = true; // Let the audience know
}

/******************************************************************************/
/**
 * Allows to retrieve the size of random number arrays
 *
 * @return The current value of the arraySize_ variable
 */
std::size_t GRandomFactory::getCurrentArraySize() const {
	return DEFAULTARRAYSIZE;
}

/******************************************************************************/
/**
 * Retrieves the size of the random buffer, i.e. the array holding the random
 * number packages.
 *
 * @return The size of the random buffer
 */
std::size_t GRandomFactory::getBufferSize() const {
	return DEFAULTFACTORYBUFFERSIZE;
}

/******************************************************************************/
/**
 * This function returns a random number from a pseudo random sequence
 *
 * @return A seed taken from a local seed_seq object
 */
seed_type GRandomFactory::getSeed() {
	boost::unique_lock<boost::shared_mutex> sm_lck(seedingMutex_);

	// Refill at the start of seeding or when all seeds have been used
	if(!seedingHasStarted_ || seed_cit_==seedCollection_.end()) {
		seed_seq_.generate(seedCollection_.begin(), seedCollection_.end());
		seed_cit_=seedCollection_.begin();
		seedingHasStarted_=true;
	}

	seed_type result = *seed_cit_;
	++seed_cit_;

	return result;
}

/******************************************************************************/
/**
 * Allows recycling of partially used packages. Note that this function may
 * delete its argument if it cannot be added to the buffer.
 *
 * @param r A pointer to a partially used work package
 * @param current_pos The first position in the array that holds unused random numbers
 */
void GRandomFactory::returnUsedPackage(std::unique_ptr<random_container> p) {
	// We try to add the item to the p_ret_bfr_ queue, until a timeout occurs.
	// Once this is the case we delete the package, so we do not overflow
	// with recycled packages
	try {
		p_ret_bfr_.push_front(std::move(p), std::chrono::milliseconds(DEFAULTFACTORYPUTWAIT));
	} catch (Gem::Common::condition_time_out &) { // No luck buffer is full. Delete the recycling bin
		p.reset();
	}
}

/******************************************************************************/
/**
 * Sets the number of producer threads for this factory. See also
 * http://preshing.com/20130930/double-checked-locking-is-fixed-in-cpp11/ for
 * the rationale of the double checked locking pattern.
 *
 * @param n01Threads The number of threads simultaneously producing random numbers
 */
void GRandomFactory::setNProducerThreads(const std::uint16_t &nProducerThreads) {
	// Threads might already be running, so we need to regulate access
	if(threads_started_.load()) {
		// If we enter this code-path, there is no way threads
		// could go into the "not-running" state, so we do not need
		// to check again using DCLP .
		boost::unique_lock<boost::mutex> lk(thread_creation_mutex_);
		// Make a suggestion for the number of threads, if requested
		std::uint16_t nProducerThreads_local =
			(nProducerThreads > 0) ? nProducerThreads : (boost::numeric_cast<std::uint16_t>(Gem::Common::getNHardwareThreads(DEFAULT01PRODUCERTHREADS)));

		if (nProducerThreads_local > nProducerThreads_.load()) { // start new 01 threads
			for (std::uint16_t i = nProducerThreads_.load(); i < nProducerThreads_local; i++) {
				producer_threads_.create_thread(
					[this]() { this->producer(this->getSeed()); }
				);
			}
		} else if (nProducerThreads_local < nProducerThreads_.load()) { // We need to remove threads
			// remove_last will internally call "interrupt" for these threads
			producer_threads_.remove_last(nProducerThreads_.load() - nProducerThreads_local);
		}
	} else { // Double-checked locking pattern
		// Here it appears that no threads were running. We do need to check again, though (DLCP)
		boost::unique_lock<boost::mutex> tc_lk(thread_creation_mutex_);
		// Make a suggestion for the number of threads, if requested
		std::uint16_t nProducerThreads_local =
			(nProducerThreads > 0) ? nProducerThreads : (boost::numeric_cast<std::uint16_t>(Gem::Common::getNHardwareThreads(DEFAULT01PRODUCERTHREADS)));

		if (threads_started_.load()) { // Someone has started the threads in the meantime. Adjust the number of threads
			if (nProducerThreads_local > nProducerThreads_.load()) { // start new 01 threads
				for (std::uint16_t i = nProducerThreads_.load(); i < nProducerThreads_local; i++) {
					producer_threads_.create_thread(
						[this]() { this->producer(this->getSeed()); }
					);
				}
			} else if (nProducerThreads_local < nProducerThreads_.load()) { // We need to remove threads
				// remove_last will internally call "interrupt" for these threads
				producer_threads_.remove_last(nProducerThreads_.load() - nProducerThreads_local);
			}
		}

		// Whether they were already running or not -- we may now adjust the number of producer threads
		nProducerThreads_ = nProducerThreads_local;
	}
}

/******************************************************************************/
/**
 * When objects need a new container [0,1[ of random numbers with the current
 * default size, they call this function. See also
 * http://preshing.com/20130930/double-checked-locking-is-fixed-in-cpp11/ for
 * the rationale.
 *
 * @return A packet of new [0,1[ random numbers
 */
std::unique_ptr <random_container> GRandomFactory::getNewRandomContainer() {
	// Start the producer threads upon first access to this function
	if (!threads_started_.load()) {
		boost::unique_lock<boost::mutex> tc_lk(thread_creation_mutex_);
		if (!threads_started_.load()) { // double checked locking pattern
			//---------------------------------------------------------
			for (std::uint16_t i = 0; i < nProducerThreads_.load(); i++) {
				producer_threads_.create_thread(
					[this]() { this->producer(this->getSeed()); }
				);
			}
			//---------------------------------------------------------

			threads_started_.store(true);
		}
	}

	std::unique_ptr <random_container> p; // empty
	try {
		p_fresh_bfr_.pop_back(p, std::chrono::milliseconds(DEFAULTFACTORYGETWAIT));
	} catch (Gem::Common::condition_time_out &) {
		// nothing - our way of signaling a time out
		// is to return an empty std::unique_ptr
		p = std::unique_ptr<random_container>();
	}

	return p;
}

/******************************************************************************/
/**
 * The production of [0,1[ random numbers takes place here. As this function
 * is called in a thread, it may only throw using Genevas mechanisms. Exceptions
 * could otherwise go unnoticed. Hence this function has a possibly confusing
 * setup.
 *
 * @param seed A seed for our local random number generator
 */
void GRandomFactory::producer(std::uint32_t seed) { // TODO: should be result_type ?
	try {
		G_BASE_GENERATOR mt(seed);
		std::unique_ptr<random_container> p;

		while (true) {
			// Check whether an interruption was requested
			Gem::Common::thread::interruption_point();

			if (!p) { // buffer is still empty
				// First we try to retrieve a "recycled" item from the p_ret_bfr_ buffer. If this
				// fails (likely because the buffer is empty), we create a new item instead
				try {
					p_ret_bfr_.pop_back(p, std::chrono::milliseconds(DEFAULTFACTORYGETWAIT));

					// If we reach this line, we have successfully retrieved a recycled container.
					// First do some error-checking
#ifdef DEBUG
		         if(!p) {
		            glogger
		            << "In RandomFactory::producer(): Error!" << std::endl
		            << "Got empty recycling pointer" << std::endl
		            << GEXCEPTION;
		         }

#endif /* DEBUG */

					// Finally we replace "used" random numbers with new ones
					p->refresh(mt);

				} catch (Gem::Common::condition_time_out &) { // O.k., so we need to create a new container
					p = std::unique_ptr<random_container>(new random_container(mt));
				}
			}

			// Thanks to the following call, thread creation will be mostly idle if the buffer is full
			try {
				// Put the bufffer in the queue
				p_fresh_bfr_.push_front(std::move(p), std::chrono::milliseconds(DEFAULTFACTORYPUTWAIT));
				// Reset the shared_ptr so the next pointer may be created
				p.reset();
			} catch (Gem::Common::condition_time_out &) {
				continue; // Try again, if we didn't succeed
			}
		}
	} catch (Gem::Common::thread_interrupted &) { // Not an error
		return; // We're done
	} catch (std::bad_alloc &e) {
		glogger
		<< "In GRandomFactory::producer(): Error!" << std::endl
		<< "Caught std::bad_alloc exception with message"
		<< std::endl << e.what() << std::endl
		<< GEXCEPTION;
	} catch (std::invalid_argument &e) {
		glogger
		<< "In GRandomFactory::producer(): Error!" << std::endl
		<< "Caught std::invalid_argument exception with message" << std::endl
		<< e.what() << std::endl
		<< GEXCEPTION;
	} catch (boost::thread_resource_error &) {
		glogger
		<< "In GRandomFactory::producer(): Error!" << std::endl
		<< "Caught boost::thread_resource_error exception which" << std::endl
		<< "likely indicates that a mutex could not be locked." << std::endl
		<< GEXCEPTION;
	} catch (...) {
		glogger
		<< "In GRandomFactory::producer(): Error!" << std::endl
		<< "Caught unkown exception." << std::endl
		<< GEXCEPTION;
	}
}

/******************************************************************************/

} /* namespace Hap */
} /* namespace Gem */
