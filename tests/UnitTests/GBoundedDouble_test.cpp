/**
 * @file GBoundedDouble_test.cpp
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

#include <boost/shared_ptr.hpp>

// Geneva header files go here
#include "GenevaExceptions.hpp"
#include "GRandom.hpp"
#include "GBoundedDouble.hpp"
#include "GDoubleGaussAdaptor.hpp"

using namespace Gem;
using namespace Gem::Util;
using namespace Gem::GenEvA;

using boost::unit_test_framework::test_suite;
using boost::unit_test_framework::test_case;

/********************************************************************************************/
// The actual unit tests for this class
class GBoundedDouble_test {
public:
	/***********************************************************************************/
	// Test features that are expected to work
	void no_failure_expected() {
		// Test instantiation in different modes
		GBoundedDouble gbd0;
		GBoundedDouble gbd1(-10,10);
		GBoundedDouble gbd2(1.,-10,10);
		GBoundedDouble gbd7(3); // has maximum boundaries
		GBoundedDouble gbd3(gbd2);

		BOOST_CHECK(gbd3 == gbd2);
		BOOST_CHECK(gbd2 != gbd1);
		BOOST_CHECK(gbd2 != gbd0);
		BOOST_CHECK(gbd1 != gbd0);
		BOOST_CHECK(gbd7 != gbd0);

		// Check that value calculation works repeatedly. Internal value should be == external value for gbd7
		const std::size_t NCHECKS=10000;
		for(std::size_t i=0; i<NCHECKS; i++) {
			double in=-5000.+10000.*double(i)/double(NCHECKS), out = 0.;
			BOOST_CHECK_NO_THROW(out = gbd7.calculateExternalValue(in));
			BOOST_CHECK(in==out);
		}
		// Try resetting the boundaries to a finite value (which includes the current external value)
		BOOST_CHECK_NO_THROW(gbd7.setBoundaries(-6000.,6000));
		BOOST_CHECK_NO_THROW(gbd7 = 10);
		BOOST_CHECK_NO_THROW(gbd7.setBoundaries(-10.,10.));

		// (Repeated) assignment
		GBoundedDouble gbd3_2;
		gbd3_2 = gbd3 = gbd0;
		BOOST_CHECK(gbd3 != gbd2);
		BOOST_CHECK(gbd3 == gbd0);
		BOOST_CHECK(gbd3_2 != gbd2);
		BOOST_CHECK(gbd3_2 == gbd0);

		// Cloning and loading
		GObject * gbd4;
		BOOST_CHECK_NO_THROW(gbd4 = gbd3.clone());
		GBoundedDouble gbd5;
		BOOST_CHECK_NO_THROW(gbd5.load(gbd4));
		delete gbd4;
		BOOST_CHECK(gbd5 == gbd3);

		// Value assignment
		gbd5=gbd1;
		BOOST_CHECK(gbd5 == gbd1);
		gbd5 = 2.;
		BOOST_CHECK(gbd5.value() == 2.);
		BOOST_CHECK(!gbd5.isEqualTo(gbd3));
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

		// Mutate a couple of times and check that the value indeed changes
		const std::size_t NMUTATIONS=10000;
		boost::shared_ptr<GDoubleGaussAdaptor> gdga(new GDoubleGaussAdaptor(0.1,0.001,0.,1.));
		gbd5.addAdaptor(gdga);
		double oldValue = gbd5.value();
		for(std::size_t i=0; i<NMUTATIONS; i++) {
			gbd5.mutate();
			BOOST_CHECK(gbd5.value() != oldValue);
			oldValue = gbd5.value();
		}

		// Test serialization and loading in different serialization modes
		{ // plain text format
			// Copy construction of a new object
			GBoundedDouble gbd6(0.,-10.,10.);
			GBoundedDouble gbd6_cp(gbd6);

			// Check equalities and inequalities
			BOOST_CHECK(gbd6_cp == gbd6);
			// Re-assign a new value to gbd6_cp
			gbd6_cp = 1.;
			BOOST_CHECK(gbd6_cp.value() == 1.);
			BOOST_CHECK(gbd6_cp != gbd6);

			// Serialize gbd6 and load into gbd6_co, check equalities and similarities
			BOOST_CHECK_NO_THROW(gbd6_cp.fromString(gbd6.toString(TEXTSERIALIZATION), TEXTSERIALIZATION));
			BOOST_CHECK(gbd6_cp.isSimilarTo(gbd6, exp(-10)));
		}

		{ // XML format
			// Copy construction of a new object
			GBoundedDouble gbd6(0.,-10.,10.);
			GBoundedDouble gbd6_cp(gbd6);

			// Check equalities and inequalities
			BOOST_CHECK(gbd6_cp == gbd6);
			// Re-assign a new value to gbd6_cp
			gbd6_cp = 1.;
			BOOST_CHECK(gbd6_cp.value() == 1.);
			BOOST_CHECK(gbd6_cp != gbd6);

			// Serialize gbd6 and load into gbd6_co, check equalities and similarities
			BOOST_CHECK_NO_THROW(gbd6_cp.fromString(gbd6.toString(XMLSERIALIZATION), XMLSERIALIZATION));
			BOOST_CHECK(gbd6_cp.isSimilarTo(gbd6, exp(-10)));
		}

		{ // binary test format
			// Copy construction of a new object
			GBoundedDouble gbd6(0.,-10.,10.);
			GBoundedDouble gbd6_cp(gbd6);

			// Check equalities and inequalities
			BOOST_CHECK(gbd6_cp == gbd6);
			// Re-assign a new value to gbd6_cp
			gbd6_cp = 1.;
			BOOST_CHECK(gbd6_cp.value() == 1.);
			BOOST_CHECK(gbd6_cp != gbd6);

			// Serialize gbd6 and load into gbd6_co, check equalities and similarities
			BOOST_CHECK_NO_THROW(gbd6_cp.fromString(gbd6.toString(BINARYSERIALIZATION), BINARYSERIALIZATION));
			BOOST_CHECK(gbd6_cp.isEqualTo(gbd6));
		}
	}

	/***********************************************************************************/
	// Test features that are expected to fail
	void failures_expected() {
		// Assignment of value outside of the allowed range
		{
			GBoundedDouble gbd(-10,10.);
			BOOST_CHECK_THROW(gbd=11., Gem::GenEvA::geneva_error_condition);
		}

		// Setting boundaries so that the value lies outside of the new boundaries should throw
		{
			GBoundedDouble gbd(10); // Has boundaries -DBL_MAX, DBL_MAX
			BOOST_CHECK_THROW(gbd.setBoundaries(-7, 7), Gem::GenEvA::geneva_error_condition);
		}

#ifdef DEBUG
		// Self assignment should throw in DEBUG mode
		{
			GBoundedDouble gbd(-10,10.);
			BOOST_CHECK_THROW(gbd.load(&gbd), Gem::GenEvA::geneva_error_condition);
		}
#endif /* DEBUG */
	}

	/***********************************************************************************/
private:
	GRandom gr;
};

/********************************************************************************************/
// Test features of the the GBoundedDouble class. Please also have a look at the manual test,
// as it gives a graphical representation of the mapping.
class GBoundedDoubleSuite: public test_suite
{
public:
	GBoundedDoubleSuite() :test_suite("GBoundedDoubleSuite") {
	  // create an instance of the test cases class
	  boost::shared_ptr<GBoundedDouble_test> instance(new GBoundedDouble_test());

	  test_case* no_failure_expected_test_case = BOOST_CLASS_TEST_CASE(&GBoundedDouble_test::no_failure_expected, instance);
	  test_case* failures_expected_test_case = BOOST_CLASS_TEST_CASE(&GBoundedDouble_test::failures_expected, instance);

	  add(no_failure_expected_test_case);
	  add(failures_expected_test_case);
	}
};

/********************************************************************************************/
