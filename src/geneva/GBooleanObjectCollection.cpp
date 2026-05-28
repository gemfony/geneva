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

#include "geneva/par/GBooleanObjectCollection.hpp"

#ifdef GEM_TESTING
#include <catch2/catch_test_macros.hpp>
#endif /* GEM_TESTING */

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::Parameters::GBooleanObjectCollection) // NOLINT
namespace Gem::Geneva::Parameters {

/******************************************************************************/
/**
 * Initialization with a number of identical GBooleanObject objects
 */
GBooleanObjectCollection::GBooleanObjectCollection(
    const std::size_t &n_vals,
    std::shared_ptr<GBooleanObject> tmpl_ptr
)
  : GParameterTCollectionT<GBooleanObject>(n_vals, tmpl_ptr) { /* nothing */
}

// Tested in this file

/******************************************************************************/
/**
 * Initialization with a number of GBoolean objects with a given probability for the value "true"
 */
GBooleanObjectCollection::GBooleanObjectCollection(
    const std::size_t &n_vals,
    const double &probability
) {
    for(std::size_t i = 0; i < n_vals; i++) {
        this->push_back(std::make_shared<GBooleanObject>(probability));
    }
}

// Tested in this file

/******************************************************************************/
/**
 * Creates a deep clone of this object.
 *
 * @return A copy of this object, camouflaged as a GParameterBase
 */
GParameterBase *GBooleanObjectCollection::clone_() const {
    return new GBooleanObjectCollection(*this);
}

/******************************************************************************/
/**
 * Searches for compliance with expectations with respect to another object
 * of the same type
 *
 * @param cp A constant reference to another GParameterBase object
 * @param e The expected outcome of the comparison
 * @param limit The maximum deviation for floating point values (important for similarity checks)
 */
void GBooleanObjectCollection::compare_(
    const GParameterBase &cp,
    const Gem::Common::expectation &e,
    const double & /*limit*/
) const {
    using namespace Gem::Common;

    // Check that we are dealing with a GBooleanObjectCollection reference independent of this object and convert the pointer
    const GBooleanObjectCollection *p_load =
        Gem::Common::g_convert_and_compare<GParameterBase, GBooleanObjectCollection>(cp, this);

    GToken token("GBooleanObjectCollection", e);

    // Compare our parent data ...
    Gem::Common::compare_base_t<GParameterTCollectionT<GBooleanObject>>(*this, *p_load, token);

    // ... no local data

    // React on deviations from the expectation
    token.evaluate();
}

/***********************************************************************************/
/**
 * Emits a name for this class / object
 */
std::string GBooleanObjectCollection::name_() const {
    return std::string("GBooleanObjectCollection");
}

/******************************************************************************/
/**
 * Loads the data of another GParameterBase
 *
 * @param cp A copy of another GBooleanObjectCollection object, camouflaged as a GParameterBase
 */
void GBooleanObjectCollection::load_(const GParameterBase *cp) {
    // Convert the pointer to our target type and check for self-assignment
    const GBooleanObjectCollection *p_load =
        Gem::Common::g_convert_and_compare<GParameterBase, GBooleanObjectCollection>(cp, this);

    // Load our parent class'es data ...
    GParameterTCollectionT<GBooleanObject>::load_(cp);

    // ... no local data
}

/******************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GBooleanObjectCollection::modify_GUnitTests_() {
#ifdef GEM_TESTING
    this->fillWithObjects_(10);

    // Call the parent class'es function
    GParameterTCollectionT<GBooleanObject>::modify_GUnitTests_();

    return true;

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset("GBooleanObjectCollection::modify_GUnitTests", "GEM_TESTING");
    return false;
#endif                  /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Fills the collection with GBooleanObject objects
 */
void GBooleanObjectCollection::fillWithObjects_(const std::size_t &n_added_objects) {
#ifdef GEM_TESTING
    // A random generator
    Gem::Hap::GRandomT<Gem::Hap::RANDFLAVOURS::RANDOMPROXY> gr;

    // Clear the collection, so we can start fresh
    CHECK_NOTHROW(this->clear());

    // Add GBooleanObject items with adaptors to p_test1
    for(std::size_t i = 0; i < n_added_objects; i++) {
        // Create a suitable adaptor
        std::shared_ptr<GBooleanAdaptor> gba_ptr;

        CHECK_NOTHROW(gba_ptr = std::make_shared<GBooleanAdaptor>(1.0));
        CHECK_NOTHROW(
            gba_ptr->setAdaptionThreshold(0)
        ); // Make sure the adaptor's internal parameters don't change through the adaption
        CHECK_NOTHROW(gba_ptr->setAdaptionMode(adaptionMode::ALWAYS)); // Always adapt

        // Create a suitable GBooleanObject object
        std::shared_ptr<GBooleanObject> gbo_ptr;

        CHECK_NOTHROW(
            gbo_ptr = std::make_shared<GBooleanObject>()
        ); // Initialization with standard values

        // Add the adaptor
        CHECK_NOTHROW(gbo_ptr->addAdaptor(gba_ptr));

        // Randomly initialize the GBooleanObject object, so it is unique
        CHECK_NOTHROW(gbo_ptr->randomInit(activityMode::ALLPARAMETERS, gr));

        // Add the object to the collection
        CHECK_NOTHROW(this->push_back(gbo_ptr));
    }

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset("GBooleanObjectCollection::fillWithObjects", "GEM_TESTING");
#endif                  /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GBooleanObjectCollection::specificTestsNoFailureExpected_GUnitTests_() {
#ifdef GEM_TESTING
    // Some settings
    constexpr std::size_t n_added_objects = 10;
    constexpr std::size_t n_tests = 10000;
    constexpr double lowerinitboundary = -10;
    constexpr double upperinitboundary = 10;
    constexpr double fixedvalueinit = 1.;
    constexpr double multvalue = 3.;
    constexpr double randlowerboundary = 0.;
    constexpr double randupperboundary = 10.;
    constexpr double lowerbnd = 0.8;
    constexpr double upperbnd = 1.2;

    // Get a random number generator
    Gem::Hap::GRandomT<Gem::Hap::RANDFLAVOURS::RANDOMPROXY> gr;

    //----------------------------------------------------------------------------

    { // Call the parent class'es function
        std::shared_ptr<GBooleanObjectCollection> p_test = this->clone<GBooleanObjectCollection>();

        // Fill p_test with objects
        p_test->fillWithObjects_(n_added_objects);

        // Call the parent's tests
        p_test
            ->GParameterTCollectionT<GBooleanObject>::specificTestsNoFailureExpected_GUnitTests_();
    }

    //----------------------------------------------------------------------------

    { // Check default construction
        GBooleanObjectCollection gboc;
        CHECK(gboc.empty());
    }

    //----------------------------------------------------------------------------

    { // Check copy construction
        GBooleanObjectCollection gboc1;
        gboc1.push_back(std::make_shared<GBooleanObject>(0.5));
        CHECK(gboc1.size() == 1);
        GBooleanObjectCollection gboc2(gboc1);
        CHECK(gboc1.size() == gboc2.size());
        INFO(
            "\n"
            << "gboc1.at(0)->value() = " << gboc1.at(0)->value()
            << "gboc2.at(0)->value() = " << gboc2.at(0)->value()
        );
        CHECK(gboc1.at(0)->value() == gboc2.at(0)->value());
    }

    //----------------------------------------------------------------------------

    { // Check construction with a number of object templates
        std::shared_ptr<GBooleanObject> gbo_ptr(
            new GBooleanObject(Gem::Common::GDefaultValueT<bool>::value())
        );
        GBooleanObjectCollection gboc(n_tests, gbo_ptr);

        INFO("\n" << "gboc.size() = " << gboc.size() << "n_tests = " << n_tests);
        CHECK(gboc.size() == n_tests);

        for(std::size_t i = 0; i < n_tests; i++) {
            INFO(
                "\n"
                << "gboc.at(" << i << ")->value() = " << gboc.at(i)->value()
                << "Gem::Common::GDefaultValueT<bool>::value() = "
                << Gem::Common::GDefaultValueT<bool>::value()
            );
            CHECK(gboc.at(i)->value() == Gem::Common::GDefaultValueT<bool>::value());
        }
    }

    // --------------------------------------------------------------------------

    { // Check construction with a number of GBooleanObject with a given probability for "true"
        GBooleanObjectCollection gboc(n_tests, 0.5);

        std::size_t n_true = 0;
        std::size_t n_false = 0;
        for(std::size_t i = 0; i < n_tests; i++) {
            gboc.at(i)->value() ? n_true++ : n_false++;
        }

        // We allow a slight deviation, as the initialization is a random process
        REQUIRE(n_false != 0); // There should be a few false values
        double ratio = static_cast<double>(n_true) / static_cast<double>(n_false);
        INFO(
            "\n"
            << "ratio = " << ratio << "\n"
            << "n_true = " << n_true << "\n"
            << "n_false = " << n_false << "\n"
        );
        CHECK((ratio > lowerbnd && ratio < upperbnd));
    }

    // --------------------------------------------------------------------------

    { // Test that the fp-family of functions has no effect on this object (and contained objects)
        std::shared_ptr<GBooleanObjectCollection> p_test1 = this->clone<GBooleanObjectCollection>();
        std::shared_ptr<GBooleanObjectCollection> p_test2 = this->clone<GBooleanObjectCollection>();

        // Fill p_test1 with objects
        CHECK_NOTHROW(p_test1->fillWithObjects_(n_added_objects));

        // Make sure it has the expected size
        CHECK(p_test1->size() == n_added_objects);

        // Load the data into p_test2
        CHECK_NOTHROW(p_test2->load(p_test1));

        // Check that both items are identical
        CHECK(*p_test1 == *p_test2);

        // Try to add a fixed fp value to p_test1 and check whether it has changed
        CHECK_NOTHROW(p_test1->fixedValueInit<double>(fixedvalueinit, activityMode::ALLPARAMETERS));
        CHECK(*p_test1 == *p_test2);

        // Try to multiply p_test1 with a fixed fp value and check whether it has changed
        CHECK_NOTHROW(p_test1->multiplyBy<double>(multvalue, activityMode::ALLPARAMETERS));
        CHECK(*p_test1 == *p_test2);

        // Try to multiply p_test1 with a random fp value in a given range and check whether it has changed
        CHECK_NOTHROW(p_test1->multiplyByRandom<double>(
            randlowerboundary,
            randupperboundary,
            activityMode::ALLPARAMETERS,
            gr
        ));
        CHECK(*p_test1 == *p_test2);

        // Try to multiply p_test1 with a random fp value in the range [0,1[ and check whether it has changed
        CHECK_NOTHROW(p_test1->multiplyByRandom<double>(activityMode::ALLPARAMETERS, gr));
        CHECK(*p_test1 == *p_test2);

        // Try to add p_test2 to p_test1 and see whether it has changed
        CHECK_NOTHROW(p_test1->add<double>(p_test2, activityMode::ALLPARAMETERS));
        CHECK(*p_test1 == *p_test2);

        // Try to subtract p_test2 from p_test1 and see whether it has changed
        CHECK_NOTHROW(p_test1->subtract<double>(p_test2, activityMode::ALLPARAMETERS));
        CHECK(*p_test1 == *p_test2);
    }

    // --------------------------------------------------------------------------

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset(
        "GBooleanObjectCollection::specificTestsNoFailureExpected_GUnitTests",
        "GEM_TESTING"
    );
#endif                  /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GBooleanObjectCollection::specificTestsFailuresExpected_GUnitTests_() {
#ifdef GEM_TESTING
    // Call the parent class'es function
    GParameterTCollectionT<GBooleanObject>::specificTestsFailuresExpected_GUnitTests_();

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset(
        "GBooleanObjectCollection::specificTestsFailuresExpected_GUnitTests",
        "GEM_TESTING"
    );
#endif                  /* GEM_TESTING */
}

/******************************************************************************/

} /* namespace Gem::Geneva::Parameters */
