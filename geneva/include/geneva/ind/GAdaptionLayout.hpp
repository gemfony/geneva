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
#include <vector>

// Geneva headers go here
#include "common/GCommonMathHelperFunctionsT.hpp"
#include "geneva/GOptimizationEnums.hpp"
#include "geneva/ind/GAdaptionKernels.hpp"

namespace Gem::Geneva::Parameters {

/******************************************************************************/
/**
 * The adaption floating-point type for a parameter type: double parameters adapt in double, float
 * parameters in float, everything else (int / bool) carries a double-typed Gauss config that simply
 * stays unused (those channels are mutated by flip adaptors, added later). This mirrors the tree's
 * adaption_fp_type trait but is defined locally so the layout (in geneva/ind/) does not depend on the
 * parameter-object hierarchy (geneva/par/).
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
 * One adaption group: a contiguous run of values within a single channel that share one evolving
 * adaption state (one Gauss sigma, in the FP case). A group of length 1 mirrors a standalone
 * parameter object; a longer group mirrors a collection. The group owns the *static* adaption config
 * (rates / bounds / mode) and the *seed* values for the per-individual GaussState (which the genome
 * installs into its auxiliary store at construction); the evolving state itself lives in the store,
 * not here, so the layout stays shared and immutable.
 */
template <typename T>
struct GroupSpec {
    std::uint32_t start = 0;       ///< index of the first value of this group within the channel
    std::uint32_t len = 1;         ///< number of values in this group
    bool active = true;            ///< false ⇔ adaptionMode::NEVER (the group is never adapted)
    bool has_gauss = false;        ///< whether a Gauss adaptor is configured (FP groups only)
    GaussConfig<adaption_fp_t<T>> gauss{};        ///< the static Gauss configuration (valid iff has_gauss)
    adaption_fp_t<T> start_sigma = adaption_fp_t<T>(1);   ///< GaussState sigma seed + updateOnStall reset target
    adaption_fp_t<T> start_ad_prob = adaption_fp_t<T>(1); ///< GaussState ad_prob seed + updateOnStall reset target
    T range = T(1);                ///< comparative range for the Gauss step (upper-lower, or the init range)
};

/******************************************************************************/
/**
 * The structural description of one value channel (all parameters of a single type). Per-value:
 * bounds, kind, init range (for randomInit) and an active flag; plus the list of adaption groups
 * tiling the channel. Held by the shared GAdaptionLayout; the per-individual GFlatGenome only stores
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
    std::vector<GroupSpec<T>> groups;///< contiguous groups tiling [0, size())

    std::size_t size() const { return lower.size(); }
};

/******************************************************************************/
/**
 * The shared, immutable per-type structural descriptor of a flat genome (DM §2 metadata). It holds
 * one ChannelLayout per supported value type. A single layout is built once (by GGenomeBuilder, and
 * in turn by a factory) and shared by every individual of a problem via std::shared_ptr<const ...>,
 * so per-individual state is just the value arrays. The layout carries no evolving state; the
 * per-individual, per-group adaption state (sigma, ...) lives in each genome's GAuxiliaryStore.
 */
class GAdaptionLayout {
public:
    ChannelLayout<double> d;        ///< the double channel
    ChannelLayout<float> f;         ///< the float channel
    ChannelLayout<std::int32_t> i;  ///< the int32 channel
    ChannelLayout<bool> b;          ///< the bool channel
};

/******************************************************************************/

} /* namespace Gem::Geneva::Parameters */
