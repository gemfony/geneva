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
#include <type_traits>
#include <utility>
#include <vector>

// Geneva headers go here
#include "hap/GNormalSource.hpp"
#include "hap/GRandomBase.hpp"
#include "hap/GRandomDistributionsT.hpp"

namespace Gem::Hap {

/******************************************************************************/
/**
 * @brief A per-consumer prefetch cache that moves a distribution's transform off the hot path.
 *
 * Random-number demand in an evolutionary algorithm is @b bursty: each generation issues a burst
 * of draws, then the algorithm spends an evaluation @b gap drawing (almost) nothing. The
 * raw-@f$\texttt{uint64}@f$ @f$\rightarrow@f$ deviate transform of a distribution (for the normal,
 * a @f$\sqrt{\cdot}@f$ + @f$\log@f$ per draw) is a hot-spot @e inside the burst. This cache lets a
 * consumer "subscribe" to a distribution and pre-produce its values during the gap, so the burst
 * only pops a value that is already transformed.
 *
 * It wraps any @c GRandomBase (every @c GRandomT proxy is one) and any object that models a C++20
 * @c RandomNumberDistribution callable as @f$\texttt{dist}(\texttt{urbg})@f$ (a
 * @c std::*_distribution or a Hap @c g_* distribution). The distribution's parameters are @b fixed
 * for the lifetime of the cache, so the cached values are the @e final deviates. The URBG path is
 * untouched for non-subscribers; the cache is entirely opt-in.
 *
 * @par Data structure
 * A fixed-capacity ring buffer with a head index and a live count; @c prefetch() appends at the
 * tail, @c operator() pops from the head.
 * @verbatim
   prefetch(n): fill tail while count < min(n, capacity)        (runs in the GAP)
                        |
                        v
   buf:  [ . . | v v v v v v v | . . ]   capacity slots, count live
                ^head           ^head+count (mod capacity)
                |
                v
   operator(): pop one from head ----> burst consumer    (runs in the BURST)
                |
                +-- if count == 0 (underflow): produce one inline, never blocks
   @endverbatim
 *
 * @par Algorithm
 * Let @f$Q@f$ be the capacity and @f$c@f$ the live count. @c prefetch(n) tops the buffer up to a
 * target @f$t = \min(n, Q)@f$, running the transform @f$t-c@f$ times: while @f$c < t@f$, store
 * @f$\texttt{dist}(\texttt{src})@f$ at @f$(\texttt{head}+c)\bmod Q@f$ and increment @f$c@f$.
 * @c operator() returns @f$\texttt{buf}[\texttt{head}]@f$ and advances
 * @f$\texttt{head}\leftarrow(\texttt{head}+1)\bmod Q@f$, @f$c\leftarrow c-1@f$; on @f$c=0@f$ it
 * falls back to an inline @f$\texttt{dist}(\texttt{src})@f$, so it is always correct and never
 * blocks -- it merely forfeits the prefetch benefit for that one draw. Intended use: call
 * @c prefetch() at the start of the gap, draw through @c operator() during the burst.
 *
 * Usage:
 *   GDistributionCacheT<std::exponential_distribution<double>> cache(gr, dist, capacity);
 *   // in the gap:   cache.prefetch(n);
 *   // in the burst: double x = cache();   // pops a pre-produced value (inline-produces on underflow)
 *
 * Not thread-safe: like a @c GRandomT proxy, one cache belongs to one consumer/thread.
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
 * The EA mutation step is @f$N(0,\sigma)@f$ with @f$\sigma@f$ re-adapted every generation, so the
 * @e final value cannot be pre-baked. This cache instead pre-produces @b standard normals
 * @f$z\sim N(0,1)@f$ -- which is exactly the expensive @f$\sqrt{\cdot}@f$ + @f$\log@f$ work
 * (Marsaglia polar) -- in the gap, and applies the cheap affine
 * @f[
 *   x \;=\; \mu + \sigma\,z, \qquad z \sim N(0,1),
 * @f]
 * at consume time, so each draw still chooses its own mean @f$\mu@f$ and standard deviation
 * @f$\sigma@f$ while the transform itself has already left the burst. The same standardize-then-
 * scale idea generalizes to any @e location-scale family (e.g. unit-rate exponentials scaled by
 * @f$1/\lambda@f$); the normal is the first concrete member.
 *
 * @par Data structure / algorithm
 * Identical ring-buffer mechanics to GDistributionCacheT, but the buffered values are the
 * standardized deviates @f$z@f$: @c prefetch(n) fills the tail with @f$N(0,1)@f$ draws (the
 * @f$\sqrt{\cdot}@f$/@f$\log@f$ work, in the gap); @c operator(mean,stddev) pops a @f$z@f$ and
 * returns @f$\mu+\sigma z@f$, or, on underflow (@f$c=0@f$), draws one standard normal inline and
 * scales it -- never blocking.
 *
 * @par Reproducibility caveat
 * Because the standard normals are drawn here, in the gap, rather than interleaved with the
 * consumer's other inline draws (e.g. a bernoulli gate), the raw-word consumption order differs
 * from the non-prefetched path: the result is statistically identical but @e not bit-for-bit
 * reproducible against it.
 *
 * @par Measured benefit
 * In the GRandomMutationLoad benchmark (which mirrors the EA Gaussian-mutation draw mix), moving
 * the transform into the gap raised in-burst throughput by roughly @f$1.8@f$--@f$2.0\times@f$ on
 * every random source.
 *
 * @tparam fp_type The floating-point type of the deviates
 */
template <std::floating_point fp_type = double>
class GNormalCacheT {
public:
    using result_type = fp_type;
    using param_type  = typename Gem::Hap::g_normal_distribution<fp_type>::param_type;

    /**
     * @brief CPU per-value mode: the standard normals are transformed on the CPU from @p src.
     *
     * @param src      The underlying random source (any GRandomT proxy)
     * @param capacity The maximum number of standard normals buffered
     */
    GNormalCacheT(Gem::Hap::GRandomBase &src, std::size_t capacity)
      : src_(&src)
      , buf_(capacity) { /* nothing */ }

    /**
     * @brief Bulk mode: the standard normals are produced by generateStandardNormals (GPU-native when
     *        available), with no per-proxy source. prefetch() then fills a whole chunk per call.
     *
     * @param capacity The maximum number of standard normals buffered
     */
    explicit GNormalCacheT(std::size_t capacity)
      : src_(nullptr)
      , buf_(capacity) { /* nothing */ }

    /**
     * @brief Pre-produces standard normals now (intended for the evaluation gap), up to min(n, capacity).
     *
     * In bulk mode the buffer is filled in contiguous runs by generateStandardNormals (one call per
     * run -- batched, and GPU-offloaded where a device is present); in CPU mode each is transformed
     * one at a time from the source.
     *
     * @param n The number of standard normals that should be ready after the call
     */
    void prefetch(std::size_t n) {
        const std::size_t target = std::min(n, buf_.size());
        while(count_ < target) {
            const std::size_t tail = (head_ + count_) % buf_.size();
            const std::size_t run  = std::min(target - count_, buf_.size() - tail); // contiguous
            if(src_ == nullptr) {
                fillBulk(buf_.data() + tail, run);
            }
            else {
                const param_type standard(fp_type(0), fp_type(1));
                for(std::size_t i = 0; i < run; ++i) {
                    buf_[tail + i] = dist_(*src_, standard);
                }
            }
            count_ += run;
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
            if(src_ != nullptr) {
                z = dist_(*src_, param_type(fp_type(0), fp_type(1))); // CPU underflow: inline
            }
            else {
                fillBulk(&z, 1); // bulk underflow: a single deviate (rare)
            }
        }
        else {
            z     = buf_[head_];
            head_ = (head_ + 1) % buf_.size();
            --count_;
        }
        return mean + (stddev * z);
    }

    /** @return The number of pre-produced standard normals currently buffered. */
    [[nodiscard]] std::size_t ready() const { return count_; }

private:
    /** @brief Fills run standard normals at dst via generateStandardNormals (double-native; converts for float). */
    static void fillBulk(fp_type *dst, std::size_t run) {
        if constexpr (std::is_same_v<fp_type, double>) {
            Gem::Hap::generateStandardNormals(dst, run);
        }
        else {
            std::vector<double> tmp(run);
            Gem::Hap::generateStandardNormals(tmp.data(), run);
            for(std::size_t i = 0; i < run; ++i) {
                dst[i] = static_cast<fp_type>(tmp[i]);
            }
        }
    }

    Gem::Hap::GRandomBase                   *src_;       ///< raw source (CPU mode); nullptr in bulk mode
    Gem::Hap::g_normal_distribution<fp_type> dist_;      ///< drives the CPU transform (and its spare deviate)
    std::vector<fp_type>                     buf_;       ///< ring buffer of standard normals
    std::size_t                              head_  = 0; ///< index of the next value to pop
    std::size_t                              count_ = 0; ///< number of buffered values
};

/******************************************************************************/

} /* namespace Gem::Hap */
