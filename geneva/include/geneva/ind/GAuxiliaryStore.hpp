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
#include <memory>

// Geneva headers go here
#include "common/GCommonHelperFunctionsT.hpp"
#include "geneva/GPersonalityTraits.hpp"

namespace Gem::Geneva::Parameters {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * The unified, per-individual home for optimization-algorithm-owned auxiliary data.
 *
 * It is genome-layout-agnostic: the tree individual (GTreeGenome) and the future flat
 * individual both hold exactly one of these, so the place where an optimization algorithm
 * stashes its per-individual data is the same regardless of how the genome is stored.
 *
 * Today the store carries the single per-individual OA *object*: the personality traits
 * (the algorithm-scoped scratch that GBaseParChildPersonalityTraits, the swarm traits, ... use).
 * It is deliberately a named container rather than a bare pointer so that the second, POD-shaped
 * channel -- the per-group memcpy-clonable auxiliary blocks that hold adaptor state (sigma /
 * adaption-probability / counter), swarm velocities, gradient/direction vectors, ... for the flat
 * genome -- can join it here without touching the individual's surface. See
 * prompts/2026-06-13-agnostic-individual-data-model-and-api.md (the GAuxiliaryStore design).
 *
 * Copy semantics are deep: copying the store deep-clones its objects (so cloning an individual
 * yields an independent personality), mirroring the cloneable-smart-pointer behaviour the
 * individual previously had for its bare pt_ptr_ member.
 */
class GAuxiliaryStore {
public:
    /** @brief The default constructor */
    GAuxiliaryStore() = default;

    /** @brief The copy constructor deep-clones the stored objects */
    GAuxiliaryStore(const GAuxiliaryStore &cp) {
        Gem::Common::copyCloneableSmartPointer(cp.personality_, personality_);
    }

    /** @brief The move constructor */
    GAuxiliaryStore(GAuxiliaryStore &&) = default;

    /** @brief The destructor */
    ~GAuxiliaryStore() = default;

    /** @brief Copy assignment deep-clones the stored objects */
    GAuxiliaryStore &operator=(const GAuxiliaryStore &cp) {
        if(this != &cp) {
            Gem::Common::copyCloneableSmartPointer(cp.personality_, personality_);
        }
        return *this;
    }

    /** @brief Move assignment */
    GAuxiliaryStore &operator=(GAuxiliaryStore &&) = default;

    /***************************************************************************/
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
    /**
     * Clears all algorithm-scoped scratch held by this store. Today that is the personality
     * traits (reset between optimization algorithms in a Go2 chain). When the per-group POD aux
     * blocks arrive, the scratch-scoped ones are dropped here too, while problem-persistent blocks
     * (e.g. evolved adaptor sigmas, if carried over) would be kept.
     */
    void clearScratch() {
        personality_.reset();
    }

private:
    /***************************************************************************/
    // Data

    /** @brief The personality traits -- the per-individual, algorithm-scoped OA object */
    std::shared_ptr<Gem::Geneva::GPersonalityTraits> personality_;
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Gem::Geneva::Parameters */
