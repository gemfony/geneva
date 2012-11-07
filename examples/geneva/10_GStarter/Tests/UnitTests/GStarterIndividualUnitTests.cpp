/**
 * @file GNeuralNetworkUnitTests.cpp
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


#include <boost/test/unit_test.hpp>
#include <boost/cstdint.hpp>
#include <boost/shared_ptr.hpp>

using boost::unit_test_framework::test_suite;
using namespace boost::unit_test;

// The class to be tested
#include "../../GStarterIndividual.hpp"

// Contains the necessary tests
#include "GStandard_test.hpp"

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
		typedef boost::mpl::list<
		      GStarterIndividual
		>
		userind_types;

		/****************************************************************************************/

		add( BOOST_TEST_CASE_TEMPLATE( StandardTests_no_failure_expected,  userind_types) );
		add( BOOST_TEST_CASE_TEMPLATE( StandardTests_failures_expected,  userind_types) );
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
