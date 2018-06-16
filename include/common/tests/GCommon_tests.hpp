/**
 * @file GCommon_tests.hpp
 *
 * Tests for the common library.
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

#pragma once

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard header files go here
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <memory>

// Boost header files go here

// Geneva header files go here
#include "common/tests/GBoundedBufferT_tests.hpp"

using namespace Gem::Common;
using namespace Gem::Common::Tests;

using boost::unit_test_framework::test_suite;
using boost::unit_test_framework::test_case;

/********************************************************************************************/
/**
 * This test suite checks as much as possible of the functionality provided
 + by the GCommon library.
 */
class GCommonSuite: public test_suite
{
public:
	 GCommonSuite() :test_suite("GCommonSuite") {
		 // create an instance of the test cases class
		 boost::shared_ptr<GBoundedBufferT_tests> instance(new GBoundedBufferT_tests());

		 test_case* GBoundedBufferT_no_failure_expected_test_case
			 = BOOST_CLASS_TEST_CASE(&GBoundedBufferT_tests::no_failure_expected, instance);
		 test_case* GBoundedBufferT_failures_expected_test_case
			 = BOOST_CLASS_TEST_CASE(&GBoundedBufferT_tests::failures_expected, instance);

		 add(GBoundedBufferT_no_failure_expected_test_case);
		 add(GBoundedBufferT_failures_expected_test_case);
	 }
};

/********************************************************************************************/

