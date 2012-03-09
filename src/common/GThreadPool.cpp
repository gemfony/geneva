/**
 * @file GThreadPool.cpp
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt)
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) com
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
 * http://www.gemfony.com .
 */

#include "common/GThreadPool.hpp"

namespace Gem {
namespace Common {

/***************************************************************************************/
/**
 * Initialization with the "native" number of threads for this architecture
 */
GThreadPool::GThreadPool()
	: errorCounter_(0)
	, submittedJobs_(0)
	, completedJobs_(0)
{
	// Store a worker (a place holder, really) in the io_service_ object
	work_.reset(
		new boost::asio::io_service::work(io_service_)
	);

	gtg_.create_threads (
		boost::bind(
			&boost::asio::io_service::run, &io_service_
		)
		, getNHardwareThreads() // The number of threads native to this architecture (e.g.: 4 cores -> 4 threads)
	);
}

/***************************************************************************************/
/**
 * Initialization with a number of threads
 *
 * @param nThreads The desired number of threads executing work concurrently in the pool
 */
GThreadPool::GThreadPool(const std::size_t& nThreads)
	: errorCounter_(0)
	, submittedJobs_(0)
	, completedJobs_(0)
{
	// Store a worker (a place holder, really) in the io_service_ object
	work_.reset(
		new boost::asio::io_service::work(io_service_)
	);

	gtg_.create_threads (
		boost::bind(
			&boost::asio::io_service::run, &io_service_
		)
		, nThreads
	);
}

/***************************************************************************************/
/**
 * The destructor
 */
GThreadPool::~GThreadPool() {
	work_.reset(); // reset/clear the place holder; io_service.run() will then terminate.
    gtg_.join_all(); // wait for the threads to terminate
}

/***************************************************************************************/
/**
 * Allows to check whether any errors have occurred
 *
 * @return A boolean indicating whether any errors exist
 */
bool GThreadPool::hasErrors() const {
	boost::shared_lock<boost::shared_mutex> lck(error_mutex_);

	if(errorCounter_ > 0) {
		return true;
	}

	return false;
}

/***************************************************************************************/
/**
 * Retrieves the errors
 *
 * @param errorLog The vector to which the errors should be saved
 */
void GThreadPool::getErrors(std::vector<std::string>& errorLog) {
	errorLog = errorLog_;
}

/***************************************************************************************/
/**
 * Clears the error logs
 */
void GThreadPool::clearErrors() {
	boost::lock_guard<boost::shared_mutex> lck(error_mutex_); // We protect against concurrent write access
	errorCounter_ = 0;
	errorLog_.clear();
}

/***************************************************************************************/
/**
 * Waits for all submitted jobs to be cleared from the pool
 *
 * @return A boolean indicating whether errors have occured
 */
bool GThreadPool::wait() {
	// Acquire the lock, then return it as long as the condition hasn't
	// been fulfilled
	boost::lock_guard<boost::mutex> lck(job_counter_mutex_);
	while(submittedJobs_ > completedJobs_) { // Deal with spurious wake-ups
		condition_.wait(job_counter_mutex_);
	}

	// Reset the counters
	submittedJobs_ = 0;
	completedJobs_ = 0;

	return !this->hasErrors();
}

/***************************************************************************************/

} /* namespace Common */
} /* namespace Gem */
