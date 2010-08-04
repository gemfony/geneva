/**
 * @file GenevaStandardTests.cpp
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

#include <boost/test/unit_test.hpp>
#include <boost/cstdint.hpp>

using boost::unit_test_framework::test_suite;
using namespace boost::unit_test;

// All classes that will be tested in this file
#include "geneva/GIdentityAdaptorT.hpp"
#include "geneva/GInt32FlipAdaptor.hpp"
#include "geneva/GBooleanAdaptor.hpp"
#include "geneva/GInt32GaussAdaptor.hpp"
#include "geneva/GDoubleGaussAdaptor.hpp"
#include "geneva/GBooleanObjectCollection.hpp"
#include "geneva/GInt32ObjectCollection.hpp"
#include "geneva/GDoubleObjectCollection.hpp"
#include "geneva/GConstrainedDoubleCollection.hpp"
#include "geneva/GConstrainedInt32Collection.hpp"
#include "geneva/GBoolean.hpp"
#include "geneva/GConstrainedInt32.hpp"
#include "geneva/GInt32.hpp"
#include "geneva/GDouble.hpp"
#include "geneva/GConstrainedDouble.hpp"
#include "geneva/GConstrainedDouble.hpp"
#include "geneva/GConstrainedInt32.hpp"
#include "geneva/GInt32Collection.hpp"
#include "geneva/GDoubleCollection.hpp"
#include "geneva/GBooleanCollection.hpp"
#include "geneva/GEvolutionaryAlgorithm.hpp"
#include "geneva/GMultiThreadedEA.hpp"
#include "geneva/GEAPersonalityTraits.hpp"
#include "geneva/GGDPersonalityTraits.hpp"
#include "geneva/GSwarmPersonalityTraits.hpp"

#include "GStandard_test.hpp"

// For reasons that are not understood the export statements in the
// .cpp files do not get pulled in here. We get an error "unregistered class"
// when these two statements are not present below.
#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT(Gem::Geneva::GEvolutionaryAlgorithm)

using namespace Gem::Hap;
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
		      GIdentityAdaptorT<boost::int32_t>
			, GInt32FlipAdaptor
			, GBooleanAdaptor
			, GInt32GaussAdaptor
			, GDoubleGaussAdaptor
		>
		adaptor_types;

		typedef boost::mpl::list<
			  GBooleanObjectCollection
			, GInt32ObjectCollection
			, GDoubleObjectCollection
			, GConstrainedDoubleCollection
			, GBoolean
			, GConstrainedDouble
			, GConstrainedInt32
			, GConstrainedInt32Collection
			, GDouble
			, GInt32
			, GInt32Collection
			, GDoubleCollection
			, GBooleanCollection
		>
		data_types;

		typedef boost::mpl::list<
			  GEvolutionaryAlgorithm
			, GMultiThreadedEA
		>
		algorithm_types;

		typedef boost::mpl::list<
			  GEAPersonalityTraits
			, GGDPersonalityTraits
			, GSwarmPersonalityTraits
		>
		trait_types;

		/*****************************************************************************************/

		add( BOOST_TEST_CASE_TEMPLATE( StandardTests_no_failure_expected, adaptor_types ) );
		add( BOOST_TEST_CASE_TEMPLATE( StandardTests_failures_expected, adaptor_types ) );

		add( BOOST_TEST_CASE_TEMPLATE( StandardTests_no_failure_expected, data_types ) );
		add( BOOST_TEST_CASE_TEMPLATE( StandardTests_failures_expected, data_types ) );

		add( BOOST_TEST_CASE_TEMPLATE( StandardTests_no_failure_expected, algorithm_types ) );
		add( BOOST_TEST_CASE_TEMPLATE( StandardTests_failures_expected, algorithm_types ) );

		add( BOOST_TEST_CASE_TEMPLATE( StandardTests_no_failure_expected, trait_types ) );
		add( BOOST_TEST_CASE_TEMPLATE( StandardTests_failures_expected, trait_types ) );
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
