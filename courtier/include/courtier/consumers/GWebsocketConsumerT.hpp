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

#include "common/GGlobalDefines.hpp"

// Standard headers
#include <atomic>
#include <chrono>
#include <cstddef>
#include <memory>
#include <thread>

// Boost headers
#include <boost/asio.hpp>

// Geneva headers
#include "common/GErrorStreamer.hpp"
#include "common/GExceptions.hpp"
#include "common/GLogger.hpp"
#include "common/GThreadGroup.hpp"
#include "courtier/transport/GWebsocketTransportT.hpp" // reuse the existing session + client + protocol
#include "courtier/consumers/GNetworkedConsumerT.hpp"

namespace Gem::Courtier {

/******************************************************************************/
/**
 * The courtier websocket consumer. Functionally the websocket twin of GAsioConsumerT: a TCP server
 * whose accepted connections are upgraded to websocket sessions (with keep-alive ping/pong) that
 * hand out work from the current batch and collect results, reconciled via the inherited
 * GNetworkedConsumerT / GBaseConsumerT machinery.
 *
 * As with the ASIO consumer it reuses the existing courtier session
 * (Gem::Courtier::Consumers::GWebsocketConsumerSessionT) and GCommandContainerT wire protocol, so an
 * unmodified Gem::Courtier::Consumers::GWebsocketClientT serves a courtier server.
 *
 * @tparam processable_type The work-item type handed to clients and reconciled back into the population
 */
template <typename processable_type>
class GWebsocketConsumerT final
  : public GNetworkedConsumerT<processable_type>
  , public std::enable_shared_from_this<GWebsocketConsumerT<processable_type>> {
public:
    using session_type = Gem::Courtier::Consumers::GWebsocketConsumerSessionT<processable_type>;

    /***************************************************************************/
    /** @brief Constructs the websocket consumer.
     *
     *  @param port The TCP port the server listens on (0 lets the OS pick; the chosen port is read back)
     *  @param n_threads Number of io threads to run; 0 selects the hardware concurrency
     *  @param serialization_mode Which serialization format (binary/XML/text) the wire protocol uses
     *  @param ping_interval Time in seconds between keep-alive pings each session sends to its client
     *  @param verbose_control_frames If true, sessions log a diagnostic message for every control frame */
    explicit GWebsocketConsumerT(
        unsigned short port,
        std::size_t n_threads = 0,
        Gem::Common::serializationMode serialization_mode = Gem::Common::serializationMode::BINARY,
        std::size_t ping_interval = 5,
        bool verbose_control_frames = false
    )
        : port_(port)
        , n_threads_(n_threads == 0 ? default_threads() : n_threads)
        , serialization_mode_(serialization_mode)
        , ping_interval_(ping_interval)
        , verbose_control_frames_(verbose_control_frames)
    { /* nothing */ }

    /** @brief The destructor. Stops the server (idempotent). */
    ~GWebsocketConsumerT() override { this->stopServer(); }

    GWebsocketConsumerT(const GWebsocketConsumerT &) = delete;
    GWebsocketConsumerT(GWebsocketConsumerT &&) = delete;
    GWebsocketConsumerT &operator=(const GWebsocketConsumerT &) = delete;
    GWebsocketConsumerT &operator=(GWebsocketConsumerT &&) = delete;

    /***************************************************************************/
    /** @brief Returns the TCP port the server is bound to (the OS-chosen port after startServer()
     *  when 0 was requested).
     *  @return The active listening port */
    [[nodiscard]] unsigned short getPort() const noexcept { return port_; }
    /** @brief Returns the number of currently active client sessions.
     *  @return The live session count */
    [[nodiscard]] std::size_t getNActiveSessions() const noexcept { return n_active_sessions_.load(); }

    /***************************************************************************/
    /** @brief Opens, binds and listens on the acceptor, then starts accepting connections and spins
     *  up the io threads. Throws a geneva_exception if the acceptor cannot be opened/bound. */
    void startServer() {
        boost::system::error_code ec;

        boost::asio::ip::tcp::endpoint endpoint{boost::asio::ip::tcp::v4(), port_};
        acceptor_.open(endpoint.protocol(), ec);
        if(ec || not acceptor_.is_open()) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In Gem::Courtier::GWebsocketConsumerT::startServer(): could not open the acceptor: "
                << ec.message() << '\n'
            );
        }
        acceptor_.set_option(boost::asio::socket_base::reuse_address(true));
        acceptor_.bind(endpoint, ec);
        if(ec) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In Gem::Courtier::GWebsocketConsumerT::startServer(): could not bind to port "
                << port_ << ": " << ec.message() << '\n'
            );
        }
        port_ = acceptor_.local_endpoint().port();

        acceptor_.listen(boost::asio::socket_base::max_listen_connections, ec);
        if(ec) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In Gem::Courtier::GWebsocketConsumerT::startServer(): could not listen: "
                << ec.message() << '\n'
            );
        }

        this->async_start_accept();

        for(std::size_t t = 0; t < n_threads_; ++t) {
            gtg_.create_thread([this] { io_context_.run(); });
        }
    }

    /***************************************************************************/
    /** @brief Stops the server (idempotent): requests a stop, closes the acceptor on the accept
     *  strand, releases the work guard and joins the io threads. */
    void stopServer() {
        if(stopped_already_.exchange(true)) {
            return;
        }
        this->requestStop();

        // Close the acceptor and cancel the retry timer ON the accept strand, so this never runs
        // concurrently with the accept handler (a tcp::acceptor is not thread-safe).
        boost::asio::post(accept_strand_, [this]() {
            boost::system::error_code ec;
            acceptor_.close(ec);
            accept_retry_timer_.cancel();
        });

        work_guard_.reset();
        io_context_.stop();
        gtg_.join_all();
    }

protected:
    /***************************************************************************/
    /** @brief The websocket consumer has a real client-liveness signal (its persistent session +
     *  keep-alive ping/pong), so it reclaims a lost item immediately on disconnect via the session's
     *  CheckoutLease and does NOT use the time lease -- a live-but-slow client keeps its item. */
    bool usesTimeLease() const override { return false; }

private:
    /***************************************************************************/
    /** @brief Default io-thread count derived from the hardware concurrency (at least 1).
     *  @return The number of io threads to use when none was requested */
    static std::size_t default_threads() {
        const unsigned int hc = std::thread::hardware_concurrency();
        return hc == 0 ? 1u : hc;
    }

    /***************************************************************************/
    /** @brief Arms one asynchronous accept, bound to the accept strand so all (non-thread-safe)
     *  acceptor access is serialized across the io threads. */
    void async_start_accept() {
        // Connectionless async_accept overload (a fresh socket per accept -- no shared socket_ to
        // race on), with the handler bound to accept_strand_ so all acceptor access is serialized
        // across the io threads (the acceptor is not thread-safe).
        auto self = this->shared_from_this();
        acceptor_.async_accept(
            boost::asio::bind_executor(
                accept_strand_,
                [self](boost::system::error_code ec, boost::asio::ip::tcp::socket socket) {
                    self->when_accepted(ec, std::move(socket));
                }
            )
        );
    }

    /***************************************************************************/
    /** @brief Accept handler: starts a new persistent session for the connection (with a CheckoutLease
     *  that requeues any in-flight items if the client dies), then re-arms the next accept. Backs off
     *  briefly on a transient accept failure instead of busy-spinning.
     *
     *  @param ec The error code of a potential accept failure
     *  @param socket The freshly accepted TCP socket, moved into the new session */
    void when_accepted(boost::system::error_code ec, boost::asio::ip::tcp::socket socket) {
        if(this->stopped()) {
            return; // shutting down: do not start a session and do not re-arm
        }

        if(ec) {
            // A closed acceptor (operation_aborted / bad_descriptor) means we are stopping -- give up.
            if(ec == boost::asio::error::operation_aborted
               || ec == boost::asio::error::bad_descriptor) {
                return;
            }
            // A transient accept failure (e.g. EMFILE -- too many open files) must NOT be retried in
            // a tight loop: back off briefly so the io thread is not pinned and the listen backlog can
            // drain as file descriptors free up.
            glogger << "In Gem::Courtier::GWebsocketConsumerT::when_accepted(): " << ec.message()
                    << " -- backing off before retrying accept" << '\n'
                    << GWARNING;
            accept_retry_timer_.expires_after(std::chrono::milliseconds(100));
            auto self = this->shared_from_this();
            accept_retry_timer_.async_wait(
                boost::asio::bind_executor(accept_strand_, [self](boost::system::error_code tec) {
                    if(not tec && not self->stopped()) {
                        self->async_start_accept();
                    }
                })
            );
            return;
        }

        // The websocket session is persistent (the client keeps the connection open while it
        // evaluates), so a disconnect IS a real death signal. Give the session a CheckoutLease: if it
        // dies still holding items, the lease requeues all of them immediately for other clients --
        // liveness-driven put-back, no time lease needed (see usesTimeLease()). A prefetching client may
        // hold several items at once, so the lease tracks the whole in-flight set, not just the latest.
        auto lease = std::make_shared<typename GNetworkedConsumerT<processable_type>::CheckoutLease>();
        lease->on_abandon = [w = this->weak_from_this()](Gem::Courtier::CORRELATION_ID_TYPE id) {
            if(auto s = w.lock()) {
                s->requeue(id);
            }
        };

        std::make_shared<session_type>(
            io_context_,
            std::move(socket),
            [self = this->shared_from_this(), lease]() -> std::unique_ptr<processable_type> {
                auto p = self->checkout();
                lease->add(p); // no-op for a null item; records p's correlation id
                return p;
            },
            [self = this->shared_from_this(), lease](std::unique_ptr<processable_type> p) {
                lease->remove(p); // returned normally -> nothing for the lease to reclaim
                self->checkin(std::move(p));
            },
            [self = this->shared_from_this()]() -> bool { return self->stopped(); },
            [self = this->shared_from_this()](bool sign_on) {
                if(sign_on) {
                    ++self->n_active_sessions_;
                }
                else if(self->n_active_sessions_.load() > 0) {
                    --self->n_active_sessions_;
                }
            },
            serialization_mode_,
            ping_interval_,
            verbose_control_frames_
        )
            ->async_start_run();

        // Re-arm for the next connection.
        this->async_start_accept();
    }

    /***************************************************************************/
    unsigned short port_;
    std::size_t n_threads_;
    Gem::Common::serializationMode serialization_mode_;
    std::size_t ping_interval_;
    bool verbose_control_frames_;

    boost::asio::io_context io_context_;
    boost::asio::executor_work_guard<boost::asio::io_context::executor_type> work_guard_ =
        boost::asio::make_work_guard(io_context_);
    boost::asio::ip::tcp::acceptor acceptor_{io_context_};
    /// Serializes all acceptor access (async_accept re-arm, close, retry timer) across the io threads.
    boost::asio::strand<boost::asio::io_context::executor_type> accept_strand_{
        io_context_.get_executor()
    };
    /// Backoff timer used to retry accept after a transient failure (e.g. EMFILE) without busy-spinning.
    boost::asio::steady_timer accept_retry_timer_{io_context_};

    Gem::Common::GThreadGroup gtg_;
    std::atomic<std::size_t> n_active_sessions_{0};
    std::atomic<bool> stopped_already_{false};
};

/******************************************************************************/

} /* namespace Gem::Courtier */
