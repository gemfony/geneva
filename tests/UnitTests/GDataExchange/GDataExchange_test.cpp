/**
 * @file GDataExchange_test.cpp
 *
 * This test checks all public member functions of the GDataExchange class plus
 * dependent classes. You should run this test both in DEBUG and in RELEASE mode,
 * as some functions may work differently in this case.
 */

/* Copyright (C)2009 Dr. Ruediger Berlich
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
#include <utility>

// Boost header files go here

#define BOOST_TEST_MODULE GDataExchange_test
#include <boost/test/unit_test.hpp>

#include <boost/shared_ptr.hpp>

// Geneva header files go here
#include "GRandom.hpp"
#include "GDataExchange.hpp"

using namespace Gem;
using namespace Gem::GenEvA;
using namespace Gem::Util;

/***********************************************************************************/
/**
 * Some explanation
 */
BOOST_AUTO_TEST_CASE( gdga_gobject_test_no_failure_expected )
{

}

/***********************************************************************************/

// EOF
