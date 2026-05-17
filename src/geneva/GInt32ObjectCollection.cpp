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

#include "geneva/par/GInt32ObjectCollection.hpp"

#ifdef GEM_TESTING
#include <catch2/catch_test_macros.hpp>
#endif /* GEM_TESTING */

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::Parameters::GInt32ObjectCollection) // NOLINT

namespace Gem::Geneva::Parameters {

/******************************************************************************/
/**
 * Initialization with a number of identical GConstrainedDoubleObject objects
 */
GInt32ObjectCollection::GInt32ObjectCollection(
    const std::size_t &n_cp,
    std::shared_ptr<GInt32Object> tmpl_ptr
)
  : GParameterTCollectionT<GInt32Object>(n_cp, tmpl_ptr) { /* nothing */
}

/******************************************************************************/
/**
 * Creates a deep clone of this object.
 *
 * @return A copy of this object, camouflaged as a GObject
 */
GObject *GInt32ObjectCollection::clone_() const {
    return new GInt32ObjectCollection(*this);
}

/******************************************************************************/
/**
 * Searches for compliance with expectations with respect to another object
 * of the same type
 *
 * @param cp A constant reference to another GObject object
 * @param e The expected outcome of the comparison
 * @param limit The maximum deviation for floating point values (important for similarity checks)
 */
void GInt32ObjectCollection::compare_(
    const GObject &cp,
    const Gem::Common::expectation &e,
    const double & /*limit*/
) const {
    using namespace Gem::Common;

    // Check that we are dealing with a GInt32ObjectCollection reference independent of this object and convert the pointer
    const GInt32ObjectCollection *p_load =
        Gem::Common::g_convert_and_compare<GObject, GInt32ObjectCollection>(cp, this);

    GToken token("GInt32ObjectCollection", e);

    // Compare our parent data ...
    Gem::Common::compare_base_t<GParameterTCollectionT<GInt32Object>>(*this, *p_load, token);

    // ... no local data

    // React on deviations from the expectation
    token.evaluate();
}

/***********************************************************************************/
/**
 * Emits a name for this class / object
 */
std::string GInt32ObjectCollection::name_() const {
    return std::string("GInt32ObjectCollection");
}

/******************************************************************************/
/**
 * Loads the data of another GObject
 *
 * @param cp A copy of another GInt32ObjectCollection object, camouflaged as a GObject
 */
void GInt32ObjectCollection::load_(const GObject *cp) {
    // Convert the pointer to our target type and check for self-assignment
    const GInt32ObjectCollection *p_load =
        Gem::Common::g_convert_and_compare<GObject, GInt32ObjectCollection>(cp, this);

    // Load our parent class'es data ...
    GParameterTCollectionT<GInt32Object>::load_(cp);

    // ... no local data
}

/******************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GInt32ObjectCollection::modify_GUnitTests_() {
#ifdef GEM_TESTING
    this->fillWithObjects_(10);

    // Call the parent class'es function
    GParameterTCollectionT<GInt32Object>::modify_GUnitTests_();

    return true;

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset("GInt32ObjectCollection::modify_GUnitTests", "GEM_TESTING");
    return false;
#endif                  /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Fills the collection with GInt32Object objects
 */
void GInt32ObjectCollection::fillWithObjects_(const std::size_t &n_added_objects) {
#ifdef GEM_TESTING
    // A random generator
    Gem::Hap::GRandomT<Gem::Hap::RANDFLAVOURS::RANDOMPROXY> gr;

    // Clear the collection, so we can start fresh
    CHECK_NOTHROW(this->clear());

    // Add GInt32Object items with adaptors to p_test1
    for(std::size_t i = 0; i < n_added_objects; i++) {
        // Create a suitable adaptor
        std::shared_ptr<GInt32GaussAdaptor> giga_ptr;

        CHECK_NOTHROW(
            giga_ptr =
                std::make_shared<GInt32GaussAdaptor>(0.025, 0.1, 0, 1, 1.0)
        );
        CHECK_NOTHROW(
            giga_ptr->setAdaptionThreshold(0)
        ); // Make sure the adaptor's internal parameters don't change through the adaption
        CHECK_NOTHROW(giga_ptr->setAdaptionMode(adaptionMode::ALWAYS)); // Always adapt

        // Create a suitable GInt32Object object
        std::shared_ptr<GInt32Object> gio_ptr;

        CHECK_NOTHROW(
            gio_ptr = std::make_shared<GInt32Object>(-100, 100)
        ); // Initialization in the range -100, 100

        // Add the adaptor
        CHECK_NOTHROW(gio_ptr->addAdaptor(giga_ptr));

        // Randomly initialize the GInt32Object object, so it is unique
        CHECK_NOTHROW(gio_ptr->randomInit(activityMode::ALLPARAMETERS, gr));

        // Add the object to the collection
        CHECK_NOTHROW(this->push_back(gio_ptr));
    }

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset("GInt32ObjectCollection::fillWithObjects", "GEM_TESTING");
#endif                  /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GInt32ObjectCollection::specificTestsNoFailureExpected_GUnitTests_() {
#ifdef GEM_TESTING
    // A random generator
    Gem::Hap::GRandomT<Gem::Hap::RANDFLAVOURS::RANDOMPROXY> gr;

    // Some settings
    const std::size_t n_added_objects = 10;
    const std::size_t n_tests = 100;
    const double lowerinitboundary = -10;
    const double upperinitboundary = 10;
    const double fixedvalueinit = 1.;
    const double multvalue = 3.;
    const double randlowerboundary = 0.;
    const double randupperboundary = 10.;

    // --------------------------------------------------------------------------

    { // Call the parent class'es function
        std::shared_ptr<GInt32ObjectCollection> p_test = this->clone<GInt32ObjectCollection>();

        // Fill p_test with objects
        p_test->fillWithObjects_(n_added_objects);

        // Execute the parent class'es tests
        p_test->GParameterTCollectionT<GInt32Object>::specificTestsNoFailureExpected_GUnitTests_();
    }

    // --------------------------------------------------------------------------

    { // Test that the fp-family of functions has no effect on this object (and contained objects)
        std::shared_ptr<GInt32ObjectCollection> p_test1 = this->clone<GInt32ObjectCollection>();
        std::shared_ptr<GInt32ObjectCollection> p_test2 = this->clone<GInt32ObjectCollection>();

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
        "GInt32ObjectCollection::specificTestsNoFailureExpected_GUnitTests",
        "GEM_TESTING"
    );
#endif                  /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GInt32ObjectCollection::specificTestsFailuresExpected_GUnitTests_() {
#ifdef GEM_TESTING
    // Call the parent class'es function
    GParameterTCollectionT<GInt32Object>::specificTestsFailuresExpected_GUnitTests_();

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset(
        "GInt32ObjectCollection::specificTestsFailuresExpected_GUnitTests",
        "GEM_TESTING"
    );
#endif                  /* GEM_TESTING */
}

/******************************************************************************/

} /* namespace Gem::Geneva::Parameters */
