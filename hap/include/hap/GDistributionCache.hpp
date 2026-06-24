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
#include <algorithm>
#include <concepts>
#include <cstddef>
#include <utility>
#include <vector>

// Geneva headers go here
#include "hap/GRandomBase.hpp"
#include "hap/GRandomDistributionsT.hpp"

namespace Gem::Hap {

/******************************************************************************/
/**
 * @brief A per-consumer prefetch cache that moves a distribution's transform off the hot path.
 *
 * Random-number demand in an evolutionary algorithm is bursty: a burst of draws per generation,
 * then an evaluation gap. The raw-uint64 -> deviate transform of a distribution (for the normal,
 * a sqrt + log per draw) is a hot-spot inside the burst. This cache lets a consumer "subscribe"
 * to a distribution and pre-produce its values during the GAP, so the burst only pops a ready
 * value.
 *
 * It wraps any GRandomBase (every GRandomT proxy is one) and any object that models a C++20
 * RandomNumberDistribution callable as `dist(urbg)` (a std::*_distribution or a Hap g_*
 * distribution). The distribution's parameters are FIXED for the lifetime of the cache, so the
 * cached values are the final deviates. The URBG path is untouched for non-subscribers; using the
 * cache is entirely opt-in.
 *
 * Usage:
 *   GDistributionCacheT<std::exponential_distribution<double>> cache(gr, dist, capacity);
 *   // in the gap:   cache.prefetch(n);
 *   // in the burst: double x = cache();   // pops a pre-produced value (inline-produces on underflow)
 *
 * Not thread-safe: like a GRandomT proxy, one cache belongs to one consumer/thread.
 *
 * @tparam Distribution A RandomNumberDistribution callable as dist(GRandomBase&)
 */
template <typename Distribution>
class GDistributionCacheT {
public:
    using result_type = typename Distribution::result_type;

    /**
     * @param src      The underlying random source (any GRandomT proxy)
     * @param dist     The distribution to pre-produce (its parameters are fixed for the cache's life)
     * @param capacity The maximum number of values buffered
     */
    GDistributionCacheT(Gem::Hap::GRandomBase &src, Distribution dist, std::size_t capacity)
      : src_(src)
      , dist_(std::move(dist))
      , buf_(capacity) { /* nothing */ }

    /**
     * @brief Pre-produces values now (intended for the evaluation gap), topping the buffer up to min(n, capacity).
     *
     * This is where the distribution's transform runs, off the hot path.
     *
     * @param n The number of values that should be ready after the call
     */
    void prefetch(std::size_t n) {
        const std::size_t target = std::min(n, buf_.size());
        while(count_ < target) {
            buf_[(head_ + count_) % buf_.size()] = dist_(src_);
            ++count_;
        }
    }

    /**
     * @brief Returns the next value: a pre-produced one if available, else produced inline.
     *
     * The inline fallback (on an empty buffer) keeps the cache always correct and never blocking;
     * it just forfeits the prefetch benefit for that draw.
     *
     * @return The next distribution value
     */
    result_type operator()() {
        if(count_ == 0) {
            return dist_(src_); // underflow: never blocks, never wrong
        }
        const result_type v = buf_[head_];
        head_                = (head_ + 1) % buf_.size();
        --count_;
        return v;
    }

    /** @return The number of pre-produced values currently buffered. */
    [[nodiscard]] std::size_t ready() const { return count_; }

private:
    Gem::Hap::GRandomBase   &src_;       ///< the underlying raw source
    Distribution             dist_;      ///< the subscribed distribution (fixed parameters)
    std::vector<result_type> buf_;       ///< ring buffer of pre-produced values
    std::size_t              head_  = 0; ///< index of the next value to pop
    std::size_t              count_ = 0; ///< number of buffered values
};

/******************************************************************************/
/**
 * @brief A prefetch cache for normal deviates whose mean/stddev may vary per draw.
 *
 * The EA mutation step is N(0, sigma) with sigma re-adapted every generation, so the final value
 * cannot be pre-baked. Instead this caches STANDARD normals N(0,1) -- which is exactly the
 * sqrt/log work -- and applies the cheap affine `mean + stddev*z` at consume time. It therefore
 * moves the expensive transform into the gap while still letting each draw choose its own mean and
 * standard deviation. (This standardize-then-scale idea generalizes to any location-scale family;
 * normal is the first concrete member.)
 *
 * Note: because the standard normals are drawn here (in the gap) rather than interleaved with the
 * consumer's other inline draws (e.g. a bernoulli gate), the raw-word consumption order differs
 * from the non-prefetched path -- the result is statistically identical but not bit-for-bit
 * reproducible against it.
 *
 * @tparam fp_type The floating-point type of the deviates
 */
template <std::floating_point fp_type = double>
class GNormalCacheT {
public:
    using result_type = fp_type;
    using param_type  = typename Gem::Hap::g_normal_distribution<fp_type>::param_type;

    /**
     * @param src      The underlying random source (any GRandomT proxy)
     * @param capacity The maximum number of standard normals buffered
     */
    GNormalCacheT(Gem::Hap::GRandomBase &src, std::size_t capacity)
      : src_(src)
      , buf_(capacity) { /* nothing */ }

    /**
     * @brief Pre-produces standard normals now (intended for the evaluation gap), up to min(n, capacity).
     *
     * @param n The number of standard normals that should be ready after the call
     */
    void prefetch(std::size_t n) {
        const param_type       standard(fp_type(0), fp_type(1));
        const std::size_t      target = std::min(n, buf_.size());
        while(count_ < target) {
            buf_[(head_ + count_) % buf_.size()] = dist_(src_, standard);
            ++count_;
        }
    }

    /**
     * @brief Returns the next N(mean, stddev) deviate: a pre-produced standard normal scaled, else inline.
     *
     * @param mean   The desired mean
     * @param stddev The desired standard deviation
     * @return A normally distributed deviate
     */
    fp_type operator()(fp_type mean, fp_type stddev) {
        fp_type z; // NOLINT(cppcoreguidelines-init-variables)
        if(count_ == 0) {
            z = dist_(src_, param_type(fp_type(0), fp_type(1))); // underflow: standard normal inline
        }
        else {
            z    = buf_[head_];
            head_ = (head_ + 1) % buf_.size();
            --count_;
        }
        return mean + (stddev * z);
    }

    /** @return The number of pre-produced standard normals currently buffered. */
    [[nodiscard]] std::size_t ready() const { return count_; }

private:
    Gem::Hap::GRandomBase                  &src_;       ///< the underlying raw source
    Gem::Hap::g_normal_distribution<fp_type> dist_;     ///< drives the transform (and its spare deviate)
    std::vector<fp_type>                     buf_;       ///< ring buffer of standard normals
    std::size_t                              head_  = 0; ///< index of the next value to pop
    std::size_t                              count_ = 0; ///< number of buffered values
};

/******************************************************************************/

} /* namespace Gem::Hap */
