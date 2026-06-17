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
#include <cmath>
#include <cstdint>
#include <limits>
#include <string>
#include <vector>

// Geneva headers go here
#include "common/GCommonMathHelperFunctionsT.hpp"
#include "geneva/GOptimizationEnums.hpp"
#include "geneva/ind/GAdaptionKernels.hpp"

namespace Gem::Geneva::Genome {

/******************************************************************************/
/**
 * The adaption floating-point type for a parameter type: double parameters adapt in double, float
 * parameters in float, everything else (int / bool) carries a double-typed Gauss config that simply
 * stays unused (those channels are mutated by flip adaptors, added later). This mirrors the tree's
 * adaption_fp_type trait but is defined locally so the layout (in geneva/ind/) does not depend on the
 * parameter-object hierarchy (geneva/par/).
 *
 * @tparam T The parameter type whose adaption floating-point type is being selected.
 */
template <typename T> struct adaption_fp { using type = double; };
template <> struct adaption_fp<double> { using type = double; };
template <> struct adaption_fp<float> { using type = float; };
template <typename T> using adaption_fp_t = typename adaption_fp<T>::type;

/******************************************************************************/
/** @brief Whether a value is plain (unbounded) or constrained to a half-open / closed interval. */
enum class ParamKind : std::uint8_t { Plain, Constrained };

/******************************************************************************/
/**
 * The reflecting fold of GConstrainedFPT::transfer(), re-expressed as a free function over (val, lo,
 * hi). It maps an unbounded internal value into the half-open external range [lo, hi). Computed in
 * long double, like the original, to keep the double behaviour bit-for-bit. NaN / infinity are
 * rejected unconditionally (they would silently corrupt the range comparison and every value derived
 * from them), matching the tree.
 *
 * @tparam T The floating-point value type (double or float).
 * @param val The unbounded internal value to fold into range.
 * @param lo The lower (inclusive) boundary of the external range.
 * @param hi The upper (exclusive) boundary of the external range.
 * @return The folded value in the half-open range [lo, hi); lo if the range is collapsed (hi <= lo).
 * @throw geneva_exception if val is NaN or infinite.
 */
template <typename T>
T foldConstrainedFP(const T &val, const T &lo, const T &hi) {
    if(std::isnan(val) || std::isinf(val)) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In foldConstrainedFP(): Error" << '\n'
            << "val is " << (std::isnan(val) ? "NaN" : "infinite") << '\n'
        );
    }

    // A FROZEN parameter has lo == hi (a user may fix a parameter by setting equal bounds). The fold
    // interval [lo, hi) is then empty and the region computation below would divide by (upper - lower) == 0.
    // The only meaningful value of a collapsed range is that single point, so return it directly.
    if(hi <= lo) {
        return lo;
    }

    const long double local_val = Gem::Common::narrow<long double>(val);
    const long double lower = Gem::Common::narrow<long double>(lo);
    const long double upper = Gem::Common::narrow<long double>(hi);

    if(local_val >= lower && local_val < upper) {
        return val;
    }

    // Determine which fold region the value lies in (compare transferFunction.pdf shipped with the
    // tree implementation). floor() is required for correctness with negative values.
    const std::int64_t region =
        Gem::Common::narrow<std::int64_t>(std::floor((local_val - lower) / (upper - lower)));

    long double mapping = 0.L;
    if(region % 2 == 0) { // even region (0, 2, ... or a negative even range): ascending
        mapping = local_val - static_cast<long double>(region) * (upper - lower);
    }
    else { // odd region: descending
        mapping = -local_val + (static_cast<long double>(region - 1) * (upper - lower) + 2 * upper);
    }

    T result = Gem::Common::narrow<T>(mapping);

    // The long-double fold lands strictly inside [lo, hi); a narrowing cast can, at low precision,
    // round onto a boundary. Enforce the half-open contract (a no-op for double in practice).
    if(result < lo) {
        result = lo;
    }
    else if(result >= hi) {
        result = std::nextafter(hi, lo);
    }
    return result;
}

/******************************************************************************/
/**
 * The reflecting fold of GConstrainedIntT::transfer(), re-expressed as a free function over (val, lo,
 * hi). Integer ranges are CLOSED [lo, hi] (both boundaries included), unlike the half-open FP range.
 *
 * @tparam T The integer value type.
 * @param val The unbounded internal value to fold into range.
 * @param lo The lower (inclusive) boundary of the closed external range.
 * @param hi The upper (inclusive) boundary of the closed external range.
 * @return The folded value in the closed range [lo, hi].
 */
template <typename T>
T foldConstrainedInt(const T &val, const T &lo, const T &hi) {
    if(val >= lo && val <= hi) {
        return val;
    }

    const T value_range = hi - lo + T(1); // both boundaries are included
    T mapping = T(0);

    if(val < lo) {
        const T n_below = (lo - (val + T(1))) / value_range;
        mapping = val + (value_range * (n_below + T(1)));
        if(n_below % 2 == 0) {
            mapping = lo + hi - mapping; // revert (descending range)
        }
    }
    else { // val > hi
        const T n_above = (val - hi - T(1)) / value_range;
        mapping = val - (value_range * (n_above + T(1)));
        if(n_above % 2 == 0) {
            mapping = lo + hi - mapping; // revert (descending range)
        }
    }
    return mapping;
}

/******************************************************************************/
/**
 * The STRUCTURE of one adaption group, as held by the shared genome layout: a contiguous run of values
 * within a single channel that form one group (a group of length 1 mirrors a standalone parameter object;
 * a longer group a collection). This is structure ONLY -- the adaptor configuration (Gauss / bi-Gauss /
 * flip rates and seeds) is NOT here; it lives on the OA-owned GAdaptionConfig (a GroupSpec, below). The
 * layout therefore carries no adaption intent at all, staying a pure, shared, immutable structural
 * descriptor. `range` and `active` are structural (the parameter's natural scale, derived from its bounds,
 * and whether it is mutable at all -- adaptionMode::NEVER), so they stay here and are copied into the
 * config when one is built from the genome.
 */
template <typename T>
struct GroupStructure {
    std::uint32_t start = 0;    ///< index of the first value of this group within the channel
    std::uint32_t len = 1;      ///< number of values in this group
    std::int32_t label_id = -1; ///< interned label index into GGenomeLayout::labels (-1 = unlabeled)
    bool active = true;         ///< false ⇔ adaptionMode::NEVER (the group is never adapted / mutated)
    T range = T(1);             ///< comparative range for an adaptor step (upper-lower, or the init range)
};

/******************************************************************************/
/**
 * One adaption group's full configuration, owned by the OA's GAdaptionConfig (NOT by the genome layout).
 * It carries the group's STRUCTURE (mirrored from the layout's GroupStructure: start / len / label_id /
 * active / range) plus the *static* adaption config (rates / bounds / mode) and the *seed* values for the
 * per-individual adaption state (which the OA installs into a slot's auxiliary store). The evolving state
 * itself lives in the store, not here. A GAdaptionConfig is built from a genome (snapshotting the
 * structure) and then authored via its fluent API; the adaptor settings never touch the shared layout.
 */
template <typename T>
struct GroupSpec {
    std::uint32_t start = 0;       ///< index of the first value of this group within the channel
    std::uint32_t len = 1;         ///< number of values in this group
    std::int32_t label_id = -1;    ///< interned label index into GGenomeLayout::labels (-1 = unlabeled)
    bool active = true;            ///< false ⇔ adaptionMode::NEVER (the group is never adapted)
    bool has_gauss = false;        ///< whether a Gauss adaptor is configured (FP groups only)
    GaussConfig<adaption_fp_t<T>> gauss{};        ///< the static Gauss configuration (valid iff has_gauss)
    adaption_fp_t<T> start_sigma = adaption_fp_t<T>(1);   ///< GaussState sigma seed + updateOnStall reset target
    adaption_fp_t<T> start_ad_prob = adaption_fp_t<T>(1); ///< Gauss/BiGauss/Flip ad_prob seed + reset target
    T range = T(1);                ///< comparative range for the Gauss step (upper-lower, or the init range)

    // Bi-gaussian alternative (FP groups only): mutually exclusive with has_gauss for the same group.
    bool has_bigauss = false;      ///< whether a bi-gaussian adaptor is configured (FP groups only)
    BiGaussConfig<adaption_fp_t<T>> bigauss{};            ///< the static bi-gaussian configuration (valid iff has_bigauss)
    adaption_fp_t<T> start_sigma1 = adaption_fp_t<T>(1);  ///< BiGaussState sigma1 seed + reset target
    adaption_fp_t<T> start_sigma2 = adaption_fp_t<T>(1);  ///< BiGaussState sigma2 seed + reset target
    adaption_fp_t<T> start_delta = adaption_fp_t<T>(0.5); ///< BiGaussState delta seed + reset target

    // Flip adaptor (int32 / bool groups only).
    bool has_flip = false;         ///< whether a flip adaptor is configured (int / bool groups only)
    FlipConfig flip{};             ///< the static flip configuration (valid iff has_flip)
};

/******************************************************************************/
/**
 * The structural description of one value channel (all parameters of a single type). Per-value:
 * bounds, kind, init range (for randomInit) and an active flag; plus the list of adaption groups
 * tiling the channel. Held by the shared GGenomeLayout; the per-individual GFlatGenome only stores
 * the value array and a handle to this.
 */
template <typename T>
struct ChannelLayout {
    std::vector<T> lower;            ///< per value: lower boundary (Constrained) / unused (Plain)
    std::vector<T> upper;            ///< per value: upper boundary (Constrained) / unused (Plain)
    std::vector<T> init_lower;       ///< per value: lower random-init boundary
    std::vector<T> init_upper;       ///< per value: upper random-init boundary
    std::vector<ParamKind> kind;     ///< per value: Plain or Constrained
    std::vector<std::uint8_t> active;///< per value: mirrors the owning group's active flag (1/0)
    std::vector<GroupStructure<T>> groups;///< contiguous groups tiling [0, size()) -- STRUCTURE only

    /** @brief The number of values in this channel. @return The per-value array length. */
    std::size_t size() const { return lower.size(); }
};

/******************************************************************************/
/** @brief Identifies one of the four value channels (used to address a group across channels). */
enum class ChannelTag : std::uint8_t { Double, Float, Int32, Bool };

/******************************************************************************/
/** @brief A reference to one adaption group: which channel it lives in + its index within that channel. */
struct GroupRef {
    ChannelTag channel;     ///< the value channel the group belongs to
    std::size_t index;      ///< the group's index within that channel's `groups`
};

/******************************************************************************/
/**
 * The shared, immutable per-type structural descriptor of a flat genome (DM §2 metadata). It holds
 * one ChannelLayout per supported value type. A single layout is built once (by GGenomeBuilder, and
 * in turn by a factory) and shared by every individual of a problem via std::shared_ptr<const ...>,
 * so per-individual state is just the value arrays. The layout carries no evolving state; the
 * per-individual, per-group adaption state (sigma, ...) lives in each genome's GAuxiliaryStore.
 *
 * Optionally, groups carry an interned LABEL: `labels` holds each distinct label string once, and a
 * GroupSpec stores a small integer index into it (GroupSpec::label_id, -1 = unlabeled). Labels are
 * per-GROUP (never per-value) and one-to-many -- a single label can tag many groups (e.g. "position"
 * tags every cx/cy group in an image problem), so a per-label adaption setting applies to all of them
 * while each group keeps its own evolving state. Labels let a (later, OA-owned) adaption config address
 * groups by name; structureless problems carry no labels and address by index.
 */
class GGenomeLayout {
public:
    ChannelLayout<double> d;        ///< the double channel
    ChannelLayout<float> f;         ///< the float channel
    ChannelLayout<std::int32_t> i;  ///< the int32 channel
    ChannelLayout<bool> b;          ///< the bool channel

    std::vector<std::string> labels;///< the interned, distinct group-label strings (label_id indexes this)

    /**
     * @brief Interns a label string, returning its id; an already-present string returns its existing id.
     * @param name The label string to intern.
     * @return The id (index into `labels`) of the interned or pre-existing string.
     */
    std::int32_t internLabel(const std::string &name) {
        const std::int32_t existing = labelId(name);
        if(existing >= 0) {
            return existing;
        }
        labels.push_back(name);
        return static_cast<std::int32_t>(labels.size() - 1);
    }

    /**
     * @brief Resolves a label string to its id, or -1 if it is not interned.
     * @param name The label string to look up.
     * @return The id (index into `labels`) of the string, or -1 if it is not present.
     */
    std::int32_t labelId(const std::string &name) const {
        for(std::size_t k = 0; k < labels.size(); ++k) {
            if(labels[k] == name) {
                return static_cast<std::int32_t>(k);
            }
        }
        return -1;
    }

    /**
     * @brief Resolves a label id to its string; returns "" for -1 / out-of-range.
     * @param id The label id to resolve (index into `labels`).
     * @return A reference to the label string, or a reference to an empty string for -1 / out-of-range.
     */
    const std::string &labelName(std::int32_t id) const {
        static const std::string empty;
        if(id < 0 || static_cast<std::size_t>(id) >= labels.size()) {
            return empty;
        }
        return labels[static_cast<std::size_t>(id)];
    }

    /**
     * @brief Resolves a label string to every group it tags, across all channels (one-to-many).
     * @param name The label string to look up.
     * @return A vector of GroupRef for every group tagged with this label; empty if the label is not interned.
     */
    std::vector<GroupRef> groupsForLabel(const std::string &name) const {
        std::vector<GroupRef> out;
        const std::int32_t id = labelId(name);
        if(id < 0) {
            return out;
        }
        collectGroups(d, ChannelTag::Double, id, out);
        collectGroups(f, ChannelTag::Float, id, out);
        collectGroups(i, ChannelTag::Int32, id, out);
        collectGroups(b, ChannelTag::Bool, id, out);
        return out;
    }

private:
    /**
     * @brief Appends a GroupRef for every group in one channel that carries the given label id.
     * @tparam T The channel's value type.
     * @param ch The channel to scan.
     * @param tag The channel tag recorded in each emitted GroupRef.
     * @param id The label id to match against each group's label_id.
     * @param out The output vector that matching GroupRef entries are appended to.
     */
    template <typename T>
    static void
    collectGroups(const ChannelLayout<T> &ch, ChannelTag tag, std::int32_t id, std::vector<GroupRef> &out) {
        for(std::size_t gi = 0; gi < ch.groups.size(); ++gi) {
            if(ch.groups[gi].label_id == id) {
                out.push_back(GroupRef{tag, gi});
            }
        }
    }
};

/******************************************************************************/

} /* namespace Gem::Geneva::Genome */
