/**
 * @file GenevaStandardTests.cpp
 */

/*
 * Copyright (C) Authors of the Geneva library collection and Karlsruhe
 * Institute of Technology (University of the State of Baden-Wuerttemberg
 * and National Laboratory of the Helmholtz Association).
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: info [at] gemfony (dot) com
 *
 * This file is part of the Geneva library collection
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

#include <iostream>

#include <boost/test/unit_test.hpp>
#include <boost/cstdint.hpp>

using boost::unit_test_framework::test_suite;
using namespace boost::unit_test;

// All classes that will be tested in this file
#include "geneva/GInt32FlipAdaptor.hpp"
#include "geneva/GBooleanAdaptor.hpp"
#include "geneva/GInt32GaussAdaptor.hpp"
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
#include "geneva/GConstrainedDoubleObject.hpp"
#include "geneva/GInt32Collection.hpp"
#include "geneva/GDoubleCollection.hpp"
#include "geneva/GBooleanCollection.hpp"
#include "geneva/GEvolutionaryAlgorithm.hpp"
#include "geneva/GMultiThreadedEA.hpp"
#include "geneva/GEAPersonalityTraits.hpp"
#include "geneva/GGDPersonalityTraits.hpp"
#include "geneva/GSwarmPersonalityTraits.hpp"
#include "geneva/GSwarm.hpp"
#include "geneva/GMultiThreadedSwarm.hpp"
#include "geneva/GGradientDescent.hpp"
#include "geneva/GMultiThreadedGD.hpp"
#include "geneva/GParameterObjectCollection.hpp"
#include "geneva/Go.hpp"
#include "geneva-individuals/GTestIndividual1.hpp"

#include "GStandard_test.hpp"

using namespace Gem::Geneva;

// For reasons that are not understood, some export statements in the
// .cpp files do not get pulled in here. We get an error "unregistered class"
// when these statements are not present below.
#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT(GGradientDescent);
BOOST_CLASS_EXPORT(GSwarm);
BOOST_CLASS_EXPORT(GOptimizationAlgorithmT<GIndividual>::GOptimizationMonitorT);
BOOST_CLASS_EXPORT(GOptimizationAlgorithmT<GParameterSet>::GOptimizationMonitorT);
BOOST_CLASS_EXPORT(GEvolutionaryAlgorithm::GEAOptimizationMonitor);
BOOST_CLASS_EXPORT(GSwarm::GSwarmOptimizationMonitor);
BOOST_CLASS_EXPORT(GGradientDescent::GGDOptimizationMonitor);
BOOST_CLASS_EXPORT(Go);

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
			, GDoubleGaussAdaptor
		>
		adaptor_types;

		typedef boost::mpl::list<
			GParameterObjectCollection
			, GBooleanObjectCollection
			, GInt32ObjectCollection
			, GConstrainedInt32ObjectCollection
			, GDoubleObjectCollection
			, GConstrainedDoubleObjectCollection
			, GBooleanObject
			, GInt32Object
			, GDoubleObject
			, GConstrainedInt32Object
			, GConstrainedDoubleObject
			, GInt32Collection
			, GDoubleCollection
			, GBooleanCollection
		>
		data_types;

		typedef boost::mpl::list<
			GEvolutionaryAlgorithm
			, GMultiThreadedEA
			, GSwarm
			, GMultiThreadedSwarm
			, GGradientDescent
			, GMultiThreadedGD
			, Go
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
			, GEvolutionaryAlgorithm::GEAOptimizationMonitor
			, GSwarm::GSwarmOptimizationMonitor
			, GGradientDescent::GGDOptimizationMonitor
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
