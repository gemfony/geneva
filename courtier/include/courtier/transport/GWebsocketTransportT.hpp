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
#include <boost/asio/executor_work_guard.hpp>
#include <boost/asio/post.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/thread_pool.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/rfc6455.hpp>
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
        f_when_control_frame_arrived_ = [this](frame_type frame_t, string_view s) {
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
        ws_.control_callback(f_when_control_frame_arrived_);
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
    void async_start_write(std::string message) {
        // Do nothing if we have been asked to stop
        if(this->halt()) {
            return;
        }

        // We need to persist the message for asynchronous operations. It is hence moved into a
        // class variable (all callers pass a freshly serialized rvalue, so this avoids a copy of
        // the potentially large payload).
        outgoing_message_ = std::move(message);

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
            glogger << "In GWebsocketClientT<processable_type>::when_connected():" << '\n'
                    << "Got ec(\"" << ec.message() << "\"). async_handshake() will not be executed."
                    << '\n'
                    << "This will terminate the client." << '\n'
                    << GLOGGING;

            // Give the audience a hint why we are terminating
            close_code_ = boost::beast::websocket::close_code::going_away;

            return;
        }

        // Disable Nagle's algorithm on the underlying TCP socket: the request/response messages
        // are small and latency-sensitive.
        boost::system::error_code nd_ec;
        ws_.next_layer().set_option(boost::asio::ip::tcp::no_delay(true), nd_ec);

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

        // Full-duplex operation: keep exactly one async_read outstanding at all times so Beast can
        // service incoming pings (-> automatic pong) even while a work item is being evaluated on the
        // compute pool. Without an in-flight read, a long (unbounded) evaluation would starve the pong
        // and the server would wrongly declare this healthy client dead. The periodic halt poll
        // cancels the outstanding read on shutdown so io_context::run() can drain.
        async_start_read();
        start_halt_timer();

        // Send the first request. Subsequent requests are written from handle_message() (NODATA) and
        // finish_compute() (RESULT); the read above is re-armed in when_read().
        async_start_write(
            Gem::Courtier::container_to_string(
                command_container_.reset(networked_consumer_payload_command::GETDATA),
                serialization_mode_
            )
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
            // operation_aborted is the expected outcome of do_close() during shutdown -- not an error.
            if(ec != boost::asio::error::operation_aborted) {
                glogger << "In GWebsocketClientT<processable_type>::when_written():" << '\n'
                        << "Got ec(\"" << ec.message() << "\")." << '\n'
                        << "This will terminate the client." << '\n'
                        << GLOGGING;
            }
            close_code_ = boost::beast::websocket::close_code::going_away;
            return;
        }

        // The request has been sent; the server's response is delivered to the always-outstanding
        // read (re-armed in when_read()), so we do NOT start a read here. Just release the buffer.
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
        [[maybe_unused]] std::size_t nothing
    ) {
        if(ec) {
            // operation_aborted is the expected outcome of do_close() during shutdown -- not an error.
            if(ec != boost::asio::error::operation_aborted) {
                glogger << "In GWebsocketClientT<processable_type>::when_read():" << '\n'
                        << "Got ec(\"" << ec.message() << "\")." << '\n'
                        << "This will terminate the client." << '\n'
                        << GLOGGING;
            }
            close_code_ = boost::beast::websocket::close_code::going_away;
            return;
        }

        // Handle the message: a COMPUTE item is moved to the compute pool (so the io thread stays free
        // to service pings while the -- possibly long -- evaluation runs); lighter commands (NODATA)
        // are answered directly. Then re-arm the read so exactly one read stays outstanding.
        handle_message();

        if(not this->halt()) {
            async_start_read();
        }
    }

    //-------------------------------------------------------------------------
    /**
	  * Processing of incoming messages and creation of responses takes place here
	  */
    void handle_message() {
        // Extract the string from the buffer
        auto message = boost::beast::buffers_to_string(incoming_buffer_.data());

        // Clear the buffer, so we may later fill it with data to be sent
        incoming_buffer_.consume(incoming_buffer_.size());

        // De-serialize the object. A malformed/truncated message makes this throw; that must
        // not escape into io_context::run() (it would unwind the client's only io thread).
        try {
            Gem::Courtier::container_from_string(message, command_container_, serialization_mode_);
        }
        catch(const std::exception &e) {
            glogger << "In GWebsocketClientT<processable_type>::handle_message():" << '\n'
                    << "Could not de-serialize an incoming message:" << '\n'
                    << e.what() << '\n'
                    << "The client will shut down." << '\n'
                    << GWARNING;
            close_code_ = boost::beast::websocket::close_code::internal_error;
            return;
        }

        // Act on the command received
        switch(command_container_.get_command()) {
            using enum Gem::Courtier::networked_consumer_payload_command;
        case COMPUTE:
            // Hand the (possibly long-running, unbounded) evaluation to the compute pool so the io
            // thread stays free to service pings -> auto-pong while it runs. The result is written
            // back from finish_compute() once the worker is done.
            dispatch_compute();
            break;

        case NODATA: {
            // No work available yet. Wait briefly, then ask again. A short sleep on the io thread is
            // acceptable here -- it is far below the ping interval and only happens while idle.
            n_nodata_++;
            std::uniform_int_distribution<> dist(50, 200);
            std::this_thread::sleep_for(std::chrono::milliseconds(dist(rng_engine_)));
            send_command_(command_container_.reset(networked_consumer_payload_command::GETDATA));
        } break;

        default:
            // An unknown/invalid command is unrecoverable; log and shut down cleanly (do NOT throw
            // -- that would unwind the io thread).
            glogger << "In GWebsocketClientT<processable_type>::handle_message():" << '\n'
                    << "Received invalid command " << pcToStr(command_container_.get_command())
                    << '\n'
                    << "The client will shut down." << '\n'
                    << GWARNING;
            close_code_ = boost::beast::websocket::close_code::internal_error;
            break;
        }
    }

    //-------------------------------------------------------------------------
    /**
	  * Moves the just-received work item to the compute pool for evaluation, keeping the io thread
	  * free to service pings. A work guard pins io_context::run() open from here until
	  * finish_compute() has run, so the posted result is always delivered -- no premature drain and
	  * no leftover-handler leak on shutdown.
	  */
    void dispatch_compute() {
        // The item being evaluated lives in in_flight_container_, owned by the worker until it posts
        // the result back; command_container_ is then free for the next read. Only one evaluation is
        // ever in flight (the server sends the next item only after receiving the RESULT), so this
        // container is never touched by two threads at once.
        in_flight_container_ = std::move(command_container_);

        auto self = this->shared_from_this();
        auto guard = boost::asio::make_work_guard(io_context_);
        boost::asio::post(
            compute_pool_,
            [self, guard = std::move(guard)]() mutable {
                try {
                    self->in_flight_container_.process();
                }
                catch(const g_processing_exception &e) {
                    glogger << "In GWebsocketClientT<processable_type>::dispatch_compute():" << '\n'
                            << "The work item flagged a processing exception:" << '\n'
                            << e.what() << '\n'
                            << "It is returned to the server flagged; the client keeps running."
                            << '\n'
                            << GWARNING;
                }
                // Hop back onto the io thread to send the result -- ws_ must only be touched there.
                boost::asio::post(
                    self->io_context_,
                    [self, guard = std::move(guard)]() mutable { self->finish_compute(); }
                );
            }
        );
    }

    //-------------------------------------------------------------------------
    /** @brief Runs on the io thread once an evaluation has completed: returns the result to the
	 *  server. The work guard captured by the posting lambda is released when this returns. */
    void finish_compute() {
        this->incrementProcessingCounter();
        in_flight_container_.set_command(networked_consumer_payload_command::RESULT);
        send_command_(in_flight_container_);
    }

    //-------------------------------------------------------------------------
    /** @brief Serializes @p container and writes it to the server, guarding against serialization
	 *  failures (which must not unwind the io thread). */
    void send_command_(
        const GCommandContainerT<processable_type, networked_consumer_payload_command> &container
    ) {
        try {
            this->async_start_write(
                Gem::Courtier::container_to_string(container, serialization_mode_)
            );
        }
        catch(const std::exception &e) {
            glogger << "In GWebsocketClientT<processable_type>::send_command_():" << '\n'
                    << "Could not serialize the outgoing message:" << '\n'
                    << e.what() << '\n'
                    << "The client will shut down." << '\n'
                    << GWARNING;
            close_code_ = boost::beast::websocket::close_code::internal_error;
        }
    }

    //-------------------------------------------------------------------------
    /**
	  * Closes the connection to the peer
	  *
	  * @param cc The close code to be sent to the peer
	  */
    void do_close(close_code cc) {
        // Stop the halt-poll timer; if it has already fired this is a harmless no-op.
        halt_timer_.cancel();

        if(ws_.is_open()) {
            boost::system::error_code wc_ec;
            ws_.close(cc, wc_ec); // ec-overload: we are tearing down, do not throw
        }

        if(ws_.next_layer().is_open()) {
            boost::system::error_code ec;

            ws_.next_layer().shutdown(socket::shutdown_both, ec);
            ws_.next_layer().close(ec);

            // A failed shutdown/close (commonly the peer already went away) must NOT throw out of
            // this async-handler context -- that would unwind the io thread. Log and move on.
            if(ec && ec != boost::asio::error::not_connected) {
                glogger << "In GWebsocketClientT<processable_type>::do_close():" << '\n'
                        << "Shutdown of the next layer reported: " << ec.message() << '\n'
                        << GLOGGING;
            }
        }
    }

    //-------------------------------------------------------------------------
    /**
	  * Arms a periodic timer that polls the base-class halt() condition (max runtime / stop request /
	  * error flag). Because a read is kept outstanding at all times, a halt would otherwise never be
	  * observed; on halt we do_close(), which aborts the outstanding read/write so io_context::run()
	  * drains and the client terminates.
	  */
    void start_halt_timer() {
        halt_timer_.expires_after(std::chrono::seconds(1));
        auto self = this->shared_from_this();
        halt_timer_.async_wait([self](boost::system::error_code ec) { self->on_halt_timer(ec); });
    }

    //-------------------------------------------------------------------------
    /** @brief Timer callback: tears the connection down once a halt condition is reached. */
    void on_halt_timer(boost::system::error_code ec) {
        if(ec) { // the timer was cancelled (do_close) -- stop polling
            return;
        }
        if(this->halt()) {
            do_close(boost::beast::websocket::close_code::normal);
            return;
        }
        start_halt_timer(); // keep polling
    }

    //-------------------------------------------------------------------------

    /** @brief Callback for control frames */
    std::function<void(boost::beast::websocket::frame_type, boost::beast::string_view)>
        f_when_control_frame_arrived_;

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
    }; ///< Holds the current command and payload (read/written on the io thread)

    GCommandContainerT<processable_type, networked_consumer_payload_command> in_flight_container_{
        networked_consumer_payload_command::NONE
    }; ///< Holds the work item currently being evaluated on the compute pool (one at a time)

    boost::asio::steady_timer halt_timer_{
        io_context_
    }; ///< Periodically polls halt() to cancel the outstanding read on shutdown

    /// A single-thread pool that runs the (possibly long, unbounded) work-item evaluation OFF the io
    /// thread, so the io thread stays free to answer websocket pings while a work item is computed.
    /// Declared last so it is destroyed (and its thread joined) before io_context_ and ws_.
    boost::asio::thread_pool compute_pool_{1};

    // Note: the heavy evaluation now runs on compute_pool_ (not inline on the io thread); the io
    // thread keeps an async_read outstanding throughout, so Beast services pings during evaluation.
    // Only the io thread ever touches ws_ (the worker posts the result write back to it), so Beast's
    // single-reader/single-writer rule is upheld.

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
        f_when_control_frame_arrived_ = [this](frame_type frame_t, string_view s) {
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
        ws_.control_callback(f_when_control_frame_arrived_);

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

        // Disable Nagle's algorithm on the underlying TCP socket (already connected at this
        // point): the request/response messages are small and latency-sensitive.
        boost::system::error_code nd_ec;
        ws_.next_layer().set_option(boost::asio::ip::tcp::no_delay(true), nd_ec);

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
            // operation_aborted = the timer was cancelled by do_close() during teardown -- expected.
            return;
        }

        if(ping_state_ == beast_ping_state::CONNECTION_IS_ALIVE) {
            // A ping or pong was seen since the last tick: the peer is healthy. Reset the miss
            // counter and start the next ping interval.
            missed_pings_ = 0;
            async_start_ping();
            return;
        }

        // No pong came back within this interval. Tolerate a few consecutive misses before declaring
        // the connection dead: a healthy client now answers pings promptly (it evaluates work items
        // off its io thread), so a single miss is a transient hiccup, not death. Only a peer that is
        // genuinely unresponsive over several intervals is torn down -- which reclaims the session
        // and its socket (so the server does not slowly leak connections to vanished clients).
        if(++missed_pings_ >= max_missed_pings_) {
            if(not this->check_server_stopped_()) {
                glogger << "GWebsocketConsumerSessionT<processable_type>::when_timer_fired():" << '\n'
                        << "No pong after " << max_missed_pings_
                        << " ping intervals; closing the connection." << '\n'
                        << GLOGGING;
            }
            do_close(boost::beast::websocket::close_code::going_away);
            return;
        }

        // Give the peer another interval to respond.
        async_start_ping();

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
        [[maybe_unused]] std::size_t nothing
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
        [[maybe_unused]] std::size_t nothing
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
        f_when_control_frame_arrived_;

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
            boost::system::error_code wc_ec;
            ws_.close(cc, wc_ec); // ec-overload: tearing down, do not throw
        }

        if(ws_.next_layer().is_open()) {
            boost::system::error_code ec;

            // Closing the socket cancels all outstanding operations. They
            // will complete with boost::asio::error::operation_aborted
            ws_.next_layer().shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
            ws_.next_layer().close(ec);

            // A failed shutdown/close (commonly the peer already vanished) must NOT throw out of this
            // async-handler context -- that would unwind an io thread. Log and move on.
            if(ec && ec != boost::asio::error::not_connected) {
                glogger << "GWebsocketConsumerSessionT<processable_type>::do_close():" << '\n'
                        << "Shutdown of the next layer reported: " << ec.message() << '\n'
                        << GLOGGING;
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
                using enum Gem::Courtier::networked_consumer_payload_command;
            case GETDATA: {
                return getAndSerializeWorkItem();
            } /* break; */ // break is unreachable

            case RESULT: {
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

    /// Consecutive ping intervals without a pong; the connection is declared dead only once this
    /// reaches max_missed_pings_ (tolerating transient hiccups -- a healthy client now pongs promptly).
    unsigned int missed_pings_ = 0;
    static constexpr unsigned int max_missed_pings_ = 3;

    GCommandContainerT<processable_type, networked_consumer_payload_command> command_container_{
        networked_consumer_payload_command::NONE
    }; ///< Holds the current command and payload (if any)

    //-------------------------------------------------------------------------
};


/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Gem::Courtier::Consumers */
