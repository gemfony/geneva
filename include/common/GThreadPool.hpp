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

#pragma once

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard header files go here
#include <functional>
#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <type_traits>

// Boost header files go here
#include <boost/asio.hpp>
#include <boost/utility.hpp>
#include <boost/exception/all.hpp>

// Geneva header files go here
#include "common/GLogger.hpp"
#include "common/GExceptions.hpp"
#include "common/GErrorStreamer.hpp"
#include "common/GThreadGroup.hpp"
#include "common/GCommonEnums.hpp"
#include "common/GCommonHelperFunctions.hpp"

namespace Gem::Common {

/******************************************************************************/
/**
 * This class implements a simple thread pool, based on the facilities already
 * provided by Boost.ASIO . This is meant as a drop-in replacement, until a more
 * versatile thread pool becomes commonly available in Boost. The class is not
 * meant to be copyable, as this concept does not make much sense for running
 * threads.
 */
class GThreadPool {
public:
	 /** @brief Deleted default constructor enforces setting of the number of threads */
	 G_API_COMMON GThreadPool() = delete;
	 /** @brief Initialization with a number of threads */
	 explicit G_API_COMMON GThreadPool(unsigned int);
	 /** @brief The destructor */
	 G_API_COMMON ~GThreadPool();

	 /** @brief Sets the number of threads currently used */
	 G_API_COMMON void setNThreads(unsigned int);
	 /** @brief Retrieves the current number of threads being used in the pool */
	 G_API_COMMON unsigned int getNThreads() const;

	 /** @brief Blocks until all submitted jobs have been cleared from the pool */
	 G_API_COMMON void wait();

	 /***************************************************************************/
	 // Some deleted functions and constructors
	 G_API_COMMON GThreadPool(const GThreadPool&) = delete; // deleted copy constructor
	 G_API_COMMON GThreadPool& operator=(GThreadPool&) = delete; // deleted assignment operator
	 G_API_COMMON GThreadPool(const GThreadPool&&) = delete; // deleted move constructor
	 G_API_COMMON GThreadPool& operator=(GThreadPool&&) = delete; // deleted move-assignment operator

	 /***************************************************************************/
	 /**
	  * Submits the task to Boost.ASIO's io_service. This function will return immediately,
	  * before the completion of the task. This overload deals with tasks that have a
	  * void return-type.
	  *
	  * @param f The function to be executed by the threads in the pool
	  * @param args A parameter pack -- the arguments of f
	  * @return A std::future holding any exceptions that have occurred
	  */
	 template <typename F, typename... Args>
	 auto async_schedule(
		 F &&f
		 , Args &&... args
		 , typename std::enable_if<std::is_void<typename std::result_of<F(Args...)>::type>::value>::type *dummy = nullptr
	 ) -> std::future<typename std::result_of<F(Args...)>::type> {
		 // We may only submit new jobs if job_lck can be acquired. This is important
		 // so we have a means of letting the submission queue run empty.
		 std::unique_lock<std::mutex> job_lck(m_task_submission_mutex);

		 // Determine whether threads have already been started
		 // If not, start them. Access to the threads is blocked by job_lck
		 if (not m_threads_started) {
			 std::unique_lock<std::mutex> tc_lk(m_thread_creation_mutex);
			 if (not m_threads_started) { // double checked locking pattern
				 // Some error checks
				 if(0==m_nThreads.load()) {
					 throw gemfony_exception(
						 g_error_streamer(DO_LOG, time_and_place)
							 << "In GThreadPool::async_schedule(F f): Error!" << std::endl
							 << "The number of threads is set to 0" << std::endl
					 );
				 }
				 if(m_gtg.size() > 0) {
					 throw gemfony_exception(
						 g_error_streamer(DO_LOG, time_and_place)
							 << "In GThreadPool::async_schedule(F f): Error!" << std::endl
							 << "The thread group already has entries, although" << std::endl
							 << "m_threads_started is set to false" << std::endl
					 );
				 }

				 // Store a worker (a place holder, really) in the m_io_service object
				 m_work_guard_ptr.reset(
					 new boost::asio::io_context::work(m_io_context)
				 );

				 // No need to let the threads join, as none were running so far

				 m_gtg.create_threads(
					 [this]() { this->m_io_context.run(); }
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
		 // cannot "look" into the io_service, we need an external counter that
		 // is incremented upon submission, not at start of execution. Otherwise
		 // we might submit too many jobs.
		 {
			 std::unique_lock<std::mutex> cnt_lck(m_task_counter_mutex);
			 m_tasksInFlight++;
		 }

		 using result_type = typename std::result_of<F(Args&&...)>::type;
		 auto promise_ptr = std::make_shared<std::promise<result_type>>();
		 std::future<result_type> result = promise_ptr->get_future();

		 boost::asio::post(
		 	m_io_context
			, [this, promise_ptr, f = std::bind<result_type>(std::forward<F>(f), std::forward<Args>(args)...)]() {
				 try {
					 f();
				 } catch(boost::exception& e) {
					 // Convert to a std::runtime_exception
					 std::runtime_error r(boost::diagnostic_information(e));

					 try { // Whatever was thrown may be stored in the promise
						 promise_ptr->set_exception(std::make_exception_ptr(r));
					 } catch(...) { // Unfortunately set_exception() may throw too
						 glogger
							 << "In GThreadPool::async_schedule(/void/)::" << std::endl
							 << "promise.set_exception() has thrown." << std::endl
							 << "We cannot continue" << std::endl
							 << GTERMINATION;
					 }
				 } catch(...) {
					 try { // Whatever was thrown may be stored in the promise
						 promise_ptr->set_exception(std::current_exception());
					 } catch(...) { // Unfortunately set_exception() may throw too
						 glogger
							 << "In GThreadPool::async_schedule(/void/):" << std::endl
							 << "promise.set_exception() has thrown." << std::endl
							 << "We cannot continue" << std::endl
							 << GTERMINATION;
					 }
				 }


				 { // Update the submission counter -- we need an external means to check whether the pool has run empty
					 std::unique_lock<std::mutex> cnt_lck(m_task_counter_mutex);
#ifdef DEBUG
					 if(0==m_tasksInFlight.load()) {
						 glogger
							 << "In GThreadPool::async_schedule(/void/):" << std::endl
							 << "Trying to decrement a task counter that is already 0" << std::endl
							 << "We cannot continue"
							 << GTERMINATION;
					 }
#endif /* DEBUG */
					 m_tasksInFlight--;
					 cnt_lck.unlock();
					 m_condition.notify_one();
				 }
			 }
		 );

		 return result;
	 };

	 /***************************************************************************/
	 /**
	  * Submits the task to Boost.ASIO's io_service. This function will return immediately,
	  * before the completion of the task. This overload deals with tasks that have a
	  * non-void return-type.
	  *
	  * @param f The function to be executed by the threads in the pool
	  * @param args A parameter pack -- the arguments of f
	  * @return A std::future holding the results of f and any exceptions that have occurred
	  */
	 template <typename F, typename... Args>
	 auto async_schedule(
		 F &&f
		 , Args &&... args
		 , typename std::enable_if<not std::is_void<typename std::result_of<F(Args...)>::type>::value>::type *dummy = nullptr
	 ) -> std::future<typename std::result_of<F(Args...)>::type> {
		 // We may only submit new jobs if job_lck can be acquired. This is important
		 // so we have a means of letting the submission queue run empty.
		 std::unique_lock<std::mutex> job_lck(m_task_submission_mutex);

		 // Determine whether threads have already been started
		 // If not, start them. Access to the threads is blocked by job_lck
		 if (not m_threads_started) {
			 std::unique_lock<std::mutex> tc_lk(m_thread_creation_mutex);
			 if (not m_threads_started) { // double checked locking pattern
				 // Some error checks
				 if(0==m_nThreads.load()) {
					 throw gemfony_exception(
						 g_error_streamer(DO_LOG, time_and_place)
							 << "In GThreadPool::async_schedule(F f): Error!" << std::endl
							 << "The number of threads is set to 0" << std::endl
					 );
				 }
				 if(m_gtg.size() > 0) {
					 throw gemfony_exception(
						 g_error_streamer(DO_LOG, time_and_place)
							 << "In GThreadPool::async_schedule(F f): Error!" << std::endl
							 << "The thread group already has entries, although" << std::endl
							 << "m_threads_started is set to false" << std::endl
					 );
				 }

				 // Store a worker (a place holder, really) in the m_io_service object
				 m_work_guard_ptr.reset(
					 new boost::asio::io_context::work(m_io_context)
				 );

				 // No need to let the threads join, as none were running so far

				 m_gtg.create_threads(
					 [this]() { this->m_io_context.run(); }
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
		 // cannot "look" into the io_service, we need an external counter that
		 // is incremented upon submission, not at start of execution. Otherwise
		 // we might submit too many jobs.
		 {
			 std::unique_lock<std::mutex> cnt_lck(m_task_counter_mutex);
			 m_tasksInFlight++;
		 }

		 using result_type = typename std::result_of<F(Args&&...)>::type;
		 auto promise_ptr = std::make_shared<std::promise<result_type>>();
		 std::future<result_type> result = promise_ptr->get_future();

		 boost::asio::post(
		 	m_io_context
		 	, [this, promise_ptr, f = std::bind<result_type>(std::forward<F>(f), std::forward<Args>(args)...)]() {
				 try {
					 promise_ptr->set_value(f());
				 } catch(boost::exception& e) {
					 // Convert to a std::runtime_exception
					 std::runtime_error r(boost::diagnostic_information(e));

					 try { // Whatever was thrown may be stored in the promise
						 promise_ptr->set_exception(std::make_exception_ptr(r));
					 } catch(...) { // Unfortunately set_exception() may throw too
						 glogger
							 << "In GThreadPool::async_schedule(/non-void/)::" << std::endl
							 << "promise.set_exception() has thrown." << std::endl
							 << "We cannot continue" << std::endl
							 << GTERMINATION;
					 }
				 } catch(...) {
					 try { // Whatever was thrown may be stored in the promise
						 promise_ptr->set_exception(std::current_exception());
					 } catch(...) { // Unfortunately set_exception() may throw too
						 glogger
							 << "In GThreadPool::async_schedule(/non-void/):" << std::endl
							 << "promise.set_exception() has thrown." << std::endl
							 << "We cannot continue" << std::endl
							 << GTERMINATION;
					 }
				 }

				 { // Update the submission counter -- we need an external means to check whether the pool has run empty
					 std::unique_lock<std::mutex> cnt_lck(m_task_counter_mutex);
#ifdef DEBUG
					 if(0==m_tasksInFlight.load()) {
						 glogger
							 << "In GThreadPool::async_schedule(/non-void/):" << std::endl
							 << "Trying to decrement a task counter that is already 0" << std::endl
							 << "We cannot continue"
							 << GTERMINATION;
					 }
#endif /* DEBUG */
					 m_tasksInFlight--;
					 m_condition.notify_one();
				 }
			 }
		 );

		 return result;
	 };

private:
	 /***************************************************************************/

	 boost::asio::io_context m_io_context; ///< Manages the concurrent thread execution
	 std::shared_ptr<boost::asio::io_context::work> m_work_guard_ptr; ///< A place holder ensuring that the io_service doesn't stop prematurely

	 GThreadGroup m_gtg; ///< Holds the actual threads

	 std::atomic<std::uint32_t> m_tasksInFlight {0};  ///< The number of jobs that have been submitted in this round
	 std::mutex m_task_counter_mutex; ///< Protects access to the "submitted" job counter

	 /// Allows to prevent further job submissions, particularly when waiting for the pool to clear or when resetting the pool
	 std::mutex m_task_submission_mutex;

	 std::mutex m_thread_creation_mutex; ///< Synchronization of access to the threads_started_ variable

	 ///< Protects the job counter, so we may let the pool run empty
	 std::condition_variable_any m_condition;

	 std::atomic<unsigned int> m_nThreads; ///< The number of concurrent threads in the pool
	 std::atomic<bool> m_threads_started{false}; ///< Indicates whether threads have already been started
};

/******************************************************************************/

} /* namespace Gem::Common */
