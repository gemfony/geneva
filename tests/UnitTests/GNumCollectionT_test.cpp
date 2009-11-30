/**
 * @file GNumCollectionT_test.cpp
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
#include "GRandom.hpp"
#include "GNumCollectionT.hpp"
#include "GInt32Collection.hpp"
#include "GDoubleCollection.hpp"
#include "GGaussAdaptorT.hpp"
#include "GStdSimpleVectorInterfaceT.hpp"
#include "GStdVectorInterface_test.hpp"

using namespace Gem;
using namespace Gem::Util;
using namespace Gem::GenEvA;

using boost::unit_test_framework::test_suite;
using boost::unit_test_framework::test_case;

/***********************************************************************************/
// Test features that are expected to work
BOOST_TEST_CASE_TEMPLATE_FUNCTION( GNumCollectionT_no_failure_expected, T)
{
	GRandom gr;

	// Construction in different modes
	GNumCollectionT<T> gnct0; // default construction, should be empty
	BOOST_CHECK(gnct0.empty());

	// Check the vector interface
	T templItem = T(0);
	T findItem = T(1);
	// Test the functionality of the underlying vector implementation
	stdvectorinterfacetest(gnct0, templItem, findItem);

	GNumCollectionT<T> gnct1(100, T(-10),T(10));  // 100 items in the range -10,10
	GNumCollectionT<T> gnct2(100, T(-10), T(10));  // 100 items in the range -10,10
	BOOST_CHECK(gnct1.size() == 100);
	BOOST_CHECK(gnct2.size() == 100);
	BOOST_CHECK(gnct1 != gnct2);

	// Copy construction
	GNumCollectionT<T> gnct3(gnct2);
	BOOST_CHECK(gnct3 == gnct2);

	// Assignment
	GNumCollectionT<T> gnct4;
	gnct4 = gnct3;
	BOOST_CHECK(gnct4 == gnct2);

	// Cloning and loading
	GObject *gnct5 = gnct4.clone();
	GNumCollectionT<T> gnct6;
	gnct6.load(gnct5);
	delete gnct5;
	BOOST_CHECK(gnct6 == gnct2);

	// Adding random data
	gnct6.addRandomData(1900, T(-100), T(100));
	BOOST_CHECK(gnct6 != gnct2);
	BOOST_CHECK(gnct6.size() == 2000);

	// Loading through the GParameterBase base pointer
	GParameterBase *gpb = new GNumCollectionT<T>();
	BOOST_CHECK(!gpb->isEqualTo(gnct6));
	gpb->load(&gnct6);
	BOOST_CHECK(gpb->isEqualTo(gnct6));
	GNumCollectionT<T> *gnct6_2 = static_cast<GNumCollectionT<T> *>(gpb);
	gnct6_2->addRandomData(1900, T(-100), T(100));
	BOOST_CHECK(!gpb->isEqualTo(gnct6));
	delete gpb;

	// Adding an adaptor with rather large gauss
	boost::shared_ptr<GGaussAdaptorT<T> > gba(new GGaussAdaptorT<T>(10,0.1,2,100));
	gnct6.addAdaptor(gba);

	const std::size_t NMUTATIONS=1000;
	GNumCollectionT<T> gnct6_old(gnct6);
	for(std::size_t i=0; i<NMUTATIONS; i++) {
		gnct6.mutate();
	}
	BOOST_CHECK(gnct6 != gnct6_old);

	// Test of serialization in different modes
	// Test serialization and loading in different serialization modes
	{ // plain text format
		// Copy construction of a new object
		GNumCollectionT<T> gnct7(100, T(-100), T(100));
		GNumCollectionT<T> gnct7_cp(gnct7);

		// Check equalities and inequalities
		BOOST_CHECK(gnct7_cp == gnct7);
		// Re-assign a new value to gnct7_cp
		gnct7_cp.addRandomData(100, T(-100), T(100));
		BOOST_CHECK(gnct7_cp.size() == 200);
		BOOST_CHECK(gnct7_cp != gnct7);

		// Serialize gnct7 and load into gnct7_co, check equalities and similarities
		BOOST_CHECK_NO_THROW(gnct7_cp.fromString(gnct7.toString(TEXTSERIALIZATION), TEXTSERIALIZATION));
		BOOST_CHECK(gnct7_cp.isSimilarTo(gnct7, exp(-10)));
	}

	{ // XML format
		// Copy construction of a new object
		GNumCollectionT<T> gnct7(100, T(-100), T(100));
		GNumCollectionT<T> gnct7_cp(gnct7);

		// Check equalities and inequalities
		BOOST_CHECK(gnct7_cp == gnct7);
		// Re-assign a new value to gnct7_cp
		gnct7_cp.addRandomData(100, T(-100), T(100));
		BOOST_CHECK(gnct7_cp.size() == 200);
		BOOST_CHECK(gnct7_cp != gnct7);

		// Serialize gnct7 and load into gnct7_co, check equalities and similarities
		BOOST_CHECK_NO_THROW(gnct7_cp.fromString(gnct7.toString(XMLSERIALIZATION), XMLSERIALIZATION));
		BOOST_CHECK(gnct7_cp.isSimilarTo(gnct7, exp(-10)));
	}

	{ // binary test format
		// Copy construction of a new object
		GNumCollectionT<T> gnct7(100, T(-100), T(100));
		GNumCollectionT<T> gnct7_cp(gnct7);

		// Check equalities and inequalities
		BOOST_CHECK(gnct7_cp == gnct7);
		// Re-assign a new value to gnct7_cp
		gnct7_cp.addRandomData(100, T(-100), T(100));
		BOOST_CHECK(gnct7_cp.size() == 200);
		BOOST_CHECK(gnct7_cp != gnct7);

		// Serialize gnct7 and load into gnct7_co, check equalities and similarities
		BOOST_CHECK_NO_THROW(gnct7_cp.fromString(gnct7.toString(BINARYSERIALIZATION), BINARYSERIALIZATION));
		BOOST_CHECK(gnct7_cp.isEqualTo(gnct7));
	}
}

/***********************************************************************************/
// Test features that are expected to fail
BOOST_TEST_CASE_TEMPLATE_FUNCTION( GNumCollectionT_failures_expected, T)
{
	GRandom gr;

	{
		// Self assignment should throw in DEBUG mode
#ifdef DEBUG
		GNumCollectionT<T> gnct;
		BOOST_CHECK_THROW(gnct.load(&gnct), Gem::GenEvA::geneva_error_condition);
#endif /* DEBUG */
	}
}
/***********************************************************************************/

/***********************************************************************************/
// This test suite checks as much as possible of the functionality provided
// by the GNumCollectionT class
class GNumCollectionTSuite: public test_suite
{
public:
	GNumCollectionTSuite() :test_suite("GNumCollectionTSuite") {
		typedef boost::mpl::list<boost::int32_t, double> test_types;

		add( BOOST_TEST_CASE_TEMPLATE( GNumCollectionT_no_failure_expected, test_types ) );
		add( BOOST_TEST_CASE_TEMPLATE( GNumCollectionT_failures_expected, test_types ) );
	}
};
