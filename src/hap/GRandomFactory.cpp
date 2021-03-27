/********************************************************************************
 *
 * This file is part of the Geneva library collection. The following license
 * applies to this file:
 *
 * ------------------------------------------------------------------------------
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * ------------------------------------------------------------------------------
 *
 * Note that other files in the Geneva library collection may use a different
 * license. Please see the licensing information in each file.
 *
 ********************************************************************************
 *
 * Geneva was started by Dr. Rüdiger Berlich and was later maintained together
 * with Dr. Ariel Garcia under the auspices of Gemfony scientific. For further
 * information on Gemfony scientific, see http://www.gemfomy.eu .
 *
 * The majority of files in Geneva was released under the Apache license v2.0
 * in February 2020.
 *
 * See the NOTICE file in the top-level directory of the Geneva library
 * collection for a list of contributors and copyright information.
 *
 ********************************************************************************/

#include "hap/GRandomFactory.hpp"

namespace Gem {
namespace Hap {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * Initialization of static data members
 */
std::atomic<bool> GRandomFactory::m_multiple_call_trap = ATOMIC_VAR_INIT(false);

/******************************************************************************/
/**
 * The standard constructor, which seeds the random number generator and checks
 * that this class is instantiated only once.
 */
GRandomFactory::GRandomFactory() {
	/*
	 * Apparently the entropy() call currently always returns 0 with g++ and clang,
	 * as this call is not fully implemented.
	 *
	// Check whether enough entropy is available. Warn, if this is not the case
	if (0. == m_multiple_call_trap.entropy()) {
		glogger
		<< "In GSeedManager::GSeedManager(): Error!" << std::endl
		<< "Source of non-deterministic random numbers" << std::endl
		<< "has entropy 0." << std::endl
		<< GWARNING;
	}
	*/

	if (m_multiple_call_trap) {
		glogger
			<< "Error in GRandomFactory::GRandomFactory():" << std::endl
			<< "Class has been instantiated before." << std::endl
			<< "and may be instantiated only once" << std::endl
			<< GTERMINATION;
	} else {
		m_multiple_call_trap.store(true);
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
	if (m_finalized) return;

	// Flag all threads to stop
	m_threads_stop_requested.store(true);
	// Wait for all threads to return
	m_producer_threads.join_all();

	// Let the audience know
	m_finalized.store(true);
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
 * Retrieves the size of the random buffer, i.e. the data structure holding the
 * random number packages.
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
	std::unique_lock<std::mutex> sm_lck(m_seeding_mutex);

	// Refill at the start of seeding or when all seeds have been used
	if(not m_seeding_has_started || m_seed_cit==m_seed_collection.end()) {
		m_seed_seq.generate(m_seed_collection.begin(), m_seed_collection.end());
		m_seed_cit=m_seed_collection.begin();
		m_seeding_has_started.store(true);
	}

	seed_type result = *m_seed_cit;
	++m_seed_cit;

	return result;
}

/******************************************************************************/
/**
 * Allows recycling of (possibly partially used) packages. This way we avoid
 * the continuous allocation and deletion of new buffers. Note that this function
 * may delete its argument if it cannot be added to the buffer.
 *
 * @param r A pointer to a partially used work package
 * @param current_pos The first position in the array that holds unused random numbers
 */
void GRandomFactory::returnUsedPackage(std::unique_ptr<random_container>&& p) {
	// We try to add the item to the m_p_ret_bfr queue.
	if(not m_p_ret_bfr.try_push_move(std::move(p))) {
		p.reset();
	}
}

/******************************************************************************/
/**
 * Sets the number of producer threads for this factory. See also
 * http://preshing.com/20130930/double-checked-locking-is-fixed-in-cpp11/ for
 * the rationale of the double checked locking pattern. Note that only an
 * increase of the number of threads is allowed when threads are already running.
 *
 * @param n01Threads The number of threads simultaneously producing random numbers
 */
void GRandomFactory::setNProducerThreads(const std::uint16_t &nProducerThreads) {
	// Threads might already be running, so we need to regulate access
	if(m_threads_started) {
		// If we enter this code-path, there is no way threads
		// could go into the "not-running" state, so we do not need
		// to check again using DCLP .
		std::unique_lock<std::mutex> lk(m_thread_creation_mutex);
		// Make a suggestion for the number of threads, if requested
		std::uint16_t nProducerThreads_local = DEFAULT01PRODUCERTHREADS;
		if(0 == nProducerThreads) {
			glogger
				<< "In GRandomFactory::setNProducerThreads(nProducerThreads) / 1:" << std::endl
				<< "nProducerThreads == 0 was requested. nProducerThreads_local was set to the default "
				<< DEFAULT01PRODUCERTHREADS << std::endl
				<< GWARNING;
		} else {
			nProducerThreads_local = nProducerThreads;
		}

		if (nProducerThreads_local > m_n_producer_threads.load()) { // start new 01 threads
			for (std::uint16_t i = m_n_producer_threads.load(); i < nProducerThreads_local; i++) {
				m_producer_threads.create_thread(
					[this]() { this->producer(this->getSeed()); }
				);
			}
		} else if (nProducerThreads_local < m_n_producer_threads.load()) { // We need to remove threads
			glogger
				<< "In GRandomFactory::setNProducerThreads(" << nProducerThreads << "): Warning!" << std::endl
				<< "Attempt to decrease the number of producer threads from " << m_n_producer_threads.load() << " to " << nProducerThreads << std::endl
				<< "while threads were already running. The number of threads will remain unchanged." << std::endl
				<< GWARNING;

			return;
		}
	} else { // Double-checked locking pattern
		// Here it appears that no threads were running. We do need to check again, though (DLCP)
		std::unique_lock<std::mutex> tc_lk(m_thread_creation_mutex);
		// Make a suggestion for the number of threads, if requested
		std::uint16_t nProducerThreads_local = DEFAULT01PRODUCERTHREADS;
		if(nProducerThreads == 0) {
			glogger
				<< "In GRandomFactory::setNProducerThreads(nProducerThreads) / 2:" << std::endl
				<< "nProducerThreads == 0 was requested. nProducerThreads_local was set to the default "
				<< DEFAULT01PRODUCERTHREADS << std::endl
			    << GWARNING;
		} else {
			nProducerThreads_local = nProducerThreads;
		}

		if (m_threads_started) { // Someone has started the threads in the meantime. Adjust the number of threads
			if (nProducerThreads_local > m_n_producer_threads.load()) { // start new 01 threads
				for (std::uint16_t i = m_n_producer_threads.load(); i < nProducerThreads_local; i++) {
					m_producer_threads.create_thread(
						[this]() { this->producer(this->getSeed()); }
					);
				}
			} else if (nProducerThreads_local < m_n_producer_threads.load()) { // We need to remove threads
				glogger
					<< "In GRandomFactory::setNProducerThreads(" << nProducerThreads << "): Warning!" << std::endl
					<< "Attempt to decrease the number of producer threads from " << m_n_producer_threads.load() << " to " << nProducerThreads << std::endl
					<< "while threads were alredy running. The number of threads will remain unchanged." << std::endl
					<< GWARNING;

				return;
			}
		}

		// Whether they were already running or not -- we may now adjust the number of producer threads
		m_n_producer_threads = nProducerThreads_local;
	}
}

/******************************************************************************/
/**
 * When objects need a new container of [0,1[ -random numbers with the current
 * default size, they call this function. See also
 * http://preshing.com/20130930/double-checked-locking-is-fixed-in-cpp11/ for
 * the rationale.
 *
 * @return A packet of new [0,1[ random numbers
 */
std::unique_ptr<random_container> GRandomFactory::getNewRandomContainer() {
	// Start the producer threads upon first access to this function
	if (not m_threads_started) {
		std::unique_lock<std::mutex> tc_lk(m_thread_creation_mutex);
		if (not m_threads_started) { // double checked locking pattern
			//---------------------------------------------------------
			for (std::uint16_t i = 0; i < m_n_producer_threads.load(); i++) {
				m_producer_threads.create_thread(
					[this]() { this->producer(this->getSeed()); }
				);
			}
			//---------------------------------------------------------

			m_threads_started.store(true);
		}
	}

	std::unique_ptr<random_container> p; // empty
	if(not m_p_fresh_bfr.pop_and_wait_move(
		p
		, std::chrono::milliseconds(DEFAULTFACTORYGETWAIT))) {
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
void GRandomFactory::producer(std::uint32_t seed) {
	try {
		G_BASE_GENERATOR mt(seed);
		std::unique_ptr<random_container> p;

		while(not m_threads_stop_requested) {
			// First we try to retrieve a "recycled" item from the m_p_ret_bfr buffer. If this
			// fails (likely because the buffer is empty), we create a new item instead
			if(m_p_ret_bfr.try_pop_move(p)) {

				// If we reach this line, we have successfully retrieved a recycled container.
				// First do some error-checking
#ifdef DEBUG
				if(not p) {
					throw gemfony_exception(
						g_error_streamer(DO_LOG,  time_and_place)
							<< "In RandomFactory::producer(): Error!" << std::endl
							<< "Got empty recycling pointer" << std::endl
					);
				}

#endif /* DEBUG */

				// Replace "used" random numbers with new ones
				p->refresh(mt);
			} else { // O.k., so we need to create a new container
				p.reset(new random_container(mt));
			}

			// Try to submit the item and check for termination conditions along the way
			while(not m_threads_stop_requested) {
				if(not m_p_fresh_bfr.try_push_move(std::move(p))){
#ifdef DEBUG
					// p should never be empty here
					if(not p) {
						throw gemfony_exception(
							g_error_streamer(DO_LOG,  time_and_place)
								<< "In RandomFactory::producer(): Error!" << std::endl
								<< "Got empty pointer after unsuccesfull submission" << std::endl
						);
					}
#endif
					std::this_thread::sleep_for(std::chrono::milliseconds(100));
					continue;
				} else { // We have submitted the item -- stop the inner loop
					break;
				}
			}
		}
	} catch (std::bad_alloc &e) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GRandomFactory::producer(): Error!" << std::endl
				<< "Caught std::bad_alloc exception with message" << std::endl
				<< e.what() << std::endl
		);
	} catch (std::invalid_argument &e) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GRandomFactory::producer(): Error!" << std::endl
				<< "Caught std::invalid_argument exception with message" << std::endl
				<< e.what() << std::endl
		);
	} catch (std::system_error& e) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GRandomFactory::producer(): Error!" << std::endl
				<< "Caught std::system_error exception with message" << std::endl
				<< e.what() << std::endl
				<< "which might indicate that a mutex could not be locked." << std::endl
		);
	} catch (...) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GRandomFactory::producer(): Error!" << std::endl
				<< "Caught unkown exception." << std::endl
		);
	}
}

/******************************************************************************/

} /* namespace Hap */
} /* namespace Gem */
