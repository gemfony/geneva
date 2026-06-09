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
 * The adaption mode of a group, mirroring Gem::Geneva::adaptionMode.
 */
enum class FlatAdaptionMode : std::uint8_t { Never = 0, WithProbability = 1, Always = 2 };

/******************************************************************************/
/**
 * Static (shared, immutable) configuration of a Gauss adaptor for one contiguous run of
 * double slots. This is the "config" half of the data-oriented split: it is identical for
 * every individual of a problem and therefore lives in the shared layout. The evolving
 * per-individual state (sigma, ad_prob, adaption counter) lives in GFlatParameters instead.
 *
 * present == false means the group's double slots carry no (Gauss) adaptor and are not mutated
 * by the flat EA/SA adapt kernel (e.g. a non-Gauss adaptor, which is not yet flattened).
 */
struct FlatGaussConfig {
    bool present = false;            ///< whether a Gauss adaptor was captured for this group
    bool constrained = false;        ///< fold the value back into [lower, upper) after adaption
    std::size_t start = 0;           ///< offset into the double value array
    std::size_t count = 0;           ///< number of slots
    FlatAdaptionMode mode = FlatAdaptionMode::WithProbability;
    double sigma_sigma = 0.;         ///< meta-step for sigma's log-normal self-adaption
    double min_sigma = 0.;
    double max_sigma = 1.;
    double adapt_ad_prob = 0.;       ///< meta-step for ad_prob's log-normal self-adaption
    double min_ad_prob = 0.;
    double max_ad_prob = 1.;
    double adapt_adaption_probability = 0.;
    std::uint32_t adaption_threshold = 0;

private:
    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        ar &boost::serialization::make_nvp("present", present) &
            boost::serialization::make_nvp("constrained", constrained) &
            boost::serialization::make_nvp("start", start) &
            boost::serialization::make_nvp("count", count) &
            boost::serialization::make_nvp("mode", mode) &
            boost::serialization::make_nvp("sigma_sigma", sigma_sigma) &
            boost::serialization::make_nvp("min_sigma", min_sigma) &
            boost::serialization::make_nvp("max_sigma", max_sigma) &
            boost::serialization::make_nvp("adapt_ad_prob", adapt_ad_prob) &
            boost::serialization::make_nvp("min_ad_prob", min_ad_prob) &
            boost::serialization::make_nvp("max_ad_prob", max_ad_prob) &
            boost::serialization::make_nvp("adapt_adaption_probability", adapt_adaption_probability) &
            boost::serialization::make_nvp("adaption_threshold", adaption_threshold);
    }
};

/******************************************************************************/
/**
 * Static (shared, immutable) configuration of a FLIP adaptor for one contiguous run of int/bool
 * slots. Flip adaptors (GInt32FlipAdaptor, GBooleanAdaptor -- the int/bool defaults) carry no
 * sigma; only the flip probability (ad_prob) self-adapts between adaptions. The evolving
 * per-individual state (ad_prob, counter) lives in GFlatParameters.
 */
struct FlatFlipConfig {
    bool present = false;
    bool constrained = false;   ///< int only (folds after the ±1 flip); bool is never constrained
    std::size_t start = 0;
    std::size_t count = 0;
    FlatAdaptionMode mode = FlatAdaptionMode::WithProbability;
    double adapt_ad_prob = 0.;
    double min_ad_prob = 0.;
    double max_ad_prob = 1.;

private:
    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        ar &boost::serialization::make_nvp("present", present) &
            boost::serialization::make_nvp("constrained", constrained) &
            boost::serialization::make_nvp("start", start) &
            boost::serialization::make_nvp("count", count) &
            boost::serialization::make_nvp("mode", mode) &
            boost::serialization::make_nvp("adapt_ad_prob", adapt_ad_prob) &
            boost::serialization::make_nvp("min_ad_prob", min_ad_prob) &
            boost::serialization::make_nvp("max_ad_prob", max_ad_prob);
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

    /// Static adaptor config per channel, one entry per source parameter object. The matching
    /// per-individual evolving state lives in GFlatParameters. double/float use Gauss; int/bool flip.
    std::vector<FlatGaussConfig> d_adaptor_groups;
    std::vector<FlatGaussConfig> f_adaptor_groups;
    std::vector<FlatFlipConfig> i_adaptor_groups;
    std::vector<FlatFlipConfig> b_adaptor_groups;

private:
    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        ar &boost::serialization::make_nvp("d", d) & boost::serialization::make_nvp("f", f) &
            boost::serialization::make_nvp("i", i) & boost::serialization::make_nvp("b", b) &
            boost::serialization::make_nvp("d_adaptor_groups", d_adaptor_groups) &
            boost::serialization::make_nvp("f_adaptor_groups", f_adaptor_groups) &
            boost::serialization::make_nvp("i_adaptor_groups", i_adaptor_groups) &
            boost::serialization::make_nvp("b_adaptor_groups", b_adaptor_groups);
    }
};

/******************************************************************************/

} /* namespace Gem::Geneva::Parameters */
