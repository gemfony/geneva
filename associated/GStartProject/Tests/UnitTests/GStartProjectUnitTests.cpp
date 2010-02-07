/**
 * @file GStartProjectUnitTests.cpp
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

#include "GStartIndividual.hpp" // The class to be tested

#include "GStandard_test.hpp" // Contains the necessary tests

using namespace Gem;
using namespace Gem::Util;
using namespace Gem::GenEvA;


/************************************************************************************************/
//////////////////////////////////////////////////////////////////////////////////////////////////
/************************************************************************************************/
/**
 * As the GStartIndividual has a private default constructor, we need to provide a
 * specialization of the factory function that creates GStartProjectIndividual objects
 */
template <>
GStartIndividual *TFactory<GStartIndividual>() {
	return new GStartIndividual(100,-1.,1.);
}

/*************************************************************************************************/
/**
 * Performs specific modifications for this object. This is needed by the standard tests defined
 * by Geneva.
 * As we know that the object is filled
 * with data, we simply call mutate().
 *
 * @return A boolean indicating that a modification has indeed happened
 */
template <>
bool modify<GStartIndividual>(GStartIndividual& cp) {
	cp.mutate();
	return true;
}

/*************************************************************************************************/
/**
 * This function performs specific tests for GStartIndividual. Add further tests here when you
 * add functionality to your individual.
 */
template <>
void specificTestsFailuresExpected<GStartIndividual>() {
	const boost::uint32_t NITERATIONS=100;

	// Create an individual
	boost::shared_ptr<GStartIndividual> p(TFactory<GStartIndividual>());

	// Mutate a number of times and check that there were changes
	double oldfitness = p->fitness();
	for(boost::uint32_t i=0; i<NITERATIONS; i++) {
		p->mutate();
		double newfitness = p->fitness();
		BOOST_CHECK_MESSAGE(newfitness != oldfitness, "Rare failures are normal for this test / " << i);
		oldfitness = newfitness;
	}
}

/************************************************************************************************/
//////////////////////////////////////////////////////////////////////////////////////////////////
/************************************************************************************************/
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
		      GStartIndividual
		>
		gstartproject_types;

		/****************************************************************************************/

		add( BOOST_TEST_CASE_TEMPLATE( StandardTests_no_failure_expected,  gstartproject_types) );
		add( BOOST_TEST_CASE_TEMPLATE( StandardTests_failures_expected,  gstartproject_types) );
	}
};

/************************************************************************************************/
/**
 * The test program entry point
 */
test_suite* init_unit_test_suite(int argc, char** argv) {
   framework::master_test_suite().add(new GenevaStandardTestSuite());
   return 0;
}

/************************************************************************************************/
