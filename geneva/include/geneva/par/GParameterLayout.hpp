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
#include <cstdint>
#include <vector>

// Boost headers go here
#include <boost/serialization/access.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/vector.hpp>

namespace Gem::Geneva::Parameters {

/******************************************************************************/
/**
 * The "kind" of a single flat parameter slot. Plain parameters expose their
 * value directly; constrained parameters keep an internal (unconstrained) value
 * that is folded into [lower, upper) by a transfer function on the way out.
 *
 * NOTE: In the current (Phase-0) state of the flat-genome work, GFlatParameters
 * treats every slot as already-external (Plain) -- constrained transfer is a
 * later phase. The enum is defined now so the layout schema is stable.
 */
enum class SlotKind : std::uint8_t { Plain = 0, Constrained = 1 };

/******************************************************************************/
/**
 * Per-scalar-type schema for the flat genome: one entry per parameter slot of
 * type T. This is the *shared, immutable* description of a problem's parameter
 * structure -- it is identical for every individual of a given problem and is
 * therefore meant to be referenced (shared) rather than copied per individual.
 * Only the actual values live per-individual (see GFlatParameters).
 */
template <typename T>
struct ChannelLayout {
    std::vector<SlotKind> kind;  ///< per-slot kind (Plain / Constrained)
    std::vector<T> lower;        ///< per-slot lower boundary (constraint or init range)
    std::vector<T> upper;        ///< per-slot upper boundary (constraint or init range)

    /** @brief The number of slots in this channel */
    [[nodiscard]] std::size_t size() const {
        return lower.size();
    }

private:
    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        ar &boost::serialization::make_nvp("kind", kind) &
            boost::serialization::make_nvp("lower", lower) &
            boost::serialization::make_nvp("upper", upper);
    }
};

/******************************************************************************/
/**
 * The flat-genome layout: the shared schema describing the parameter structure
 * of an individual, split per scalar type (double / float / int32 / bool). It
 * carries everything that is the *same* for every individual of a problem
 * (slot counts, boundaries, kinds), leaving only the values to GFlatParameters.
 *
 * This is the "Layout" half of the Layout + Genome split described in
 * prompts/2026-06-08-flat-genome-proposal.md.
 */
struct GParameterLayout {
    ChannelLayout<double> d;
    ChannelLayout<float> f;
    ChannelLayout<std::int32_t> i;
    ChannelLayout<bool> b;

private:
    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        ar &boost::serialization::make_nvp("d", d) & boost::serialization::make_nvp("f", f) &
            boost::serialization::make_nvp("i", i) & boost::serialization::make_nvp("b", b);
    }
};

/******************************************************************************/

} /* namespace Gem::Geneva::Parameters */
