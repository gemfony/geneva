/**
 * @file GThreadPool.cpp
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

#include "common/GThreadPool.hpp"

namespace Gem {
namespace Common {

/******************************************************************************/
/**
 * Initialization with the "native" number of threads for this architecture
 */
GThreadPool::GThreadPool()
	: m_errorCounter(0)
   , m_tasksInFlight(0)
   , m_nThreads(getNHardwareThreads())
   , m_threads_started(ATOMIC_FLAG_INIT) // false
{ /* nothing */ }

/******************************************************************************/
/**
 * Initialization with a number of threads. If set to 0, the function will
 * attempt to determine the number of hardware threads.
 *
 * @param nThreads The desired number of threads executing work concurrently in the pool
 */
GThreadPool::GThreadPool(const unsigned int &nThreads)
	: m_errorCounter(0)
   , m_tasksInFlight(0)
   , m_nThreads(nThreads ? nThreads : getNHardwareThreads())
   , m_threads_started(ATOMIC_FLAG_INIT) // false
{ /* nothing */ }

/******************************************************************************/
/**
 * The destructor. This function is not thread-safe and does assume that
 * at the time of its call no calls to async_schedule(), setNThreads() or
 * wait() may occur. It does allow the queue to run empty, though.
 */
GThreadPool::~GThreadPool() {
	// Make sure no new jobs may be submitted and let the pool run empty
	std::unique_lock<std::mutex> job_lck(m_task_submission_mutex);
	{ // Makes sure cnt_lck is released
		// Acquire the lock, then return it as long as the condition hasn't been fulfilled
		std::unique_lock<std::mutex> cnt_lck(m_task_counter_mutex);
		while (m_tasksInFlight.load() > 0) { // Deal with spurious wake-ups
			m_condition.wait(cnt_lck);
		}
	}

	// Clear the thread group
	m_work.reset(); // This will initiate termination of all threads
	m_gtg.join_all(); // wait for the threads to terminate
	m_gtg.clearThreads(); // Clear the thread group

	if (this->hasErrors()) {
		std::unique_lock<std::mutex> error_lck(m_error_mutex);

		std::ostringstream errors;
		std::vector<std::string>::iterator it;
		for (it = m_errorLog.begin(); it != m_errorLog.end(); ++it) {
			errors << *it << std::endl;
		}
		glogger
		<< "Report from GThreadPool::~GThreadPool(): There were errors during thread execution:" << std::endl
		<< errors.str()
		<< GWARNING;
	}
}

/******************************************************************************/
/**
 * Sets the number of threads currently used. When no threads are running yet,
 * the function will leave starting of threads to async_submit. Otherwise the function
 * will let the pool run empty of jobs. It will then either reset the local thread group,
 * so that all "old" thread objects are gone (needed when the thread pool size is decreased)
 * or simply add new threads. Nothing is done if the desired number of threads already
 * equals the current number of threads. Note that this function may NOT be called
 * from a task running inside of the pool.
 *
 * @param nThreads The desired number of threads
 */
void GThreadPool::setNThreads(unsigned int nThreads) {
	// Make sure no new jobs may be submitted
	std::unique_lock<std::mutex> job_lck(m_task_submission_mutex, std::defer_lock);
	// Make sure no threads may be created by other entities
	std::unique_lock<std::mutex> tc_lk(m_thread_creation_mutex, std::defer_lock);

	// Simulataneously lock both locks
	std::lock(job_lck, tc_lk);

	// Check if any work needs to be done
	unsigned int nThreadsLocal = nThreads>0 ? nThreads : getNHardwareThreads();
	if (m_gtg.size() == nThreadsLocal) { // We do nothing if we already have the desired size
		return;
	}

	// At this point all potential async_schedule calls, just like the wait() function,
	// must be waiting to acquire the m_task_submission_mutex.

	{ // Let the pool run empty
		// Acquire the lock, then return it as long as the condition hasn't been fulfilled
		std::unique_lock<std::mutex> cnt_lck(m_task_counter_mutex);
		while (m_tasksInFlight.load() > 0) { // Deal with spurious wake-ups
			m_condition.wait(cnt_lck);
		}
	}

	// If threads were already running, either add new threads or recreate the pool
	if (true == m_threads_started.load()) {
		if (nThreadsLocal > m_nThreads.load()) { // We simply add the required number of threads
			m_gtg.create_threads(
				[this]() { this->m_io_service.run(); }
				, nThreadsLocal - m_nThreads.load()
			);
		} else { // We need to remove threads and thus reset the entire pool
			m_work.reset(); // This will initiate termination of all threads
			m_gtg.join_all(); // wait for the threads to terminate
			m_gtg.clearThreads(); // Clear the thread group

			// Reset the io_service object, so run may be called again
			m_io_service.reset();

			// Store a new worker (a place holder, really) in the m_io_service object
			m_work.reset(
				new boost::asio::io_service::work(m_io_service)
			);

			// Start the threads
			m_gtg.create_threads(
				[&]() { m_io_service.run(); }
				, nThreadsLocal
			);
		}
	}

	// Finally set the new number of threads
	m_nThreads = nThreadsLocal;
}

/******************************************************************************/
/**
 * Retrieves the current "true" number of threads being used in the pool
 */
unsigned int GThreadPool::getNThreads() const {
	return boost::numeric_cast<unsigned int>(m_gtg.size());
}

/******************************************************************************/
/**
 * Allows to check whether any errors have occurred
 *
 * @return A boolean indicating whether any errors exist
 */
bool GThreadPool::hasErrors() const {
	std::unique_lock<std::mutex> error_lck(m_error_mutex);
	return (m_errorCounter.load() > 0);
}

/******************************************************************************/
/**
 * Retrieves the errors. We return by value in order to maintain
 * thread-safety.
 *
 * @param errorLog The vector to which the errors should be saved
 */
std::vector<std::string> GThreadPool::getErrors() const {
	std::unique_lock<std::mutex> error_lck(m_error_mutex);
	return m_errorLog;
}

/******************************************************************************/
/**
 * Clears the error logs
 */
void GThreadPool::clearErrors() {
	std::unique_lock<std::mutex> error_lck(m_error_mutex); // Prevent concurrent write access
	m_errorCounter = 0;
	m_errorLog.clear();
}

/******************************************************************************/
/**
 * Waits for all submitted jobs to be cleared from the pool. Note that this
 * function may NOT be called from a task running inside of the pool.
 */
void GThreadPool::wait() {
	// Make sure no new jobs may be submitted
	std::unique_lock<std::mutex> job_lck(m_task_submission_mutex);

	{ // Makes sure cnt_lck is released
		// Acquire the lock, then return it as long as the condition hasn't been fulfilled
		std::unique_lock<std::mutex> cnt_lck(m_task_counter_mutex);
		while (m_tasksInFlight.load() > 0) { // Deal with spurious wake-ups
			m_condition.wait(cnt_lck);
		}
	}
}

/******************************************************************************/

} /* namespace Common */
} /* namespace Gem */
