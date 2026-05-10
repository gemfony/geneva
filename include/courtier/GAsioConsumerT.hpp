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

namespace Gem::Courtier {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class is responsible for the client side of network communication
 * with Boost::Beast. Connections are kept open permanently.
 */
template <typename processable_type>
class GAsioConsumerClientT final
  : public Gem::Courtier::GBaseClientT<processable_type>
  , public std::enable_shared_from_this<GAsioConsumerClientT<processable_type>> {
    //-------------------------------------------------------------------------
    // Make the code easier to read

    using error_code = boost::system::error_code;
    using resolver = boost::asio::ip::tcp::resolver;
    using socket = boost::asio::ip::tcp::socket;

public:
    //-------------------------------------------------------------------------
    /**
	  * Initialization with host/ip and port
	  */
    GAsioConsumerClientT(
        std::string address,
        unsigned short port,
        Gem::Common::serializationMode serialization_mode,
        std::size_t max_reconnects
    )
      : address_(std::move(address))
      , port_(port)
      , serialization_mode_(serialization_mode)
      , max_reconnects_(max_reconnects) { /* nothing */
    }

    //-------------------------------------------------------------------------
    /**
	  * The destructor
	  */
    ~GAsioConsumerClientT() override {
        glogger << std::endl
                << "GAsioConsumerClientT<> is shutting down. Processed " << this->getNProcessed()
                << " items in total" << std::endl
                << "\"no data\" was received " << n_nodata_ << " times" << std::endl
                << std::endl
                << GLOGGING;
    }

    //-------------------------------------------------------------------------
    // Deleted functions

    // Deleted default-constructor -- enforce usage of a particular constructor
    GAsioConsumerClientT() = delete;
    // Deleted copy-constructors and assignment operators -- the client is non-copyable
    GAsioConsumerClientT(const GAsioConsumerClientT<processable_type> &) = delete;
    GAsioConsumerClientT(GAsioConsumerClientT<processable_type> &&) = delete;
    GAsioConsumerClientT<processable_type> &
    operator=(const GAsioConsumerClientT<processable_type> &) = delete;
    GAsioConsumerClientT<processable_type> &
    operator=(GAsioConsumerClientT<processable_type> &&) = delete;

    //-------------------------------------------------------------------------

private:
    //-------------------------------------------------------------------------
    /**
	  * Starts the main run-loop
	  */
    void run_() override {
        // Prepare the outgoing string for the first request
        outgoing_message_str_ = Gem::Courtier::container_to_string(
            command_container_.reset(networked_consumer_payload_command::GETDATA),
            serialization_mode_
        );

        // Asynchronously submit the container to the remote side
        async_start_send_chain();

        // This call will block until no more work remains in the ASIO work queue
        io_context_.run();

        // Let the audience know that we have finished the shutdown
        glogger << "GAsioConsumerClientT<processable_type>::run_(): Client has terminated"
                << std::endl
                << GLOGGING;
    }

    //-------------------------------------------------------------------------
    /**
	  * Asynchronously starts a call chain to send command_container_ to the remote side.
	  * The function assumes that the command container has been prepared appropriately
	  * and remains unchanged until all data has been submitted.
	  */
    void async_start_send_chain() {
        // Check if we have been asked to stop operation
        if(this->halt()) {
            this->shutdown();
            return;
        }

        // Prepare a new socket. This will delete the old socket.
        socket_ptr_ = std::make_unique<boost::asio::ip::tcp::socket>(io_context_);

        // Start looking up the domain name. This call will return immediately,
        // when_resolved() will be called once the operation is complete.
        auto self = this->shared_from_this();
        resolver_.async_resolve(
            address_,
            std::to_string(port_),
            [self](boost::system::error_code ec, const resolver::results_type &results) {
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
    void when_resolved(boost::system::error_code ec, const resolver::results_type &results) {
        if(ec) {
            glogger << "In GAsioConsumerClientT<processable_type>::when_resolved():" << std::endl
                    << "Got ec(\"" << ec.message() << "\"). async_connect() will not be executed."
                    << std::endl
                    << "This will terminate the client." << std::endl
                    << GLOGGING;

            // Terminate operation and return
            this->shutdown();
            return;
        }

        // Make the ASIO connection on the endpoint we get from a lookup
        auto self = this->shared_from_this();
        boost::asio::async_connect(
            *socket_ptr_,
            results.begin(),
            results.end(),
            [self](boost::system::error_code ec, auto /* unused */) { self->when_connected(ec); }
        );
    }

    //-------------------------------------------------------------------------
    /**
	  * Code to be executed when the async_connect call has completed
	  *
	  * @param ec Indicates a possible error
	  */
    void when_connected(boost::system::error_code ec) {
        if(ec) {
            if(n_reconnects_++ >= max_reconnects_) { // Terminate the client
                glogger << "In GAsioConsumerClientT<processable_type>::when_connected():"
                        << std::endl
                        << n_reconnects_ << " / " << max_reconnects_
                        << " failed connection attempts" << std::endl
                        << "Got error code \"" << ec.message() << "\"." << std::endl
                        << "The client will terminate now." << std::endl
                        << GLOGGING;

                // Terminate operation and return
                this->shutdown();
            }
            else { // Try to reconnect after issuing a warning
                glogger << "In GAsioConsumerClientT<processable_type>::when_connected():"
                        << std::endl
                        << n_reconnects_ << " / " << max_reconnects_
                        << " failed connection attempts" << std::endl
                        << "Got error code \"" << ec.message() << "\"." << std::endl
                        << "We will try to reconnect" << std::endl
                        << GLOGGING;

                // Get rid of the old socket
                socket_ptr_.reset();

                // sleep for a short while (between 50 and 200 milliseconds, randomly),
                // before we try to connect again.
                std::uniform_int_distribution<> dist(500, 1000);
                std::this_thread::sleep_for(std::chrono::milliseconds(dist(rng_engine_)));

                // Restart the send chain
                async_start_send_chain();
            }

            return;
        }

        // Reset the number of connection attempts so we start at 0 next time
        n_reconnects_ = 0;

        // Send the command container off to the remote side
        auto self = this->shared_from_this();
        boost::asio::async_write(
            *socket_ptr_,
            boost::asio::buffer(outgoing_message_str_),
            [self](boost::system::error_code ec, std::size_t nBytesTransferred) {
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
        boost::system::error_code ec,
        std::size_t /* nothing */
    ) {
        if(ec) {
            glogger << "In GAsioConsumerClientT<processable_type>::when_written():" << std::endl
                    << "Got ec(\"" << ec.message()
                    << "\"). async_start_read() will not be executed." << std::endl
                    << "This will terminate the client." << std::endl
                    << GLOGGING;

            // Terminate operation and return
            this->shutdown();
            return;
        }

        // Shutdown the socket in send direction. This will result in an ec of boost::asio::error::eof
        // on the server side indicating that all data was written.
        socket_ptr_->shutdown(boost::asio::socket_base::shutdown_send);

        // Clear the outgoing message -- no longer needed
        outgoing_message_str_.clear();

        // Initiate the read-sequence: Every transmission from client to server
        // should be answered, so we expect a response.
        auto self = this->shared_from_this();
        boost::asio::async_read(
            *socket_ptr_,
            boost::asio::dynamic_buffer(incoming_message_str_),
            [self](boost::system::error_code ec, std::size_t nBytesTransferred) {
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
        boost::system::error_code ec,
        std::size_t /* nothing */
    ) {
        if(ec == boost::asio::error::eof) { // The expected outcome
            // Disconnect from the remote side by destroying the socket
            socket_ptr_.reset();

            // Deal with the message and send a response back
            async_process_request();
        }
        else {
            if(ec) {
                glogger << "GAsioConsumerClientT<processable_type>::when_read(): " << std::endl
                        << "Leaving due to error code " << ec.message() << std::endl
                        << GLOGGING;
            }
            else {
                glogger << "GAsioConsumerClientT<processable_type>::when_read(): " << std::endl
                        << "No error code was received. Expected boost::asio::error::eof"
                        << std::endl
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
            incoming_message_str_,
            command_container_,
            serialization_mode_
        ); // may throw

        // Clear the buffer, so we may later fill it with data to be sent
        incoming_message_str_.clear();

        // Extract the command
        auto inboundCommand = command_container_.get_command();

        // Act on the command received
        switch(inboundCommand) {
        case networked_consumer_payload_command::COMPUTE: {
            // Process the work item ...
            command_container_.process();

            // Update the processed counter
            this->incrementProcessingCounter();

            // ... and set the command for the way back to the server
            command_container_.set_command(networked_consumer_payload_command::RESULT);
        } break;

        case networked_consumer_payload_command::NODATA: { // This must be a command payload
            // Update the nodata counter for bookkeeping
            n_nodata_++;

            // sleep for a short while (between 50 and 200 milliseconds, randomly),
            // before we ask for new work.
            std::uniform_int_distribution<> dist(50, 200);
            std::this_thread::sleep_for(std::chrono::milliseconds(dist(rng_engine_)));

            // Tell the server again we need work
            command_container_.reset(networked_consumer_payload_command::GETDATA);
        } break;

        default: {
            // Terminate operation and return
            this->shutdown();

            // Emit an exception
            throw geneva_exception(
                g_error_streamer(DO_LOG, time_and_place)
                << "GWebsocketClientT<processable_type>::process_request():" << std::endl
                << "Got unknown or invalid command "
                << boost::lexical_cast<std::string>(inboundCommand) << std::endl
            );
        } break;
        }

        // Transfer the command contaner into the outgoing message string
        outgoing_message_str_ =
            Gem::Courtier::container_to_string(command_container_, serialization_mode_);

        // Asynchronously submit the container to the remote side
        async_start_send_chain();
    }

    //-------------------------------------------------------------------------
    /**
	  * Shuts down the client
	  */
    void shutdown() {
        // Clear the socket
        socket_ptr_.reset();
        // Reset the work object, so it no longer keels the io_context alive
        work_.reset();
    }

    //-------------------------------------------------------------------------
    // Data

    boost::asio::io_context
        io_context_; ///< The io-service object handling the asynchronous processing
    std::unique_ptr<boost::asio::ip::tcp::socket> socket_ptr_; ///< Holds the current socket
    boost::asio::executor_work_guard<boost::asio::io_context::executor_type> work_ =
        boost::asio::make_work_guard(io_context_); ///< Keeps io_context.run() running
    resolver resolver_{io_context_};              ///< Helps to resolve the peer

    std::string address_; ///< The ip address or name of the peer system
    unsigned int port_;   ///< The peer port
    Gem::Common::serializationMode serialization_mode_ = Gem::Common::serializationMode::
        BINARY; ///< Determines which seriliztion mode should be used

    std::size_t n_reconnects_ = 0;
    std::size_t max_reconnects_ = 0;

    std::uint64_t n_nodata_ = 0;

    std::string incoming_message_str_; ///< Receives incoming messages
    std::string outgoing_message_str_; ///< Helps to persist outgoing messages

    std::random_device nondet_rng_; ///< Source of non-deterministic random numbers
    std::mt19937 rng_engine_{
        nondet_rng_()
    }; ///< The actual random number engine, seeded my nondet_rng_

    GCommandContainerT<processable_type, networked_consumer_payload_command> command_container_{
        networked_consumer_payload_command::NONE
    }; ///< Holds the current command and payload (if any)
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * Consumer-side handling of client-connection. A new session is started for each
 * new connection and will be shut down when the request was served.
 */
template <typename processable_type>
class GAsioConsumerSessionT // NOLINT(cppcoreguidelines-special-member-functions)
  : public std::enable_shared_from_this<GAsioConsumerSessionT<processable_type>> {
public:
    //-------------------------------------------------------------------------
    /**
	  * The main constructor for this class
	  *
	  * @param socket The socket used for readung and writing data
	  * @param get_payload_item A callback used to retrieve a raw payload item from the server
	  * @param put_payload_item A callback used to submit a processed payload item to the server
	  * @param check_server_stopped A callback used to check whether the server has been stopped
	  * @param serialization_mode The serialization mode used for data transfers (binary, xml or plain text)
	  */
    GAsioConsumerSessionT(
        boost::asio::io_context &io_context,
        boost::asio::ip::tcp::socket socket,
        std::function<std::shared_ptr<processable_type>()> get_payload_item,
        std::function<void(std::shared_ptr<processable_type>)> put_payload_item,
        std::function<bool()> check_server_stopped,
        Gem::Common::serializationMode serialization_mode
    )
      : socket_(std::move(socket))
      , strand_(io_context.get_executor())
      , get_payload_item_(std::move(get_payload_item))
      , put_payload_item_(std::move(put_payload_item))
      , check_server_stopped_(std::move(check_server_stopped))
      , serialization_mode_(serialization_mode) { /* nothing */
    }

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

    GAsioConsumerSessionT() = delete;
    GAsioConsumerSessionT(const GAsioConsumerSessionT<processable_type> &) = delete;
    GAsioConsumerSessionT(GAsioConsumerSessionT<processable_type> &&) = delete;
    GAsioConsumerSessionT<processable_type> &
    operator=(const GAsioConsumerSessionT<processable_type> &) = delete;
    GAsioConsumerSessionT<processable_type> &
    operator=(GAsioConsumerSessionT<processable_type> &&) = delete;

private:
    //-------------------------------------------------------------------------
    /**
	  * Starts an asynchronous read session, whose termination is signalled by
	  * a call to the when_read()-function
	  */
    void async_start_read() {
        if(check_server_stopped_())
            return;

        auto self = this->shared_from_this();
        boost::asio::async_read(
            socket_,
            boost::asio::dynamic_buffer(incoming_message_str_),
            boost::asio::bind_executor(
                strand_,
                [self](boost::system::error_code ec, std::size_t nBytesTransferred) {
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
        boost::system::error_code ec,
        std::size_t /* nothing */
    ) {
        if(ec ==
           boost::asio::error::
               eof) { // The expected outcome, when the client has shut down its socket in send direction
            // Deal with the message and send a response back
            async_start_write(process_request());
        }
        else {
            if(ec) {
                glogger << "GAsioConsumerSessionT<processable_type>::when_read(): " << std::endl
                        << "Leaving due to error code " << ec.message() << std::endl
                        << "Server session will terminate" << std::endl
                        << GLOGGING;
            }
            else {
                glogger << "GAsioConsumerSessionT<processable_type>::when_read(): " << std::endl
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
        outgoing_message_str_ = std::move(message);

        // Return an answer
        auto self = this->shared_from_this();
        boost::asio::async_write(
            socket_,
            boost::asio::buffer(outgoing_message_str_),
            boost::asio::bind_executor(
                strand_,
                [self](boost::system::error_code ec, std::size_t nBytesTransferred) {
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
        boost::system::error_code ec,
        std::size_t /* nothing */
    ) {
        if(ec) {
            glogger << "GAsioConsumerSessionT<processable_type>::when_written(): " << std::endl
                    << "Got error code " << ec.message() << std::endl
                    << GLOGGING;
        }

        // Shutdown the socket in send direction. This will result in an ec of boost::asio::error::eof
        // on the client-side indicating that all data was written.
        socket_.shutdown(boost::asio::socket_base::shutdown_send);

        // Clear the outgoing message string, no longer needed
        outgoing_message_str_.clear();
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
                incoming_message_str_,
                command_container_,
                serialization_mode_
            ); // may throw

            // Clear the buffer, so we may later fill it with data to be sent
            incoming_message_str_.clear();

            // Extract the command
            auto inboundCommand = command_container_.get_command();

            // Act on the command received
            switch(inboundCommand) {
            case networked_consumer_payload_command::GETDATA: {
                return getAndSerializeWorkItem();
            } break;

            case networked_consumer_payload_command::RESULT: {
                // Retrieve the payload from the command container
                auto payload_ptr = command_container_.get_payload();

                // Submit the payload to the server (which will send it to the broker)
                if(payload_ptr) {
                    this->put_payload_item_(payload_ptr);
                }
                else {
                    glogger << "GAsioConsumerSessionT<processable_type>::process_request():"
                            << std::endl
                            << "payload is empty even though a result was expected" << std::endl
                            << GWARNING;
                }

                // Retrieve the next work item and send it to the client for processing
                return getAndSerializeWorkItem();
            } break;

            default: {
                glogger << "GAsioConsumerSessionT<processable_type>::process_request():"
                        << std::endl
                        << "Got unknown or invalid command "
                        << boost::lexical_cast<std::string>(inboundCommand) << std::endl
                        << GWARNING;
            } break;
            }
        }
        catch(
            ...
        ) { // NOLINT(bugprone-empty-catch) — intentionally swallowed; session ends, caller retries
            glogger
                << "GAsioConsumerSessionT<processable_type>::process_request(): Caught exception"
                << std::endl
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
        auto payload_ptr = this->get_payload_item_();

        if(payload_ptr) { // Did we get a valid item ?
            command_container_.reset(networked_consumer_payload_command::COMPUTE, payload_ptr);
        }
        else {
            // Let the remote side know whe don't have work
            command_container_.reset(networked_consumer_payload_command::NODATA);
        }

        return Gem::Courtier::container_to_string(command_container_, serialization_mode_);
    }

    //-------------------------------------------------------------------------
    // Data

    std::string incoming_message_str_;
    std::string outgoing_message_str_;

    boost::asio::ip::tcp::socket socket_;
    boost::asio::strand<boost::asio::io_context::executor_type> strand_;

    std::function<std::shared_ptr<processable_type>()> get_payload_item_;
    std::function<void(std::shared_ptr<processable_type>)> put_payload_item_;
    std::function<bool()> check_server_stopped_;

    Gem::Common::serializationMode serialization_mode_ = Gem::Common::serializationMode::BINARY;

    GCommandContainerT<processable_type, networked_consumer_payload_command> command_container_{
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
template <typename processable_type>
class GAsioConsumerT // NOLINT(cppcoreguidelines-special-member-functions)
  : public Gem::Courtier::GBaseConsumerT<processable_type> // note: GBaseConsumerT<> is non-copyable
  , public std::enable_shared_from_this<GAsioConsumerT<processable_type>> {
    //-------------------------------------------------------------------------
    // Simplify usage of namespaces
    using error_code = boost::system::error_code;

public:
    //-------------------------------------------------------------------------
    /** @brief The default constructor */
    GAsioConsumerT() = default;

    //-------------------------------------------------------------------------
    // Deleted copy-/move-constructors and assignment operators.
    GAsioConsumerT(const GAsioConsumerT<processable_type> &) = delete;
    GAsioConsumerT(GAsioConsumerT<processable_type> &&) = delete;
    GAsioConsumerT &operator=(const GAsioConsumerT<processable_type> &) = delete;
    GAsioConsumerT &operator=(GAsioConsumerT<processable_type> &&) = delete;

    //-------------------------------------------------------------------------
    /**
	  * Sets the server name
	  *
	  * @param server The name of the server to be used by this class
	  */
    void setServerName(const std::string &server) {
        server_ = server;
    }

    //-------------------------------------------------------------------------
    /**
	  * Allows to retrieve the server name
	  *
	  * @return The name of the server configured for this class
	  */
    std::string getServerName() const {
        return server_;
    }

    //-------------------------------------------------------------------------
    /**
	  * Sets the serer port
	  *
	  * @param port The port to be used by the server
	  */
    void setPort(unsigned short port) {
        port_ = port;
    }

    //-------------------------------------------------------------------------
    /**
	  * Allows to retrieve the port
	  *
	  * @return The port configured for this server
	  */
    unsigned short getPort() const {
        return port_;
    }

    //-------------------------------------------------------------------------
    /**
	  * Allos to configure the serialization mode for the communication between
	  * clients and server
	  *
	  * @param serializationMode The serialization mode to be configured for this class
	  */
    void setSerializationMode(Gem::Common::serializationMode serializationMode) {
        serializationMode_ = serializationMode;
    }

    //-------------------------------------------------------------------------
    /**
	  * Allows to retrieve the serialization mode configured for this class
	  *
	  * @return The serialization mode configured for this class
	  */
    Gem::Common::serializationMode getSerializationMode() const {
        return serializationMode_;
    }

    //-------------------------------------------------------------------------
    /**
	  * Configures the number of threads to be used by this class
	  *
	  * @param The number of threads to be used by this class
	  */
    void setNThreads(std::size_t nThreads) {
        // Adapt the number of processing threads, if automatic detection was requested
        if(0 == nThreads) {
            glogger << "In GAsioConsumerT<>::setNThreads(): " << std::endl
                    << "nThreads was set to 0. n_threads_ will be set to default "
                    << GCONSUMERLISTENERTHREADS << std::endl
                    << "This replaces the old behaviour where a value of 0 would have" << std::endl
                    << "resulted in the number of hardware threads being unsed" << std::endl
                    << GWARNING;

            n_threads_ = GCONSUMERLISTENERTHREADS;
        }
        else {
            n_threads_ = nThreads;
        }
    }

    //-------------------------------------------------------------------------
    /**
	  * Allows to retrieve the number of processing threads to be used for processing
	  * incoming connections in the server
	  */
    std::size_t getNProcessingThreads() const {
        return n_threads_;
    }

    //-------------------------------------------------------------------------
    /**
	  * Configures the maximum number of times a client will try to connect to
	  * the server until it terminates.
	  */
    void setMaxReconnects(std::size_t n_max_reconnects) {
        n_max_reconnects_ = n_max_reconnects;
    }

    //-------------------------------------------------------------------------
    /**
	  * Allows to retrieve the maximum number of times a client will try to connect to
	  * the server until it terminates.
	  */
    std::size_t getMaxReconnects() const {
        return n_max_reconnects_;
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
        io_context_.stop();

        //------------------------------------------------------
        // Wait for context threads to finish
        for(auto &t : context_thread_cnt_) {
            t.join();
        }
        context_thread_cnt_.clear();

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

        visible
            .add_options()("asio_ip", po::value<std::string>(&server_)->default_value(GCONSUMERDEFAULTSERVER), "\t[asio] The name or ip of the server")(
                "asio_port",
                po::value<unsigned short>(&port_)->default_value(GCONSUMERDEFAULTPORT),
                "\t[asio] The port of the server"
            );

        hidden.add_options()
			 ("asio_serializationMode", po::value<Gem::Common::serializationMode>(&serializationMode_)->default_value(GCONSUMERSERIALIZATIONMODE),
				 "\t[asio] Specifies whether serialization shall be done in TEXTMODE (0), XMLMODE (1) or BINARYMODE (2)")
			 ("asio_nProcessingThreads", po::value<std::size_t>(&n_threads_)->default_value(GCONSUMERLISTENERTHREADS),
				 "\t[asio] The number of threads used to process incoming connections")
			 ("asio_maxReconnects", po::value<std::size_t>(&n_max_reconnects_)->default_value(GASIOCONSUMERMAXCONNECTIONATTEMPTS),
			 	 "\t[asio] The maximum number of times a client will try to reconnect to the server when no connection could be established");
    }

    //-------------------------------------------------------------------------
    /**
	  * Takes a boost::program_options::variables_map object and acts on
	  * the received command line options.
	  */
    void actOnCLOptions_(const boost::program_options::variables_map &vm) override { /* nothing */
    }

    //-------------------------------------------------------------------------
    /**
	  * A unique identifier for a given consumer
	  *
	  * @return A unique identifier for a given consumer
	  */
    std::string getConsumerName_() const override {
        return std::string("GAsioConsumerT");
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
        endpoint_ = std::move(boost::asio::ip::tcp::endpoint{boost::asio::ip::tcp::v4(), port_});

        // Open the acceptor
        acceptor_.open(endpoint_.protocol(), ec);
        if(ec || not acceptor_.is_open()) {
            if(ec) {
                throw geneva_exception(
                    g_error_streamer(DO_LOG, time_and_place)
                    << "GAsioConsumerT<>::async_startProcessing_() / acceptor_.open: Got error "
                       "message \""
                    << ec.message() << "\"" << std::endl
                    << "No connections will be accepted. The server is not running" << std::endl
                );
            }
            else {
                throw geneva_exception(
                    g_error_streamer(DO_LOG, time_and_place)
                    << "GAsioConsumerT<>::async_startProcessing_() / acceptor_.open did not "
                       "succeed."
                    << std::endl
                    << "No connections will be accepted. The server is not running" << std::endl
                );
            }
        }

        // Bind to the server address
        acceptor_.bind(endpoint_, ec);
        if(ec) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, time_and_place)
                << "GAsioConsumerT<>::async_startProcessing_() / acceptor_.bind: Got error "
                   "message \""
                << ec.message() << "\"" << std::endl
                << "No connections will be accepted. The server is not running" << std::endl
            );
        }

        // Some acceptor options
        boost::asio::socket_base::reuse_address option(true);
        acceptor_.set_option(option);

        // Start listening for connections
        acceptor_.listen(boost::asio::socket_base::max_listen_connections, ec);
        if(ec) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, time_and_place)
                << "GAsioConsumerT<>::async_startProcessing_() / acceptor_.listen: Got error "
                   "message \""
                << ec.message() << "\"" << std::endl
                << "No connections will be accepted. The server is not running" << std::endl
            );
        }

        // Start accepting connections
        async_start_accept();

        // Cross-check ...
        assert(n_threads_ > 0);

        // Allow to serve requests from multiple threads
        context_thread_cnt_.reserve(n_threads_);
        for(std::size_t t_cnt = 0; t_cnt < n_threads_; t_cnt++) {
            context_thread_cnt_.emplace_back([this]() { this->io_context_.run(); });
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
        acceptor_.async_accept(socket_, [self](boost::system::error_code ec) {
            self->when_accepted(ec);
        });
    }

    //-------------------------------------------------------------------------
    /**
	  * This callback will be executed when a new session has been accepted
	  *
	  * @param ec The code of a possible error
	  */
    void when_accepted(error_code ec) {
        if(ec) {
            glogger << "In GAsioConsumerT<>::when_accepted(): Got error code \"" << ec.message()
                    << "\"" << std::endl
                    << "We will nevertheless try to accept more connections" << std::endl
                    << GWARNING;
        }
        else {
            // Create the GAsioConsumerSessionT and run it. This call will return immediately.
            std::make_shared<GAsioConsumerSessionT<processable_type>>(
                io_context_,
                std::move(socket_) // Our local socket_ will stay in a valid state
                ,
                [this]() -> std::shared_ptr<processable_type> { return this->getPayloadItem(); },
                [this](std::shared_ptr<processable_type> p) { this->putPayloadItem(p); },
                [this]() -> bool { return this->stopped(); },
                serializationMode_
            )
                ->async_start_run();
        }

        // Accept another connection
        if(not this->stopped())
            async_start_accept();
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
        broker_ptr_->get(p, timeout_);

        // May be empty, if we ran into a timeout
        return p;
    }

    //-------------------------------------------------------------------------
    /**
	  * Submits a work item to the server, observing a timeout
	  */
    void putPayloadItem(std::shared_ptr<processable_type> p) {
        if(not p) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, time_and_place)
                << "GAsioConsumerT<>::putPayloadItem():" << std::endl
                << "Function called with empty work item" << std::endl
            );
        }

        if(not broker_ptr_->put(p, timeout_)) {
            glogger << "In GAsioConsumerT<>::putPayloadItem():" << std::endl
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
    std::shared_ptr<typename Gem::Courtier::GBaseClientT<processable_type>>
    getClient_() const override {
        return std::shared_ptr<typename Gem::Courtier::GBaseClientT<processable_type>>(
            new GAsioConsumerClientT<processable_type>(
                server_,
                port_,
                serializationMode_,
                n_max_reconnects_
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
        return n_active_sessions_.load();
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

    std::string server_ = GCONSUMERDEFAULTSERVER; ///< The name or ip if the server
    unsigned short port_ =
        GCONSUMERDEFAULTPORT; ///< The port on which the server is supposed to listen
    boost::asio::ip::tcp::endpoint endpoint_{boost::asio::ip::tcp::v4(), port_};
    std::size_t n_threads_ =
        GCONSUMERLISTENERTHREADS; ///< The number of threads used to process incoming connections through io_context::run()
    boost::asio::io_context io_context_{boost::numeric_cast<int>(n_threads_)};
    boost::asio::ip::tcp::acceptor acceptor_{io_context_};
    boost::asio::ip::tcp::socket socket_{io_context_};
    Gem::Common::serializationMode serializationMode_ =
        Gem::Common::serializationMode::BINARY; ///< Specifies the serialization mode
    std::vector<std::thread> context_thread_cnt_;
    std::atomic<std::size_t> n_active_sessions_{0};
    std::size_t n_max_reconnects_ = GASIOCONSUMERMAXCONNECTIONATTEMPTS;

    std::shared_ptr<typename Gem::Courtier::GBrokerT<processable_type>> broker_ptr_ =
        GBROKER(processable_type); ///< Simplified access to the broker
    const std::chrono::duration<double> timeout_ = std::chrono::milliseconds(
        GBEASTMSTIMEOUT
    ); ///< A timeout for put- and get-operations via the broker

    //-------------------------------------------------------------------------
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Gem::Courtier */
