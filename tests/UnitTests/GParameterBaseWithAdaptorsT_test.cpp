/**
 * @file GParameterBaseWithAdaptorsT_test.cpp
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
#include <algorithm>
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
#include "GParameterBaseWithAdaptorsT.hpp"
#include "GBoolean.hpp"

using namespace Gem;
using namespace Gem::Util;
using namespace Gem::GenEvA;
using namespace Gem::GLogFramework;

/***********************************************************************************/
// This test suite checks as much as possible of the functionality provided
// by the GParameterBaseWithAdaptorsT class. As this class can not be instantiated
// directly, the (indirect) derivative class GBoolean is used instead. Note that GBoolean
// is itself just a typedef of GParameterT<bool>, so we are also testing that class
// here to some extent.
BOOST_AUTO_TEST_SUITE(GParameterBaseWithAdaptorsTSuite)

/***********************************************************************************/
// Test features that are expected to work
BOOST_AUTO_TEST_CASE( GParameterBaseWithAdaptorsT_no_failure_expected)
{
	GRandom gr;

	// Test default construction
	BOOST_CHECK_NO_THROW(GBoolean gb);

	// Test construction with a value
	GBoolean gb0(false), gb1(true);
	BOOST_CHECK(gb0 != gb1);

	// Test copy construction
	GBoolean gb2(gb1);
	BOOST_CHECK(gb2 == gb1);
	BOOST_CHECK(gb2 != gb0);

	// Test assignment
	GBoolean gb3;
	gb3 = gb1;
	BOOST_CHECK(gb3 == gb1);
	BOOST_CHECK(gb3 != gb0);

	// Test cloning
	GObject *gb3_clone = gb3.clone();

	// Test loading
	BOOST_CHECK_NO_THROW(gb0.load(gb3_clone));
	BOOST_CHECK_NO_THROW(delete gb3_clone);
	BOOST_CHECK(gb0 == gb3);

	// Re-assign the original value
	gb0 = false;
	BOOST_CHECK(gb0 != gb3);

	// Test (de-)serialization in differnet modes
	{ // plain text format
	   GBoolean gb4(gb0);
	   BOOST_CHECK(gb4.isEqualTo(gb0));
	   gb4.fromString(gb1.toString(TEXTSERIALIZATION), TEXTSERIALIZATION);
	   BOOST_CHECK(!gb4.isEqualTo(gb0));
	   BOOST_CHECK(gb4.isSimilarTo(gb1, exp(-10)));
	}
	{ // XML format
	   GBoolean gb4(gb0);
	   BOOST_CHECK(gb4.isEqualTo(gb0));
	   gb4.fromString(gb1.toString(XMLSERIALIZATION), XMLSERIALIZATION);
	   BOOST_CHECK(!gb4.isEqualTo(gb0));
	   BOOST_CHECK(gb4.isSimilarTo(gb1, exp(-10)));
	}
	{ // binary test format
	   GBoolean gb4(gb0);
	   BOOST_CHECK(gb4 == gb0);
	   gb4.fromString(gb1.toString(BINARYSERIALIZATION), BINARYSERIALIZATION);
	   BOOST_CHECK(gb4 != gb0);
	   BOOST_CHECK(gb4 == gb1);
	}


}

/***********************************************************************************/
// Test features that are expected to fail
BOOST_AUTO_TEST_CASE( GParameterBaseWithAdaptorsT_failures_expected )
{
	GRandom gr;

	// Check that self-assignment throws
}
/***********************************************************************************/

BOOST_AUTO_TEST_SUITE_END()
