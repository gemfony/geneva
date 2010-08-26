/**
 * @file GStandard_test.hpp
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
#include <cmath>
#include <algorithm>

// Boost header files go here

#include <boost/test/unit_test.hpp>
#include <boost/test/test_case_template.hpp>
#include <boost/mpl/list.hpp>
#include <boost/shared_ptr.hpp>

using namespace Gem::Hap;
using namespace Gem::Geneva;

using boost::unit_test_framework::test_suite;
using boost::unit_test_framework::test_case;

#ifndef GSTANDARDTEST_HPP_
#define GSTANDARDTEST_HPP_

// Geneva headers go here
#include "GEqualityPrinter.hpp"
#include "geneva/GObject.hpp"
#include "common/GExceptions.hpp"
#include "common/GCommonEnums.hpp"
#include "common/GUnitTestFrameworkT.hpp"

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
						 Gem::Common::CE_WITH_MESSAGES);

	//---------------------------------------------------------------------------//
	// Tests of construction, loading, cloning, ...

	// We use boost::shared_ptr so we can refer to the objects from within BOOST_CHECK
	boost::shared_ptr<T> T_ptr, T_ptr_cp, T_ptr_clone, T_ptr_load, T_ptr_assign;

	{
		// Default construction
		BOOST_REQUIRE_NO_THROW(T_ptr = TFactory_GUnitTests<T>());

		// Make sure the object is not in pristine condition
		BOOST_REQUIRE_NO_THROW(T_ptr->modify_GUnitTests());

		// Copy construction
		BOOST_REQUIRE_NO_THROW(T_ptr_cp = boost::shared_ptr<T>(new T(*T_ptr)))

		// Check for equivalence and similarity
		BOOST_CHECK(gep.isEqual(*T_ptr_cp, *T_ptr));
		BOOST_CHECK(gep.isSimilar(*T_ptr_cp, *T_ptr));

		// Check that the smart pointers are unique
		BOOST_CHECK(T_ptr.unique());
		BOOST_CHECK(T_ptr_cp.unique());

		// Check destruction. Resetting the smart pointer will delete
		// the stored object if it was the last remaining reference to it.
		BOOST_REQUIRE_NO_THROW(T_ptr.reset());
		BOOST_REQUIRE_NO_THROW(T_ptr_cp.reset());
	}

	{
		// Default construction
		BOOST_REQUIRE_NO_THROW(T_ptr = TFactory_GUnitTests<T>());

		// Make sure the object is not in pristine condition
		BOOST_REQUIRE_NO_THROW(T_ptr->modify_GUnitTests());

		// Cloning
		BOOST_REQUIRE_NO_THROW(T_ptr_clone = T_ptr->GObject::clone<T>());

		// Check for equivalence and similarity
		BOOST_CHECK(gep.isEqual(*T_ptr_clone, *T_ptr));
		BOOST_CHECK(gep.isSimilar(*T_ptr_clone, *T_ptr));

		// Check that the smart pointers are unique
		BOOST_CHECK(T_ptr.unique());
		BOOST_CHECK(T_ptr_clone.unique());

		// Check destruction. Resetting the smart pointer will delete
		// the stored object if it was the last remaining reference to it.
		BOOST_REQUIRE_NO_THROW(T_ptr.reset());
		BOOST_REQUIRE_NO_THROW(T_ptr_clone.reset());
	}

	{
		// Default construction
		BOOST_REQUIRE_NO_THROW(T_ptr = TFactory_GUnitTests<T>());

		// Make sure the object is not in pristine condition
		BOOST_REQUIRE_NO_THROW(T_ptr->modify_GUnitTests());

		// Loading
		BOOST_REQUIRE_NO_THROW(T_ptr_load = TFactory_GUnitTests<T>());
		BOOST_REQUIRE_NO_THROW(T_ptr_load->GObject::load(T_ptr));

		// Check for equivalence and similarity
		BOOST_CHECK(gep.isEqual(*T_ptr_load, *T_ptr));
		BOOST_CHECK(gep.isSimilar(*T_ptr_load, *T_ptr));

		// Check that the smart pointers are unique
		BOOST_CHECK(T_ptr.unique());
		BOOST_CHECK(T_ptr_load.unique());

		// Check destruction. Resetting the smart pointer will delete
		// the stored object if it was the last remaining reference to it.
		BOOST_REQUIRE_NO_THROW(T_ptr.reset());
		BOOST_REQUIRE_NO_THROW(T_ptr_load.reset());
	}

	{
		// Default construction
		BOOST_REQUIRE_NO_THROW(T_ptr = TFactory_GUnitTests<T>());

		// Make sure the object is not in pristine condition
		BOOST_REQUIRE_NO_THROW(T_ptr->modify_GUnitTests());

		// Assignment
		BOOST_REQUIRE_NO_THROW(T_ptr_assign = TFactory_GUnitTests<T>());
		BOOST_REQUIRE_NO_THROW(*T_ptr_assign = *T_ptr);

		// Check for equivalence and similarity
		BOOST_CHECK(gep.isEqual(*T_ptr_assign, *T_ptr));
		BOOST_CHECK(gep.isSimilar(*T_ptr_assign, *T_ptr));

		// Check that the smart pointers are unique
		BOOST_CHECK(T_ptr.unique());
		BOOST_CHECK(T_ptr_assign.unique());

		// Check destruction. Resetting the smart pointer will delete
		// the stored object if it was the last remaining reference to it.
		BOOST_REQUIRE_NO_THROW(T_ptr.reset());
		BOOST_REQUIRE_NO_THROW(T_ptr_assign.reset());
	}

	//---------------------------------------------------------------------------//
	// Check (de-)serialization in different modes.
	{ // plain text format
		boost::shared_ptr<T> T_ptr1 = TFactory_GUnitTests<T>();
		boost::shared_ptr<T> T_ptr2 = TFactory_GUnitTests<T>();

		//Modify and check inequality
		if(T_ptr1->modify_GUnitTests()) BOOST_CHECK(gep.isInEqual(*T_ptr1, *T_ptr2));

		// Serialize gbc7 and load into gbc7_co, check equalities and similarities
		// BOOST_REQUIRE_NO_THROW(T_ptr2->GObject::fromString(T_ptr1->GObject::toString(Gem::Common::SERIALIZATIONMODE_TEXT), Gem::Common::SERIALIZATIONMODE_TEXT));
		T_ptr2->GObject::fromString(T_ptr1->GObject::toString(Gem::Common::SERIALIZATIONMODE_TEXT), Gem::Common::SERIALIZATIONMODE_TEXT);
		BOOST_CHECK(gep.isSimilar(*T_ptr1, *T_ptr2));
	}

	{ // XML format
		boost::shared_ptr<T> T_ptr1 = TFactory_GUnitTests<T>();
		boost::shared_ptr<T> T_ptr2 = TFactory_GUnitTests<T>();

		//Modify and check inequality
		if(T_ptr1->modify_GUnitTests()) BOOST_CHECK(gep.isInEqual(*T_ptr1, *T_ptr2));

		// Serialize gbc7 and load into gbc7_co, check equalities and similarities
		BOOST_REQUIRE_NO_THROW(T_ptr2->GObject::fromString(T_ptr1->GObject::toString(Gem::Common::SERIALIZATIONMODE_XML), Gem::Common::SERIALIZATIONMODE_XML));
		BOOST_CHECK(gep.isSimilar(*T_ptr1, *T_ptr2));
	}

	{ // binary test format
		boost::shared_ptr<T> T_ptr1 = TFactory_GUnitTests<T>();
		boost::shared_ptr<T> T_ptr2 = TFactory_GUnitTests<T>();

		//Modify and check inequality
		if(T_ptr1->modify_GUnitTests()) BOOST_CHECK(gep.isInEqual(*T_ptr1, *T_ptr2));

		// Serialize gbc7 and load into gbc7_co, check equalities and similarities
		BOOST_REQUIRE_NO_THROW(T_ptr2->GObject::fromString(T_ptr1->GObject::toString(Gem::Common::SERIALIZATIONMODE_BINARY), Gem::Common::SERIALIZATIONMODE_BINARY));
		BOOST_CHECK(gep.isEqual(*T_ptr1, *T_ptr2));
	}

	//---------------------------------------------------------------------------//
	// Run specific tests for the current object type
	{
		boost::shared_ptr<T> T_ptr = TFactory_GUnitTests<T>();
		BOOST_CHECK_NO_THROW(T_ptr->specificTestsNoFailureExpected_GUnitTests());
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
						 Gem::Common::CE_WITH_MESSAGES);

	{
		// Checks that self-assignment throws in DEBUG mode
#ifdef DEBUG
		boost::shared_ptr<T> T_ptr1 = TFactory_GUnitTests<T>();
		BOOST_CHECK_THROW(T_ptr1->GObject::load(T_ptr1);, Gem::Common::gemfony_error_condition);
#endif
	}

	//---------------------------------------------------------------------------//
	// Run specific tests for the current object type
	{
		boost::shared_ptr<T> T_ptr = TFactory_GUnitTests<T>();
		BOOST_CHECK_NO_THROW(T_ptr->specificTestsFailuresExpected_GUnitTests());
	}
}

/*************************************************************************************************/

#endif /* GSTANDARDTEST_HPP_ */
