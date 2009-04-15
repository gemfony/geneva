/**
 * @file GAdaptorT_test.cpp
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
#include <cmath>

// Boost header files go here

#include <boost/test/unit_test.hpp>

#include <boost/shared_ptr.hpp>
#include <boost/cstdint.hpp>

// Geneva header files go here
#include "GenevaExceptions.hpp"
#include "GLogger.hpp"
#include "GLogTargets.hpp"
#include "GRandom.hpp"
#include "GAdaptorT.hpp"
#include "GBooleanAdaptor.hpp"

using namespace Gem;
using namespace Gem::Util;
using namespace Gem::GenEvA;
using namespace Gem::GLogFramework;

/***********************************************************************************/
// This test suite checks as much as possible of the functionality provided
// by the GAdaptorT class. As GAdaptorT cannot be instantiated itself, we perform
// testing through a "near" instantiable class. Not all functions of GAdaptorT are
// tested, particularly if these functions also exist in the derived class (and
// internally call the GAdaptorT version).
BOOST_AUTO_TEST_SUITE(GAdaptorTSuite)

/***********************************************************************************/
// Test features that are expected to work
BOOST_AUTO_TEST_CASE( GAdaptorT_no_failure_expected )
{
	GRandom gr;

	GBooleanAdaptor gba0;

	// Perform tests with various settings of the adaptionThreshold.
	BOOST_CHECK(gba0.getAdaptionThreshold() == 0); // Should have been initialized with this value
	BOOST_CHECK(gba0.getAdaptionCounter() == 0);
	gba0.setAdaptionThreshold(1);
	BOOST_CHECK(gba0.getAdaptionThreshold() == 1);

	// Test mutation, including a test of the incrementation of the
	// adaption counter after each mutation. This is also a good test
	// of some of GBooleanAdaptor's functionality
	bool mutationTarget=false;
	for(boost::uint32_t aT=0; aT<100; aT++) {
		gba0.setAdaptionThreshold(aT);
		boost::uint32_t oldAdaptionCounter=gba0.getAdaptionCounter();
		for(boost::uint32_t m=0; m<1000; m++) { // mutation counter
			gba0.mutate(mutationTarget);
			boost::uint32_t currentAdaptionCounter=gba0.getAdaptionCounter();
			BOOST_CHECK(currentAdaptionCounter<=aT);
			if(aT != 0) BOOST_CHECK(currentAdaptionCounter != oldAdaptionCounter);
			oldAdaptionCounter = currentAdaptionCounter;
		}
	}
}

/***********************************************************************************/
// Test features that are expected to fail
BOOST_AUTO_TEST_CASE( GAdaptorT_failures_expected )
{
	GRandom gr;

	/* nothing */
}
/***********************************************************************************/

BOOST_AUTO_TEST_SUITE_END()
