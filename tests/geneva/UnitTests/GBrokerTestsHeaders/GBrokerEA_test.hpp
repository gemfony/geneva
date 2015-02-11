/**
 * @file GBrokerPopulation_test.cpp
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt)
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) eu
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
 * http://www.gemfony.eu .
 */

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard header files go here
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

// Boost header files go here

#include <boost/test/unit_test.hpp>
#include <boost/shared_ptr.hpp>

#ifndef GBROKEREA_TEST_HPP_
#define GBROKEREA_TEST_HPP_

// Geneva header files go here
#include "common/GExceptions.hpp"
#include "hap/GRandomT.hpp"
#include "geneva/GBrokerEA.hpp"

using namespace Gem::Hap;
using namespace Gem::Geneva;

using boost::unit_test_framework::test_suite;
using boost::unit_test_framework::test_case;

/********************************************************************************************/
// The actual unit tests for this class
class G_API GBrokerEA_test {
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
// by the GBrokerEA class.
class G_API GBrokerEASuite: public test_suite
{
public:
	GBrokerEASuite() :test_suite("GBrokerEASuite") {
	  // create an instance of the test cases class
	  boost::shared_ptr<GBrokerEA_test> instance(new GBrokerEA_test());

	  test_case* no_failure_expected_test_case = BOOST_CLASS_TEST_CASE(&GBrokerEA_test::no_failure_expected, instance);
	  test_case* failures_expected_test_case = BOOST_CLASS_TEST_CASE(&GBrokerEA_test::failures_expected, instance);

	  add(no_failure_expected_test_case);
	  add(failures_expected_test_case);
	}
};

/********************************************************************************************/

#endif /* GBROKEREA_TEST_HPP_ */
