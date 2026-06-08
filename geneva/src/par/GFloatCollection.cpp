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

#include "geneva/par/GFloatCollection.hpp"
#include "common/GCommonHelperFunctionsT.hpp"
#include "common/GContainerT.hpp"
#include "common/GExceptions.hpp"
#include "common/GExpectationChecksT.hpp"
#include "common/GLogger.hpp"
#include "geneva/GOptimizationEnums.hpp"
#include "geneva/par/GAdaptorT.hpp"
#include "geneva/par/GFloatGaussAdaptor.hpp"
#include "geneva/par/GFPNumCollectionT.hpp"
#include "geneva/par/GParameterBase.hpp"
#include "geneva/par/GParameterCollectionT.hpp"
#include "hap/GHapEnums.hpp"
#include "hap/GRandomBase.hpp"
#include "hap/GRandomT.hpp"
#include <algorithm>
#include <cstddef>
#include <map>
#include <memory>
#include <random>
#include <vector>

#ifdef GEM_TESTING
#include <catch2/catch_test_macros.hpp>
#endif /* GEM_TESTING */

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::Parameters::GFloatCollection) // NOLINT
namespace Gem::Geneva::Parameters {

/******************************************************************************/
/**
 * Initialization with a number of random values in a given range
 *
 * @param nval The amount of random values
 * @param min The minimum random value
 * @param max The maximum random value
 */
GFloatCollection::GFloatCollection(const std::size_t &nval, const float &min, const float &max)
  : GFPNumCollectionT<float>(nval, min, max) { /* nothing */
}

/******************************************************************************/
/**
 * Initialization with a number of predefined values in all positions
 *
 * @param nval The amount of random values
 * @param val The predefined value to be assigned to all positions
 * @param min The minimum random value
 * @param max The maximum random value
 */
GFloatCollection::GFloatCollection(
    const std::size_t &nval,
    const float &val,
    const float &min,
    const float &max
)
  : GFPNumCollectionT<float>(nval, val, min, max) { /* nothing */
}

/******************************************************************************/
/**
 * Creates a deep clone of this object.
 *
 * @return A copy of this object, camouflaged as a GParameterBase
 */
GParameterBase *GFloatCollection::clone_() const {
    return new GFloatCollection(*this);
}

/******************************************************************************/
/**
 * Searches for compliance with expectations with respect to another object
 * of the same type
 *
 * @param cp A constant reference to another GParameterBase object
 * @param e The expected outcome of the comparison
 */
void GFloatCollection::compare_(
    const GParameterBase &cp,
    const Gem::Common::expectation &e,
    [[maybe_unused]] const double & limit
) const {
    using namespace Gem::Common;

    // Check that we are dealing with a GFloatCollection reference independent of this object and convert the pointer
    const GFloatCollection *p_load =
        Gem::Common::g_convert_and_compare<GParameterBase, GFloatCollection>(cp, this);

    GToken token("GFloatCollection", e);

    // Compare our parent data ...
    Gem::Common::compare_base_t<GFPNumCollectionT<float>>(*this, *p_load, token);

    // ... no local data

    // React on deviations from the expectation
    token.evaluate();
}

/***********************************************************************************/
/**
 * Emits a name for this class / object
 */
std::string GFloatCollection::name_() const {
    return std::string("GFloatCollection");
}

/******************************************************************************/
/**
 * Attach our local values to the vector. This is used to collect all parameters of this type
 * in the sequence in which they were registered.
 */
void GFloatCollection::floatStreamline(
    std::vector<float> &par_vec,
    [[maybe_unused]] const activityMode & am
) const {
    for(const auto &val : *this) {
        par_vec.push_back(val);
    }
}

/******************************************************************************/

/******************************************************************************/
/**
 * Attach boundaries of type float to the vectors
 */
void GFloatCollection::floatBoundaries(
    std::vector<float> &l_bnd_vec,
    std::vector<float> &u_bnd_vec,
    [[maybe_unused]] const activityMode & am
) const {
    // Add as man lower and upper boundaries to the vector as
    // there are variables
    GFloatCollection::const_iterator cit;
    for(cit = this->begin(); cit != this->end(); ++cit) {
        l_bnd_vec.push_back(this->getLowerInitBoundary());
        u_bnd_vec.push_back(this->getUpperInitBoundary());
    }
}

/******************************************************************************/
/**
 * Tell the audience that we own a number of float values
 *
 * @return The number of float parameters
 */
std::size_t GFloatCollection::countFloatParameters(
    [[maybe_unused]] const activityMode & am
) const {
    return this->size();
}

/******************************************************************************/
/**
 * Assigns part of a value vector to the parameter
 */
void GFloatCollection::assignFloatValueVector(
    const std::vector<float> &par_vec,
    std::size_t &pos,
    [[maybe_unused]] const activityMode & am
) {
    for(float & it : *this) {
#ifdef DEBUG
        // Do we have a valid position ?
        if(pos >= par_vec.size()) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GFloatCollection::assignFloatValueVector(const std::vector<float>&, "
                   "std::size_t&):"
                << '\n'
                << "Tried to access position beyond end of vector: " << par_vec.size() << "/" << pos
                << '\n'
            );
        }
#endif

        it = par_vec[pos];
        pos++;
    }
}

/******************************************************************************/

/******************************************************************************/
/**
 * Multiplication with a random value in a given range
 */
void GFloatCollection::floatMultiplyByRandom(
    const float &min,
    const float &max,
    [[maybe_unused]] const activityMode & am
    ,
    Gem::Hap::GRandomBase &gr // NOLINT(misc-unused-parameters)
) {
    std::uniform_real_distribution<float> uniform_real_distribution(min, max);
    for(std::size_t pos = 0; pos < this->size(); pos++) {
        GParameterCollectionT<float>::setValue(
            pos,
            this->value(pos) * uniform_real_distribution(gr)
        );
    }
}

/******************************************************************************/
/**
 * Multiplication with a random value in the range [0,1[
 */
void GFloatCollection::floatMultiplyByRandom(
    [[maybe_unused]] const activityMode & am
    ,
    Gem::Hap::GRandomBase &gr // NOLINT(misc-unused-parameters)
) {
    std::uniform_real_distribution<float> uniform_real_distribution(0., 1.);
    for(std::size_t pos = 0; pos < this->size(); pos++) {
        GParameterCollectionT<float>::setValue(
            pos,
            this->value(pos) * uniform_real_distribution(gr)
        );
    }
}

/******************************************************************************/
/**
 * Multiplication with a constant value
 */
void GFloatCollection::floatMultiplyBy(
    const float &val // NOLINT(misc-unused-parameters)
    ,
    [[maybe_unused]] const activityMode & am
) {
    for(std::size_t pos = 0; pos < this->size(); pos++) {
        GParameterCollectionT<float>::setValue(pos, val * this->value(pos));
    }
}

/******************************************************************************/
/**
 * Initialization with a constant value
 */
void GFloatCollection::floatFixedValueInit(
    const float &val,
    const activityMode & /*am*/ // NOLINT(misc-unused-parameters)
) {
    for(std::size_t pos = 0; pos < this->size(); pos++) {
        GParameterCollectionT<float>::setValue(pos, val);
    }
}

/******************************************************************************/
/**
 * Adds the "same-type" parameters of another GParameterBase object to this one
 */
void GFloatCollection::floatAdd(
    std::shared_ptr<GParameterBase> p_base,
    [[maybe_unused]] const activityMode & am
) {
    // We first need to convert p_base into the local type
    std::shared_ptr<GFloatCollection> p =
        GParameterBase::parameterbase_cast<GFloatCollection>(p_base);

    // Cross-check that the sizes match
    if(this->size() != p->size()) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GFloatCollection::floatAdd():" << '\n'
            << "Sizes of vectors don't match: " << this->size() << "/" << p->size() << '\n'
        );
    }

    for(std::size_t pos = 0; pos < this->size(); pos++) {
        GParameterCollectionT<float>::setValue(pos, this->value(pos) + p->value(pos));
    }
}

/******************************************************************************/
/**
 * Adds the "same-type" parameters of another GParameterBase object to this one
 */
void GFloatCollection::floatSubtract(
    std::shared_ptr<GParameterBase> p_base,
    [[maybe_unused]] const activityMode & am
) {
    // We first need to convert p_base into the local type
    std::shared_ptr<GFloatCollection> p =
        GParameterBase::parameterbase_cast<GFloatCollection>(p_base);

    // Cross-check that the sizes match
    if(this->size() != p->size()) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GFloatCollection::floatSubtract():" << '\n'
            << "Sizes of vectors don't match: " << this->size() << "/" << p->size() << '\n'
        );
    }

    for(std::size_t pos = 0; pos < this->size(); pos++) {
        GParameterCollectionT<float>::setValue(pos, this->value(pos) - p->value(pos));
    }
}

/******************************************************************************/
/**
 * Loads the data of another GParameterBase
 *
 * @param cp A copy of another GFloatCollection object, camouflaged as a GParameterBase
 */
void GFloatCollection::load_(const GParameterBase *cp) {
    // Convert the pointer to our target type and check for self-assignment
    const GFloatCollection *p_load =
        Gem::Common::g_convert_and_compare<GParameterBase, GFloatCollection>(cp, this);

    // Load our parent class'es data ...
    GFPNumCollectionT<float>::load_(cp);

    // ... no local data
}

/******************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GFloatCollection::modify_GUnitTests_() {
#ifdef GEM_TESTING
    bool result = false;

    // Call the parent class'es function
    if(GFPNumCollectionT<float>::modify_GUnitTests_()) {
        result = true;
    }

    this->fillWithData_(10);
    result = true;

    return result;

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset("GFloatCollection::modify_GUnitTests", "GEM_TESTING");
    return false;
#endif                  /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Fills the collection with some random data
 */
void GFloatCollection::fillWithData_(const std::size_t &n_items) {
#ifdef GEM_TESTING
    // Get a random number generator
    Gem::Hap::GRandomT<Gem::Hap::RANDFLAVOURS::RANDOMPROXY> gr;

    // Make sure the collection is empty
    CHECK_NOTHROW(this->clear());

    // Cross check that it really is
    CHECK(this->empty());
    // Use another method
    CHECK(this->empty());

    // Add a single item of defined value, so we can test the find() and count() functions
    CHECK_NOTHROW(this->push_back(0.));

    std::uniform_real_distribution<float> uniform_real_distribution(-10., 10.);
    for(std::size_t i = 1; i < n_items - 1; i++) {
        CHECK_NOTHROW(this->push_back(uniform_real_distribution(gr)));
    }

    // Add a single item of defined value, so we can test the find() and count() functions
    CHECK_NOTHROW(this->push_back(1.));

    // Cross-check the size
    CHECK(this->size() == n_items);
    CHECK(not this->empty());

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset("GFloatCollection::fillWithData", "GEM_TESTING");
#endif                  /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GFloatCollection::specificTestsNoFailureExpected_GUnitTests_() {
#ifdef GEM_TESTING
    // A few settings
    constexpr std::size_t n_items = 10000;
    constexpr std::size_t n_tests = 10;
    constexpr float fixedvalueinit = 1.;

    // Make sure we have an appropriate adaptor loaded when performing these tests
    bool adaptor_stored = false;
    std::unique_ptr<adaptor_base_t> stored_adaptor;

    if(this->hasAdaptor()) {
        stored_adaptor = this->getAdaptor().clone_unique();
        adaptor_stored = true;
    }

    std::shared_ptr<GFloatGaussAdaptor> gfga_ptr(new GFloatGaussAdaptor(0.025, 0.1, 0., 1., 1.0));
    gfga_ptr->setAdaptionThreshold(
        0
    ); // Make sure the adaptor's internal parameters don't change through the adaption
    gfga_ptr->setAdaptionMode(adaptionMode::ALWAYS); // Always adapt
    this->addAdaptor(gfga_ptr);

    // Call the parent class'es function
    GFPNumCollectionT<float>::specificTestsNoFailureExpected_GUnitTests_();

    // Get a random number generator
    Gem::Hap::GRandomT<Gem::Hap::RANDFLAVOURS::RANDOMPROXY> gr;

    //------------------------------------------------------------------------------

    { // Test the GParameterT<T>::adapt() implementation
        std::shared_ptr<GFloatCollection> p_test1 = this->clone<GFloatCollection>();
        std::shared_ptr<GFloatCollection> p_test2 = this->clone<GFloatCollection>();

        if(p_test1->hasAdaptor()) {
            // Make sure the collection is clean
            p_test1->clear();

            // Add a few items
            for(std::size_t i = 0; i < n_items; i++) {
                p_test1->push_back(fixedvalueinit);
            }

            for(std::size_t t = 0; t < n_tests; t++) {
                // Load p_test1 into p_test2
                CHECK_NOTHROW(p_test2->load(p_test1));

                // Make sure the objects match
                CHECK(*p_test1 == *p_test2);

                // Adapt p_test1
                CHECK_NOTHROW(p_test1->adapt(gr));

                // Test whether the two objects differ now
                CHECK(*p_test1 != *p_test2);

                // Check that each element differs
                for(std::size_t i = 0; i < n_items; i++) {
                    CHECK(p_test1->at(i) != p_test2->at(i));
                }
            }
        }
    }

    //------------------------------------------------------------------------------

    { // Test the GPODVectorT<float>::reserve(), capacity() and max_size() functions
        std::shared_ptr<GFloatCollection> p_test1 = this->clone<GFloatCollection>();

        // Make sure the collection is empty
        CHECK_NOTHROW(p_test1->clear());

        // Check the site
        CHECK(p_test1->empty());
        CHECK(p_test1->empty());

        // Check that the maximum size is > 0
        CHECK(p_test1->max_size() > 0);

        // Reserve some space
        CHECK_NOTHROW(p_test1->reserve(n_items));

        // Check that the capacity is > 0
        CHECK(p_test1->capacity() > 0);

        // Add some data
        CHECK_NOTHROW(p_test1->fillWithData_(n_items));

        // Check the size again
        CHECK(p_test1->size() == n_items);
        CHECK(not p_test1->empty());
    }

    //------------------------------------------------------------------------------

    { // Test the GPODVectorT<float>::count(), find() and begin() functions
        std::shared_ptr<GFloatCollection> p_test1 = this->clone<GFloatCollection>();

        // Add some data
        CHECK_NOTHROW(p_test1->fillWithData_(n_items));

        // Count the number of values == 0. . Should be >= 1
        CHECK(p_test1->count(0.) >= 1);
        // Count the number of values == 1. . Should be >= 1
        CHECK(p_test1->count(1.) >= 1);

        // Find the item with value 0. -- the first one is in position 0
        GFloatCollection::const_iterator find_it;
        GFloatCollection::const_iterator pos_it;
        CHECK_NOTHROW(pos_it = p_test1->begin());
        CHECK_NOTHROW(find_it = p_test1->find(0.));
        CHECK(find_it == pos_it);
    }

    //------------------------------------------------------------------------------

    { // Test setting and retrieval of items with the operator[] and at() functions of GPODVectorT<float>
        std::shared_ptr<GFloatCollection> p_test1 = this->clone<GFloatCollection>();

        // Add some data
        CHECK_NOTHROW(p_test1->fillWithData_(n_items));

        // Retrieve items
        CHECK((*p_test1)[0] == 0.);
        CHECK(p_test1->at(0) == 0.);

        // Set and retrieve an item using two different functions
        CHECK_NOTHROW((*p_test1)[0] = 1.);
        CHECK((*p_test1)[0] == 1.);
        CHECK_NOTHROW(p_test1->at(0) = 2.);
        CHECK(p_test1->at(0) == 2.);
    }

    //------------------------------------------------------------------------------

    { // Test the GPODVectorT<float>::front() and back() functions
        std::shared_ptr<GFloatCollection> p_test1 = this->clone<GFloatCollection>();

        // Add some data
        CHECK_NOTHROW(p_test1->fillWithData_(n_items));

        // Check the front and back of the vector -- we know the values
        CHECK(p_test1->front() == 0.);
        CHECK(p_test1->back() == 1.);
    }

    //------------------------------------------------------------------------------

    { // Test iteration over the vector and retrieval of the end() iterator (Test of GPODVectorT<float> functionality)
        std::shared_ptr<GFloatCollection> p_test1 = this->clone<GFloatCollection>();

        // Add some data
        CHECK_NOTHROW(p_test1->fillWithData_(n_items));

        // Iterate over the sequence
        GFloatCollection::iterator it;
        std::size_t item_count = 0;
        for(it = p_test1->begin(); it != p_test1->end(); ++it) {
            item_count++;
        }
        CHECK(item_count == n_items);
    }

    //------------------------------------------------------------------------------

    { // Test inserting and erasure of items, the pop_and_block and resize functions and the getDataCopy and operator= functions (Test of GPODVectorT<float> functionality)
        std::shared_ptr<GFloatCollection> p_test1 = this->clone<GFloatCollection>();

        // Add some data
        CHECK_NOTHROW(p_test1->fillWithData_(n_items));

        // Insert 1 item at position 1 and cross-check
        CHECK_NOTHROW(p_test1->insert(p_test1->begin() + 1, 1.));
        CHECK(p_test1->at(1) == 1.);
        CHECK(p_test1->size() == n_items + 1);

        // Insert another (nItems - 1 ) items at position 0
        CHECK_NOTHROW(p_test1->insert(p_test1->begin(), n_items - 1, 1.));
        CHECK(p_test1->size() == 2 * n_items);
        CHECK(p_test1->at(0) == 1.);

        // Erase 1 item at the beginning and cross-check
        CHECK_NOTHROW(p_test1->erase(p_test1->begin()));
        CHECK(p_test1->size() == 2 * n_items - 1);

        // Erase another nItems - 1 items from the beginning
        CHECK_NOTHROW(p_test1->erase(p_test1->begin(), p_test1->begin() + n_items - 1));
        CHECK(p_test1->size() == n_items);

        // Remove another item at the end
        CHECK_NOTHROW(p_test1->pop_back());
        CHECK(p_test1->size() == n_items - 1);

        // Remove all remaining items
        CHECK_NOTHROW(p_test1->resize(0, 0.));
        CHECK(p_test1->empty());

        // Add a number of identical items, using the resize() function and cross-check
        CHECK_NOTHROW(p_test1->resize(n_items, 1.));
        CHECK(p_test1->size() == n_items);
        CHECK(p_test1->count(1.) == n_items);

        std::vector<float> data_copy;
        CHECK_NOTHROW(p_test1->getDataCopy(data_copy));
        CHECK(data_copy.size() == n_items);
        CHECK(static_cast<std::size_t>(std::count(data_copy.begin(), data_copy.end(), 1.)) == n_items);

        // Assign 1 to all positions and add further items
        for(float & i : data_copy) {
            i = 0.;
        }
        for(std::size_t i = 0; i < n_items; i++) {
            data_copy.push_back(0.);
        }

        // Assign the vector to p_test1 and cross-check
        CHECK_NOTHROW(p_test1->Gem::Common::GPodContainerT<float>::operator=(data_copy));
        CHECK(p_test1->size() == 2 * n_items);
        CHECK(p_test1->count(0.) == 2 * n_items);
    }

    //------------------------------------------------------------------------------

    // Restore the object to its pristine condition
    this->resetAdaptor();

    // Load the old adaptor, if needed
    if(adaptor_stored) {
        this->addAdaptor(Gem::Common::nonOwningShared(stored_adaptor));
    }

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset(
        "GFloatCollection::specificTestsNoFailureExpected_GUnitTests",
        "GEM_TESTING"
    );
#endif                  /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GFloatCollection::specificTestsFailuresExpected_GUnitTests_() {
#ifdef GEM_TESTING
    // Make sure we have an appropriate adaptor loaded when performing these tests
    bool adaptor_stored = false;
    std::unique_ptr<adaptor_base_t> stored_adaptor;

    if(this->hasAdaptor()) {
        stored_adaptor = this->getAdaptor().clone_unique();
        adaptor_stored = true;
    }

    std::shared_ptr<GFloatGaussAdaptor> gfga_ptr(new GFloatGaussAdaptor(0.025, 0.1, 0., 1., 1.0));
    gfga_ptr->setAdaptionThreshold(
        0
    ); // Make sure the adaptor's internal parameters don't change through the adaption
    gfga_ptr->setAdaptionMode(adaptionMode::ALWAYS); // Always adapt
    this->addAdaptor(gfga_ptr);

    // Call the parent class'es function
    GFPNumCollectionT<float>::specificTestsFailuresExpected_GUnitTests_();

    // Nothing to check -- no local data

    // Remove the test adaptor
    this->resetAdaptor();

    // Restore the adaptor to its pristine condition
    if(adaptor_stored) {
        this->addAdaptor(Gem::Common::nonOwningShared(stored_adaptor));
    }

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset(
        "GFloatCollection::specificTestsFailuresExpected_GUnitTests",
        "GEM_TESTING"
    );
#endif                  /* GEM_TESTING */
}

/******************************************************************************/

} /* namespace Gem::Geneva::Parameters */
