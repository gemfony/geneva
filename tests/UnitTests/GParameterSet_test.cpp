/**
 * @file GParameterSet_test.cpp
 */

/* Copyright (C) 2009 Dr. Ruediger Berlich
 *
 * This file is part of Geneva, Gemfony scientific's optimization library.
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
 */

// Standard header files go here
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

// Boost header files go here

#include <boost/test/unit_test.hpp>

#include <boost/shared_ptr.hpp>

// Geneva header files go here
#include "GenevaExceptions.hpp"
#include "GLogger.hpp"
#include "GLogTargets.hpp"
#include "GRandom.hpp"
#include "GParameterSet.hpp"
#include "GParabolaIndividual.hpp"
#include "GDoubleCollection.hpp"
#include "GDoubleGaussAdaptor.hpp"
#include "GStdVectorInterface_test.hpp"

using namespace Gem;
using namespace Gem::Util;
using namespace Gem::GenEvA;
using namespace Gem::GLogFramework;

/***********************************************************************************/
// This test suite checks as much as possible of the functionality provided
// by the GParameterSet class, using the GParabolaIndividual class. It also
// checks the functionality of the GMutableSetT and the GIndividual classes,
// as far as possible.

BOOST_AUTO_TEST_SUITE(GParameterSetSuite)

/***********************************************************************************/
// Test features that are expected to work
BOOST_AUTO_TEST_CASE( GParameterSet_no_failure_expected )
{
	GRandom gr;

	// Default construction
	GParabolaIndividual gpi;

	GDoubleCollection tempIItem(100, -10., 10.);
	boost::shared_ptr<GDoubleGaussAdaptor> gdga1(new GDoubleGaussAdaptor(1.,0.001,0.,1.));
	tempIItem.addAdaptor(gdga1);

	GDoubleCollection findItem(100, -10., 10.);
	boost::shared_ptr<GDoubleGaussAdaptor> gdga2(new GDoubleGaussAdaptor(2.,0.001,0.,2.));
	findItem.addAdaptor(gdga2);

	// Test the vector interface
	stdvectorinterfacetestSP(gpi, tempIItem, findItem);
}

/***********************************************************************************/
// Test features that are expected to fail
BOOST_AUTO_TEST_CASE( GParameterSet_failures_expected )
{
	GRandom gr;

	// Default construction
	GParabolaIndividual gpi;

	// Self-assignment should throw
	BOOST_CHECK_THROW(gpi.load(&gpi), Gem::GenEvA::geneva_error_condition);
}
/***********************************************************************************/

BOOST_AUTO_TEST_SUITE_END()
