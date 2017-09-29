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
	if(!m_seeding_has_started || m_seed_cit==m_seed_collection.end()) {
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
void GRandomFactory::returnUsedPackage(std::unique_ptr<random_container>& p) {
	// We try to add the item to the m_p_ret_bfr queue.
	if(!m_p_ret_bfr.try_push(p)) {
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
		std::uint16_t nProducerThreads_local =
			(nProducerThreads > 0) ? nProducerThreads : (boost::numeric_cast<std::uint16_t>(Gem::Common::getNHardwareThreads(DEFAULT01PRODUCERTHREADS)));

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
	} else { // Double-checked locking pattern
		// Here it appears that no threads were running. We do need to check again, though (DLCP)
		std::unique_lock<std::mutex> tc_lk(m_thread_creation_mutex);
		// Make a suggestion for the number of threads, if requested
		std::uint16_t nProducerThreads_local =
			(nProducerThreads > 0) ? nProducerThreads : (boost::numeric_cast<std::uint16_t>(Gem::Common::getNHardwareThreads(DEFAULT01PRODUCERTHREADS)));

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
	if (!m_threads_started) {
		std::unique_lock<std::mutex> tc_lk(m_thread_creation_mutex);
		if (!m_threads_started) { // double checked locking pattern
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
	if(!m_p_fresh_bfr.pop_and_wait(
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

		while(!m_threads_stop_requested) {
			// First we try to retrieve a "recycled" item from the m_p_ret_bfr buffer. If this
			// fails (likely because the buffer is empty), we create a new item instead
			if(m_p_ret_bfr.try_pop(p)) {

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

				// Replace "used" random numbers with new ones
				p->refresh(mt);
			} else { // O.k., so we need to create a new container
				p.reset(new random_container(mt));
			}

			// Try to submit the item and check for termination conditions along the way
			while(!m_threads_stop_requested) {
				if(!m_p_fresh_bfr.try_push(p)){
#ifdef DEBUG
					// p should never be empty here
					if(!p) {
						glogger
							<< "In RandomFactory::producer(): Error!" << std::endl
							<< "Got empty pointer after unsuccesfull submission" << std::endl
							<< GEXCEPTION;
					}
#endif
					std::this_thread::yield();
					continue;
				} else { // We have submitted the item -- stop the inner loop
					break;
				}
			}
		}
	} catch (std::bad_alloc &e) {
		glogger
			<< "In GRandomFactory::producer(): Error!" << std::endl
			<< "Caught std::bad_alloc exception with message" << std::endl
			<< e.what() << std::endl
			<< GEXCEPTION;
	} catch (std::invalid_argument &e) {
		glogger
			<< "In GRandomFactory::producer(): Error!" << std::endl
			<< "Caught std::invalid_argument exception with message" << std::endl
			<< e.what() << std::endl
			<< GEXCEPTION;
	} catch (std::system_error& e) {
		glogger
			<< "In GRandomFactory::producer(): Error!" << std::endl
			<< "Caught std::system_error exception with message" << std::endl
			<< e.what() << std::endl
			<< "which might indicate that a mutex could not be locked." << std::endl
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
