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
#include <cstddef>
#include <cstdint>
#include <functional>
#include <list>
#include <mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>

// Geneva headers go here
#include "common/GCommonEnums.hpp" // Gem::Common::serializationMode

namespace Gem::Courtier {

/******************************************************************************/
/**
 * The transport-side "layout send-once" machinery (Phase 9). A flat genome ships a by-value copy of its
 * shared, immutable structural layout with every individual; that layout is identical across an entire
 * population, so re-sending it with every work item dominates the wire size for a large structured
 * genome. This header provides the TRANSPORT-AGNOSTIC plumbing that lets a serializer send each distinct
 * layout once and reference it by a content id thereafter, with a mandatory cache-miss -> fetch fallback
 * for networked transports (reconnect / late-joining workers).
 *
 * It lives in the courtier (consumer/session) layer so all three transports -- Asio, websocket, MPI --
 * inherit it once, and it deliberately knows NOTHING about the genome or its layout type: a layout is an
 * OPAQUE serialized blob (a std::string) keyed by an opaque 128-bit content id. The upper (geneva) layer
 * computes the id, (de)serializes the layout to/from the blob, and consults the thread-local context set
 * around a (de)serialization; courtier owns the registry and the session-scoped bookkeeping (which peer
 * already holds which layout, eviction), because session lifetime is a courtier concern.
 *
 * Checkpoint / file serialization does NOT use this machinery (a checkpoint must stay self-contained):
 * it is engaged only when a transport establishes a GWireSerializationScope around the wire encoding.
 */

/******************************************************************************/
/** @brief An opaque 128-bit content id of an interned layout blob (the high/low halves of a hash). */
using GWireLayoutId = std::array<std::uint64_t, 2>;

/** @brief Identifies a transport peer (a server-side session / a connected client). 0 == "no specific
 *  peer", used on the worker side where there is only the single upstream server. */
using GWirePeerId = std::uint64_t;

/******************************************************************************/
/** @brief Hashes a GWireLayoutId (which is already a strong hash) into a size_t for the registry maps. */
struct GWireLayoutIdHash {
    /** @brief @param id The id to hash. @return A size_t hash of the id. */
    std::size_t operator()(const GWireLayoutId &id) const noexcept {
        return static_cast<std::size_t>(id[0] ^ (id[1] + 0x9E3779B97F4A7C15ULL + (id[0] << 6) + (id[0] >> 2)));
    }
};

/******************************************************************************/
/**
 * The thread-safe, content-addressed registry of interned layout blobs, shared by a consumer's sessions
 * (server) or held by a client (worker). It serves two roles with one store:
 *
 *  - BLOB STORE (both sides): id -> serialized-layout-blob. A worker caches every layout it has received
 *    so a later id-only reference resolves locally; a server caches every layout it has sent so it can
 *    answer a worker's cache-miss fetch.
 *  - PER-PEER ACK TRACKING (server side): for each connected peer, the set of layout ids that peer is
 *    known to already hold, so the server sends a given layout to a given peer exactly once and references
 *    it by id afterwards. A peer's set is dropped when its session ends (forgetPeer) -- on reconnect the
 *    peer starts empty, and the miss -> fetch fallback transparently re-sends what it needs.
 *
 * A capacity bound (LRU over the blob store) keeps a long run with many evolving layouts from growing
 * without limit; eviction never breaks correctness because a missing blob is re-fetched on demand.
 */
class GWireLayoutRegistry {
public:
    GWireLayoutRegistry() = default;

    GWireLayoutRegistry(const GWireLayoutRegistry &) = delete;
    GWireLayoutRegistry &operator=(const GWireLayoutRegistry &) = delete;

    /***************************************************************************/
    // Blob store (used by both server and worker).

    /** @brief Whether a blob for the given id is cached.
     *  @param id The layout id to look up. @return true iff the blob is present. */
    bool has(const GWireLayoutId &id) const {
        std::lock_guard<std::mutex> lk(mtx_);
        return blobs_.find(id) != blobs_.end();
    }

    /** @brief Copies out the cached blob for an id, if present (and marks it most-recently-used).
     *  @param id The layout id to look up.
     *  @param out Receives the blob on a hit (left unchanged on a miss).
     *  @return true on a hit (out written), false on a miss. */
    bool tryGet(const GWireLayoutId &id, std::string &out) const {
        std::lock_guard<std::mutex> lk(mtx_);
        auto it = blobs_.find(id);
        if(it == blobs_.end()) {
            return false;
        }
        touch_locked(it->second.lru_pos);
        out = it->second.blob;
        return true;
    }

    /** @brief Caches a blob under its id (idempotent; refreshes recency on a repeat). Enforces the
     *  capacity bound afterwards.
     *  @param id The layout id. @param blob The serialized layout blob to cache. */
    void put(const GWireLayoutId &id, std::string blob) {
        std::lock_guard<std::mutex> lk(mtx_);
        auto it = blobs_.find(id);
        if(it != blobs_.end()) {
            it->second.blob = std::move(blob);
            touch_locked(it->second.lru_pos);
            return;
        }
        lru_.push_front(id);
        blobs_.emplace(id, Entry{std::move(blob), lru_.begin()});
        evict_locked();
    }

    /***************************************************************************/
    // Per-peer ack tracking (server side).

    /** @brief Whether a peer is known to already hold a given layout.
     *  @param peer The peer (session) to query. @param id The layout id.
     *  @return true iff the peer was previously recorded as holding this layout. */
    bool peerHasLayout(GWirePeerId peer, const GWireLayoutId &id) const {
        std::lock_guard<std::mutex> lk(mtx_);
        auto it = peer_acked_.find(peer);
        return it != peer_acked_.end() && it->second.find(id) != it->second.end();
    }

    /** @brief Records that a peer now holds a given layout (so it is referenced by id thereafter).
     *  @param peer The peer (session). @param id The layout id the peer now holds. */
    void markPeerHasLayout(GWirePeerId peer, const GWireLayoutId &id) {
        std::lock_guard<std::mutex> lk(mtx_);
        peer_acked_[peer].insert(id);
    }

    /** @brief Drops all per-peer ack state for a peer (its session ended / it reconnected). The blob
     *  store is left intact (other peers may still need it, and it answers future fetches).
     *  @param peer The peer whose ack state is forgotten. */
    void forgetPeer(GWirePeerId peer) {
        std::lock_guard<std::mutex> lk(mtx_);
        peer_acked_.erase(peer);
    }

    /***************************************************************************/
    // Capacity / introspection.

    /** @brief Sets the maximum number of blobs retained (LRU eviction beyond it); 0 == unbounded.
     *  @param max_blobs The capacity bound (0 disables eviction). */
    void setCapacity(std::size_t max_blobs) {
        std::lock_guard<std::mutex> lk(mtx_);
        capacity_ = max_blobs;
        evict_locked();
    }

    /** @brief @return The number of blobs currently cached. */
    std::size_t size() const {
        std::lock_guard<std::mutex> lk(mtx_);
        return blobs_.size();
    }

    /** @brief @return The number of peers with recorded ack state. */
    std::size_t trackedPeers() const {
        std::lock_guard<std::mutex> lk(mtx_);
        return peer_acked_.size();
    }

    /** @brief Clears the entire registry (blob store + all per-peer ack state). */
    void clear() {
        std::lock_guard<std::mutex> lk(mtx_);
        blobs_.clear();
        lru_.clear();
        peer_acked_.clear();
    }

private:
    /** @brief One cached blob plus its position in the LRU recency list. lru_pos is mutable because a
     *  read (tryGet, a const op) refreshes recency. */
    struct Entry {
        std::string blob;                       ///< the serialized layout blob
        mutable std::list<GWireLayoutId>::iterator lru_pos; ///< this id's node in lru_ (front == most recent)
    };

    /** @brief Moves an id to the front of the recency list (most-recently-used). Caller holds mtx_.
     *  @param pos The id's current node in lru_ (updated in place to the new front node). */
    void touch_locked(std::list<GWireLayoutId>::iterator &pos) const {
        lru_.splice(lru_.begin(), lru_, pos);
        pos = lru_.begin();
    }

    /** @brief Evicts least-recently-used blobs until the capacity bound is met. Per-peer ack sets are
     *  intentionally NOT pruned here: a missing blob is simply re-fetched on demand. Caller holds mtx_. */
    void evict_locked() {
        if(capacity_ == 0) {
            return;
        }
        while(blobs_.size() > capacity_ && not lru_.empty()) {
            const GWireLayoutId victim = lru_.back();
            lru_.pop_back();
            blobs_.erase(victim);
        }
    }

    mutable std::mutex mtx_;
    std::unordered_map<GWireLayoutId, Entry, GWireLayoutIdHash> blobs_; ///< id -> (blob, lru node)
    mutable std::list<GWireLayoutId> lru_;                              ///< recency list (front == MRU)
    std::unordered_map<GWirePeerId, std::unordered_set<GWireLayoutId, GWireLayoutIdHash>>
        peer_acked_;            ///< server side: per-peer set of layout ids the peer already holds
    std::size_t capacity_ = 0;  ///< max blobs retained (0 == unbounded)
};

/******************************************************************************/
/**
 * The per-(de)serialization context that engages the layout send-once machinery. A transport sets one
 * up (via a GWireSerializationScope) around the encoding/decoding of a work item; the upper-layer genome
 * serializer consults the thread-local current() and, when it is enabled, references shared layouts by id
 * against the registry instead of writing a full copy. When no scope is active (the default everywhere,
 * and always for checkpoint/file serialization) the genome falls back to its self-contained full encoding.
 */
struct GWireSerializationContext {
    bool enabled = false;       ///< master switch: false -> genome uses its full, self-contained encoding
    Gem::Common::serializationMode mode =
        Gem::Common::serializationMode::BINARY; ///< the format the surrounding archive uses
    GWirePeerId peer = 0;       ///< server: the destination session; worker: 0 (single upstream)
    GWireLayoutRegistry *registry = nullptr; ///< the shared blob store + ack tracker (not owned)

    /** @brief Worker-side cache-miss fetch: given a layout id whose blob is absent locally, performs the
     *  blocking round trip to the server (REQUEST_LAYOUT -> SEND_LAYOUT) and returns the serialized blob,
     *  or an empty string on failure. Null on the server side (which never fetches). */
    std::function<std::string(const GWireLayoutId &)> fetch_blob;
};

/******************************************************************************/
/**
 * RAII installer of the thread-local active GWireSerializationContext. A transport constructs one around
 * a single work-item (de)serialization; the genome serializer reads GWireSerializationScope::current()
 * for the duration. Scopes nest (the previous context is restored on destruction), so a fetch round trip
 * performed mid-decode can safely run with its own (or no) context. Passing nullptr installs "no context"
 * (the genome then uses its full encoding) -- this is how the wire path stays default-off until the
 * switch is flipped.
 */
class GWireSerializationScope {
public:
    /** @brief Installs @p ctx as the active context for this thread, saving the previous one.
     *  @param ctx The context to make current (may be nullptr to install "no context"). */
    explicit GWireSerializationScope(const GWireSerializationContext *ctx) noexcept
      : prev_(t_current_) {
        t_current_ = ctx;
    }
    /** @brief Restores the previously-active context. */
    ~GWireSerializationScope() { t_current_ = prev_; }

    GWireSerializationScope(const GWireSerializationScope &) = delete;
    GWireSerializationScope &operator=(const GWireSerializationScope &) = delete;
    GWireSerializationScope(GWireSerializationScope &&) = delete;
    GWireSerializationScope &operator=(GWireSerializationScope &&) = delete;

    /** @brief The context active on this thread, or nullptr if none (or it is disabled, treated as none
     *  by callers). @return The current context pointer, or nullptr. */
    static const GWireSerializationContext *current() noexcept { return t_current_; }

private:
    const GWireSerializationContext *prev_;
    inline static thread_local const GWireSerializationContext *t_current_ = nullptr;
};

/******************************************************************************/

} /* namespace Gem::Courtier */
