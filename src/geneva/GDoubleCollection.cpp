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

#include "geneva/par/GDoubleCollection.hpp"

#ifdef GEM_TESTING
#include <catch2/catch_test_macros.hpp>
#endif /* GEM_TESTING */

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::Parameters::GDoubleCollection) // NOLINT
namespace Gem::Geneva::Parameters {

/******************************************************************************/
/**
 * Initialization with a number of random values in a given range
 *
 * @param nval The amount of random values
 * @param min The minimum random value
 * @param max The maximum random value
 */
GDoubleCollection::GDoubleCollection(const std::size_t &nval, const double &min, const double &max)
  : GFPNumCollectionT<double>(nval, min, max) { /* nothing */
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
GDoubleCollection::GDoubleCollection(
    const std::size_t &nval,
    const double &val,
    const double &min,
    const double &max
)
  : GFPNumCollectionT<double>(nval, val, min, max) { /* nothing */
}

/******************************************************************************/
/**
 * Creates a deep clone of this object.
 *
 * @return A copy of this object, camouflaged as a GObject
 */
GObject *GDoubleCollection::clone_() const {
    return new GDoubleCollection(*this);
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
void GDoubleCollection::compare_(
    const GObject &cp,
    const Gem::Common::expectation &e,
    const double & /*limit*/
) const {
    using namespace Gem::Common;

    // Check that we are dealing with a GDoubleCollection reference independent of this object and convert the pointer
    const GDoubleCollection *p_load =
        Gem::Common::g_convert_and_compare<GObject, GDoubleCollection>(cp, this);

    GToken token("GDoubleCollection", e);

    // Compare our parent data ...
    Gem::Common::compare_base_t<GFPNumCollectionT<double>>(*this, *p_load, token);

    // ... no local data

    // React on deviations from the expectation
    token.evaluate();
}

/***********************************************************************************/
/**
 * Emits a name for this class / object
 */
std::string GDoubleCollection::name_() const {
    return std::string("GDoubleCollection");
}

/******************************************************************************/
/**
 * Attach our local values to the vector. This is used to collect all parameters of this type
 * in the sequence in which they were registered.
 *
 * @param par_vec The vector to which the local value should be attached
 */
void GDoubleCollection::doubleStreamline(
    std::vector<double> &par_vec,
    const activityMode & /*am*/
) const {
    GDoubleCollection::const_iterator cit;
    for(cit = this->begin(); cit != this->end(); ++cit) {
        par_vec.push_back(*cit);
    }
}

/******************************************************************************/
/**
 * Attach our local values to the map. Names are built from the object name and the
 * position in the array.
 *
 * @param par_vec The map to which the local value should be attached
 */
void GDoubleCollection::doubleStreamline(
    std::map<std::string, std::vector<double>> &par_vec,
    const activityMode &am
) const {
#ifdef DEBUG
    if((this->getParameterName()).empty()) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In GDoubleCollection::doubleStreamline(std::map<std::string, std::vector<double>>& "
               "par_vec) const: Error!"
            << '\n'
            << "No name was assigned to the object" << '\n'
        );
    }
#endif /* DEBUG */

    std::vector<double> parameters;
    this->streamline(parameters, am);
    par_vec[this->getParameterName()] = parameters;
}

/******************************************************************************/
/**
 * Attach boundaries of type double to the vectors
 *
 * @param l_bnd_vec A vector of lower double parameter boundaries
 * @param u_bnd_vec A vector of upper double parameter boundaries
 */
void GDoubleCollection::doubleBoundaries(
    std::vector<double> &l_bnd_vec,
    std::vector<double> &u_bnd_vec,
    const activityMode & /*am*/
) const {
    // Add as man lower and upper boundaries to the vector as
    // there are variables
    GDoubleCollection::const_iterator cit;
    for(cit = this->begin(); cit != this->end(); ++cit) {
        l_bnd_vec.push_back(this->getLowerInitBoundary());
        u_bnd_vec.push_back(this->getUpperInitBoundary());
    }
}

/******************************************************************************/
/**
 * Tell the audience that we own a number of double values
 *
 * @param @param am An enum indicating whether only information about active, inactive or all parameters of this type should be extracted
 * @return The number of double parameters
 */
std::size_t GDoubleCollection::countDoubleParameters(
    const activityMode & /*am*/
) const {
    return this->size();
}

/******************************************************************************/
/**
 * Assigns part of a value vector to the parameter
 *
 * @param par_vec The vector from which the data should be taken
 * @param pos The position inside of the vector from which the data is extracted in each turn of the loop
 */
void GDoubleCollection::assignDoubleValueVector(
    const std::vector<double> &par_vec,
    std::size_t &pos,
    const activityMode & /*am*/
) {
    for(GDoubleCollection::iterator it = this->begin(); it != this->end(); ++it) {
#ifdef DEBUG
        // Do we have a valid position ?
        if(pos >= par_vec.size()) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, time_and_place)
                << "In GDoubleCollection::assignDoubleValueVector(const std::vector<double>&, "
                   "std::size_t&):"
                << '\n'
                << "Tried to access position beyond end of vector: " << par_vec.size() << "/" << pos
                << '\n'
            );
        }
#endif

        (*it) = par_vec[pos];
        pos++;
    }
}

/******************************************************************************/
/**
 * Assigns part of a value map to the parameter
 */
void GDoubleCollection::assignDoubleValueVectors(
    const std::map<std::string, std::vector<double>> &par_map,
    const activityMode & /*am*/
) {
    GDoubleCollection::iterator it;
    std::size_t cnt = 0;
    for(it = this->begin(); it != this->end(); ++it) {
        *it = (Gem::Common::getMapItem(par_map, this->getParameterName())).at(cnt++);
    }
}

/******************************************************************************/
/**
 * Multiplication with a random value in a given range
 */
void GDoubleCollection::doubleMultiplyByRandom(
    const double &min,
    const double &max,
    const activityMode & /*am*/
    ,
    Gem::Hap::GRandomBase &gr // NOLINT(misc-unused-parameters)
) {
    std::uniform_real_distribution<double> uniform_real_distribution(min, max);
    for(std::size_t pos = 0; pos < this->size(); pos++) {
        GParameterCollectionT<double>::setValue(
            pos,
            this->value(pos) * uniform_real_distribution(gr)
        );
    }
}

/******************************************************************************/
/**
 * Multiplication with a random value in the range [0,1[
 */
void GDoubleCollection::doubleMultiplyByRandom(
    const activityMode & /*am*/
    ,
    Gem::Hap::GRandomBase &gr // NOLINT(misc-unused-parameters)
) {
    std::uniform_real_distribution<double> uniform_real_distribution(0., 1.);
    for(std::size_t pos = 0; pos < this->size(); pos++) {
        GParameterCollectionT<double>::setValue(
            pos,
            this->value(pos) * uniform_real_distribution(gr)
        );
    }
}

/******************************************************************************/
/**
 * Multiplication with a constant value
 */
void GDoubleCollection::doubleMultiplyBy(
    const double &val // NOLINT(misc-unused-parameters)
    ,
    const activityMode & /*am*/
) {
    for(std::size_t pos = 0; pos < this->size(); pos++) {
        GParameterCollectionT<double>::setValue(pos, val * this->value(pos));
    }
}

/******************************************************************************/
/**
 * Initialization with a constant value
 */
void GDoubleCollection::doubleFixedValueInit(
    const double &val,
    const activityMode & /*am*/ // NOLINT(misc-unused-parameters)
) {
    for(std::size_t pos = 0; pos < this->size(); pos++) {
        GParameterCollectionT<double>::setValue(pos, val);
    }
}

/******************************************************************************/
/**
 * Adds the "same-type" parameters of another GParameterBase object to this one
 */
void GDoubleCollection::doubleAdd(
    std::shared_ptr<GParameterBase> p_base,
    const activityMode & /*am*/
) {
    // We first need to convert p_base into the local type
    std::shared_ptr<GDoubleCollection> p =
        GParameterBase::parameterbase_cast<GDoubleCollection>(p_base);

    // Cross-check that the sizes match
    if(this->size() != p->size()) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In GDoubleCollection::doubleAdd():" << '\n'
            << "Sizes of vectors don't match: " << this->size() << "/" << p->size() << '\n'
        );
    }

    for(std::size_t pos = 0; pos < this->size(); pos++) {
        GParameterCollectionT<double>::setValue(pos, this->value(pos) + p->value(pos));
    }
}

/******************************************************************************/
/**
 * Adds the "same-type" parameters of another GParameterBase object to this one
 */
void GDoubleCollection::doubleSubtract(
    std::shared_ptr<GParameterBase> p_base,
    const activityMode & /*am*/
) {
    // We first need to convert p_base into the local type
    std::shared_ptr<GDoubleCollection> p =
        GParameterBase::parameterbase_cast<GDoubleCollection>(p_base);

    // Cross-check that the sizes match
    if(this->size() != p->size()) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In GDoubleCollection::doubleSubtract():" << '\n'
            << "Sizes of vectors don't match: " << this->size() << "/" << p->size() << '\n'
        );
    }

    for(std::size_t pos = 0; pos < this->size(); pos++) {
        GParameterCollectionT<double>::setValue(pos, this->value(pos) - p->value(pos));
    }
}

/******************************************************************************/
/**
 * Loads the data of another GObject
 *
 * @param cp A copy of another GDoubleCollection object, camouflaged as a GObject
 */
void GDoubleCollection::load_(const GObject *cp) {
    // Convert the pointer to our target type and check for self-assignment
    const GDoubleCollection *p_load =
        Gem::Common::g_convert_and_compare<GObject, GDoubleCollection>(cp, this);

    // Load our parent class'es data ...
    GFPNumCollectionT<double>::load_(cp);

    // ... no local data
}

/******************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GDoubleCollection::modify_GUnitTests_() {
#ifdef GEM_TESTING
    bool result = false;

    // Call the parent class'es function
    if(GFPNumCollectionT<double>::modify_GUnitTests_()) {
        result = true;
    }

    this->fillWithData_(10);
    result = true;

    return result;

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset("GDoubleCollection::modify_GUnitTests", "GEM_TESTING");
    return false;
#endif                  /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Fills the collection with some random data
 */
void GDoubleCollection::fillWithData_(const std::size_t &n_items) {
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

    std::uniform_real_distribution<double> uniform_real_distribution(-10., 10.);
    for(std::size_t i = 1; i < n_items - 1; i++) {
        CHECK_NOTHROW(this->push_back(uniform_real_distribution(gr)));
    }

    // Add a single item of defined value, so we can test the find() and count() functions
    CHECK_NOTHROW(this->push_back(1.));

    // Cross-check the size
    CHECK(this->size() == n_items);
    CHECK(not this->empty());

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset("GDoubleCollection::fillWithData", "GEM_TESTING");
#endif                  /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GDoubleCollection::specificTestsNoFailureExpected_GUnitTests_() {
#ifdef GEM_TESTING
    // A few settings
    const std::size_t n_items = 10000;
    const std::size_t n_tests = 10;
    const double fixedvalueinit = 1.;

    // Make sure we have an appropriate adaptor loaded when performing these tests
    bool adaptor_stored = false;
    std::shared_ptr<GAdaptorT<double>> stored_adaptor;

    if(this->hasAdaptor()) {
        stored_adaptor = this->getAdaptor();
        adaptor_stored = true;
    }

    std::shared_ptr<GDoubleGaussAdaptor> gdga_ptr(new GDoubleGaussAdaptor(0.025, 0.1, 0., 1., 1.0));
    gdga_ptr->setAdaptionThreshold(
        0
    ); // Make sure the adaptor's internal parameters don't change through the adaption
    gdga_ptr->setAdaptionMode(adaptionMode::ALWAYS); // Always adapt
    this->addAdaptor(gdga_ptr);

    // Call the parent class'es function
    GFPNumCollectionT<double>::specificTestsNoFailureExpected_GUnitTests_();

    // Get a random number generator
    Gem::Hap::GRandomT<Gem::Hap::RANDFLAVOURS::RANDOMPROXY> gr;

    //------------------------------------------------------------------------------

    { // Test the GParameterT<T>::adapt() implementation
        std::shared_ptr<GDoubleCollection> p_test1 = this->clone<GDoubleCollection>();
        std::shared_ptr<GDoubleCollection> p_test2 = this->clone<GDoubleCollection>();

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

    { // Test of GParameterCollectionT<T>::swap(const GParameterCollectionT<T>&)
        std::shared_ptr<GDoubleCollection> p_test1 = this->clone<GDoubleCollection>();
        std::shared_ptr<GDoubleCollection> p_test2 = this->clone<GDoubleCollection>();
        std::shared_ptr<GDoubleCollection> p_test3 = this->clone<GDoubleCollection>();

        if(p_test1->hasAdaptor()) {
            // Make sure the collection is clean
            p_test1->clear();

            // Add a few items
            for(std::size_t i = 0; i < n_items; i++) {
                p_test1->push_back(fixedvalueinit);
            }

            // Load p_test1 into p_test2 and p_test3
            CHECK_NOTHROW(p_test2->load(p_test1));
            CHECK_NOTHROW(p_test3->load(p_test1));

            // Make sure the objects match
            CHECK(*p_test1 == *p_test2);
            CHECK(*p_test1 == *p_test3);
            CHECK(*p_test3 == *p_test2);

            // Adapt p_test1
            CHECK_NOTHROW(p_test1->adapt(gr));

            // Test whether p_test1 and p_test2/3 differ now
            CHECK(*p_test1 != *p_test2);
            CHECK(*p_test1 != *p_test3);
            // Test whether p_test2 is still the same as p_test3
            CHECK(*p_test3 == *p_test2);

            // Swap the data of p_test2 and p_test1
            CHECK_NOTHROW(p_test2->swap(*p_test1));

            // Extract the data vectors from p_test1 and p_test3
            std::vector<double> data1;
            std::vector<double> data3;
            CHECK_NOTHROW(p_test1->Gem::Common::GPodContainerT<double>::getDataCopy(data1));
            CHECK_NOTHROW(p_test3->Gem::Common::GPodContainerT<double>::getDataCopy(data3));

            // Now p_test1->data  and p_test3->data should be the same, while p_test2 differs from both
            CHECK(data1 == data3);
            CHECK(*p_test2 != *p_test1);
            CHECK(*p_test2 != *p_test3);
        }
    }

    //------------------------------------------------------------------------------

    { // Test the GPODVectorT<double>::reserve(), capacity() and max_size() functions
        std::shared_ptr<GDoubleCollection> p_test1 = this->clone<GDoubleCollection>();

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

    { // Test the GPODVectorT<double>::count(), find() and begin() functions
        std::shared_ptr<GDoubleCollection> p_test1 = this->clone<GDoubleCollection>();

        // Add some data
        CHECK_NOTHROW(p_test1->fillWithData_(n_items));

        // Count the number of values == 0. . Should be >= 1
        CHECK(p_test1->count(0.) >= 1);
        // Count the number of values == 1. . Should be >= 1
        CHECK(p_test1->count(1.) >= 1);

        // Find the item with value 0. -- the first one is in position 0
        GDoubleCollection::const_iterator find_it;
        GDoubleCollection::const_iterator pos_it;
        CHECK_NOTHROW(pos_it = p_test1->begin());
        CHECK_NOTHROW(find_it = p_test1->find(0.));
        CHECK(find_it == pos_it);
    }

    //------------------------------------------------------------------------------

    { // Test setting and retrieval of items with the operator[] and at() functions of GPODVectorT<double>
        std::shared_ptr<GDoubleCollection> p_test1 = this->clone<GDoubleCollection>();

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

    { // Test the GPODVectorT<double>::front() and back() functions
        std::shared_ptr<GDoubleCollection> p_test1 = this->clone<GDoubleCollection>();

        // Add some data
        CHECK_NOTHROW(p_test1->fillWithData_(n_items));

        // Check the front and back of the vector -- we know the values
        CHECK(p_test1->front() == 0.);
        CHECK(p_test1->back() == 1.);
    }

    //------------------------------------------------------------------------------

    { // Test iteration over the vector and retrieval of the end() iterator (Test of GPODVectorT<double> functionality)
        std::shared_ptr<GDoubleCollection> p_test1 = this->clone<GDoubleCollection>();

        // Add some data
        CHECK_NOTHROW(p_test1->fillWithData_(n_items));

        // Iterate over the sequence
        GDoubleCollection::iterator it;
        std::size_t item_count = 0;
        for(it = p_test1->begin(); it != p_test1->end(); ++it) {
            item_count++;
        }
        CHECK(item_count == n_items);
    }

    //------------------------------------------------------------------------------

    { // Test inserting and erasure of items, the pop_and_block and resize functions and the getDataCopy and operator= functions (Test of GPODVectorT<double> functionality)
        std::shared_ptr<GDoubleCollection> p_test1 = this->clone<GDoubleCollection>();

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

        std::vector<double> data_copy;
        CHECK_NOTHROW(p_test1->getDataCopy(data_copy));
        CHECK(data_copy.size() == n_items);
        CHECK(static_cast<std::size_t>(std::count(data_copy.begin(), data_copy.end(), 1.)) == n_items);

        // Assign 1 to all positions and add further items
        for(std::size_t i = 0; i < data_copy.size(); i++) {
            data_copy[i] = 0.;
        }
        for(std::size_t i = 0; i < n_items; i++) {
            data_copy.push_back(0.);
        }

        // Assign the vector to p_test1 and cross-check
        CHECK_NOTHROW(p_test1->Gem::Common::GPodContainerT<double>::operator=(data_copy));
        CHECK(p_test1->size() == 2 * n_items);
        CHECK(p_test1->count(0.) == 2 * n_items);
    }

    //------------------------------------------------------------------------------

    // Restore the object to its pristine condition
    this->resetAdaptor();

    // Load the old adaptor, if needed
    if(adaptor_stored) {
        this->addAdaptor(stored_adaptor);
    }

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset(
        "GDoubleCollection::specificTestsNoFailureExpected_GUnitTests",
        "GEM_TESTING"
    );
#endif                  /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GDoubleCollection::specificTestsFailuresExpected_GUnitTests_() {
#ifdef GEM_TESTING
    // Make sure we have an appropriate adaptor loaded when performing these tests
    bool adaptor_stored = false;
    std::shared_ptr<GAdaptorT<double>> stored_adaptor;

    if(this->hasAdaptor()) {
        stored_adaptor = this->getAdaptor();
        adaptor_stored = true;
    }

    std::shared_ptr<GDoubleGaussAdaptor> gdga_ptr(new GDoubleGaussAdaptor(0.025, 0.1, 0., 1., 1.0));
    gdga_ptr->setAdaptionThreshold(
        0
    ); // Make sure the adaptor's internal parameters don't change through the adaption
    gdga_ptr->setAdaptionMode(adaptionMode::ALWAYS); // Always adapt
    this->addAdaptor(gdga_ptr);

    // Call the parent class'es function
    GFPNumCollectionT<double>::specificTestsFailuresExpected_GUnitTests_();

    // Nothing to check -- no local data

    // Remove the test adaptor
    this->resetAdaptor();

    // Restore the adaptor to its pristine condition
    if(adaptor_stored) {
        this->addAdaptor(stored_adaptor);
    }

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset(
        "GDoubleCollection::specificTestsFailuresExpected_GUnitTests",
        "GEM_TESTING"
    );
#endif                  /* GEM_TESTING */
}

/******************************************************************************/

} /* namespace Gem::Geneva::Parameters */
