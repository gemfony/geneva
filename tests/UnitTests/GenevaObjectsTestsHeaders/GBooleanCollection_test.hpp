/**
 * @file GBooleanCollection_test.cpp
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
#include <boost/optional.hpp>
#include <boost/shared_ptr.hpp>

#ifndef GBOOLEANCOLLECTION_TEST_HPP_
#define GBOOLEANCOLLECTION_TEST_HPP_

// Geneva header files go here
#include "common/GExceptions.hpp"
#include "hap/GRandom.hpp"
#include "geneva/GParameterBase.hpp"
#include "geneva/GBooleanCollection.hpp"
#include "geneva/GBooleanAdaptor.hpp"
#include "geneva/GStdSimpleVectorInterfaceT.hpp"
#include "GStdVectorInterface_test.hpp"
#include "GEqualityPrinter.hpp"

using namespace Gem;
using namespace Gem::Hap;
using namespace Gem::Geneva;

using boost::unit_test_framework::test_suite;
using boost::unit_test_framework::test_case;

/********************************************************************************************/
// The actual unit tests for this class
class GBooleanCollection_test {
public:
	/***********************************************************************************/
	// Test features that are expected to work
	void no_failure_expected() {
		// Prepare printing of error messages in object comparisons
		GEqualityPrinter gep("GBooleanCollection_test::no_failure_expected()",
							 pow(10,-10),
							 Gem::Common::CE_WITH_MESSAGES);

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
		BOOST_CHECK(gep.isEqual(gbc3, gbc2));

		// Assignment
		GBooleanCollection gbc4;
		gbc4 = gbc3;
		BOOST_CHECK(gbc4 == gbc2);

		// Cloning and loading
		GBooleanCollection gbc6;
		{
		   boost::shared_ptr<GObject> gbc5 = gbc4.GObject::clone();
		   gbc6.GObject::load(gbc5);
		}
		BOOST_CHECK(gbc6 == gbc2);

		// Re-initialization in two different modes
		gbc6.randomInit(); // equal likelihood for true/false
		BOOST_CHECK(gbc6 != gbc2);
		BOOST_CHECK(gep.isInEqual(gbc6, gbc2));

		GBooleanCollection gbc6_2(gbc6);
		BOOST_CHECK(gbc6_2 == gbc6);
		gbc6_2.randomInit(0.1);
		BOOST_CHECK(gbc6_2 != gbc6);

		// Adding an adaptor
		boost::shared_ptr<GBooleanAdaptor> gba(new GBooleanAdaptor());
		gbc6.addAdaptor(gba);

		const std::size_t NADAPTIONS=1000;
		GBooleanCollection gbc6_old(gbc6);
		for(std::size_t i=0; i<NADAPTIONS; i++) {
			gbc6.adapt();
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
			gbc7_cp.randomInit();
			BOOST_CHECK(gbc7_cp != gbc7);

			// Serialize gbc7 and load into gbc7_cp, check equalities and similarities
			BOOST_REQUIRE_NO_THROW(gbc7_cp.fromString(gbc7.toString(Gem::Common::SERIALIZATIONMODE_TEXT), Gem::Common::SERIALIZATIONMODE_TEXT));
			BOOST_CHECK(gep.isSimilar(gbc7_cp, gbc7));
		}

		{ // XML format
			// Copy construction of a new object
			GBooleanCollection gbc7(100);
			GBooleanCollection gbc7_cp(gbc7);

			// Check equalities and inequalities
			BOOST_CHECK(gbc7_cp == gbc7);
			// Re-assign a new value to gbc7_cp
			gbc7_cp.randomInit();
			BOOST_CHECK(gbc7_cp != gbc7);

			// Serialize gbc7 and load into gbc7_cp, check equalities and similarities
			BOOST_REQUIRE_NO_THROW(gbc7_cp.fromString(gbc7.toString(Gem::Common::SERIALIZATIONMODE_XML), Gem::Common::SERIALIZATIONMODE_XML));
			BOOST_CHECK(gep.isSimilar(gbc7_cp, gbc7));
		}

		{ // binary test format
			// Copy construction of a new object
			GBooleanCollection gbc7(100);
			GBooleanCollection gbc7_cp(gbc7);

			// Check equalities and inequalities
			BOOST_CHECK(gbc7_cp == gbc7);
			// Re-assign a new value to gbc7_cp
			gbc7_cp.randomInit();
			BOOST_CHECK(gbc7_cp != gbc7);

			// Serialize gbc7 and load into gbc7_cp, check equalities and similarities
			BOOST_REQUIRE_NO_THROW(gbc7_cp.fromString(gbc7.toString(Gem::Common::SERIALIZATIONMODE_BINARY), Gem::Common::SERIALIZATIONMODE_BINARY));
			BOOST_CHECK(gep.isEqual(gbc7_cp, gbc7));
		}
	}

	/***********************************************************************************/
	// Test features that are expected to fail
	void failures_expected() {
		{
			// Self assignment should throw in DEBUG mode
#ifdef DEBUG
			boost::shared_ptr<GBooleanCollection> gbc_ptr(new GBooleanCollection(100));
			BOOST_CHECK_THROW(gbc_ptr->GObject::load(gbc_ptr), Gem::Common::gemfony_error_condition);
#endif /* DEBUG */
		}
	}

	/***********************************************************************************/
private:
	GRandom gr;
};

/********************************************************************************************/
// This test suite checks as much as possible of the functionality provided
// by the GBooleanCollection class, as well as some important parent classes
// (GParameterCollectionT and GStdSimpleVectorInterface).
class GBooleanCollectionSuite: public test_suite
{
public:
	GBooleanCollectionSuite() :test_suite("GBooleanCollectionSuite") {
	  // create an instance of the test cases class
	  boost::shared_ptr<GBooleanCollection_test> instance(new GBooleanCollection_test());

	  test_case* no_failure_expected_test_case = BOOST_CLASS_TEST_CASE(&GBooleanCollection_test::no_failure_expected, instance);
	  test_case* failures_expected_test_case = BOOST_CLASS_TEST_CASE(&GBooleanCollection_test::failures_expected, instance);

	  add(no_failure_expected_test_case);
	  add(failures_expected_test_case);
	}
};

#endif /* GBOOLEANCOLLECTION_TEST_HPP_ */
