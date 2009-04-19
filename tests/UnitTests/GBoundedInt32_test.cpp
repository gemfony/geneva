/**
 * @file GBoundedInt32_test.cpp
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
#include "GBoundedInt32.hpp"
#include "GInt32FlipAdaptor.hpp"

using namespace Gem;
using namespace Gem::Util;
using namespace Gem::GenEvA;
using namespace Gem::GLogFramework;

/***********************************************************************************/
// This test suite checks as much as possible of the functionality provided
// by the GBoundedInt32 class. Please also have a look at the manual test,
// as it gives a graphical representation of the mapping.
BOOST_AUTO_TEST_SUITE(GBoundedInt32Suite)

const std::size_t NMUTATIONS=10000;

/***********************************************************************************/
// Test features that are expected to work
BOOST_AUTO_TEST_CASE( GBoundedInt32_no_failure_expected )
{
	GRandom gr;

	// Test instantiation in different modes
	GBoundedInt32 gbi0;
	GBoundedInt32 gbi1(-10,10);
	GBoundedInt32 gbi2(1,-10,10);
	GBoundedInt32 gbi7(3); // has maximum boundaries
	GBoundedInt32 gbi3(gbi2);

	BOOST_CHECK(gbi3 == gbi2);
	if(gbi1.value() != 1) BOOST_CHECK(gbi2 != gbi1); // gbi1 may randomly be assigned the value, which would result in an error being erroneously reported
	BOOST_CHECK(gbi2 != gbi0);
	BOOST_CHECK(gbi1 != gbi0);
	BOOST_CHECK(gbi7 != gbi0);

	// Check that value calculation works repeatedly. Internal value should be == external value for gbi7
	const std::size_t NCHECKS=10000;
	for(std::size_t i=0; i<NCHECKS; i++) {
		boost::int32_t in=boost::int32_t(-5000.+10000.*double(i)/double(NCHECKS)), out = 0;
		BOOST_CHECK_NO_THROW(out = gbi7.calculateExternalValue(in));
		BOOST_CHECK(in==out);
	}
	// Try resetting the boundaries to a finite value (which includes the current external value)
	BOOST_CHECK_NO_THROW(gbi7.setBoundaries(-6000,6000));
	BOOST_CHECK_NO_THROW(gbi7 = 10);
	BOOST_CHECK_NO_THROW(gbi7.setBoundaries(-10,10));

	// (Repeated) assignment
	GBoundedInt32 gbi3_2;
	gbi3_2 = gbi3 = gbi0;
	BOOST_CHECK(gbi3 != gbi2);
	BOOST_CHECK(gbi3 == gbi0);
	BOOST_CHECK(gbi3_2 != gbi2);
	BOOST_CHECK(gbi3_2 == gbi0);

	// Cloning and loading
	GObject * gbi4;
	BOOST_CHECK_NO_THROW(gbi4 = gbi3.clone());
	GBoundedInt32 gbi5;
	BOOST_CHECK_NO_THROW(gbi5.load(gbi4));
	BOOST_CHECK(gbi5 == gbi3);
	delete gbi4;

	// Value assignment
	gbi5=gbi1;
	BOOST_CHECK(gbi5 == gbi1);
	gbi5 = 2;
	BOOST_CHECK(gbi5.value() == 2);
	BOOST_CHECK(!gbi5.isEqualTo(gbi3));
	BOOST_CHECK(gbi5.getLowerBoundary() ==  -10);
	BOOST_CHECK(gbi5.getUpperBoundary() ==  10);

	// Test automatic conversion to boost::int32_t
	boost::int32_t val=0;
	val = gbi5;
	BOOST_CHECK(val == 2);

	// Attach an adaptor to the integer and check that no values outside of the
	// value range occur
	GBoundedInt32 mutTest(2, 1, 5);
	boost::shared_ptr<GInt32FlipAdaptor> gifa(new GInt32FlipAdaptor);
	mutTest.addAdaptor(gifa);
	for(std::size_t i=0; i<NMUTATIONS; i++) {
		mutTest.mutate();
		BOOST_CHECK(mutTest.value() >= 1 && mutTest.value() <= 5);
	}

	// Check that input of bounded int leads to the desired output (in this
	// case same number of entries in the vector
	std::vector<boost::int32_t> iVec(20,0);
	GBoundedInt32 cc(-10,9); // cc == "checkConversion; 20 values
	for(boost::int32_t i=0; i<1000; i++) {
		boost::int32_t val =  cc.calculateExternalValue(-10+i%20);
		iVec[boost::numeric_cast<std::size_t>(10 + val)] += 1;
	}
	for(std::size_t i=1; i<20; i++) {
		BOOST_CHECK(iVec[i] == iVec[i-1]);
	}

	// Test serialization and loading in different serialization modes
	{ // plain text format
		// Copy construction of a new object
		GBoundedInt32 gbi6(0,-10,10);
		GBoundedInt32 gbi6_cp(gbi6);

		// Check equalities and inequalities
		BOOST_CHECK(gbi6_cp == gbi6);
		// Re-assign a new value to gbi6_cp
		gbi6_cp = 1;
		BOOST_CHECK(gbi6_cp.value() == 1);
		BOOST_CHECK(gbi6_cp != gbi6);

		// Serialize gbi6 and load into gbi6_co, check equalities and similarities
		BOOST_REQUIRE_NO_THROW(gbi6_cp.fromString(gbi6.toString(TEXTSERIALIZATION), TEXTSERIALIZATION));
		BOOST_CHECK(gbi6_cp.isSimilarTo(gbi6, exp(-10)));
	}

	{ // XML format
		// Copy construction of a new object
		GBoundedInt32 gbi6(0,-10,10);
		GBoundedInt32 gbi6_cp(gbi6);

		// Check equalities and inequalities
		BOOST_CHECK(gbi6_cp == gbi6);
		// Re-assign a new value to gbi6_cp
		gbi6_cp = 1;
		BOOST_CHECK(gbi6_cp.value() == 1);
		BOOST_CHECK(gbi6_cp != gbi6);

		// Serialize gbi6 and load into gbi6_co, check equalities and similarities
		BOOST_REQUIRE_NO_THROW(gbi6_cp.fromString(gbi6.toString(XMLSERIALIZATION), XMLSERIALIZATION));
		BOOST_CHECK(gbi6_cp.isSimilarTo(gbi6, exp(-10)));
	}

	{ // binary test format
		// Copy construction of a new object
		GBoundedInt32 gbi6(0,-10,10);
		GBoundedInt32 gbi6_cp(gbi6);

		// Check equalities and inequalities
		BOOST_CHECK(gbi6_cp == gbi6);
		// Re-assign a new value to gbi6_cp
		gbi6_cp = 1;
		BOOST_CHECK(gbi6_cp.value() == 1);
		BOOST_CHECK(gbi6_cp != gbi6);

		// Serialize gbi6 and load into gbi6_co, check equalities and similarities
		BOOST_REQUIRE_NO_THROW(gbi6_cp.fromString(gbi6.toString(BINARYSERIALIZATION), BINARYSERIALIZATION));
		BOOST_CHECK(gbi6_cp.isEqualTo(gbi6));
	}
}

/***********************************************************************************/
// Test features that are expected to fail
BOOST_AUTO_TEST_CASE( GBoundedInt32_failures_expected )
{
	GRandom gr;

	// Assignment of value outside of the allowed range
	GBoundedInt32 gbi(-10,10);
	BOOST_CHECK_THROW(gbi=11, Gem::GenEvA::geneva_error_condition);

	// Self assignment should throw
	BOOST_CHECK_THROW(gbi.load(&gbi), Gem::GenEvA::geneva_error_condition);
}
/***********************************************************************************/

BOOST_AUTO_TEST_SUITE_END()
