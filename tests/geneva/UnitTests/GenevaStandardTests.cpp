/**
 * @file GenevaStandardTests.cpp
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

#include <iostream>

// #define BOOST_TEST_MODULE GenevaStandardTestSuite
// #define BOOST_TEST_DYN_LINK  // cmp. http://stackoverflow.com/questions/39171467/there-is-no-argument-provided-for-parameter-color-output-with-boost-test-and-cte
#include <boost/test/unit_test.hpp>

using boost::unit_test_framework::test_suite;
using namespace boost::unit_test;

// All classes that will be tested in this file
#include "geneva/GInt32FlipAdaptor.hpp"
#include "geneva/GBooleanAdaptor.hpp"
#include "geneva/GInt32GaussAdaptor.hpp"
#include "geneva/GDoubleBiGaussAdaptor.hpp"
#include "geneva/GDoubleGaussAdaptor.hpp"
#include "geneva/GBooleanObjectCollection.hpp"
#include "geneva/GInt32ObjectCollection.hpp"
#include "geneva/GDoubleObjectCollection.hpp"
#include "geneva/GConstrainedDoubleObjectCollection.hpp"
#include "geneva/GConstrainedInt32ObjectCollection.hpp"
#include "geneva/GBooleanObject.hpp"
#include "geneva/GConstrainedInt32Object.hpp"
#include "geneva/GInt32Object.hpp"
#include "geneva/GDoubleObject.hpp"
#include "geneva/GConstrainedDoubleCollection.hpp"
#include "geneva/GConstrainedDoubleObject.hpp"
#include "geneva/GInt32Collection.hpp"
#include "geneva/GDoubleCollection.hpp"
#include "geneva/GBooleanCollection.hpp"
#include "geneva/G_OptimizationAlgorithm_GradientDescent_PersonalityTraits.hpp"
#include "geneva/G_OptimizationAlgorithm_ParameterScan_PersonalityTraits.hpp"
#include "geneva/G_OptimizationAlgorithm_SwarmAlgorithm_PersonalityTraits.hpp"
#include "geneva/GParameterObjectCollection.hpp"
#include "geneva/GTestIndividual1.hpp"
#include "geneva-individuals/GTestIndividual3.hpp"
#include "geneva-individuals/GFunctionIndividual.hpp"
#include "geneva-individuals/GDelayIndividual.hpp"
#include "geneva-individuals/GExternalEvaluatorIndividual.hpp"

#include "geneva/tests/Geneva_tests.hpp"

using namespace Gem::Geneva;

/*************************************************************************************************/
/**
 * This test suite checks as much as possible of the functionality provided by Geneva classes.
 * All instantiable core Geneva classes should be listed here.
 */
class GenevaStandardTestSuite
	: public test_suite
{
public:
    G_API_GENEVA GenevaStandardTestSuite() :test_suite("GenevaStandardTestSuite") {

		using adaptor_types = boost::mpl::list<
			GInt32FlipAdaptor
			, GBooleanAdaptor
			, GInt32GaussAdaptor
			, GDoubleBiGaussAdaptor
			, GDoubleGaussAdaptor
		>;

		using data_types = boost::mpl::list<
			GBooleanObject
			, GInt32Object
			, GDoubleObject
			, GConstrainedInt32Object
			, GConstrainedDoubleObject
		>;

		using object_collection_types = boost::mpl::list<
			GParameterObjectCollection
			, GBooleanObjectCollection
			, GInt32ObjectCollection
			, GConstrainedInt32ObjectCollection
			, GDoubleObjectCollection
			, GConstrainedDoubleObjectCollection
		>;

		using pod_collection_types = boost::mpl::list<
			GInt32Collection
			, GDoubleCollection
			, GBooleanCollection
			, GConstrainedDoubleCollection
		>;

		// TODO: Add tests for algorithm types
		using algorithm_types = boost::mpl::list<
		>;

		using trait_types = boost::mpl::list<
			GEvolutionaryAlgorithm_PersonalityTraits
			, GGradientDescent_PersonalityTraits
			, GSwarmAlgorithm_PersonalityTraits
			, GSimulatedAnnealing_PersonalityTraits
			, GParameterScan_PersonalityTraits
		>;

		using individual_types = boost::mpl::list<
			Gem::Tests::GTestIndividual1
			// , Gem::Tests::GTestIndividual3 // TODO: Add test for GTestIndividual3
			, GFunctionIndividual
			, GDelayIndividual
			, GExternalEvaluatorIndividual
		>;

		/*****************************************************************************************/

		add( BOOST_TEST_CASE_TEMPLATE( StandardTests_no_failure_expected, adaptor_types ) );
		add( BOOST_TEST_CASE_TEMPLATE( StandardTests_failures_expected, adaptor_types ) );

		add( BOOST_TEST_CASE_TEMPLATE( StandardTests_no_failure_expected, data_types ) );
		add( BOOST_TEST_CASE_TEMPLATE( StandardTests_failures_expected, data_types ) );

		add( BOOST_TEST_CASE_TEMPLATE( StandardTests_no_failure_expected, object_collection_types ) );
		add( BOOST_TEST_CASE_TEMPLATE( StandardTests_failures_expected, object_collection_types ) );

		add( BOOST_TEST_CASE_TEMPLATE( StandardTests_no_failure_expected, pod_collection_types ) );
		add( BOOST_TEST_CASE_TEMPLATE( StandardTests_failures_expected, pod_collection_types ) );

		// add( BOOST_TEST_CASE_TEMPLATE( StandardTests_no_failure_expected, algorithm_types ) );
		// add( BOOST_TEST_CASE_TEMPLATE( StandardTests_failures_expected, algorithm_types ) );

		add( BOOST_TEST_CASE_TEMPLATE( StandardTests_no_failure_expected, trait_types ) );
		add( BOOST_TEST_CASE_TEMPLATE( StandardTests_failures_expected, trait_types ) );

		add( BOOST_TEST_CASE_TEMPLATE( StandardTests_no_failure_expected, individual_types ) );
		add( BOOST_TEST_CASE_TEMPLATE( StandardTests_failures_expected, individual_types ) );
	}

    G_API_GENEVA ~GenevaStandardTestSuite() {
		std::cout << "GenevaStandardTestSuite has ended." << std::endl;
	}
};

/*************************************************************************************************/
/**
 * The test program entry point
 */
test_suite* init_unit_test_suite(int argc, char** const argv) {
	framework::master_test_suite().add(new GenevaStandardTestSuite());
	return 0;
}

/*************************************************************************************************/
