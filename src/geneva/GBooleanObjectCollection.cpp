/**
 * @file
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
 * Geneva was started by Dr. Rüdiger Berlich and was later maintained together
 * with Dr. Ariel Garcia under the auspices of Gemfony scientific. For further
 * information on Gemfony scientific, see http://www.gemfomy.eu .
 *
 * The majority of files in Geneva was released under the Apache license v2.0
 * in February 2020.
 *
 * See the NOTICE file in the top-level directory of the Geneva library
 * collection for a list of contributors and copyright information.
 *
 ********************************************************************************/

#include "geneva/GBooleanObjectCollection.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GBooleanObjectCollection)

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * Initialization with a number of identical GBooleanObject objects
 */
GBooleanObjectCollection::GBooleanObjectCollection(
    const std::size_t &nVals, std::shared_ptr<GBooleanObject> tmpl_ptr
)
    :
    GParameterTCollectionT<GBooleanObject>(
        nVals
        , tmpl_ptr
    ) { /* nothing */ }

// Tested in this file

/******************************************************************************/
/**
 * Initialization with a number of GBoolean objects with a given probability for the value "true"
 */
GBooleanObjectCollection::GBooleanObjectCollection(
    const std::size_t &nVals, const double &probability
) {
    for (std::size_t i = 0; i < nVals; i++) {
        this->push_back(std::shared_ptr<GBooleanObject>(new GBooleanObject(probability)));
    }
}

// Tested in this file

/******************************************************************************/
/**
 * Creates a deep clone of this object.
 *
 * @return A copy of this object, camouflaged as a GObject
 */
GObject *GBooleanObjectCollection::clone_() const {
    return new GBooleanObjectCollection(*this);
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
void GBooleanObjectCollection::compare_(
    const GObject &cp, const Gem::Common::expectation &e, const double &limit
) const {
    using namespace Gem::Common;

    // Check that we are dealing with a GBooleanObjectCollection reference independent of this object and convert the pointer
    const GBooleanObjectCollection *p_load = Gem::Common::g_convert_and_compare<GObject, GBooleanObjectCollection>(
        cp
        , this
    );

    GToken token(
        "GBooleanObjectCollection"
        , e
    );

    // Compare our parent data ...
    Gem::Common::compare_base_t<GParameterTCollectionT<GBooleanObject>>(
        *this
        , *p_load
        , token
    );

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
 * Loads the data of another GObject
 *
 * @param cp A copy of another GBooleanObjectCollection object, camouflaged as a GObject
 */
void GBooleanObjectCollection::load_(const GObject *cp) {
    // Convert the pointer to our target type and check for self-assignment
    const GBooleanObjectCollection *p_load = Gem::Common::g_convert_and_compare<GObject, GBooleanObjectCollection>(
        cp
        , this
    );

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

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset("GBooleanObjectCollection::modify_GUnitTests", "GEM_TESTING");
    return false;
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Fills the collection with GBooleanObject objects
 */
void GBooleanObjectCollection::fillWithObjects_(const std::size_t &nAddedObjects) {
#ifdef GEM_TESTING
    // A random generator
    Gem::Hap::GRandomT<Gem::Hap::RANDFLAVOURS::RANDOMPROXY> gr;

    // Clear the collection, so we can start fresh
    BOOST_CHECK_NO_THROW(this->clear());

    // Add GBooleanObject items with adaptors to p_test1
    for (std::size_t i = 0; i < nAddedObjects; i++) {
        // Create a suitable adaptor
        std::shared_ptr<GBooleanAdaptor> gba_ptr;

        BOOST_CHECK_NO_THROW(gba_ptr = std::shared_ptr<GBooleanAdaptor>(new GBooleanAdaptor(1.0)));
        BOOST_CHECK_NO_THROW(gba_ptr->setAdaptionThreshold(
            0
        )); // Make sure the adaptor's internal parameters don't change through the adaption
        BOOST_CHECK_NO_THROW(gba_ptr->setAdaptionMode(adaptionMode::ALWAYS)); // Always adapt

        // Create a suitable GBooleanObject object
        std::shared_ptr<GBooleanObject> gbo_ptr;

        BOOST_CHECK_NO_THROW(gbo_ptr
                                 = std::shared_ptr<GBooleanObject>(new GBooleanObject())); // Initialization with standard values

        // Add the adaptor
        BOOST_CHECK_NO_THROW(gbo_ptr->addAdaptor(gba_ptr));

        // Randomly initialize the GBooleanObject object, so it is unique
        BOOST_CHECK_NO_THROW(gbo_ptr->randomInit(
            activityMode::ALLPARAMETERS
            , gr
        ));

        // Add the object to the collection
        BOOST_CHECK_NO_THROW(this->push_back(gbo_ptr));
    }

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset("GBooleanObjectCollection::fillWithObjects", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GBooleanObjectCollection::specificTestsNoFailureExpected_GUnitTests_() {
#ifdef GEM_TESTING
    // Some settings
    const std::size_t nAddedObjects = 10;
    const std::size_t nTests = 10000;
    const double LOWERINITBOUNDARY = -10;
    const double UPPERINITBOUNDARY = 10;
    const double FIXEDVALUEINIT = 1.;
    const double MULTVALUE = 3.;
    const double RANDLOWERBOUNDARY = 0.;
    const double RANDUPPERBOUNDARY = 10.;
    const double LOWERBND = 0.8, UPPERBND = 1.2;

    // Get a random number generator
    Gem::Hap::GRandomT<Gem::Hap::RANDFLAVOURS::RANDOMPROXY> gr;

    //----------------------------------------------------------------------------

    { // Call the parent class'es function
        std::shared_ptr<GBooleanObjectCollection> p_test = this->clone<GBooleanObjectCollection>();

        // Fill p_test with objects
        p_test->fillWithObjects_(nAddedObjects);

        // Call the parent's tests
        p_test->GParameterTCollectionT<GBooleanObject>::specificTestsNoFailureExpected_GUnitTests_();
    }

    //----------------------------------------------------------------------------

    { // Check default construction
        GBooleanObjectCollection gboc;
        BOOST_CHECK(gboc.empty());
    }

    //----------------------------------------------------------------------------

    { // Check copy construction
        GBooleanObjectCollection gboc1;
        gboc1.push_back(std::shared_ptr<GBooleanObject>(new GBooleanObject(0.5)));
        BOOST_CHECK(gboc1.size() == 1);
        GBooleanObjectCollection gboc2(gboc1);
        BOOST_CHECK(gboc1.size() == gboc2.size());
        BOOST_CHECK_MESSAGE(
            gboc1.at(0)->value() == gboc2.at(0)->value()
            , "\n"
            << "gboc1.at(0)->value() = " << gboc1.at(0)->value()
            << "gboc2.at(0)->value() = " << gboc2.at(0)->value()
        );
    }

    //----------------------------------------------------------------------------

    { // Check construction with a number of object templates
        std::shared_ptr<GBooleanObject> gbo_ptr(new GBooleanObject(Gem::Common::GDefaultValueT<bool>::value()));
        GBooleanObjectCollection gboc(
            nTests
            , gbo_ptr
        );

        BOOST_CHECK_MESSAGE(
            gboc.size() == nTests
            , "\n"
            << "gboc.size() = " << gboc.size()
            << "nTests = " << nTests
        );

        for (std::size_t i = 0; i < nTests; i++) {
            BOOST_CHECK_MESSAGE(
                gboc.at(i)->value() == Gem::Common::GDefaultValueT<bool>::value()
                , "\n"
                << "gboc.at(" << i << ")->value() = " <<
                gboc.at(i)->value()
                <<
                "Gem::Common::GDefaultValueT<bool>::value() = " <<
                Gem::Common::GDefaultValueT<bool>::value()
            );
        }
    }

    // --------------------------------------------------------------------------

    { // Check construction with a number of GBooleanObject with a given probability for "true"
        GBooleanObjectCollection gboc(
            nTests
            , 0.5
        );

        std::size_t nTrue = 0, nFalse = 0;
        for (std::size_t i = 0; i < nTests; i++) {
            gboc.at(i)->value() ? nTrue++ : nFalse++;
        }

        // We allow a slight deviation, as the initialization is a random process
        BOOST_REQUIRE(nFalse != 0); // There should be a few false values
        double ratio = double(nTrue) / double(nFalse);
        BOOST_CHECK_MESSAGE(
            ratio > LOWERBND && ratio < UPPERBND
            , "\n"
            << "ratio = " << ratio << "\n"
            << "nTrue = " << nTrue << "\n"
            << "nFalse = " << nFalse << "\n"
        );
    }

    // --------------------------------------------------------------------------

    { // Test that the fp-family of functions has no effect on this object (and contained objects)
        std::shared_ptr<GBooleanObjectCollection> p_test1 = this->clone<GBooleanObjectCollection>();
        std::shared_ptr<GBooleanObjectCollection> p_test2 = this->clone<GBooleanObjectCollection>();

        // Fill p_test1 with objects
        BOOST_CHECK_NO_THROW(p_test1->fillWithObjects_(nAddedObjects));

        // Make sure it has the expected size
        BOOST_CHECK(p_test1->size() == nAddedObjects);

        // Load the data into p_test2
        BOOST_CHECK_NO_THROW(p_test2->load(p_test1));

        // Check that both items are identical
        BOOST_CHECK(*p_test1 == *p_test2);

        // Try to add a fixed fp value to p_test1 and check whether it has changed
        BOOST_CHECK_NO_THROW(p_test1->fixedValueInit<double>(
            FIXEDVALUEINIT
            , activityMode::ALLPARAMETERS
        ));
        BOOST_CHECK(*p_test1 == *p_test2);

        // Try to multiply p_test1 with a fixed fp value and check whether it has changed
        BOOST_CHECK_NO_THROW(p_test1->multiplyBy<double>(
            MULTVALUE
            , activityMode::ALLPARAMETERS
        ));
        BOOST_CHECK(*p_test1 == *p_test2);

        // Try to multiply p_test1 with a random fp value in a given range and check whether it has changed
        BOOST_CHECK_NO_THROW(p_test1->multiplyByRandom<double>(
            RANDLOWERBOUNDARY
            , RANDUPPERBOUNDARY
            , activityMode::ALLPARAMETERS
            , gr
        ));
        BOOST_CHECK(*p_test1 == *p_test2);

        // Try to multiply p_test1 with a random fp value in the range [0,1[ and check whether it has changed
        BOOST_CHECK_NO_THROW(p_test1->multiplyByRandom<double>(
            activityMode::ALLPARAMETERS
            , gr
        ));
        BOOST_CHECK(*p_test1 == *p_test2);

        // Try to add p_test2 to p_test1 and see whether it has changed
        BOOST_CHECK_NO_THROW(p_test1->add<double>(
            p_test2
            , activityMode::ALLPARAMETERS
        ));
        BOOST_CHECK(*p_test1 == *p_test2);

        // Try to subtract p_test2 from p_test1 and see whether it has changed
        BOOST_CHECK_NO_THROW(p_test1->subtract<double>(
            p_test2
            , activityMode::ALLPARAMETERS
        ));
        BOOST_CHECK(*p_test1 == *p_test2);
    }

    // --------------------------------------------------------------------------

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset("GBooleanObjectCollection::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
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
    Gem::Common::condnotset("GBooleanObjectCollection::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */
