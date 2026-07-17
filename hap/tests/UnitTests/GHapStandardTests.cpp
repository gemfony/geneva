/**
 * @file GHapStandardTests.cpp
 */

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

#include <thread>

#include <catch2/catch_test_macros.hpp>

#include "GHap_tests.hpp"
#include "hap/GRandomDefines.hpp"

TEST_CASE_METHOD(GHap_tests, "GHap no_failure_expected", "[hap][standard]") {
    no_failure_expected();
}

TEST_CASE_METHOD(GHap_tests, "GHap failures_expected", "[hap][standard][failures-expected]") {
    failures_expected();
}

// PRE-2: a request of 0 producer threads must auto-size to the hardware (not fall back to a fixed 2).
TEST_CASE("GHap autoProducerThreadCount is hardware-derived and clamped", "[hap][standard]") {
    const std::uint16_t n = Gem::Hap::autoProducerThreadCount();

    // Always within the documented clamp, and never zero (so producers actually start).
    REQUIRE(n >= Gem::Hap::DEFAULT01PRODUCERTHREADS);
    REQUIRE(n <= Gem::Hap::MAXAUTOPRODUCERTHREADS);

    const unsigned int hw = std::thread::hardware_concurrency();
    if(hw == 0) {
        // Core count undeterminable -> the fixed fallback.
        REQUIRE(n == Gem::Hap::DEFAULT01PRODUCERTHREADS);
    }
    else {
        // Exactly the clamp(hw / divisor, floor, cap) the helper documents.
        const unsigned int scaled = std::max<unsigned int>(
            Gem::Hap::DEFAULT01PRODUCERTHREADS, hw / Gem::Hap::PRODUCERTHREADS_HW_DIVISOR);
        const std::uint16_t expected =
            static_cast<std::uint16_t>(std::min<unsigned int>(scaled, Gem::Hap::MAXAUTOPRODUCERTHREADS));
        REQUIRE(n == expected);
    }
}
