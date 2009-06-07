/**
 * @file GBasePopulation_test.cpp
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

// Boost header files go here

#include <boost/test/unit_test.hpp>

#include <boost/shared_ptr.hpp>

// Geneva header files go here
#include "GenevaExceptions.hpp"
#include "GRandom.hpp"
#include "GBasePopulation.hpp"

using namespace Gem;
using namespace Gem::Util;
using namespace Gem::GenEvA;

using boost::unit_test_framework::test_suite;
using boost::unit_test_framework::test_case;

/********************************************************************************************/
// The actual unit tests for this class
class GBasePopulation_test {
public:
	/***********************************************************************************/
	// Test features that are expected to work
	void no_failure_expected() {
		/* empty */
	}

	/***********************************************************************************/
	// Test features that are expected to fail
	void failures_expected() {
		/* empty */
	}

	/***********************************************************************************/
private:
	GRandom gr;
};

/********************************************************************************************/
// This test suite checks as much as possible of the functionality provided
// by the GBasePopulation class.
class GBasePopulationSuite: public test_suite
{
public:
	GBasePopulationSuite() :test_suite("GBasePopulationSuite") {
	  // create an instance of the test cases class
	  boost::shared_ptr<GBasePopulation_test> instance(new GBasePopulation_test());

	  test_case* GBasePopulation_no_failure_expected_test_case = BOOST_CLASS_TEST_CASE(&GBasePopulation_test::no_failure_expected, instance);
	  test_case* GBasePopulation_failures_expected_test_case = BOOST_CLASS_TEST_CASE(&GBasePopulation_test::failures_expected, instance);

	  add(GBasePopulation_no_failure_expected_test_case);
	  add(GBasePopulation_failures_expected_test_case);
	}
};

/********************************************************************************************/
