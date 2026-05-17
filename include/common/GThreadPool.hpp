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
 * See the NOTICE file in the top-level directory of the Geneva library
 * collection for a list of contributors and copyright information.
 *
 ********************************************************************************/

#pragma once

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard header files go here
#include <atomic>
#include <concepts>
#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <thread>
#include <type_traits>

// Boost header files go here
#include <boost/asio.hpp>

// Geneva header files go here
#include "common/GCommonEnums.hpp"
#include "common/GCommonHelperFunctions.hpp"
#include "common/GErrorStreamer.hpp"
#include "common/GExceptions.hpp"
#include "common/GLogger.hpp"
#include "common/GThreadGroup.hpp"

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
    GThreadPool() = delete;
    /** @brief Initialization with a number of threads */
    explicit GThreadPool(unsigned int);
    /** @brief The destructor */
    ~GThreadPool();

    /** @brief Sets the number of threads currently used */
    void setNThreads(unsigned int);
    /** @brief Retrieves the current number of threads being used in the pool */
    unsigned int getNThreads() const;

    /** @brief Blocks until all submitted jobs have been cleared from the pool */
    void wait();

    /***************************************************************************/
    // Some deleted functions and constructors
    GThreadPool(const GThreadPool &) = delete;      // deleted copy constructor
    GThreadPool &operator=(GThreadPool &) = delete; // deleted assignment operator
    GThreadPool(const GThreadPool &&) = delete;     // deleted move constructor
    GThreadPool &
    operator=(GThreadPool &&) = delete; // deleted move-assignment operator

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
        requires std::same_as<std::invoke_result_t<F, Args...>, void>
    auto async_schedule(F &&f, Args &&...args) -> std::future<std::invoke_result_t<F, Args...>> {
        // We may only submit new jobs if job_lck can be acquired. This is important
        // so we have a means of letting the submission queue run empty.
        std::unique_lock<std::mutex> job_lck(task_submission_mutex_);

        // Determine whether threads have already been started
        // If not, start them. Access to the threads is blocked by job_lck
        if(not threads_started_) {
            std::unique_lock<std::mutex> tc_lk(thread_creation_mutex_);
            if(not threads_started_) {
                // double checked locking pattern
                // Some error checks
                if(0 == n_threads_.load()) {
                    throw geneva_exception(
                        g_error_streamer(DO_LOG, time_and_place)
                        << "In GThreadPool::async_schedule(F f): Error!" << '\n'
                        << "The number of threads is set to 0" << '\n'
                    );
                }
                if(gtg_.size() > 0) {
                    throw geneva_exception(
                        g_error_streamer(DO_LOG, time_and_place)
                        << "In GThreadPool::async_schedule(F f): Error!" << '\n'
                        << "The thread group already has entries, although" << '\n'
                        << "threads_started_ is set to false" << '\n'
                    );
                }

                // Store a worker (a place holder, really) in the io_service_ object
                work_guard_ptr_ = std::make_shared<
                    boost::asio::executor_work_guard<boost::asio::io_context::executor_type>>(
                    io_context_.get_executor()
                );

                // No need to let the threads join, as none were running so far

                gtg_.create_threads([this]() { this->io_context_.run(); }, n_threads_.load());

                threads_started_ = true;
            }
        }

        // Update the task counter. NOTE: This needs to happen here
        // and not in taskWrapper. tasks_in_flight_ helps the wait()-function
        // to determine whether any jobs have been submitted to the Boost.ASIO
        // ioservice that haven't been processed yet. taskWrapper will
        // only start execution when it is assigned to a thread. As we
        // cannot "look" into the io_service, we need an external counter that
        // is incremented upon submission, not at start of execution. Otherwise
        // we might submit too many jobs.
        {
            std::unique_lock<std::mutex> cnt_lck(task_counter_mutex_);
            tasks_in_flight_++;
        }

        using result_type = std::invoke_result_t<F, Args&&...>;
        auto promise_ptr = std::make_shared<std::promise<result_type>>();
        std::future<result_type> result = promise_ptr->get_future();

        boost::asio::post(
            io_context_,
            [this,
             promise_ptr,
             f = std::bind<result_type>(std::forward<F>(f), std::forward<Args>(args)...)]() {
                try {
                    f();
                    // Required for the void specialisation: without this the
                    // future's shared state is destroyed without ever being
                    // satisfied, and callers calling `.get()` / `.wait()` see
                    // a std::future_error("Broken promise"). The non-void
                    // overload below already does this implicitly through
                    // set_value(f()).
                    promise_ptr->set_value();
                }
                catch(
                    ...
                ) // NOLINT(bugprone-empty-catch) — propagates via set_exception or terminates
                {
                    try {
                        // Whatever was thrown may be stored in the promise
                        promise_ptr->set_exception(std::current_exception());
                    }
                    catch(
                        ...
                    ) // NOLINT(bugprone-empty-catch) — logs and terminates via GTERMINATION
                    {
                        // Unfortunately set_exception() may throw too
                        glogger << "In GThreadPool::async_schedule(/void/):" << '\n'
                                << "promise.set_exception() has thrown." << '\n'
                                << "We cannot continue" << '\n'
                                << GTERMINATION;
                    }
                }

                {
                    // Update the submission counter -- we need an external means to check whether the pool has run empty
                    std::unique_lock<std::mutex> cnt_lck(task_counter_mutex_);
#ifdef DEBUG
                    if(0 == tasks_in_flight_.load()) {
                        glogger << "In GThreadPool::async_schedule(/void/):" << '\n'
                                << "Trying to decrement a task counter that is already 0"
                                << '\n'
                                << "We cannot continue" << GTERMINATION;
                    }
#endif /* DEBUG */
                    tasks_in_flight_--;
                    cnt_lck.unlock();
                    condition_.notify_one();
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
        requires (!std::same_as<std::invoke_result_t<F, Args...>, void>)
    auto async_schedule(F &&f, Args &&...args) -> std::future<std::invoke_result_t<F, Args...>> {
        // We may only submit new jobs if job_lck can be acquired. This is important
        // so we have a means of letting the submission queue run empty.
        std::unique_lock<std::mutex> job_lck(task_submission_mutex_);

        // Determine whether threads have already been started
        // If not, start them. Access to the threads is blocked by job_lck
        if(not threads_started_) {
            std::unique_lock<std::mutex> tc_lk(thread_creation_mutex_);
            if(not threads_started_) {
                // double checked locking pattern
                // Some error checks
                if(0 == n_threads_.load()) {
                    throw geneva_exception(
                        g_error_streamer(DO_LOG, time_and_place)
                        << "In GThreadPool::async_schedule(F f): Error!" << '\n'
                        << "The number of threads is set to 0" << '\n'
                    );
                }
                if(gtg_.size() > 0) {
                    throw geneva_exception(
                        g_error_streamer(DO_LOG, time_and_place)
                        << "In GThreadPool::async_schedule(F f): Error!" << '\n'
                        << "The thread group already has entries, although" << '\n'
                        << "threads_started_ is set to false" << '\n'
                    );
                }

                // Store a worker (a place holder, really) in the io_service_ object
                work_guard_ptr_ = std::make_shared<
                    boost::asio::executor_work_guard<boost::asio::io_context::executor_type>>(
                    io_context_.get_executor()
                );

                // No need to let the threads join, as none were running so far

                gtg_.create_threads([this]() { this->io_context_.run(); }, n_threads_.load());

                threads_started_ = true;
            }
        }

        // Update the task counter. NOTE: This needs to happen here
        // and not in taskWrapper. tasks_in_flight_ helps the wait()-function
        // to determine whether any jobs have been submitted to the Boost.ASIO
        // ioservice that haven't been processed yet. taskWrapper will
        // only start execution when it is assigned to a thread. As we
        // cannot "look" into the io_service, we need an external counter that
        // is incremented upon submission, not at start of execution. Otherwise
        // we might submit too many jobs.
        {
            std::unique_lock<std::mutex> cnt_lck(task_counter_mutex_);
            tasks_in_flight_++;
        }

        using result_type = std::invoke_result_t<F, Args&&...>;
        auto promise_ptr = std::make_shared<std::promise<result_type>>();
        std::future<result_type> result = promise_ptr->get_future();

        boost::asio::post(
            io_context_,
            [this,
             promise_ptr,
             f = std::bind<result_type>(std::forward<F>(f), std::forward<Args>(args)...)]() {
                try {
                    promise_ptr->set_value(f());
                }
                catch(
                    ...
                ) // NOLINT(bugprone-empty-catch) — propagates via set_exception or terminates
                {
                    try {
                        // Whatever was thrown may be stored in the promise
                        promise_ptr->set_exception(std::current_exception());
                    }
                    catch(
                        ...
                    ) // NOLINT(bugprone-empty-catch) — logs and terminates via GTERMINATION
                    {
                        // Unfortunately set_exception() may throw too
                        glogger << "In GThreadPool::async_schedule(/non-void/):" << '\n'
                                << "promise.set_exception() has thrown." << '\n'
                                << "We cannot continue" << '\n'
                                << GTERMINATION;
                    }
                }

                {
                    // Update the submission counter -- we need an external means to check whether the pool has run empty
                    std::unique_lock<std::mutex> cnt_lck(task_counter_mutex_);
#ifdef DEBUG
                    if(0 == tasks_in_flight_.load()) {
                        glogger << "In GThreadPool::async_schedule(/non-void/):" << '\n'
                                << "Trying to decrement a task counter that is already 0"
                                << '\n'
                                << "We cannot continue" << GTERMINATION;
                    }
#endif /* DEBUG */
                    tasks_in_flight_--;
                    condition_.notify_one();
                }
            }
        );

        return result;
    };

private:
    /***************************************************************************/

    boost::asio::io_context io_context_; ///< Manages the concurrent thread execution
    std::shared_ptr<boost::asio::executor_work_guard<boost::asio::io_context::executor_type>>
        work_guard_ptr_;

    GThreadGroup gtg_; ///< Holds the actual threads

    std::atomic<std::uint32_t> tasks_in_flight_{
        0
    };                               ///< The number of jobs that have been submitted in this round
    std::mutex task_counter_mutex_; ///< Protects access to the "submitted" job counter

    /// Allows to prevent further job submissions, particularly when waiting for the pool to clear or when resetting the pool
    std::mutex task_submission_mutex_;

    std::mutex
        thread_creation_mutex_; ///< Synchronization of access to the threads_started_ variable

    ///< Protects the job counter, so we may let the pool run empty
    std::condition_variable_any condition_;

    std::atomic<unsigned int> n_threads_; ///< The number of concurrent threads in the pool
    std::atomic<bool> threads_started_{
        false
    }; ///< Indicates whether threads have already been started
};

/******************************************************************************/
} /* namespace Gem::Common */
