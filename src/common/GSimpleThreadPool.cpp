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

#include "common/GSimpleThreadPool.hpp"

namespace Gem::Common {

/*******************************************************************************/
/**
 * Initialization with a number of threads.
 *
 * @param nThreads The desired number of threads executing work concurrently in the pool
 */
GSimpleThreadPool::GSimpleThreadPool(std::size_t nThreads)
    : m_nThreads(nThreads > 0 ? nThreads : std::size_t(DEFAULTNHARDWARETHREADS))
{
    if(0 == nThreads) {
        glogger
            << "In GNewThreadPool::GNewThreadPool(std::size_t nThreads):" << std::endl
            << "User requested nThreads == 0. We are using the default number of threads " << DEFAULTNHARDWARETHREADS << " instead" << std::endl
            << GWARNING;
    }
}

/*******************************************************************************/
/**
 * The destructor. This function is not thread-safe and does assume that
 * at the time of its call no other thread-pool related functions are called.
 * It does allow the queue to run empty, though.
 */
GSimpleThreadPool::~GSimpleThreadPool() {
    // Make sure no new jobs may be submitted and let the pool run empty
    std::unique_lock<std::mutex> job_lck(m_task_submission_mutex);
    { // Makes sure cnt_lck is released
        // Acquire the lock, then return it as long as the condition hasn't been fulfilled
        std::unique_lock<std::mutex> cnt_lck(m_task_counter_mutex);
        m_condition.wait(cnt_lck,
           [this]() -> bool {
                return (m_tasksInFlight == 0);
           }
        );
    }
}

/*******************************************************************************/

} /* namespace Gem::Common */