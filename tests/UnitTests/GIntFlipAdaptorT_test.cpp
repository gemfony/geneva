/**
 * @file GIntFlipAdaptorT_test.cpp
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
#include <boost/test/test_case_template.hpp>
#include <boost/mpl/list.hpp>

#include <boost/shared_ptr.hpp>
#include <boost/cstdint.hpp>

// Geneva header files go here
#include "GenevaExceptions.hpp"
#include "GLogger.hpp"
#include "GLogTargets.hpp"
#include "GRandom.hpp"
#include "GIntFlipAdaptorT.hpp"

using namespace Gem;
using namespace Gem::Util;
using namespace Gem::GenEvA;
using namespace Gem::GLogFramework;

/***********************************************************************************/
// This test suite checks as much as possible of the functionality provided
// by the GIntFlipAdaptorT class. The template is instantiated for all types
// defined in the above mpl::list .
BOOST_AUTO_TEST_SUITE(GIntFlipAdaptorTSuite)

typedef boost::mpl::list<boost::int32_t, bool,  char> test_types;

/***********************************************************************************/
// Test features that are expected to work
BOOST_AUTO_TEST_CASE_TEMPLATE( GIntFlipAdaptorT_no_failure_expected, T, test_types )
{
	GRandom gr;

	// Test simple instantiation
	GIntFlipAdaptorT<T> gifat0;
	// Test instantiation with a probability mutation
	GIntFlipAdaptorT<T> gifat1(0.2);

	BOOST_CHECK(gifat1 != gifat0);

	// Test copy construction
	GIntFlipAdaptorT<T> gifat2(gifat1);

	BOOST_CHECK(gifat2 == gifat1);
	BOOST_CHECK(gifat2 != gifat0);

	// Test assignment
	GIntFlipAdaptorT<T> gifat3;
	gifat3 = gifat1;

	BOOST_CHECK(gifat3 == gifat1);
	BOOST_CHECK(gifat3 != gifat0);

	// Retrieve the mutation probablilty and modify it slightly. Then check similariy and equality.
	double mutProb = gifat3.getMutationProbability();
	mutProb -= exp(-10);
	gifat3.setMutationProbability(mutProb);
	BOOST_CHECK(gifat3 != gifat1); // May no longer be equal
	BOOST_CHECK(!gifat3.isEqualTo(gifat1)); // Should have the same result
	BOOST_CHECK(gifat3.isSimilarTo( gifat1, exp(-9)));

	// Repeated mutation, before and after setting the mutation parameters
}

/***********************************************************************************/
// Test features that are expected to fail
BOOST_AUTO_TEST_CASE_TEMPLATE( GIntFlipAdaptorT_failures_expected, T, test_types )
{
	GRandom gr;

	// Simple instantiation
	GIntFlipAdaptorT<T> gifat0;
	// Assignment of an invalid mutation probability
	BOOST_CHECK_THROW(gifat0.setMutationProbability(-0.1), Gem::GenEvA::geneva_error_condition);
	BOOST_CHECK_THROW(gifat0.setMutationProbability(1.1), Gem::GenEvA::geneva_error_condition);
}
/***********************************************************************************/

BOOST_AUTO_TEST_SUITE_END()
