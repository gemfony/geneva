/**
 * @file GBoundedDoubleCollection_test.cpp
 *
 * This test checks all public member functions of the GBoundedDoubleCollection class plus
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
#include <fstream>
#include <string>
#include <vector>
#include <utility>
#include <cmath>

// Boost header files go here

#define BOOST_TEST_MODULE GBoundedDoubleCollection_test
#include <boost/test/unit_test.hpp>

#include <boost/shared_ptr.hpp>

// Geneva header files go here
#include "GRandom.hpp"
#include "GBoundedDoubleCollection.hpp"
#include "GDoubleGaussAdaptor.hpp"

using namespace Gem;
using namespace Gem::GenEvA;
using namespace Gem::Util;

/***********************************************************************************/
/**
 * Tests the various member functions of the GBoundedDouble class and
 * parent classes.
 */
BOOST_AUTO_TEST_CASE(gboundeddouble_no_failure_expected) {
	// Get access to a random number generator
	GRandom gr;

	// Construction with two boundaries
	boost::shared_ptr<GBoundedDouble> gbd0(new GBoundedDouble(-10.,10.));

	BOOST_REQUIRE(gbd0->getLowerBoundary() == -10.);
	BOOST_REQUIRE(gbd0->getUpperBoundary() == 10.);

	// Construction with a value and two boundaries two boundaries
	boost::shared_ptr<GBoundedDouble> gbd1(new GBoundedDouble(5.,-11.,11.));

	BOOST_REQUIRE(gbd1->value() == 5.);
	BOOST_REQUIRE(gbd1->getLowerBoundary() == -11.);
	BOOST_REQUIRE(gbd1->getUpperBoundary() == 11.);

	// Construction via copy constructor, as a copy of gbd1
	boost::shared_ptr<GBoundedDouble> gbd2(new GBoundedDouble(*gbd1));

	BOOST_REQUIRE(*gbd2 == *gbd1);
	BOOST_REQUIRE(*gbd2 != *gbd0);

	BOOST_REQUIRE(gbd2->isSimilarTo(*gbd1));
	BOOST_REQUIRE(!gbd2->isSimilarTo(*gbd0));
}

/***********************************************************************************/
/**
 * Tests the various member functions of the GBoundedDoubleCollection class
 * and parent classes.
 */
BOOST_AUTO_TEST_CASE(gboundeddoublecollection_no_failure_expected)
{
	// Get access to a random number generator
	GRandom gr;

	// Default construction
	boost::shared_ptr<GBoundedDoubleCollection> gbdc(new GBoundedDoubleCollection);

	// Add GBoundedDouble objects to the object
}

/***********************************************************************************/

// EOF
