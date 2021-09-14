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

// Standard headers go here
#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <cstdio>
#include <functional>
#include <mutex>
#include <queue>
#include <shared_mutex>
#include <string>
#include <thread>
#include <vector>

// Boost headers go here

// Geneva headers go here

namespace Gem::Common {

class GAsyncDispatchQueue
{
    typedef std::function<void(void)> fp_t;

    std::mutex               m_queue_mutex;
    std::shared_mutex        m_dispatch_block_mutex; ///< Multiple simultaneous dispatch, may be jointly blocked
    std::vector<std::thread> m_workers_vec;
    std::queue<fp_t>         m_queue;
    std::condition_variable  m_dispatch_cv;

    std::atomic<bool>        m_stop {false};

    G_API_COMMON void submission_processor_();

public:
    G_API_COMMON explicit GAsyncDispatchQueue(std::size_t thread_cnt = std::size_t(std::thread::hardware_concurrency()));
    G_API_COMMON ~GAsyncDispatchQueue();

    // dispatch and copy
    G_API_COMMON void dispatch(const fp_t& op);
    // dispatch and move
    G_API_COMMON void dispatch(fp_t&& op);

    /** @brief Indicate that processing should stop */
    void stop();
    /** @brief Wait for the task queue to run empty */
    G_API_COMMON void drain_queue();

    /***************************************************************************/
    // Some deleted functions and constructors
    G_API_COMMON GAsyncDispatchQueue()                         = delete;
    G_API_COMMON GAsyncDispatchQueue( const GAsyncDispatchQueue & ) = delete;                    // deleted copy constructor
    G_API_COMMON GAsyncDispatchQueue & operator=( GAsyncDispatchQueue & )             = delete;  // deleted assignment operator
    G_API_COMMON GAsyncDispatchQueue( const GAsyncDispatchQueue && ) = delete;  // deleted move constructor
    G_API_COMMON GAsyncDispatchQueue & operator=( GAsyncDispatchQueue && ) = delete;  // deleted move-assignment operator

    /***************************************************************************/
};

} /* namespace Gem::Common */