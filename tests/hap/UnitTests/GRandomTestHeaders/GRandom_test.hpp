/**
 * @file GRandom_test.cpp
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt)
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) com
 *
 * This file is part of the Geneva library collection.
 *
 * Geneva was developed with kind support from Karlsruhe Institute of
 * Technology (KIT) and Steinbuch Centre for Computing (SCC). Further
 * information about KIT and SCC can be found at http://www.kit.edu/english
 * and http://scc.kit.edu .
 *
 * Geneva is free software: you can redistribute and/or modify it under
 * the terms of version 3 of the GNU Affero General Public License
 * as published by the Free Software Foundation.
 *
 * Geneva is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with the Geneva library. If not, see <http://www.gnu.org/licenses/>.
 *
 * For further information on Gemfony scientific and Geneva, visit
 * http://www.gemfony.com .
 */

// Standard header files go here
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>

// Boost header files go here

#include <boost/test/unit_test.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/cstdint.hpp>

#ifndef GRANDOM_TEST_HPP_
#define GRANDOM_TEST_HPP_

// Geneva header files go here
#include "common/GExceptions.hpp"
#include "common/GCommonEnums.hpp"
#include "hap/GRandomT.hpp"

using namespace Gem::Hap;

using boost::unit_test_framework::test_suite;
using boost::unit_test_framework::test_case;

/********************************************************************************************/
// The actual unit tests for this class
class GRandom_test {
public:
	/***********************************************************************************/
	// The default constructor
	GRandom_test():
		nTests(100000)
	{ /* empty */ }

	/***********************************************************************************/
	// Test features that are expected to work
	void no_failure_expected() {
		using namespace Gem::Hap;

		//------------------------------------------------------------------------------

		{ // Check seeding
			// Check that we can set and retrieve the current seed. This needs to be the first test
			const boost::uint32_t startSeed = 41;
			bool seedSetSuccess;
			BOOST_CHECK_NO_THROW(seedSetSuccess = GRANDOMFACTORY->setStartSeed(startSeed));
			BOOST_CHECK(seedSetSuccess == true);
			BOOST_CHECK(GRANDOMFACTORY->checkSeedingIsInitialized() == true); // The seeding thread should have been started
			boost::uint32_t testSeed;
			BOOST_CHECK_NO_THROW(testSeed = GRANDOMFACTORY->getStartSeed());
			BOOST_CHECK_MESSAGE(testSeed == startSeed, "testSeed = " << testSeed << ", should be " << startSeed);

			// Retrieve a single seed and check that the seeding thread has started
			boost::uint32_t singleSeed;
			BOOST_CHECK_NO_THROW(singleSeed = GRANDOMFACTORY->getSeed());
			BOOST_CHECK(GRANDOMFACTORY->checkSeedingIsInitialized() == true);
			BOOST_CHECK(singleSeed != startSeed);
		}

		//------------------------------------------------------------------------------

		{ // Test that uniform_int(min,max) covers the entire range, including the upper boundary in RANDOMLOCAL mode
			// A few settings
			const boost::int32_t MINRANDOM=-10;
			const boost::int32_t MAXRANDOM= 10;

			boost::shared_ptr<GRandomT<Gem::Hap::RANDOMLOCAL> > gr_ptr(new Gem::Hap::GRandomT<Gem::Hap::RANDOMLOCAL>());

			std::vector<boost::int32_t> randomHist(21); // 21 positions from -10 to 10

			// Initialize with 0
			for(std::size_t i=0; i<21; i++) randomHist.at(i) = 0;

			for(std::size_t i=0; i<nTests; i++) {
				boost::int32_t randVal;

				// Produce a single random number
				BOOST_CHECK_NO_THROW(randVal = gr_ptr->uniform_int(MINRANDOM, MAXRANDOM));

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
			const boost::int32_t MINRANDOM=-10;
			const boost::int32_t MAXRANDOM= 10;

			boost::shared_ptr<GRandomT<Gem::Hap::RANDOMPROXY> > gr_ptr(new Gem::Hap::GRandomT<Gem::Hap::RANDOMPROXY>());

			std::vector<boost::int32_t> randomHist(21); // 21 positions from -10 to 10

			// Initialize with 0
			for(std::size_t i=0; i<21; i++) randomHist.at(i) = 0;

			for(std::size_t i=0; i<nTests; i++) {
				boost::int32_t randVal;

				// Produce a single random number
				BOOST_CHECK_NO_THROW(randVal = gr_ptr->uniform_int(MINRANDOM, MAXRANDOM));

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

		{ // Test that uniform_int(max) covers the entire range, including the upper boundary in RANDOMLOCAL mode
			// A few settings
			const boost::int32_t MAXRANDOM= 10;

			boost::shared_ptr<GRandomT<Gem::Hap::RANDOMLOCAL> > gr_ptr(new Gem::Hap::GRandomT<Gem::Hap::RANDOMLOCAL>());

			std::vector<boost::int32_t> randomHist(11); // 11 positions from 0 to 10

			// Initialize with 0
			for(std::size_t i=0; i<11; i++) randomHist.at(i) = 0;

			for(std::size_t i=0; i<nTests; i++) {
				boost::int32_t randVal;

				// Produce a single random number
				BOOST_CHECK_NO_THROW(randVal = gr_ptr->uniform_int(MAXRANDOM));

				// Is it in the allowed range ?
				BOOST_CHECK(randVal >= 0 && randVal <= MAXRANDOM);

				// Add the value to the vector
				BOOST_CHECK_NO_THROW(randomHist.at(std::size_t(randVal)) += 1);
			}

			// Due to the large number of entries, we should have > 0 entries in all positions
			for(std::size_t i=0; i<11; i++) {
				BOOST_CHECK(randomHist.at(i) > 0);
			}
		}

		//------------------------------------------------------------------------------

		{ // Test that uniform_int(max) covers the entire range, including the upper boundary in RANDOMPROXy mode
			// A few settings
			const boost::int32_t MAXRANDOM= 10;

			boost::shared_ptr<GRandomT<Gem::Hap::RANDOMPROXY> > gr_ptr(new Gem::Hap::GRandomT<Gem::Hap::RANDOMPROXY>());

			std::vector<boost::int32_t> randomHist(11); // 11 positions from 0 to 10

			// Initialize with 0
			for(std::size_t i=0; i<11; i++) randomHist.at(i) = 0;

			for(std::size_t i=0; i<nTests; i++) {
				boost::int32_t randVal;

				// Produce a single random number
				BOOST_CHECK_NO_THROW(randVal = gr_ptr->uniform_int(MAXRANDOM));

				// Is it in the allowed range ?
				BOOST_CHECK(randVal >= 0 && randVal <= MAXRANDOM);

				// Add the value to the vector
				BOOST_CHECK_NO_THROW(randomHist.at(std::size_t(randVal)) += 1);
			}

			// Due to the large number of entries, we should have > 0 entries in all positions
			for(std::size_t i=0; i<11; i++) {
				BOOST_CHECK(randomHist.at(i) > 0);
			}
		}

		//------------------------------------------------------------------------------

		{ // Check that using extreme values for the boundaries of uniform_int(min,max) and producing random numbers doesn't throw in RAMDOMLOCAL mode
			// A few settings
			const boost::int32_t MINRANDOM=-10;
			const boost::int32_t MAXRANDOM= 10;

			boost::shared_ptr<GRandomT<Gem::Hap::RANDOMLOCAL> > gr_ptr(new Gem::Hap::GRandomT<Gem::Hap::RANDOMLOCAL>());

			volatile boost::int32_t randVal;
			for(std::size_t i=0; i<nTests; i++) {
				BOOST_CHECK_NO_THROW(randVal = gr_ptr->uniform_int(-std::numeric_limits<boost::int32_t>::max(), std::numeric_limits<boost::int32_t>::max()));
			}
		}

		//------------------------------------------------------------------------------

		{ // Check that using extreme values for the boundaries of uniform_int(min,max) and producing random numbers doesn't throw in RAMDOMPROXY mode
			// A few settings
			const boost::int32_t MINRANDOM=-10;
			const boost::int32_t MAXRANDOM= 10;

			boost::shared_ptr<GRandomT<Gem::Hap::RANDOMPROXY> > gr_ptr(new Gem::Hap::GRandomT<Gem::Hap::RANDOMPROXY>());

			volatile boost::int32_t randVal;
			for(std::size_t i=0; i<nTests; i++) {
				BOOST_CHECK_NO_THROW(randVal = gr_ptr->uniform_int(-std::numeric_limits<boost::int32_t>::max(), std::numeric_limits<boost::int32_t>::max()));
			}
		}

		//------------------------------------------------------------------------------

		{ // Check that using extreme values for the boundaries of uniform_int(max) and producing random numbers doesn't throw in RAMDOMLOCAL mode
			// A few settings
			const boost::int32_t MAXRANDOM= 10;

			boost::shared_ptr<GRandomT<Gem::Hap::RANDOMLOCAL> > gr_ptr(new Gem::Hap::GRandomT<Gem::Hap::RANDOMLOCAL>());

			volatile boost::int32_t randVal;
			for(std::size_t i=0; i<nTests; i++) {
				BOOST_CHECK_NO_THROW(randVal = gr_ptr->uniform_int(std::numeric_limits<boost::int32_t>::max()));
			}
		}

		//------------------------------------------------------------------------------

		{ // Check that using extreme values for the boundaries of uniform_int(max) and producing random numbers doesn't throw in RAMDOMPROXY mode
			// A few settings
			const boost::int32_t MAXRANDOM= 10;

			boost::shared_ptr<GRandomT<Gem::Hap::RANDOMPROXY> > gr_ptr(new Gem::Hap::GRandomT<Gem::Hap::RANDOMPROXY>());

			volatile boost::int32_t randVal;
			for(std::size_t i=0; i<nTests; i++) {
				BOOST_CHECK_NO_THROW(randVal = gr_ptr->uniform_int(std::numeric_limits<boost::int32_t>::max()));
			}
		}

		//------------------------------------------------------------------------------

		{ // Test that uniform_smallint(min,max) covers the entire range, including the upper boundary in RANDOMLOCAL mode
			// A few settings
			const boost::int32_t MINRANDOM=-10;
			const boost::int32_t MAXRANDOM= 10;

			boost::shared_ptr<GRandomT<Gem::Hap::RANDOMLOCAL> > gr_ptr(new Gem::Hap::GRandomT<Gem::Hap::RANDOMLOCAL>());

			std::vector<boost::int32_t> randomHist(21); // 21 positions from -10 to 10

			// Initialize with 0
			for(std::size_t i=0; i<21; i++) randomHist.at(i) = 0;

			for(std::size_t i=0; i<nTests; i++) {
				boost::int32_t randVal;

				// Produce a single random number
				BOOST_CHECK_NO_THROW(randVal = gr_ptr->uniform_smallint(MINRANDOM, MAXRANDOM));

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

		{ // Test that uniform_smallint(min,max) covers the entire range, including the upper boundary in RANDOMPROXY mode
			// A few settings
			const boost::int32_t MINRANDOM=-10;
			const boost::int32_t MAXRANDOM= 10;

			boost::shared_ptr<GRandomT<Gem::Hap::RANDOMPROXY> > gr_ptr(new Gem::Hap::GRandomT<Gem::Hap::RANDOMPROXY>());

			std::vector<boost::int32_t> randomHist(21); // 21 positions from -10 to 10

			// Initialize with 0
			for(std::size_t i=0; i<21; i++) randomHist.at(i) = 0;

			for(std::size_t i=0; i<nTests; i++) {
				boost::int32_t randVal;

				// Produce a single random number
				BOOST_CHECK_NO_THROW(randVal = gr_ptr->uniform_smallint(MINRANDOM, MAXRANDOM));

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

		{ // Test that uniform_smallint(max) covers the entire range, including the upper boundary in RANDOMLOCAL mode
			// A few settings
			const boost::int32_t MAXRANDOM= 10;

			boost::shared_ptr<GRandomT<Gem::Hap::RANDOMLOCAL> > gr_ptr(new Gem::Hap::GRandomT<Gem::Hap::RANDOMLOCAL>());

			std::vector<boost::int32_t> randomHist(11); // 11 positions from 0 to 10

			// Initialize with 0
			for(std::size_t i=0; i<11; i++) randomHist.at(i) = 0;

			for(std::size_t i=0; i<nTests; i++) {
				boost::int32_t randVal;

				// Produce a single random number
				BOOST_CHECK_NO_THROW(randVal = gr_ptr->uniform_smallint(MAXRANDOM));

				// Is it in the allowed range ?
				BOOST_CHECK(randVal >= 0 && randVal <= MAXRANDOM);

				// Add the value to the vector
				BOOST_CHECK_NO_THROW(randomHist.at(std::size_t(randVal)) += 1);
			}

			// Due to the large number of entries, we should have > 0 entries in all positions
			for(std::size_t i=0; i<11; i++) {
				BOOST_CHECK(randomHist.at(i) > 0);
			}
		}

		//------------------------------------------------------------------------------

		{ // Test that uniform_smallint(max) covers the entire range, including the upper boundary in RANDOMPROXy mode
			// A few settings
			const boost::int32_t MAXRANDOM= 10;

			boost::shared_ptr<GRandomT<Gem::Hap::RANDOMPROXY> > gr_ptr(new Gem::Hap::GRandomT<Gem::Hap::RANDOMPROXY>());

			std::vector<boost::int32_t> randomHist(11); // 11 positions from 0 to 10

			// Initialize with 0
			for(std::size_t i=0; i<11; i++) randomHist.at(i) = 0;

			for(std::size_t i=0; i<nTests; i++) {
				boost::int32_t randVal;

				// Produce a single random number
				BOOST_CHECK_NO_THROW(randVal = gr_ptr->uniform_smallint(MAXRANDOM));

				// Is it in the allowed range ?
				BOOST_CHECK(randVal >= 0 && randVal <= MAXRANDOM);

				// Add the value to the vector
				BOOST_CHECK_NO_THROW(randomHist.at(std::size_t(randVal)) += 1);
			}

			// Due to the large number of entries, we should have > 0 entries in all positions
			for(std::size_t i=0; i<11; i++) {
				BOOST_CHECK(randomHist.at(i) > 0);
			}
		}

		//------------------------------------------------------------------------------
	}

	/***********************************************************************************/
	// Test features that are expected to fail
	void failures_expected() {
		{ /* nothing */ }
	}

	/***********************************************************************************/
private:
	const std::size_t nTests;
};

/********************************************************************************************/
// This test suite checks as much as possible of the functionality provided
// by the GRandom class. Please also have a look at the histograms created
// in the "manual" test section.
class GHapSuite: public test_suite
{
public:
	GHapSuite() :test_suite("GHapSuite") {
	  // create an instance of the test cases class
	  boost::shared_ptr<GRandom_test> instance(new GRandom_test());

	  test_case* GRandom_no_failure_expected_test_case = BOOST_CLASS_TEST_CASE(&GRandom_test::no_failure_expected, instance);
	  test_case* GRandom_failures_expected_test_case = BOOST_CLASS_TEST_CASE(&GRandom_test::failures_expected, instance);

	  add(GRandom_no_failure_expected_test_case);
	  add(GRandom_failures_expected_test_case);
	}
};

/********************************************************************************************/

#endif /* GRANDOM_TEST_HPP_ */
