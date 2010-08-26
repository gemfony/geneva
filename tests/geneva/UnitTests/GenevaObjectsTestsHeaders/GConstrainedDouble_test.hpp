/**
 * @file GConstrainedDouble_test.cpp
 */

/*
 * Copyright (C) Authors of the Geneva library collection and Karlsruhe
 * Institute of Technology (University of the State of Baden-Wuerttemberg
 * and National Laboratory of the Helmholtz Association).
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: info [at] gemfony (dot) com
 *
 * This file is part of the Geneva library collection
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
#include <boost/optional.hpp>
#include <boost/shared_ptr.hpp>

#ifndef GBOUNDEDDOUBLE_TEST_HPP_
#define GBOUNDEDDOUBLE_TEST_HPP_

// Geneva header files go here
#include "common/GExceptions.hpp"
#include "hap/GRandomT.hpp"
#include "geneva/GConstrainedDouble.hpp"
#include "geneva/GDoubleGaussAdaptor.hpp"
#include "GEqualityPrinter.hpp"

using namespace Gem::Hap;
using namespace Gem::Geneva;

using boost::unit_test_framework::test_suite;
using boost::unit_test_framework::test_case;

/********************************************************************************************/
// The actual unit tests for this class
class GConstrainedDouble_test {
public:
	/***********************************************************************************/
	// Test features that are expected to work
	void no_failure_expected() {
		// Prepare printing of error messages in object comparisons
		GEqualityPrinter gep("GConstrainedDouble_test::no_failure_expected()",
							 pow(10,-10),
							 Gem::Common::CE_WITH_MESSAGES);

		// Test instantiation in different modes
		GConstrainedDouble gbd0;
		GConstrainedDouble gbd1(-10,10);
		GConstrainedDouble gbd2(1.,-10,10);
		GConstrainedDouble gbd7(3); // has maximum boundaries
		GConstrainedDouble gbd3(gbd2);

		BOOST_CHECK(gbd3 == gbd2);
		BOOST_CHECK(gbd2 != gbd1);
		BOOST_CHECK(gbd2 != gbd0);
		BOOST_CHECK(gbd1 != gbd0);
		BOOST_CHECK(gbd7 != gbd0);

		// Check that value calculation works repeatedly. Internal value should be == external value for gbd7
		const std::size_t NCHECKS=10000;
		for(std::size_t i=0; i<NCHECKS; i++) {
			double in=-5000.+10000.*double(i)/double(NCHECKS), out = 0.;
			BOOST_CHECK_NO_THROW(out = gbd7.transfer(in));
			BOOST_CHECK(in==out);
		}
		// Try resetting the boundaries to a finite value (which includes the current external value)
		BOOST_CHECK_NO_THROW(gbd7.setBoundaries(-6000.,6000));
		BOOST_CHECK_NO_THROW(gbd7 = 10);
		BOOST_CHECK_NO_THROW(gbd7.setBoundaries(-10.,10.));

		// (Repeated) assignment
		GConstrainedDouble gbd3_2;
		gbd3_2 = gbd3 = gbd0;
		BOOST_CHECK(gbd3 != gbd2);
		BOOST_CHECK(gbd3 == gbd0);
		BOOST_CHECK(gbd3_2 != gbd2);
		BOOST_CHECK(gbd3_2 == gbd0);

		// Cloning and loading
		GConstrainedDouble gbd5;
		{
		   boost::shared_ptr<GObject> gbd4;
		   BOOST_CHECK_NO_THROW(gbd4 = gbd3.GObject::clone());
		   BOOST_CHECK_NO_THROW(gbd5.GObject::load(gbd4));
		   BOOST_CHECK(gbd5 == gbd3);
		}

		// Value assignment
		gbd5=gbd1;
		BOOST_CHECK(gbd5 == gbd1);
		gbd5 = 2.;
		BOOST_CHECK(gbd5.value() == 2.);
		BOOST_CHECK(gep.isInEqual(gbd5, gbd3));

		BOOST_CHECK(gbd5.getLowerBoundary() ==  -10.);
		BOOST_CHECK(gbd5.getUpperBoundary() ==  10.);

		// Check resetting of boundaries
		BOOST_CHECK_NO_THROW(gbd5.setBoundaries(-8.,8.));
		BOOST_CHECK(gbd5.getLowerBoundary() ==  -8.);
		BOOST_CHECK(gbd5.getUpperBoundary() ==  8.);
		BOOST_CHECK(gbd5.value() == 2); // Should have stayed the same
		// Set back to the old value
		BOOST_CHECK_NO_THROW(gbd5.setBoundaries(-10.,10.));
		BOOST_CHECK(gbd5.getLowerBoundary() ==  -10.);
		BOOST_CHECK(gbd5.getUpperBoundary() ==  10.);
		BOOST_CHECK(gbd5.value() == 2); // Should have stayed the same

		// Test automatic conversion to double
		double val=0.;
		val = gbd5;
		BOOST_CHECK(val == 2);

		// Adapt a couple of times and check that the value indeed changes
		const std::size_t NADAPTIONS=10000;
		boost::shared_ptr<GDoubleGaussAdaptor> gdga(new GDoubleGaussAdaptor(0.1,0.001,0.,1.));
		gbd5.addAdaptor(gdga);
		double oldValue = gbd5.value();
		for(std::size_t i=0; i<NADAPTIONS; i++) {
			gbd5.adapt();
			BOOST_CHECK(gbd5.value() != oldValue);
			oldValue = gbd5.value();
		}

		// Test serialization and loading in different serialization modes
		{ // plain text format
			// Copy construction of a new object
			GConstrainedDouble gbd6(0.,-10.,10.);
			GConstrainedDouble gbd6_cp(gbd6);

			// Check equalities and inequalities
			BOOST_CHECK(gbd6_cp == gbd6);
			// Re-assign a new value to gbd6_cp
			gbd6_cp = 1.;
			BOOST_CHECK(gbd6_cp.value() == 1.);
			BOOST_CHECK(gbd6_cp != gbd6);

			// Serialize gbd6 and load into gbd6_co, check equalities and similarities
			BOOST_CHECK_NO_THROW(gbd6_cp.fromString(gbd6.toString(Gem::Common::SERIALIZATIONMODE_TEXT), Gem::Common::SERIALIZATIONMODE_TEXT));
			BOOST_CHECK(gep.isSimilar(gbd6_cp, gbd6));
		}

		{ // XML format
			// Copy construction of a new object
			GConstrainedDouble gbd6(0.,-10.,10.);
			GConstrainedDouble gbd6_cp(gbd6);

			// Check equalities and inequalities
			BOOST_CHECK(gbd6_cp == gbd6);
			// Re-assign a new value to gbd6_cp
			gbd6_cp = 1.;
			BOOST_CHECK(gbd6_cp.value() == 1.);
			BOOST_CHECK(gbd6_cp != gbd6);

			// Serialize gbd6 and load into gbd6_co, check equalities and similarities
			BOOST_CHECK_NO_THROW(gbd6_cp.fromString(gbd6.toString(Gem::Common::SERIALIZATIONMODE_XML), Gem::Common::SERIALIZATIONMODE_XML));
			BOOST_CHECK(gep.isSimilar(gbd6_cp, gbd6));
		}

		{ // binary test format
			// Copy construction of a new object
			GConstrainedDouble gbd6(0.,-10.,10.);
			GConstrainedDouble gbd6_cp(gbd6);

			// Check equalities and inequalities
			BOOST_CHECK(gbd6_cp == gbd6);
			// Re-assign a new value to gbd6_cp
			gbd6_cp = 1.;
			BOOST_CHECK(gbd6_cp.value() == 1.);
			BOOST_CHECK(gbd6_cp != gbd6);

			// Serialize gbd6 and load into gbd6_co, check equalities and similarities
			BOOST_CHECK_NO_THROW(gbd6_cp.fromString(gbd6.toString(Gem::Common::SERIALIZATIONMODE_BINARY), Gem::Common::SERIALIZATIONMODE_BINARY));
			BOOST_CHECK(gep.isEqual(gbd6_cp, gbd6));
		}
	}

	/***********************************************************************************/
	// Test features that are expected to fail
	void failures_expected() {
		// Assignment of value outside of the allowed range
		{
			GConstrainedDouble gbd(-10,10.);
			BOOST_CHECK_THROW(gbd=11., Gem::Common::gemfony_error_condition);
		}

		// Setting boundaries so that the value lies outside of the new boundaries should throw
		{
			GConstrainedDouble gbd(10); // Has boundaries -DBL_MAX, DBL_MAX
			BOOST_CHECK_THROW(gbd.setBoundaries(-7, 7), Gem::Common::gemfony_error_condition);
		}

#ifdef DEBUG
		// Self assignment should throw in DEBUG mode
		{
			boost::shared_ptr<GConstrainedDouble> gbd_ptr(new GConstrainedDouble(-10,10.));
			BOOST_CHECK_THROW(gbd_ptr->load(gbd_ptr), Gem::Common::gemfony_error_condition);
		}
#endif /* DEBUG */
	}

	/***********************************************************************************/
private:
	GRandomT<RANDOMLOCAL> gr;
};

/********************************************************************************************/
// Test features of the the GConstrainedDouble class. Please also have a look at the manual test,
// as it gives a graphical representation of the mapping.
class GConstrainedDoubleSuite: public test_suite
{
public:
	GConstrainedDoubleSuite() :test_suite("GConstrainedDoubleSuite") {
	  // create an instance of the test cases class
	  boost::shared_ptr<GConstrainedDouble_test> instance(new GConstrainedDouble_test());

	  test_case* no_failure_expected_test_case = BOOST_CLASS_TEST_CASE(&GConstrainedDouble_test::no_failure_expected, instance);
	  test_case* failures_expected_test_case = BOOST_CLASS_TEST_CASE(&GConstrainedDouble_test::failures_expected, instance);

	  add(no_failure_expected_test_case);
	  add(failures_expected_test_case);
	}
};

/********************************************************************************************/

#endif /* GBOUNDEDDOUBLE_TEST_HPP_ */
