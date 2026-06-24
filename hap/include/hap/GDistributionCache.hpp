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
#include <cstddef>
#include <utility>
#include <vector>

// Geneva headers go here
#include "hap/GRandomBase.hpp"
#include "hap/GRandomDistributionsT.hpp"

namespace Gem::Hap {

/******************************************************************************/
/**
 * @brief A self-sizing, per-consumer prefetch cache that moves a distribution's transform off the hot path.
 *
 * Random-number demand in an evolutionary algorithm is @b bursty: each generation issues a burst
 * of draws, then the algorithm spends an evaluation @b gap drawing (almost) nothing. The
 * raw-@f$\texttt{uint64}\rightarrow@f$ deviate transform of a distribution (for the normal, a
 * @f$\sqrt{\cdot}@f$ + @f$\log@f$ per draw via Marsaglia polar) is a hot-spot @e inside the burst.
 * This cache pre-produces the distribution's values during the gap, so the burst only pops a value
 * that is already transformed. It pulls raw @f$\texttt{uint64}@f$ from whatever backend the supplied
 * proxy uses -- it never knows or cares which (queue, rotating pool, local).
 *
 * It works with any object that models a C++20 @c RandomNumberDistribution callable as
 * @f$\texttt{dist}(\texttt{urbg})@f$ (a @c std::*_distribution or a Hap @c g_* distribution); the
 * distribution's parameters are @b fixed for the cache's life, so the cached values are the @e final
 * deviates. For a @e location-scale family whose parameters vary per draw (the EA's
 * @f$N(0,\sigma)@f$ value step, with @f$\sigma@f$ re-adapted every generation), subscribe to the
 * @e standardized form (@f$N(0,1)@f$) and let the consumer apply the cheap affine
 * @f$x=\mu+\sigma z@f$ at draw time -- the expensive @f$\sqrt{\cdot}@f$/@f$\log@f$ still leaves the
 * burst, and one template serves every distribution.
 *
 * The proxy is passed to @c prefetch()/@c operator() rather than stored, so the cache is safe to
 * hold @b as a member of a copyable owner (e.g. an individual): a copy carries no dangling
 * reference, and copy/assignment yield a fresh, empty cache (it never duplicates pre-produced
 * numbers, which would correlate streams).
 *
 * @par Data structure
 * A ring buffer of @f$Q@f$ slots (capacity) with a head index and a live count @f$c@f$, plus a
 * one-bit underflow flag. @c prefetch() appends at the tail, @c operator() pops from the head.
 * @verbatim
   prefetch(src):  if underflowed last turn -> Q *= 2 (double);  then fill tail while c < Q
                          |
                          v
   buf:  [ . . | z z z z z z z | . . ]   Q slots, c live (fresh)
                ^head           ^(head+c) mod Q
                |
                v
   operator(src):  pop one from head ----> burst consumer
                          |
                          +-- if c == 0 (underflow): produce one inline + set the underflow flag
   @endverbatim
 *
 * @par Algorithm (self-sizing)
 * Let @f$Q@f$ be the capacity, @f$c@f$ the live count, and @f$u\in\{0,1\}@f$ the underflow flag.
 *  - @c operator(src) draws one value. If @f$c>0@f$ it pops @f$\texttt{buf}[\texttt{head}]@f$ and
 *    advances @f$\texttt{head}\leftarrow(\texttt{head}+1)\bmod Q@f$, @f$c\leftarrow c-1@f$. If
 *    @f$c=0@f$ it produces @f$\texttt{dist}(\texttt{src})@f$ @e inline (never blocking, never wrong)
 *    and sets @f$u\leftarrow 1@f$.
 *  - @c prefetch(src), called once per gap, first grows on demand and then tops up:
 *    @f[
 *      \text{if } u:\quad Q \leftarrow 2Q,\; u \leftarrow 0;
 *      \qquad\text{then while } c<Q:\; \texttt{buf}[(\texttt{head}+c)\bmod Q]\leftarrow
 *      \texttt{dist}(\texttt{src}),\; c \leftarrow c+1 .
 *    @f]
 * Two consequences make this need @b no demand counter and converge fast:
 *  - @b Top-up only. @c prefetch regenerates exactly @f$Q-c@f$ values -- the amount consumed since
 *    the last call -- and keeps the @f$c@f$ unused ones (pre-generated randoms never go stale), so
 *    no work is wasted.
 *  - @b Doubling to fit. Each generation whose burst out-draws the buffer raises @f$u@f$, so the
 *    next @c prefetch doubles @f$Q@f$. Starting from a modest @f$Q_0@f$ (1000), the capacity reaches
 *    the steady demand @f$D@f$ in @f$\lceil\log_2(D/Q_0)\rceil@f$ generations -- a handful -- after
 *    which @f$Q>D@f$ permanently, so @f$c>0@f$ every turn, @f$u@f$ stays @f$0@f$, and it never grows
 *    or underflows again. During the brief warm-up the over-draws use the inline fallback (== the
 *    non-prefetched cost), and growth happens while the buffer is empty (@f$c=0@f$ at underflow),
 *    so the reallocation copies nothing. The user therefore declares no size -- the cache learns it.
 *
 * @par Reproducibility caveat
 * Because the values are drawn here, in the gap, rather than interleaved with the consumer's other
 * inline draws, the raw-word consumption order differs from the non-prefetched path: the result is
 * statistically identical but @e not bit-for-bit reproducible against it.
 *
 * Not thread-safe: like a @c GRandomT proxy, one cache belongs to one consumer.
 *
 * @tparam Distribution A RandomNumberDistribution callable as dist(GRandomBase&)
 */
template <typename Distribution>
class GRNGDistributionCacheT {
public:
    using result_type = typename Distribution::result_type;

    /** @brief The default starting capacity (self-corrected on underflow). */
    static constexpr std::size_t DEFAULT_INITIAL_CAPACITY = 1000;

    /**
     * @param dist     The distribution to pre-produce (parameters fixed for the cache's life)
     * @param capacity The starting capacity; doubled on underflow until the burst fits
     */
    explicit GRNGDistributionCacheT(Distribution dist,
                                    std::size_t capacity = DEFAULT_INITIAL_CAPACITY)
      : dist_(std::move(dist))
      , buf_(capacity == 0 ? DEFAULT_INITIAL_CAPACITY : capacity) { /* nothing */ }

    /** @brief Copy yields a fresh, EMPTY cache of the same capacity (never duplicates buffered values). */
    GRNGDistributionCacheT(const GRNGDistributionCacheT &cp)
      : dist_(cp.dist_)
      , buf_(cp.buf_.size()) { /* head_/count_/underflowed_ default to empty */
    }

    /** @brief Move transfers the buffer (the moved-from cache is left valid but unspecified). */
    GRNGDistributionCacheT(GRNGDistributionCacheT &&) noexcept = default;

    /** @brief Copy-assignment resets to a fresh, EMPTY cache of the source's capacity. */
    GRNGDistributionCacheT &operator=(const GRNGDistributionCacheT &cp) {
        if(this != &cp) {
            dist_ = cp.dist_;
            buf_.assign(cp.buf_.size(), result_type{});
            head_        = 0;
            count_       = 0;
            underflowed_ = false;
        }
        return *this;
    }

    GRNGDistributionCacheT &operator=(GRNGDistributionCacheT &&) noexcept = default;
    ~GRNGDistributionCacheT()                                            = default;

    /**
     * @brief Pre-produces values now (intended for the evaluation gap): grow-if-needed, then top up.
     *
     * Doubles the capacity if the previous burst underflowed, then refills the buffer to capacity --
     * regenerating only the values consumed since the last call. The transform runs here, off the burst.
     *
     * @param src The random source (this consumer's proxy) the values are transformed from
     */
    void prefetch(Gem::Hap::GRandomBase &src) {
        if(underflowed_) {
            // The buffer is empty at underflow (count_ == 0), so the copy loop moves nothing; it is
            // written generally so the invariant is not load-bearing.
            std::vector<result_type> grown(buf_.size() * 2);
            for(std::size_t i = 0; i < count_; ++i) {
                grown[i] = buf_[(head_ + i) % buf_.size()];
            }
            buf_         = std::move(grown);
            head_        = 0;
            underflowed_ = false;
        }
        while(count_ < buf_.size()) {
            buf_[(head_ + count_) % buf_.size()] = dist_(src);
            ++count_;
        }
    }

    /**
     * @brief Returns the next value: a pre-produced one if available, else produced inline from @p src.
     *
     * The inline fallback (empty buffer) keeps the cache always correct and never blocking, and flags
     * that the buffer was too small so the next prefetch() grows it.
     *
     * @param src The random source to fall back to on underflow
     * @return The next distribution value
     */
    result_type operator()(Gem::Hap::GRandomBase &src) {
        if(count_ == 0) {
            underflowed_ = true;
            return dist_(src); // never blocks, never wrong; triggers a grow next prefetch()
        }
        const result_type v = buf_[head_];
        head_                = (head_ + 1) % buf_.size();
        --count_;
        return v;
    }

    /** @return The number of pre-produced values currently buffered. */
    [[nodiscard]] std::size_t ready() const { return count_; }
    /** @return The current capacity (grows by doubling until the burst fits). */
    [[nodiscard]] std::size_t capacity() const { return buf_.size(); }

private:
    Distribution             dist_;             ///< the subscribed distribution (fixed parameters)
    std::vector<result_type> buf_;              ///< ring buffer of pre-produced values
    std::size_t              head_        = 0;  ///< index of the next value to pop
    std::size_t              count_       = 0;  ///< number of buffered (fresh) values
    bool                     underflowed_ = false; ///< set when operator() fell back inline -> grow next prefetch
};

/******************************************************************************/

} /* namespace Gem::Hap */
