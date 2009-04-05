/**
 * @file GParameterTCollectionT_test.cpp
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
#include <cmath>
#include <algorithm>

// Boost header files go here

#include <boost/test/unit_test.hpp>
#include <boost/test/test_case_template.hpp>
#include <boost/mpl/list.hpp>

#include <boost/shared_ptr.hpp>
#include <boost/cstdint.hpp>

// Geneva header files go here
#include "GenevaExceptions.hpp"
#include "GLogger.hpp"
#include "GLogTargets.hpp"
#include "GRandom.hpp"
#include "GParameterT.hpp"
#include "GParameterTCollectionT.hpp"
#include "GBoolean.hpp"
#include "GBooleanAdaptor.hpp"
#include "GChar.hpp"
#include "GCharFlipAdaptor.hpp"
#include "GInt32.hpp"
#include "GInt32FlipAdaptor.hpp"
#include "GInt32GaussAdaptor.hpp"
#include "GDouble.hpp"
#include "GDoubleGaussAdaptor.hpp"
#include "GBoundedDouble.hpp"
#include "GDouble.hpp"
#include "GChar.hpp"
#include "GInt32.hpp"
#include "GBoolean.hpp"
#include "GBoundedDoubleCollection.hpp"
#include "GDoubleObjectCollection.hpp"
#include "GBooleanObjectCollection.hpp"
#include "GCharObjectCollection.hpp"
#include "GInt32ObjectCollection.hpp"
#include "GStdVectorInterface_test.hpp"

using namespace Gem;
using namespace Gem::Util;
using namespace Gem::GenEvA;
using namespace Gem::GLogFramework;

/***********************************************************************************/
// This test suite checks as much as possible of the functionality provided
// by the GParameterT class. Tests include features of the parent class
// GParameterBaseWithAdaptorsT, as it cannot be instantiated itself.
BOOST_AUTO_TEST_SUITE(GParameterTCollectionTSuite)

typedef boost::mpl::list<GDouble, GBoundedDouble, GChar, GInt32, GBoolean> test_types;

/***********************************************************************************/
// Test features that are expected to work
BOOST_AUTO_TEST_CASE_TEMPLATE( GParameterTCollectionT_no_failure_expected, T, test_types )
{
	GRandom gr;

	// Default construction
	GParameterTCollectionT<T> gptct;
}

/***********************************************************************************/
// Test features that are expected to work - GDouble case
BOOST_AUTO_TEST_CASE( GParameterTCollectionT_GDouble_no_failure_expected)
{
	GRandom gr;

	// Default construction
	GParameterTCollectionT<GDouble> gptct;

	// Check the vector interface
	GDouble templItem(0);
	GDouble findItem(1);
	stdvectorinterfacetestSP(gptct, templItem, findItem);
}

/***********************************************************************************/
// Test features that are expected to work - GBoundedDouble case
BOOST_AUTO_TEST_CASE( GParameterTCollectionT_GBoundedDouble_no_failure_expected)
{
	GRandom gr;

}

/***********************************************************************************/
// Test features that are expected to work - GChar case
BOOST_AUTO_TEST_CASE( GParameterTCollectionT_GChar_no_failure_expected)
{
	GRandom gr;

}

/***********************************************************************************/
// Test features that are expected to work - GInt32 case
BOOST_AUTO_TEST_CASE( GParameterTCollectionT_GInt32_no_failure_expected)
{
	GRandom gr;

}

/***********************************************************************************/
// Test features that are expected to work - GDouble case
BOOST_AUTO_TEST_CASE( GParameterTCollectionT_GBoolean_no_failure_expected)
{
	GRandom gr;

}

/***********************************************************************************/
// Test features that are expected to fail
BOOST_AUTO_TEST_CASE( GParmeterTCollectionT_failures_expected)
{
	GRandom gr;


	// Self assignment should throw
}
/***********************************************************************************/

BOOST_AUTO_TEST_SUITE_END()
