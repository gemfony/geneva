/**
 * @file GExternalEvaluatorUnitTests.cpp
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

#include <boost/test/unit_test.hpp>
#include <boost/cstdint.hpp>
#include <boost/shared_ptr.hpp>

using boost::unit_test_framework::test_suite;
using namespace boost::unit_test;

// The class to be tested
#include "geneva-individuals/GExternalEvaluatorIndividual.hpp"

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
 * All instantiable core Geneva classes should be listed here.
 */
class GenevaStandardTestSuite
	: public test_suite
{
public:
	GenevaStandardTestSuite() :test_suite("GenevaStandardTestSuite") {
		typedef boost::mpl::list<
		      GExternalEvaluatorIndividual
		>
		gexternalevaluator_types;

		/****************************************************************************************/

		add( BOOST_TEST_CASE_TEMPLATE( StandardTests_no_failure_expected,  gexternalevaluator_types) );
		add( BOOST_TEST_CASE_TEMPLATE( StandardTests_failures_expected,  gexternalevaluator_types) );
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
