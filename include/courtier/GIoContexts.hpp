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
#include <pthread.h>
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
 * This class centralized creation and management of io_context threads. It also
 * allows pinning of threads to a given CPU
 */
class GIoContexts {
public:
    //-------------------------------------------------------------------------
    /**
     * Initialization with number of concurrent io_contexts
     *
     * @param c_size
     */
    explicit GIoContexts( int c_size ) {
        m_pinned = c_size < 0;
        m_size   = std::abs( c_size );

        if ( m_size == 0 ) {
            throw gemfony_exception( g_error_streamer( DO_LOG, time_and_place )
                                     << "In GIoContexts() ctor: Error! pool size cannot be 0" << std::endl );
        }
        // check underlying hardware used
        if ( m_size > std::thread::hardware_concurrency() ) {
            glogger << "In GIoContexts() ctor:" << std::endl
                    << "pool size" << m_size << " too large for the underlying hardware, set to available max_cpus: "
                    << std::thread::hardware_concurrency() << std::endl
                    << GWARNING;

            m_size = std::thread::hardware_concurrency();
        }


        // All io_contexts will run() until they are explicitly stopped.
        for ( std::size_t i = 0; i < m_size; ++i ) {
            std::shared_ptr<boost::asio::io_context> ioc( new boost::asio::io_context );
            m_ioContexts.push_back( ioc );

#if BOOST_VERSION >= 107400
            m_work.push_back(
                boost::asio::require( ioc->get_executor(), boost::asio::execution::outstanding_work_t::tracked ) );
#else
            m_work.push_back( boost::asio::make_work_guard( *ioc ) );
#endif
        }
    }

    //----------------------------------------------------------------------------------
    // Deleted constructors and assignment operators

    GIoContexts() = delete;
    GIoContexts(GIoContexts const&) = delete;
    GIoContexts(GIoContexts &&) = delete;

    GIoContexts& operator=(GIoContexts const&) = delete;
    GIoContexts& operator=(GIoContexts &&) = delete;

    //----------------------------------------------------------------------------------

    void
    run()
    {
        size_t i = 0;

        for ( auto & ioc: m_ioContexts ) {
            std::shared_ptr<std::thread> thread( new std::thread( boost::bind( &boost::asio::io_context::run, ioc ) ) );
            m_threads.push_back( thread );

            if ( m_pinned ) {
                // Create a cpu_set_t object representing a set of CPUs.
                cpu_set_t cpuset;
                CPU_ZERO( &cpuset );
                CPU_SET( i, &cpuset );
                int rc
                    = pthread_setaffinity_np( ( m_threads[i].get() )->native_handle(), sizeof( cpu_set_t ), &cpuset );
                ++i;
                if ( rc != 0 ) {
                    throw gemfony_exception( g_error_streamer( DO_LOG, time_and_place )
                                             << "In GIoContexts()::run(): Error calling pthread_setaffinity_np: " << rc
                                             << std::endl );
                }
            }
        }  //!for
    }

    void
    stop()
    {
        for ( auto & ioc: m_ioContexts ) ioc->stop();
    }

    boost::asio::io_context &
    get()
    {
        // picking up io_context in round robin fashion
        return *( m_ioContexts[( m_nextContext++ % m_ioContexts.size() )] );
    }


private:
    int                                                   m_size;
    bool                                                  m_pinned;
    std::atomic<std::size_t>                              m_nextContext { 0 };
    std::vector<std::shared_ptr<std::thread>>             m_threads;
    std::vector<std::shared_ptr<boost::asio::io_context>> m_ioContexts;

#if BOOST_VERSION >= 107400
    std::list<boost::asio::any_io_executor> m_work;
#else
    std::list<boost::asio::executor_work_guard<boost::asio::io_context::executor_type>> m_work;
#endif
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

}  // namespace Gem::Courtier
