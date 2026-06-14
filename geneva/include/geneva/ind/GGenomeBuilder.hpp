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
#include <cstdint>
#include <limits>
#include <memory>
#include <type_traits>
#include <vector>

// Geneva headers go here
#include "geneva/GOptimizationEnums.hpp"
#include "geneva/ind/GAdaptionLayout.hpp"

namespace Gem::Geneva::Parameters {

/******************************************************************************/
/**
 * The product of GGenomeBuilder::build(): the per-individual value arrays plus a handle to the
 * shared, immutable adaption layout. A flat individual is set up by handing this to
 * GFlatGenome::setGenome(). The layout is shared (std::shared_ptr<const>) -- a factory builds it
 * once and every produced individual binds to the same instance, so per-individual cost is only the
 * value-array copy.
 */
struct Genome {
    std::vector<double> dv;        ///< the double channel start values
    std::vector<float> fv;         ///< the float channel start values
    std::vector<std::int32_t> iv;  ///< the int32 channel start values
    std::vector<std::uint8_t> bv;  ///< the bool channel start values (1/0)
    std::shared_ptr<const GAdaptionLayout> layout; ///< the shared structural descriptor
};

/******************************************************************************/
/**
 * A fluent handle to the group of parameters just added to a GGenomeBuilder. It lets the user attach
 * an adaptor and tune the init policy without knowing the storage layout. The handle is a thin view
 * onto the builder's channel + value array; it must not outlive the builder. Returned by the add*
 * methods and meant to be used immediately, e.g.
 *   b.addDouble(0., -10., 10.).gaussAdaptor(0.5, 0.8, 1e-3, 2., 1.);
 */
template <typename T>
class ParamHandle {
public:
    using adfp = adaption_fp_t<T>;

    ParamHandle(ChannelLayout<T> *ch, std::vector<T> *values, std::size_t group_index)
      : ch_(ch)
      , values_(values)
      , gi_(group_index) {
        /* nothing */
    }

    /** @brief Attaches a Gauss adaptor to this group (FP groups). */
    ParamHandle &gaussAdaptor(
        adfp sigma,
        adfp sigma_sigma,
        adfp min_sigma,
        adfp max_sigma,
        adfp ad_prob,
        adfp adapt_ad_prob = adfp(0),
        std::uint32_t adaption_threshold = 1,
        adaptionMode mode = adaptionMode::WITHPROBABILITY
    ) {
        static_assert(
            std::is_floating_point_v<T>,
            "gaussAdaptor() is only available for floating point parameters"
        );
        GroupSpec<T> &g = ch_->groups.at(gi_);
        g.has_gauss = true;
        g.start_sigma = sigma;
        g.start_ad_prob = ad_prob;
        g.gauss.sigma_sigma = sigma_sigma;
        g.gauss.min_sigma = min_sigma;
        g.gauss.max_sigma = max_sigma;
        g.gauss.min_ad_prob = adfp(0);
        g.gauss.max_ad_prob = adfp(1);
        g.gauss.adapt_ad_prob = adapt_ad_prob;
        g.gauss.adaption_threshold = adaption_threshold;
        g.gauss.mode = mode;
        return *this;
    }

    /** @brief Attaches a bi-gaussian adaptor to this group (FP groups). */
    ParamHandle &biGaussAdaptor(
        adfp sigma1,
        adfp sigma_sigma1,
        adfp min_sigma1,
        adfp max_sigma1,
        adfp sigma2,
        adfp sigma_sigma2,
        adfp min_sigma2,
        adfp max_sigma2,
        adfp delta,
        adfp sigma_delta,
        adfp min_delta,
        adfp max_delta,
        adfp ad_prob,
        bool use_symmetric_sigmas = false,
        adfp adapt_ad_prob = adfp(0),
        std::uint32_t adaption_threshold = 1,
        adaptionMode mode = adaptionMode::WITHPROBABILITY
    ) {
        static_assert(
            std::is_floating_point_v<T>,
            "biGaussAdaptor() is only available for floating point parameters"
        );
        GroupSpec<T> &g = ch_->groups.at(gi_);
        g.has_bigauss = true;
        g.start_sigma1 = sigma1;
        g.start_sigma2 = sigma2;
        g.start_delta = delta;
        g.start_ad_prob = ad_prob;
        g.bigauss.sigma_sigma1 = sigma_sigma1;
        g.bigauss.sigma_sigma2 = sigma_sigma2;
        g.bigauss.sigma_delta = sigma_delta;
        g.bigauss.min_sigma1 = min_sigma1;
        g.bigauss.max_sigma1 = max_sigma1;
        g.bigauss.min_sigma2 = min_sigma2;
        g.bigauss.max_sigma2 = max_sigma2;
        g.bigauss.min_delta = min_delta;
        g.bigauss.max_delta = max_delta;
        g.bigauss.min_ad_prob = adfp(0);
        g.bigauss.max_ad_prob = adfp(1);
        g.bigauss.adapt_ad_prob = adapt_ad_prob;
        g.bigauss.adaption_threshold = adaption_threshold;
        g.bigauss.use_symmetric_sigmas = use_symmetric_sigmas;
        g.bigauss.mode = mode;
        return *this;
    }

    /** @brief Attaches a flip adaptor to this group (int32 / bool groups). */
    ParamHandle &flipAdaptor(
        double ad_prob,
        double adapt_ad_prob = 0.,
        double min_ad_prob = 0.,
        double max_ad_prob = 1.,
        adaptionMode mode = adaptionMode::WITHPROBABILITY
    ) {
        static_assert(
            std::is_integral_v<T>,
            "flipAdaptor() is only available for integer and boolean parameters"
        );
        GroupSpec<T> &g = ch_->groups.at(gi_);
        g.has_flip = true;
        g.start_ad_prob = ad_prob;
        g.flip.min_ad_prob = min_ad_prob;
        g.flip.max_ad_prob = max_ad_prob;
        g.flip.adapt_ad_prob = adapt_ad_prob;
        g.flip.mode = mode;
        return *this;
    }

    /** @brief Sets the adaption mode for this group (NEVER ⇒ the group is inactive). */
    ParamHandle &adaptionMode(Gem::Geneva::adaptionMode mode) {
        GroupSpec<T> &g = ch_->groups.at(gi_);
        g.gauss.mode = mode;
        g.bigauss.mode = mode;
        g.flip.mode = mode;
        g.active = (mode != adaptionMode::NEVER);
        for(std::uint32_t k = 0; k < g.len; ++k) {
            ch_->active.at(g.start + k) = g.active ? 1 : 0;
        }
        return *this;
    }

    /** @brief Sets a fixed start value for every parameter of this group. */
    ParamHandle &init(T v) {
        const GroupSpec<T> &g = ch_->groups.at(gi_);
        for(std::uint32_t k = 0; k < g.len; ++k) {
            values_->at(g.start + k) = v;
        }
        return *this;
    }

    /** @brief Sets the random-initialization perimeter for this group. */
    ParamHandle &perimeter(T lo, T hi) {
        const GroupSpec<T> &g = ch_->groups.at(gi_);
        for(std::uint32_t k = 0; k < g.len; ++k) {
            ch_->init_lower.at(g.start + k) = lo;
            ch_->init_upper.at(g.start + k) = hi;
        }
        return *this;
    }

private:
    ChannelLayout<T> *ch_;     ///< the channel this group lives in (owned by the builder)
    std::vector<T> *values_;   ///< the builder's start-value array for this channel
    std::size_t gi_;           ///< the index of this group within the channel
};

/******************************************************************************/
/**
 * The imperative authoring API for a flat genome: the user declares each parameter (or group / array
 * of parameters) once, optionally attaching an adaptor, and calls build() to obtain a Genome (value
 * arrays + shared layout). This replaces the tree's "push_back parameter objects + adaptors" idiom
 * without losing fine-grained control (see the migration plan's mapping table).
 *
 * Three group shapes per type:
 *  - addX(init[,min,max])          : one parameter, its own adaption group (size 1)
 *  - addXGroup(n[,min,max])        : n parameters sharing ONE adaption group (like a collection)
 *  - addXArray(n[,min,max])        : n parameters, each its own adaption group (n groups of 1)
 */
class GGenomeBuilder {
public:
    /***************************************************************************/
    // Double channel

    ParamHandle<double> addDouble(double init, double min, double max) {
        return addOne(layout_.d, dv_, init, min, max, ParamKind::Constrained);
    }
    ParamHandle<double> addDouble(double init) {
        return addOne(layout_.d, dv_, init, 0., 1., ParamKind::Plain);
    }
    ParamHandle<double> addDoubleGroup(std::size_t n, double min, double max) {
        return addGrouped(layout_.d, dv_, n, min, max, ParamKind::Constrained);
    }
    ParamHandle<double> addDoubleArray(std::size_t n, double min, double max) {
        return addArray(layout_.d, dv_, n, min, max, ParamKind::Constrained);
    }

    /***************************************************************************/
    // Float channel

    ParamHandle<float> addFloat(float init, float min, float max) {
        return addOne(layout_.f, fv_, init, min, max, ParamKind::Constrained);
    }
    ParamHandle<float> addFloat(float init) {
        return addOne(layout_.f, fv_, init, 0.f, 1.f, ParamKind::Plain);
    }
    ParamHandle<float> addFloatGroup(std::size_t n, float min, float max) {
        return addGrouped(layout_.f, fv_, n, min, max, ParamKind::Constrained);
    }
    ParamHandle<float> addFloatArray(std::size_t n, float min, float max) {
        return addArray(layout_.f, fv_, n, min, max, ParamKind::Constrained);
    }

    /***************************************************************************/
    // Int32 channel (Gauss adaptor not yet available -- flip adaptor added later)

    ParamHandle<std::int32_t> addInt32(std::int32_t init, std::int32_t min, std::int32_t max) {
        return addOne(layout_.i, iv_, init, min, max, ParamKind::Constrained);
    }
    ParamHandle<std::int32_t> addInt32(std::int32_t init) {
        return addOne(
            layout_.i,
            iv_,
            init,
            std::numeric_limits<std::int32_t>::lowest(),
            std::numeric_limits<std::int32_t>::max(),
            ParamKind::Plain
        );
    }
    ParamHandle<std::int32_t> addInt32Group(std::size_t n, std::int32_t min, std::int32_t max) {
        return addGrouped(layout_.i, iv_, n, min, max, ParamKind::Constrained);
    }
    ParamHandle<std::int32_t> addInt32Array(std::size_t n, std::int32_t min, std::int32_t max) {
        return addArray(layout_.i, iv_, n, min, max, ParamKind::Constrained);
    }

    /***************************************************************************/
    // Bool channel

    ParamHandle<bool> addBool(bool init) {
        return addOne(layout_.b, bvBool_, init, false, true, ParamKind::Plain);
    }
    ParamHandle<bool> addBoolGroup(std::size_t n) {
        return addGrouped(layout_.b, bvBool_, n, false, true, ParamKind::Plain);
    }
    ParamHandle<bool> addBoolArray(std::size_t n) {
        return addArray(layout_.b, bvBool_, n, false, true, ParamKind::Plain);
    }

    /***************************************************************************/
    /** @brief Produces the value arrays + an interned, shared adaption layout. */
    Genome build() const {
        Genome g;
        g.dv = dv_;
        g.fv = fv_;
        g.iv = iv_;
        g.bv.reserve(bvBool_.size());
        for(bool v : bvBool_) {
            g.bv.push_back(v ? std::uint8_t(1) : std::uint8_t(0));
        }
        g.layout = std::make_shared<const GAdaptionLayout>(layout_);
        return g;
    }

    /** @brief Produces the shared layout only (for factories that bind value arrays separately). */
    std::shared_ptr<const GAdaptionLayout> buildLayout() const {
        return std::make_shared<const GAdaptionLayout>(layout_);
    }

private:
    /***************************************************************************/
    // One parameter = one group of size 1.
    template <typename T>
    ParamHandle<T> addOne(
        ChannelLayout<T> &ch,
        std::vector<T> &values,
        T init,
        T min,
        T max,
        ParamKind kind
    ) {
        return addGroupImpl(ch, values, 1, init, min, max, kind, /* grouped = */ true);
    }

    // n parameters sharing one group.
    template <typename T>
    ParamHandle<T> addGrouped(
        ChannelLayout<T> &ch,
        std::vector<T> &values,
        std::size_t n,
        T min,
        T max,
        ParamKind kind
    ) {
        return addGroupImpl(ch, values, n, min, min, max, kind, /* grouped = */ true);
    }

    // n parameters, each its own group of size 1. Returns a handle to the first group.
    template <typename T>
    ParamHandle<T> addArray(
        ChannelLayout<T> &ch,
        std::vector<T> &values,
        std::size_t n,
        T min,
        T max,
        ParamKind kind
    ) {
        const std::size_t first_group = ch.groups.size();
        for(std::size_t k = 0; k < n; ++k) {
            addGroupImpl(ch, values, 1, min, min, max, kind, /* grouped = */ true);
        }
        return ParamHandle<T>(&ch, &values, first_group);
    }

    /***************************************************************************/
    /**
     * Appends one group of `len` values to a channel, extending all per-value vectors and registering
     * the group. `init` seeds the start value of every member; the init perimeter defaults to the
     * bounds (Constrained) or [0,1] (Plain), and can be overridden via the returned handle.
     */
    template <typename T>
    ParamHandle<T> addGroupImpl(
        ChannelLayout<T> &ch,
        std::vector<T> &values,
        std::size_t len,
        T init,
        T min,
        T max,
        ParamKind kind,
        bool /* grouped */
    ) {
        const auto start = static_cast<std::uint32_t>(ch.size());

        const T init_lo = (kind == ParamKind::Constrained) ? min : T(0);
        const T init_hi = (kind == ParamKind::Constrained) ? max : T(1);

        for(std::size_t k = 0; k < len; ++k) {
            ch.lower.push_back(min);
            ch.upper.push_back(max);
            ch.init_lower.push_back(init_lo);
            ch.init_upper.push_back(init_hi);
            ch.kind.push_back(kind);
            ch.active.push_back(1);
            values.push_back(init);
        }

        GroupSpec<T> g;
        g.start = start;
        g.len = static_cast<std::uint32_t>(len);
        g.active = true;
        g.has_gauss = false;
        // The comparative range mirrors the tree: (upper-lower) for constrained, the init range
        // otherwise. Used to scale the Gauss step independently of a parameter's value range. Only
        // meaningful for the FP channels (the only ones with a Gauss adaptor); left at 1 elsewhere.
        if constexpr(std::is_floating_point_v<T>) {
            g.range = (kind == ParamKind::Constrained) ? (max - min) : (init_hi - init_lo);
        }
        else {
            g.range = T(1);
        }

        const std::size_t gi = ch.groups.size();
        ch.groups.push_back(g);
        return ParamHandle<T>(&ch, &values, gi);
    }

    /***************************************************************************/
    // The layout under construction and the matching start-value arrays.
    GAdaptionLayout layout_;
    std::vector<double> dv_;
    std::vector<float> fv_;
    std::vector<std::int32_t> iv_;
    std::vector<bool> bvBool_;
};

/******************************************************************************/

} /* namespace Gem::Geneva::Parameters */
