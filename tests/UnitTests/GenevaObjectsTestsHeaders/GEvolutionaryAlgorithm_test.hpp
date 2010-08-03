/**
 * @file GEvolutionaryAlgorithm_test.cpp
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

// Boost header files go here

#include <boost/test/unit_test.hpp>
#include <boost/shared_ptr.hpp>

#ifndef GEVOLUTIONARYALGORITHM_TEST_HPP_
#define GEVOLUTIONARYALGORITHM_TEST_HPP_

// Geneva header files go here
#include "common/GExceptions.hpp"
#include "hap/GRandomT.hpp"
#include "geneva/GEvolutionaryAlgorithm.hpp"
#include "GEqualityPrinter.hpp"

using namespace Gem::Hap;
using namespace Gem::Geneva;

using boost::unit_test_framework::test_suite;
using boost::unit_test_framework::test_case;

/********************************************************************************************/
// The actual unit tests for this class
class GEvolutionaryAlgorithm_test {
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
	GRandomT<RANDOMLOCAL> gr;
};

/********************************************************************************************/
// This test suite checks as much as possible of the functionality provided
// by the GEvolutionaryAlgorithm class.
class GEvolutionaryAlgorithmSuite: public test_suite
{
public:
	GEvolutionaryAlgorithmSuite() :test_suite("GEvolutionaryAlgorithmSuite") {
	  // create an instance of the test cases class
	  boost::shared_ptr<GEvolutionaryAlgorithm_test> instance(new GEvolutionaryAlgorithm_test());

	  test_case* GEvolutionaryAlgorithm_no_failure_expected_test_case = BOOST_CLASS_TEST_CASE(&GEvolutionaryAlgorithm_test::no_failure_expected, instance);
	  test_case* GEvolutionaryAlgorithm_failures_expected_test_case = BOOST_CLASS_TEST_CASE(&GEvolutionaryAlgorithm_test::failures_expected, instance);

	  add(GEvolutionaryAlgorithm_no_failure_expected_test_case);
	  add(GEvolutionaryAlgorithm_failures_expected_test_case);
	}
};

/********************************************************************************************/

#endif /* GEVOLUTIONARYALGORITHM_TEST_HPP_ */
