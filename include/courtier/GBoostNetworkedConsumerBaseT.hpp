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
#include <array>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

// Boost headers go here
#include <boost/algorithm/string.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/asio.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/utility.hpp>

// Geneva headers go here
#include "common/GCommonEnums.hpp"
#include "common/GCommonHelperFunctions.hpp"
#include "common/GCommonHelperFunctionsT.hpp"
#include "common/GSerializationHelperFunctionsT.hpp"
#include "common/GThreadGroup.hpp"
#include "common/GThreadPool.hpp"
#include "courtier/GBaseClientT.hpp"
#include "courtier/GBaseConsumerT.hpp"
#include "courtier/GBrokerT.hpp"
#include "courtier/GCommandContainerT.hpp"
#include "courtier/GCourtierEnums.hpp"
#include "courtier/GCourtierHelperFunctions.hpp"
#include "courtier/GIoContexts.hpp"

namespace Gem::Courtier {

/******************************************************************************/

//-------------------------------------------------------------------------
// Simplification

#define M_ACCEPTOR    this->asio_network_context_ptr->m_acceptor
#define M_SOCKET      this->asio_network_context_ptr->m_socket
#define M_ENDPOINT    this->asio_network_context_ptr->m_endpoint
#define M_IO_CONTEXTS this->asio_network_context_ptr->m_io_contexts

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class encodes the consumer logic common for ASIO networking and Beast
 * websockets
 */
template <typename processable_type>
class GBoostNetworkedConsumerBaseT
    : public Gem::Courtier::GBaseConsumerT<processable_type>  // note: GBaseConsumerT<> is non-copyable
    , public std::enable_shared_from_this<GBoostNetworkedConsumerBaseT<processable_type>>
{
    //-------------------------------------------------------------------------
    // Simplify usage of namespaces
    using error_code = boost::system::error_code;

protected:
    //-------------------------------------------------------------------------
    // Describe the network context
    struct asio_network_context
    {
    public:
        /**
         * Common initialization of io_contexts, endpoint, acceptor and socket
         *
         * @param pool_size The number of concurrent threads used for processing
         * @param pinned Whether each threads should be pinned to one core
         * @param use_multiple_io_contexts Whether each run() should be called for individual io_context-objects
         */
        explicit asio_network_context(std::size_t pool_size = 0, bool pinned = false, bool use_multiple_io_contexts = false)
            : m_io_contexts{ pool_size, pinned, use_multiple_io_contexts }
            , m_endpoint { boost::asio::ip::tcp::v4(), 10000 } ///< The endpoint is reset in async_startProcessing
            , m_acceptor(m_io_contexts.get())
            , m_socket(m_io_contexts.get())
        { /* nothing */ }

        //---------------------------------------------------------------------

        GIoContexts                    m_io_contexts;

        boost::asio::ip::tcp::endpoint m_endpoint;
        boost::asio::ip::tcp::acceptor m_acceptor;
        boost::asio::ip::tcp::socket   m_socket;
    };

public:
    //-------------------------------------------------------------------------
    /** @brief The default constructor */
    GBoostNetworkedConsumerBaseT() = default;

    //-------------------------------------------------------------------------
    // Deleted copy-/move-constructors and assignment operators.
    GBoostNetworkedConsumerBaseT( const GBoostNetworkedConsumerBaseT<processable_type> & ) = delete;
    GBoostNetworkedConsumerBaseT( GBoostNetworkedConsumerBaseT<processable_type> && )      = delete;
    GBoostNetworkedConsumerBaseT & operator=( const GBoostNetworkedConsumerBaseT<processable_type> & ) = delete;
    GBoostNetworkedConsumerBaseT & operator=( GBoostNetworkedConsumerBaseT<processable_type> && ) = delete;

    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    /**
	  * Sets the server name
	  *
	  * @param server The name of the server to be used by this class
	  */
    void
    setServerName( const std::string & server )
    {
        m_server = server;
    }

    //-------------------------------------------------------------------------
    /**
	  * Allows to retrieve the server name
	  *
	  * @return The name of the server configured for this class
	  */
    [[nodiscard]] std::string
    getServerName() const
    {
        return m_server;
    }

    //-------------------------------------------------------------------------
    /**
	  * Sets the serer port
	  *
	  * @param port The port to be used by the server
	  */
    void
    setPort( unsigned short port )
    {
        m_port = port;
    }

    //-------------------------------------------------------------------------
    /**
	  * Allows to retrieve the port
	  *
	  * @return The port configured for this server
	  */
    [[nodiscard]] unsigned short
    getPort() const
    {
        return m_port;
    }

    //-------------------------------------------------------------------------
    /**
	  * Allos to configure the serialization mode for the communication between
	  * clients and server
	  *
	  * @param serializationMode The serialization mode to be configured for this class
	  */
    void
    setSerializationMode( Gem::Common::serializationMode serializationMode )
    {
        m_serializationMode = serializationMode;
    }

    //-------------------------------------------------------------------------
    /**
	  * Allows to retrieve the serialization mode configured for this class
	  *
	  * @return The serialization mode configured for this class
	  */
    [[nodiscard]] Gem::Common::serializationMode
    getSerializationMode() const
    {
        return m_serializationMode;
    }

    //-------------------------------------------------------------------------
    /**
	  * Configures the number of threads to be used by this class
	  *
	  * @param The number of threads to be used by this class
	  */
    void
    setNThreads( std::size_t nThreads )
    {
        m_n_threads = nThreads;
    }

    //-------------------------------------------------------------------------
    /**
	  * Allows to retrieve the number of processing threads to be used for processing
	  * incoming connections in the server
	  */
    [[nodiscard]] std::size_t
    getNThreads() const
    {
        return m_n_threads;
    }

    //-------------------------------------------------------------------------
    /**
	 * Allows to set the socket's reuse_address option
     *
     * @param reuse_address Indicates whether the socket's reuse_address option should be set
	 */
    void
    setReuseAddress(bool reuse_address)
    {
        m_reuse_address = reuse_address;
    }

    //-------------------------------------------------------------------------
    /**
	  * Allows to retrieve the socket's reuse_address option
	  */
    [[nodiscard]] bool
    getReuseAddress() const
    {
        return m_reuse_address;
    }

protected:
    //-------------------------------------------------------------------------
    /**
     * Initialization of the consumer
     */
    void
    init_() override
    {
        // Make sure the io_context objects are properly set up, as well as depending objects
        asio_network_context_ptr = std::make_unique<asio_network_context>(m_n_threads, m_use_pinning, m_use_multiple_io_contexts);

        // Set up the endpoint according to the endpoint information we have received from the command line
        M_ENDPOINT = boost::asio::ip::tcp::endpoint { boost::asio::ip::tcp::v4(), m_port };
    }

    //-------------------------------------------------------------------------
    /**
	  * Stop execution
	  */
    void
    shutdown_() override
    {
        //------------------------------------------------------
        // Set the class-wide shutdown-flag
        GBaseConsumerT<processable_type>::shutdown_();

        // Make sure the network context exists
        if(not asio_network_context_ptr) {
            throw gemfony_exception( g_error_streamer( DO_LOG, time_and_place )
                                         << "In GBoostNetworkedConsumerBaseT<>::shutdown_():" << std::endl
                                         << "asio_network_context_ptr is empty"
                                         << std::endl );
        }

        // Stop the io_context::run()-cycle
        M_IO_CONTEXTS.stop();

        // Reset the network context
        asio_network_context_ptr.reset();
    }

    //-------------------------------------------------------------------------
    /**
	  * Adds local command line options to a boost::program_options::options_description object.
	  *
	  * @param visible Command line options that should always be visible
	  * @param hidden Command line options that should only be visible upon request
	  */
    void
    addCLOptions_( boost::program_options::options_description & visible,
                   boost::program_options::options_description & hidden ) override
    {
        namespace po = boost::program_options;

        visible.add_options()( "asio_ip",
                               po::value<std::string>( &m_server )->default_value( GCONSUMERDEFAULTSERVER ),
                               "\t[asio_base] The name or ip of the server" )(
            "asio_port",
            po::value<unsigned short>( &m_port )->default_value( GCONSUMERDEFAULTPORT ),
            "\t[asio_base] The port of the server" );

        hidden.add_options()
            ("asio_serializationMode", po::value<Gem::Common::serializationMode>( &m_serializationMode )->default_value( GCONSUMERSERIALIZATIONMODE ),
             "\t[asio_base] Specifies whether serialization shall be done in TEXTMODE (0), XMLMODE (1) or BINARYMODE (2)" )
            ("asio_nListenerThreads", po::value<std::size_t>( &m_n_threads )->default_value( m_n_threads ),
             "\t[asio_base] The number of threads used to listen for incoming connections" )
            ("asio_use_pinning", po::value<bool>( &m_use_pinning )->default_value(DEFAULTUSECOREPINNING), "\t[asio_base] Whether to pin each thread to a given core")
            ("asio_use_multiple_io_contexts", po::value<bool>( &m_use_multiple_io_contexts )->default_value(DEFAULTMULTIPLEIOCONTEXTS), "\t[asio_base] Whether to use one io_context-object for each run()-call")
            ("asio_set_no_delay", po::value<bool>( &m_use_no_delay_option )->default_value(DEFAULTUSENODELAY), "\t[asio_base] Whether to set the no_delay option on sockets")
            ("asio_reuse_address", po::value<bool>( &m_reuse_address)->default_value(DEFAULTREUSEADDRESS), "\t[asio_base] Whether the socket's reuse_address option should be set");
    }

    //-------------------------------------------------------------------------
    /**
	  * Takes a boost::program_options::variables_map object and acts on
	  * the received command line options.
	  */
    void
    actOnCLOptions_( const boost::program_options::variables_map & vm ) override
    { /* nothing */ }

    //-------------------------------------------------------------------------
    /**
	 * Starts the consumer responder loops
  	 */
    void
    async_startProcessing_() override
    {
        boost::system::error_code ec;

        // Make sure the network context exists
        if(not asio_network_context_ptr) {
            throw gemfony_exception( g_error_streamer( DO_LOG, time_and_place )
                                         << "In GBoostNetworkedConsumerBaseT<>::async_startProcessing_():" << std::endl
                                         << "asio_network_context_ptr is empty"
                                         << std::endl );
        }

        // Activate the no_delay option for the socket, if desired
        if( m_use_no_delay_option ) this->setNoDelay_();

        // Open the acceptor
        M_ACCEPTOR.open( M_ENDPOINT.protocol(), ec );
        if ( ec || not M_ACCEPTOR.is_open() ) {
            if ( ec ) {
                throw gemfony_exception(
                    g_error_streamer( DO_LOG, time_and_place )
                        << "GBoostNetworkedConsumerBaseT<>::async_startProcessing_() / m_acceptor.open: Got error message \""
                        << ec.message() << "\"" << std::endl
                        << "No connections will be accepted. The server is not running" << std::endl );
            }
            else {
                throw gemfony_exception(
                    g_error_streamer( DO_LOG, time_and_place )
                        << "GBoostNetworkedConsumerBaseT<>::async_startProcessing_() / m_acceptor.open did not succeed." << std::endl
                        << "No connections will be accepted. The server is not running" << std::endl );
            }
        }

        // Bind to the server address
        M_ACCEPTOR.bind( M_ENDPOINT, ec );
        if ( ec ) {
            throw gemfony_exception(
                g_error_streamer( DO_LOG, time_and_place )
                    << "GBoostNetworkedConsumerBaseT<>::async_startProcessing_() / m_acceptor.bind: Got error message \"" << ec.message()
                    << "\"" << std::endl
                    << "No connections will be accepted. The server is not running" << std::endl );
        }

        // Some acceptor options
        boost::asio::socket_base::reuse_address option( m_reuse_address );
        M_ACCEPTOR.set_option( option );

        // Start listening for connections
        M_ACCEPTOR.listen( boost::asio::socket_base::max_listen_connections, ec );
        if ( ec ) {
            throw gemfony_exception(
                g_error_streamer( DO_LOG, time_and_place )
                    << "GBoostNetworkedConsumerBaseT<>::async_startProcessing_() / m_acceptor.listen(): Got error message \""
                    << ec.message() << "\"" << std::endl
                    << "No connections will be accepted. The server is not running" << std::endl );
        }

        // Start accepting connections
        async_start_accept();

        // Start the run-cycle
        M_IO_CONTEXTS.run();

        // Done -- the function will return immediately
    }

    //-------------------------------------------------------------------------
    /**
	  * Asynchronously accepts new sessions requests (on the ASIO- and not the
	  * Websocket-level).
	  */
    void
    async_start_accept()
    {
        auto self = this->shared_from_this();
        M_ACCEPTOR.async_accept(M_SOCKET, [self]( boost::system::error_code ec ) { self->when_accepted( ec ); } );
    }

    //-------------------------------------------------------------------------
    /**
	  * Tries to retrieve a work item from the server, observing a timeout
	  *
	  * @return A work item (possibly empty)
	  */
    std::shared_ptr<processable_type>
    getPayloadItem()
    {
        std::shared_ptr<processable_type> p;

        // Try to retrieve a work item from the broker
        m_broker_ptr->get( p, m_timeout );

        // May be empty, if we ran into a timeout
        return p;
    }

    //-------------------------------------------------------------------------
    /**
	  * Submits a work item to the server, observing a timeout
	  */
    void
    putPayloadItem( std::shared_ptr<processable_type> p )
    {
        if ( not p ) {
            throw gemfony_exception( g_error_streamer( DO_LOG, time_and_place )
                                         << "GBoostNetworkedConsumerBaseT<>::putPayloadItem():" << std::endl
                                         << "Function called with empty work item" << std::endl );
        }

        if ( not m_broker_ptr->put( p, m_timeout ) ) {
            glogger << "In GBoostNetworkedConsumerBaseT<>::putPayloadItem():" << std::endl
                    << "Work item could not be submitted to the broker" << std::endl
                    << "The item will be discarded" << std::endl
                    << GWARNING;
        }
    }

    //-------------------------------------------------------------------------
    /**
     * Actions to be taken when a connection has been accepted
     * @param ec
     */
    virtual void when_accepted( error_code ) = 0;

    //-------------------------------------------------------------------------
    // Protected data
    std::unique_ptr<asio_network_context>   asio_network_context_ptr; ///< Will be initialized with potentially multiple io_context objects in the init call

private:
    //-------------------------------------------------------------------------
    /**
     * Activate the "no_delay"-option of sockets
     */
    void
    setNoDelay_()
    {
        boost::system::error_code      set_option_err;
        boost::asio::ip::tcp::no_delay no_delay( true );
        M_SOCKET.set_option( no_delay, set_option_err );
    }

    //-------------------------------------------------------------------------
    /**
	  * Allows to check whether this consumer needs a client to operate.
	  *
	  * @return A boolean indicating whether this consumer needs a client to operate
	  */
    [[nodiscard]] bool
    needsClient_() const noexcept override
    {
        return true;
    }

    //-------------------------------------------------------------------------
    // Private data

    std::string    m_server = GCONSUMERDEFAULTSERVER;  ///< The name or ip of the server
    unsigned short m_port   = GCONSUMERDEFAULTPORT;    ///< The port on which the server is supposed to listen

    bool           m_use_pinning = false; ///< Whether to pin each thread to its own core
    bool           m_use_multiple_io_contexts = false; ///< Whether to use a seperate io_context for each run()-call
    bool           m_use_no_delay_option = false; ///< Whether to activate the no_delay option
    bool           m_reuse_address = true; ///< Whether to set the socket's reuse_address option

    std::size_t                    m_n_threads = 0;  ///< The number of threads used to process incoming connections (0 means "automatic")

    Gem::Common::serializationMode m_serializationMode
        = Gem::Common::serializationMode::BINARY;  ///< Specifies the serialization mode

    std::shared_ptr<typename Gem::Courtier::GBrokerT<processable_type>> m_broker_ptr = GBROKER( processable_type );  ///< Simplified access to the broker
    const std::chrono::duration<double> m_timeout = std::chrono::milliseconds( GBEASTMSTIMEOUT );  ///< A timeout for put- and get-operations via the broker

    //-------------------------------------------------------------------------
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

}  // namespace Gem::Courtier