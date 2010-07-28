/**
 * @file GObject_test.cpp
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
#include <cmath>

// Boost header files go here

#include <boost/test/unit_test.hpp>
#include <boost/shared_ptr.hpp>

#ifndef GOBJECT_TEST_HPP_
#define GOBJECT_TEST_HPP_

// Geneva header files go here
#include "common/GExceptions.hpp"
#include "hap/GRandom.hpp"
#include "geneva/GObject.hpp"
#include "geneva/GBooleanAdaptor.hpp"
#include "GEqualityPrinter.hpp"

using namespace Gem;
using namespace Gem::Hap;
using namespace Gem::Geneva;

using boost::unit_test_framework::test_suite;
using boost::unit_test_framework::test_case;

/********************************************************************************************/
// The actual unit tests for this class
class GObject_test {
public:
	/***********************************************************************************/
	// Test features that are expected to work
	void no_failure_expected() {
		// Prepare printing of error messages in object comparisons
		GEqualityPrinter gep("GObject_test::no_failure_expected()"
				             , pow(10,-10)
				             , Gem::Common::CE_WITH_MESSAGES);

		// Default construction
		GBooleanAdaptor gba0;

		// Test that the object can be translated into a string and back
		// again and that afterwards both objects are at least similar.
		// Note that text-io may result in a lost of precision, so that double
		// values might differ slightly.
		// text mode:
		{ // Explicit scope results in the destruction of the contained objects
			std::ostringstream stream;
			stream << gba0.toString(Gem::Common::SERIALIZATIONMODE_TEXT);
			GBooleanAdaptor gba1; // Create a new, pristine object
			gba1.fromString(stream.str(),Gem::Common::SERIALIZATIONMODE_TEXT);
			BOOST_CHECK(gep.isSimilar(gba1, gba0));
		} // Explicit scope results in the destruction of the contained objects

		{
			std::ostringstream stream;
			stream << gba0.toString(Gem::Common::SERIALIZATIONMODE_XML);
			GBooleanAdaptor gba1; // Create a new, pristine object
			gba1.fromString(stream.str(),Gem::Common::SERIALIZATIONMODE_XML);
			BOOST_CHECK(gep.isSimilar(gba1, gba0));
		}

		{
			std::ostringstream stream;
			stream << gba0.toString(Gem::Common::SERIALIZATIONMODE_BINARY);
			GBooleanAdaptor gba1; // Create a new, pristine object
			gba1.fromString(stream.str(),Gem::Common::SERIALIZATIONMODE_BINARY); // We should be able to achieve equality for this mode
			BOOST_CHECK(gep.isEqual(gba1, gba0));
		}

		// Clone the object, load into another GBooleanAdaptor.
		BOOST_CHECK_NO_THROW(boost::shared_ptr<GBooleanAdaptor> gba0_clone2 = gba0.clone<GBooleanAdaptor>());
		GBooleanAdaptor gba2;
		{
			boost::shared_ptr<GObject> gba0_clone3 = gba0.GObject::clone();
			BOOST_CHECK_NO_THROW(gba2.GObject::load(gba0_clone3));
		}

		std::string report =  gba0.report();
		BOOST_CHECK(report.size() != 0);
	}

	/***********************************************************************************/
	// Test features that are expected to fail
	void failures_expected() {
		{
			// Self assignment should throw in DEBUG mode
#ifdef DEBUG
			boost::shared_ptr<GBooleanAdaptor> gba0_ptr(new GBooleanAdaptor());
			BOOST_CHECK_THROW(gba0_ptr->load(gba0_ptr), Gem::Common::gemfony_error_condition);
#endif /* DEBUG */
		}
	}

	/***********************************************************************************/
private:
	GRandom gr;
};

/********************************************************************************************/
// This test suite checks as much as possible of the functionality provided
// by the GObject class. As GObject cannot be instantiated itself, we perform
// testing through a "near" instantiable class. Not all functions of GObject are
// tested, particularly if these functions also exist in the derived class (and
// internally call the GObject version).
class GObjectSuite: public test_suite
{
public:
	GObjectSuite() :test_suite("GObjectSuite") {
		  // create an instance of the test cases class
		  boost::shared_ptr<GObject_test> instance(new GObject_test());

		  test_case* GObject_no_failure_expected_test_case = BOOST_CLASS_TEST_CASE(&GObject_test::no_failure_expected, instance);
		  test_case* GObject_failures_expected_test_case = BOOST_CLASS_TEST_CASE(&GObject_test::failures_expected, instance);

		  add(GObject_no_failure_expected_test_case);
		  add(GObject_failures_expected_test_case);
	}
};

#endif /* GOBJECT_TEST_HPP_ */
