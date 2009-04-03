/**
 * @file GObject_test.cpp
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

// Geneva header files go here
#include "GenevaExceptions.hpp"
#include "GLogger.hpp"
#include "GLogTargets.hpp"
#include "GRandom.hpp"
#include "GObject.hpp"
#include "GBooleanAdaptor.hpp"

using namespace Gem;
using namespace Gem::Util;
using namespace Gem::GenEvA;
using namespace Gem::GLogFramework;

/***********************************************************************************/
// This test suite checks as much as possible of the functionality provided
// by the GObject class. As GObject cannot be instantiated itself, we perform
// testing through a "near" instantiable class. Not all functions of GObject are
// tested, particularly if these functions also exist in the derived class (and
// internally call the GObject version).
BOOST_AUTO_TEST_SUITE(GObjectSuite)

/***********************************************************************************/
// Test features that are expected to work
BOOST_AUTO_TEST_CASE( GObject_no_failure_expected )
{
	GRandom gr;

	// Test name setting
	GBooleanAdaptor gba0;
	BOOST_CHECK(gba0.name() == GINTFLIPADAPTORSTANDARDNAME);
	gba0.setName("myNewName");
	BOOST_CHECK(gba0.name() == "myNewName");

	// Test that the object can be translated into a string and back
	// again and that afterwards both objects are at least similar.
	// Note that text-io may result in a lost of precision, so that double
	// values might differ slightly.
	// text mode:
	{
		std::ostringstream stream;
		stream << gba0.toString(TEXTSERIALIZATION);
		GBooleanAdaptor gba1; // Create a new, pristine object
		gba1.fromString(stream.str(),TEXTSERIALIZATION);
		BOOST_CHECK(gba1.isSimilarTo(gba0,exp(-10)));
	}

	{
		std::ostringstream stream;
		stream << gba0.toString(XMLSERIALIZATION);
		GBooleanAdaptor gba1; // Create a new, pristine object
		gba1.fromString(stream.str(),XMLSERIALIZATION);
		BOOST_CHECK(gba1.isSimilarTo(gba0,exp(-10)));
	}

	{
		std::ostringstream stream;
		stream << gba0.toString(BINARYSERIALIZATION);
		GBooleanAdaptor gba1; // Create a new, pristine object
		gba1.fromString(stream.str(),BINARYSERIALIZATION); // We shpuld be able to achieve equality for this mode
		BOOST_CHECK(gba1.isEqualTo(gba0));
	}

}

/***********************************************************************************/
// Test features that are expected to fail
BOOST_AUTO_TEST_CASE( GObject_failures_expected )
{
	GRandom gr;

}
/***********************************************************************************/

BOOST_AUTO_TEST_SUITE_END()
