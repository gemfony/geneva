/**
 * @file GStandard_test.hpp
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt)
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) eu
 *
 * This file is part of the Geneva library collection.
 *
 * Geneva was developed with kind support from Karlsruhe Institute of
 * Technology (KIT) and Steinbuch Centre for Computing (SCC). Further
 * information about KIT and SCC can be found at http://www.kit.edu/english
 * and http://scc.kit.edu .
 *
 * Geneva is free software: you can redistribute and/or modify it under
 * the terms of version 3 of the GNU Affero General Public License
 * as published by the Free Software Foundation.
 *
 * Geneva is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with the Geneva library. If not, see <http://www.gnu.org/licenses/>.
 *
 * For further information on Gemfony scientific and Geneva, visit
 * http://www.gemfony.eu .
 */

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard header files go here
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>
#include <typeinfo>
#include <tuple>

// Boost header files go here

#include <boost/test/unit_test.hpp>
#include <boost/test/test_case_template.hpp>
#include <boost/mpl/list.hpp>

using namespace Gem::Hap;
using namespace Gem::Geneva;

using boost::unit_test_framework::test_suite;
using boost::unit_test_framework::test_case;

#ifndef GSTANDARDTEST_HPP_
#define GSTANDARDTEST_HPP_

// Geneva headers go here
#include "common/GExceptions.hpp"
#include "common/GCommonEnums.hpp"
#include "common/GSerializationHelperFunctionsT.hpp"
#include "common/GUnitTestFrameworkT.hpp"
#include "common/GTupleIO.hpp"
#include "geneva/GObject.hpp"

#include "GEqualityPrinter.hpp"

/*************************************************************************************************/
/**
 * This function performs common tests that need to be passed by every core Geneva class and
 * should be passed by user individuals as well. Most notably, this includes (de-)serialization
 * in different modes.
 */
BOOST_TEST_CASE_TEMPLATE_FUNCTION( StandardTests_no_failure_expected, T){
	// Prepare printing of error messages in object comparisons
	GEqualityPrinter gep(
      "StandardTests_no_failure_expected"
      , pow(10,-7)
		, Gem::Common::CE_WITH_MESSAGES
	);

	//---------------------------------------------------------------------------//
	// Tests of construction, loading, cloning, ...

	{ // Test default construction and copy construction
		std::shared_ptr<T> T_ptr, T_ptr_cp;

		// Default construction
		BOOST_REQUIRE_NO_THROW(T_ptr = TFactory_GUnitTests<T>());
		BOOST_REQUIRE(T_ptr); // must point somewhere

		// Make sure the object is not in pristine condition
		BOOST_REQUIRE_NO_THROW(T_ptr->modify_GUnitTests());

		// Copy construction
		BOOST_REQUIRE_NO_THROW(T_ptr_cp = std::shared_ptr<T>(new T(*T_ptr)));
		// T_ptr_cp = std::shared_ptr<T>(new T(*T_ptr));

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

   { // Test cloning to GObject
      std::shared_ptr<GObject> T_ptr, T_ptr_clone;

      // Default construction
      BOOST_REQUIRE_NO_THROW(T_ptr = TFactory_GUnitTests<T>());
      BOOST_REQUIRE(T_ptr); // must point somewhere

      // Make sure the object is not in pristine condition
      BOOST_REQUIRE_NO_THROW(T_ptr->modify_GUnitTests());

      // Cloning
      BOOST_REQUIRE_NO_THROW(T_ptr_clone = T_ptr->GObject::clone());

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

	{ // Test cloning to a target type
		std::shared_ptr<T> T_ptr, T_ptr_clone;

		// Default construction
		BOOST_REQUIRE_NO_THROW(T_ptr = TFactory_GUnitTests<T>());
		BOOST_REQUIRE(T_ptr); // must point somewhere

		// Make sure the object is not in pristine condition
		BOOST_REQUIRE_NO_THROW(T_ptr->modify_GUnitTests());

		// Cloning
		BOOST_REQUIRE_NO_THROW(T_ptr_clone = T_ptr->GObject::template clone<T>());

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

	{ // Test loading through a std::shared_ptr
		std::shared_ptr<T> T_ptr, T_ptr_load;

		// Default construction
		BOOST_REQUIRE_NO_THROW(T_ptr = TFactory_GUnitTests<T>());
		BOOST_REQUIRE(T_ptr); // must point somewhere

		// Make sure the object is not in pristine condition
		BOOST_REQUIRE_NO_THROW(T_ptr->modify_GUnitTests());

		// Loading
		BOOST_REQUIRE_NO_THROW(T_ptr_load = TFactory_GUnitTests<T>());
		BOOST_REQUIRE(T_ptr_load); // must point somewhere

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

   { // Test loading through a reference
      std::shared_ptr<T> T_ptr, T_ptr_load;

      // Default construction
      BOOST_REQUIRE_NO_THROW(T_ptr = TFactory_GUnitTests<T>());
      BOOST_REQUIRE(T_ptr); // must point somewhere

      // Make sure the object is not in pristine condition
      BOOST_REQUIRE_NO_THROW(T_ptr->modify_GUnitTests());

      // Loading
      BOOST_REQUIRE_NO_THROW(T_ptr_load = TFactory_GUnitTests<T>());
      BOOST_REQUIRE(T_ptr_load); // must point somewhere
      BOOST_REQUIRE_NO_THROW(T_ptr_load->GObject::load(*T_ptr));
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

	{ // Check assignment using operator=
		std::shared_ptr<T> T_ptr, T_ptr_assign;

		// Default construction
		BOOST_REQUIRE_NO_THROW(T_ptr = TFactory_GUnitTests<T>());
		BOOST_REQUIRE(T_ptr); // must point somewhere

		// Make sure the object is not in pristine condition
		BOOST_REQUIRE_NO_THROW(T_ptr->modify_GUnitTests());

		// Assignment
		BOOST_REQUIRE_NO_THROW(T_ptr_assign = TFactory_GUnitTests<T>());
		BOOST_REQUIRE(T_ptr_assign); // must point somewhere
		BOOST_REQUIRE_NO_THROW(T_ptr_assign->load(*T_ptr));

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
	// Check (de-)serialization in different modes through object functions

	{ // plain text format
		std::shared_ptr<T> T_ptr1 = TFactory_GUnitTests<T>();
		BOOST_REQUIRE(T_ptr1); // must point somewhere
		std::shared_ptr<T> T_ptr2 = TFactory_GUnitTests<T>();
		BOOST_REQUIRE(T_ptr2); // must point somewhere

		// Modify and check inequality
		if(T_ptr1->modify_GUnitTests()) { // Has the object been modified ?
			BOOST_CHECK(gep.isInEqual(*T_ptr1, *T_ptr2));

			// Serialize T_ptr1 and load into T_ptr1, check equalities and similarities
			BOOST_REQUIRE_NO_THROW(T_ptr2->GObject::fromString(T_ptr1->GObject::toString(Gem::Common::serializationMode::SERIALIZATIONMODE_TEXT), Gem::Common::serializationMode::SERIALIZATIONMODE_TEXT));
			BOOST_CHECK(gep.isSimilar(*T_ptr1, *T_ptr2));
		} else {
			std::cout << "Internal (de-)serialization test for object with name " << typeid(T).name() << " not run because original objects are identical / TEXT" << std::endl;
		}
	}

	{ // XML format
		std::shared_ptr<T> T_ptr1 = TFactory_GUnitTests<T>();
		BOOST_REQUIRE(T_ptr1); // must point somewhere
		std::shared_ptr<T> T_ptr2 = TFactory_GUnitTests<T>();
		BOOST_REQUIRE(T_ptr2); // must point somewhere

		// Modify and check inequality
		if(T_ptr1->modify_GUnitTests()) {
			BOOST_CHECK(gep.isInEqual(*T_ptr1, *T_ptr2));

			// Serialize T_ptr1 and load into T_ptr1, check equalities and similarities
			BOOST_REQUIRE_NO_THROW(T_ptr2->GObject::fromString(T_ptr1->GObject::toString(Gem::Common::serializationMode::SERIALIZATIONMODE_XML), Gem::Common::serializationMode::SERIALIZATIONMODE_XML));
			BOOST_CHECK(gep.isSimilar(*T_ptr1, *T_ptr2));
		} else {
			std::cout << "Internal (de-)serialization test for object with name " << typeid(T).name() << " not run because original objects are identical / XML" << std::endl;
		}
	}

	{ // binary test format
		std::shared_ptr<T> T_ptr1 = TFactory_GUnitTests<T>();
		BOOST_REQUIRE(T_ptr1); // must point somewhere
		std::shared_ptr<T> T_ptr2 = TFactory_GUnitTests<T>();
		BOOST_REQUIRE(T_ptr2); // must point somewhere

		// Modify and check inequality
		if(T_ptr1->modify_GUnitTests()) {
			BOOST_CHECK(gep.isInEqual(*T_ptr1, *T_ptr2));

			// Serialize T_ptr1 and load into T_ptr1, check equalities and similarities
			BOOST_REQUIRE_NO_THROW(T_ptr2->GObject::fromString(T_ptr1->GObject::toString(Gem::Common::serializationMode::SERIALIZATIONMODE_BINARY), Gem::Common::serializationMode::SERIALIZATIONMODE_BINARY));
			BOOST_CHECK(gep.isSimilar(*T_ptr1, *T_ptr2));
		} else {
			std::cout << "Internal (de-)serialization test for object with name " << typeid(T).name() << " not run because original objects are identical / BINARY" << std::endl;
		}
	}

	//---------------------------------------------------------------------------//
	// Check (de-)serialization in different modes through external Gem::Common functions
	// These are particularly used in the Courtier library

	{ // plain text mode
		std::shared_ptr<T> T_ptr1 = TFactory_GUnitTests<T>();
		BOOST_REQUIRE(T_ptr1); // must point somewhere
		std::shared_ptr<T> T_ptr2 = TFactory_GUnitTests<T>();
		BOOST_REQUIRE(T_ptr2); // must point somewhere

		// Modify and check inequality
		if(T_ptr1->modify_GUnitTests()) { // Has the object been modified ?
			BOOST_CHECK(gep.isInEqual(*T_ptr1, *T_ptr2));

			// Serialize T_ptr1 and load into T_ptr1, check equalities and similarities
			std::string serializedObject = Gem::Common::sharedPtrToString(T_ptr1, Gem::Common::serializationMode::SERIALIZATIONMODE_TEXT);
			T_ptr2 = Gem::Common::sharedPtrFromString<T>(serializedObject, Gem::Common::serializationMode::SERIALIZATIONMODE_TEXT);
			BOOST_CHECK(gep.isSimilar(*T_ptr1, *T_ptr2));
		} else {
			std::cout << "External (de-)serialization test for object with name " << typeid(T).name() << " not run because original objects are identical / TEXT" << std::endl;
		}
	}

	{ // XML mode
		std::shared_ptr<T> T_ptr1 = TFactory_GUnitTests<T>();
		BOOST_REQUIRE(T_ptr1); // must point somewhere
		std::shared_ptr<T> T_ptr2 = TFactory_GUnitTests<T>();
		BOOST_REQUIRE(T_ptr2); // must point somewhere

		// Modify and check inequality
		if(T_ptr1->modify_GUnitTests()) { // Has the object been modified ?
			BOOST_CHECK(gep.isInEqual(*T_ptr1, *T_ptr2));

			// Serialize T_ptr1 and load into T_ptr1, check equalities and similarities
			std::string serializedObject = Gem::Common::sharedPtrToString(T_ptr1, Gem::Common::serializationMode::SERIALIZATIONMODE_XML);
			T_ptr2 = Gem::Common::sharedPtrFromString<T>(serializedObject, Gem::Common::serializationMode::SERIALIZATIONMODE_XML);
			BOOST_CHECK(gep.isSimilar(*T_ptr1, *T_ptr2));
		} else {
			std::cout << "External (de-)serialization test for object with name " << typeid(T).name() << " not run because original objects are identical / XML" << std::endl;
		}
	}

	{ // Binary mode
		std::shared_ptr<T> T_ptr1 = TFactory_GUnitTests<T>();
		BOOST_REQUIRE(T_ptr1); // must point somewhere
		std::shared_ptr<T> T_ptr2 = TFactory_GUnitTests<T>();
		BOOST_REQUIRE(T_ptr2); // must point somewhere

		// Modify and check inequality
		if(T_ptr1->modify_GUnitTests()) { // Has the object been modified ?
			BOOST_CHECK(gep.isInEqual(*T_ptr1, *T_ptr2));

			// Serialize T_ptr1 and load into T_ptr1, check equalities and similarities
			std::string serializedObject = Gem::Common::sharedPtrToString(T_ptr1, Gem::Common::serializationMode::SERIALIZATIONMODE_BINARY);
			T_ptr2 = Gem::Common::sharedPtrFromString<T>(serializedObject, Gem::Common::serializationMode::SERIALIZATIONMODE_BINARY);
			BOOST_CHECK(gep.isSimilar(*T_ptr1, *T_ptr2));
		} else {
			std::cout << "External (de-)serialization test for object with name " << typeid(T).name() << " not run because original objects are identical / BINARY" << std::endl;
		}
	}

	//---------------------------------------------------------------------------//

	{ // Run specific tests for the current object type
		std::shared_ptr<T> T_ptr;
		BOOST_CHECK_NO_THROW(T_ptr = TFactory_GUnitTests<T>());
		BOOST_REQUIRE(T_ptr); // must point somewhere
		// BOOST_CHECK_NO_THROW(T_ptr->specificTestsNoFailureExpected_GUnitTests());
		T_ptr->specificTestsNoFailureExpected_GUnitTests();
	}
}

/*************************************************************************************************/
/**
 * This function performs common tests that should lead to a failure for every core Geneva class as
 * as user individuals. Most notably, self-assignment should fail.
 */
BOOST_TEST_CASE_TEMPLATE_FUNCTION( StandardTests_failures_expected, T){
	// Prepare printing of error messages in object comparisons
	GEqualityPrinter gep(
      "StandardTests_failures_expected"
	   , pow(10,-10)
		, Gem::Common::CE_WITH_MESSAGES
	);

	{
		// Checks that self-assignment throws in DEBUG mode
#ifdef DEBUG
		std::shared_ptr<T> T_ptr1 = TFactory_GUnitTests<T>();
		BOOST_REQUIRE(T_ptr1); // must point somewhere
		BOOST_CHECK_THROW(T_ptr1->GObject::load(T_ptr1);, gemfony_exception);
#endif
	}

	//---------------------------------------------------------------------------//
	// Run specific tests for the current object type
	{
		std::shared_ptr<T> T_ptr = TFactory_GUnitTests<T>();
		BOOST_REQUIRE(T_ptr); // must point somewhere
		BOOST_CHECK_NO_THROW(T_ptr->specificTestsFailuresExpected_GUnitTests());
	}
}

/*************************************************************************************************/

#endif /* GSTANDARDTEST_HPP_ */
