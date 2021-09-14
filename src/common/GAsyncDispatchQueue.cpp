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

#include "common/GAsyncDispatchQueue.hpp"

namespace Gem::Common {

G_API_COMMON GAsyncDispatchQueue::GAsyncDispatchQueue(std::size_t thread_cnt)
{
    for ( std::size_t t_cnt = 0; t_cnt < thread_cnt; ++t_cnt ) {
        m_workers_vec.emplace_back( [this] { submission_processor_(); } );
    }
}

G_API_COMMON GAsyncDispatchQueue::~GAsyncDispatchQueue()
{
    m_stop = true;
    m_dispatch_cv.notify_all();

    // Join all joinable threads
    for(auto & t: m_workers_vec) {
        if( t.joinable()) t.join();
    }
}

G_API_COMMON void
GAsyncDispatchQueue::stop() {
    m_stop = true;
}

G_API_COMMON void
GAsyncDispatchQueue::dispatch(const fp_t& op)
{
    // Do nothing if the stop-flag is set
    if( m_stop ) return;

    // Block further dispatches, if shared lock cannot be acquired
    std::shared_lock dispatcher_lock(m_dispatch_block_mutex );

    std::unique_lock lock( m_queue_mutex );
    m_queue.push(op);

    // Manual unlocking is done before notifying, to avoid waking up
    // the waiting thread only to block again (see notify_one for details)
    lock.unlock();
    m_dispatch_cv.notify_one();
}

G_API_COMMON void
GAsyncDispatchQueue::dispatch(fp_t&& op)
{
    // Do nothing if the stop-flag is set
    if( m_stop ) return;

    // Block further dispatches, if shared lock cannot be acquired
    std::shared_lock dispatcher_lock(m_dispatch_block_mutex );

    std::unique_lock lock( m_queue_mutex );
    m_queue.push(std::move(op));

    // Manual unlocking is done before notifying, to avoid waking up
    // the waiting thread only to block again (see notify_one for details)
    lock.unlock();
    m_dispatch_cv.notify_one();
}

G_API_COMMON void
GAsyncDispatchQueue::submission_processor_()
{
    do {
        std::unique_lock queue_lock( m_queue_mutex );

        // We will be woken up if there is data or if we
        // have been asked to quit.
        m_dispatch_cv.wait( queue_lock, [this]{
           return ( not m_queue.empty() || m_stop );
        });

        if(m_stop) break;

        // So the queue must have work items
        auto f = std::move( m_queue.front());
        m_queue.pop();

        // Give other threads an opportunity to access the queue
        queue_lock.unlock();

        // Do the actual processing
        f();
    } while (true);
}

G_API_COMMON void
GAsyncDispatchQueue::drain_queue() {
    // Block further dispatches
    std::lock_guard dispatcher_lock(m_dispatch_block_mutex );

    // Request exclusive access to the queue
    std::unique_lock queue_lock( m_queue_mutex );

    // Wait for the queue to run empty
    m_dispatch_cv.wait(queue_lock, [this]{
      return ( m_queue.empty() || m_stop );
    });

    // dispatcher_lock will cease to exist here and dispatches can start again
}

} /* namespace Gem::Common */