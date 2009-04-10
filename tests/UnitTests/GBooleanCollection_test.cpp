/**
 * @file GBooleanCollection_test.cpp
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

// Geneva header files go here
#include "GenevaExceptions.hpp"
#include "GLogger.hpp"
#include "GLogTargets.hpp"
#include "GRandom.hpp"
#include "GBooleanCollection.hpp"
#include "GBooleanAdaptor.hpp"
#include "GStdSimpleVectorInterfaceT.hpp"
#include "GStdVectorInterface_test.hpp"

using namespace Gem;
using namespace Gem::Util;
using namespace Gem::GenEvA;
using namespace Gem::GLogFramework;

/***********************************************************************************/
// This test suite checks as much as possible of the functionality provided
// by the GBooleanCollection class, as well as some important parent classes
// (GParameterCollectionT and GStdSimpleVectorInterface).
BOOST_AUTO_TEST_SUITE(GBooleanCollectionSuite)

/***********************************************************************************/
/**
 * A simple class that allows a self-test of GStdSimpleVectorInterface
 */
namespace Gem {
namespace GenEvA {

template <typename T>
class viTestT
	:public GStdSimpleVectorInterfaceT<T>
{
public:
	inline void swap(viTestT<T>& cp) { GStdSimpleVectorInterfaceT<T>::swap(cp.data); }

protected:
	virtual void dummyFunction() { /* nothing */ }
};

} /* namespace GenEvA */
} /* namespace Gem */

/***********************************************************************************/
/**
 * Test that GStdSimpleVectorInterface<T> does its job
 */

typedef boost::mpl::list<bool, boost::int32_t, double, char> test_types;

BOOST_AUTO_TEST_CASE_TEMPLATE( GStdSimpleVectorInterfaceT_no_failure_expected, T, test_types )
{
	Gem::GenEvA::viTestT<T> vt;
	T templItem = T(0);
	T findItem = T(1);
	// Make sure both items are indeed different
	stdvectorinterfacetest(vt, templItem, findItem);
}

/***********************************************************************************/
// Test features that are expected to work
BOOST_AUTO_TEST_CASE(GBooleanCollection_no_failure_expected )
{
	GRandom gr;

	// Construction in different modes
	GBooleanCollection gbc0; // default construction, should be empty
	BOOST_CHECK(gbc0.empty());

	// Check the vector interface
	bool templItem = false;
	bool findItem = true;
	// Test the functionality of the underlying vector implementation
	stdvectorinterfacetest<GBooleanCollection, bool>(gbc0, templItem, findItem);

	GBooleanCollection gbc1(100);  // 100 items
	GBooleanCollection gbc1_2(100);  // 100 items
	BOOST_CHECK(gbc1.size() == 100);
	BOOST_CHECK(gbc1_2.size() == 100);
	BOOST_CHECK(gbc1 != gbc1_2);

	GBooleanCollection gbc2(100,0.7);  // 100 items, "true" is preferred
	GBooleanCollection gbc2_2(100,0.7);  // 100 items, "true" is preferred
	BOOST_CHECK(gbc2.size() == 100);
	BOOST_CHECK(gbc2_2.size() == 100);
	BOOST_CHECK(gbc2 != gbc2_2);

	// Copy construction
	GBooleanCollection gbc3(gbc2);
	BOOST_CHECK(gbc3 == gbc2);

	// Assignment
	GBooleanCollection gbc4;
	gbc4 = gbc3;
	BOOST_CHECK(gbc4 == gbc2);

	// Cloning and loading
	GObject *gbc5 = gbc4.clone();
	GBooleanCollection gbc6;
	gbc6.load(gbc5);
	delete gbc5;
	BOOST_CHECK(gbc6 == gbc2);

	// Adding random data using two different methods
	gbc6.addRandomData(100);
	BOOST_CHECK(gbc6 != gbc2);
	BOOST_CHECK(gbc6.size() == 200);
	gbc6.addRandomData(1800, 0.1);
	BOOST_CHECK(gbc6.size() == 2000);

	// Adding an adaptor
	boost::shared_ptr<GBooleanAdaptor> gba(new GBooleanAdaptor());
	gbc6.addAdaptor(gba);

	const std::size_t NMUTATIONS=1000;
	GBooleanCollection gbc6_old(gbc6);
	for(std::size_t i=0; i<NMUTATIONS; i++) {
		gbc6.mutate();
		BOOST_CHECK(gbc6 != gbc6_old);
		gbc6_old = gbc6;
	}

	// Test of serialization in different modes
	// Test serialization and loading in different serialization modes
	{ // plain text format
		// Copy construction of a new object
		GBooleanCollection gbc7(100);
		GBooleanCollection gbc7_cp(gbc7);

		// Check equalities and inequalities
		BOOST_CHECK(gbc7_cp == gbc7);
		// Re-assign a new value to gbc7_cp
		gbc7_cp.addRandomData(100);
		BOOST_CHECK(gbc7_cp.size() == 200);
		BOOST_CHECK(gbc7_cp != gbc7);

		// Serialize gbc7 and load into gbc7_co, check equalities and similarities
		BOOST_REQUIRE_NO_THROW(gbc7_cp.fromString(gbc7.toString(TEXTSERIALIZATION), TEXTSERIALIZATION));
		BOOST_CHECK(gbc7_cp.isSimilarTo(gbc7, exp(-10)));
	}

	{ // XML format
		// Copy construction of a new object
		GBooleanCollection gbc7(100);
		GBooleanCollection gbc7_cp(gbc7);

		// Check equalities and inequalities
		BOOST_CHECK(gbc7_cp == gbc7);
		// Re-assign a new value to gbc7_cp
		gbc7_cp.addRandomData(100);
		BOOST_CHECK(gbc7_cp.size() == 200);
		BOOST_CHECK(gbc7_cp != gbc7);

		// Serialize gbc7 and load into gbc7_co, check equalities and similarities
		BOOST_REQUIRE_NO_THROW(gbc7_cp.fromString(gbc7.toString(XMLSERIALIZATION), XMLSERIALIZATION));
		BOOST_CHECK(gbc7_cp.isSimilarTo(gbc7, exp(-10)));
	}

	{ // binary test format
		// Copy construction of a new object
		GBooleanCollection gbc7(100);
		GBooleanCollection gbc7_cp(gbc7);

		// Check equalities and inequalities
		BOOST_CHECK(gbc7_cp == gbc7);
		// Re-assign a new value to gbc7_cp
		gbc7_cp.addRandomData(100);
		BOOST_CHECK(gbc7_cp.size() == 200);
		BOOST_CHECK(gbc7_cp != gbc7);

		// Serialize gbc7 and load into gbc7_co, check equalities and similarities
		BOOST_REQUIRE_NO_THROW(gbc7_cp.fromString(gbc7.toString(BINARYSERIALIZATION), BINARYSERIALIZATION));
		BOOST_CHECK(gbc7_cp.isEqualTo(gbc7));
	}
}

/***********************************************************************************/
// Test features that are expected to fail
BOOST_AUTO_TEST_CASE( GBooleanCollection_failures_expected )
{
	GRandom gr;

	// Self-assignment should throw
	GBooleanCollection gbc(100);
	BOOST_CHECK_THROW(gbc.load(&gbc), geneva_error_condition);
}
/***********************************************************************************/

BOOST_AUTO_TEST_SUITE_END()
