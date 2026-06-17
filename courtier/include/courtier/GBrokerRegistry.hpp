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
#include <map>
#include <memory>
#include <mutex>

// Geneva headers
#include "common/GSingletonT.hpp"
#include "courtier/GBrokerT.hpp"

namespace Gem::Courtier {

/******************************************************************************/
/**
 * The kind of consumer a broker fronts. The single-shared-consumer invariant is "at most one consumer
 * of a given kind per process, shared by every algorithm that needs that kind", so the registry below
 * holds at most one broker PER KIND: a local serial consumer, a local thread-pool consumer, and a
 * networked consumer (the one external clients connect to) may coexist, but never two of the same kind.
 */
enum class broker_kind {
    serial,        ///< Inline, single-threaded local consumer (GSerialConsumerT)
    multithreaded, ///< Local thread-pool consumer (GStdThreadConsumerT) -- one worker pool per process
    networked      ///< A networked consumer (asio / websocket / mpi) -- one listening endpoint per process
};

/******************************************************************************/
/**
 * A process-global, per-kind registry of brokers. It lets every un-injected optimization algorithm that
 * needs a consumer of a given kind converge on ONE shared broker (and hence one consumer / one worker
 * pool / one networked port) instead of each building its own. The execution policy consults it after an
 * explicitly-injected broker (which always wins): explicit injection -> the process-global broker of the
 * requested kind -> build the default consumer of that kind and register it here.
 *
 * The single instance is handed out via GSingletonT (it outlives ordinary statics and is reachable
 * concurrently). All mutation is guarded by an internal mutex, so concurrent submitters (e.g. the inner
 * algorithms of a meta-optimization, evaluated in parallel) race-freely converge on one broker. The
 * registry is CLEARABLE (clear / clearAll) so unit tests can start from a known-empty state -- without
 * that, a broker registered by one test would leak into the next.
 *
 * @tparam processable_type The work-item type the registered brokers' consumers handle
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
    /** @brief The broker currently registered for @p kind, or nullptr if none.
     *  @param kind The consumer kind to look up
     *  @return The registered broker of that kind, or nullptr */
    broker_ptr get(broker_kind kind) {
        const std::lock_guard<std::mutex> lk(mtx_);
        const auto it = store_.find(kind);
        return it == store_.end() ? broker_ptr{} : it->second;
    }

    /***************************************************************************/
    /**
     * @brief Returns the broker registered for @p kind, building and registering one via @p factory if
     * none exists yet. Idempotent under concurrency: only the first caller for a given kind runs the
     * factory; later callers (and concurrent ones) get the same broker, so there is never more than one
     * consumer of a kind. The factory runs under the registry lock (broker construction is cheap and
     * one-shot per kind).
     *
     * @param kind The consumer kind to resolve
     * @param factory Builds a ready broker (consumer registered, clone function set) when none is present
     * @return The shared broker for @p kind (the pre-existing one, or the freshly built+registered one)
     */
    broker_ptr getOrRegister(broker_kind kind, const std::function<broker_ptr()> &factory) {
        const std::lock_guard<std::mutex> lk(mtx_);
        const auto it = store_.find(kind);
        if(it != store_.end()) {
            return it->second;
        }
        broker_ptr b = factory();
        store_[kind] = b;
        return b;
    }

    /***************************************************************************/
    /** @brief Registers (or replaces) the broker for @p kind. Used to publish an externally-built broker
     *  (e.g. the one Go2 constructs) so un-injected algorithms of that kind share it.
     *  @param kind The consumer kind to publish under
     *  @param b The broker to register for that kind */
    void set(broker_kind kind, broker_ptr b) {
        const std::lock_guard<std::mutex> lk(mtx_);
        store_[kind] = std::move(b);
    }

    /***************************************************************************/
    /** @brief Drops the broker registered for @p kind (no-op if none). The next request for that kind
     *  rebuilds it. Primarily for tests and explicit teardown.
     *  @param kind The consumer kind to clear */
    void clear(broker_kind kind) {
        const std::lock_guard<std::mutex> lk(mtx_);
        store_.erase(kind);
    }

    /***************************************************************************/
    /** @brief Drops all registered brokers, returning the registry to its empty state. Primarily for
     *  tests (start each from a known-empty registry) and explicit teardown. */
    void clearAll() {
        const std::lock_guard<std::mutex> lk(mtx_);
        store_.clear();
    }

private:
    std::mutex mtx_;                          ///< Guards store_ against concurrent submitters
    std::map<broker_kind, broker_ptr> store_; ///< At most one broker per kind
};

/******************************************************************************/

} /* namespace Gem::Courtier */
