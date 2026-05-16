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

#include "common/GThreadPool.hpp"
#include "common/GCommonHelperFunctionsT.hpp"


namespace Gem::Common {

/******************************************************************************/
/**
 * Initialization with a number of threads.
 *
 * @param n_threads The desired number of threads executing work concurrently in the pool
 */
GThreadPool::GThreadPool(unsigned int n_threads)
  : nThreads_(n_threads > 0 ? n_threads : DEFAULTNHARDWARETHREADS) {
    if(0 == n_threads) {
        glogger << "In GThreadPool::GThreadPool(unsigned int const &n_threads):" << '\n'
                << "User requested n_threads == 0. n_threads was reset to the default "
                << DEFAULTNHARDWARETHREADS << '\n'
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
    std::unique_lock<std::mutex> job_lck(task_submission_mutex_);
    {
        // Makes sure cnt_lck is released
        // Acquire the lock, then return it as long as the condition hasn't been fulfilled
        std::unique_lock<std::mutex> cnt_lck(task_counter_mutex_);
        while(tasksInFlight_.load() > 0) {
            // Deal with spurious wake-ups
            condition_.wait(cnt_lck);
        }
    }

    // Clear the thread group
    work_guard_ptr_.reset(); // This will initiate termination of all threads
    gtg_.join_all();         // wait for the threads to terminate
    gtg_.clearThreads();     // Clear the thread group
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
 * @param n_threads The desired number of threads
 */
void GThreadPool::setNThreads(unsigned int n_threads) {
    // Make sure no new jobs may be submitted and no threads may be created concurrently
    std::scoped_lock lk(task_submission_mutex_, thread_creation_mutex_);

    // Check if any work needs to be done
    if(gtg_.size() == n_threads) {
        // We do nothing if we already have the desired size
        return;
    }

    // At this point all potential async_schedule calls, just like the wait() function,
    // must be waiting to acquire the task_submission_mutex_.

    {
        // Let the pool run empty
        // Acquire the lock, then return it as long as the condition hasn't been fulfilled
        std::unique_lock<std::mutex> cnt_lck(task_counter_mutex_);
        while(tasksInFlight_.load() > 0) {
            // Deal with spurious wake-ups
            condition_.wait(cnt_lck);
        }
    }

    // If threads were already running, either add new threads or recreate the pool
    if(threads_started_) {
        if(n_threads > nThreads_.load()) {
            // We simply add the required number of threads
            gtg_.create_threads(
                [this]() { this->io_context_.run(); },
                n_threads - nThreads_.load()
            );
        }
        else {
            // We need to remove threads and thus reset the entire pool
            work_guard_ptr_.reset(); // This will initiate termination of all threads
            gtg_.join_all();         // wait for the threads to terminate
            gtg_.clearThreads();     // Clear the thread group

            // Reset the io_service object, so run may be called again
            io_context_.restart();

            // Store a new worker (a place holder, really) in the io_service_ object
            work_guard_ptr_ = std::make_shared<
                boost::asio::executor_work_guard<boost::asio::io_context::executor_type>>(
                io_context_.get_executor()
            );

            // Start the threads
            gtg_.create_threads([&]() { io_context_.run(); }, n_threads);
        }
    }

    // Finally set the new number of threads
    nThreads_ = n_threads;
}

/******************************************************************************/
/**
 * Retrieves the current "true" number of threads being used in the pool
 */
unsigned int GThreadPool::getNThreads() const {
    return Gem::Common::narrow_cast<unsigned int>(gtg_.size());
}

/******************************************************************************/
/**
 * Waits for all submitted jobs to be cleared from the pool. Note that this
 * function may NOT be called from a task running inside the pool.
 */
void GThreadPool::wait() {
    // Make sure no new jobs may be submitted
    std::unique_lock<std::mutex> job_lck(task_submission_mutex_);

    {
        // Makes sure cnt_lck is released
        // Acquire the lock, then return it as long as the condition hasn't been fulfilled
        std::unique_lock<std::mutex> cnt_lck(task_counter_mutex_);
        condition_.wait(cnt_lck, [this]() -> bool { return (tasksInFlight_.load() == 0); });
    }
}

/******************************************************************************/

} /* namespace Gem::Common */
