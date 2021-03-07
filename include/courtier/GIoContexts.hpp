#pragma once

#include <pthread.h>
#include <climits>
#include <cstdlib>
#include <mutex>
#include <thread>
#include <vector>

#include <boost/asio.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/version.hpp>

#include "common/GErrorStreamer.hpp"
#include "common/GGlobalDefines.hpp"
#include "common/GLogger.hpp"


namespace Gem::Courtier {

class GIoContexts : private boost::noncopyable
{
public:
    explicit GIoContexts( int c_size )
    {
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
            m_work.push_back( boost::asio::require( ioc->get_executor(), boost::asio::execution::outstanding_work.tracked ) );
#else
            m_work.push_back( boost::asio::make_work_guard( *ioc ) );
#endif
        }
    }

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
    int                                           m_size;
    bool                                          m_pinned;
    std::atomic<std::size_t>                      m_nextContext { 0 };
    std::vector<std::shared_ptr<std::thread>>     m_threads;
    std::vector<std::shared_ptr<boost::asio::io_context>> m_ioContexts;

#if BOOST_VERSION >= 107400
    std::list<boost::asio::any_io_executor> m_work;
#else
    std::list<boost::asio::executor_work_guard<boost::asio::io_context::executor_type>> m_work;
#endif
};


}  // namespace Gem::Courtier
