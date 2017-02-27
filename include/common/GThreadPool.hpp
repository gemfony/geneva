/**
 * @file GThreadPool.hpp
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

// Standard header files go here
#include <functional>
#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>

// Boost header files go here
#include <boost/asio/io_service.hpp>
#include <boost/utility.hpp>
#include <boost/exception/all.hpp>

#ifndef GTHREADPOOL_HPP_
#define GTHREADPOOL_HPP_


// Geneva header files go here
#include "common/GLogger.hpp"
#include "common/GStdThreadGroup.hpp"
#include "common/GHelperFunctions.hpp"

namespace Gem {
namespace Common {

/******************************************************************************/
/**
 * This class implements a simple thread pool, based on the facilities already
 * provided by Boost.ASIO . This is meant as a drop-in replacement, until a more
 * versatile thread pool becomes commonly available in Boost. The class is not
 * meant to be copyable, as this concept does not make much sense for running
 * threads.
 */
class GThreadPool
	: private boost::noncopyable // make sure the pool cannot be copied
{
public:
	 /** @brief Initialization with the "native" number of threads for this architecture */
	 G_API_COMMON GThreadPool();
	 /** @brief Initialization with a number of threads */
	 G_API_COMMON GThreadPool(const unsigned int &);
	 /** @brief The destructor */
	 G_API_COMMON ~GThreadPool();

	 /** @brief Sets the number of threads currently used */
	 G_API_COMMON void setNThreads(unsigned int);
	 /** @brief Retrieves the current number of threads being used in the pool */
	 G_API_COMMON unsigned int getNThreads() const;

	 /** @brief Blocks until all submitted jobs have been cleared from the pool */
	 G_API_COMMON void wait();

	 /** @brief Allows to check whether any errors have occurred */
	 G_API_COMMON bool hasErrors() const;
	 /** @brief Retrieves the errors */
	 G_API_COMMON std::vector<std::string> getErrors() const;
	 /** @brief Clears the error logs */
	 G_API_COMMON void clearErrors();

	 /***************************************************************************/
	 /**
	  * Submits the task to Boost.ASIO's io_service. This function will return immediately,
	  * before the completion of the task.
	  *
	  * @param f The function to be executed by the threads in the pool
	  */
	 template<typename F>
	 void async_schedule(F f) {
		 // We may only submit new jobs if job_lck can be acquired. This is
		 // important so we have a means of letting the submission queue run
		 // empty or of resetting the internal thread group.
		 std::unique_lock<std::mutex> job_lck(m_task_submission_mutex);

		 // Determine whether threads have already been started
		 // If not, start them. Access to the threads is blocked by job_lck
		 if (false==m_threads_started.load()) {
			 std::unique_lock<std::mutex> tc_lk(m_thread_creation_mutex);
			 if (false==m_threads_started.load()) { // double checked locking pattern
				 // Some error checks
				 if(0==m_nThreads.load()) {
					 glogger
					 << "In GThreadPool::async_schedule(F f): Error!" << std::endl
					 << "The number of threads is set to 0" << std::endl
					 << GEXCEPTION;
				 }
				 if(m_gtg.size() > 0) {
					 glogger
					 << "In GThreadPool::async_schedule(F f): Error!" << std::endl
					 << "The thread group already has entries, although" << std::endl
					 << "m_threads_started is set to false" << std::endl
					 << GEXCEPTION;
				 }

				 // Store a worker (a place holder, really) in the m_io_service object
				 m_work.reset(
					 new boost::asio::io_service::work(m_io_service)
				 );

				 // No need to let the threads join, as none were running so far

				 m_gtg.create_threads(
					 [this]() { this->m_io_service.run(); }
					 , m_nThreads.load()
				 );

				 m_threads_started = true;
			 }
		 }

		 // Update the task counter. NOTE: This needs to happen here
		 // and not in taskWrapper. m_tasksInFlight helps the wait()-function
		 // to determine whether any jobs have been submitted to the Boost.ASIO
		 // ioservice that haven't been processed yet. taskWrapper will
		 // only start execution when it is assigned to a thread. As we
		 // cannot "look" into ioservice, we need an external counter that
		 // is incremented upon submission, not start of execution.
		 {
			 std::unique_lock<std::mutex> cnt_lck(m_task_counter_mutex);
			 m_tasksInFlight++;
		 }

		 // Finally submit to the io_service
		 m_io_service.post(
			 [f, this]() {
				 this->taskWrapper<F>(f);
			 }
		 );
	 }

private:
	 /***************************************************************************/
	 /**
	  * A wrapper for the thread execution that takes care of exceptions thrown by our
	  * function and allows to track how many jobs are still pending
	  *
	  * @param f A function to be executed by the threads in the pool
	  */
	 template<typename F>
	 void taskWrapper(F f) {
		 try { // Execute the actual worker task
			 f();
		 } catch (Gem::Common::gemfony_error_condition &e) {
			 // Extract the error
			 std::ostringstream error;
			 error
			 << "In GThreadPool::taskWrapper(F f): Caught Gem::Common::gemfony_error_condition with message" << std::endl
			 << e.what() << std::endl;

			 { // Store the error for later reference
				 std::unique_lock<std::mutex> error_lck(m_error_mutex); // We protect against concurrent write access
				 m_errorCounter++;
				 m_errorLog.push_back(error.str());
			 }
		 } catch (boost::exception &e) {
			 // Extract the error
			 std::ostringstream error;
			 error
			 << "In GThreadPool::taskWrapper(): Caught boost::exception" << std::endl
			 << boost::diagnostic_information(e) << std::endl;

			 { // Store the error for later reference
				 std::unique_lock<std::mutex> error_lck(m_error_mutex); // We protect against concurrent write access
				 m_errorCounter++;
				 m_errorLog.push_back(error.str());
			 }
		 } catch (std::exception &e) {
			 // Extract the error
			 std::ostringstream error;
			 error
			 << "In GThreadPool::taskWrapper(F f): Caught std::exception with message" << std::endl
			 << e.what() << std::endl;

			 { // Store the error for later reference
				 std::unique_lock<std::mutex> error_lck(m_error_mutex); // We protect against concurrent write access
				 m_errorCounter++;
				 m_errorLog.push_back(error.str());
			 }
		 } catch (...) {
			 std::ostringstream error;
			 error
			 << "GThreadPool::taskWrapper(F f): Caught unknown exception" << std::endl;

			 { // Store the error for later reference
				 std::unique_lock<std::mutex> error_lck(m_error_mutex); // We protect against concurrent write access
				 m_errorCounter++;
				 m_errorLog.push_back(error.str());
			 }
		 }

		 { // Update the submission counter -- we need an external means to check whether the pool has run empty
			 std::unique_lock<std::mutex> cnt_lck(m_task_counter_mutex);
#ifdef DEBUG
			 if(0==m_tasksInFlight.load()) {
				 glogger
				 << "In GThreadPool::taskWrapper(F f): Error!" << std::endl
				 << "Trying to decrement a task counter that is already 0" << std::endl
				 << GWARNING;

				 // We do not use an exception here, as taskWrapper runs inside of a thread
			 }
#endif /* DEBUG */
			 m_tasksInFlight--;
			 m_condition.notify_one();
		 }
	 }

	 /***************************************************************************/

	 boost::asio::io_service m_io_service; ///< Manages the concurrent thread execution
	 std::shared_ptr <boost::asio::io_service::work> m_work; ///< A place holder ensuring that the io_service doesn't stop prematurely

	 GStdThreadGroup m_gtg; ///< Holds the actual threads

	 std::atomic<std::uint32_t> m_errorCounter; ///< The number of exceptions thrown by the pay load
	 std::vector<std::string> m_errorLog; ///< Holds error descriptions emitted by the work load
	 mutable std::mutex m_error_mutex; ///< Protects access to the error log and error counter; mutable, as "hasErrors() is const

	 std::atomic<std::uint32_t> m_tasksInFlight;  ///< The number of jobs that have been submitted in this round
	 std::mutex m_task_counter_mutex; ///< Protects access to the "submitted" job counter

	 /// Allows to prevent further job submissions, particularly when waiting for the pool to clear or when resetting the pool
	 std::mutex m_task_submission_mutex;

	 std::mutex m_thread_creation_mutex; ///< Synchronization of access to the threads_started_ variable

	 ///< Protects the job counter, so we may let the pool run empty
	 std::condition_variable_any m_condition;

	 std::atomic<unsigned int> m_nThreads; ///< The number of concurrent threads in the pool
	 std::atomic<bool> m_threads_started; ///< Indicates whether threads have already been started
};

/******************************************************************************/

} /* namespace Common */
} /* namespace Gem */

#endif /* GTHREADPOOL_HPP_ */
