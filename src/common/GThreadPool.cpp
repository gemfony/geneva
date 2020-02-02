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

#include "common/GThreadPool.hpp"

namespace Gem {
namespace Common {

/******************************************************************************/
/**
 * Initialization with a number of threads.
 *
 * @param nThreads The desired number of threads executing work concurrently in the pool
 */
GThreadPool::GThreadPool(unsigned int nThreads)
   : m_nThreads(nThreads > 0 ? nThreads : DEFAULTNHARDWARETHREADS)
{
	if(0 == nThreads) {
		glogger
			<< "In GThreadPool::GThreadPool(unsigned int const &nThreads):" << std::endl
			<< "User requested nThreads == 0. nThreads was reset to the default " << DEFAULTNHARDWARETHREADS << std::endl
			<< GWARNING;
	}
}

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
	m_work_guard_ptr.reset(); // This will initiate termination of all threads
	m_gtg.join_all(); // wait for the threads to terminate
	m_gtg.clearThreads(); // Clear the thread group
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
	if (m_gtg.size() == nThreads) { // We do nothing if we already have the desired size
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
	if (m_threads_started) {
		if (nThreads > m_nThreads.load()) { // We simply add the required number of threads
			m_gtg.create_threads(
				[this]() { this->m_io_context.run(); }
				, nThreads - m_nThreads.load()
			);
		} else { // We need to remove threads and thus reset the entire pool
			m_work_guard_ptr.reset(); // This will initiate termination of all threads
			m_gtg.join_all(); // wait for the threads to terminate
			m_gtg.clearThreads(); // Clear the thread group

			// Reset the io_service object, so run may be called again
			m_io_context.restart();

			// Store a new worker (a place holder, really) in the m_io_service object
			m_work_guard_ptr.reset(
				new boost::asio::io_context::work(m_io_context)
			);

			// Start the threads
			m_gtg.create_threads(
				[&]() { m_io_context.run(); }
				, nThreads
			);
		}
	}

	// Finally set the new number of threads
	m_nThreads = nThreads;
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
 * Waits for all submitted jobs to be cleared from the pool. Note that this
 * function may NOT be called from a task running inside of the pool.
 */

void GThreadPool::wait() {
	// Make sure no new jobs may be submitted
	std::unique_lock<std::mutex> job_lck(m_task_submission_mutex);

	{ // Makes sure cnt_lck is released
		// Acquire the lock, then return it as long as the condition hasn't been fulfilled
		std::unique_lock<std::mutex> cnt_lck(m_task_counter_mutex);
		m_condition.wait(
			cnt_lck
			, [this]() -> bool { return (m_tasksInFlight.load() == 0); }
		);
	}
}

/******************************************************************************/

} /* namespace Common */
} /* namespace Gem */
