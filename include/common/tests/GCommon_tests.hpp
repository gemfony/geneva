/**
 * @file GCommon_tests.hpp
 *
 * Tests for the common library.
 */

/********************************************************************************
 *
 * This file is part of the Geneva library collection. The following license
 * applies to this file:
 *
 * ------------------------------------------------------------------------------
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * ------------------------------------------------------------------------------
 *
 * Note that other files in the Geneva library collection may use a different
 * license. Please see the licensing information in each file.
 *
 ********************************************************************************
 *
 * Geneva was started by Dr. Rüdiger Berlich and was later maintained together
 * with Dr. Ariel Garcia under the auspices of Gemfony scientific. For further
 * information on Gemfony scientific, see http://www.gemfomy.eu .
 *
 * The majority of files in Geneva was released under the Apache license v2.0
 * in February 2020.
 *
 * See the NOTICE file in the top-level directory of the Geneva library
 * collection for a list of contributors and copyright information.
 *
 ********************************************************************************/

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

