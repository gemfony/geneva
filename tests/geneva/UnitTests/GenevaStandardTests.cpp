/**
 * @file GenevaStandardTests.cpp
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt)
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) com
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
 * http://www.gemfony.com .
 */

#include <iostream>

#include <boost/test/unit_test.hpp>
#include <boost/cstdint.hpp>

using boost::unit_test_framework::test_suite;
using namespace boost::unit_test;

// All classes that will be tested in this file
#include "geneva/GInt32FlipAdaptor.hpp"
#include "geneva/GBooleanAdaptor.hpp"
#include "geneva/GInt32GaussAdaptor.hpp"
#include "geneva/GFloatBiGaussAdaptor.hpp"
#include "geneva/GFloatCollection.hpp"
#include "geneva/GFloatObject.hpp"
#include "geneva/GFloatObjectCollection.hpp"
#include "geneva/GDoubleBiGaussAdaptor.hpp"
#include "geneva/GDoubleGaussAdaptor.hpp"
#include "geneva/GBooleanObjectCollection.hpp"
#include "geneva/GInt32ObjectCollection.hpp"
#include "geneva/GDoubleObjectCollection.hpp"
#include "geneva/GConstrainedFloatCollection.hpp"
#include "geneva/GConstrainedFloatObject.hpp"
#include "geneva/GConstrainedFloatObjectCollection.hpp"
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
#include "geneva/GSerialEA.hpp"
#include "geneva/GMultiThreadedEA.hpp"
#include "geneva/GEAPersonalityTraits.hpp"
#include "geneva/GGDPersonalityTraits.hpp"
#include "geneva/GSwarmPersonalityTraits.hpp"
#include "geneva/GSerialSwarm.hpp"
#include "geneva/GMultiThreadedSwarm.hpp"
#include "geneva/GSerialGD.hpp"
#include "geneva/GMultiThreadedGD.hpp"
#include "geneva/GParameterObjectCollection.hpp"
#include "geneva/Go.hpp"
#include "geneva/GParameterSet.hpp"
#include "geneva/GEvaluator.hpp"
#include "geneva-individuals/GTestIndividual1.hpp"

#include "GStandard_test.hpp"

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

		typedef boost::mpl::list<
			GInt32FlipAdaptor
			, GBooleanAdaptor
			, GInt32GaussAdaptor
			//, GFloatBiGaussAdaptor
			//, GFloatGaussAdaptor
			, GDoubleBiGaussAdaptor
			, GDoubleGaussAdaptor
		>
		adaptor_types;

		typedef boost::mpl::list<
			GBooleanObject
			, GInt32Object
			//, GFloatObject
			, GDoubleObject
			, GConstrainedInt32Object
			//, GConstrainedFloatObject
			, GConstrainedDoubleObject
		>
		data_types;

		typedef boost::mpl::list<
			GParameterObjectCollection
			, GBooleanObjectCollection
			, GInt32ObjectCollection
			, GConstrainedInt32ObjectCollection
			//, GFloatObjectCollection
			, GDoubleObjectCollection
			//, GConstrainedFloatObjectCollection
			, GConstrainedDoubleObjectCollection
		>
		object_collection_types;

		typedef boost::mpl::list<
			GInt32Collection
			//, GFloatCollection
			, GDoubleCollection
			, GBooleanCollection
			//, GConstrainedFloatCollection
			, GConstrainedDoubleCollection
			, GParameterSet
		>
		pod_collection_types;

		typedef boost::mpl::list<
			GSerialEA
			, GMultiThreadedEA
			, GSerialSwarm
			, GMultiThreadedSwarm
			, GSerialGD
			, GMultiThreadedGD
			, Go
			, GEvaluator
		>
		algorithm_types;

		typedef boost::mpl::list<
			GEAPersonalityTraits
			, GGDPersonalityTraits
			, GSwarmPersonalityTraits
		>
		trait_types;

		typedef boost::mpl::list<
			GOptimizationAlgorithmT<GIndividual>::GOptimizationMonitorT
			, GOptimizationAlgorithmT<GParameterSet>::GOptimizationMonitorT
			, GSerialEA::GEAOptimizationMonitor
			, GSerialSwarm::GSwarmOptimizationMonitor
			, GSerialGD::GGDOptimizationMonitor
		>
		monitor_types;

		typedef boost::mpl::list<
			Gem::Tests::GTestIndividual1
		>
		individual_types;

		/*****************************************************************************************/

		add( BOOST_TEST_CASE_TEMPLATE( StandardTests_no_failure_expected, adaptor_types ) );
		add( BOOST_TEST_CASE_TEMPLATE( StandardTests_failures_expected, adaptor_types ) );

		add( BOOST_TEST_CASE_TEMPLATE( StandardTests_no_failure_expected, data_types ) );
		add( BOOST_TEST_CASE_TEMPLATE( StandardTests_failures_expected, data_types ) );

		add( BOOST_TEST_CASE_TEMPLATE( StandardTests_no_failure_expected, object_collection_types ) );
		add( BOOST_TEST_CASE_TEMPLATE( StandardTests_failures_expected, object_collection_types ) );

		add( BOOST_TEST_CASE_TEMPLATE( StandardTests_no_failure_expected, pod_collection_types ) );
		add( BOOST_TEST_CASE_TEMPLATE( StandardTests_failures_expected, pod_collection_types ) );

		add( BOOST_TEST_CASE_TEMPLATE( StandardTests_no_failure_expected, algorithm_types ) );
		add( BOOST_TEST_CASE_TEMPLATE( StandardTests_failures_expected, algorithm_types ) );

		add( BOOST_TEST_CASE_TEMPLATE( StandardTests_no_failure_expected, trait_types ) );
		add( BOOST_TEST_CASE_TEMPLATE( StandardTests_failures_expected, trait_types ) );

		add( BOOST_TEST_CASE_TEMPLATE( StandardTests_no_failure_expected, monitor_types ) );
		add( BOOST_TEST_CASE_TEMPLATE( StandardTests_failures_expected, monitor_types ) );

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
test_suite* init_unit_test_suite(int argc, char** argv) {
   framework::master_test_suite().add(new GenevaStandardTestSuite());
   return 0;
}

/*************************************************************************************************/
