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

#include "hap/GQuarantineSource.hpp"
#include "hap/GRandomFactory.hpp" // for randomFactory() / getSeed()
#include "GFillBackend.hpp"       // library-private engine selection (GPU/SIMD/scalar)

#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <thread>
#include <vector>

// The benign race is between the background producer's bulk refill of a pool and
// a (stale) reader still dereferencing that pool. The reads/writes are aligned
// 64-bit, so on the supported architectures (x86-64 / AArch64) every access is a
// single hardware-atomic instruction: a reader observes a clean old-or-new value,
// never a torn half, and both are valid random numbers. It is *formally* still a
// data race (a scheduling delay creates no happens-before edge), so under
// ThreadSanitizer we mark the writing side no_sanitize -- suppressing one side of
// the pair is enough to silence the (benign, by design) report.
#if defined(__SANITIZE_THREAD__) || (defined(__has_feature) && __has_feature(thread_sanitizer))
#define HAP_NO_TSAN __attribute__((no_sanitize("thread")))
#else
#define HAP_NO_TSAN
#endif

namespace Gem::Hap::detail {

namespace {

/******************************************************************************/
// Pool geometry. N_POOLS rotating pools, each holding CHUNKS_PER_POOL spans of
// QUARANTINE_CHUNK_WORDS words. N_POOLS >= 3 gives the quarantine: the producer
// refills a pool only once the claim cursor is a full rotation away from it.
constexpr int         N_POOLS         = 4;
constexpr std::size_t CHUNKS_PER_POOL = 64;
constexpr std::size_t POOL_WORDS      = QUARANTINE_CHUNK_WORDS * CHUNKS_PER_POOL; // 256k words = 2 MiB

// Aligned 64-bit access must be tear-free for the benign race to hold. atomic_ref's
// lock-freedom is the portable proxy for "an aligned uint64_t load/store is a single
// hardware-atomic instruction here"; the architecture gate (x86-64 / AArch64) and
// this assertion together refuse to build the source where that does not hold.
static_assert(std::atomic_ref<std::uint64_t>::is_always_lock_free,
              "GQuarantineSource requires lock-free aligned 64-bit access (its benign race "
              "relies on tear-free old-or-new reads); this platform does not provide it.");

/******************************************************************************/
/**
 * @brief N rotating, bulk-filled pools read in place, kept ahead of the cursor by one producer.
 *
 * A claim atomically advances a global chunk cursor and returns a pointer to that
 * chunk's span inside pool (gen % N_POOLS), where gen is the chunk's pool
 * generation. A single background producer keeps pools filled up to (lead +
 * N_POOLS-1) generations, so the pool the cursor is currently in is never the pool
 * being refilled -- only a stale (descheduled) reader can share a pool with the
 * refiller, and that read is benign (aligned-64-bit, old-or-new). The producer
 * never waits on readers (no reader tracking), so a dormant proxy is harmless.
 */
class GQuarantinePool {
public:
    GQuarantinePool()
      : backend_(static_cast<std::uint64_t>(randomFactory()->getSeed()))
      , pools_(N_POOLS) {
        for(auto &pool : pools_) {
            pool.resize(POOL_WORDS);
        }
        // Fill the initial N generations (pool k holds generation k).
        for(int k = 0; k < N_POOLS; ++k) {
            fillPool(k);
        }
        filled_seq_.store(N_POOLS - 1, std::memory_order_relaxed);
        producer_ = std::thread([this]() { this->produce(); });
    }

    ~GQuarantinePool() {
        stop_.store(true, std::memory_order_relaxed);
        if(producer_.joinable()) {
            producer_.join();
        }
    }

    GQuarantinePool(const GQuarantinePool &) = delete;
    GQuarantinePool(GQuarantinePool &&) = delete;
    GQuarantinePool &operator=(const GQuarantinePool &) = delete;
    GQuarantinePool &operator=(GQuarantinePool &&) = delete;

    /**
     * @brief Atomically claims the next chunk and returns a pointer to its span (read in place).
     *
     * @return Pointer to QUARANTINE_CHUNK_WORDS words inside one of the pools
     */
    const std::uint64_t *claimSpan() {
        const std::uint64_t s   = chunk_seq_.fetch_add(1, std::memory_order_relaxed);
        const std::uint64_t gen = s / CHUNKS_PER_POOL;
        const int           k   = static_cast<int>(gen % N_POOLS);
        const std::size_t   off = (s % CHUNKS_PER_POOL) * QUARANTINE_CHUNK_WORDS;
        return pools_[static_cast<std::size_t>(k)].data() + off;
    }

private:
    /** @brief Bulk-fills pool k from the backend. Marked no_sanitize: this is the write side of
     *         the documented benign race (a stale reader may concurrently dereference pool k). */
    HAP_NO_TSAN void fillPool(int k) {
        backend_.generate(pools_[static_cast<std::size_t>(k)].data(), POOL_WORDS);
    }

    /** @brief Producer loop: keep pools filled up to (lead + N_POOLS-1) generations ahead. */
    void produce() {
        while(not stop_.load(std::memory_order_relaxed)) {
            const std::uint64_t lead   = chunk_seq_.load(std::memory_order_relaxed) / CHUNKS_PER_POOL;
            const std::uint64_t target = lead + static_cast<std::uint64_t>(N_POOLS - 1);
            const std::uint64_t fs     = filled_seq_.load(std::memory_order_relaxed);
            if(fs < target) {
                const std::uint64_t next = fs + 1;
                fillPool(static_cast<int>(next % N_POOLS));
                filled_seq_.store(next, std::memory_order_relaxed);
            }
            else {
                std::this_thread::sleep_for(std::chrono::microseconds(50));
            }
        }
    }

    GFillBackend                            backend_;     ///< the shared bulk source (chosen once)
    std::vector<std::vector<std::uint64_t>> pools_;       ///< N_POOLS rotating pools, read in place
    std::atomic<std::uint64_t>              chunk_seq_{0}; ///< monotonic claim cursor (chunk index)
    std::atomic<std::uint64_t>              filled_seq_{0}; ///< highest pool generation the producer has filled
    std::atomic<bool>                       stop_{false}; ///< shutdown flag for the producer thread
    std::thread                             producer_;    ///< the single refill thread (never waits on readers)
};

/******************************************************************************/

} /* anonymous namespace */

/******************************************************************************/
/**
 * @brief Claims the next span from the process-wide quarantine pool set.
 *
 * The pool set is a function-local static, constructed on the first claim (after
 * the random factory exists) and shared by every QUARANTINE proxy.
 *
 * @return Pointer to a QUARANTINE_CHUNK_WORDS-word span inside a shared pool
 */
const std::uint64_t *quarantineClaimSpan() {
    static GQuarantinePool pool; // thread-safe initialisation (C++ magic statics)
    return pool.claimSpan();
}

/******************************************************************************/

} /* namespace Gem::Hap::detail */
