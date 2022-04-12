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

// TODO: This entire file is copied from GAsioConsumerT with simply renaming the classes.
//      So we need to change the implementation of all functions

#pragma once

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard headers go here
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <array>

// Boost headers go here
#include <boost/enable_shared_from_this.hpp>
#include <boost/utility.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/lexical_cast.hpp>

// MPI headers go here

#include <mpi.h>

// Geneva headers go here
#include "common/GThreadGroup.hpp"
#include "common/GThreadPool.hpp"
#include "common/GSerializationHelperFunctionsT.hpp"
#include "common/GCommonHelperFunctions.hpp"
#include "common/GCommonHelperFunctionsT.hpp"
#include "common/GCommonEnums.hpp"
#include "courtier/GCourtierHelperFunctions.hpp"
#include "courtier/GBrokerT.hpp"
#include "courtier/GCourtierEnums.hpp"
#include "courtier/GBaseConsumerT.hpp"
#include "courtier/GBaseClientT.hpp"
#include "courtier/GCourtierEnums.hpp"
#include "courtier/GBaseConsumerT.hpp"
#include "courtier/GCommandContainerT.hpp"

namespace Gem::Courtier {
/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class is responsible for the client side of network communication through MPI
 */
    template<typename processable_type>
    class GMPIConsumerWorkerNodeT final
            : public Gem::Courtier::GBaseClientT<processable_type>,
              public std::enable_shared_from_this<GMPIConsumerWorkerNodeT<processable_type>> {

        //-------------------------------------------------------------------------
        // Make the code easier to read
        using error_code = boost::system::error_code;
        using resolver = boost::asio::ip::tcp::resolver;
        using socket = boost::asio::ip::tcp::socket;

    public:
        GMPIConsumerWorkerNodeT(Gem::Common::serializationMode serializationMode)
        : m_serialization_mode(serializationMode) { /* nothing */ }

        //-------------------------------------------------------------------------
        /**
         * DEPRECATED, remains so far for syntactic correctness
         * Initialization with host/ip and port
         */
        GMPIConsumerWorkerNodeT(
                std::string address,
                unsigned short port,
                Gem::Common::serializationMode serialization_mode,
                std::size_t max_reconnects
        )
                : m_address(std::move(address)), m_port(port),
                  m_serialization_mode(serialization_mode),
                  m_max_reconnects(max_reconnects) { /* nothing */ }

        //-------------------------------------------------------------------------
        /**
         * The destructor
         */
        ~GMPIConsumerWorkerNodeT() override {
            glogger
                    << std::endl
                    << "GMPIConsumerClientT<> is shutting down. Processed " << this->getNProcessed()
                    << " items in total" << std::endl
                    << "\"no data\" was received " << m_n_nodata << " times" << std::endl
                    << std::endl
                    << GLOGGING;
        }

        //-------------------------------------------------------------------------
        // Deleted functions

        // Deleted default-constructor -- enforce usage of a particular constructor
        GMPIConsumerWorkerNodeT() = delete;

        // Deleted copy-constructors and assignment operators -- the client is non-copyable
        GMPIConsumerWorkerNodeT(const GMPIConsumerWorkerNodeT<processable_type> &) = delete;

        GMPIConsumerWorkerNodeT(GMPIConsumerWorkerNodeT<processable_type> &&) = delete;

        GMPIConsumerWorkerNodeT<processable_type> &
        operator=(const GMPIConsumerWorkerNodeT<processable_type> &) = delete;

        GMPIConsumerWorkerNodeT<processable_type> &operator=(GMPIConsumerWorkerNodeT<processable_type> &&) = delete;

        //-------------------------------------------------------------------------

    private:
        //-------------------------------------------------------------------------
        /**
         * Starts the main run-loop
         */
        void run_() override {
            // Prepare the outgoing string for the first request
            m_outgoing_message_str = Gem::Courtier::container_to_string(
                    m_command_container.reset(networked_consumer_payload_command::GETDATA),
                    m_serialization_mode
            );

            // Asynchronously submit the container to the remote side
            async_start_send_chain();

            // This call will block until no more work remains in the ASIO work queue
            m_io_context.run();

            // Let the audience know that we have finished the shutdown
            glogger
                    << "GMPIConsumerClientT<processable_type>::run_(): Client has terminated" << std::endl
                    << GLOGGING;
        }

        //-------------------------------------------------------------------------
        /**
         * Asynchronously starts a call chain to send m_command_container to the remote side.
         * The function assumes that the command container has been prepared appropriately
         * and remains unchanged until all data has been submitted.
         */
        void async_start_send_chain() {
            // Check if we have been asked to stop operation
            if (this->halt()) {
                this->shutdown();
                return;
            }

            // Prepare a new socket. This will delete the old socket.
            m_socket_ptr = Gem::Common::g_make_unique<boost::asio::ip::tcp::socket>(m_io_context);

            // Start looking up the domain name. This call will return immediately,
            // when_resolved() will be called once the operation is complete.
            auto self = this->shared_from_this();
            m_resolver.async_resolve(
                    m_address, std::to_string(m_port), [self](
                            boost::system::error_code ec, const resolver::results_type &results
                    ) {
                        self->when_resolved(ec, results);
                    }
            );
        }

        //-------------------------------------------------------------------------
        /**
         * Code to be executed when the async_resolve-operation has completed
         *
         * @param ec Indicates a possible error
         * @param results The results of the resolve-operation
         */
        void when_resolved(
                boost::system::error_code ec, const resolver::results_type &results
        ) {
            if (ec) {
                glogger
                        << "In GMPIConsumerClientT<processable_type>::when_resolved():" << std::endl
                        << "Got ec(\"" << ec.message() << "\"). async_connect() will not be executed." << std::endl
                        << "This will terminate the client." << std::endl
                        << GLOGGING;

                // Terminate operation and return
                this->shutdown();
                return;
            }

            // Make the ASIO connection on the endpoint we get from a lookup
            auto self = this->shared_from_this();
            boost::asio::async_connect(
                    *m_socket_ptr, results.begin(), results.end(),
                    [self](boost::system::error_code ec,
                           const boost::asio::ip::tcp::resolver::iterator & /* unused */) {
                        self->when_connected(ec);
                    }
            );
        }

        //-------------------------------------------------------------------------
        /**
         * Code to be executed when the async_connect call has completed
         *
         * @param ec Indicates a possible error
         */
        void when_connected(boost::system::error_code ec) {
            if (ec) {
                if (m_n_reconnects++ >= m_max_reconnects) { // Terminate the client
                    glogger
                            << "In GMPIConsumerClientT<processable_type>::when_connected():" << std::endl
                            << m_n_reconnects << " / " << m_max_reconnects << " failed connection attempts" << std::endl
                            << "Got error code \"" << ec.message() << "\"." << std::endl
                            << "The client will terminate now." << std::endl
                            << GLOGGING;

                    // Terminate operation and return
                    this->shutdown();
                } else { // Try to reconnect after issuing a warning
                    glogger
                            << "In GMPIConsumerClientT<processable_type>::when_connected():" << std::endl
                            << m_n_reconnects << " / " << m_max_reconnects << " failed connection attempts" << std::endl
                            << "Got error code \"" << ec.message() << "\"." << std::endl
                            << "We will try to reconnect" << std::endl
                            << GLOGGING;

                    // Get rid of the old socket
                    m_socket_ptr.reset();

                    // sleep for a short while (between 50 and 200 milliseconds, randomly),
                    // before we try to connect again.
                    std::uniform_int_distribution<> dist(500, 1000);
                    std::this_thread::sleep_for(std::chrono::milliseconds(dist(m_rng_engine)));

                    // Restart the send chain
                    async_start_send_chain();
                }

                return;
            }

            // Reset the number of connection attempts so we start at 0 next time
            m_n_reconnects = 0;

            // Send the command container off to the remote side
            auto self = this->shared_from_this();
            boost::asio::async_write(
                    *m_socket_ptr, boost::asio::buffer(m_outgoing_message_str), [self](
                            boost::system::error_code ec, std::size_t nBytesTransferred
                    ) {
                        self->when_written(ec, nBytesTransferred);
                    }
            );
        }

        //-------------------------------------------------------------------------
        /**
         * Code to be executed when the entire message was sent to the remote side
         *
         * @param ec Indicates a possible error
         */
        void when_written(
                boost::system::error_code ec, std::size_t /* nothing */
        ) {
            if (ec) {
                glogger
                        << "In GMPIConsumerClientT<processable_type>::when_written():" << std::endl
                        << "Got ec(\"" << ec.message() << "\"). async_start_read() will not be executed." << std::endl
                        << "This will terminate the client." << std::endl
                        << GLOGGING;

                // Terminate operation and return
                this->shutdown();
                return;
            }

            // Shutdown the socket in send direction. This will result in an ec of boost::asio::error::eof
            // on the server side indicating that all data was written.
            m_socket_ptr->shutdown(boost::asio::socket_base::shutdown_send);

            // Clear the outgoing message -- no longer needed
            m_outgoing_message_str.clear();

            // Initiate the read-sequence: Every transmission from client to server
            // should be answered, so we expect a response.
            auto self = this->shared_from_this();
            boost::asio::async_read(
                    *m_socket_ptr, boost::asio::dynamic_buffer(m_incoming_message_str), [self](
                            boost::system::error_code ec, std::size_t nBytesTransferred
                    ) {
                        self->when_read(ec, nBytesTransferred);
                    }
            );
        }

        //-------------------------------------------------------------------------
        /**
         * Callback to be executed when all data was written. The server session
         * is expected to shutdown its socket in send-direction, resulting in an
         * ec of boost::asio::error::eof which we use as an indication that all
         * data was received.
         *
         * @param ec A possible error code
         */
        void when_read(
                boost::system::error_code ec, std::size_t /* nothing */
        ) {

            if (ec == boost::asio::error::eof) { // The expected outcome
                // Disconnect from the remote side by destroying the socket
                m_socket_ptr.reset();

                // Deal with the message and send a response back
                async_process_request();
            } else {
                if (ec) {
                    glogger
                            << "GMPIConsumerClientT<processable_type>::when_read(): " << std::endl
                            << "Leaving due to error code " << ec.message() << std::endl
                            << GLOGGING;
                } else {
                    glogger
                            << "GMPIConsumerClientT<processable_type>::when_read(): " << std::endl
                            << "No error code was received. Expected boost::asio::error::eof" << std::endl
                            << "to indicate end of transmission." << std::endl
                            << GLOGGING;
                }

                // Terminate operation and return
                this->shutdown();
                return;
            }
        }

        //-------------------------------------------------------------------------
        /**
         * Processing of incoming messages and creation of responses takes place here
         */
        void async_process_request() {
            // Extract the string from the buffer and de-serialize the object
            Gem::Courtier::container_from_string(
                    m_incoming_message_str, m_command_container, m_serialization_mode
            ); // may throw

            // Clear the buffer, so we may later fill it with data to be sent
            m_incoming_message_str.clear();

            // Extract the command
            auto inboundCommand = m_command_container.get_command();

            // Act on the command received
            switch (inboundCommand) {
                case networked_consumer_payload_command::COMPUTE: {
                    // Process the work item ...
                    m_command_container.process();

                    // Update the processed counter
                    this->incrementProcessingCounter();

                    // ... and set the command for the way back to the server
                    m_command_container.set_command(networked_consumer_payload_command::RESULT);
                }
                    break;

                case networked_consumer_payload_command::NODATA: { // This must be a command payload
                    // Update the nodata counter for bookkeeping
                    m_n_nodata++;

                    // sleep for a short while (between 50 and 200 milliseconds, randomly),
                    // before we ask for new work.
                    std::uniform_int_distribution<> dist(50, 200);
                    std::this_thread::sleep_for(std::chrono::milliseconds(dist(m_rng_engine)));

                    // Tell the server again we need work
                    m_command_container.reset(networked_consumer_payload_command::GETDATA);
                }
                    break;

                default: {
                    // Terminate operation and return
                    this->shutdown();

                    // Emit an exception
                    throw gemfony_exception(
                            g_error_streamer(DO_LOG, time_and_place)
                                    << "GWebsocketClientT<processable_type>::process_request():" << std::endl
                                    << "Got unknown or invalid command "
                                    << boost::lexical_cast<std::string>(inboundCommand) << std::endl
                    );
                }
                    break;
            }

            // Transfer the command container into the outgoing message string
            m_outgoing_message_str = Gem::Courtier::container_to_string(
                    m_command_container, m_serialization_mode
            );

            // Asynchronously submit the container to the remote side
            async_start_send_chain();
        }

        //-------------------------------------------------------------------------
        /**
         * Shuts down the client
         */
        void shutdown() {
            // Clear the socket
            m_socket_ptr.reset();
            // Reset the work object, so it no longer keels the io_context alive
            m_work.reset();
        }

        //-------------------------------------------------------------------------
        // Data

        boost::asio::io_context m_io_context; ///< The io-service object handling the asynchronous processing
        std::unique_ptr<boost::asio::ip::tcp::socket> m_socket_ptr; ///< Holds the current socket
        boost::asio::executor_work_guard<boost::asio::io_context::executor_type> m_work = boost::asio::make_work_guard(
                m_io_context); ///< Keeps io_context.run() running
        resolver m_resolver{m_io_context}; ///< Helps to resolve the peer

        std::string m_address; ///< The ip address or name of the peer system
        unsigned int m_port; ///< The peer port
        Gem::Common::serializationMode m_serialization_mode = Gem::Common::serializationMode::BINARY; ///< Determines which serialization mode should be used

        std::size_t m_n_reconnects = 0;
        std::size_t m_max_reconnects = 0;

        std::uint64_t m_n_nodata = 0;

        std::string m_incoming_message_str; ///< Receives incoming messages
        std::string m_outgoing_message_str; ///< Helps to persist outgoing messages

        std::random_device m_nondet_rng; ///< Source of non-deterministic random numbers
        std::mt19937 m_rng_engine{m_nondet_rng()}; ///< The actual random number engine, seeded my m_nondet_rng

        GCommandContainerT<processable_type, networked_consumer_payload_command> m_command_container{
                networked_consumer_payload_command::NONE}; ///< Holds the current command and payload (if any)
    };

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * Consumer-side handling of client-connection. A new session is started for each
 * new connection and will be shut down when the request was served.
 */
    template<typename processable_type>
    class GMPIConsumerSessionT
            : public std::enable_shared_from_this<GMPIConsumerSessionT<processable_type>> {
    public:
        //-------------------------------------------------------------------------
        /**
         * The main constructor for this class
         *
         * @param socket The socket used for reading and writing data
         * @param get_payload_item A callback used to retrieve a raw payload item from the server
         * @param put_payload_item A callback used to submit a processed payload item to the server
         * @param check_server_stopped A callback used to check whether the server has been stopped
         * @param serialization_mode The serialization mode used for data transfers (binary, xml or plain text)
         */
        GMPIConsumerSessionT(
                boost::asio::io_context &io_context, boost::asio::ip::tcp::socket socket,
                std::function<std::shared_ptr<processable_type>()> get_payload_item,
                std::function<void(std::shared_ptr<processable_type>)> put_payload_item,
                std::function<bool()> check_server_stopped, Gem::Common::serializationMode serialization_mode
        )
                : m_socket(std::move(socket)), m_strand(io_context.get_executor()),
                  m_get_payload_item(std::move(get_payload_item)), m_put_payload_item(std::move(put_payload_item)),
                  m_check_server_stopped(std::move(check_server_stopped)),
                  m_serialization_mode(serialization_mode) { /* nothing */ }

        //-------------------------------------------------------------------------
        /**
         * Starts the read-write cycle as the main purpose of this class
         */
        void async_start_run() {
            // Initiate the read session -- we expect an incoming message
            async_start_read();
        }

        //-------------------------------------------------------------------------
        // Deleted constructors and assignment operators

        GMPIConsumerSessionT() = delete;

        GMPIConsumerSessionT(const GMPIConsumerSessionT<processable_type> &) = delete;

        GMPIConsumerSessionT(GMPIConsumerSessionT<processable_type> &&) = delete;

        GMPIConsumerSessionT<processable_type> &operator=(const GMPIConsumerSessionT<processable_type> &) = delete;

        GMPIConsumerSessionT<processable_type> &operator=(GMPIConsumerSessionT<processable_type> &&) = delete;

    private:
        //-------------------------------------------------------------------------
        /**
         * Starts an asynchronous read session, whose termination is signalled by
         * a call to the when_read()-function
         */
        void async_start_read() {
            if (m_check_server_stopped()) return;

            auto self = this->shared_from_this();
            boost::asio::async_read(
                    m_socket, boost::asio::dynamic_buffer(m_incoming_message_str), boost::asio::bind_executor(
                            m_strand, [self](
                                    boost::system::error_code ec, std::size_t nBytesTransferred
                            ) {
                                self->when_read(ec, nBytesTransferred);
                            }
                    )
            );
        }

        //-------------------------------------------------------------------------
        /**
         * A function to be called when all data has been read from the socket.
         * This is signalled by an eof, as the sender closes its socket in
         * send-direction.
         *
         * @param ec Indicates possible error conditions
         */
        void when_read(
                boost::system::error_code ec, std::size_t /* nothing */
        ) {
            if (ec ==
                boost::asio::error::eof) { // The expected outcome, when the client has shut down its socket in send direction
                // Deal with the message and send a response back
                async_start_write(process_request());
            } else {
                if (ec) {
                    glogger
                            << "GMPIConsumerSessionT<processable_type>::when_read(): " << std::endl
                            << "Leaving due to error code " << ec.message() << std::endl
                            << "Server session will terminate" << std::endl
                            << GLOGGING;
                } else {
                    glogger
                            << "GMPIConsumerSessionT<processable_type>::when_read(): " << std::endl
                            << "No ec received but expected boost::asio::error::eof" << std::endl
                            << "Server session will terminate" << std::endl
                            << GLOGGING;
                }
            }
        }

        //-------------------------------------------------------------------------
        /**
         * Asynchronously sends a response to the client
         *
         * @param message The message to be sent to the client
         */
        void async_start_write(std::string message) {
            // We need to persist the message for asynchronous operations
            m_outgoing_message_str = std::move(message);

            // Return an answer
            auto self = this->shared_from_this();
            boost::asio::async_write(
                    m_socket, boost::asio::buffer(m_outgoing_message_str), boost::asio::bind_executor(
                            m_strand, [self](
                                    boost::system::error_code ec, std::size_t nBytesTransferred
                            ) {
                                self->when_written(ec, nBytesTransferred);
                            }
                    )
            );
        }

        //-------------------------------------------------------------------------
        /**
         * A callback to be executed when all data was sent to the client (or when
         * an error has occurred).
         *
         * @param ec Indicates a possible error condition
         */
        void when_written(
                boost::system::error_code ec, std::size_t /* nothing */
        ) {
            if (ec) {
                glogger
                        << "GMPIConsumerSessionT<processable_type>::when_written(): " << std::endl
                        << "Got error code " << ec.message() << std::endl
                        << GLOGGING;
            }

            // Shutdown the socket in send direction. This will result in an ec of boost::asio::error::eof
            // on the client-side indicating that all data was written.
            m_socket.shutdown(boost::asio::socket_base::shutdown_send);

            // Clear the outgoing message string, no longer needed
            m_outgoing_message_str.clear();
        }

        //-------------------------------------------------------------------------
        /**
         * Steps to be taken when a request was received from the client
         *
         * @return Data to be sent to the client as a response to the request
         */
        std::string process_request() {
            try {
                // De-serialize the object
                Gem::Courtier::container_from_string(
                        m_incoming_message_str, m_command_container, m_serialization_mode
                ); // may throw

                // Clear the buffer, so we may later fill it with data to be sent
                m_incoming_message_str.clear();

                // Extract the command
                auto inboundCommand = m_command_container.get_command();

                // Act on the command received
                switch (inboundCommand) {
                    case networked_consumer_payload_command::GETDATA: {
                        return getAndSerializeWorkItem();
                    }
                        break;

                    case networked_consumer_payload_command::RESULT: {
                        // Retrieve the payload from the command container
                        auto payload_ptr = m_command_container.get_payload();

                        // Submit the payload to the server (which will send it to the broker)
                        if (payload_ptr) {
                            this->m_put_payload_item(payload_ptr);
                        } else {
                            glogger
                                    << "GMPIConsumerSessionT<processable_type>::process_request():" << std::endl
                                    << "payload is empty even though a result was expected" << std::endl
                                    << GWARNING;
                        }

                        // Retrieve the next work item and send it to the client for processing
                        return getAndSerializeWorkItem();
                    }
                        break;

                    default: {
                        glogger
                                << "GMPIConsumerSessionT<processable_type>::process_request():" << std::endl
                                << "Got unknown or invalid command " << boost::lexical_cast<std::string>(inboundCommand)
                                << std::endl
                                << GWARNING;
                    }
                        break;
                }
            } catch (...) {
                glogger
                        << "GMPIConsumerSessionT<processable_type>::process_request(): Caught exception" << std::endl
                        << GLOGGING;
            }

            // Make the compiler happy
            return std::string();
        }

        //-------------------------------------------------------------------------
        /**
         * Retrieval of a work item from the server and serialization
         *
         * @return A serialized representation of the work item
         */
        std::string getAndSerializeWorkItem() {
            // Obtain a container_payload object from the queue, serialize it and send it off
            auto payload_ptr = this->m_get_payload_item();

            if (payload_ptr) { // Did we get a valid item ?
                m_command_container.reset(networked_consumer_payload_command::COMPUTE, payload_ptr);
            } else {
                // Let the remote side know whe don't have work
                m_command_container.reset(networked_consumer_payload_command::NODATA);
            }

            return Gem::Courtier::container_to_string(
                    m_command_container, m_serialization_mode
            );
        }

        //-------------------------------------------------------------------------
        // Data

        std::string m_incoming_message_str;
        std::string m_outgoing_message_str;

        boost::asio::ip::tcp::socket m_socket;
        boost::asio::strand<boost::asio::io_context::executor_type> m_strand;

        std::function<std::shared_ptr<processable_type>()> m_get_payload_item;
        std::function<void(std::shared_ptr<processable_type>)> m_put_payload_item;
        std::function<bool()> m_check_server_stopped;

        Gem::Common::serializationMode m_serialization_mode = Gem::Common::serializationMode::BINARY;

        GCommandContainerT<processable_type, networked_consumer_payload_command> m_command_container{
                networked_consumer_payload_command::NONE
        }; ///< Holds the current command and payload (if any)

        //-------------------------------------------------------------------------
    };


/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * It is the main responsibility of this class to start new server sessions
 * for each client connection, and to interact with the Broker. A new connection
 * is opened for each request from the client and closed once the request was
 * fulfilled.
 */
    template<typename processable_type>
    class GMPIConsumerMasterNodeT
            : public Gem::Courtier::GBaseConsumerT<processable_type> // note: GBaseConsumerT<> is non-copyable
                    , public std::enable_shared_from_this<GMPIConsumerMasterNodeT<processable_type>> {
        //-------------------------------------------------------------------------
        // Simplify usage of namespaces
        using error_code = boost::system::error_code;

    public:
        //-------------------------------------------------------------------------
        // note: this was changed in order to comply with the new MPI interface
        GMPIConsumerMasterNodeT(Gem::Common::serializationMode serializationMode)
        : m_serializationMode{serializationMode}{}

        //-------------------------------------------------------------------------
        // Deleted copy-/move-constructors and assignment operators.
        GMPIConsumerMasterNodeT(const GMPIConsumerMasterNodeT<processable_type> &) = delete;

        GMPIConsumerMasterNodeT(GMPIConsumerMasterNodeT<processable_type> &&) = delete;

        GMPIConsumerMasterNodeT &operator=(const GMPIConsumerMasterNodeT<processable_type> &) = delete;

        GMPIConsumerMasterNodeT &operator=(GMPIConsumerMasterNodeT<processable_type> &&) = delete;

        //-------------------------------------------------------------------------
        /**
         * Sets the server name
         *
         * @param server The name of the server to be used by this class
         */
        void setServerName(const std::string &server) {
            m_server = server;
        }

        //-------------------------------------------------------------------------
        /**
         * Allows to retrieve the server name
         *
         * @return The name of the server configured for this class
         */
        std::string getServerName() const {
            return m_server;
        }

        //-------------------------------------------------------------------------
        /**
         * Sets the serer port
         *
         * @param port The port to be used by the server
         */
        void setPort(unsigned short port) {
            m_port = port;
        }

        //-------------------------------------------------------------------------
        /**
         * Allows to retrieve the port
         *
         * @return The port configured for this server
         */
        unsigned short getPort() const {
            return m_port;
        }

        //-------------------------------------------------------------------------
        /**
         * Allos to configure the serialization mode for the communication between
         * clients and server
         *
         * @param serializationMode The serialization mode to be configured for this class
         */
        void setSerializationMode(Gem::Common::serializationMode serializationMode) {
            m_serializationMode = serializationMode;
        }

        //-------------------------------------------------------------------------
        /**
         * Allows to retrieve the serialization mode configured for this class
         *
         * @return The serialization mode configured for this class
         */
        Gem::Common::serializationMode getSerializationMode() const {
            return m_serializationMode;
        }

        //-------------------------------------------------------------------------
        /**
         * Configures the number of threads to be used by this class
         *
         * @param The number of threads to be used by this class
         */
        void setNThreads(std::size_t nThreads) {
            // Adapt the number of processing threads, if automatic detection was requested
            if (0 == nThreads) {
                glogger
                        << "In GMPIConsumerT<>::setNThreads(): " << std::endl
                        << "nThreads was set to 0. m_n_threads will be set to default " << GCONSUMERLISTENERTHREADS
                        << std::endl
                        << "This replaces the old behaviour where a value of 0 would have" << std::endl
                        << "resulted in the number of hardware threads being unused" << std::endl
                        << GWARNING;

                m_n_threads = GCONSUMERLISTENERTHREADS;
            } else {
                m_n_threads = nThreads;
            }
        }

        //-------------------------------------------------------------------------
        /**
         * Allows to retrieve the number of processing threads to be used for processing
         * incoming connections in the server
         */
        std::size_t getNProcessingThreads() const {
            return m_n_threads;
        }

        //-------------------------------------------------------------------------
        /**
         * Configures the maximum number of times a client will try to connect to
         * the server until it terminates.
         */
        void setMaxReconnects(std::size_t n_max_reconnects) {
            m_n_max_reconnects = n_max_reconnects;
        }

        //-------------------------------------------------------------------------
        /**
         * Allows to retrieve the maximum number of times a client will try to connect to
         * the server until it terminates.
         */
        std::size_t getMaxReconnects() const {
            return m_n_max_reconnects;
        }

    protected:
        //-------------------------------------------------------------------------
        /**
         * Stop execution
         */
        void shutdown_() override {
            //------------------------------------------------------
            // Set the class-wide shutdown-flag
            GBaseConsumerT<processable_type>::shutdown_();

            // Make sure context threads may terminate
            m_io_context.stop();

            //------------------------------------------------------
            // Wait for context threads to finish
            for (auto &t: m_context_thread_cnt) { t.join(); }
            m_context_thread_cnt.clear();

            //------------------------------------------------------
        }

    private:
        //-------------------------------------------------------------------------
        /**
         * Adds local command line options to a boost::program_options::options_description object.
         *
         * @param visible Command line options that should always be visible
         * @param hidden Command line options that should only be visible upon request
         */
        void addCLOptions_(
                boost::program_options::options_description &visible,
                boost::program_options::options_description &hidden
        ) override {
            namespace po = boost::program_options;

            visible.add_options()
                    ("asio_ip", po::value<std::string>(&m_server)->default_value(GCONSUMERDEFAULTSERVER),
                     "\t[asio] The name or ip of the server")
                    ("asio_port", po::value<unsigned short>(&m_port)->default_value(GCONSUMERDEFAULTPORT),
                     "\t[asio] The port of the server");

            hidden.add_options()
                    ("asio_serializationMode",
                     po::value<Gem::Common::serializationMode>(&m_serializationMode)->default_value(
                             GCONSUMERSERIALIZATIONMODE),
                     "\t[asio] Specifies whether serialization shall be done in TEXTMODE (0), XMLMODE (1) or BINARYMODE (2)")
                    ("asio_nProcessingThreads",
                     po::value<std::size_t>(&m_n_threads)->default_value(GCONSUMERLISTENERTHREADS),
                     "\t[asio] The number of threads used to process incoming connections")
                    ("asio_maxReconnects",
                     po::value<std::size_t>(&m_n_max_reconnects)->default_value(GMPICONSUMERMAXCONNECTIONATTEMPTS),
                     "\t[asio] The maximum number of times a client will try to reconnect to the server when no connection could be established");
        }

        //-------------------------------------------------------------------------
        /**
         * Takes a boost::program_options::variables_map object and acts on
         * the received command line options.
         */
        void actOnCLOptions_(const boost::program_options::variables_map &vm) override { /* nothing */ }

        //-------------------------------------------------------------------------
        /**
         * A unique identifier for a given consumer
         *
         * @return A unique identifier for a given consumer
         */
        std::string getConsumerName_() const override {
            return std::string("GMPIConsumerT");
        }

        //-------------------------------------------------------------------------
        /**
         * Returns a short identifier for this consumer
         */
        std::string getMnemonic_() const override {
            return std::string("asio");
        }

        //-------------------------------------------------------------------------
        /**
         * Starts the consumer responder loops
         */
        void async_startProcessing_() override {
            boost::system::error_code ec;

            // Set up the endpoint according to the endpoint information we have received from the command line
            m_endpoint = std::move(boost::asio::ip::tcp::endpoint{boost::asio::ip::tcp::v4(), m_port});

            // Open the acceptor
            m_acceptor.open(m_endpoint.protocol(), ec);
            if (ec || not m_acceptor.is_open()) {
                if (ec) {
                    throw gemfony_exception(
                            g_error_streamer(DO_LOG, time_and_place)
                                    << "GMPIConsumerT<>::async_startProcessing_() / m_acceptor.open: Got error message \""
                                    << ec.message() << "\"" << std::endl
                                    << "No connections will be accepted. The server is not running" << std::endl
                    );
                } else {
                    throw gemfony_exception(
                            g_error_streamer(DO_LOG, time_and_place)
                                    << "GMPIConsumerT<>::async_startProcessing_() / m_acceptor.open did not succeed."
                                    << std::endl
                                    << "No connections will be accepted. The server is not running" << std::endl
                    );
                }
            }

            // Bind to the server address
            m_acceptor.bind(m_endpoint, ec);
            if (ec) {
                throw gemfony_exception(
                        g_error_streamer(DO_LOG, time_and_place)
                                << "GMPIConsumerT<>::async_startProcessing_() / m_acceptor.bind: Got error message \""
                                << ec.message() << "\"" << std::endl
                                << "No connections will be accepted. The server is not running" << std::endl
                );
            }

            // Some acceptor options
            boost::asio::socket_base::reuse_address option(true);
            m_acceptor.set_option(option);

            // Start listening for connections
            m_acceptor.listen(boost::asio::socket_base::max_listen_connections, ec);
            if (ec) {
                throw gemfony_exception(
                        g_error_streamer(DO_LOG, time_and_place)
                                << "GMPIConsumerT<>::async_startProcessing_() / m_acceptor.listen: Got error message \""
                                << ec.message() << "\"" << std::endl
                                << "No connections will be accepted. The server is not running" << std::endl
                );
            }

            // Start accepting connections
            async_start_accept();

            // Cross-check ...
            assert(m_n_threads > 0);

            // Allow to serve requests from multiple threads
            m_context_thread_cnt.reserve(m_n_threads);
            for (std::size_t t_cnt = 0; t_cnt < m_n_threads; t_cnt++) {
                m_context_thread_cnt.emplace_back(
                        [this]() {
                            this->m_io_context.run();
                        }
                );
            }

            // Done -- the function will return immediately
        }

        //-------------------------------------------------------------------------
        /**
         * Asynchronously accepts new sessions requests (on the ASIO- and not the
         * Websocket-level).
         */
        void async_start_accept() {
            auto self = this->shared_from_this();
            m_acceptor.async_accept(
                    m_socket, [self](boost::system::error_code ec) {
                        self->when_accepted(ec);
                    }
            );
        }

        //-------------------------------------------------------------------------
        /**
         * This callback will be executed when a new session has been accepted
         *
         * @param ec The code of a possible error
         */
        void when_accepted(error_code ec) {
            if (ec) {
                glogger
                        << "In GMPIConsumerT<>::when_accepted(): Got error code \"" << ec.message() << "\""
                        << std::endl
                        << "We will nevertheless try to accept more connections" << std::endl
                        << GWARNING;
            } else {
                // Create the GMPIConsumerSessionT and run it. This call will return immediately.
                std::make_shared<GMPIConsumerSessionT<processable_type>>(
                        m_io_context, std::move(m_socket) // Our local m_socket will stay in a valid state
                        , [this]() -> std::shared_ptr<processable_type> { return this->getPayloadItem(); },
                        [this](std::shared_ptr<processable_type> p) { this->putPayloadItem(p); },
                        [this]() -> bool { return this->stopped(); }, m_serializationMode
                )->async_start_run();
            }

            // Accept another connection
            if (not this->stopped()) async_start_accept();
        }

        //-------------------------------------------------------------------------
        /**
         * Tries to retrieve a work item from the server, observing a timeout
         *
         * @return A work item (possibly empty)
         */
        std::shared_ptr<processable_type> getPayloadItem() {
            std::shared_ptr<processable_type> p;

            // Try to retrieve a work item from the broker
            m_broker_ptr->get(p, m_timeout);

            // May be empty, if we ran into a timeout
            return p;
        }

        //-------------------------------------------------------------------------
        /**
         * Submits a work item to the server, observing a timeout
         */
        void putPayloadItem(std::shared_ptr<processable_type> p) {
            if (not p) {
                throw gemfony_exception(
                        g_error_streamer(DO_LOG, time_and_place)
                                << "GMPIConsumerT<>::putPayloadItem():" << std::endl
                                << "Function called with empty work item" << std::endl
                );
            }

            if (not m_broker_ptr->put(p, m_timeout)) {
                glogger
                        << "In GMPIConsumerT<>::putPayloadItem():" << std::endl
                        << "Work item could not be submitted to the broker" << std::endl
                        << "The item will be discarded" << std::endl
                        << GWARNING;
            }
        }


        //-------------------------------------------------------------------------
        /**
         * This function returns a client associated with this consumer. By default
         * it returns an empty smart pointer, so that consumers without the need for
         * clients do not need to re-implement this function.
         */
        std::shared_ptr<typename Gem::Courtier::GBaseClientT<processable_type>> getClient_() const override {
            return std::shared_ptr<typename Gem::Courtier::GBaseClientT<processable_type>>(
                    new GMPIConsumerWorkerNodeT<processable_type>(
                            m_server, m_port, m_serializationMode, m_n_max_reconnects
                    )
            );
        }

        //-------------------------------------------------------------------------
        /**
         * Allows to check whether this consumer needs a client to operate.
         *
         * @return A boolean indicating whether this consumer needs a client to operate
         */
        bool needsClient_() const noexcept override {
            return true;
        }

        //-------------------------------------------------------------------------
        /**
         * Returns the (possibly estimated) number of concurrent processing units.
         * Note that this function does not make any assumptions whether processing
         * units are dedicated solely to a given task.
         */
        std::size_t getNProcessingUnitsEstimate_(bool &exact) const override {
            exact = false; // mark the answer as approximate
            return m_n_active_sessions.load();
        }

        //-------------------------------------------------------------------------
        /**
         * Returns an indication whether full return can be expected from this
         * consumer. Since evaluation is performed remotely, we assume that this
         * is not the case.
         */
        bool capableOfFullReturn_() const override {
            return false;
        }

        //-------------------------------------------------------------------------
        // Data

        std::string m_server = GCONSUMERDEFAULTSERVER;  ///< The name or ip if the server
        unsigned short m_port = GCONSUMERDEFAULTPORT; ///< The port on which the server is supposed to listen
        boost::asio::ip::tcp::endpoint m_endpoint{boost::asio::ip::tcp::v4(), m_port};
        std::size_t m_n_threads = GCONSUMERLISTENERTHREADS;  ///< The number of threads used to process incoming connections through io_context::run()
        boost::asio::io_context m_io_context{boost::numeric_cast<int>(m_n_threads)};
        boost::asio::ip::tcp::acceptor m_acceptor{m_io_context};
        boost::asio::ip::tcp::socket m_socket{m_io_context};
        Gem::Common::serializationMode m_serializationMode = Gem::Common::serializationMode::BINARY; ///< Specifies the serialization mode
        std::vector<std::thread> m_context_thread_cnt;
        std::atomic<std::size_t> m_n_active_sessions{0};
        std::size_t m_n_max_reconnects = GMPICONSUMERMAXCONNECTIONATTEMPTS;

        std::shared_ptr<typename Gem::Courtier::GBrokerT<processable_type>> m_broker_ptr = GBROKER(
                processable_type); ///< Simplified access to the broker
        const std::chrono::duration<double> m_timeout = std::chrono::milliseconds(
                GBEASTMSTIMEOUT); ///< A timeout for put- and get-operations via the broker

        //-------------------------------------------------------------------------
    };


/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class is responsible for checking whether the current process is the master node (rank 0) or a worker node (any other rank).
 * It is derived from both base classes - GBaseClient and GBaseConsumer and can therefore be used as either of these.
 * It instantiates the correct classes (GMPIConsumerMasterNode or GMPIConsumerWorkerNode) and forwards calls to member
 * functions to one of the two classes. This serves as an abstraction of MPI such that the user does not need to explicitly
 * ask for the process's rank.
 *
 */
    template<typename processable_type>
    class GMPIConsumerT final
            : public Gem::Courtier::GBaseConsumerT<processable_type>,
              public Gem::Courtier::GBaseClientT<processable_type>,
              public std::enable_shared_from_this<GMPIConsumerT<processable_type>> {
    public:

        //-------------------------------------------------------------------------
        /**
         * Initialization of a consumer
         *
         * @param serialization_mode the method of serialization used by the consumer
         */
        explicit GMPIConsumerT(Gem::Common::serializationMode serialization_mode,
                               int *argc = nullptr,
                               char ***argv = nullptr)
                : m_worldRank{},
                  m_worldSize{} {

            // initialize MPI
            MPI_Init(argc, argv);

            // set members regarding the position of the process in the MPI cluster
            MPI_Comm_size(MPI_COMM_WORLD, &m_worldSize);
            MPI_Comm_rank(MPI_COMM_WORLD, &m_worldRank);

            // instantiate the correct class according to the position in the cluster
            if (isMasterNode()) {
                m_masterNode = std::make_shared<GMPIConsumerMasterNodeT<processable_type>>(serialization_mode);
            } else {
                m_workerNode = std::make_shared<GMPIConsumerWorkerNodeT<processable_type>>(serialization_mode);
            }

            std::cout << "MPI node with rank " << m_worldRank << " started up." << std::endl;
        }

        //-------------------------------------------------------------------------
        /**
         * The destructor
         *
         * This constructor must call the MPI_Finalize function as this function encapsulates all
         * MPI-specific action.
         *
         */
        ~GMPIConsumerT() override {
            MPI_Finalize();
        }

        //-------------------------------------------------------------------------
        // Deleted functions

        // Deleted default-constructor -- enforce usage of a particular constructor
        GMPIConsumerT() = delete;

        // Deleted copy-constructors and assignment operators -- the client is non-copyable
        GMPIConsumerT(const GMPIConsumerT<processable_type> &) = delete;

        GMPIConsumerT(GMPIConsumerT<processable_type> &&) = delete;

        GMPIConsumerT<processable_type> &operator=(const GMPIConsumerT<processable_type> &) = delete;

        GMPIConsumerT<processable_type> &operator=(GMPIConsumerT<processable_type> &&) = delete;

        //-------------------------------------------------------------------------

        [[nodiscard]] inline bool isMasterNode() const {
            return m_worldRank == 0;
        }

        [[nodiscard]] inline bool isWorkerNode() const {
            return m_worldRank != 0;
        }

    protected:
        /**
         * Inherited from GBaseConsumerT and GBaseClientT
         */
        void shutdown_() override {
            // TODO: this is inherited from the client and the server, is that an issue?
            // TODO: implement this
            GBaseConsumerT<processable_type>::shutdown_();
        }

        void addConfigurationOptions(Common::GParserBuilder &gpb) override {
            // TODO: implement this
            GBaseConsumerT<processable_type>::addConfigurationOptions(gpb);
        }

    private:
        //-------------------------------------------------------------------------
        // Private methods

        /**
         * Inherited from GBaseConsumerT
         * @param description
         * @param optionsDescription
         */
        void addCLOptions_(boost::program_options::options_description &description,
                           boost::program_options::options_description &optionsDescription) BASE override {
            // TODO: implement this
        }

        /**
         * Inherited from GBaseConsumerT
         * @param map
         */
        void actOnCLOptions_(const boost::program_options::variables_map &map) BASE override {
            // TODO: implement this
        }

        /**
         * Inherited from GBaseConsumerT
         * @return
         */
        [[nodiscard]] std::string getConsumerName_() const BASE override {
            // TODO: implement this
            return {};
        }

        /**
         * Inherited form GBaseConsumerT
         * @return
         */
        [[nodiscard]] std::string getMnemonic_() const BASE override {
            // TODO: implement this
            return {};
        }

        /**
         * Inherited from GBaseConsumerT
         */
        void async_startProcessing_() BASE override {
            // TODO: implement this
        }

        /**
         * Inherited from GBaseConsumerT
         * @return
         */
        std::shared_ptr<GBaseClientT<processable_type>> getClient_() const override {
            // TODO: implement this
        }

        /**
         * Inherited from GBaseConsumerT
         * @return
         */
        [[nodiscard]] bool needsClient_() const noexcept override {
            // TODO: implement this
        }

        /**
         * Inherited from GBaseConsumerT
         * @param exact
         * @return
         */
        size_t getNProcessingUnitsEstimate_(bool &exact) const BASE override {
            // TODO: implement this
            return 0;
        }

        /**
         * Inherited from GBaseConsumerT
         * @return
         */
        [[nodiscard]] bool capableOfFullReturn_() const BASE override {
            // TODO: implement this
            return false;
        }

        /**
         * Inherited from GBaseClient
         */
        void run_() BASE override {
            // TODO: implement this
            glogger << "MPIConsumerT called run()_" << std::endl;
        }

        //-------------------------------------------------------------------------
        // Data

        std::shared_ptr<GMPIConsumerMasterNodeT<processable_type>> m_masterNode;
        std::shared_ptr<GMPIConsumerWorkerNodeT<processable_type>> m_workerNode;
        int m_worldSize;
        int m_worldRank;
    };


} /* namespace Gem::courtier */

