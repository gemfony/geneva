/**
 * @file GRandom_test.cpp
 */

/* Copyright (C) Dr. Ruediger Berlich and Karlsruhe Institute of Technology
 * (University of the State of Baden-Wuerttemberg and National Laboratory
 * of the Helmholtz Association)
 *
 * Contact: info [at] gemfony (dot) com
 *
 * This file is part of the Geneva library, Gemfony scientific's optimization
 * library.
 *
 * Geneva is free software: you can redistribute it and/or modify
 * it under the terms of version 3 of the GNU Affero General Public License
 * as published by the Free Software Foundation.
 *
 * Geneva is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with the Geneva library.  If not, see <http://www.gnu.org/licenses/>.
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
		NRNR(100000)
	{ /* empty */ }

	/***********************************************************************************/
	// Test features that are expected to work
	void no_failure_expected() {
		using namespace Gem::Hap;

		const boost::uint32_t startSeed = 41;
		BOOST_CHECK(GRANDOMFACTORY->checkSeedingIsInitialized() == false);

		// Check that we can set and retrieve the current seed
		bool seedSetSuccess = GRANDOMFACTORY->setStartSeed(startSeed);
		BOOST_CHECK(seedSetSuccess == true);
		BOOST_CHECK(GRANDOMFACTORY->checkSeedingIsInitialized() == false); // The thread hasn't started yet
		boost::uint32_t  testSeed = GRANDOMFACTORY->getStartSeed();
		BOOST_CHECK_MESSAGE(testSeed == startSeed, "testSeed = " << testSeed << ", should be " << startSeed);

		// Retrieve a single seed and check that the seeding thread has started
		boost::uint32_t singleSeed = GRANDOMFACTORY->getSeed();
		BOOST_CHECK(GRANDOMFACTORY->checkSeedingIsInitialized() == true);
		BOOST_CHECK(singleSeed != startSeed);

		// Check that seeding creates different values during every call for a predefined number of calls
		std::size_t seedingQueueSize = GRANDOMFACTORY->getSeedingQueueSize();
		std::vector<boost::uint32_t> seedVec(static_cast<boost::uint32_t>(seedingQueueSize));
		for(std::size_t i=0; i<seedingQueueSize; i++) {
			seedVec.at(i) = GRANDOMFACTORY->getSeed();
		}
		std::sort(seedVec.begin(), seedVec.end());
		for(std::size_t i=1; i<seedingQueueSize; i++) {
			BOOST_CHECK(seedVec.at(i) > seedVec.at(i-1));
		}

		/*
		GRandomT<RANDOMLOCAL> gr1, gr2, gr3;
		boost::shared_ptr<GRandomBase> gr4_ptr(new GRandom());

		// Make gr1 and gr2 use the random factory
		BOOST_CHECK_NO_THROW(gr1.setRNRFactoryMode());
		BOOST_CHECK_NO_THROW(gr2.setRNRFactoryMode());

		// gr1 and gr2 should have received different start seeds
		boost::uint32_t seed1, seed2;
		seed1 = gr1.getSeed();
		seed2 = gr2.getSeed();
		BOOST_CHECK_MESSAGE(seed1 != seed2, "Error: Found equal seeds: " << seed1 << " " << seed2);

		// Check that we can set gr3's production flags and can produce a number of random numbers
		gr3.setRNRFactoryMode();
		BOOST_CHECK(gr3.getRnrGenerationMode () == Gem::Hap::RNRFACTORY);
		double last = -1., now=0.;
		for(std::size_t i=0; i<NRNR; i++) {
			now = gr3.evenRandom();
			BOOST_CHECK_MESSAGE(now != last, "Found equal random numbers(1)" << last << " " << now);
			last = now;
		}
		gr3.setRNRLocalMode();
		BOOST_CHECK(gr3.getRnrGenerationMode () == Gem::Hap::RNRLOCAL);
		last = -1., now=0.;
		for(std::size_t i=0; i<NRNR; i++) {
			now = gr3.evenRandom();
			BOOST_CHECK_MESSAGE(now != last, "Found equal random numbers(1)" << last << " " << now);
			last = now;
		}

		// Check that two objects emit different values
		const std::size_t arraySize=10;
		std::vector<boost::int32_t> intRnr1, intRnr2;
		for(std::size_t i=0; i<arraySize; i++) {
			intRnr1.push_back(gr1.discreteRandom(-10,10));
			intRnr2.push_back(gr2.discreteRandom(-10,10));
		}
		BOOST_CHECK(intRnr1 != intRnr2);

		std::vector<double> dRnr1, dRnr2;
		for(std::size_t i=0; i<arraySize; i++) {
			dRnr1.push_back(gr1.evenRandom(-10.,10.));
			dRnr2.push_back(gr2.evenRandom(-10.,10.));
		}
		BOOST_CHECK(dRnr1 != dRnr2);

		// Check that "ranged" distributions never fall outside of the desired range
		for(std::size_t i=0; i<NRNR; i++) {
			boost::int32_t irnr = gr1.discreteRandom<boost::int32_t>(-10,15);
			double drnr = gr1.evenRandom(-10.,15.);
			BOOST_CHECK(irnr >= -10 && irnr <15); // note the "<" . We want to check that the upper boundary is not reached
			BOOST_CHECK(drnr >= -10. && drnr <= 15.);
		}

		// Check that it is possible to use GRandom objects as producers
		// for boost's random number distributions
		boost::uniform_int<> dist(1, 6);
		boost::variate_generator<Gem::Hap::GRandom&, boost::uniform_int<> > die(gr1, dist);
		for(std::size_t i=0; i<NRNR; i++) {
			double drnr = die();
			BOOST_CHECK(drnr >= 1 && drnr <= 6);
		}
		*/
	}

	/***********************************************************************************/
	// Test features that are expected to fail
	void failures_expected() {
		{
			// Test (as the very last test) that multiple creation of GRandomFactory throws.
			// Note that one instance of GRandomFactory should already be running, simply by virtue of
			// the inclusion of the GRandom header file.
			BOOST_CHECK_THROW(boost::shared_ptr<GRandomFactory>(new GRandomFactory()), Gem::Common::gemfony_error_condition);
		}
	}

	/***********************************************************************************/
private:
	const std::size_t NRNR;
};

/********************************************************************************************/
// This test suite checks as much as possible of the functionality provided
// by the GRandom class. Please also have a look at the histograms created
// in the "manual" test section.
class GRandomSuite: public test_suite
{
public:
	GRandomSuite() :test_suite("GRandomSuite") {
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
