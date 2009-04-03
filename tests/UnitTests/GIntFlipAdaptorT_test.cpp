/**
 * @file GIntFlipAdaptorT_test.cpp
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
#include <boost/test/test_case_template.hpp>
#include <boost/mpl/list.hpp>

#include <boost/shared_ptr.hpp>
#include <boost/cstadint.hpp>

// Geneva header files go here
#include "GenevaExceptions.hpp"
#include "GLogger.hpp"
#include "GLogTargets.hpp"
#include "GRandom.hpp"
#include "GIntFlipAdaptorT.hpp"

using namespace Gem;
using namespace Gem::Util;
using namespace Gem::GenEvA;
using namespace Gem::GLogFramework;

typedef boost::mpl::list<boost::int32_t, bool,  char> test_types;

/***********************************************************************************/
// This test suite checks as much as possible of the functionality provided
// by the GIntFlipAdaptorT class.
BOOST_AUTO_TEST_SUITE(GIntFlipAdaptorT)

/***********************************************************************************/
// Test features that are expected to work
BOOST_AUTO_TEST_CASE_TEMPLATE( GIntFlipAdaptorT_no_failure_expected, T, test_types )
{
	GRandom gr;

}

/***********************************************************************************/
// Test features that are expected to fail
BOOST_AUTO_TEST_CASE_TEMPLATE( GIntFlipAdaptorT_failures_expected, T, test_types )
{
	GRandom gr;

}
/***********************************************************************************/

BOOST_AUTO_TEST_SUITE_END()
