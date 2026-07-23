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
#include <expected>
#include <functional>
#include <list>
#include <mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>

// Geneva headers go here
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/unique_ptr.hpp> // Boost (de)serialization of std::unique_ptr (null-aware)

#include "common/GCommonEnums.hpp" // Gem::Common::serializationMode
#include "common/GMemberReflectionT.hpp" // member_desc / load_copy_ptr / cmp_skip (the wire-omitted-ptr descriptor)
#include "common/concurrency/GContentAddressedStoreT.hpp" // the generic store GWireLayoutRegistry specializes

namespace Gem::Courtier {

/******************************************************************************/
/**
 * The transport-side "layout send-once" machinery. A flat genome ships a by-value copy of its
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
    static std::size_t operator()(const GWireLayoutId &id) noexcept {
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
class GWireLayoutRegistry
    : public Gem::Common::Concurrency::GContentAddressedStoreT<GWireLayoutId, std::string, GWirePeerId, GWireLayoutIdHash> {
public:
    using base_type =
        Gem::Common::Concurrency::GContentAddressedStoreT<GWireLayoutId, std::string, GWirePeerId, GWireLayoutIdHash>;

    GWireLayoutRegistry() = default;

    // The generic store supplies the blob store (has / tryGet / put), capacity (setCapacity), per-peer
    // tracking (peerHas / markPeerHas / forgetPeer) and introspection (size / trackedPeers / clear). Only
    // the two domain-named per-peer aliases are added here, matching this header's "layout" vocabulary and
    // the existing call sites; everything else is inherited unchanged.

    /** @brief Whether a peer is known to already hold a given layout (domain alias for base peerHas).
     *  @param peer The peer (session) to query. @param id The layout id. @return true iff recorded. */
    bool peerHasLayout(GWirePeerId peer, const GWireLayoutId &id) const { return peerHas(peer, id); }

    /** @brief Records that a peer now holds a given layout (domain alias for base markPeerHas).
     *  @param peer The peer (session). @param id The layout id the peer now holds. */
    void markPeerHasLayout(GWirePeerId peer, const GWireLayoutId &id) { markPeerHas(peer, id); }
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

    /// Set on the WORKER side: serialising a work item here means returning a processed RESULT to the
    /// server, which still holds the originally-submitted item. The default (lightweight) return then
    /// omits the input parameters/genome and ships only the computed results -- unless the individual
    /// itself requests a full return (it was modified, e.g. by a nested/tiered optimisation). False on
    /// the server side, where serialising means SUBMITTING work (the full genome must travel).
    bool returning = false;

    /** @brief Worker-side cache-miss fetch: given a layout id whose blob is absent locally, performs the
     *  blocking round trip to the server (REQUEST_LAYOUT -> SEND_LAYOUT) and returns the serialized blob on
     *  success, or a std::unexpected carrying the failure reason (timeout, transport error, malformed/empty
     *  reply). The value is always a non-empty blob. Null on the server side (which never fetches). */
    std::function<std::expected<std::string, std::string>(const GWireLayoutId &)> fetch_blob;
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
/**
 * @brief Serialize policy for a "server-side, wire-omitted" optional owned pointer -- an owned aggregate
 * (e.g. an individual's OA scratch) that rides a checkpoint / file but must NOT travel on the wire, so a
 * remote worker neither receives nor can mutate it.
 *
 * Presence is Boost's own smart-pointer null marker (a single NVP), so no separate has-flag is needed:
 * off the wire the pointer is saved as-is (null-or-object); ON the wire it is saved EMPTY (its null marker
 * travels, so the member is omitted while the stream stays self-describing and symmetric).
 *
 * On LOAD an omitted (null) member must leave the target's existing pointer UNTOUCHED -- a freshly
 * constructed target already holds a default owned object, and overwriting it with null would break a
 * non-null invariant the owner may rely on (e.g. an individual's scratch(), dereferenced unguarded). So
 * the load reads into a temporary and adopts it only when non-null; a genuine value from a checkpoint
 * replaces the target, a wire-omitted null leaves it as-is. Compose it into a localMembers_() entry via
 * make_wire_omitted_ptr_member.
 */
struct ser_wire_omitted_ptr {
    /**
     * @brief (De)serialises the pointer, omitting it (saving null) under an active, enabled wire scope; on
     * load an omitted (null) member leaves the target pointer untouched.
     * @tparam Archive The Boost.Serialization archive type
     * @tparam PtrT The smart-pointer member type (e.g. std::unique_ptr<T>)
     * @param ar The archive to (de)serialize through
     * @param name The NVP tag
     * @param ref The pointer member
     */
    template <typename Archive, typename PtrT>
    static void serialize(Archive &ar, const char *name, PtrT &ref) {
        if constexpr(Archive::is_saving::value) {
            const auto *ctx = GWireSerializationScope::current();
            const bool on_wire = (ctx != nullptr) && ctx->enabled;
            if(on_wire) {
                PtrT empty; // a null pointer: on the wire the member is omitted (only its null marker travels)
                ar & boost::serialization::make_nvp(name, empty);
            }
            else {
                ar & boost::serialization::make_nvp(name, ref);
            }
        }
        else {
            PtrT loaded;
            ar & boost::serialization::make_nvp(name, loaded);
            if(loaded) { ref = std::move(loaded); } // omitted -> null -> leave ref (the default) untouched
        }
    }
};

/**
 * @brief Builds a localMembers_() descriptor for a server-side, wire-omitted optional owned pointer.
 *
 * Serialized via ser_wire_omitted_ptr (by value on disk, omitted on the wire), deep-copied on an in-memory
 * load via load_copy_ptr (copy-constructing the pointee), and excluded from comparison (cmp_skip) -- OA
 * scratch is not part of per-individual identity.
 *
 * @tparam T The (deduced) smart-pointer member type
 * @param name The serialization NVP tag
 * @param ref A reference to the pointer member
 * @return A member_desc composing ser_wire_omitted_ptr / load_copy_ptr / cmp_skip
 */
template <typename T>
Gem::Common::member_desc<T, ser_wire_omitted_ptr, Gem::Common::load_copy_ptr, Gem::Common::cmp_skip>
make_wire_omitted_ptr_member(const char *name, T &ref) {
    return Gem::Common::member_desc<T, ser_wire_omitted_ptr, Gem::Common::load_copy_ptr, Gem::Common::cmp_skip>{
        name, ref
    };
}

/******************************************************************************/

} /* namespace Gem::Courtier */
