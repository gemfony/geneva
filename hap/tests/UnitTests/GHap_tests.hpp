/**
 * @file GHap_tests.hpp
 *
 * Tests for the hap library
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

#pragma once

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard header files go here
#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

// Boost header files go here
#include <catch2/catch_test_macros.hpp>

// Geneva header files go here
#include "common/GCommonEnums.hpp"
#include "common/GExceptions.hpp"
#include "hap/GRandomDistributionsT.hpp"
#include "hap/GRandomT.hpp"

using namespace Gem::Hap;

/********************************************************************************************/
/**
 * The unit tests for this library
 */
class GHap_tests {
public:
    /***********************************************************************************/
    // The default constructor
    GHap_tests()
      : n_tests_(100000)
      , n_seeds_(100000) { /* empty */
    }

    /***********************************************************************************/
    /**
	 * Test of features that are expected to work
	 */
    void no_failure_expected() {
        using namespace Gem::Hap;

        //------------------------------------------------------------------------------

        { // Check seeding
            // Check that we are running with more seeds than the amount of
            // pre-fabricated seeds
            CHECK(n_seeds_ > DEFAULTSEEDVECTORSIZE);

            // Check that we always get different seeds
            seed_type lastSeed;
            CHECK_NOTHROW(lastSeed = randomFactory()->getSeed());
            for(std::size_t s = 0; s < n_seeds_ - 1; s++) {
                seed_type currentSeed = randomFactory()->getSeed();
                CHECK(lastSeed != currentSeed);
                lastSeed = currentSeed;
            }
        }

        //------------------------------------------------------------------------------

        { // Test that uniform_int(min,max) covers the entire range, including the upper boundary in randomSource::LOCAL mode
            // A few settings
            constexpr std::int32_t MINRANDOM = -10;
            constexpr std::int32_t MAXRANDOM = 10;

            std::shared_ptr<GRandomT<Gem::Hap::randomSource::LOCAL>> gr_ptr(
                new Gem::Hap::GRandomT<Gem::Hap::randomSource::LOCAL>()
            );
            std::uniform_int_distribution<std::int32_t> uniform_int_distribution(
                MINRANDOM,
                MAXRANDOM
            );

            std::vector<std::int32_t> randomHist(21); // 21 positions from -10 to 10

            // Initialize with 0
            for(std::size_t i = 0; i < 21; i++) {
                randomHist.at(i) = 0;
            }

            for(std::size_t i = 0; i < n_tests_; i++) {
                std::int32_t randVal;

                // Produce a single random number
                CHECK_NOTHROW(randVal = uniform_int_distribution(*gr_ptr));

                // Is it in the allowed range ?
                CHECK((randVal >= MINRANDOM && randVal <= MAXRANDOM));

                // Add the value to the vector
                CHECK_NOTHROW(randomHist.at(std::size_t(randVal + 10)) += 1);
            }

            // Due to the large number of entries, we should have > 0 entries in all positions
            for(std::size_t i = 0; i < 21; i++) {
                CHECK(randomHist.at(i) > 0);
            }
        }

        //------------------------------------------------------------------------------

        { // Test that uniform_int(min,max) covers the entire range, including the upper boundary in QUEUE mode
            // A few settings
            constexpr std::int32_t MINRANDOM = -10;
            constexpr std::int32_t MAXRANDOM = 10;

            std::shared_ptr<GRandomT<Gem::Hap::randomSource::QUEUE>> gr_ptr(
                new Gem::Hap::GRandomT<Gem::Hap::randomSource::QUEUE>()
            );
            std::uniform_int_distribution<std::int32_t> uniform_int_distribution(
                MINRANDOM,
                MAXRANDOM
            );

            std::vector<std::int32_t> randomHist(21); // 21 positions from -10 to 10

            // Initialize with 0
            for(std::size_t i = 0; i < 21; i++) {
                randomHist.at(i) = 0;
            }

            for(std::size_t i = 0; i < n_tests_; i++) {
                std::int32_t randVal;

                // Produce a single random number
                CHECK_NOTHROW(randVal = uniform_int_distribution(*gr_ptr));

                // Is it in the allowed range ?
                CHECK((randVal >= MINRANDOM && randVal <= MAXRANDOM));

                // Add the value to the vector
                CHECK_NOTHROW(randomHist.at(std::size_t(randVal + 10)) += 1);
            }

            // Due to the large number of entries, we should have > 0 entries in all positions
            for(std::size_t i = 0; i < 21; i++) {
                CHECK(randomHist.at(i) > 0);
            }
        }

        //------------------------------------------------------------------------------

        { // Check that using extreme values for the boundaries of uniform_int(min,max) and producing random numbers doesn't throw in RAMDOMLOCAL mode
            std::shared_ptr<GRandomT<Gem::Hap::randomSource::LOCAL>> gr_ptr(
                new Gem::Hap::GRandomT<Gem::Hap::randomSource::LOCAL>()
            );
            std::uniform_int_distribution<std::int32_t> uniform_int_distribution(
                -(std::numeric_limits<std::int32_t>::max)(),
                (std::numeric_limits<std::int32_t>::max)()
            );

            volatile std::int32_t randVal;
            for(std::size_t i = 0; i < n_tests_; i++) {
                CHECK_NOTHROW(randVal = uniform_int_distribution(*gr_ptr));
            }
        }

        //------------------------------------------------------------------------------

        { // Check that using extreme values for the boundaries of uniform_int(min,max) and producing random numbers doesn't throw in RAMDOMPROXY mode
            std::shared_ptr<GRandomT<Gem::Hap::randomSource::QUEUE>> gr_ptr(
                new Gem::Hap::GRandomT<Gem::Hap::randomSource::QUEUE>()
            );
            std::uniform_int_distribution<std::int32_t> uniform_int_distribution(
                -(std::numeric_limits<std::int32_t>::max)(),
                (std::numeric_limits<std::int32_t>::max)()
            );

            volatile std::int32_t randVal;
            for(std::size_t i = 0; i < n_tests_; i++) {
                CHECK_NOTHROW(randVal = uniform_int_distribution(*gr_ptr));
            }
        }

        //------------------------------------------------------------------------------
    }

    /***********************************************************************************/
    /**
	 * Test features that are expected to fail
	 */
    void failures_expected() {
        { /* nothing */
        }
    }

    /***********************************************************************************/
private:
    const std::size_t n_tests_;
    const std::size_t n_seeds_;
};

/********************************************************************************************/
