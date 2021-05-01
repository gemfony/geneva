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
 * The code in this file is based on a contribution by Dr. Denis Bertini
 * under the same license.
 *
 ********************************************************************************
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
 * This enum signifies the run state of the GIoContexts object
 */
enum class context_state { constructing = 0, initialized = 1, running = 2, stopped = 3 };

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class centralizes creation and management of io_context threads for networked operation.
 * The class covers the following states:
 * - Multiple threads with individual io_context objects. run() is called for each object()
 * - Multiple threads with multiple run() calls execute on one io_context object
 * It also allows pinning of threads to a given CPU-core. Note that the maxmimum number of
 * threads created is limited to the number of hardware threads of a given machine.
 */
class GIoContexts
{
public:
    //----------------------------------------------------------------------------------
    /**
     * Standard contructor
     *
     * @param pool_size The number of concurrent threads used for processing
     * @param pinned Whether each threads should be pinned to one core
     * @param use_multiple_io_contexts Whether each run() should be called for individual io_context-objects
     */
    explicit GIoContexts( std::size_t pool_size                = DEFAULTIOCONTEXTPOOLSIZE,
                          bool        pinned                   = DEFAULTUSECOREPINNING,
                          bool        use_multiple_io_contexts = DEFAULTMULTIPLEIOCONTEXTS )
        : m_context_state( context_state::constructing )
        , m_pool_size( pool_size )
        , m_pinned( pinned )
        , m_use_multiple_io_contexts( use_multiple_io_contexts )
    {
        // Rectify the pool sizes, if necessary
        if ( m_pool_size == 0 || m_pool_size > m_max_threads ) {
            if ( m_pool_size > m_max_threads ) {
                glogger << "In GIoContexts::run()" << std::endl
                        << "pool size" << m_pool_size
                        << " too large for the underlying hardware, set to available max_cpus: " << m_max_threads
                        << std::endl
                        << GWARNING;
            }
            else {
                glogger << "In GIoContexts::GIoContexts(std::size_t, bool)" << std::endl
                        << "Setting pool size to available max_cpus: " << m_max_threads << std::endl
                        << GLOGGING;
            }

            m_pool_size = m_max_threads;
        }

        // Create and add new io_context objects.
        for ( std::size_t i = 0; i < m_pool_size; ++i ) {
            m_ioContext_ptr_vec.emplace_back( std::make_shared<boost::asio::io_context>() );
            auto ioc_ptr = m_ioContext_ptr_vec.back();

#if BOOST_VERSION >= 107400
            m_work_vec.emplace_back(
                boost::asio::require( ioc_ptr->get_executor(), boost::asio::execution::outstanding_work_t::tracked ) );
#else
            m_work.emplace_back( boost::asio::make_work_guard( *ioc_ptr ) );
#endif

            // Just create one io_context object, if we do not want
            // one context per run()
            if ( not m_use_multiple_io_contexts ) break;
        }
    }

    //----------------------------------------------------------------------------------
    // Deleted default- copy-/move-constructors and (move-)assignment operators

    GIoContexts()                      = delete;
    GIoContexts( GIoContexts const & ) = delete;
    GIoContexts( GIoContexts && )      = delete;

    GIoContexts & operator=( GIoContexts const & ) = delete;
    GIoContexts & operator=( GIoContexts && ) = delete;

    //----------------------------------------------------------------------------------
    /**
     * Initialization and checks
     */
    void
    init()
    {
        // Make sure this call can not run concurrently
        const std::lock_guard<std::mutex> lock( m_mutex );

        // Make sure the object is in the right state when this function is called
        switch ( m_context_state ) {
            case context_state::initialized:  // Multiple initializations will be ignored
            {
                glogger << "In GIoContexts::init()" << std::endl
                        << "init() called more than once." << std::endl
                        << "This will be ignored." << std::endl
                        << GWARNING;
            }
                return;

            case context_state::running:  // Attempts to initialize the object while running are an error
            {
                throw gemfony_exception( g_error_streamer( DO_LOG, time_and_place )
                                         << "In GIoContexts()::init(): Attempt to initialize while in running-state"
                                         << std::endl
                                         << "Call stop first" << std::endl );
            }

            case context_state::constructing:
            case context_state::stopped:  // do nothing, these are valid states
                break;
        }

        // Set the current run-state
        m_context_state = context_state::initialized;
    }

    //----------------------------------------------------------------------------------
    /**
     * Starts a new run
     */
    void
    run()
    {
        // Make sure this call can not run concurrently
        const std::lock_guard<std::mutex> lock( m_mutex );

        // Make sure the object is in the right state when this function is called
        switch ( m_context_state ) {
            case context_state::initialized:  // This is the expected state at the start of this call
                break;

            case context_state::running:  // Attempts to call run() on an already running object will be ignored
            {
                glogger << "In GIoContexts::run()" << std::endl
                        << "run() called more than once without stop()." << std::endl
                        << "This will be ignored." << std::endl
                        << GWARNING;
            }
                return;

            case context_state::constructing:  // The object should have passed the init-state already
            case context_state::stopped:       // init() must be called before run() is called
            {
                throw gemfony_exception( g_error_streamer( DO_LOG, time_and_place )
                                         << "In GIoContexts()::run(): Attempt to call run() for stopped object"
                                         << std::endl
                                         << "Call init() first" << std::endl );
            }
        }

        if ( m_use_multiple_io_contexts ) {  // Start one run() for each io_context object
            // There should be m_pool_size io_context objects in the collection
            if ( m_ioContext_ptr_vec.size() != m_pool_size ) {
                throw gemfony_exception( g_error_streamer( DO_LOG, time_and_place )
                                         << "In GIoContexts()::run(): Invalid number of io_context objects: "
                                         << m_ioContext_ptr_vec.size() << std::endl
                                         << "Should have been " << m_pool_size << std::endl );
            }

            for ( auto & ioc_ptr: m_ioContext_ptr_vec ) {
                m_thread_ptr_vec.emplace_back( std::make_shared<std::thread>( [ioc_ptr] { ioc_ptr->run(); } ) );
            }  //!for
        }
        else {  // multiple run()-calls for a single io_context object
                // Check that there is only a single io_context object in the collection
            if ( m_ioContext_ptr_vec.size() != 1 ) {
                throw gemfony_exception( g_error_streamer( DO_LOG, time_and_place )
                                         << "In GIoContexts()::run(): Invalid number of io_context objects: "
                                         << m_ioContext_ptr_vec.size() << std::endl );
            }

            auto & ioc_ptr = m_ioContext_ptr_vec.back();  // There should only be one
            for ( std::size_t t = 0; t < m_pool_size; t++ ) {
                m_thread_ptr_vec.emplace_back( std::make_shared<std::thread>( [ioc_ptr] { ioc_ptr->run(); } ) );
            }
        }

        // Pin threads, if desired
        if ( m_pinned ) {
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
        m_context_state = context_state::running;
    }

    //----------------------------------------------------------------------------------
    /**
     * Terminates all worker threads
     */
    void
    stop()
    {
        // Make sure this call can not run concurrently
        const std::lock_guard<std::mutex> lock( m_mutex );

        // Make sure the object is in the right state when this function is called
        switch ( m_context_state ) {
            case context_state::constructing:  // stop() may be called both after, construction, init() and  run()
            case context_state::initialized:
            case context_state::running:
                break;

            case context_state::stopped:  // Multiple calls to stop() will be ignored
            {
                glogger << "In GIoContexts::stop()" << std::endl
                        << "stop() called more than once in a row" << std::endl
                        << "This will be ignored." << std::endl
                        << GWARNING;
            }
                return;
        }
	
        // Notify all workers that they must stop
        for ( auto & ioc_ptr: m_ioContext_ptr_vec ) ioc_ptr->stop();
        // Wait for all threads to terminate
        for ( auto & thread_ptr: m_thread_ptr_vec ) thread_ptr->join();
        // Clear threads, workers and io_context objects
	m_thread_ptr_vec.clear();
        // Reset all io_contexts 
        for ( auto & ioc_ptr: m_ioContext_ptr_vec ) ioc_ptr->reset();
	
        // Checkme, call to  io_contexts vector clear is crashing 
	//  m_ioContext_ptr_vec.clear();

	// Clear all workers
	m_work_vec.clear();

        // Reset the io_context id
	m_nextContext = 0;

        // Make it known that a run has stopped
	m_context_state = context_state::stopped;
    }

    //----------------------------------------------------------------------------------
    /**
     * Retrieves the current io_context object in a round-robin fashion. Note that this
     * function will work even when only a single io_context object is available
     *
     * @return A reference to an io_context object selected in a round-robin fashion
     */
    boost::asio::io_context &
    get()
    {
        // Make sure this call can not run concurrently
        const std::lock_guard<std::mutex> lock( m_mutex );

        // Make sure the object is in the right state when this function is called
        switch ( m_context_state ) {
            case context_state::constructing:  // These are valid states for this call
            case context_state::running:
                break;

            case context_state::initialized:  // get() may only be called when the object is in running()-state
            case context_state::stopped: {
                throw gemfony_exception( g_error_streamer( DO_LOG, time_and_place )
                                         << "In GIoContexts()::get(): Attempt to call function" << std::endl
                                         << "for object that is not in running state" << std::endl );
            }
        }

#ifdef DEBUG
        if ( m_ioContext_ptr_vec.empty() ) {
            throw gemfony_exception( g_error_streamer( DO_LOG, time_and_place )
                                     << "In GIoContexts()::run():  m_ioContexts_vec is empty()" << std::endl );
        }
#endif

        // picking up io_context in round robin fashion
        return *( m_ioContext_ptr_vec[( m_nextContext++ % m_ioContext_ptr_vec.size() )] );
    }

    //----------------------------------------------------------------------------------
    /**
     * Check whether the run-state of this object. Note that the return value can only
     * be an indication, as the state may change short time after the call to this function
     *
     * @return A context_state class enum
     */
    [[nodiscard]] context_state
    get_context_state() const
    {
        return m_context_state;
    }

private:
    std::atomic<context_state> m_context_state;  ///< Indicates in which state the object is

    const std::size_t m_max_threads { std::thread::hardware_concurrency() > 0 ? std::thread::hardware_concurrency()
                                                                              : GCONSUMERLISTENERTHREADS };
    std::size_t       m_pool_size { DEFAULTMULTIPLEIOCONTEXTS };  ///< 0 means "automatic"

    bool m_pinned { DEFAULTUSECOREPINNING };                    ///< Whether to pin threads to individual cores
    bool m_use_multiple_io_contexts { DEFAULTMULTIPLEIOCONTEXTS };  ///< Whether to use one io_context object for each run()-call

    std::atomic<std::size_t> m_nextContext { 0 };

    std::vector<std::shared_ptr<std::thread>>             m_thread_ptr_vec;
    std::vector<std::shared_ptr<boost::asio::io_context>> m_ioContext_ptr_vec;

#if BOOST_VERSION >= 107400
    std::vector<boost::asio::any_io_executor> m_work_vec;
#else
    std::vector<boost::asio::executor_work_guard<boost::asio::io_context::executor_type>> m_work_vec;
#endif

    std::mutex m_mutex;  ///< Secures access to the init(), run(), stop() and get()-functions
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

}  // namespace Gem::Courtier
