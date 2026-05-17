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
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/rfc6455.hpp>
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
class GWebsocketClientT final
  : public Gem::Courtier::GBaseClientT<processable_type>
  , public std::enable_shared_from_this<GWebsocketClientT<processable_type>> {
    //-------------------------------------------------------------------------
    // Make the code easier to read

    using error_code = boost::system::error_code;
    using resolver = boost::asio::ip::tcp::resolver;
    using socket = boost::asio::ip::tcp::socket;
    using close_code = boost::beast::websocket::close_code;
    using frame_type = boost::beast::websocket::frame_type;
    using string_view = boost::beast::string_view;

public:
    //-------------------------------------------------------------------------
    /**
	  * Initialization with host/ip and port
	  */
    GWebsocketClientT(
        std::string address,
        unsigned short port,
        Gem::Common::serializationMode serialization_mode,
        bool verbose_control_frames
    )
      : resolver_(io_context_)
      , ws_(io_context_)
      , address_(std::move(address))
      , port_(port)
      , serialization_mode_(serialization_mode)
      , verbose_control_frames_(verbose_control_frames) {
        // Set the auto_fragment option, so control frames are delivered timely
        ws_.auto_fragment(true);
        ws_.write_buffer_bytes(16384);

        // Set the transfer mode
        switch(serialization_mode_) {
        case Gem::Common::serializationMode::BINARY:
            ws_.binary(true);
            break;
        case Gem::Common::serializationMode::XML:
        case Gem::Common::serializationMode::TEXT:
            ws_.binary(false);
            break;
        }

        // Set a control-frame callback
        f_when_control_frame_arrived = [this](frame_type frame_t, string_view s) {
            // Let the audience know what type of control frame we have received
            // if the user has requested it.
            if(this->verbose_control_frames_) {
                if(boost::beast::websocket::frame_type::close == frame_t) {
                    glogger << "GWebsocketClientT<> client has received a close frame" << '\n'
                            << GLOGGING;
                }
                else if(boost::beast::websocket::frame_type::ping == frame_t) {
                    glogger << "GWebsocketClientT<> client has received a ping frame" << '\n'
                            << GLOGGING;
                }
                else if(boost::beast::websocket::frame_type::pong == frame_t) {
                    glogger << "GWebsocketClientT<> client has received a pong frame" << '\n'
                            << GLOGGING;
                }
            }
        };

        // Set the callback to be executed on every incoming control frame.
        ws_.control_callback(f_when_control_frame_arrived);
    }

    //-------------------------------------------------------------------------
    /**
	  * The destructor
	  */
    ~GWebsocketClientT() override {
        glogger << '\n'
                << "GWebsocketClientT<> is shutting down. Processed " << this->getNProcessed()
                << " items in total" << '\n'
                << "\"no data\" was received " << n_nodata_ << " times" << '\n'
                << '\n'
                << GLOGGING;
    }

    //-------------------------------------------------------------------------
    // Deleted functions

    // Deleted default-constructor -- enforce usage of a particular constructor
    GWebsocketClientT() = delete;
    // Deleted copy-constructors and assignment operators -- the client is non-copyable
    GWebsocketClientT(const GWebsocketClientT<processable_type> &) = delete;
    GWebsocketClientT(GWebsocketClientT<processable_type> &&) = delete;
    GWebsocketClientT<processable_type> &
    operator=(const GWebsocketClientT<processable_type> &) = delete;
    GWebsocketClientT<processable_type> &operator=(GWebsocketClientT<processable_type> &&) = delete;

private:
    //-------------------------------------------------------------------------
    /**
	  * Starts the main run-loop
	  */
    void run_() override {
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

        // This call will block until no more work remains in the ASIO work queue
        io_context_.run();

        // Finally close all outstanding connections
        do_close(close_code_);

        // Let the audience know that we have finished the shutdown
        glogger << "GWebsocketClientT<processable_type>::run_(): Client session has terminated"
                << '\n'
                << GLOGGING;
    }

    //-------------------------------------------------------------------------
    /**
	  * Starts a new write session
	  *
	  * @param message The message to be transferred to the peer
	  */
    void async_start_write(const std::string &message) {
        // Do nothing if we have been asked to stop
        if(this->halt()) {
            return;
        }

        // We need to persist the message for asynchronous operations.
        // It is hence stored in a class variable.
        outgoing_message_ = message;

        // Send the message
        auto self = this->shared_from_this();
        ws_.async_write(
            boost::asio::buffer(outgoing_message_),
            [self](boost::system::error_code ec, std::size_t nBytesTransferred) {
                self->when_written(ec, nBytesTransferred);
            }
        );
    }

    //-------------------------------------------------------------------------
    /**
	  * Starts a new read session
	  */
    void async_start_read() {
        // Do nothing if we have been asked to stop
        if(this->halt()) {
            return;
        }

        auto self = this->shared_from_this();
        ws_.async_read(
            incoming_buffer_,
            [self](boost::system::error_code ec, std::size_t nBytesTransferred) {
                self->when_read(ec, nBytesTransferred);
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
            glogger << "In GWebsocketClientT<processable_type>::when_resolved():" << '\n'
                    << "Got ec(\"" << ec.message() << "\"). async_connect() will not be executed."
                    << '\n'
                    << "This will terminate the client." << '\n'
                    << GLOGGING;

            // Give the audience a hint why we are terminating
            close_code_ = boost::beast::websocket::close_code::going_away;

            return;
        }

        // Make the ASIO connection on the endpoint we get from a lookup
        auto self = this->shared_from_this();
        boost::asio::async_connect(
            ws_.next_layer(),
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
            glogger << "In GWebsocketClientT<processable_type>::when_connected():" << '\n'
                    << "Got ec(\"" << ec.message() << "\"). async_handshake() will not be executed."
                    << '\n'
                    << "This will terminate the client." << '\n'
                    << GLOGGING;

            // Give the audience a hint why we are terminating
            close_code_ = boost::beast::websocket::close_code::going_away;

            return;
        }

        // Perform the handshake
        auto self = this->shared_from_this();
        ws_.async_handshake(address_, "/", [self](boost::system::error_code ec) {
            self->when_handshake_complete(ec);
        });
    }

    //-------------------------------------------------------------------------
    /**
	  * Code to be executed when the async_handshake-operation has completed
	  *
	  * @param ec Indicates a possible error
	  */

    void when_handshake_complete(boost::system::error_code ec) {
        if(ec) {
            glogger << "In GWebsocketClientT<processable_type>::when_handshake_complete():"
                    << '\n'
                    << "Got ec(\"" << ec.message()
                    << "\"). async_start_write() will not be executed." << '\n'
                    << "This will terminate the client." << '\n'
                    << GLOGGING;

            // Give the audience a hint why we are terminating
            close_code_ = boost::beast::websocket::close_code::going_away;

            // This will terminate the client
            return;
        }

        // Send the first command to the server
        async_start_write(
            Gem::Courtier::container_to_string(
                command_container_.reset(networked_consumer_payload_command::GETDATA),
                serialization_mode_
            )
        );

        // Start the read cycle -- it will keep itself alife
        async_start_read();
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
            glogger << "In GWebsocketClientT<processable_type>::when_written():" << '\n'
                    << "Got ec(\"" << ec.message() << "\")." << '\n'
                    << "This will terminate the client." << '\n'
                    << GLOGGING;

            // Give the audience a hint why we are terminating
            close_code_ = boost::beast::websocket::close_code::going_away;

            // This will terminate the client
            // TODO: It is not quite true that this will terminate the client
            return;
        }

        // Clear the outgoing message -- no longer needed
        outgoing_message_.clear();
    }

    //-------------------------------------------------------------------------
    /**
	  * Code to be executed when an entire new message was read
	  *
	  * @param ec Indicates a possible error
	  */
    void when_read(
        boost::system::error_code ec,
        std::size_t /* nothing */
    ) {
        if(ec) {
            glogger << "In GWebsocketClientT<processable_type>::when_read():" << '\n'
                    << "Got ec(\"" << ec.message()
                    << "\"). async_start_write() will not be executed." << '\n'
                    << "This will terminate the client." << '\n'
                    << GLOGGING;

            // Give the audience a hint why we are terminating
            close_code_ = boost::beast::websocket::close_code::going_away;

            // This will terminate the client
            return;
        }

        // There should be no situation where in this location processing is active
        if(processing_is_active_) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, time_and_place)
                << "In GWebsocketClientT<processable_type>::when_read():" << '\n'
                << "process_reques() is running in a location where it shouldn't be" << '\n'
            );
        }

        // Deal with the message and send a response back. Processing
        // of work items is done inside of process_request().
        try {
            // Start asynchronous processing of the work item.
            auto self = this->shared_from_this();
            gtp_.async_schedule([self]() { self->process_request(); });

            async_start_read();
        }
        catch(...) {
            // Give the audience a hint why we are terminating
            glogger << "In GWebsocketClientT<processable_type>::when_read():" << '\n'
                    << "Caught exception" << '\n'
                    << GWARNING;

            close_code_ = boost::beast::websocket::close_code::internal_error;
        }
    }

    //-------------------------------------------------------------------------
    /**
	  * Processing of incoming messages and creation of responses takes place here
	  */
    void process_request() {
        // Make it known that we are processing a new work item
        processing_is_active_ = true;

        // Extract the string from the buffer
        auto message = boost::beast::buffers_to_string(incoming_buffer_.data());

        // Clear the buffer, so we may later fill it with data to be sent
        incoming_buffer_.consume(incoming_buffer_.size());

        // De-serialize the object
        Gem::Courtier::container_from_string(
            message,
            command_container_,
            serialization_mode_
        ); // may throw

        // Extract the command
        auto inboundCommand = command_container_.get_command();

        // Act on the command received
        switch(inboundCommand) {
        case networked_consumer_payload_command::COMPUTE: {
            // Process the work item
            command_container_.process();

            // Update the processed counter
            this->incrementProcessingCounter();

            // Set the command for the way back to the server
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
            throw geneva_exception(
                g_error_streamer(DO_LOG, time_and_place)
                << "GWebsocketClientT<processable_type><>::process_request():" << '\n'
                << "Received invalid command " << pcToStr(inboundCommand) << '\n'
            );
        } /* break; */ // break is unreachable
        }

        // Processing has finished
        processing_is_active_ = false;

        // Serialize the object again and return the result
        this->async_start_write(
            Gem::Courtier::container_to_string(command_container_, serialization_mode_)
        );
    }

    //-------------------------------------------------------------------------
    /**
	  * Closes the connection to the peer
	  *
	  * @param cc The close code to be sent to the peer
	  */
    void do_close(close_code cc) {
        if(ws_.is_open()) {
            ws_.close(cc);
        }

        if(ws_.next_layer().is_open()) {
            boost::system::error_code ec;

            ws_.next_layer().shutdown(socket::shutdown_both, ec);
            ws_.next_layer().close(ec);

            if(ec) {
                glogger << "In GWebsocketClientT<processable_type>::do_close():" << '\n'
                        << "Got ec(\"" << ec.message() << "\")." << '\n'
                        << "We will throw an exception, as there are no other options left"
                        << '\n'
                        << GLOGGING;

                // Not much more we can do
                throw geneva_exception(
                    g_error_streamer(DO_LOG, time_and_place)
                    << "GWebsocketClientT<processable_type>::do_close():" << '\n'
                    << "Shutdown of next layer has failed" << '\n'
                );
            }
        }
    }

    //-------------------------------------------------------------------------

    /** @brief Callback for control frames */
    std::function<void(boost::beast::websocket::frame_type, boost::beast::string_view)>
        f_when_control_frame_arrived;

    //-------------------------------------------------------------------------
    // Data

    boost::asio::io_context
        io_context_; ///< The io-service object handling the asynchronous processing
    resolver resolver_{io_context_}; ///< Helps to resolve the peer
    boost::beast::websocket::stream<socket> ws_{
        io_context_
    }; ///< All messages are sent and received through this socket

    std::string address_; ///< The ip address or name of the peer system
    unsigned int port_;   ///< The peer port

    boost::beast::multi_buffer incoming_buffer_;
    std::string outgoing_message_; ///< Helps to persist outgoing messages

    std::random_device nondet_rng_; ///< Source of non-deterministic random numbers
    std::mt19937 rng_engine_{
        nondet_rng_()
    }; ///< The actual random number engine, seeded my nondet_rng_

    boost::beast::websocket::close_code close_code_ = boost::beast::websocket::close_code::
        normal; ///< Holds the close code when terminating the connection

    Gem::Common::serializationMode serialization_mode_ = Gem::Common::serializationMode::
        BINARY; ///< Determines which seriliztion mode should be used
    bool verbose_control_frames_ =
        false; ///< Whether a diagnostic message should be emitted when a control frame arrives

    std::uint64_t n_nodata_ = 0;

    GCommandContainerT<processable_type, networked_consumer_payload_command> command_container_{
        networked_consumer_payload_command::NONE
    }; ///< Holds the current command and payload (if any)

    Gem::Common::GThreadPool gtp_{
        1
    }; ///< Holds workers doing the processing and serialization of incoming workloads

    std::atomic<bool> processing_is_active_{
        false
    }; ///< A safeguard against accidental processing of two work items

    //-------------------------------------------------------------------------
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * Consumer-side handling of client-connection. A new session is started for each
 * new connection and will be kept open until a stop condition is reached.
 */
template <typename processable_type>
class GWebsocketConsumerSessionT final
  : public std::enable_shared_from_this<GWebsocketConsumerSessionT<processable_type>> {
    //-------------------------------------------------------------------------
    // Make the code easier to read
    using error_code = boost::system::error_code;
    using frame_type = boost::beast::websocket::frame_type;
    using string_view = boost::beast::string_view;

public:
    //-------------------------------------------------------------------------
    /**
	  * The only allowed constructor for this class
	  *
	  * @param socket All communication goes through this socket
	  * @param get_payload_item Callback for the retrieval of payload items
	  * @param put_payload_item Callback for the submission of payload items
	  * @param check_server_stopped Callback used to check whether a halt was requested by the server
	  * @param server_sign_on Callback to inform the server that a new session is active or has retired
	  * @param serialization_mode Informs the session which Boost.Serialization mode should be used
	  * @param ping_interval The interval between two consecutive pings
	  * @param verbose_control_frames Whether the session should emit diagnostic messages upon receipt of a control frame
	  */
    GWebsocketConsumerSessionT(
        boost::asio::io_context &io_context,
        boost::asio::ip::tcp::socket socket,
        std::function<std::shared_ptr<processable_type>()> get_payload_item,
        std::function<void(std::shared_ptr<processable_type>)> put_payload_item,
        std::function<bool()> check_server_stopped,
        std::function<void(bool)> server_sign_on,
        Gem::Common::serializationMode serialization_mode,
        std::size_t ping_interval,
        bool verbose_control_frames
    )
      : ws_(std::move(socket))
      , strand_(io_context.get_executor())
      , timer_(io_context, (std::chrono::steady_clock::time_point::max)())
      , get_payload_item_(std::move(get_payload_item))
      , put_payload_item_(std::move(put_payload_item))
      , check_server_stopped_(std::move(check_server_stopped))
      , server_sign_on_(std::move(server_sign_on))
      , serialization_mode_(serialization_mode)
      , ping_interval_(std::chrono::seconds(ping_interval))
      , verbose_control_frames_(verbose_control_frames) {
        // ---------------------------------------------------
        // Make it known to the server that a new session has started
        this->server_sign_on_(true);

        // ---------------------------------------------------
        // Prepare ping cycle. It must start after the handshake, upon whose
        // completion the when_connection_accepted() function is called.
        // async_start_ping() is executed from there.

        // Set a control-frame callback
        f_when_control_frame_arrived = [this](frame_type frame_t, string_view s) {
            if(
				 // We might have received a pong as an answer to our own ping,
				 // or someone might be sending us pings. In either case the conection is alive.
				 boost::beast::websocket::frame_type::pong==frame_t
				 || boost::beast::websocket::frame_type::ping==frame_t
				 ) {
                // Note that the connection is alive
                this->ping_state_ = beast_ping_state::CONNECTION_IS_ALIVE;
            }

            // Let the audience know what type of control frame we have received
            // if the user has requested it.
            if(this->verbose_control_frames_) {
                if(boost::beast::websocket::frame_type::close == frame_t) {
                    glogger << "GWebsocketConsumerSessionT<> session has received a close frame"
                            << '\n'
                            << GLOGGING;
                }
                else if(boost::beast::websocket::frame_type::ping == frame_t) {
                    glogger << "GWebsocketConsumerSessionT<> session has received a ping frame"
                            << '\n'
                            << GLOGGING;
                }
                else if(boost::beast::websocket::frame_type::pong == frame_t) {
                    glogger << "GWebsocketConsumerSessionT<> session has received a pong frame"
                            << '\n'
                            << GLOGGING;
                }
            }
        };

        // Set the callback to be executed on every incoming control frame.
        ws_.control_callback(f_when_control_frame_arrived);

        // ---------------------------------------------------
        // Set the auto_fragment option, so control frames are delivered timely
        ws_.auto_fragment(true);
        ws_.write_buffer_bytes(16384);

        // ---------------------------------------------------
        // Set the transfer mode according to the defines in CMakeLists.txt
        // Set the transfer mode
        switch(serialization_mode_) {
        case Gem::Common::serializationMode::BINARY:
            ws_.binary(true);
            break;
        case Gem::Common::serializationMode::XML:
        case Gem::Common::serializationMode::TEXT:
            ws_.binary(false);
            break;
        }

        // ---------------------------------------------------
    }

    //-------------------------------------------------------------------------
    /** @brief The destructor */
    ~GWebsocketConsumerSessionT() {
        // Make it known to the server that this session has terminated
        this->server_sign_on_(false);
    }

    //-------------------------------------------------------------------------
    /**
	  * Initiates all communication and processing
	  */
    void async_start_run() {
        // ---------------------------------------------------
        // Connections and communication

        // Wait for a new websocket connection. Note that the
        // ASIO connection should already be active at this place.
        async_start_accept();

        // ---------------------------------------------------
        // This function will terminate shortly after it was called,
        // as all operations are performed asynchronously.
    }

    //-------------------------------------------------------------------------
    // Deleted functions
    GWebsocketConsumerSessionT() = delete;
    GWebsocketConsumerSessionT(const GWebsocketConsumerSessionT<processable_type> &) = delete;
    GWebsocketConsumerSessionT(GWebsocketConsumerSessionT<processable_type> &&) = delete;
    GWebsocketConsumerSessionT<processable_type> &
    operator=(const GWebsocketConsumerSessionT<processable_type> &) = delete;
    GWebsocketConsumerSessionT<processable_type> &
    operator=(GWebsocketConsumerSessionT<processable_type> &&) = delete;

private:
    //-------------------------------------------------------------------------
    /**
	  * Initiates a new asynchroneous read session
	  */
    void async_start_read() {
        // Read a message into our buffer
        auto self = this->shared_from_this();
        ws_.async_read(
            incoming_buffer_,
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
	  * Initiates a new asynchroneous write session
	  *
	  * @param message The message to be sent to the peer
	  */
    void async_start_write(std::string message) {
        // We need to persist the message for asynchronous operations
        outgoing_message_ = std::move(message);

        // Return an answer
        auto self = this->shared_from_this();
        ws_.async_write(
            boost::asio::buffer(outgoing_message_),
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
	  * Initiates pinging of the peer, so the connection may be kept alive
	  */
    void async_start_ping() {
        // Set the timer
        timer_.expires_after(ping_interval_);

        // Start to wait asynchronously. This call will return immediately.
        // when_timer_fired() will be called once the timer has expired.
        auto self = this->shared_from_this();
        timer_.async_wait(
            boost::asio::bind_executor(strand_, [self](boost::system::error_code ec) {
                self->when_timer_fired(ec);
            })
        );

        // Setting the ping state must be done before the ping is sent, or
        // else the pong might arrive before the SENDING_PING state is set
        // and we might overwrite the CONNECTION_IS_ALIVE state set by the
        // control-frame callback
        ping_state_ = beast_ping_state::SENDING_PING;

        // Start the ping session
        ws_.async_ping(
            ping_data_,
            boost::asio::bind_executor(strand_, [self](boost::system::error_code ec) {
                self->when_ping_sent(ec);
            })
        );
    }

    //-------------------------------------------------------------------------
    /**
	  * Starts waiting for websocket handshakes over an already established
	  * ASIO connection.
	  */
    void async_start_accept() {
        // This function initiates an asynchronous chain of callbacks, where each callback is
        // executed when the previous call (here: async_accept) is completed. Error handling is
        // done in the callback, using an error code provided by Boost.Beast and/or Boost.ASIO.
        auto self = this->shared_from_this();
        ws_.async_accept(
            boost::asio::bind_executor(strand_, [self](boost::system::error_code ec) {
                self->when_connection_accepted(ec);
            })
        );
    }

    //-------------------------------------------------------------------------
    /**
	  * Reacts on errors when sending a ping
	  *
	  * @param ec The error code of a potential error
	  */
    void when_ping_sent(boost::system::error_code ec) {
        if(ec) {
            if(ec != boost::asio::error::operation_aborted) {
                glogger << "GWebsocketConsumerSessionT<processable_type>::when_ping_sent(): "
                        << ec.message() << '\n'
                        << GLOGGING;
            }

            ping_state_ = beast_ping_state::CONNECTION_IS_STALE;
        }
    }

    //-------------------------------------------------------------------------
    /**
	  * A timer is started in regular intervals. This code is executed when
	  * the timer expires
	  *
	  * @param ec The error code of a potential error
	  */
    void when_timer_fired(boost::system::error_code ec) {
        if(ec) {
            if(ec != boost::asio::error::operation_aborted) {
                glogger << "GWebsocketConsumerSessionT<processable_type>::when_timer_fired(): "
                        << ec.message() << '\n'
                        << GLOGGING;
            }

            ping_state_ = beast_ping_state::CONNECTION_IS_STALE;
            return;
        }

        if(ping_state_ == beast_ping_state::CONNECTION_IS_ALIVE) {
            // Start the next ping session, if this is a healthy connection
            async_start_ping();
            return;
        }
        else {
            ping_state_ = beast_ping_state::CONNECTION_IS_STALE;

            if(not this->check_server_stopped_()) {
                // Either this is a stale connection or the SENDING_PING flag is still set
                glogger << "GWebsocketConsumerSessionT<processable_type>::when_timer_fired():"
                        << '\n'
                        << "Connection seems to be dead: " << ping_state_ << '\n'
                        << GLOGGING;
            }
            return;
        }
    }

    //-------------------------------------------------------------------------
    /**
	  * Code to be executed when a handshake was accepted
	  *
	  * @param ec The error code of a potential error
	  */
    void when_connection_accepted(boost::system::error_code ec) {
        if(ec) {
            glogger << "GWebsocketConsumerSessionT<processable_type>::when_connection_accepted(): "
                    << ec.message() << '\n'
                    << GLOGGING;

            do_close(boost::beast::websocket::close_code::going_away);

            return;
        }

        // Start reading an incoming message. This
        // call will return immediately.
        async_start_read();

        // Start the ping cycle
        async_start_ping();
    }

    //-------------------------------------------------------------------------
    /**
	  * Code to be executed when a new message was read
	  *
	  * @param ec The error code of a potential error
	  */
    void when_read(
        boost::system::error_code ec,
        std::size_t /* nothing */
    ) {
        if(ec) {
            if(ec != boost::beast::websocket::error::closed) {
                glogger << "GWebsocketConsumerSessionT<processable_type>::when_read(): "
                        << ec.message() << '\n'
                        << GLOGGING;
            }

            do_close(boost::beast::websocket::close_code::going_away);
            return;
        }

        // Deal with the message and send a response back
        async_start_write(process_request());
    }

    //-------------------------------------------------------------------------
    /**
	  * Code to be executed when a message was written
	  *
	  * @param ec The error code of a potential error
	  */
    void when_written(
        boost::system::error_code ec,
        std::size_t /* nothing */
    ) {
        if(ec) {
            if(ec != boost::beast::websocket::error::closed) {
                glogger << "GWebsocketConsumerSessionT<processable_type>::when_written(): "
                        << ec.message() << '\n'
                        << GLOGGING;
            }

            do_close(boost::beast::websocket::close_code::going_away);
            return;
        }

        // Clear the outgoing message -- no longer needed
        outgoing_message_.clear();

        if(this->check_server_stopped_()) {
            // Do not continue if a stop criterion was reached
            do_close(boost::beast::websocket::close_code::normal);
        }
        else {
            // Start another read cycle
            async_start_read();
        }
    }

    //-------------------------------------------------------------------------
    /** @brief Callback for control frames */
    std::function<void(boost::beast::websocket::frame_type, boost::beast::string_view)>
        f_when_control_frame_arrived;

    //-------------------------------------------------------------------------
    /**
	  * Shuts down the (websocket and ASIO) connection to the peer
	  *
	  * @param cc A close-code to be transmitted to the peer
	  */
    void do_close(boost::beast::websocket::close_code cc) {
        // Store the close code for later reference
        close_code_ = cc;

        // Make sure no more pings are sent and the timer expires
        timer_.cancel();

        if(ws_.is_open()) {
            // Close the connection
            ws_.close(cc);
        }

        if(ws_.next_layer().is_open()) {
            boost::system::error_code ec;

            // Closing the socket cancels all outstanding operations. They
            // will complete with boost::asio::error::operation_aborted
            ws_.next_layer().shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
            ws_.next_layer().close(ec);

            if(ec) {
                // Not much else we can do here
                throw geneva_exception(
                    g_error_streamer(DO_LOG, time_and_place)
                    << "GWebsocketConsumerSessionT<processable_type>::do_close():" << '\n'
                    << "Shutdown of next layer has failed" << '\n'
                    << "Got error code " << ec.message() << '\n'
                );
            }
        }
    }

    //-------------------------------------------------------------------------
    /**
	  * Processing of incoming messages and creation of a response-string
	  *
	  * @return The response to be sent to the server in the case of an incoming message
	  */
    std::string process_request() {
        try {
            // Extract the string from the buffer
            auto message = boost::beast::buffers_to_string(incoming_buffer_.data());

            // De-serialize the object
            Gem::Courtier::container_from_string(
                message,
                command_container_,
                serialization_mode_
            ); // may throw

            // Clear the buffer, so we may later fill it with data to be sent
            incoming_buffer_.consume(incoming_buffer_.size());

            // Extract the command
            auto inboundCommand = command_container_.get_command();

            // Act on the command received
            switch(inboundCommand) {
            case networked_consumer_payload_command::GETDATA: {
                return getAndSerializeWorkItem();
            } /* break; */ // break is unreachable

            case networked_consumer_payload_command::RESULT: {
                // Retrieve the payload from the command container
                auto payload_ptr = command_container_.get_payload();

                // Submit the payload to the server (which will send it to the broker)
                if(payload_ptr) {
                    this->put_payload_item_(payload_ptr);
                }
                else {
                    glogger << "GWebsocketConsumerSessionT<processable_type>::process_request():"
                            << '\n'
                            << "payload is empty even though a result was expected" << '\n'
                            << GWARNING;
                }

                // Retrieve the next work item and send it to the client for processing
                return getAndSerializeWorkItem();
            } /* break; */ // break is unreachable

            default: {
                glogger << "GWebsocketConsumerSessionT<processable_type>::process_request():"
                        << '\n'
                        << "Got unknown or invalid command "
                        << inboundCommand << '\n'
                        << GWARNING;
            } break;
            }
        }
        catch(...) {
            glogger << "GWebsocketConsumerSessionT<processable_type>::process_request(): Caught "
                       "exception"
                    << '\n'
                    << GLOGGING;

            do_close(boost::beast::websocket::close_code::internal_error);
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

    boost::beast::websocket::stream<boost::asio::ip::tcp::socket> ws_;
    boost::asio::strand<boost::asio::io_context::executor_type> strand_;

    boost::beast::multi_buffer incoming_buffer_;
    std::string outgoing_message_;

    boost::asio::steady_timer timer_;

    std::function<std::shared_ptr<processable_type>()> get_payload_item_;
    std::function<void(std::shared_ptr<processable_type>)> put_payload_item_;
    std::function<bool()> check_server_stopped_;
    std::function<void(bool)> server_sign_on_;

    boost::beast::websocket::close_code close_code_ = boost::beast::websocket::close_code::
        normal; ///< Holds the close code when terminating the connection

    Gem::Common::serializationMode serialization_mode_ = Gem::Common::serializationMode::BINARY;

    const std::chrono::seconds ping_interval_{
        GBEASTCONSUMERPINGINTERVAL
    }; // Time between two pings in seconds
    bool verbose_control_frames_ = false;

    std::atomic<beast_ping_state> ping_state_{beast_ping_state::CONNECTION_IS_ALIVE};
    const boost::beast::websocket::ping_data ping_data_;

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
 * for each client connection, and to interact with the Broker. Once a session
 * was started, the connection is kept open, and communication happens via
 * Boost.Beast.
 */
template <typename processable_type>
class GWebsocketConsumerT // NOLINT(cppcoreguidelines-special-member-functions)
  : public Gem::Courtier::GBaseConsumerT<processable_type> // note: GBaseConsumerT<> is non-copyable
  , public std::enable_shared_from_this<GWebsocketConsumerT<processable_type>> {
    //-------------------------------------------------------------------------
    // Simplify usage of namespaces
    using error_code = boost::system::error_code;

public:
    //-------------------------------------------------------------------------
    /** @brief The default constructor */
    GWebsocketConsumerT() = default;

    //-------------------------------------------------------------------------
    // Deleted copy-/move-constructors and assignment operators.
    GWebsocketConsumerT(const GWebsocketConsumerT<processable_type> &) = delete;
    GWebsocketConsumerT(GWebsocketConsumerT<processable_type> &&) = delete;
    GWebsocketConsumerT &operator=(const GWebsocketConsumerT<processable_type> &) = delete;
    GWebsocketConsumerT &operator=(GWebsocketConsumerT<processable_type> &&) = delete;

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
        for(auto &t : io_context_thread_cnt_) {
            t.join();
        }
        io_context_thread_cnt_.clear();

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
            .add_options()("beast_ip", po::value<std::string>(&server_)->default_value(GCONSUMERDEFAULTSERVER), "\t[beast] The name or ip of the server")(
                "beast_port",
                po::value<unsigned short>(&port_)->default_value(GCONSUMERDEFAULTPORT),
                "\t[beast] The port of the server"
            );

        hidden.add_options()
			 ("beast_serializationMode", po::value<Gem::Common::serializationMode>(&serializationMode_)->default_value(GCONSUMERSERIALIZATIONMODE),
				 "\t[beast] Specifies whether serialization shall be done in TEXTMODE (0), XMLMODE (1) or BINARYMODE (2)")
			 ("beast_nListenerThreads", po::value<std::size_t>(&n_listener_threads_)->default_value(n_listener_threads_),
				 "\t[beast] The number of threads used to listen for incoming connections")
			 ("beast_pingInterval", po::value<std::size_t>(&ping_interval_)->default_value(GBEASTCONSUMERPINGINTERVAL),
				 "\t[beast] The number of seconds between two consecutive pings")
			 ("beast_verboseControlFrames", po::value<bool>(&verbose_control_frames_)->default_value(false)->implicit_value(true),
				 "\t[beast] Whether sending and arrival of ping/pong and receipt of a close frame should be announced by client and server");
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
        return std::string("GWebsocketConsumerT");
    }

    //-------------------------------------------------------------------------
    /**
	  * Returns a short identifier for this consumer
	  */
    std::string getMnemonic_() const override {
        return std::string("beast");
    }

    //-------------------------------------------------------------------------
    /**
	  * Starts the consumer responder loops
	  */
    void async_startProcessing_() override {
        boost::system::error_code ec;

        // Set up the endpoint according to the endpoint information we have received from the command line
        endpoint_ = boost::asio::ip::tcp::endpoint{boost::asio::ip::tcp::v4(), port_};

        // Open the acceptor
        acceptor_.open(endpoint_.protocol(), ec);
        if(ec || not acceptor_.is_open()) {
            if(ec) {
                throw geneva_exception(
                    g_error_streamer(DO_LOG, time_and_place)
                    << "GWebsocketConsumerT<>::async_startProcessing_() / acceptor_.open: Got "
                       "error message \""
                    << ec.message() << "\"" << '\n'
                    << "No connections will be accepted. The server is not running" << '\n'
                );
            }
            else {
                throw geneva_exception(
                    g_error_streamer(DO_LOG, time_and_place)
                    << "GWebsocketConsumerT<>::async_startProcessing_() / acceptor_.open did not "
                       "succeed."
                    << '\n'
                    << "No connections will be accepted. The server is not running" << '\n'
                );
            }
        }

        // Bind to the server address
        acceptor_.bind(endpoint_, ec);
        if(ec) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, time_and_place)
                << "GWebsocketConsumerT<>::async_startProcessing_() / acceptor_.bind: Got error "
                   "message \""
                << ec.message() << "\"" << '\n'
                << "No connections will be accepted. The server is not running" << '\n'
            );
        }

        // Some acceptor options
        boost::asio::socket_base::reuse_address option(true);
        acceptor_.set_option(option);

        // Start listening for connections  TODO: Check if this should be increased
        acceptor_.listen(boost::asio::socket_base::max_listen_connections, ec);
        if(ec) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, time_and_place)
                << "GWebsocketConsumerT<>::async_startProcessing_() / acceptor_.listen: Got error "
                   "message \""
                << ec.message() << "\"" << '\n'
                << "No connections will be accepted. The server is not running" << '\n'
            );
        }

        // Start accepting connections
        async_start_accept();

        // Allow to serve requests simultaneously from multiple threads
        io_context_thread_cnt_.reserve(n_listener_threads_);
        for(std::size_t t_cnt = 0; t_cnt < n_listener_threads_; t_cnt++) {
            io_context_thread_cnt_.emplace_back([this]() { this->io_context_.run(); });
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
            glogger << "In GWebsocketConsumerT<>::when_accepted(): Got error code \""
                    << ec.message() << "\"" << '\n'
                    << "We will nevertheless try to accept more connections" << '\n'
                    << GWARNING;
        }
        else {
            // Create the GWebsocketConsumerSessionT and run it. This call will return immediately.
            std::make_shared<GWebsocketConsumerSessionT<processable_type>>(
                io_context_,
                std::move(socket_) // socket_ will stay in a valid state
                ,
                [this]() -> std::shared_ptr<processable_type> { return this->getPayloadItem(); },
                [this](std::shared_ptr<processable_type> p) { this->putPayloadItem(p); },
                [this]() -> bool { return this->stopped(); },
                [this](bool sign_on) {
                    if(true == sign_on) {
                        this->n_active_sessions_++;
                    }
                    else {
                        if(this->n_active_sessions_ > 0) {
                            // This won't help, though, if n_active_sessions_ becomes 0 after the if-check
                            this->n_active_sessions_--;
                        }
                        else {
                            throw geneva_exception(
                                g_error_streamer(DO_LOG, time_and_place)
                                << "In GWebsocketConsumerT<>::when_accepted():" << '\n'
                                << "Tried to decrement #sessions which is already 0" << '\n'
                            );
                        }
                    }

                    glogger << "GWebsocketConsumerT<>: " << this->n_active_sessions_
                            << " active sessions" << '\n'
                            << GLOGGING;
                },
                serializationMode_,
                ping_interval_,
                verbose_control_frames_
            )
                ->async_start_run();
        }

        // Accept another connection
        if(not this->stopped()) {
            async_start_accept();
        }
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
                << "GWebsocketConsumerT<>::putPayloadItem():" << '\n'
                << "Function called with empty work item" << '\n'
            );
        }

        if(not broker_ptr_->put(p, timeout_)) {
            glogger << "In GWebsocketConsumerT<>::putPayloadItem():" << '\n'
                    << "Work item could not be submitted to the broker" << '\n'
                    << "The item will be discarded" << '\n'
                    << GWARNING;
        }
    }

    //-------------------------------------------------------------------------
    /**
	  * This function returns a client associated with this consumer. By default
	  * it returns an empty smart pointer, so that consumers without the need for
	  * clients do not need to re-implement this function.
	  */
    std::shared_ptr<GBaseClientT<processable_type>> getClient_() const override {
        return std::shared_ptr<GBaseClientT<processable_type>>(
            new GWebsocketClientT<processable_type>(
                server_,
                port_,
                serializationMode_,
                verbose_control_frames_
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
    std::size_t n_listener_threads_ =
        GCONSUMERLISTENERTHREADS; ///< The number of threads used to listen for incoming connections through io_context::run()
    boost::asio::io_context io_context_{Gem::Common::narrow_cast<int>(n_listener_threads_)};
    boost::asio::ip::tcp::acceptor acceptor_{io_context_};
    boost::asio::ip::tcp::socket socket_{io_context_};
    Gem::Common::serializationMode serializationMode_ =
        Gem::Common::serializationMode::BINARY; ///< Specifies the serialization mode
    std::vector<std::thread> io_context_thread_cnt_;
    std::atomic<std::size_t> n_active_sessions_{0};
    std::size_t ping_interval_ = GBEASTCONSUMERPINGINTERVAL;
    bool verbose_control_frames_ =
        false; ///< Whether the control_callback should emit information when a control frame is received

    std::shared_ptr<GBrokerT<processable_type>> broker_ptr_ =
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
