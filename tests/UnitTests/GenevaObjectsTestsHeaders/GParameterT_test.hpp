/**
 * @file GGaussAdaptorT_test.cpp
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
#include <algorithm>

// Boost header files go here

#include <boost/test/unit_test.hpp>
#include <boost/test/test_case_template.hpp>
#include <boost/mpl/list.hpp>

#include <boost/shared_ptr.hpp>
#include <boost/cstdint.hpp>

#ifndef GPARAMETERT_TEST_HPP_
#define GPARAMETERT_TEST_HPP_

// Geneva header files go here
#include "GenevaExceptions.hpp"
#include "GRandom.hpp"
#include "GParameterT.hpp"
#include "GBoolean.hpp"
#include "GBooleanAdaptor.hpp"
#include "GChar.hpp"
#include "GCharFlipAdaptor.hpp"
#include "GInt32.hpp"
#include "GInt32FlipAdaptor.hpp"
#include "GInt32GaussAdaptor.hpp"
#include "GDouble.hpp"
#include "GDoubleGaussAdaptor.hpp"
#include "GEqualityPrinter.hpp"

using namespace Gem;
using namespace Gem::Util;
using namespace Gem::GenEvA;

using boost::unit_test_framework::test_suite;
using boost::unit_test_framework::test_case;

/*******************************************************************************************/
// Test general features that are expected to work in GParameterT
BOOST_TEST_CASE_TEMPLATE_FUNCTION( GParameterT_no_failure_expected, T ) {
	// Prepare printing of error messages in object comparisons
	GEqualityPrinter gep("GParameterT_no_failure_expected",
						 pow(10,-10),
						 Gem::Util::CE_WITH_MESSAGES);

	// A local random number generator
	GRandom gr;

	// Test default construction
	BOOST_CHECK_NO_THROW(T gpt);

	// Test construction with a value
	T gpt0(0), gpt1(1);
	BOOST_CHECK(gpt0 != gpt1);

	// Test copy construction
	T gpt2(gpt1);
	BOOST_CHECK(gpt2 == gpt1);
	BOOST_CHECK(gpt2 != gpt0);

	// Test assignment
	T gpt3;
	gpt3 = gpt1;
	BOOST_CHECK(gpt3 == gpt1);
	BOOST_CHECK(gpt3 != gpt0);

	// Test cloning and loading
	{
		boost::shared_ptr<GObject> gpt3_clone = gpt3.GObject::clone();
		BOOST_CHECK_NO_THROW(gpt0.GObject::load(gpt3_clone));
	}
	BOOST_CHECK(gpt0 == gpt3);

	// Re-assign the original value
	gpt0 = T(0);
	BOOST_CHECK(gpt0 != gpt3);

	// Test (de-)serialization in different modes
	{ // plain text format
		T gpt4(gpt0);
		BOOST_CHECK(gep.isEqual(gpt4, gpt0));

		BOOST_CHECK_NO_THROW(gpt4.fromString(gpt1.toString(TEXTSERIALIZATION), TEXTSERIALIZATION));
		BOOST_CHECK(gep.isInEqual(gpt4, gpt0));
		BOOST_CHECK(gep.isSimilar(gpt4, gpt1));
	}

	{ // XML format
		T gpt4(gpt0);
		BOOST_CHECK(gep.isEqual(gpt4, gpt0));
		BOOST_CHECK_NO_THROW(gpt4.fromString(gpt1.toString(XMLSERIALIZATION), XMLSERIALIZATION));
		BOOST_CHECK(gep.isInEqual(gpt4, gpt0));
		BOOST_CHECK(gep.isSimilar(gpt4, gpt1));
	}

	{ // binary test format
		T gpt4(gpt0);
		BOOST_CHECK(gpt4 == gpt0);
		BOOST_CHECK_NO_THROW(gpt4.fromString(gpt1.toString(BINARYSERIALIZATION), BINARYSERIALIZATION));
		BOOST_CHECK(gpt4 != gpt0);
		BOOST_CHECK(gpt4 == gpt1);
	}
}

/********************************************************************************************/
// Test features for some particular types that cannot be implemented with a single code base
// The actual unit tests for this class
class GParameterT_test {
public:
	/***********************************************************************************/
	// Test features that are expected to work: boolean case
	void bool_no_failure_expected() {
		// Default construction
		GBoolean gpt0;

		// Adding a single adaptor
		BOOST_CHECK(!gpt0.hasAdaptor());
		BOOST_CHECK_NO_THROW(gpt0.addAdaptor(boost::shared_ptr<GBooleanAdaptor>(new GBooleanAdaptor())));
		BOOST_CHECK(gpt0.hasAdaptor());

		// Retrieve the adaptor again, as a GAdaptorT
		BOOST_CHECK_NO_THROW(boost::shared_ptr<GAdaptorT<bool> > gadb0_ptr = gpt0.getAdaptor());

		// Retrieve the adaptor in its original form
		BOOST_CHECK_NO_THROW(boost::shared_ptr<GBooleanAdaptor> gba0_ptr = gpt0.adaptor_cast<GBooleanAdaptor>());

		// Check mutations
		const std::size_t NMUTATIONS=10000;
		std::vector<bool> mutvals(NMUTATIONS);
		bool originalValue = gpt0.value();
		for(std::size_t i=0; i<NMUTATIONS; i++) {
			BOOST_CHECK_NO_THROW(gpt0.mutate());
			mutvals[i] = gpt0.value();
		}

		// Check that values do not stay the same for a larger number of mutations
		std::size_t nOriginalValues = std::count(mutvals.begin(), mutvals.end(), originalValue);
		BOOST_CHECK(nOriginalValues < NMUTATIONS);
	}

	/***********************************************************************************/
	// Test features that are expected to work: char case
	void char_no_failure_expected() {
		// Default construction
		GChar gpt0;

		// Adding a single adaptor
		BOOST_CHECK_NO_THROW(gpt0.addAdaptor(boost::shared_ptr<GCharFlipAdaptor>(new GCharFlipAdaptor())));

		// Retrieve the adaptor again, as a GAdaptorT
		BOOST_CHECK_NO_THROW(boost::shared_ptr<GAdaptorT<char> > gadb0_ptr = gpt0.getAdaptor());

		// Retrieve the adaptor in its original form
		BOOST_CHECK_NO_THROW(boost::shared_ptr<GCharFlipAdaptor> gpt0_ptr = gpt0.adaptor_cast<GCharFlipAdaptor>());

		// Check mutations
		const std::size_t NMUTATIONS=10000;
		std::vector<char> mutvals(NMUTATIONS);
		char originalValue = gpt0.value();
		for(std::size_t i=0; i<NMUTATIONS; i++) {
			BOOST_CHECK_NO_THROW(gpt0.mutate());
			mutvals[i] = gpt0.value();
		}

		// Check that values do not stay the same for a larger number of mutations
		std::size_t nOriginalValues = std::count(mutvals.begin(), mutvals.end(), originalValue);
		BOOST_CHECK(nOriginalValues < NMUTATIONS);
	}

	/***********************************************************************************/
	// Test features that are expected to work: boost::int32_t case
	void int32_no_failure_expected() {
		GInt32 gpt0;

		// Adding a single adaptor
		BOOST_CHECK_NO_THROW(gpt0.addAdaptor(boost::shared_ptr<GInt32FlipAdaptor>(new GInt32FlipAdaptor())));

		// Retrieve the adaptor again, as a GAdaptorT
		BOOST_CHECK_NO_THROW(boost::shared_ptr<GAdaptorT<boost::int32_t> > gadb0_ptr = gpt0.getAdaptor());

		// Retrieve the adaptor in its original form
		BOOST_CHECK_NO_THROW(boost::shared_ptr<GInt32FlipAdaptor> gpt0_ptr = gpt0.adaptor_cast<GInt32FlipAdaptor>());

		// Check mutations
		const std::size_t NMUTATIONS=10000;
		std::vector<boost::int32_t> mutvals(NMUTATIONS);
		boost::int32_t originalValue = gpt0.value();
		for(std::size_t i=0; i<NMUTATIONS; i++) {
			BOOST_CHECK_NO_THROW(gpt0.mutate());
			mutvals[i] = gpt0.value();
		}

		// Check that values do not stay the same for a larger number of mutations
		std::size_t nOriginalValues = std::count(mutvals.begin(), mutvals.end(), originalValue);
		BOOST_CHECK(nOriginalValues < NMUTATIONS);
	}

	/***********************************************************************************/
	// Test features that are expected to work: double case
	void double_no_failure_expected() {
		GDouble gpt0;

		// Adding a single adaptor
		BOOST_CHECK_NO_THROW(gpt0.addAdaptor(boost::shared_ptr<GDoubleGaussAdaptor>(new GDoubleGaussAdaptor())));

		// Retrieve the adaptor again, as a GAdaptorT
		BOOST_CHECK_NO_THROW(boost::shared_ptr<GAdaptorT<double> > gadb0_ptr = gpt0.getAdaptor());

		// Retrieve the adaptor in its original form
		BOOST_CHECK_NO_THROW(boost::shared_ptr<GDoubleGaussAdaptor> gpt0_ptr = gpt0.adaptor_cast<GDoubleGaussAdaptor>());

		// Check mutations
		const std::size_t NMUTATIONS=10000;
		std::vector<double> mutvals(NMUTATIONS);
		double originalValue = gpt0.value();
		for(std::size_t i=0; i<NMUTATIONS; i++) {
			BOOST_CHECK_NO_THROW(gpt0.mutate());
			mutvals[i] = gpt0.value();
		}

		// Check that values do not stay the same for a larger number of mutations
		std::size_t nOriginalValues = std::count(mutvals.begin(), mutvals.end(), originalValue);
		BOOST_CHECK(nOriginalValues < NMUTATIONS);
	}

	/***********************************************************************************/
	// Test features that are expected to fail
	void failures_expected() {
		{
			// Default construction
			GInt32 gpt0;
			// Adding an empty adaptor should throw
			BOOST_CHECK_THROW(gpt0.addAdaptor(boost::shared_ptr<GInt32FlipAdaptor>()), Gem::GenEvA::geneva_error_condition);
		}

#ifdef DEBUG
		{
			// Default construction
			GInt32 gpt0;
			// Extracting an adaptor of wrong type should throw in DEBUG mode
			BOOST_CHECK_NO_THROW(gpt0.addAdaptor(boost::shared_ptr<GInt32FlipAdaptor>(new GInt32FlipAdaptor())));
			BOOST_CHECK_THROW(gpt0.adaptor_cast<GCharFlipAdaptor>(), Gem::GenEvA::geneva_error_condition);
		}
#endif /* DEBUG */

		{
			// Self assignment should throw in DEBUG mode
#ifdef DEBUG
			GInt32 gpt0;
			BOOST_CHECK_THROW(gpt0.load(&gpt0), Gem::GenEvA::geneva_error_condition);
#endif /* DEBUG */
		}
	}

	/***********************************************************************************/
private:
	GRandom gr;
};


/********************************************************************************************/
// This test suite checks as much as possible of the functionality provided
// by the GParameterT class. Tests include features of the parent class
// GParameterBaseWithAdaptorsT, as it cannot be instantiated itself.
class GParameterTSuite: public test_suite
{
public:
	GParameterTSuite() :test_suite("GParameterTSuite") {
		typedef boost::mpl::list<GBoolean, GChar, GInt32, GDouble> test_types;
		add( BOOST_TEST_CASE_TEMPLATE( GParameterT_no_failure_expected, test_types ) );

		// create an instance of the test cases class
		boost::shared_ptr<GParameterT_test> instance(new GParameterT_test());

		test_case* GParameterT_bool_no_failure_expected_test_case = BOOST_CLASS_TEST_CASE(&GParameterT_test::bool_no_failure_expected, instance);
		test_case* GParameterT_char_no_failure_expected_test_case = BOOST_CLASS_TEST_CASE(&GParameterT_test::char_no_failure_expected, instance);
		test_case* GParameterT_int32_no_failure_expected_test_case = BOOST_CLASS_TEST_CASE(&GParameterT_test::int32_no_failure_expected, instance);
		test_case* GParameterT_double_no_failure_expected_test_case = BOOST_CLASS_TEST_CASE(&GParameterT_test::double_no_failure_expected, instance);
		test_case* GParameterT_failures_expected_test_case = BOOST_CLASS_TEST_CASE(&GParameterT_test::failures_expected, instance);

		add(GParameterT_bool_no_failure_expected_test_case);
		add(GParameterT_char_no_failure_expected_test_case);
		add(GParameterT_int32_no_failure_expected_test_case);
		add(GParameterT_double_no_failure_expected_test_case);
		add(GParameterT_failures_expected_test_case);
	}
};

/********************************************************************************************/

#endif /* GPARAMETERT_TEST_HPP_ */
