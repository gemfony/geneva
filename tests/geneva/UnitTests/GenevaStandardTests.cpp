/**
 * @file GenevaStandardTests.cpp
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
#include "geneva/G_OA_GradientDescent_PersonalityTraits.hpp"
#include "geneva/G_OA_ParameterScan_PersonalityTraits.hpp"
#include "geneva/G_OA_SwarmAlgorithm_PersonalityTraits.hpp"
#include "geneva/GParameterObjectCollection.hpp"
#include "geneva/GParameterSet.hpp"
#include "geneva/GTestIndividual1.hpp"
#include "geneva-individuals/GTestIndividual3.hpp"
#include "geneva-individuals/GFunctionIndividual.hpp"
#include "geneva-individuals/GDelayIndividual.hpp"
#include "geneva-individuals/GExternalEvaluatorIndividual.hpp"

#include "geneva/tests/GStandard_test.hpp"

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
	GenevaStandardTestSuite() :test_suite("GenevaStandardTestSuite") {

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
			, GParameterSet
		>;

		// TODO: Add tests for algorithm types
		using algorithm_types = boost::mpl::list<
		>;

		using trait_types = boost::mpl::list<
			G_OA_EvolutionaryAlgorithm_PersonalityTraits
			, G_OA_GradientDescent_PersonalityTraits
			, G_OA_SwarmAlgorithm_PersonalityTraits
			, G_OA_SimulatedAnnealing_PersonalityTraits
			, G_OA_ParameterScan_PersonalityTraits
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

	~GenevaStandardTestSuite() {
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
