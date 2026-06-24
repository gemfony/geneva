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

#include "hap/GStagedSource.hpp"
#include "hap/GRandomFactory.hpp" // for randomFactory() / getSeed()
#include "GFillBackend.hpp"       // library-private engine selection (GPU/SIMD/scalar)

#include <cstddef>
#include <cstdint>
#include <cstring> // for std::memcpy
#include <mutex>
#include <vector>

namespace Gem::Hap::detail {

namespace {

/******************************************************************************/
/**
 * The number of 64-bit words the shared staging pool generates per refill. One
 * GFillBackend::generate() call fills the whole buffer (one large GPU/SIMD bulk
 * call -- the property that makes STAGED a good GPU fit), after which it is
 * handed out STAGED_CHUNK_WORDS at a time. A multiple of STAGED_CHUNK_WORDS, so
 * claims tile the buffer exactly and no tail is ever discarded.
 */
constexpr std::size_t STAGED_POOL_WORDS = STAGED_CHUNK_WORDS * 256; // 256 chunks (= 8 MiB)

/******************************************************************************/
/**
 * @brief The process-wide staging pool: a large, bulk-filled buffer doled out in chunks.
 *
 * A single mutex guards both the refill and the copy-out, so a claiming thread
 * never copies from a region that is concurrently being regenerated -- the
 * design is race-free by construction (no lock-free reader/refiller overlap to
 * reason about). Because every claim hands the caller a private copy and the
 * pool is touched only for the duration of one memcpy, a proxy that goes dormant
 * pins no pool memory.
 *
 * The lock is taken once per STAGED_CHUNK_WORDS draws, so its cost amortises to a
 * negligible fraction of a single random number. (A lock-free atomic-cursor ring
 * is the natural measured optimisation should contention ever show up; it is
 * deliberately not used here because a whole-buffer refill would otherwise race
 * an in-flight copy-out.)
 */
class GStagingPool {
public:
    GStagingPool()
      : backend_(static_cast<std::uint64_t>(randomFactory()->getSeed()))
      , buf_(STAGED_POOL_WORDS)
      , pos_(STAGED_POOL_WORDS) // start exhausted -> the first claim triggers a fill
    { /* nothing */ }

    /**
     * @brief Copies n fresh words out of the pool, refilling it first if it cannot satisfy the claim.
     *
     * @param dst Destination buffer (at least n words)
     * @param n   Number of 64-bit words to copy (n <= STAGED_POOL_WORDS)
     */
    void claim(std::uint64_t *dst, std::size_t n) {
        const std::lock_guard<std::mutex> lock(mutex_);
        if(pos_ + n > buf_.size()) {
            backend_.generate(buf_.data(), buf_.size());
            pos_ = 0;
        }
        std::memcpy(dst, buf_.data() + pos_, n * sizeof(std::uint64_t));
        pos_ += n;
    }

private:
    std::mutex mutex_;            ///< serialises refill + copy-out (the whole race-freedom argument)
    GFillBackend backend_;        ///< the shared bulk source (GPU/SIMD/scalar, chosen once)
    std::vector<std::uint64_t> buf_; ///< the large staging buffer, refilled in one bulk call
    std::size_t pos_;            ///< read cursor into buf_ (== size() means "exhausted, refill")
};

/******************************************************************************/

} /* anonymous namespace */

/******************************************************************************/
/**
 * @brief Copies one chunk of fresh random words out of the process-wide staging pool.
 *
 * The pool is a function-local static, so it is constructed on the first claim
 * (after the random factory exists) and shared by every STAGED proxy.
 *
 * @param dst Start of the destination buffer (at least n words)
 * @param n   Number of 64-bit words to copy out
 */
void stagedClaim(std::uint64_t *dst, std::size_t n) {
    static GStagingPool pool; // thread-safe initialisation (C++ magic statics)
    pool.claim(dst, n);
}

/******************************************************************************/

} /* namespace Gem::Hap::detail */
