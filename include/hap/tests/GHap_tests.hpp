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
 * Geneva was started by Dr. Rüdiger Berlich and was later maintained together
 * with Dr. Ariel Garcia under the auspices of Gemfony scientific. For further
 * information on Gemfony scientific, see http://www.gemfomy.eu .
 *
 * The majority of files in Geneva was released under the Apache license v2.0
 * in February 2020.
 *
 * See the NOTICE file in the top-level directory of the Geneva library
 * collection for a list of contributors and copyright information.
 *
 ********************************************************************************/

#pragma once

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard header files go here
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>

// Boost header files go here
#include <boost/test/unit_test.hpp>
#include <boost/shared_ptr.hpp>

// Geneva header files go here
#include "common/GExceptions.hpp"
#include "common/GCommonEnums.hpp"
#include "hap/GRandomT.hpp"
#include "hap/GRandomDistributionsT.hpp"

using namespace Gem::Hap;

using boost::unit_test_framework::test_suite;
using boost::unit_test_framework::test_case;

/********************************************************************************************/
/**
 * The unit tests for this library
 */
class GHap_tests {
public:
	/***********************************************************************************/
	// The default constructor
	GHap_tests():
		nTests_(100000)
		, nSeeds_(100000)
	{ /* empty */ }

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
			BOOST_CHECK(nSeeds_>DEFAULTSEEDVECTORSIZE);

			// Check that we always get different seeds
			seed_type lastSeed;
			BOOST_CHECK_NO_THROW(lastSeed = GRANDOMFACTORY->getSeed());
			for(std::size_t s=0; s<nSeeds_-1; s++) {
				seed_type currentSeed=GRANDOMFACTORY->getSeed();
				BOOST_CHECK(lastSeed != currentSeed);
				lastSeed=currentSeed;
			}
		}

		//------------------------------------------------------------------------------

		{ // Test that uniform_int(min,max) covers the entire range, including the upper boundary in RANDFLAVOURS::RANDOMLOCAL mode
			// A few settings
			const std::int32_t MINRANDOM=-10;
			const std::int32_t MAXRANDOM= 10;

			std::shared_ptr<GRandomT<Gem::Hap::RANDFLAVOURS::RANDOMLOCAL>> gr_ptr(new Gem::Hap::GRandomT<Gem::Hap::RANDFLAVOURS::RANDOMLOCAL>());
			std::uniform_int_distribution<std::int32_t> uniform_int_distribution(MINRANDOM, MAXRANDOM);

			std::vector<std::int32_t> randomHist(21); // 21 positions from -10 to 10

			// Initialize with 0
			for(std::size_t i=0; i<21; i++) randomHist.at(i) = 0;

			for(std::size_t i=0; i<nTests_; i++) {
				std::int32_t randVal;

				// Produce a single random number
				BOOST_CHECK_NO_THROW(randVal = uniform_int_distribution(*gr_ptr));

				// Is it in the allowed range ?
				BOOST_CHECK(randVal >= MINRANDOM && randVal <= MAXRANDOM);

				// Add the value to the vector
				BOOST_CHECK_NO_THROW(randomHist.at(std::size_t(randVal+10)) += 1);
			}

			// Due to the large number of entries, we should have > 0 entries in all positions
			for(std::size_t i=0; i<21; i++) {
				BOOST_CHECK(randomHist.at(i) > 0);
			}
		}

		//------------------------------------------------------------------------------

		{ // Test that uniform_int(min,max) covers the entire range, including the upper boundary in RANDOMPROXY mode
			// A few settings
			const std::int32_t MINRANDOM=-10;
			const std::int32_t MAXRANDOM= 10;

			std::shared_ptr<GRandomT<Gem::Hap::RANDFLAVOURS::RANDOMPROXY>> gr_ptr(new Gem::Hap::GRandomT<Gem::Hap::RANDFLAVOURS::RANDOMPROXY>());
			std::uniform_int_distribution<std::int32_t> uniform_int_distribution(MINRANDOM, MAXRANDOM);

			std::vector<std::int32_t> randomHist(21); // 21 positions from -10 to 10

			// Initialize with 0
			for(std::size_t i=0; i<21; i++) randomHist.at(i) = 0;

			for(std::size_t i=0; i<nTests_; i++) {
				std::int32_t randVal;

				// Produce a single random number
				BOOST_CHECK_NO_THROW(randVal = uniform_int_distribution(*gr_ptr));

				// Is it in the allowed range ?
				BOOST_CHECK(randVal >= MINRANDOM && randVal <= MAXRANDOM);

				// Add the value to the vector
				BOOST_CHECK_NO_THROW(randomHist.at(std::size_t(randVal+10)) += 1);
			}

			// Due to the large number of entries, we should have > 0 entries in all positions
			for(std::size_t i=0; i<21; i++) {
				BOOST_CHECK(randomHist.at(i) > 0);
			}
		}

		//------------------------------------------------------------------------------

		{ // Check that using extreme values for the boundaries of uniform_int(min,max) and producing random numbers doesn't throw in RAMDOMLOCAL mode
			std::shared_ptr<GRandomT<Gem::Hap::RANDFLAVOURS::RANDOMLOCAL>> gr_ptr(new Gem::Hap::GRandomT<Gem::Hap::RANDFLAVOURS::RANDOMLOCAL>());
			std::uniform_int_distribution<std::int32_t> uniform_int_distribution(
				-(std::numeric_limits<std::int32_t>::max)()
				, (std::numeric_limits<std::int32_t>::max)()
			);

			volatile std::int32_t randVal;
			for(std::size_t i=0; i<nTests_; i++) {
				BOOST_CHECK_NO_THROW(randVal = uniform_int_distribution(*gr_ptr));
			}
		}

		//------------------------------------------------------------------------------

		{ // Check that using extreme values for the boundaries of uniform_int(min,max) and producing random numbers doesn't throw in RAMDOMPROXY mode
			std::shared_ptr<GRandomT<Gem::Hap::RANDFLAVOURS::RANDOMPROXY>> gr_ptr(new Gem::Hap::GRandomT<Gem::Hap::RANDFLAVOURS::RANDOMPROXY>());
			std::uniform_int_distribution<std::int32_t> uniform_int_distribution(
					-(std::numeric_limits<std::int32_t>::max)()
					, (std::numeric_limits<std::int32_t>::max)()
			);

			volatile std::int32_t randVal;
			for(std::size_t i=0; i<nTests_; i++) {
				BOOST_CHECK_NO_THROW(randVal = uniform_int_distribution(*gr_ptr));
			}
		}

		//------------------------------------------------------------------------------
	}

	/***********************************************************************************/
	/**
	 * Test features that are expected to fail
	 */
	void failures_expected() {
		{ /* nothing */ }
	}

	/***********************************************************************************/
private:
	const std::size_t nTests_;
	const std::size_t nSeeds_;
};

/********************************************************************************************/
/**
 * This test suite checks as much as possible of the functionality provided
 + by the Hap library. Please also have a look at the histograms created
 * in the "manual" test section.
 */
class GHapSuite: public test_suite
{
public:
	GHapSuite() :test_suite("GHapSuite") {
		// create an instance of the test cases class
		boost::shared_ptr<GHap_tests> instance(new GHap_tests());

		test_case* GRandom_no_failure_expected_test_case = BOOST_CLASS_TEST_CASE(&GHap_tests::no_failure_expected, instance);
		test_case* GRandom_failures_expected_test_case = BOOST_CLASS_TEST_CASE(&GHap_tests::failures_expected, instance);

		add(GRandom_no_failure_expected_test_case);
		add(GRandom_failures_expected_test_case);
	}
};

/********************************************************************************************/
