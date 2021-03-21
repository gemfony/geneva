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
 * The code in this file was originally contributed by Dr. Denis Bertini.
 *
 ********************************************************************************/

#pragma once

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard headers go here
#if GENEVA_OS_ID == GENEVA_LINUX_OS
#    include <pthread.h>
#endif

#include <climits>
#include <cstdlib>
#include <mutex>
#include <thread>
#include <vector>

// Boost headers go here
#include <boost/asio.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/version.hpp>

// Geneva headers go here
#include "common/GErrorStreamer.hpp"
#include "common/GGlobalDefines.hpp"
#include "common/GLogger.hpp"

namespace Gem::Courtier {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class centralizes creation and management of io_context threads for networked operation.
 * The class covers the following states:
 * - Multiple threads with individual io_context objects. run() is called for each object()
 * - Multiple threads with multiple run() calls execute on one io_context object
 * It also allows pinning of threads to a given CPU. All modes can be influenced via the
 * run() function. Note that the maxmimum number of threads created is limited to the number
 * of hardware threads of a given machine.
 */
class GIoContexts
{
public:
    //----------------------------------------------------------------------------------
    // Trivial default constructor
    GIoContexts() = default;

    //----------------------------------------------------------------------------------
    // Deleted copy-/move-constructors and (move-)assignment operators

    GIoContexts( GIoContexts const & ) = delete;
    GIoContexts( GIoContexts && )      = delete;

    GIoContexts & operator=( GIoContexts const & ) = delete;
    GIoContexts & operator=( GIoContexts && ) = delete;

    //----------------------------------------------------------------------------------
    /**
     * Starts a new run
     *
     * @param pool_size The number of concurrent threads used for processing
     * @param pinned Whether each threads should be pinned to one core
     * @param use_multiple_contexts Whether each run() should be called for individual io_context-objects
     */
    void
    run(std::size_t pool_size = 0, bool pinned = false, bool use_multiple_contexts = false)
    {
        // Make sure this call can not run concurrently to a stop() or get()-call
        const std::lock_guard<std::mutex> lock(m_run_mutex);

        // We want to make sure that calling run() multiple times without stop() has no effect.
        if( m_running ) {
            glogger << "In GIoContexts::run()" << std::endl
                    << "run() called more than once without stop()." << std::endl
                    << "This will be ignored." << std::endl
                    << GWARNING;

            return;
        }

#ifdef DEBUG
        // Check that all vectors are in pristine condition
        if(not m_thread_ptr_vec.empty() || not m_ioContext_ptr_vec.empty() || not m_work_vec.empty()) {
            throw gemfony_exception(
                g_error_streamer( DO_LOG, time_and_place )
                    << "In GIoContexts()::run(): Not all vectors are empty:"
                    << "m_threads_vec.empty(): " << m_thread_ptr_vec.empty() << std::endl
                    << "m_ioContexts_vec.empty(): " << m_ioContext_ptr_vec.empty() << std::endl
                    << "m_work_vec.empty(): " << m_work_vec.empty() << std::endl );
        }
#endif

        // Rectify the pool sizes, if necessary
        std::size_t l_pool_size = pool_size;
        if ( l_pool_size == 0 || l_pool_size > m_max_threads ) {
            if ( l_pool_size > m_max_threads ) {
                glogger << "In GIoContexts::run()" << std::endl
                        << "pool size" << l_pool_size
                        << " too large for the underlying hardware, set to available max_cpus: " << m_max_threads
                        << std::endl
                        << GWARNING;
            } else {
                glogger << "In GIoContexts::GIoContexts(std::size_t, bool)" << std::endl
                        << "Setting pool size to available max_cpus: " << m_max_threads << std::endl
                        << GLOGGING;
            }

            l_pool_size = m_max_threads;
        }

        // All io_contexts will run until they are explicitly stopped.
        for ( std::size_t i = 0; i < l_pool_size; ++i ) {
            auto ioc_ptr = std::make_shared<boost::asio::io_context>();
            m_ioContext_ptr_vec.push_back( ioc_ptr );

#if BOOST_VERSION >= 107400
            m_work_vec.emplace_back(boost::asio::require( ioc_ptr->get_executor(), boost::asio::execution::outstanding_work_t::tracked ) );
#else
            m_work.emplace_back( boost::asio::make_work_guard( *ioc_ptr ) );
#endif

            // Just create one io_context object, if we do not want
            // one context per run()
            if ( not use_multiple_contexts ) break;
        }

        if ( use_multiple_contexts ) { // Start one run() for each io_context object
            for ( auto & ioc_ptr: m_ioContext_ptr_vec ) {
                m_thread_ptr_vec.emplace_back( std::make_shared<std::thread>( [ioc_ptr] { ioc_ptr->run(); } ) );
            }  //!for
        } else {  // multiple run()-calls for a single io_context object
#ifdef DEBUG
            if( m_ioContext_ptr_vec.size() != 1 ) {
                throw gemfony_exception(
                    g_error_streamer( DO_LOG, time_and_place )
                        << "In GIoContexts()::run(): Invalid number of io_context objects: " << m_ioContext_ptr_vec.size()
                        << std::endl );
            }
#endif

            auto & ioc_ptr = m_ioContext_ptr_vec.back();  // There should only be one
            for ( std::size_t t = 0; t < pool_size; t++ ) {
                m_thread_ptr_vec.emplace_back( std::make_shared<std::thread>( [ioc_ptr] { ioc_ptr->run(); } ) );
            }
        }

        // Pin threads, if desired
        if ( pinned ) {
            std::size_t pos = 0;
            for ( auto & t_ptr: m_thread_ptr_vec ) {
#if GENEVA_OS_ID == GENEVA_LINUX_OS
                // Create a cpu_set_t object representing a set of CPUs.
                cpu_set_t cpuset;
                CPU_ZERO( &cpuset );
                CPU_SET( pos, &cpuset );
                int rc = pthread_setaffinity_np( ( t_ptr.get() )->native_handle(), sizeof( cpu_set_t ), &cpuset );
                if ( rc != 0 ) {
                    throw gemfony_exception(
                        g_error_streamer( DO_LOG, time_and_place )
                        << "In GIoContexts()::run(): Error calling pthread_setaffinity_np in position " << pos << ":"
                        << rc << std::endl );
                }

                pos++;

                // TODO: add core pinning for GENEVA_WIN_OS, GENEVA_MAC_OS, GENEVA_FREEBSD_OS, GENEVA_CYGWIN_OS
#endif  //!GENEVA_OS_ID
            }
        }

        // Make it known that a run has started
        m_running = true;
    }

    //----------------------------------------------------------------------------------
    /**
     * Terminates all worker threads
     */
    void
    stop()
    {
        // Make sure this call can not run concurrently to a run() or get()-call
        const std::lock_guard<std::mutex> lock(m_run_mutex);

        // We want to make sure that calling stop() multiple times without run() has no effect.
        if(not m_running) {
            glogger << "In GIoContexts::stop()" << std::endl
                    << "stop() called while no run() has been started" << std::endl
                    << "This will be ignored." << std::endl
                    << GWARNING;

            return;
        }

        // Notify all workers that they must stop
        for ( auto & ioc_ptr: m_ioContext_ptr_vec ) ioc_ptr->stop();

        // Wait for all threads to terminate
        for ( auto & thread_ptr: m_thread_ptr_vec ) thread_ptr->join();

        // Clear threads, workers and io_context objects
        m_thread_ptr_vec.clear();
        m_ioContext_ptr_vec.clear();
        m_work_vec.clear();

        // Reset the io_context id
        m_nextContext = 0;

        // Make it known that we are no longer in "running-state"
        m_running = false;
    }

    //----------------------------------------------------------------------------------
    /**
     * Retrieves the current io_context object in a round-robin fashion. Note that this
     * function will work even when only a single io_context object is available
     *
     * @return An io_context object selected in a round-robin fashion
     */
    boost::asio::io_context &
    get()
    {
        // Make sure this call can not run concurrently to a stop() or stop()-call.
        // This call will also prevent trying to access the vector from multiple threads.
        const std::lock_guard<std::mutex> lock(m_run_mutex);

#ifdef DEBUG
        if( m_ioContext_ptr_vec.empty() ) {
            throw gemfony_exception( g_error_streamer( DO_LOG, time_and_place )
                                     << "In GIoContexts()::run():  m_ioContexts_vec is empty()"
                                     << std::endl );
        }
#endif

        // picking up io_context in round robin fashion
        return *( m_ioContext_ptr_vec[( m_nextContext++ % m_ioContext_ptr_vec.size() )] );
    }

    //----------------------------------------------------------------------------------
    /**
     * Check whether we are in "running"-state. Note that this can only be an indication,
     * as the "running-state" may have changed shortly after this call.
     *
     * @return A boolean indicating whether the m_running-flag is set
     */
     [[nodiscard]] bool is_running() const {
         return m_running;
     }

private:
    std::atomic<bool> m_running {false};

    const std::size_t m_max_threads { std::thread::hardware_concurrency() > 0 ? std::thread::hardware_concurrency()
                                                                              : GCONSUMERLISTENERTHREADS };
    std::atomic<std::size_t> m_nextContext { 0 };

    std::vector<std::shared_ptr<std::thread>>             m_thread_ptr_vec;
    std::vector<std::shared_ptr<boost::asio::io_context>> m_ioContext_ptr_vec;

#if BOOST_VERSION >= 107400
    std::vector<boost::asio::any_io_executor> m_work_vec;
#else
    std::vector<boost::asio::executor_work_guard<boost::asio::io_context::executor_type>> m_work_vec;
#endif

    std::mutex m_run_mutex; ///< Secures access to the run(), stop() and get()-functions
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

}  // namespace Gem::Courtier
