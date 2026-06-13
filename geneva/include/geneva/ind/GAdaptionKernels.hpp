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
#include <span>

// Geneva headers go here
#include "common/GCommonMathHelperFunctionsT.hpp"
#include "geneva/GOptimizationEnums.hpp"
#include "hap/GRandomDistributionsT.hpp"
#include "hap/GRandomT.hpp"

namespace Gem::Geneva::Parameters {

/******************************************************************************/
/**
 * Data-oriented adaption kernels: the mutation mathematics of the tree adaptors, re-expressed as
 * stateless free functions over a (config, state, values) triple. The config is the static, shared
 * part (sigma-adaption rate, bounds, mode, …); the state is the per-individual, per-group evolving
 * part (the current sigma / adaption probability / counter); both are plain POD so they live happily
 * in the shared adaption layout and the per-individual auxiliary store respectively, and the kernel
 * has no dependency on the parameter-object hierarchy.
 *
 * The Gauss kernel mirrors GAdaptorT::adapt(std::vector) + GNumGaussAdaptorT::customAdaptAdaption +
 * GFPGaussAdaptorT::customAdaptions exactly (same draw order, the same ad-prob / sigma self-adaption
 * and the same one-ULP "an adaption that fires always changes the value/sigma" guarantees), so it is
 * statistically identical to the tree adaptor (bit-identical given the same RNG draw sequence).
 *
 * Templated on the adaption floating-point type T (double for double parameters, float for float).
 */

/** @brief Static, shared Gauss-adaption configuration for one group of parameters. */
template <typename T>
struct GaussConfig {
    T sigma_sigma = T(0.8);                   ///< the log-normal self-adaption rate of sigma
    T min_sigma = T(0.001);                   ///< the lower bound of sigma
    T max_sigma = T(2.);                      ///< the upper bound of sigma
    T min_ad_prob = T(0.);                    ///< the lower bound of the adaption probability
    T max_ad_prob = T(1.);                    ///< the upper bound of the adaption probability
    T adapt_ad_prob = T(0.);                  ///< the self-adaption rate of the adaption probability (0 disables)
    T adapt_sigma_prob = T(0.);               ///< probability of a sigma self-adaption when adaption_threshold == 0
    std::uint32_t adaption_threshold = 1;     ///< sigma self-adapts every Nth adaption (0 ⇒ use adapt_sigma_prob)
    adaptionMode mode = adaptionMode::WITHPROBABILITY; ///< always / with-probability / never
};

/** @brief Per-individual, per-group evolving Gauss state. POD: standard-layout, trivially copyable. */
template <typename T>
struct GaussState {
    T sigma = T(1.);                          ///< the current mutation width
    T ad_prob = T(1.);                        ///< the current adaption probability
    std::uint32_t counter = 0;                ///< adaptions since the last sigma self-adaption
};

/******************************************************************************/
/**
 * Adapts one group of values sharing a single GaussState, mirroring the tree adaptor's vector
 * path. Returns the number of values that were actually adapted.
 */
template <typename T>
std::size_t adaptGaussGroup(
    const GaussConfig<T> &cfg,
    GaussState<T> &st,
    std::span<T> values,
    const T &range,
    Gem::Hap::GRandomBase &gr
) {
    // Distribution objects hoisted out of the per-value loop (creating them per value is measurably slower).
    Gem::Hap::g_normal_distribution<T> normal;
    Gem::Hap::g_bernoulli_distribution bernoulli;
    using n_param = typename Gem::Hap::g_normal_distribution<T>::param_type;
    using b_param = Gem::Hap::g_bernoulli_distribution::param_type;

    // 1) Self-adapt the adaption probability once for the whole group (if requested).
    if(cfg.adapt_ad_prob > T(0.)) {
        st.ad_prob *= std::exp(normal(gr, n_param(T(0.), cfg.adapt_ad_prob)));
        Gem::Common::enforceRangeConstraint<T>(
            st.ad_prob,
            cfg.min_ad_prob,
            cfg.max_ad_prob,
            "adaptGaussGroup() / ad_prob"
        );
    }

    // Self-adapts sigma (log-normal step + one-ULP guarantee + range clamp), mirroring
    // GNumGaussAdaptorT::customAdaptAdaption.
    auto self_adapt_sigma = [&]() {
        const T sigma_before = st.sigma;
        const T sigma_mult = std::exp(normal(gr, n_param(T(0.), std::abs(cfg.sigma_sigma))));
        st.sigma *= sigma_mult;
        if(st.sigma == sigma_before) {
            const T dir = (sigma_mult < T(1.)) ? std::numeric_limits<T>::lowest()
                                               : std::numeric_limits<T>::max();
            st.sigma = std::nextafter(sigma_before, dir);
        }
        Gem::Common::enforceRangeConstraint<T>(
            st.sigma,
            cfg.min_sigma,
            cfg.max_sigma,
            "adaptGaussGroup() / sigma",
            false
        );
    };

    // The sigma self-adaption trigger, mirroring GAdaptorT::adaptAdaption.
    auto adapt_adaption = [&]() {
        if(cfg.adaption_threshold > 0) {
            if(++st.counter >= cfg.adaption_threshold) {
                st.counter = 0;
                self_adapt_sigma();
            }
        }
        else if(cfg.adapt_sigma_prob != T(0.)) {
            if(bernoulli(gr, b_param(std::abs(cfg.adapt_sigma_prob)))) {
                self_adapt_sigma();
            }
        }
    };

    // The value step (range * N(0, sigma) + one-ULP guarantee), mirroring GFPGaussAdaptorT::customAdaptions.
    auto gauss_step = [&](T &v) {
        const T before = v;
        const T delta = range * normal(gr, n_param(T(0.), st.sigma));
        v = before + delta;
        if(v == before) {
            const T dir = (delta < T(0.)) ? std::numeric_limits<T>::lowest()
                                          : std::numeric_limits<T>::max();
            v = std::nextafter(before, dir);
        }
    };

    std::size_t n_adapted = 0;
    if(adaptionMode::WITHPROBABILITY == cfg.mode) {
        for(T &v : values) {
            if(bernoulli(gr, b_param(std::abs(st.ad_prob)))) {
                adapt_adaption();
                gauss_step(v);
                ++n_adapted;
            }
        }
    }
    else if(adaptionMode::ALWAYS == cfg.mode) {
        for(T &v : values) {
            adapt_adaption();
            gauss_step(v);
            ++n_adapted;
        }
    }
    // adaptionMode::NEVER: nothing to do.

    return n_adapted;
}

/******************************************************************************/

} /* namespace Gem::Geneva::Parameters */
