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
#include <memory>
#include <thread>
#include <vector>

// Boost headers
#include <boost/asio.hpp>

// Geneva headers
#include "common/GErrorStreamer.hpp"
#include "common/GExceptions.hpp"
#include "common/GLogger.hpp"
#include "common/GThreadGroup.hpp"
#include "courtier/consumers/GAsioConsumerT.hpp" // reuse the existing session + client + wire protocol
#include "courtier2/consumers/GNetworkedConsumerT.hpp"

namespace Gem::Courtier2 {

/******************************************************************************/
/**
 * The courtier2 ASIO consumer. It is a TCP server that hands out work items from the current batch
 * to connected clients and collects the processed results, reconciling them against the submission
 * policy via the inherited GNetworkedConsumerT / GBaseConsumerT machinery.
 *
 * It deliberately reuses the existing courtier session (Gem::Courtier::Consumers::
 * GAsioConsumerSessionT) and its GCommandContainerT wire protocol, so an unmodified
 * Gem::Courtier::Consumers::GAsioConsumerClientT can serve a courtier2 server (the rework is
 * behaviour-neutral at the protocol level). Only the server lifecycle and the per-batch work
 * queue are new here.
 */
template <typename processable_type>
class GAsioConsumerT final
  : public GNetworkedConsumerT<processable_type>
  , public std::enable_shared_from_this<GAsioConsumerT<processable_type>> {
public:
    using session_type = Gem::Courtier::Consumers::GAsioConsumerSessionT<processable_type>;

    /***************************************************************************/
    /** @brief Initialization with the listen port, the number of io threads and the serialization
     *  mode used on the wire. */
    explicit GAsioConsumerT(
        unsigned short port,
        std::size_t n_threads = 0,
        Gem::Common::serializationMode serialization_mode = Gem::Common::serializationMode::BINARY
    )
        : port_(port)
        , n_threads_(n_threads == 0 ? default_threads() : n_threads)
        , serialization_mode_(serialization_mode)
    { /* nothing */ }

    ~GAsioConsumerT() override { this->stopServer(); }

    GAsioConsumerT(const GAsioConsumerT &) = delete;
    GAsioConsumerT(GAsioConsumerT &&) = delete;
    GAsioConsumerT &operator=(const GAsioConsumerT &) = delete;
    GAsioConsumerT &operator=(GAsioConsumerT &&) = delete;

    /***************************************************************************/
    /** @brief The port the server listens on (useful when 0 was passed to pick an ephemeral port). */
    [[nodiscard]] unsigned short getPort() const noexcept { return port_; }

    /** @brief Number of clients currently connected and being served. */
    [[nodiscard]] std::size_t getNActiveSessions() const noexcept { return n_active_sessions_.load(); }

    /***************************************************************************/
    /**
     * Opens the acceptor and starts the io threads. Must be called once, after the consumer has
     * been registered with the broker and before any batch is submitted.
     */
    void startServer() {
        boost::system::error_code ec;

        boost::asio::ip::tcp::endpoint endpoint{boost::asio::ip::tcp::v4(), port_};
        acceptor_.open(endpoint.protocol(), ec);
        if(ec || not acceptor_.is_open()) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In Gem::Courtier2::GAsioConsumerT::startServer(): could not open the acceptor: "
                << ec.message() << '\n'
            );
        }
        acceptor_.set_option(boost::asio::socket_base::reuse_address(true));
        acceptor_.bind(endpoint, ec);
        if(ec) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In Gem::Courtier2::GAsioConsumerT::startServer(): could not bind to port "
                << port_ << ": " << ec.message() << '\n'
            );
        }
        // If port 0 was requested, learn the ephemeral port the OS assigned.
        port_ = acceptor_.local_endpoint().port();

        acceptor_.listen(boost::asio::socket_base::max_listen_connections, ec);
        if(ec) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In Gem::Courtier2::GAsioConsumerT::startServer(): could not listen: "
                << ec.message() << '\n'
            );
        }

        this->async_start_accept();

        for(std::size_t t = 0; t < n_threads_; ++t) {
            gtg_.create_thread([this] { io_context_.run(); });
        }
    }

    /***************************************************************************/
    /**
     * Stops accepting connections, lets the io threads drain and joins them. Idempotent.
     */
    void stopServer() {
        if(stopped_already_.exchange(true)) {
            return;
        }
        this->requestStop();

        boost::system::error_code ec;
        acceptor_.close(ec);

        // Drop the work guard and stop the context so run() returns on every io thread.
        work_guard_.reset();
        io_context_.stop();
        gtg_.join_all();
    }

private:
    /***************************************************************************/
    static std::size_t default_threads() {
        const unsigned int hc = std::thread::hardware_concurrency();
        return hc == 0 ? 1u : hc;
    }

    /***************************************************************************/
    void async_start_accept() {
        auto self = this->shared_from_this();
        acceptor_.async_accept(socket_, [self](boost::system::error_code ec) {
            self->when_accepted(ec);
        });
    }

    /***************************************************************************/
    void when_accepted(boost::system::error_code ec) {
        if(ec) {
            if(not this->stopped()) {
                glogger << "In Gem::Courtier2::GAsioConsumerT::when_accepted(): " << ec.message()
                        << '\n'
                        << GWARNING;
            }
        }
        else {
            std::make_shared<session_type>(
                io_context_,
                std::move(socket_),
                [self = this->shared_from_this()]() -> std::shared_ptr<processable_type> {
                    return self->checkout();
                },
                [self = this->shared_from_this()](std::shared_ptr<processable_type> p) {
                    self->checkin(p);
                },
                [self = this->shared_from_this()]() -> bool { return self->stopped(); },
                serialization_mode_,
                [self = this->shared_from_this()](bool sign_on) {
                    if(sign_on) {
                        ++self->n_active_sessions_;
                    }
                    else if(self->n_active_sessions_.load() > 0) {
                        --self->n_active_sessions_;
                    }
                }
            )
                ->async_start_run();
        }

        if(not this->stopped()) {
            this->async_start_accept();
        }
    }

    /***************************************************************************/
    unsigned short port_;
    std::size_t n_threads_;
    Gem::Common::serializationMode serialization_mode_;

    boost::asio::io_context io_context_;
    boost::asio::executor_work_guard<boost::asio::io_context::executor_type> work_guard_ =
        boost::asio::make_work_guard(io_context_);
    boost::asio::ip::tcp::acceptor acceptor_{io_context_};
    boost::asio::ip::tcp::socket socket_{io_context_};

    Gem::Common::GThreadGroup gtg_;
    std::atomic<std::size_t> n_active_sessions_{0};
    std::atomic<bool> stopped_already_{false};
};

/******************************************************************************/

} /* namespace Gem::Courtier2 */
