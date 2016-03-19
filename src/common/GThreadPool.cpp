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
	: errorCounter_(0)
   , tasksInFlight_(0)
   , nThreads_(getNHardwareThreads())
   , threads_started_(ATOMIC_FLAG_INIT) // false
{ /* nothing */ }

/******************************************************************************/
/**
 * Initialization with a number of threads. If set to 0, the function will
 * attempt to determine the number of hardware threads.
 *
 * @param nThreads The desired number of threads executing work concurrently in the pool
 */
GThreadPool::GThreadPool(const unsigned int &nThreads)
	: errorCounter_(0)
   , tasksInFlight_(0)
   , nThreads_(nThreads ? nThreads : getNHardwareThreads())
   , threads_started_(ATOMIC_FLAG_INIT) // false
{ /* nothing */ }

/******************************************************************************/
/**
 * The destructor. This function is not thread-safe and does assume that
 * at the time of its call no calls to async_schedule(), setNThreads() or
 * wait() may occur. It does allow the queue to run empty, though.
 */
GThreadPool::~GThreadPool() {
	// Make sure no new jobs may be submitted and let the pool run empty
	boost::unique_lock<boost::mutex> job_lck(task_submission_mutex_);
	{ // Makes sure cnt_lck is released
		// Acquire the lock, then return it as long as the condition hasn't been fulfilled
		boost::unique_lock<boost::mutex> cnt_lck(task_counter_mutex_);
		while (tasksInFlight_.load() > 0) { // Deal with spurious wake-ups
			condition_.wait(cnt_lck);
		}
	}

	// Clear the thread group
	work_.reset(); // This will initiate termination of all threads
	gtg_.join_all(); // wait for the threads to terminate
	gtg_.clearThreads(); // Clear the thread group

	if (this->hasErrors()) {
		boost::unique_lock<boost::mutex> error_lck(error_mutex_);

		std::ostringstream errors;
		std::vector<std::string>::iterator it;
		for (it = errorLog_.begin(); it != errorLog_.end(); ++it) {
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
	boost::unique_lock<boost::mutex> job_lck(task_submission_mutex_);
	// Make sure no threads may be created by other entities
	boost::unique_lock<boost::mutex> tc_lk(thread_creation_mutex_);

	// Check if any work needs to be done
	unsigned int nThreadsLocal = nThreads>0 ? nThreads : getNHardwareThreads();
	if (gtg_.size() == nThreadsLocal) { // We do nothing if we already have the desired size
		return;
	}

	// At this point all potential async_schedule calls, just like the wait() function,
	// must be waiting to acquire the task_submission_mutex_.

	{ // Let the pool run empty
		// Acquire the lock, then return it as long as the condition hasn't been fulfilled
		boost::unique_lock<boost::mutex> cnt_lck(task_counter_mutex_);
		while (tasksInFlight_.load() > 0) { // Deal with spurious wake-ups
			condition_.wait(cnt_lck);
		}
	}

	// If threads were already running, either add new threads or recreate the pool
	if (true == threads_started_.load()) {
		if (nThreadsLocal > nThreads_.load()) { // We simply add the required number of threads
			gtg_.create_threads(
				[this]() { this->io_service_.run(); }
				, nThreadsLocal - nThreads_.load()
			);
		} else { // We need to remove threads and thus reset the entire pool
			work_.reset(); // This will initiate termination of all threads
			gtg_.join_all(); // wait for the threads to terminate
			gtg_.clearThreads(); // Clear the thread group

			// Reset the io_service object, so run may be called again
			io_service_.reset();

			// Store a new worker (a place holder, really) in the io_service_ object
			work_.reset(
				new boost::asio::io_service::work(io_service_)
			);

			// Start the threads
			gtg_.create_threads(
				[&]() { io_service_.run(); }
				, nThreadsLocal
			);
		}
	}

	// Finally set the new number of threads
	nThreads_ = nThreadsLocal;
}

/******************************************************************************/
/**
 * Retrieves the current "true" number of threads being used in the pool
 */
unsigned int GThreadPool::getNThreads() const {
	return boost::numeric_cast<unsigned int>(gtg_.size());
}

/******************************************************************************/
/**
 * Allows to check whether any errors have occurred
 *
 * @return A boolean indicating whether any errors exist
 */
bool GThreadPool::hasErrors() const {
	boost::unique_lock<boost::mutex> error_lck(error_mutex_);
	return (errorCounter_.load() > 0);
}

/******************************************************************************/
/**
 * Retrieves the errors. We return by value in order to maintain
 * thread-safety.
 *
 * @param errorLog The vector to which the errors should be saved
 */
std::vector<std::string> GThreadPool::getErrors() const {
	boost::unique_lock<boost::mutex> error_lck(error_mutex_);
	return errorLog_;
}

/******************************************************************************/
/**
 * Clears the error logs
 */
void GThreadPool::clearErrors() {
	boost::unique_lock<boost::mutex> error_lck(error_mutex_); // Prevent concurrent write access
	errorCounter_ = 0;
	errorLog_.clear();
}

/******************************************************************************/
/**
 * Waits for all submitted jobs to be cleared from the pool. Note that this
 * function may NOT be called from a task running inside of the pool.
 */
void GThreadPool::wait() {
	// Make sure no new jobs may be submitted
	boost::unique_lock<boost::mutex> job_lck(task_submission_mutex_);

	{ // Makes sure cnt_lck is released
		// Acquire the lock, then return it as long as the condition hasn't been fulfilled
		boost::unique_lock<boost::mutex> cnt_lck(task_counter_mutex_);
		while (tasksInFlight_.load() > 0) { // Deal with spurious wake-ups
			condition_.wait(cnt_lck);
		}
	}
}

/******************************************************************************/

} /* namespace Common */
} /* namespace Gem */
