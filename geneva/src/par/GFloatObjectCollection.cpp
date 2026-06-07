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
#include "geneva/par/GFloatObjectCollection.hpp"
#include "common/GExceptions.hpp"
#include "common/GExpectationChecksT.hpp"
#include "geneva/GOptimizationEnums.hpp"
#include "geneva/par/GFloatGaussAdaptor.hpp"
#include "geneva/par/GFloatObject.hpp"
#include "geneva/par/GParameterBase.hpp"
#include "geneva/par/GParameterTCollectionT.hpp"
#include "hap/GHapEnums.hpp"
#include "hap/GRandomT.hpp"
#include <cstddef>
#include <memory>

#ifdef GEM_TESTING
#include <catch2/catch_test_macros.hpp>
#endif /* GEM_TESTING */

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::Parameters::GFloatObjectCollection) // NOLINT
namespace Gem::Geneva::Parameters {

/******************************************************************************/
/**
 * Initialization with a number of identical GFloatObject objects
 */
GFloatObjectCollection::GFloatObjectCollection(
    const std::size_t &n_cp,
    std::shared_ptr<GFloatObject> tmpl_ptr
)
  : GParameterTCollectionT<GFloatObject>(n_cp, tmpl_ptr) { /* nothing */
}

/******************************************************************************/
/**
 * Creates a deep clone of this object.
 *
 * @return A copy of this object, camouflaged as a GParameterBase
 */
GParameterBase *GFloatObjectCollection::clone_() const {
    return new GFloatObjectCollection(*this);
}

/******************************************************************************/
/**
 * Searches for compliance with expectations with respect to another object
 * of the same type
 *
 * @param cp A constant reference to another GParameterBase object
 * @param e The expected outcome of the comparison
 */
void GFloatObjectCollection::compare_(
    const GParameterBase &cp,
    const Gem::Common::expectation &e,
    [[maybe_unused]] const double & limit
) const {
    using namespace Gem::Common;

    // Check that we are dealing with a GFloatObjectCollection reference independent of this object and convert the pointer
    const GFloatObjectCollection *p_load =
        Gem::Common::g_convert_and_compare<GParameterBase, GFloatObjectCollection>(cp, this);

    GToken token("GFloatObjectCollection", e);

    // Compare our parent data ...
    Gem::Common::compare_base_t<GParameterTCollectionT<GFloatObject>>(*this, *p_load, token);

    // ... no local data

    // React on deviations from the expectation
    token.evaluate();
}

/***********************************************************************************/
/**
 * Emits a name for this class / object
 */
std::string GFloatObjectCollection::name_() const {
    return std::string("GFloatObjectCollection");
}

/******************************************************************************/
/**
 * Loads the data of another GParameterBase
 *
 * @param cp A copy of another GFloatObjectCollection object, camouflaged as a GParameterBase
 */
void GFloatObjectCollection::load_(const GParameterBase *cp) {
    // Convert the pointer to our target type and check for self-assignment
    const GFloatObjectCollection *p_load =
        Gem::Common::g_convert_and_compare<GParameterBase, GFloatObjectCollection>(cp, this);

    // Load our parent class'es data ...
    GParameterTCollectionT<GFloatObject>::load_(cp);

    // ... no local data
}

/******************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GFloatObjectCollection::modify_GUnitTests_() {
#ifdef GEM_TESTING
    this->fillWithObjects_(10);

    // Call the parent class'es function
    GParameterTCollectionT<GFloatObject>::modify_GUnitTests_();

    return true;
#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset("GFloatObjectCollection::modify_GUnitTests", "GEM_TESTING");
    return false;
#endif                  /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Fills the collection with GFloatObject objects
 */
void GFloatObjectCollection::fillWithObjects_(const std::size_t &n_added_objects) {
#ifdef GEM_TESTING
    // A random generator
    Gem::Hap::GRandomT<Gem::Hap::RANDFLAVOURS::RANDOMPROXY> gr;

    // Clear the collection, so we can start fresh
    CHECK_NOTHROW(this->clear());

    // Add GFloatObject items with adaptors to p_test1
    for(std::size_t i = 0; i < n_added_objects; i++) {
        // Create a suitable adaptor
        std::shared_ptr<GFloatGaussAdaptor> gfga_ptr;

        CHECK_NOTHROW(
            gfga_ptr = std::make_shared<GFloatGaussAdaptor>(0.025, 0.1, 0., 1., 1.0)
        );
        CHECK_NOTHROW(
            gfga_ptr->setAdaptionThreshold(0)
        ); // Make sure the adaptor's internal parameters don't change through the adaption
        CHECK_NOTHROW(gfga_ptr->setAdaptionMode(adaptionMode::ALWAYS)); // Always adapt

        // Create a suitable GFloatObject object
        std::shared_ptr<GFloatObject> gfo_ptr;

        CHECK_NOTHROW(
            gfo_ptr = std::make_shared<GFloatObject>(-100., 100.)
        ); // Initialization in the range -100, 100

        // Add the adaptor
        CHECK_NOTHROW(gfo_ptr->addAdaptor(gfga_ptr));

        // Randomly initialize the GFloatObject object, so it is unique
        CHECK_NOTHROW(gfo_ptr->randomInit(activityMode::ALLPARAMETERS, gr));

        // Add the object to the collection
        CHECK_NOTHROW(this->push_back(gfo_ptr));
    }

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset("GFloatObjectCollection::fillWithObjects", "GEM_TESTING");
#endif                  /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GFloatObjectCollection::specificTestsNoFailureExpected_GUnitTests_() {
#ifdef GEM_TESTING
    // Some settings
    constexpr std::size_t n_added_objects = 10;
    constexpr std::size_t n_tests = 100;
    constexpr float lowerinitboundary = -10.1;
    constexpr float upperinitboundary = 10.1;
    constexpr float fixedvalueinit = 1.;
    constexpr float multvalue = 3.;
    constexpr float randlowerboundary = 0.;
    constexpr float randupperboundary = 10.;

    // Get a random number generator
    Gem::Hap::GRandomT<Gem::Hap::RANDFLAVOURS::RANDOMPROXY> gr;

    // --------------------------------------------------------------------------

    { // Call the parent class'es function
        std::shared_ptr<GFloatObjectCollection> p_test = this->clone<GFloatObjectCollection>();

        // Fill p_test with objects
        p_test->fillWithObjects_(n_added_objects);

        // Execute the parent class'es tests
        p_test->GParameterTCollectionT<GFloatObject>::specificTestsNoFailureExpected_GUnitTests_();
    }

    // --------------------------------------------------------------------------

    { // Test the GParameterTCollectionT<T>::adapt() implementation
        std::shared_ptr<GFloatObjectCollection> p_test1 = this->clone<GFloatObjectCollection>();
        std::shared_ptr<GFloatObjectCollection> p_test2 = this->clone<GFloatObjectCollection>();

        // Fill p_test1 with objects
        CHECK_NOTHROW(p_test1->fillWithObjects_(n_added_objects));

        // Load the p_test1 data into p_test2
        CHECK_NOTHROW(p_test2->load(p_test1));

        // Check that both objects are identical
        CHECK(*p_test1 == *p_test2);

        // Modify p_test2 using its adapt-function
        CHECK_NOTHROW(p_test2->adapt(gr));

        // Check that both objects differ
        // Check that both objects are identical
        CHECK(*p_test1 != *p_test2);

        // All items in the collection must have been modified individually
        for(std::size_t i = 0; i < n_added_objects; i++) {
            CHECK(*(p_test1->at(i)) != *(p_test2->at(i)));
        }
    }

    // --------------------------------------------------------------------------

    { // Test initialization of GDouble objects with a fixed floating point value
        std::shared_ptr<GFloatObjectCollection> p_test1 = this->clone<GFloatObjectCollection>();

        // Fill p_test1 with objects
        CHECK_NOTHROW(p_test1->fillWithObjects_(n_added_objects));

        // Cross check the amount of items in the collection
        CHECK(p_test1->size() == n_added_objects);

        // Initialize with a fixed value
        CHECK_NOTHROW(p_test1->fixedValueInit<float>(fixedvalueinit, activityMode::ALLPARAMETERS));

        // Check that all items have the expected value
        for(std::size_t i = 0; i < n_added_objects; i++) {
            CHECK(p_test1->at(i)->value() == fixedvalueinit);
        }
    }

    // --------------------------------------------------------------------------

    { // Test multiplication with a fixed value
        std::shared_ptr<GFloatObjectCollection> p_test1 = this->clone<GFloatObjectCollection>();

        // Fill p_test1 with objects
        CHECK_NOTHROW(p_test1->fillWithObjects_(n_added_objects));

        // Cross check the amount of items in the collection
        CHECK(p_test1->size() == n_added_objects);

        // Initialize with a fixed value (1), so we have a defined start value for the multiplication
        CHECK_NOTHROW(p_test1->fixedValueInit<float>(fixedvalueinit, activityMode::ALLPARAMETERS));

        // Multiply all items with a defined value
        CHECK_NOTHROW(p_test1->multiplyBy<float>(multvalue, activityMode::ALLPARAMETERS));

        // Check the values of all items
        for(std::size_t i = 0; i < n_added_objects; i++) {
            CHECK(p_test1->at(i)->value() == multvalue);
        }
    }

    // --------------------------------------------------------------------------

    { // Test multiplication with a random number in a given range
        std::shared_ptr<GFloatObjectCollection> p_test1 = this->clone<GFloatObjectCollection>();
        std::shared_ptr<GFloatObjectCollection> p_test2 = this->clone<GFloatObjectCollection>();

        // Fill p_test1 with objects
        CHECK_NOTHROW(p_test1->fillWithObjects_(n_added_objects));

        // Make sure p_test2 is empty
        CHECK_NOTHROW(p_test2->clear());

        // Cross check the amount of items in the collection
        CHECK(p_test1->size() == n_added_objects);

        // Initialize with a fixed value (1), so we have a defined start value for the multiplication
        CHECK_NOTHROW(p_test1->fixedValueInit<float>(fixedvalueinit, activityMode::ALLPARAMETERS));

        // Load p_test1 into p_test2
        CHECK_NOTHROW(p_test2->load(p_test1));

        // Make sure both objects are the same
        CHECK(*p_test1 == *p_test2);

        // Multiply p_test1 with a random value
        CHECK_NOTHROW(p_test1->multiplyByRandom<float>(
            lowerinitboundary,
            upperinitboundary,
            activityMode::ALLPARAMETERS,
            gr
        ));

        // Check that p_test1 and p_test2 differ
        CHECK(*p_test1 != *p_test2);

        // Check that each item individually differs
        for(std::size_t i = 0; i < n_added_objects; i++) {
            CHECK(p_test1->at(i)->value() != p_test2->at(i)->value());
        }
    }

    // --------------------------------------------------------------------------

    { // Test multiplication with a random number in a the range [0,1[
        std::shared_ptr<GFloatObjectCollection> p_test1 = this->clone<GFloatObjectCollection>();
        std::shared_ptr<GFloatObjectCollection> p_test2 = this->clone<GFloatObjectCollection>();

        // Fill p_test1 with objects
        CHECK_NOTHROW(p_test1->fillWithObjects_(n_added_objects));

        // Make sure p_test2 is empty
        CHECK_NOTHROW(p_test2->clear());

        // Cross check the amount of items in the collection
        CHECK(p_test1->size() == n_added_objects);

        // Initialize with a fixed value (1), so we have a defined start value for the multiplication
        CHECK_NOTHROW(p_test1->fixedValueInit<float>(fixedvalueinit, activityMode::ALLPARAMETERS));

        // Load p_test1 into p_test2
        CHECK_NOTHROW(p_test2->load(p_test1));

        // Make sure both objects are the same
        CHECK(*p_test1 == *p_test2);

        // Multiply p_test1 with a random value
        CHECK_NOTHROW(p_test1->multiplyByRandom<float>(activityMode::ALLPARAMETERS, gr));

        // Check that p_test1 and p_test2 differ
        CHECK(*p_test1 != *p_test2);

        // Check that each item individually differs
        for(std::size_t i = 0; i < n_added_objects; i++) {
            CHECK(p_test1->at(i)->value() != p_test2->at(i)->value());
        }
    }

    // --------------------------------------------------------------------------

    { // Test addition of another object
        std::shared_ptr<GFloatObjectCollection> p_test1 = this->clone<GFloatObjectCollection>();
        std::shared_ptr<GFloatObjectCollection> p_test2 = this->clone<GFloatObjectCollection>();

        // Fill p_test1 with objects
        CHECK_NOTHROW(p_test1->fillWithObjects_(n_added_objects));

        // Make sure p_test2 is empty
        CHECK_NOTHROW(p_test2->clear());

        // Load p_test1 into p_test2
        CHECK_NOTHROW(p_test2->load(p_test1));

        // Initialize p_test1 with a fixed value (1)
        CHECK_NOTHROW(p_test1->fixedValueInit<float>(float(1.), activityMode::ALLPARAMETERS));
        // Initialize p_test2 with a fixed value (2)
        CHECK_NOTHROW(p_test2->fixedValueInit<float>(float(2.), activityMode::ALLPARAMETERS));

        // Add p_test1 to p_test2
        CHECK_NOTHROW(p_test2->add<float>(p_test1, activityMode::ALLPARAMETERS));

        // Check each position of p_test2 individually
        for(std::size_t i = 0; i < n_added_objects; i++) {
            CHECK(p_test2->at(i)->value() == float(2.) + float(1.));
        }
    }

    // --------------------------------------------------------------------------

    { // Test subtraction of another object
        std::shared_ptr<GFloatObjectCollection> p_test1 = this->clone<GFloatObjectCollection>();
        std::shared_ptr<GFloatObjectCollection> p_test2 = this->clone<GFloatObjectCollection>();

        // Fill p_test1 with objects
        CHECK_NOTHROW(p_test1->fillWithObjects_(n_added_objects));

        // Make sure p_test2 is empty
        CHECK_NOTHROW(p_test2->clear());

        // Load p_test1 into p_test2
        CHECK_NOTHROW(p_test2->load(p_test1));

        // Initialize p_test1 with a fixed value (1)
        CHECK_NOTHROW(p_test1->fixedValueInit<float>(float(1.), activityMode::ALLPARAMETERS));
        // Initialize p_test2 with a fixed value (2)
        CHECK_NOTHROW(p_test2->fixedValueInit<float>(float(2.), activityMode::ALLPARAMETERS));

        // Subtract p_test1 from p_test2
        CHECK_NOTHROW(p_test2->subtract<float>(p_test1, activityMode::ALLPARAMETERS));

        // Check each position of p_test2 individually
        for(std::size_t i = 0; i < n_added_objects; i++) {
            CHECK(p_test2->at(i)->value() == float(2.) - float(1.));
        }
    }

    // --------------------------------------------------------------------------

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset(
        "GFloatObjectCollection::specificTestsNoFailureExpected_GUnitTests",
        "GEM_TESTING"
    );
#endif                  /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GFloatObjectCollection::specificTestsFailuresExpected_GUnitTests_() {
#ifdef GEM_TESTING
    // Some settings
    constexpr std::size_t n_added_objects = 10;

    // Call the parent class'es function
    GParameterTCollectionT<GFloatObject>::specificTestsFailuresExpected_GUnitTests_();

    // --------------------------------------------------------------------------

    { // Test that fpAdd throws if an item of invalid size is added (Test of GParameterTCollectionT<T>::fpAdd() )
        std::shared_ptr<GFloatObjectCollection> p_test1 = this->clone<GFloatObjectCollection>();
        std::shared_ptr<GFloatObjectCollection> p_test2 = this->clone<GFloatObjectCollection>();

        // Fill p_test1 with objects
        CHECK_NOTHROW(p_test1->fillWithObjects_(n_added_objects));

        // Clear p_test2, so we are sure it is empty
        CHECK_NOTHROW(p_test2->clear());

        // Check that both objects are in-equal
        CHECK(*p_test1 != *p_test2);

        // Check that the sizes differ
        CHECK((p_test1->size() != p_test2->size() && p_test2->empty()));

        // Adding p_test2 to p_test1 should throw
        CHECK_THROWS_AS(
            p_test1->add<float>(p_test2, activityMode::ALLPARAMETERS),
            geneva_exception
        );
    }

    // --------------------------------------------------------------------------

    { // Test that fpSubtract throws if an item of invalid size is added (Test of GParameterTCollectionT<T>::fpSubtract() )
        std::shared_ptr<GFloatObjectCollection> p_test1 = this->clone<GFloatObjectCollection>();
        std::shared_ptr<GFloatObjectCollection> p_test2 = this->clone<GFloatObjectCollection>();

        // Fill p_test1 with objects
        CHECK_NOTHROW(p_test1->fillWithObjects_(n_added_objects));

        // Clear p_test2, so we are sure it is empty
        CHECK_NOTHROW(p_test2->clear());

        // Check that both objects are in-equal
        CHECK(*p_test1 != *p_test2);

        // Check that the sizes differ
        CHECK((p_test1->size() != p_test2->size() && p_test2->empty()));

        // Subtracting p_test2 from p_test1 should throw
        CHECK_THROWS_AS(
            p_test1->subtract<float>(p_test2, activityMode::ALLPARAMETERS),
            geneva_exception
        );
    }

    // --------------------------------------------------------------------------

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset(
        "GFloatObjectCollection::specificTestsFailuresExpected_GUnitTests",
        "GEM_TESTING"
    );
#endif                  /* GEM_TESTING */
}

/******************************************************************************/

} /* namespace Gem::Geneva::Parameters */
