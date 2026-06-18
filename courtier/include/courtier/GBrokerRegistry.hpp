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
#include <functional>
#include <memory>
#include <mutex>

// Geneva headers
#include "common/GSingletonT.hpp"
#include "courtier/GBrokerT.hpp"

namespace Gem::Courtier {

/******************************************************************************/
/**
 * The kind of consumer a broker fronts. An optimization algorithm never selects a kind -- it just
 * submits through whatever broker it is given -- so this is used only for build-time bookkeeping (e.g.
 * recognising that the shared work broker is networked, to avoid binding its port a second time).
 */
enum class broker_kind {
    serial,        ///< Inline, single-threaded local consumer (GSerialConsumerT)
    multithreaded, ///< Local thread-pool consumer (GStdThreadConsumerT) -- one worker pool per process
    networked      ///< A networked consumer (asio / websocket / mpi) -- one listening endpoint per process
};

/******************************************************************************/
/**
 * A process-global holder for the single SHARED WORK BROKER -- the one consumer every un-injected
 * optimization algorithm submits through. An algorithm is transport-agnostic: it either has a broker
 * explicitly injected (Go2 into its chained algorithms, or a meta-optimization master's own
 * orchestration pool) or, failing that, resolves the one shared work broker here and calls workOn()
 * without ever knowing whether evaluation happens locally, on a GPU, or on a remote client. So a whole
 * process exposes a single work endpoint (one pool, or one networked port) that all un-injected
 * algorithms -- including the inner algorithms of a meta-optimization -- converge on, instead of each
 * building its own.
 *
 * The single instance is handed out via GSingletonT (it outlives ordinary statics and is reachable
 * concurrently). All mutation is guarded by an internal mutex, so concurrent submitters (e.g. the inner
 * algorithms of a meta-optimization, evaluated in parallel) race-freely converge on one broker. The
 * holder is CLEARABLE (clearAll) so unit tests can start from a known-empty state -- without that, a
 * broker published by one test would leak into the next.
 *
 * @tparam processable_type The work-item type the shared broker's consumer handles
 */
template <typename processable_type>
class GBrokerRegistryT {
public:
    using broker_t = GBrokerT<processable_type>;
    using broker_ptr = std::shared_ptr<broker_t>;

    GBrokerRegistryT() = default;

    /***************************************************************************/
    /** @brief The process-global registry instance for this work-item type.
     *  @return A reference to the single per-process GBrokerRegistryT<processable_type> */
    static GBrokerRegistryT &instance() {
        return *Gem::Common::GSingletonT<GBrokerRegistryT>::instance();
    }

    /***************************************************************************/
    /** @brief The current shared work broker, or nullptr if none has been established yet.
     *  @return The shared work broker, or nullptr */
    broker_ptr sharedWorkBroker() {
        const std::lock_guard<std::mutex> lk(mtx_);
        return work_broker_;
    }

    /***************************************************************************/
    /** @brief The kind of the current shared work broker (meaningful only when one exists). Used for
     *  build-time idempotency (recognising a networked endpoint), never for algorithm-side resolution.
     *  @return The kind of the current shared work broker */
    broker_kind sharedWorkKind() {
        const std::lock_guard<std::mutex> lk(mtx_);
        return work_kind_;
    }

    /***************************************************************************/
    /**
     * @brief Returns the shared work broker, building and publishing one via @p factory if none exists
     * yet. Idempotent under concurrency: only the first caller runs the factory; later callers (and
     * concurrent ones) get the same broker, so a process has at most one shared work consumer. The
     * factory runs under the lock (broker construction is cheap and one-shot).
     *
     * @param kind The kind recorded for the broker the factory builds (for build-time bookkeeping)
     * @param factory Builds a ready broker (consumer registered, clone function set) when none is present
     * @return The shared work broker (the pre-existing one, or the freshly built+published one)
     */
    broker_ptr ensureSharedWork(broker_kind kind, const std::function<broker_ptr()> &factory) {
        const std::lock_guard<std::mutex> lk(mtx_);
        if(work_broker_) {
            return work_broker_;
        }
        work_broker_ = factory();
        work_kind_ = kind;
        return work_broker_;
    }

    /***************************************************************************/
    /** @brief Publishes (replaces) the shared work broker -- e.g. the one Go2 builds -- so every
     *  un-injected algorithm submits through it.
     *  @param kind The kind of the broker being published
     *  @param b The broker to publish as the shared work broker */
    void publishSharedWork(broker_kind kind, broker_ptr b) {
        const std::lock_guard<std::mutex> lk(mtx_);
        work_broker_ = std::move(b);
        work_kind_ = kind;
    }

    /***************************************************************************/
    /** @brief Drops the shared work broker, returning the holder to its empty state. Primarily for
     *  tests (start each from a known-empty state) and explicit teardown. */
    void clearAll() {
        const std::lock_guard<std::mutex> lk(mtx_);
        work_broker_.reset();
    }

private:
    std::mutex mtx_;                                  ///< Guards the shared work broker
    broker_ptr work_broker_;                          ///< The single shared work broker (null until established)
    broker_kind work_kind_ = broker_kind::multithreaded; ///< Kind of work_broker_ (bookkeeping only)
};

/******************************************************************************/

} /* namespace Gem::Courtier */
