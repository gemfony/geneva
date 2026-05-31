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


/********************************************************************************
 *
 * GXoshiro256pp.hpp — local xoshiro256++ pseudo-random generator for Hap2.
 *
 * xoshiro256++ 1.0 by David Blackman and Sebastiano Vigna (2018), state-of-the-
 * art for non-cryptographic Monte-Carlo work: 256-bit state, passes BigCrush,
 * ~no failures. The reference implementation is released into the PUBLIC DOMAIN
 * (CC0): http://prng.di.unimi.it/xoshiro256plusplus.c . This file is an
 * independent C++ re-implementation of that algorithm (no vendored code),
 * wrapped as a std::uniform_random_bit_generator so it can be dropped in as
 * Hap2's G_CPU_BASE_GENERATOR. Seeding expands a single 64-bit seed via
 * splitmix64 (http://prng.di.unimi.it/splitmix64.c , also CC0).
 *
 * This file is part of the Geneva library collection. Apache License 2.0
 * applies to this wrapper. See the NOTICE file in the top-level directory.
 *
 ********************************************************************************/

#pragma once

#include <bit>
#include <cstdint>
#include <limits>

namespace Gem::Hap2 {

/******************************************************************************/
/**
 * xoshiro256++ — a std::uniform_random_bit_generator producing 64-bit values.
 * Drop-in for std::mt19937_64 (same result_type, min()/max(), operator(),
 * seedable from a single 64-bit value, default-constructible).
 */
class xoshiro256pp {
public:
    using result_type = std::uint64_t;

    /** @brief Default construction uses a fixed non-zero seed. */
    xoshiro256pp() noexcept { seed(default_seed); }

    /** @brief Construction from a single 64-bit seed (expanded via splitmix64). */
    explicit xoshiro256pp(result_type s) noexcept { seed(s); }

    static constexpr result_type(min)() noexcept { return 0; }
    static constexpr result_type(max)() noexcept {
        return (std::numeric_limits<result_type>::max)();
    }

    /** @brief (Re-)seed the 256-bit state from a single 64-bit value. */
    void seed(result_type s) noexcept {
        std::uint64_t sm = s;
        for (auto &word : s_) word = splitmix64(sm);
    }

    /** @brief Produce the next 64-bit value. */
    result_type operator()() noexcept {
        const std::uint64_t result = rotl(s_[0] + s_[3], 23) + s_[0];
        const std::uint64_t t      = s_[1] << 17;
        s_[2] ^= s_[0];
        s_[3] ^= s_[1];
        s_[1] ^= s_[2];
        s_[0] ^= s_[3];
        s_[2] ^= t;
        s_[3] = rotl(s_[3], 45);
        return result;
    }

    /** @brief Advance the state by z steps (UniformRandomBitGenerator nicety). */
    void discard(unsigned long long z) noexcept {
        while (z-- > 0) (void)(*this)();
    }

private:
    static constexpr result_type default_seed = 0x9e3779b97f4a7c15ULL;

    static std::uint64_t rotl(std::uint64_t x, int k) noexcept {
        return std::rotl(x, k);
    }

    // splitmix64 — used only to expand the seed into the 256-bit state.
    static std::uint64_t splitmix64(std::uint64_t &x) noexcept {
        std::uint64_t z = (x += 0x9e3779b97f4a7c15ULL);
        z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
        z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
        return z ^ (z >> 31);
    }

    std::uint64_t s_[4]{};
};

/******************************************************************************/

} /* namespace Gem::Hap2 */
