/**
 * @file GNeuralNetworkUnitTests.cpp
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


#include <boost/test/unit_test.hpp>

using boost::unit_test_framework::test_suite;
using namespace boost::unit_test;

// The class to be tested
#include "../../GStarterIndividual.hpp"

// Contains the necessary tests
#include "geneva/tests/Geneva_tests.hpp"

using namespace Gem;
using namespace Gem::Hap;
using namespace Gem::Geneva;

/************************************************************************************************/
//////////////////////////////////////////////////////////////////////////////////////////////////
/************************************************************************************************/
/**
 * This test suite checks as much as possible of the functionality provided by Geneva classes.
 * All those Geneva classes should be listed that should be tested here.
 */
class GenevaStandardTestSuite
	: public test_suite
{
public:
	GenevaStandardTestSuite() :test_suite("GenevaStandardTestSuite") {
		using userind_types = boost::mpl::list<GStarterIndividual>;

		/****************************************************************************************/

		add( BOOST_TEST_CASE_TEMPLATE( StandardTests_no_failure_expected, userind_types) );
		add( BOOST_TEST_CASE_TEMPLATE( StandardTests_failures_expected, userind_types) );
	}
};

/************************************************************************************************/
/**
 * The test program entry point
 */
test_suite* init_unit_test_suite(int argc, char** const argv) {
	framework::master_test_suite().add(new GenevaStandardTestSuite());
	return 0;
}

/************************************************************************************************/
