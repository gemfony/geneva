/**
 * @file Geneva_tests.hpp
 *
 * INTERNAL — not installed, not part of the public Geneva API.
 *
 * Helper template functions used by Geneva's internal test drivers
 * (tests/geneva/UnitTests/ and examples/geneva/.../Tests/UnitTests/).
 * External users should write their own Catch2 test drivers directly
 * against the public Geneva methods specificTestsNoFailureExpected_GUnitTests()
 * and specificTestsFailuresExpected_GUnitTests() — see the project's
 * test driver in tests/geneva/UnitTests/GenevaStandardTests.cpp for the
 * pattern.
 */

/********************************************************************************
 *
 * This file is part of the Geneva library collection. The following license
 * applies to this file:
 *
 * ------------------------------------------------------------------------------
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * ------------------------------------------------------------------------------
 *
 * Note that other files in the Geneva library collection may use a different
 * license. Please see the licensing information in each file.
 *
 ********************************************************************************
 *
 * See the NOTICE file in the top-level directory of the Geneva library
 * collection for a list of contributors and copyright information.
 *
 ********************************************************************************/

#pragma once

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard header files go here
#include <algorithm>
#include <cmath>
#include <iostream>
#include <sstream>
#include <string>
#include <tuple>
#include <typeinfo>
#include <vector>

// Catch2 headers go here
#include <catch2/catch_test_macros.hpp>

using namespace Gem::Hap;
using namespace Gem::Geneva;

// Geneva headers go here
#include "common/GCommonEnums.hpp"
#include "common/GExceptions.hpp"
#include "common/GSerializationHelperFunctionsT.hpp"
#include "common/GTupleIO.hpp"
#include "common/GUnitTestFrameworkT.hpp"

#include "GEqualityPrinter.hpp"

/*************************************************************************************************/

namespace Gem::Geneva::Tests {

/*************************************************************************************************/
/**
 * Deduces the CRTP category root of a tested type T.
 *
 * Historically the standard tests hard-coded `GObject` as the common base of every
 * tested type. With the GObject decomposition (one CRTP root per logical category)
 * that no longer holds: e.g. personality-traits types now bottom out at
 * GPersonalityTraits, not GObject. The category root is the type parameter of the
 * Gem::Common::GCommonInterfaceT<Root> base, and it is exactly the element_type of
 * the std::shared_ptr returned by the public (inherited) clone() method. Deducing it
 * this way works for every category root uniformly (GObject-based types still resolve
 * to GObject), so the standard-test template stays category-agnostic.
 */
template <typename T>
using category_root_t = typename decltype(std::declval<const T &>().clone())::element_type;

/*************************************************************************************************/
/**
 * This function performs common tests that need to be passed by every core Geneva class and
 * should be passed by user individuals as well. Most notably, this includes (de-)serialization
 * in different modes.
 */
template <typename T>
void StandardTests_no_failure_expected() {
    // The CRTP category root of the tested type (GObject for most categories,
    // GPersonalityTraits for personality traits, etc.). Used wherever the test
    // previously hard-coded `GObject` / `GObject::`.
    using root_t = category_root_t<T>;
    // Prepare printing of error messages in object comparisons
    GEqualityPrinter gep(
        "StandardTests_no_failure_expected",
        pow(10, -7),
        Gem::Common::CE_WITH_MESSAGES
    );

    //---------------------------------------------------------------------------//
    // Tests of construction, loading, cloning, ...

    { // Test default construction and copy construction
        std::shared_ptr<T> T_ptr, T_ptr_cp;

        // Default construction
        REQUIRE_NOTHROW(T_ptr = TFactory_GUnitTests<T>());
        REQUIRE(T_ptr); // must point somewhere

        // Make sure the object is not in pristine condition
        REQUIRE_NOTHROW(T_ptr->modify_GUnitTests());

        // Copy construction
        REQUIRE_NOTHROW(T_ptr_cp = std::make_shared<T>(*T_ptr));

        // Check for equivalence and similarity
        CHECK(gep.isEqual(*T_ptr_cp, *T_ptr));
        CHECK(gep.isSimilar(*T_ptr_cp, *T_ptr));

        // Check that the smart pointers are unique
        CHECK(T_ptr.unique());
        CHECK(T_ptr_cp.unique());

        // Check destruction. Resetting the smart pointer will delete
        // the stored object if it was the last remaining reference to it.
        REQUIRE_NOTHROW(T_ptr.reset());
        REQUIRE_NOTHROW(T_ptr_cp.reset());
    }

    { // Test cloning to the category root
        std::shared_ptr<root_t> T_ptr, T_ptr_clone;

        // Default construction
        REQUIRE_NOTHROW(T_ptr = TFactory_GUnitTests<T>());
        REQUIRE(T_ptr); // must point somewhere

        // Make sure the object is not in pristine condition
        REQUIRE_NOTHROW(T_ptr->modify_GUnitTests());

        // Cloning
        REQUIRE_NOTHROW(T_ptr_clone = T_ptr->clone());

        // Check for equivalence and similarity
        CHECK(gep.isEqual(*T_ptr_clone, *T_ptr));
        CHECK(gep.isSimilar(*T_ptr_clone, *T_ptr));

        // Check that the smart pointers are unique
        CHECK(T_ptr.unique());
        CHECK(T_ptr_clone.unique());

        // Check destruction. Resetting the smart pointer will delete
        // the stored object if it was the last remaining reference to it.
        REQUIRE_NOTHROW(T_ptr.reset());
        REQUIRE_NOTHROW(T_ptr_clone.reset());
    }

    { // Test cloning to a target type
        std::shared_ptr<T> T_ptr, T_ptr_clone;

        // Default construction
        REQUIRE_NOTHROW(T_ptr = TFactory_GUnitTests<T>());
        REQUIRE(T_ptr); // must point somewhere

        // Make sure the object is not in pristine condition
        REQUIRE_NOTHROW(T_ptr->modify_GUnitTests());

        // Cloning
        REQUIRE_NOTHROW(T_ptr_clone = T_ptr->template clone<T>());

        // Check for equivalence and similarity
        CHECK(gep.isEqual(*T_ptr_clone, *T_ptr));
        CHECK(gep.isSimilar(*T_ptr_clone, *T_ptr));

        // Check that the smart pointers are unique
        CHECK(T_ptr.unique());
        CHECK(T_ptr_clone.unique());

        // Check destruction. Resetting the smart pointer will delete
        // the stored object if it was the last remaining reference to it.
        REQUIRE_NOTHROW(T_ptr.reset());
        REQUIRE_NOTHROW(T_ptr_clone.reset());
    }

    { // Test loading through a std::shared_ptr
        std::shared_ptr<T> T_ptr, T_ptr_load;

        // Default construction
        REQUIRE_NOTHROW(T_ptr = TFactory_GUnitTests<T>());
        REQUIRE(T_ptr); // must point somewhere

        // Make sure the object is not in pristine condition
        REQUIRE_NOTHROW(T_ptr->modify_GUnitTests());

        // Loading
        REQUIRE_NOTHROW(T_ptr_load = TFactory_GUnitTests<T>());
        REQUIRE(T_ptr_load); // must point somewhere

        REQUIRE_NOTHROW(T_ptr_load->load(T_ptr));
        // Check for equivalence and similarity
        CHECK(gep.isEqual(*T_ptr_load, *T_ptr));
        CHECK(gep.isSimilar(*T_ptr_load, *T_ptr));

        // Check that the smart pointers are unique
        CHECK(T_ptr.unique());
        CHECK(T_ptr_load.unique());

        // Check destruction. Resetting the smart pointer will delete
        // the stored object if it was the last remaining reference to it.
        REQUIRE_NOTHROW(T_ptr.reset());
        REQUIRE_NOTHROW(T_ptr_load.reset());
    }

    { // Test loading through a reference
        std::shared_ptr<T> T_ptr, T_ptr_load;

        // Default construction
        REQUIRE_NOTHROW(T_ptr = TFactory_GUnitTests<T>());
        REQUIRE(T_ptr); // must point somewhere

        // Make sure the object is not in pristine condition
        REQUIRE_NOTHROW(T_ptr->modify_GUnitTests());

        // Loading
        REQUIRE_NOTHROW(T_ptr_load = TFactory_GUnitTests<T>());
        REQUIRE(T_ptr_load); // must point somewhere
        REQUIRE_NOTHROW(T_ptr_load->load(*T_ptr));
        // Check for equivalence and similarity
        CHECK(gep.isEqual(*T_ptr_load, *T_ptr));
        CHECK(gep.isSimilar(*T_ptr_load, *T_ptr));

        // Check that the smart pointers are unique
        CHECK(T_ptr.unique());
        CHECK(T_ptr_load.unique());

        // Check destruction. Resetting the smart pointer will delete
        // the stored object if it was the last remaining reference to it.
        REQUIRE_NOTHROW(T_ptr.reset());
        REQUIRE_NOTHROW(T_ptr_load.reset());
    }

    { // Check assignment using operator=
        std::shared_ptr<T> T_ptr, T_ptr_assign;

        // Default construction
        REQUIRE_NOTHROW(T_ptr = TFactory_GUnitTests<T>());
        REQUIRE(T_ptr); // must point somewhere

        // Make sure the object is not in pristine condition
        REQUIRE_NOTHROW(T_ptr->modify_GUnitTests());

        // Assignment
        REQUIRE_NOTHROW(T_ptr_assign = TFactory_GUnitTests<T>());
        REQUIRE(T_ptr_assign); // must point somewhere
        REQUIRE_NOTHROW(T_ptr_assign->load(*T_ptr));

        // Check for equivalence and similarity
        CHECK(gep.isEqual(*T_ptr_assign, *T_ptr));
        CHECK(gep.isSimilar(*T_ptr_assign, *T_ptr));

        // Check that the smart pointers are unique
        CHECK(T_ptr.unique());
        CHECK(T_ptr_assign.unique());

        // Check destruction. Resetting the smart pointer will delete
        // the stored object if it was the last remaining reference to it.
        REQUIRE_NOTHROW(T_ptr.reset());
        REQUIRE_NOTHROW(T_ptr_assign.reset());
    }

    //---------------------------------------------------------------------------//
    // Check (de-)serialization in different modes through object functions

    { // plain text format
        std::shared_ptr<T> T_ptr1 = TFactory_GUnitTests<T>();
        REQUIRE(T_ptr1); // must point somewhere
        std::shared_ptr<T> T_ptr2 = TFactory_GUnitTests<T>();
        REQUIRE(T_ptr2); // must point somewhere

        // Modify and check inequality
        if(T_ptr1->modify_GUnitTests()) { // Has the object been modified ?
            CHECK(gep.isInEqual(*T_ptr1, *T_ptr2));

            // Serialize T_ptr1 and load into T_ptr1, check equalities and similarities
            REQUIRE_NOTHROW(T_ptr2->fromString(
                T_ptr1->toString(Gem::Common::serializationMode::TEXT),
                Gem::Common::serializationMode::TEXT
            ));
            CHECK(gep.isSimilar(*T_ptr1, *T_ptr2));
        }
        else {
            std::cout << "Internal (de-)serialization test for object with name "
                      << typeid(T).name()
                      << " not run because original objects are identical / TEXT" << '\n';
        }
    }

    { // XML format
        std::shared_ptr<T> T_ptr1 = TFactory_GUnitTests<T>();
        REQUIRE(T_ptr1); // must point somewhere
        std::shared_ptr<T> T_ptr2 = TFactory_GUnitTests<T>();
        REQUIRE(T_ptr2); // must point somewhere

        // Modify and check inequality
        if(T_ptr1->modify_GUnitTests()) {
            CHECK(gep.isInEqual(*T_ptr1, *T_ptr2));

            // Serialize T_ptr1 and load into T_ptr1, check equalities and similarities
            REQUIRE_NOTHROW(T_ptr2->fromString(
                T_ptr1->toString(Gem::Common::serializationMode::XML),
                Gem::Common::serializationMode::XML
            ));
            CHECK(gep.isSimilar(*T_ptr1, *T_ptr2));
        }
        else {
            std::cout << "Internal (de-)serialization test for object with name "
                      << typeid(T).name() << " not run because original objects are identical / XML"
                      << '\n';
        }
    }

    { // binary test format
        std::shared_ptr<T> T_ptr1 = TFactory_GUnitTests<T>();
        REQUIRE(T_ptr1); // must point somewhere
        std::shared_ptr<T> T_ptr2 = TFactory_GUnitTests<T>();
        REQUIRE(T_ptr2); // must point somewhere

        // Modify and check inequality
        if(T_ptr1->modify_GUnitTests()) {
            CHECK(gep.isInEqual(*T_ptr1, *T_ptr2));

            // Serialize T_ptr1 and load into T_ptr1, check equalities and similarities
            REQUIRE_NOTHROW(T_ptr2->fromString(
                T_ptr1->toString(Gem::Common::serializationMode::BINARY),
                Gem::Common::serializationMode::BINARY
            ));
            CHECK(gep.isSimilar(*T_ptr1, *T_ptr2));
        }
        else {
            std::cout << "Internal (de-)serialization test for object with name "
                      << typeid(T).name()
                      << " not run because original objects are identical / BINARY" << '\n';
        }
    }

    //---------------------------------------------------------------------------//
    // Check (de-)serialization in different modes through external Gem::Common functions
    // These are particularly used in the Courtier library

    { // plain text mode
        std::shared_ptr<T> T_ptr1 = TFactory_GUnitTests<T>();
        REQUIRE(T_ptr1); // must point somewhere
        std::shared_ptr<T> T_ptr2 = TFactory_GUnitTests<T>();
        REQUIRE(T_ptr2); // must point somewhere

        // Modify and check inequality
        if(T_ptr1->modify_GUnitTests()) { // Has the object been modified ?
            CHECK(gep.isInEqual(*T_ptr1, *T_ptr2));

            // Serialize T_ptr1 and load into T_ptr2, check equalities and similarities
            std::string serializedObject =
                Gem::Common::sharedPtrToString(T_ptr1, Gem::Common::serializationMode::TEXT);
            T_ptr2 = Gem::Common::sharedPtrFromString<T>(
                serializedObject,
                Gem::Common::serializationMode::TEXT
            );
            CHECK(gep.isSimilar(*T_ptr1, *T_ptr2));
        }
        else {
            std::cout << "External (de-)serialization test for object with name "
                      << typeid(T).name()
                      << " not run because original objects are identical / TEXT" << '\n';
        }
    }

    { // XML mode
        std::shared_ptr<T> T_ptr1 = TFactory_GUnitTests<T>();
        REQUIRE(T_ptr1); // must point somewhere
        std::shared_ptr<T> T_ptr2 = TFactory_GUnitTests<T>();
        REQUIRE(T_ptr2); // must point somewhere

        // Modify and check inequality
        if(T_ptr1->modify_GUnitTests()) { // Has the object been modified ?
            CHECK(gep.isInEqual(*T_ptr1, *T_ptr2));

            // Serialize T_ptr1 and load into T_ptr2, check equalities and similarities
            std::string serializedObject =
                Gem::Common::sharedPtrToString(T_ptr1, Gem::Common::serializationMode::XML);
            T_ptr2 = Gem::Common::sharedPtrFromString<T>(
                serializedObject,
                Gem::Common::serializationMode::XML
            );
            CHECK(gep.isSimilar(*T_ptr1, *T_ptr2));
        }
        else {
            std::cout << "External (de-)serialization test for object with name "
                      << typeid(T).name() << " not run because original objects are identical / XML"
                      << '\n';
        }
    }

    { // Binary mode
        std::shared_ptr<T> T_ptr1 = TFactory_GUnitTests<T>();
        REQUIRE(T_ptr1); // must point somewhere
        std::shared_ptr<T> T_ptr2 = TFactory_GUnitTests<T>();
        REQUIRE(T_ptr2); // must point somewhere

        // Modify and check inequality
        if(T_ptr1->modify_GUnitTests()) { // Has the object been modified ?
            CHECK(gep.isInEqual(*T_ptr1, *T_ptr2));

            // Serialize T_ptr1 and load into T_ptr2, check equalities and similarities
            std::string serializedObject =
                Gem::Common::sharedPtrToString(T_ptr1, Gem::Common::serializationMode::BINARY);
            T_ptr2 = Gem::Common::sharedPtrFromString<T>(
                serializedObject,
                Gem::Common::serializationMode::BINARY
            );
            CHECK(gep.isSimilar(*T_ptr1, *T_ptr2));
        }
        else {
            std::cout << "External (de-)serialization test for object with name "
                      << typeid(T).name()
                      << " not run because original objects are identical / BINARY" << '\n';
        }
    }

    //---------------------------------------------------------------------------//

    { // Run specific tests for the current object type
        std::shared_ptr<T> T_ptr;
        CHECK_NOTHROW(T_ptr = TFactory_GUnitTests<T>());
        REQUIRE(T_ptr); // must point somewhere
        T_ptr->specificTestsNoFailureExpected_GUnitTests();
    }
}

/*************************************************************************************************/
/**
 * This function performs common tests that should lead to a failure for every core Geneva class as
 * as user individuals. Most notably, self-assignment should fail.
 */
template <typename T>
void StandardTests_failures_expected() {
    // Prepare printing of error messages in object comparisons
    GEqualityPrinter gep(
        "StandardTests_failures_expected",
        pow(10, -10),
        Gem::Common::CE_WITH_MESSAGES
    );

    {
        // Checks that self-assignment throws in DEBUG mode
#ifdef DEBUG
        std::shared_ptr<T> T_ptr1 = TFactory_GUnitTests<T>();
        REQUIRE(T_ptr1); // must point somewhere
        CHECK_THROWS_AS(T_ptr1->load(T_ptr1), geneva_exception);
#endif
    }

    //---------------------------------------------------------------------------//
    // Run specific tests for the current object type
    {
        std::shared_ptr<T> T_ptr = TFactory_GUnitTests<T>();
        REQUIRE(T_ptr); // must point somewhere
        CHECK_NOTHROW(T_ptr->specificTestsFailuresExpected_GUnitTests());
    }
}

/*************************************************************************************************/

} /* namespace Gem::Geneva::Tests */

/*************************************************************************************************/
