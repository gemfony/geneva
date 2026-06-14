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

// Standard header files go here
#include <cstddef>
#include <cstdint>
#include <map>
#include <memory>
#include <span>
#include <type_traits>
#include <typeinfo>
#include <vector>

// Geneva headers go here
#include "common/GCommonHelperFunctionsT.hpp"
#include "common/GExceptions.hpp"
#include "geneva/GPersonalityTraits.hpp"

namespace Gem::Geneva::Parameters {

/******************************************************************************/
/** @brief Stable key identifying an auxiliary block (e.g. an FNV hash of "ea.gauss.state") */
using AuxKey = std::uint32_t;

/** @brief Whether an auxiliary POD block holds one record per genome group or one per individual */
enum class AuxScope : std::uint8_t { PerIndividual, PerGroup };

/******************************************************************************/
/**
 * An opaque, memcpy-clonable per-individual block of optimization-algorithm metadata
 * (e.g. EA Gauss state: sigma / adProb / counter, one record per genome group). The store
 * treats it as raw bytes; the OA that installed it knows the POD type and reads it back through
 * a typed span. Standard-layout, trivially-copyable POD only — no pointers / vectors inside the
 * record — so the block is memcpy-clonable and (later) GPU-uploadable.
 */
struct AuxBlock {
    AuxScope               scope = AuxScope::PerGroup; ///< per-group or per-individual
    std::uint32_t          stride = 0;                 ///< bytes per record (== sizeof(POD))
    std::uint32_t          tag = 0;                    ///< hash of the POD type, for a debug sanity check
    std::vector<std::byte> bytes;                      ///< stride * record_count bytes
};

/******************************************************************************/
/** @brief A cheap, stable-per-process type tag used to sanity-check typed POD access (debug only) */
template <typename POD>
inline std::uint32_t auxTypeTag() {
    return static_cast<std::uint32_t>(typeid(POD).hash_code());
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * The unified, per-individual home for optimization-algorithm-owned auxiliary data.
 *
 * It is genome-layout-agnostic: the tree individual (GTreeGenome) and the future flat individual
 * both hold exactly one, so the place where an optimization algorithm stashes its per-individual
 * data is the same regardless of how the genome is stored.
 *
 * It has two channels:
 *  - an OBJECT slot: the personality traits (the one per-individual OA *object*; deep-cloned).
 *  - a POD-block map: opaque, keyed, memcpy-clonable per-group/per-individual metadata (adaptor
 *    state, swarm velocity, gradient, …), typed by the *caller* (the OA).
 *
 * Both channels are OA-scoped: they are owned by whichever optimization algorithm currently holds
 * the individual, persist and evolve across that OA's iterations, and are dropped at the algorithm
 * boundary when the individual is handed to the next (possibly different) OA — an EA's sigma is
 * meaningless to a chained CGD and must not ride along. clearScratch() implements that boundary
 * reset for *both* channels; it is therefore called on hand-over, not per iteration.
 *
 * The two channels differ only in SERIALIZATION, not in scope:
 *  - The personality is part of the individual's serialized/compared identity → driven by the
 *    individual's localMembers() (make_cloneable_member on personalityRef()).
 *  - The POD blocks are transient scratch (DM §5): they are deep-copied on CLONE (so a clone
 *    mid-optimization keeps the live scratch) and dropped by clearScratch(), but they are NOT part
 *    of the genome-mode serialization / the standard compare (a full-checkpoint mode that serialises
 *    them will be added with the transport work). The genome carries the optimization forward; the
 *    per-algorithm scratch restarts when a new OA takes over.
 */
class GAuxiliaryStore {
public:
    /** @brief The default constructor */
    GAuxiliaryStore() = default;

    /** @brief The copy constructor deep-clones the personality and copies the POD blocks */
    GAuxiliaryStore(const GAuxiliaryStore &cp)
      : pods_(cp.pods_) {
        Gem::Common::copyCloneableSmartPointer(cp.personality_, personality_);
    }

    /** @brief The move constructor */
    GAuxiliaryStore(GAuxiliaryStore &&) = default;

    /** @brief The destructor */
    ~GAuxiliaryStore() = default;

    /** @brief Copy assignment deep-clones the personality and copies the POD blocks */
    GAuxiliaryStore &operator=(const GAuxiliaryStore &cp) {
        if(this != &cp) {
            Gem::Common::copyCloneableSmartPointer(cp.personality_, personality_);
            pods_ = cp.pods_;
        }
        return *this;
    }

    /** @brief Move assignment */
    GAuxiliaryStore &operator=(GAuxiliaryStore &&) = default;

    /***************************************************************************/
    // Personality (the per-individual OA object; part of the genome's serialized/compared identity).

    /**
     * Direct access to the personality-traits slot. Returned by reference so the individual's
     * serialization / load / compare machinery (make_cloneable_member) can drive it exactly as it
     * drove the former bare pt_ptr_ member.
     */
    std::shared_ptr<Gem::Geneva::GPersonalityTraits> &personalityRef() {
        return personality_;
    }
    const std::shared_ptr<Gem::Geneva::GPersonalityTraits> &personalityRef() const {
        return personality_;
    }

    /***************************************************************************/
    // POD blocks (per-parameter / per-group OA metadata; transient run-scoped scratch).

    /** @brief Whether a POD block is installed under the given key */
    bool hasAux(AuxKey key) const {
        return pods_.find(key) != pods_.end();
    }

    /**
     * Installs (or replaces) a zero-initialised POD block of record_count records of type POD under
     * the given key. The OA calls this at setup to size its per-group metadata.
     */
    template <typename POD>
    void installAuxBlock(AuxKey key, std::size_t record_count, AuxScope scope = AuxScope::PerGroup) {
        static_assert(
            std::is_trivially_copyable_v<POD> && std::is_standard_layout_v<POD>,
            "GAuxiliaryStore: auxiliary POD blocks must be standard-layout, trivially-copyable"
        );
        AuxBlock b;
        b.scope = scope;
        b.stride = static_cast<std::uint32_t>(sizeof(POD));
        b.tag = auxTypeTag<POD>();
        b.bytes.assign(record_count * sizeof(POD), std::byte{0});
        pods_[key] = std::move(b);
    }

    /** @brief A typed, mutable view over the records of the POD block under key */
    template <typename POD>
    std::span<POD> metaRecords(AuxKey key) {
        AuxBlock &b = fetch(key, sizeof(POD), auxTypeTag<POD>());
        return std::span<POD>(reinterpret_cast<POD *>(b.bytes.data()), b.bytes.size() / sizeof(POD));
    }
    /** @brief A typed, read-only view over the records of the POD block under key */
    template <typename POD>
    std::span<const POD> metaRecords(AuxKey key) const {
        const AuxBlock &b = fetch(key, sizeof(POD), auxTypeTag<POD>());
        return std::span<const POD>(
            reinterpret_cast<const POD *>(b.bytes.data()),
            b.bytes.size() / sizeof(POD)
        );
    }

    /** @brief Typed access to a per-individual (single-record) POD block */
    template <typename POD>
    POD &metaScalar(AuxKey key) {
        return metaRecords<POD>(key)[0];
    }
    template <typename POD>
    const POD &metaScalar(AuxKey key) const {
        return metaRecords<POD>(key)[0];
    }

    /** @brief Copies just the POD blocks from another store (the personality is loaded separately, via localMembers) */
    void copyPodsFrom(const GAuxiliaryStore &src) {
        pods_ = src.pods_;
    }

    /***************************************************************************/
    /**
     * Clears all algorithm-scoped scratch held by this store: the personality traits and the POD
     * blocks. Called at optimization-algorithm boundaries (the incoming algorithm re-installs its own).
     */
    void clearScratch() {
        personality_.reset();
        pods_.clear();
    }

private:
    /***************************************************************************/
    /** @brief Looks up a POD block, sanity-checking its stride and type tag in DEBUG mode */
    AuxBlock &fetch(AuxKey key, std::size_t pod_size, std::uint32_t pod_tag) {
        auto it = pods_.find(key);
#ifdef DEBUG
        verify(it != pods_.end(), key, pod_size, pod_tag, it);
#endif
        return it->second;
    }
    const AuxBlock &fetch(AuxKey key, std::size_t pod_size, std::uint32_t pod_tag) const {
        auto it = pods_.find(key);
#ifdef DEBUG
        verify(it != pods_.end(), key, pod_size, pod_tag, it);
#endif
        return it->second;
    }
#ifdef DEBUG
    template <typename It>
    void verify(bool found, AuxKey key, std::size_t pod_size, std::uint32_t pod_tag, It it) const {
        if(not found) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GAuxiliaryStore::metaRecords(): no aux block under key " << key << '\n'
            );
        }
        if(it->second.stride != pod_size || it->second.tag != pod_tag) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GAuxiliaryStore::metaRecords(): POD type mismatch for key " << key << '\n'
                << "stride " << it->second.stride << " vs sizeof " << pod_size << '\n'
            );
        }
    }
#endif

    /***************************************************************************/
    // Data

    /** @brief The personality traits -- the per-individual, algorithm-scoped OA object */
    std::shared_ptr<Gem::Geneva::GPersonalityTraits> personality_;

    /** @brief Opaque per-group/per-individual OA metadata blocks (adaptor state, …), keyed; transient scratch */
    std::map<AuxKey, AuxBlock> pods_;
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Gem::Geneva::Parameters */
