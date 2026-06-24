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
// delay creates no happens-before edge), so under ThreadSanitizer we mark the
// writing side no_sanitize -- suppressing one side of the pair silences the
// (by-design benign) report without changing code generation.
#if defined(__SANITIZE_THREAD__) || (defined(__has_feature) && __has_feature(thread_sanitizer))
#define HAP_NO_TSAN __attribute__((no_sanitize("thread")))
#else
#define HAP_NO_TSAN
#endif

namespace Gem::Hap::detail {

/******************************************************************************/
/**
 * @brief A lock-free ring of NPools bulk-filled pools, kept ahead of the claim cursor.
 *
 * This is the shared substrate for the STAGED and QUARANTINE sources. A claim
 * atomically advances a global chunk cursor (fetch_add) and maps it to a span of
 * ChunkWords words inside pool (gen % NPools), where gen is the chunk's pool
 * generation. A single background producer keeps pools filled up to (lead +
 * NPools-1) generations ahead, so the pool the cursor is currently in is never
 * the pool being refilled; the producer never waits on readers (no reader
 * tracking), so the design is lock-free and deadlock-free and a dormant proxy is
 * harmless.
 *
 * Two read modes share the same claim:
 *   - claimSpan() returns a pointer for reading in place (QUARANTINE);
 *   - copyChunkInto() copies the chunk out word-by-word into a caller buffer
 *     (STAGED's private double buffer). The copy uses per-word atomic_ref relaxed
 *     loads: on x86-64 / AArch64 those lower to the same plain aligned move (zero
 *     cost), but they keep the copy strictly per-word -- no wide SIMD load that
 *     could straddle the 64-bit granularity the old-or-new guarantee relies on --
 *     and they are not subject to the compiler assuming the memory is immutable.
 *
 * @tparam ChunkWords     Words per claimed chunk
 * @tparam ChunksPerPool  Chunks per pool (pool size = ChunkWords * ChunksPerPool)
 * @tparam NPools         Number of rotating pools (must be >= 3 for the quarantine)
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
    /** @brief Bulk-fills pool k from the backend. Marked no_sanitize: this is the write side of
     *         the documented benign race (a stale reader may concurrently touch pool k). */
    HAP_NO_TSAN void fillPool(int k) {
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
