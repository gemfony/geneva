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
{ /* nothing */ }

/******************************************************************************/
/**
 * Initialization with a number of threads. If set to 0, the function will
 * attempt to determine the number of hardware threads.
 *
 * @param nThreads The desired number of threads executing work concurrently in the pool
 */
GThreadPool::GThreadPool(const unsigned int& nThreads)
	: errorCounter_(0)
	, tasksInFlight_(0)
   , nThreads_(nThreads?nThreads:getNHardwareThreads())
{ /* nothing */ }

/******************************************************************************/
/**
 * The destructor
 */
GThreadPool::~GThreadPool() {
   // Make sure the pool is empty
   this->setNThreads(0);

   if(this->hasErrors()) {
      std::ostringstream errors;
      std::vector<std::string>::iterator it;
      for(it=errorLog_.begin(); it!=errorLog_.end(); ++it) {
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
 * Sets the number of threads currently used. The function will add workers
 * to the thread group if threads should be added. Otherwise it will reset the
 * pool and fill it anew.
 */
void GThreadPool::setNThreads(unsigned int nThreads) {
   // Prevent thread starts
   boost::shared_lock< boost::shared_mutex > lck(threads_started_mutex_);

   unsigned int nThreadsLocal = nThreads?nThreads:getNHardwareThreads();
   if(true==threads_started_) {
      if(gtg_.size() == nThreadsLocal) { // We already have the desired size
         nThreads_ = nThreadsLocal;
         return;
      } else if(nThreadsLocal > gtg_.size()) { // Add the missing threads
         gtg_.create_threads (
            boost::bind(
               &boost::asio::io_service::run
               , &io_service_
            )
            , nThreadsLocal - gtg_.size()
         );
      } else { // nThreadsLocal < gtg_.size(); Recreate the entire pool
         // Make sure no new jobs may be submitted
         boost::unique_lock<boost::mutex> job_lck(task_submission_mutex_);

         // First wait for the pool to run empty, so we do not loose jobs
         boost::unique_lock<boost::mutex> cnt_lck(task_counter_mutex_);
         while(tasksInFlight_ > 0) { // Deal with spurious wake-ups
            condition_.wait(task_counter_mutex_);
         }

         // reset/clear the place holder; io_service.run() will then terminate.
         work_.reset();

         // If there are threads in the pool, reset them and clear the pool
         if(gtg_.size() > 0) {
            gtg_.join_all(); // wait for the threads to terminate
            gtg_.clearThreads(); // Clear the thread group
         }

         // We are done if nThreads == 0
         if(0 == nThreads) { // Note: NOT nThreadsLocal
            return;
         }

         // Store a new worker (a place holder, really) in the io_service_ object.
         // This will prevent new threads from stopping
         work_.reset(
            new boost::asio::io_service::work(io_service_)
         );

         // Create the desired number of threads
         gtg_.create_threads (
            boost::bind(
               &boost::asio::io_service::run
               , &io_service_
            )
            , nThreadsLocal
         );
      }
   }

   nThreads_ = nThreadsLocal;
}

/******************************************************************************/
/**
 * Retrieves the current number of threads being used in the pool
 */
unsigned int GThreadPool::getNThreads() const {
   return gtg_.size();
}

/******************************************************************************/
/**
 * Allows to check whether any errors have occurred
 *
 * @return A boolean indicating whether any errors exist
 */
bool GThreadPool::hasErrors() const {
	boost::shared_lock<boost::shared_mutex> error_lck(error_mutex_);

	if(errorCounter_ > 0) {
		return true;
	}

	return false;
}

/******************************************************************************/
/**
 * Retrieves the errors
 *
 * @param errorLog The vector to which the errors should be saved
 */
void GThreadPool::getErrors(std::vector<std::string>& errorLog) const {
   boost::shared_lock<boost::shared_mutex> error_lck(error_mutex_);

	errorLog = errorLog_;
}

/******************************************************************************/
/**
 * Clears the error logs
 */
void GThreadPool::clearErrors() {
   boost::unique_lock<boost::shared_mutex> error_lck(error_mutex_); // Prevent concurrent write access
	errorCounter_ = 0;
	errorLog_.clear();
}

/******************************************************************************/
/**
 * Waits for all submitted jobs to be cleared from the pool
 *
 * @return A boolean indicating whether errors have occurred
 */
bool GThreadPool::wait() {
   // Make sure no new jobs may be submitted
   boost::unique_lock<boost::mutex> job_lck(task_submission_mutex_);

   { // Makes sure cnt_lck is released
      // Acquire the lock, then return it as long as the condition hasn't been fulfilled
      boost::unique_lock<boost::mutex> cnt_lck(task_counter_mutex_);
      while(tasksInFlight_ > 0) { // Deal with spurious wake-ups
         condition_.wait(task_counter_mutex_);
      }
   }

	return !this->hasErrors();
}

/******************************************************************************/

} /* namespace Common */
} /* namespace Gem */
