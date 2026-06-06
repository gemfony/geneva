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
#include <chrono>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

// Boost headers go here
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/asio.hpp>
#include <boost/serialization/vector.hpp>

// Geneva headers go here
#include "common/GCommonEnums.hpp"
#include "common/GCommonHelperFunctions.hpp"
#include "common/GCommonHelperFunctionsT.hpp"
#include "common/GSerializationHelperFunctionsT.hpp"
#include "common/GThreadPool.hpp"
#include "courtier/GBaseClientT.hpp"
#include "courtier/GCommandContainerT.hpp"
#include "courtier/GCourtierEnums.hpp"
#include "courtier/GCourtierHelperFunctions.hpp"

namespace Gem::Courtier::Consumers {

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
        glogger << '\n'
                << "GAsioConsumerClientT<> is shutting down. Processed " << this->getNProcessed()
                << " items in total" << '\n'
                << "\"no data\" was received " << n_nodata_ << " times" << '\n'
                << '\n'
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
                << '\n'
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
            glogger << "In GAsioConsumerClientT<processable_type>::when_resolved():" << '\n'
                    << "Got ec(\"" << ec.message() << "\"). async_connect() will not be executed."
                    << '\n'
                    << "This will terminate the client." << '\n'
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
            [self](boost::system::error_code ec, [[maybe_unused]] auto unused) { self->when_connected(ec); }
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
                        << '\n'
                        << n_reconnects_ << " / " << max_reconnects_
                        << " failed connection attempts" << '\n'
                        << "Got error code \"" << ec.message() << "\"." << '\n'
                        << "The client will terminate now." << '\n'
                        << GLOGGING;

                // Terminate operation and return
                this->shutdown();
            }
            else { // Try to reconnect after issuing a warning
                glogger << "In GAsioConsumerClientT<processable_type>::when_connected():"
                        << '\n'
                        << n_reconnects_ << " / " << max_reconnects_
                        << " failed connection attempts" << '\n'
                        << "Got error code \"" << ec.message() << "\"." << '\n'
                        << "We will try to reconnect" << '\n'
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

        // Disable Nagle's algorithm (TCP_NODELAY). Geneva's wire protocol is a strict, small
        // request/response exchange (GETDATA/COMPUTE/RESULT). With Nagle enabled, a small segment
        // is held back until the previous one is ACKed; combined with the peer's delayed ACKs this
        // can add tens of milliseconds of latency to every round-trip for no throughput benefit
        // (there is no second small write to coalesce with). We therefore disable it. Best-effort:
        // a failure to set the option is not fatal.
        boost::system::error_code nd_ec;
        socket_ptr_->set_option(boost::asio::ip::tcp::no_delay(true), nd_ec);

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
        [[maybe_unused]] std::size_t nothing
    ) {
        if(ec) {
            glogger << "In GAsioConsumerClientT<processable_type>::when_written():" << '\n'
                    << "Got ec(\"" << ec.message()
                    << "\"). async_start_read() will not be executed." << '\n'
                    << "This will terminate the client." << '\n'
                    << GLOGGING;

            // Terminate operation and return
            this->shutdown();
            return;
        }

        // Shutdown the socket in send direction. This will result in an ec of boost::asio::error::eof
        // on the server side indicating that all data was written. Use the non-throwing overload:
        // the peer may already have closed, and a throw out of this completion handler would
        // unwind the io thread.
        boost::system::error_code sd_ec;
        socket_ptr_->shutdown(boost::asio::socket_base::shutdown_send, sd_ec);

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
        [[maybe_unused]] std::size_t nothing
    ) {
        if(ec == boost::asio::error::eof) { // The expected outcome
            // Disconnect from the remote side by destroying the socket
            socket_ptr_.reset();

            // Deal with the message and send a response back
            async_process_request();
        }
        else {
            if(ec) {
                glogger << "GAsioConsumerClientT<processable_type>::when_read(): " << '\n'
                        << "Leaving due to error code " << ec.message() << '\n'
                        << GLOGGING;
            }
            else {
                glogger << "GAsioConsumerClientT<processable_type>::when_read(): " << '\n'
                        << "No error code was received. Expected boost::asio::error::eof"
                        << '\n'
                        << "to indicate end of transmission." << '\n'
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
        // Extract the string from the buffer and de-serialize the object. A malformed or
        // truncated message makes this throw; that must not escape into io_context::run()
        // (it would unwind the client's only io thread and silence the client for good).
        try {
            Gem::Courtier::container_from_string(
                incoming_message_str_,
                command_container_,
                serialization_mode_
            );
        }
        catch(const std::exception &e) {
            glogger << "In GAsioConsumerClientT<processable_type>::async_process_request():" << '\n'
                    << "Could not de-serialize an incoming message:" << '\n'
                    << e.what() << '\n'
                    << "The client will shut down." << '\n'
                    << GWARNING;
            this->shutdown();
            return;
        }

        // Clear the buffer, so we may later fill it with data to be sent
        incoming_message_str_.clear();

        // Extract the command
        auto inboundCommand = command_container_.get_command();

        // Act on the command received
        switch(inboundCommand) {
            using enum Gem::Courtier::networked_consumer_payload_command;
        case COMPUTE: {
            // Process the work item. A failure in the user's processing code surfaces as a
            // g_processing_exception, with the work item already flagged (EXCEPTION_CAUGHT)
            // and carrying its error description. We must NOT let that kill the client:
            // catch it and return the flagged item to the server like any other result, so
            // the item is accounted for rather than lost.
            try {
                command_container_.process();
            }
            catch(const g_processing_exception &e) {
                glogger << "In GAsioConsumerClientT<processable_type>::async_process_request():"
                        << '\n'
                        << "The work item flagged a processing exception:" << '\n'
                        << e.what() << '\n'
                        << "It is returned to the server flagged; the client keeps running."
                        << '\n'
                        << GWARNING;
            }

            // Update the processed counter
            this->incrementProcessingCounter();

            // ... and set the command for the way back to the server
            command_container_.set_command(networked_consumer_payload_command::RESULT);
        } break;

        case NODATA: { // This must be a command payload
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
            // An unknown/invalid command is unrecoverable for this client; log and shut down
            // cleanly (do NOT throw -- that would unwind the io thread).
            glogger << "In GAsioConsumerClientT<processable_type>::async_process_request():" << '\n'
                    << "Got unknown or invalid command " << inboundCommand << '\n'
                    << "The client will shut down." << '\n'
                    << GWARNING;
            this->shutdown();
            return;
        }
        }

        // Transfer the command container into the outgoing message string. Guard the
        // serialization too: a failure here must not escape the io thread.
        try {
            outgoing_message_str_ =
                Gem::Courtier::container_to_string(command_container_, serialization_mode_);
        }
        catch(const std::exception &e) {
            glogger << "In GAsioConsumerClientT<processable_type>::async_process_request():" << '\n'
                    << "Could not serialize the outgoing message:" << '\n'
                    << e.what() << '\n'
                    << "The client will shut down." << '\n'
                    << GWARNING;
            this->shutdown();
            return;
        }

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
	  */
    GAsioConsumerSessionT(
        boost::asio::io_context &io_context,
        boost::asio::ip::tcp::socket socket,
        std::function<std::shared_ptr<processable_type>()> get_payload_item,
        std::function<void(std::shared_ptr<processable_type>)> put_payload_item,
        std::function<bool()> check_server_stopped,
        Gem::Common::serializationMode serialization_mode,
        std::function<void(bool)> sign_on
    )
      : socket_(std::move(socket))
      , strand_(io_context.get_executor())
      , deadline_timer_(io_context)
      , get_payload_item_(std::move(get_payload_item))
      , put_payload_item_(std::move(put_payload_item))
      , check_server_stopped_(std::move(check_server_stopped))
      , f_sign_on_(std::move(sign_on))
      , serialization_mode_(serialization_mode) {
        // Announce that this session has become active (RAII-balanced with the destructor),
        // so the consumer can report the number of concurrently active sessions.
        if(f_sign_on_) {
            f_sign_on_(true);
        }
    }

    //-------------------------------------------------------------------------
    /**
	  * The destructor signs the session off again, so the active-session count stays accurate
	  * however the session ends (served request, error or disconnect).
	  */
    ~GAsioConsumerSessionT() {
        if(f_sign_on_) {
            f_sign_on_(false);
        }
    }

    //-------------------------------------------------------------------------
    /**
	  * Starts the read-write cycle as the main purpose of this class
	  */
    void async_start_run() {
        // Disable Nagle's algorithm on the accepted connection: the request/response messages
        // are small and latency-sensitive.
        boost::system::error_code nd_ec;
        socket_.set_option(boost::asio::ip::tcp::no_delay(true), nd_ec);

        // Arm the deadline so a client that connects but never completes its request cannot pin the
        // socket/fd forever, then initiate the read session -- we expect an incoming message.
        arm_deadline();
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
    /** @brief Arms the per-session deadline timer. On expiry the socket is closed, which aborts the
	 *  outstanding read/write so the session (and its file descriptor) is released. */
    void arm_deadline() {
        deadline_timer_.expires_after(session_timeout_);
        auto self = this->shared_from_this();
        deadline_timer_.async_wait(
            boost::asio::bind_executor(strand_, [self](boost::system::error_code ec) {
                self->on_deadline(ec);
            })
        );
    }

    //-------------------------------------------------------------------------
    /** @brief Deadline-timer callback: closes a stalled connection so its socket/fd is reclaimed. */
    void on_deadline(boost::system::error_code ec) {
        if(ec) { // the timer was cancelled because the session completed normally
            return;
        }
        if(not check_server_stopped_()) {
            glogger << "GAsioConsumerSessionT<processable_type>::on_deadline(): " << '\n'
                    << "Connection exceeded the per-session timeout; closing it to reclaim the socket."
                    << '\n'
                    << GLOGGING;
        }
        boost::system::error_code ignore;
        socket_.close(ignore); // aborts the outstanding read/write -> the session ends
    }

    //-------------------------------------------------------------------------
    /**
	  * Starts an asynchronous read session, whose termination is signalled by
	  * a call to the when_read()-function
	  */
    void async_start_read() {
        if(check_server_stopped_()) {
            return;
        }

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
        [[maybe_unused]] std::size_t nothing
    ) {
        if(ec ==
           boost::asio::error::
               eof) { // The expected outcome, when the client has shut down its socket in send direction
            // Deal with the message and send a response back
            async_start_write(process_request());
        }
        else {
            // operation_aborted is the expected result of on_deadline() closing a stalled socket --
            // not an error worth logging.
            if(ec && ec != boost::asio::error::operation_aborted) {
                glogger << "GAsioConsumerSessionT<processable_type>::when_read(): " << '\n'
                        << "Leaving due to error code " << ec.message() << '\n'
                        << "Server session will terminate" << '\n'
                        << GLOGGING;
            }
            else if(not ec) {
                glogger << "GAsioConsumerSessionT<processable_type>::when_read(): " << '\n'
                        << "No ec received but expected boost::asio::error::eof" << '\n'
                        << "Server session will terminate" << '\n'
                        << GLOGGING;
            }
            // The session ends here (no write follows); cancel the deadline so it does not keep the
            // session object alive until expiry.
            deadline_timer_.cancel();
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
        [[maybe_unused]] std::size_t nothing
    ) {
        // operation_aborted is the expected result of on_deadline() closing a stalled socket.
        if(ec && ec != boost::asio::error::operation_aborted) {
            glogger << "GAsioConsumerSessionT<processable_type>::when_written(): " << '\n'
                    << "Got error code " << ec.message() << '\n'
                    << GLOGGING;
        }

        // The exchange is complete: cancel the deadline so it does not keep the session alive.
        deadline_timer_.cancel();

        // Shutdown the socket in send direction. This will result in an ec of boost::asio::error::eof
        // on the client-side indicating that all data was written. Non-throwing overload: a throw
        // out of this completion handler would unwind the io thread.
        boost::system::error_code sd_ec;
        socket_.shutdown(boost::asio::socket_base::shutdown_send, sd_ec);

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
                using enum Gem::Courtier::networked_consumer_payload_command;
            case GETDATA: {
                return getAndSerializeWorkItem();
            } break;

            case RESULT: {
                // Retrieve the payload from the command container
                auto payload_ptr = command_container_.get_payload();

                // Submit the payload to the server (which will send it to the broker)
                if(payload_ptr) {
                    this->put_payload_item_(payload_ptr);
                }
                else {
                    glogger << "GAsioConsumerSessionT<processable_type>::process_request():"
                            << '\n'
                            << "payload is empty even though a result was expected" << '\n'
                            << GWARNING;
                }

                // Retrieve the next work item and send it to the client for processing
                return getAndSerializeWorkItem();
            } break;

            default: {
                glogger << "GAsioConsumerSessionT<processable_type>::process_request():"
                        << '\n'
                        << "Got unknown or invalid command "
                        << inboundCommand << '\n'
                        << GWARNING;
            } break;
            }
        }
        catch(
            ...
        ) { // NOLINT(bugprone-empty-catch) — intentionally swallowed; session ends, caller retries
            glogger
                << "GAsioConsumerSessionT<processable_type>::process_request(): Caught exception"
                << '\n'
                << GLOGGING;
        }

        // Make the compiler happy
        return {};
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

    /// Bounds the lifetime of a single request/response exchange. Because this is a
    /// one-connection-per-request transport and the client sends its request promptly on connect
    /// (it closes the connection BEFORE evaluating a work item, so this never overlaps computation),
    /// a connection still open after this long is a stalled/half-open/dead client and is closed --
    /// otherwise its socket+fd would be pinned forever by the never-completing read, eventually
    /// exhausting the server's file descriptors.
    boost::asio::steady_timer deadline_timer_;
    const std::chrono::seconds session_timeout_{300};

    std::function<std::shared_ptr<processable_type>()> get_payload_item_;
    std::function<void(std::shared_ptr<processable_type>)> put_payload_item_;
    std::function<bool()> check_server_stopped_;
    std::function<void(bool)> f_sign_on_; ///< Signs the session on (true) / off (false) with the consumer

    Gem::Common::serializationMode serialization_mode_ = Gem::Common::serializationMode::BINARY;

    GCommandContainerT<processable_type, networked_consumer_payload_command> command_container_{
        networked_consumer_payload_command::NONE
    }; ///< Holds the current command and payload (if any)

    //-------------------------------------------------------------------------
};


/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Gem::Courtier::Consumers */
