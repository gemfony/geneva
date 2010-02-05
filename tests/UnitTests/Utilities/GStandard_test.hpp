/**
 * @file GStandard_test.hpp
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

using namespace Gem;
using namespace Gem::Util;
using namespace Gem::GenEvA;

using boost::unit_test_framework::test_suite;
using boost::unit_test_framework::test_case;

#ifndef GSTANDARDTEST_HPP_
#define GSTANDARDTEST_HPP_

// Geneva headers go here
#include "GEqualityPrinter.hpp"
#include "GObject.hpp"
#include "GenevaExceptions.hpp"
#include "GEnums.hpp"

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif

/*************************************************************************************************/
/**
 * This function performs common tests that need to be passed by every core Geneva class and
 * should be passed by user individuals as well. Most notably, this includes (de-)serialization
 * in different modes.
 */
BOOST_TEST_CASE_TEMPLATE_FUNCTION( StandardTests_no_failure_expected, T){
	// Prepare printing of error messages in object comparisons
	GEqualityPrinter gep("StandardTests_no_failure_expected",
						 pow(10,-10),
						 Gem::Util::CE_WITH_MESSAGES);

	/**************************************************************************/
	// Tests of construction, loading, cloning, ...

	// We use boost::shared_ptr so we can refer to the objects from within BOOST_CHECK
	boost::shared_ptr<T> T_ptr, T_ptr_cp, T_ptr_clone, T_ptr_load, T_ptr_assign;

	// Default construction
	BOOST_REQUIRE_NO_THROW(T_ptr = boost::shared_ptr<T>(new T()));

	// Copy construction
	BOOST_REQUIRE_NO_THROW(T_ptr_cp = boost::shared_ptr<T>(new T(*T_ptr)))

	// Check for equivalence and similarity
	BOOST_CHECK(gep.isEqual(*T_ptr_cp, *T_ptr));
	BOOST_CHECK(gep.isSimilar(*T_ptr_cp, *T_ptr));

	// Cloning
	BOOST_REQUIRE_NO_THROW(T_ptr_clone = T_ptr->GObject::clone<T>());

	// Check for equivalence and similarity
	BOOST_CHECK(gep.isEqual(*T_ptr_clone, *T_ptr));
	BOOST_CHECK(gep.isSimilar(*T_ptr_clone, *T_ptr));

	// Loading
	BOOST_REQUIRE_NO_THROW(T_ptr_load = boost::shared_ptr<T>(new T()));
	BOOST_REQUIRE_NO_THROW(T_ptr_load->GObject::load(T_ptr));

	// Check for equivalence and similarity
	BOOST_CHECK(gep.isEqual(*T_ptr_load, *T_ptr));
	BOOST_CHECK(gep.isSimilar(*T_ptr_load, *T_ptr));

	// Assignment
	BOOST_REQUIRE_NO_THROW(T_ptr_assign = boost::shared_ptr<T>(new T()));
	BOOST_REQUIRE_NO_THROW(*T_ptr_assign = *T_ptr);

	// Check for equivalence and similarity
	BOOST_CHECK(gep.isEqual(*T_ptr_assign, *T_ptr));
	BOOST_CHECK(gep.isSimilar(*T_ptr_assign, *T_ptr));

	// Check that the four copied smart pointers are unique
	BOOST_CHECK(T_ptr.unique());
	BOOST_CHECK(T_ptr_cp.unique());
	BOOST_CHECK(T_ptr_clone.unique());
	BOOST_CHECK(T_ptr_load.unique());
	BOOST_CHECK(T_ptr_assign.unique());

	// Check destruction. Resetting the smart pointer will delete
	// the stored object if it was the last remaining reference to it.
	BOOST_REQUIRE_NO_THROW(T_ptr.reset());
	BOOST_REQUIRE_NO_THROW(T_ptr_cp.reset());
	BOOST_REQUIRE_NO_THROW(T_ptr_clone.reset());
	BOOST_REQUIRE_NO_THROW(T_ptr_load.reset());
	BOOST_REQUIRE_NO_THROW(T_ptr_assign.reset());

	/**************************************************************************/
	// Check (de-)serialization in different modes.
	{ // plain text format
		boost::shared_ptr<T> T_ptr1(new T());
		boost::shared_ptr<T> T_ptr2(new T());

		// Serialize gbc7 and load into gbc7_co, check equalities and similarities
		// BOOST_REQUIRE_NO_THROW(T_ptr2->GObject::fromString(T_ptr1->GObject::toString(TEXTSERIALIZATION), TEXTSERIALIZATION));
		T_ptr2->GObject::fromString(T_ptr1->GObject::toString(TEXTSERIALIZATION), TEXTSERIALIZATION);
		BOOST_CHECK(gep.isSimilar(*T_ptr1, *T_ptr2));
	}

	{ // XML format
		boost::shared_ptr<T> T_ptr1(new T());
		boost::shared_ptr<T> T_ptr2(new T());

		// Serialize gbc7 and load into gbc7_co, check equalities and similarities
		BOOST_REQUIRE_NO_THROW(T_ptr2->GObject::fromString(T_ptr1->GObject::toString(XMLSERIALIZATION), XMLSERIALIZATION));
		BOOST_CHECK(gep.isSimilar(*T_ptr1, *T_ptr2));
	}

	{ // binary test format
		boost::shared_ptr<T> T_ptr1(new T());
		boost::shared_ptr<T> T_ptr2(new T());

		// Serialize gbc7 and load into gbc7_co, check equalities and similarities
		BOOST_REQUIRE_NO_THROW(T_ptr2->GObject::fromString(T_ptr1->GObject::toString(BINARYSERIALIZATION), BINARYSERIALIZATION));
		BOOST_CHECK(gep.isEqual(*T_ptr1, *T_ptr2));
	}
}

/*************************************************************************************************/
/**
 * This function performs common tests that should lead to a failure for every core Geneva class as
 * as user individuals. Most notably, self-assignment should fail.
 */
BOOST_TEST_CASE_TEMPLATE_FUNCTION( StandardTests_failures_expected, T){
	// Prepare printing of error messages in object comparisons
	GEqualityPrinter gep("StandardTests_failures_expected",
						 pow(10,-10),
						 Gem::Util::CE_WITH_MESSAGES);

	{
		// Checks that self-assignment throws in DEBUG mode
#ifdef DEBUG
		boost::shared_ptr<T> T_ptr1(new T());
		BOOST_CHECK_THROW(T_ptr1->GObject::load(T_ptr1);, Gem::GenEvA::geneva_error_condition);
#endif
	}
}

/*************************************************************************************************/

#endif /* GSTANDARDTEST_HPP_ */
