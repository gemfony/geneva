/**
 * @file GGaussAdaptorT_test.cpp
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

// Boost header files go here

#include <boost/test/unit_test.hpp>
#include <boost/test/test_case_template.hpp>
#include <boost/mpl/list.hpp>

#include <boost/shared_ptr.hpp>
#include <boost/cstdint.hpp>

// Geneva header files go here
#include "GenevaExceptions.hpp"
#include "GLogger.hpp"
#include "GLogTargets.hpp"
#include "GRandom.hpp"
#include "GParameterT.hpp"

using namespace Gem;
using namespace Gem::Util;
using namespace Gem::GenEvA;
using namespace Gem::GLogFramework;

/***********************************************************************************/
// This test suite checks as much as possible of the functionality provided
// by the GGaussAdaptorT class. The template is instantiated for all types
// defined in the above mpl::list . Note that a lot of functionality of this class has
// already been covered as GBooleanAdaptor has been used as a vehicle to
// test GObject and GAdaotorT.
BOOST_AUTO_TEST_SUITE(GParameterTSuite)

typedef boost::mpl::list<bool, char, boost::int32_t, double> test_types;

/***********************************************************************************/
// Test features that are expected to work
BOOST_AUTO_TEST_CASE_TEMPLATE( GGaussAdaptorT_no_failure_expected, T, test_types )
{
	GRandom gr;

	// Test default construction
	BOOST_CHECK_NO_THROW(GParameterT<T> gpt);

	// Test construction with a value
	GParameterT<T> gpt0(T(NULL)), gpt1(T(1));
	BOOST_CHECK(gpt0 != gpt1);

	// Test copy construction
	GParameterT<T> gpt2(gpt1);
	BOOST_CHECK(gpt2 == gpt1);
	BOOST_CHECK(gpt2 != gpt0);

	// Test assignment
	GParameterT<T> gpt3;
	gpt3 = gpt1;
	BOOST_CHECK(gpt3 == gpt1);
	BOOST_CHECK(gpt3 != gpt0);

	// Test cloning
	GObject *gpt3_clone = gpt3.clone();

	// Test loading
	BOOST_CHECK_NO_THROW(gpt0.load(gpt3_clone));
	BOOST_CHECK_NO_THROW(delete gpt3_clone);
	BOOST_CHECK(gpt0 == gpt3);

	// Re-assign the original value
	gpt0 = T(NULL);
	BOOST_CHECK(gpt0 != gpt3);

	// Test (de-)serialization in differnet modes
	{ // plain text format
	   GParameterT<T> gpt4(gpt0);
	   BOOST_CHECK(gpt4.isEqualTo(gpt0));
	   gpt4.fromString(gpt1.toString(TEXTSERIALIZATION), TEXTSERIALIZATION);
	   BOOST_CHECK(!gpt4.isEqualTo(gpt0));
	   BOOST_CHECK(gpt4.isSimilarTo(gpt1, exp(-10)));
	}
	{ // XML format
	   GParameterT<T> gpt4(gpt0);
	   BOOST_CHECK(gpt4.isEqualTo(gpt0));
	   gpt4.fromString(gpt1.toString(XMLSERIALIZATION), XMLSERIALIZATION);
	   BOOST_CHECK(!gpt4.isEqualTo(gpt0));
	   BOOST_CHECK(gpt4.isSimilarTo(gpt1, exp(-10)));
	}
	{ // binary test format
	   GParameterT<T> gpt4(gpt0);
	   BOOST_CHECK(gpt4 == gpt0);
	   gpt4.fromString(gpt1.toString(BINARYSERIALIZATION), BINARYSERIALIZATION);
	   BOOST_CHECK(gpt4 != gpt0);
	   BOOST_CHECK(gpt4 == gpt1);
	}

}

/***********************************************************************************/
// Test features that are expected to fail
BOOST_AUTO_TEST_CASE_TEMPLATE( GGaussAdaptorT_failures_expected, T, test_types )
{
	GRandom gr;

}
/***********************************************************************************/

BOOST_AUTO_TEST_SUITE_END()
