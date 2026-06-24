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

// NOTE: library-private header (pulls in GFillBackend behind PRIVATE compile
// definitions); only included by gemfony-hap TUs, never installed.

// Geneva headers go here
#include "hap/GRandomFactory.hpp" // for randomFactory() / getSeed()
#include "GFillBackend.hpp"       // library-private engine selection (GPU/SIMD/scalar)

// Standard headers go here
#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <thread>
#include <vector>

// The only shared-memory hazard in this design is between the background
// producer's bulk refill of a pool and a (stale) reader still touching that
// pool. Both accesses are aligned 64-bit, so on the two architectures Geneva
// supports (x86-64 / AArch64) every access is a single hardware-atomic
// instruction: a reader observes a clean old-or-new value, never a torn half,
// and both are valid random numbers. The lock-free design (no producer ever
// waits on a reader) makes it deadlock-free; the N>=3 quarantine makes such an
// overlap reachable only for a reader descheduled across a full pool rotation,
// and even then it is benign. It is *formally* still a data race (a scheduling
// delay creates no happens-before edge), so under ThreadSanitizer we annotate
// the pool STORAGE as a benign race. We annotate the memory (not the writer)
// because the actual write happens inside the SIMD/cuRAND generate() callee,
// which is shared with the QUEUE source and must not be blanket-suppressed --
// a no_sanitize on the refill wrapper would miss it. AnnotateBenignRaceSized
// covers every access (producer write, in-place reader, atomic_ref copy-out) to
// the named range, which is exactly the pool buffer.
#if defined(__SANITIZE_THREAD__) || (defined(__has_feature) && __has_feature(thread_sanitizer))
extern "C" void AnnotateBenignRaceSized(const char *file, int line, const volatile void *mem,
                                        unsigned long size, const char *description);
#define HAP_ANNOTATE_BENIGN_RACE(addr, size, desc) \
    AnnotateBenignRaceSized(__FILE__, __LINE__, (addr), (size), (desc))
#else
#define HAP_ANNOTATE_BENIGN_RACE(addr, size, desc) ((void) 0)
#endif

namespace Gem::Hap::detail {

/******************************************************************************/
/**
 * @brief A lock-free ring of @f$N@f$ bulk-filled pools, kept ahead of an atomic claim cursor.
 *
 * This is the shared substrate for the STAGED and QUARANTINE random sources. Let
 * @f$W=@f$ @c ChunkWords (words per chunk), @f$C=@f$ @c ChunksPerPool (chunks per pool) and
 * @f$N=@f$ @c NPools. Each pool holds @f$C\cdot W@f$ 64-bit words; the @f$N@f$ pools form a ring
 * that a single background producer refills ahead of a monotonic claim cursor.
 *
 * @par Data structure
 * @verbatim
   claim cursor s : atomic<uint64_t>, only ever fetch_add(1)   (chunk index)
                 |
                 |  maps to  pool k = floor(s/C) mod N ,  offset o = (s mod C)*W
                 v
   +===========+===========+===========+===========+
   |  pool 0   |  pool 1   |  pool 2   |  pool 3   |   N rotating pools (here N=4),
   | gen g     | gen g+1   | gen g+2   | gen g-1   |   each = C chunks of W words
   +===========+===========+===========+===========+
        ^  reader is here              ^  producer refills here
        |  (the "lead" pool the        |  (>= N-1 generations AHEAD of the lead;
        |   cursor currently claims     |   i.e. the pool the cursor will reach
        |   chunks from)                |   only after a full ring rotation)
        |                               |
        +------- quarantine gap (>= N-1 pools) keeps these two apart -------+
   @endverbatim
 *
 * @par Algorithm
 * A claim advances the cursor and decodes it (no lock):
 * @f[
 *   s \;\leftarrow\; \texttt{cursor.fetch\_add}(1), \qquad
 *   g = \left\lfloor s/C \right\rfloor, \quad
 *   k = g \bmod N, \quad
 *   o = (s \bmod C)\,W ,
 * @f]
 * yielding the span @f$\texttt{pools}[k]\,[\,o,\,o+W\,)@f$, which holds pool generation @f$g@f$.
 * The background producer keeps the ring filled ahead of the cursor, maintaining the invariant
 * @f[
 *   \texttt{filled\_seq} \;\ge\; \texttt{lead} + (N-1), \qquad
 *   \texttt{lead} = \left\lfloor \texttt{cursor}/C \right\rfloor ,
 * @f]
 * by bulk-filling one pool at a time (pool @f$ (f\!+\!1)\bmod N @f$ for generation @f$f\!+\!1@f$).
 * It @e never inspects or waits on reader state, so a dormant or dead proxy can stall nothing.
 *
 * @par Two read modes (same claim)
 * @li claimSpan() returns a pointer for reading the chunk @e in @e place (QUARANTINE);
 * @li copyChunkInto() copies the chunk out word-by-word into the caller's private buffer
 *     (STAGED's double buffer). The copy uses per-word @c atomic_ref relaxed loads: on x86-64 /
 *     AArch64 each lowers to the same plain aligned move (zero cost), but it keeps the copy
 *     strictly per-word -- no wide SIMD load may straddle the 64-bit granularity the
 *     old-or-new guarantee relies on -- and the compiler may not assume the memory is immutable.
 *
 * @par Benign race condition -- when, and why, it does no harm
 * The single shared-memory hazard is the producer bulk-refilling a pool while a reader still
 * touches it. It is reachable, but harmless, under exactly these conditions:
 * @li @b Atomicity. Every shared cell is an aligned 64-bit word. On the supported architectures
 *     (x86-64, AArch64) an aligned 64-bit load/store is a @e single hardware-atomic instruction,
 *     so a reader observes a clean @b old-or-new value, never a torn half. This is the hard
 *     requirement, enforced portably by @c static_assert(@c atomic_ref<uint64_t>::is_always_lock_free)
 *     below -- a precise property test that fails the compile where it does not hold (e.g. 32-bit).
 * @li @b Separation. With @f$N\ge 3@f$ the producer refills pool
 *     @f$ g_{\text{prod}}\bmod N @f$ with @f$ g_{\text{prod}} \le \texttt{lead}+(N\!-\!1) @f$,
 *     whereas an @e active reader is in pool @f$ \texttt{lead}\bmod N @f$. These differ, so an
 *     active reader and the refiller never touch the same pool. An overlap requires a reader
 *     descheduled long enough for the cursor to advance a @e full rotation,
 *     @f$ \ge (N\!-\!1)\,C @f$ chunks -- astronomically unlikely at realistic sizes.
 * @li @b Benign @b even @b then. Should that overlap occur, the copied/read chunk is a mix of
 *     generations @f$g@f$ and @f$g+N@f$ words; by the atomicity condition every individual word
 *     is a valid random number, so the only effect is mixing two independent draws -- harmless
 *     for an RNG, where determinism is not required.
 * @li @b No @b deadlock. There are no locks, and the producer never blocks on reader state, so a
 *     dormant/dead proxy cannot stall production: lock-free @f$\Rightarrow@f$ deadlock-free.
 *
 * It is @e formally still a data race (a wall-clock delay creates no happens-before edge), so the
 * pool storage is marked with @c AnnotateBenignRaceSized (see the file-top note); a ThreadSanitizer
 * build of the concurrency tests then reports zero races while all assertions pass.
 *
 * @tparam ChunkWords     Words per claimed chunk (@f$W@f$)
 * @tparam ChunksPerPool  Chunks per pool (@f$C@f$; pool size @f$=W\cdot C@f$ words)
 * @tparam NPools         Number of rotating pools (@f$N@f$; must be @f$\ge 3@f$ for the quarantine)
 */
template <std::size_t ChunkWords, std::size_t ChunksPerPool = 64, int NPools = 4>
class GRotatingPool {
    static constexpr std::size_t PoolWords = ChunkWords * ChunksPerPool;
    static_assert(NPools >= 3, "GRotatingPool needs >= 3 pools for the quarantine separation.");

    // Aligned 64-bit access must be tear-free for the benign old-or-new read to hold.
    // atomic_ref's lock-freedom is the portable proxy for "an aligned uint64_t load/store
    // is a single hardware-atomic instruction here"; the architecture gate (x86-64 /
    // AArch64) and this assertion together refuse to build where that does not hold.
    static_assert(std::atomic_ref<std::uint64_t>::is_always_lock_free,
                  "GRotatingPool requires lock-free aligned 64-bit access (its benign race relies "
                  "on tear-free old-or-new reads); this platform does not provide it.");

public:
    GRotatingPool()
      : backend_(static_cast<std::uint64_t>(randomFactory()->getSeed()))
      , pools_(static_cast<std::size_t>(NPools)) {
        for(auto &pool : pools_) {
            pool.resize(PoolWords);
            // Mark the pool storage as an intentional benign race (no-op unless built
            // under ThreadSanitizer); see the note at the top of this file.
            HAP_ANNOTATE_BENIGN_RACE(pool.data(), PoolWords * sizeof(std::uint64_t),
                                     "GRotatingPool: benign refill-vs-stale-reader race (aligned 64-bit)");
        }
        // Fill the initial NPools generations (pool k holds generation k).
        for(int k = 0; k < NPools; ++k) {
            fillPool(k);
        }
        filled_seq_.store(static_cast<std::uint64_t>(NPools - 1), std::memory_order_relaxed);
        producer_ = std::thread([this]() { this->produce(); });
    }

    ~GRotatingPool() {
        stop_.store(true, std::memory_order_relaxed);
        if(producer_.joinable()) {
            producer_.join();
        }
    }

    GRotatingPool(const GRotatingPool &) = delete;
    GRotatingPool(GRotatingPool &&) = delete;
    GRotatingPool &operator=(const GRotatingPool &) = delete;
    GRotatingPool &operator=(GRotatingPool &&) = delete;

    /**
     * @brief Claims the next chunk and returns a pointer to its span for in-place reading.
     *
     * @return Pointer to ChunkWords words inside one of the pools
     */
    const std::uint64_t *claimSpan() {
        const std::uint64_t s   = chunk_seq_.fetch_add(1, std::memory_order_relaxed);
        const std::size_t   k   = static_cast<std::size_t>((s / ChunksPerPool) % NPools);
        const std::size_t   off = (s % ChunksPerPool) * ChunkWords;
        return pools_[k].data() + off;
    }

    /**
     * @brief Claims the next chunk and copies it out word-by-word into dst.
     *
     * Each word is read with an aligned 64-bit atomic load (old-or-new, never
     * torn), so the resulting private copy is a clean snapshot the caller can then
     * serve from without any further pool access.
     *
     * @param dst Destination buffer (at least ChunkWords words)
     */
    void copyChunkInto(std::uint64_t *dst) {
        const std::uint64_t s   = chunk_seq_.fetch_add(1, std::memory_order_relaxed);
        const std::size_t   k   = static_cast<std::size_t>((s / ChunksPerPool) % NPools);
        const std::size_t   off = (s % ChunksPerPool) * ChunkWords;
        std::uint64_t      *src = pools_[k].data() + off;
        for(std::size_t i = 0; i < ChunkWords; ++i) {
            dst[i] = std::atomic_ref<std::uint64_t>(src[i]).load(std::memory_order_relaxed);
        }
    }

private:
    /** @brief Bulk-fills pool k from the backend. The pool storage is annotated as a benign race
     *         (see the constructor / file-top note), so a concurrent stale reader is not flagged. */
    void fillPool(int k) {
        backend_.generate(pools_[static_cast<std::size_t>(k)].data(), PoolWords);
    }

    /** @brief Producer loop: keep pools filled up to (lead + NPools-1) generations ahead. */
    void produce() {
        while(not stop_.load(std::memory_order_relaxed)) {
            const std::uint64_t lead   = chunk_seq_.load(std::memory_order_relaxed) / ChunksPerPool;
            const std::uint64_t target = lead + static_cast<std::uint64_t>(NPools - 1);
            const std::uint64_t fs     = filled_seq_.load(std::memory_order_relaxed);
            if(fs < target) {
                const std::uint64_t next = fs + 1;
                fillPool(static_cast<int>(next % NPools));
                filled_seq_.store(next, std::memory_order_relaxed);
            }
            else {
                std::this_thread::sleep_for(std::chrono::microseconds(50));
            }
        }
    }

    GFillBackend                            backend_;       ///< the shared bulk source (chosen once)
    std::vector<std::vector<std::uint64_t>> pools_;         ///< NPools rotating pools
    std::atomic<std::uint64_t>              chunk_seq_{0};  ///< monotonic claim cursor (chunk index)
    std::atomic<std::uint64_t>              filled_seq_{0}; ///< highest pool generation filled
    std::atomic<bool>                       stop_{false};   ///< shutdown flag for the producer thread
    std::thread                             producer_;      ///< the single refill thread (never waits on readers)
};

/******************************************************************************/

} /* namespace Gem::Hap::detail */
