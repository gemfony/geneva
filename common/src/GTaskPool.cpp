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

#include "common/GTaskPool.hpp"
#include "common/GCommonEnums.hpp"
#include "common/GLogger.hpp"
#include <mutex>
#include <shared_mutex>

namespace Gem::Common {

/******************************************************************************/
/**
 * Initialisation with a number of threads. The worker threads are started
 * eagerly (they then block on the empty task queue at ~0 CPU).
 *
 * @param n_threads The desired number of worker threads (0 -> hardware default)
 */
GTaskPool::GTaskPool(unsigned int n_threads)
  : n_threads_(n_threads > 0 ? n_threads : DEFAULTNHARDWARETHREADS) {
    if(0 == n_threads) {
        glogger << "In GTaskPool::GTaskPool(unsigned int n_threads):" << '\n'
                << "User requested n_threads == 0. n_threads was reset to the default "
                << DEFAULTNHARDWARETHREADS << '\n'
                << GWARNING;
    }
    task_queue_.emplace(); // construct the (unbounded) task queue
    start_workers(n_threads_);
}

/******************************************************************************/
/**
 * The destructor. Closing the queue makes the workers drain any remaining tasks
 * (running them, so their futures are satisfied) and then exit on the now-empty
 * queue; we then join them. No exception may escape a destructor.
 */
GTaskPool::~GTaskPool() {
    try {
        task_queue_->close();
        worker_group_.join_all();
        worker_group_.clearThreads();
    }
    catch(...) { // NOLINT(bugprone-empty-catch) — a destructor must not throw
    }
}

/******************************************************************************/
/**
 * Starts n worker threads, each draining the current task queue.
 */
void GTaskPool::start_workers(unsigned int n) {
    worker_group_.create_threads([this]() { this->worker_loop(); }, n);
}

/******************************************************************************/
/**
 * Common submission path for async_schedule() and post(). Bumps the in-flight
 * counter and enqueues the (already type-erased) task while holding the shared
 * submission lock, so submissions run concurrently with each other but not while
 * wait()/setNThreads() drains the pool. Returns false if the queue was closed
 * (pool shutting down), leaving the in-flight counter unchanged.
 */
bool GTaskPool::enqueue(std::function<void()> task) {
    std::shared_lock<std::shared_mutex> sub_lck(submission_mutex_);
    {
        std::scoped_lock<std::mutex> cnt_lck(counter_mutex_);
        ++tasks_in_flight_;
    }
    if(task_queue_->push(std::move(task))) {
        return true;
    }
    // The queue is closed: undo the speculative increment.
    std::scoped_lock<std::mutex> cnt_lck(counter_mutex_);
    if(0 == --tasks_in_flight_) {
        all_done_.notify_all();
    }
    return false;
}

/******************************************************************************/
/**
 * The worker body. pop() blocks at ~0 CPU on an empty queue, drains remaining
 * tasks after close(), and returns std::nullopt once the queue is closed and
 * empty -- which is how a worker leaves the loop and terminates.
 */
void GTaskPool::worker_loop() {
    while(auto task = task_queue_->pop()) {
        // The task wrapper fulfils its own promise and never lets an exception
        // escape, so the in-flight bookkeeping below always runs.
        (*task)();

        std::scoped_lock<std::mutex> cnt_lck(counter_mutex_);
        if(0 == --tasks_in_flight_) {
            all_done_.notify_all();
        }
    }
}

/******************************************************************************/
/**
 * Blocks (under counter_mutex_) until no tasks are in flight. The caller must
 * already hold submission_mutex_ exclusively, so no new tasks can be added.
 */
void GTaskPool::drain() {
    std::unique_lock<std::mutex> cnt_lck(counter_mutex_);
    all_done_.wait(cnt_lck, [this]() -> bool { return 0 == tasks_in_flight_; });
}

/******************************************************************************/
/**
 * Blocks until all submitted tasks have been processed. Blocks new submissions
 * for the duration. Must NOT be called from a task running inside the pool.
 */
void GTaskPool::wait() {
    std::unique_lock<std::shared_mutex> sub_lck(submission_mutex_);
    drain();
}

/******************************************************************************/
/**
 * Retrieves the configured (and, since workers start eagerly, live) number of
 * worker threads.
 */
unsigned int GTaskPool::getNThreads() const {
    return n_threads_;
}

/******************************************************************************/
/**
 * Sets the number of worker threads. Blocks new submissions and lets the pool
 * run empty first. Growing simply adds workers to the same queue; shrinking
 * recreates the queue (its close() is terminal) and restarts the worker set.
 * Must NOT be called from a task running inside the pool.
 *
 * @param n_threads The desired number of worker threads (0 -> hardware default)
 */
void GTaskPool::setNThreads(unsigned int n_threads) {
    const unsigned int n = n_threads > 0 ? n_threads : DEFAULTNHARDWARETHREADS;

    std::unique_lock<std::shared_mutex> sub_lck(submission_mutex_);
    if(n == n_threads_) {
        return;
    }
    drain();

    if(n > n_threads_) {
        // Grow: add workers; they pull from the same (still-open) queue.
        start_workers(n - n_threads_);
    }
    else {
        // Shrink: the queue's close() is terminal, so recreate it.
        task_queue_->close();
        worker_group_.join_all();
        worker_group_.clearThreads();
        task_queue_.emplace(); // fresh, open queue
        start_workers(n);
    }
    n_threads_ = n;
}

/******************************************************************************/

} /* namespace Gem::Common */
