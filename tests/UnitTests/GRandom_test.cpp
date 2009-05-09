/**
 * @file GRandom_test.cpp
 */

/* Copyright (C) 2009 Dr. Ruediger Berlich
 *
 * This file is part of Geneva, Gemfony scientific's optimization library.
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

// Geneva header files go here
#include "GenevaExceptions.hpp"
#include "GLogger.hpp"
#include "GLogTargets.hpp"
#include "GRandom.hpp"
#include "GEnums.hpp"

using namespace Gem;
using namespace Gem::Util;
using namespace Gem::GenEvA;
using namespace Gem::GLogFramework;

/***********************************************************************************/
// This test suite checks as much as possible of the functionality provided
// by the GRandom class. Please also have a look at the histograms created
// in the "manual" test section.
BOOST_AUTO_TEST_SUITE(GRandomSuite)

const std::size_t NRNR = 100000;

/***********************************************************************************/
// Test features that are expected to work
BOOST_AUTO_TEST_CASE( GRandom_no_failure_expected )
{
	const boost::uint32_t startSeed = 41;
	BOOST_CHECK(GRANDOMFACTORY->checkSeedIsInitialized() == false);

	// Check that we can set and retrieve the current seed
	GRANDOMFACTORY->setSeed(startSeed);
	BOOST_CHECK(GRANDOMFACTORY->checkSeedIsInitialized() == true);
	boost::uint32_t  testSeed = GRANDOMFACTORY->getSeed();
	BOOST_CHECK_MESSAGE(testSeed == startSeed, "testSeed = " << testSeed << ", should be " << startSeed);

	// Check that seeding creates different values during every call
	std::vector<boost::uint32_t> seedVec(static_cast<boost::uint32_t>(NRNR));
	for(std::size_t i=0; i<NRNR; i++) {
		seedVec.at(i) = GRandomFactory::GSeed();
	}
	std::sort(seedVec.begin(), seedVec.end());
	for(std::size_t i=1; i<NRNR; i++) {
		BOOST_CHECK(seedVec.at(i) > seedVec.at(i-1));
	}

	GRandom gr1, gr2, gr3;
	boost::shared_ptr<GRandom> gr4_ptr(new GRandom());

	// Make gr1 and gr2 use the random factory
	BOOST_CHECK_NO_THROW(gr1.setRNRFactoryMode());
	BOOST_CHECK_NO_THROW(gr2.setRNRFactoryMode());

	// As now four random number generators have been started, the global seed should have
	// been incremented. Setting a new start seed should be ignored, and the current seed should
	// be different from the start seed
	BOOST_CHECK(GRANDOMFACTORY->getSeed() > startSeed);
	GRANDOMFACTORY->setSeed(startSeed);
	BOOST_CHECK(GRANDOMFACTORY->getSeed() > startSeed);

	// gr1 and gr2 should have received different seeds
	boost::uint32_t seed1, seed2;
	seed1 = gr1.getSeed();
	seed2 = gr2.getSeed();
	BOOST_CHECK_MESSAGE(seed1 != seed2, "Error: Found equal seeds: " << seed1 << " " << seed2);

	// Check that we can set gr3's production flags and can prouce a number of random numbers
	gr3.setRNRFactoryMode();
	BOOST_CHECK(gr3.getRNRGenerationMode () == Gem::Util::RNRFACTORY);
	double last = -1., now=0.;
	for(std::size_t i=0; i<NRNR; i++) {
		now = gr3.evenRandom();
		BOOST_CHECK_MESSAGE(now != last, "Found equal random numbers(1)" << last << " " << now);
		last = now;
	}
	gr3.setRNRLocalMode();
	BOOST_CHECK(gr3.getRNRGenerationMode () == Gem::Util::RNRLOCAL);
	last = -1., now=0.;
	for(std::size_t i=0; i<NRNR; i++) {
		now = gr3.evenRandom();
		BOOST_CHECK_MESSAGE(now != last, "Found equal random numbers(1)" << last << " " << now);
		last = now;
	}
	gr3.setRNRProxyMode(gr4_ptr);
	BOOST_CHECK(gr3.getRNRGenerationMode () == Gem::Util::RNRPROXY);
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
}

/***********************************************************************************/
// Test features that are expected to fail
BOOST_AUTO_TEST_CASE( GRandom_failures_expected )
{
	{
		// Test (as the very last test) that multiple creation of GRandomFactory throws.
		// Note that one instance of GRandomFactory should already be running, simply by virtue of
		// the inclusion of the GRandom header file.
		BOOST_CHECK_THROW(boost::shared_ptr<GRandomFactory>(new GRandomFactory()), Gem::GenEvA::geneva_error_condition);
	}
}
/***********************************************************************************/

BOOST_AUTO_TEST_SUITE_END()
