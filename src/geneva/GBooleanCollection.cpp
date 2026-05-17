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

#include "geneva/par/GBooleanCollection.hpp"

#ifdef GEM_TESTING
#include <catch2/catch_test_macros.hpp>
#endif /* GEM_TESTING */

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::Parameters::GBooleanCollection) // NOLINT
namespace Gem::Geneva::Parameters {

/******************************************************************************/
/**
 * Initializes the class with a set of nval random bits.
 *
 * @param nval The size of the collection
 */
GBooleanCollection::GBooleanCollection(const std::size_t &nval) {
    Gem::Hap::GRandomT<Gem::Hap::RANDFLAVOURS::RANDOMLOCAL> gr;
    std::bernoulli_distribution bernoulli_distribution; // defaults to 0.5
    for(std::size_t i = 0; i < nval; i++) {
        this->push_back(bernoulli_distribution(gr));
    }
}

// Tested in this class

/******************************************************************************/
/**
 * Initializes the class with a set of nval variables of identical value
 *
 * @param nval The size of the collection
 * @param val  The value to be assigned to each position
 */
GBooleanCollection::GBooleanCollection(const std::size_t &nval, const bool &val)
  : GParameterCollectionT<bool>(nval, val) { /* nothing */
}

// Tested in this class

/******************************************************************************/
/**
 * Initializes the class with nval random bits, of which probability percent
 * have the value true. E.g., a probability value of 0.7 results in approimately
 * 70% "true" values.
 *
 * @param nval The size of the collection
 * @param probability The probability for true values in the collection
 */
GBooleanCollection::GBooleanCollection(const std::size_t &nval, const double &probability) {
    Gem::Hap::GRandomT<Gem::Hap::RANDFLAVOURS::RANDOMLOCAL> gr;
    std::bernoulli_distribution weighted_bool(probability);
    for(std::size_t i = 0; i < nval; i++) {
        this->push_back(weighted_bool(gr));
    }
}

// Tested in this class

/******************************************************************************/
/**
 * FLips the value at a given position
 */
void GBooleanCollection::flip(const std::size_t &pos) {
#ifdef DEBUG
    if(this->size() <= pos) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In GBooleanCollection::flip(const std::size_t& " << pos << "): Error!" << '\n'
            << "Tried to exist position beyond end of vector of size " << this->size() << '\n'
        );
    }
#endif

    if(true == this->at(pos)) {
        this->at(pos) = false;
    }
    else {
        this->at(pos) = true;
    }
}

/******************************************************************************/
/**
 * Creates a deep clone of this object
 *
 * @return A deep clone of this object
 */
GObject *GBooleanCollection::clone_() const {
    return new GBooleanCollection(*this);
}

/******************************************************************************/
/**
 * Loads the data of another GBooleanCollection object, camouflaged as
 * a GObject.
 *
 * @param gb A pointer to another GBooleanCollection object, camouflaged as a GObject
 */
void GBooleanCollection::load_(const GObject *cp) {
    // Convert the pointer to our target type and check for self-assignment
    const GBooleanCollection *p_load =
        Gem::Common::g_convert_and_compare<GObject, GBooleanCollection>(cp, this);

    GParameterCollectionT<bool>::load_(cp);
}

/******************************************************************************/
/**
 * Triggers random initialization of the parameter collection. Note that this
 * function assumes that the collection has been completely set up. Data
 * that is added later will remain unaffected.
 */
bool GBooleanCollection::randomInit_(
    const activityMode & /*am*/
    ,
    Gem::Hap::GRandomBase &gr
) {
    bool randomized = false;

    std::bernoulli_distribution bernoulli_distribution; // defaults to 0.5

    // Compare http://stackoverflow.com/questions/15927033/what-is-the-correct-way-of-using-c11s-range-based-for
    for(auto &&b : this->data_cnt_) {
        b = bernoulli_distribution(gr);
        randomized = true;
    }

    return randomized;
}

/******************************************************************************/
/**
 * Random initialization with a given probability structure. E.g., a probability
 * value of 0.7 results in approimately 70% "true" values.
 *
 * @param probability The probability for true values in the collection
 */
bool GBooleanCollection::randomInit_(
    const double &probability,
    const activityMode &,
    Gem::Hap::GRandomBase &gr
) {
    bool randomized = false;

    // Do some error checks
    if(not Gem::Common::checkRangeCompliance(
           probability,
           0.,
           1.,
           "GBooleanCollection::randomInit_(probability)"
       )) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In GBooleanCollection::randomInit_(probability): Error!" << '\n'
            << "Probability " << probability << " not in allowed value range [0,1]" << '\n'
        );
    }

    // Obtain access to a random number generator
    std::bernoulli_distribution bernoulli_distribution(probability);

    // Compare http://stackoverflow.com/questions/15927033/what-is-the-correct-way-of-using-c11s-range-based-for
    for(auto &&b : this->data_cnt_) {
        b = bernoulli_distribution(gr);
        randomized = true;
    }

    return randomized;
}

/******************************************************************************/
/**
 * Random initialization. This is a helper function, without it we'd
 * have to say things like "myGBooleanCollectionObject.GParameterBase::randomInit();".
 */
bool GBooleanCollection::randomInit(const activityMode &am, Gem::Hap::GRandomBase &gr) {
    return GParameterBase::randomInit(
        am,
        gr
    ); // This will also take into account the "blocked initialization" flag
}

/******************************************************************************/
/**
 * Random initialization with a given probability structure,
 * if re-initialization has not been blocked.
 *
 * @param probability The probability for true values in the collection
 */
bool GBooleanCollection::randomInit(
    const double &probability,
    const activityMode &am,
    Gem::Hap::GRandomBase &gr
) {
    if(not GParameterBase::randomInitializationBlocked() && this->modifiableAmMatchOrHandover(am)) {
        return randomInit_(probability, am, gr);
    }
    else {
        return false;
    }
}

/***************************************************************************/
/**
 * Returns a "comparative range". In the case of boolean values this must
 * be considered to be more of a "dummy".
 */
bool GBooleanCollection::range() const {
    return true;
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
void GBooleanCollection::compare_(
    const GObject &cp,
    const Gem::Common::expectation &e,
    const double & /*limit*/
) const {
    using namespace Gem::Common;

    // Check that we are dealing with a GBooleanCollection reference independent of this object and convert the pointer
    const GBooleanCollection *p_load =
        Gem::Common::g_convert_and_compare<GObject, GBooleanCollection>(cp, this);

    GToken token("GBooleanCollection", e);

    // Compare our parent data ...
    Gem::Common::compare_base_t<GParameterCollectionT<bool>>(*this, *p_load, token);

    // ... no local data

    // React on deviations from the expectation
    token.evaluate();
}

/***********************************************************************************/
/**
 * Emits a name for this class / object
 */
std::string GBooleanCollection::name_() const {
    return std::string("GBooleanCollection");
}

/******************************************************************************/
/**
 * Attach our local values to the vector. This is used to collect all parameters of this type
 * in the sequence in which they were registered.
 *
 * @param par_vec The vector to which the local values should be attached
 */
void GBooleanCollection::booleanStreamline(std::vector<bool> &par_vec, const activityMode &) const {
    GBooleanCollection::const_iterator cit;
    for(cit = this->begin(); cit != this->end(); ++cit) {
        par_vec.push_back(*cit);
    }
}

/******************************************************************************/
/**
 * Attach our local values to the map. Names are built from the object name and the
 * position in the array.
 *
 * @param par_vec The map to which the local values should be attached
 */
void GBooleanCollection::booleanStreamline(
    std::map<std::string, std::vector<bool>> &par_vec,
    const activityMode &am
) const {
#ifdef DEBUG
    if((this->getParameterName()).empty()) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In GBooleanCollection::booleanStreamline(std::map<std::string, std::vector<bool>>& "
               "par_vec) const: Error!"
            << '\n'
            << "No name was assigned to the object" << '\n'
        );
    }
#endif /* DEBUG */

    std::vector<bool> parameters;
    this->booleanStreamline(parameters, am);
    par_vec[this->getParameterName()] = parameters;
}

/******************************************************************************/
/**
 * Attach boundaries of type bool to the vectors
 *
 * @param l_bnd_vec A vector of lower bool parameter boundaries
 * @param u_bnd_vec A vector of upper bool parameter boundaries
 */
void GBooleanCollection::booleanBoundaries(
    std::vector<bool> &l_bnd_vec,
    std::vector<bool> &u_bnd_vec,
    const activityMode & /*am*/
) const {
    GBooleanCollection::const_iterator cit;
    for(cit = this->begin(); cit != this->end(); ++cit) {
        l_bnd_vec.push_back(false);
        u_bnd_vec.push_back(true);
    }
}

/******************************************************************************/
/**
 * Tell the audience that we own a number of bool values
 *
 * @param am An enum indicating whether only information about active, inactive or all parameters of this type should be extracted
 * @return The number of bool parameters
 */
std::size_t GBooleanCollection::countBoolParameters(
    const activityMode & /*am*/
) const {
    return this->size();
}

/******************************************************************************/
/**
 * Assigns part of a value vector to the parameter
 */
void GBooleanCollection::assignBooleanValueVector(
    const std::vector<bool> &par_vec,
    std::size_t &pos,
    const activityMode & /*am*/
) {
    for(GBooleanCollection::iterator it = this->begin(); it != this->end(); ++it) {
#ifdef DEBUG
        // Do we have a valid position ?
        if(pos >= par_vec.size()) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, time_and_place)
                << "In GBooleanCollection::assignBooleanValueVector(const std::vector<bool>&, "
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
void GBooleanCollection::assignBooleanValueVectors(
    const std::map<std::string, std::vector<bool>> &par_map,
    const activityMode & /*am*/
) {
    GBooleanCollection::iterator it;
    std::size_t cnt = 0;
    for(it = this->begin(); it != this->end(); ++it) {
        *it = (Gem::Common::getMapItem<std::vector<bool>>(par_map, this->getParameterName()))
                  .at(cnt++);
    }
}

/******************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GBooleanCollection::modify_GUnitTests_() {
#ifdef GEM_TESTING
    bool result = false;

    // Call the parent class'es function
    if(GParameterCollectionT<bool>::modify_GUnitTests_()) {
        result = true;
    }

    this->push_back(true);
    return true;

    return result;

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset("GBooleanCollection::modify_GUnitTests", "GEM_TESTING");
    return false;
#endif                  /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GBooleanCollection::specificTestsNoFailureExpected_GUnitTests_() {
#ifdef GEM_TESTING
    // A few settings
    const std::size_t n_items = 10000;
    const bool fixedvalueinit = true;
    const double lowerbnd = 0.8, upperbnd = 1.2;

    // Make sure we have an appropriate adaptor loaded when performing these tests
    bool adaptor_stored = false;
    std::shared_ptr<GAdaptorT<bool>> stored_adaptor;

    if(this->hasAdaptor()) {
        stored_adaptor = this->getAdaptor();
        adaptor_stored = true;
    }

    std::shared_ptr<GBooleanAdaptor> gba_ptr(new GBooleanAdaptor(1.0));
    gba_ptr->setAdaptionThreshold(
        0
    ); // Make sure the adaptor's internal parameters don't change through the adaption
    gba_ptr->setAdaptionMode(adaptionMode::ALWAYS); // Always adapt
    this->addAdaptor(gba_ptr);

    // Call the parent class'es function
    GParameterCollectionT<bool>::specificTestsNoFailureExpected_GUnitTests_();

    // A random generator
    Gem::Hap::GRandomT<Gem::Hap::RANDFLAVOURS::RANDOMPROXY> gr;

    // --------------------------------------------------------------------------

    { // Check default constructor
        GBooleanCollection gbc;
        CHECK(gbc.empty());
    }

    // --------------------------------------------------------------------------

    { // Check copy construction
        GBooleanCollection gbc1;
        CHECK_NOTHROW(gbc1.push_back(true));
        GBooleanCollection gbc2(gbc1);
        INFO("\n" << "gbc2.size() = " << gbc2.size() << "gbc2.at(0) = " << gbc2.at(0));
        CHECK((gbc2.size() == 1 && gbc2.at(0) == true));
    }

    // --------------------------------------------------------------------------

    { // Check construction with a number of random bits
        GBooleanCollection gbc(n_items);

        INFO(
            "\n"
            << "gbc.size() = " << gbc.size() << "\n"
            << "nItems = " << n_items
        );
        CHECK(gbc.size() == n_items);

        // Count the number of true and false values
        std::size_t n_true = 0;
        std::size_t n_false = 0;
        for(std::size_t i = 0; i < n_items; i++) {
            gbc.at(i) ? n_true++ : n_false++;
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

    { // Check construction with a number of identical bits
        GBooleanCollection gbc(n_items, true);

        INFO("\n" << "gbc.size() = " << gbc.size() << "nItems = " << n_items);
        CHECK(gbc.size() == n_items);

        // Count the number of true and false values
        std::size_t n_true = 0;
        std::size_t n_false = 0;
        for(std::size_t i = 0; i < n_items; i++) {
            gbc.at(i) ? n_true++ : n_false++;
        }

        INFO(
            "\n"
            << "n_true = " << n_true << "\n"
            << "nItems = " << n_items << "\n"
        );
        CHECK(n_true == n_items);
    }

    // --------------------------------------------------------------------------

    { // Check construction with a given probability for the value true
        GBooleanCollection gbc(n_items, 0.5);

        INFO("\n" << "gbc.size() = " << gbc.size() << "nItems = " << n_items);
        CHECK(gbc.size() == n_items);

        // Count the number of true and false values
        std::size_t n_true = 0;
        std::size_t n_false = 0;
        for(std::size_t i = 0; i < n_items; i++) {
            gbc.at(i) ? n_true++ : n_false++;
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

    { // Test that random initialization with equal probability will result in roughly the same amount of true and false values
        std::shared_ptr<GBooleanCollection> p_test = this->clone<GBooleanCollection>();

        // Make sure the collection is empty
        CHECK_NOTHROW(p_test->clear());

        // Add items of fixed value
        for(std::size_t i = 0; i < n_items; i++) {
            p_test->push_back(true);
        }

        // Check the size
        CHECK(p_test->size() == n_items);

        // Randomly initialize, using the internal function
        CHECK_NOTHROW(p_test->randomInit_(activityMode::ALLPARAMETERS, gr));

        // Count the number of true and false values
        std::size_t n_true = 0;
        std::size_t n_false = 0;
        for(std::size_t i = 0; i < n_items; i++) {
            p_test->at(i) ? n_true++ : n_false++;
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

    { // Check that initialization with a probabilty of 0. for true results in just false values
        std::shared_ptr<GBooleanCollection> p_test = this->clone<GBooleanCollection>();

        // Make sure the collection is empty
        CHECK_NOTHROW(p_test->clear());

        // Add items of fixed value
        for(std::size_t i = 0; i < n_items; i++) {
            p_test->push_back(true);
        }

        // Randomly initialize, using the internal function
        CHECK_NOTHROW(p_test->randomInit_(0., activityMode::ALLPARAMETERS, gr));

        // Count the number of true and false values
        std::size_t n_true = 0;
        std::size_t n_false = 0;
        for(std::size_t i = 0; i < n_items; i++) {
            p_test->at(i) ? n_true++ : n_false++;
        }

        // Cross-check
        INFO(
            "\n"
            << "n_true = " << n_true << "\n"
            << "n_false = " << n_false << "\n"
        );
        CHECK(n_true == 0);
    }

    // --------------------------------------------------------------------------

    { // Check that initialization with a probabilty of 1. for true results in just true values
        std::shared_ptr<GBooleanCollection> p_test = this->clone<GBooleanCollection>();

        // Make sure the collection is empty
        CHECK_NOTHROW(p_test->clear());

        // Add items of fixed value
        for(std::size_t i = 0; i < n_items; i++) {
            p_test->push_back(false);
        }

        // Randomly initialize, using the internal function
        CHECK_NOTHROW(p_test->randomInit_(1., activityMode::ALLPARAMETERS, gr));

        // Count the number of true and false values
        std::size_t n_true = 0;
        std::size_t n_false = 0;
        for(std::size_t i = 0; i < n_items; i++) {
            p_test->at(i) ? n_true++ : n_false++;
        }

        // Cross-check
        INFO(
            "\n"
            << "n_true = " << n_true << "\n"
            << "n_false = " << n_false << "\n"
        );
        CHECK(n_true == n_items);
    }

    // --------------------------------------------------------------------------

    { // Test that random initialization with a given probability will result in roughly the expected amount of true and false values
        for(int prob_i = 1; prob_i <= 8; ++prob_i) {
            const double d = prob_i * 0.1;
            std::shared_ptr<GBooleanCollection> p_test = this->clone<GBooleanCollection>();

            // Make sure the collection is empty
            CHECK_NOTHROW(p_test->clear());

            // Add items of fixed value
            for(std::size_t i = 0; i < n_items; i++) {
                p_test->push_back(false);
            }

            // Randomly initialize, using the internal function and the required probability
            CHECK_NOTHROW(p_test->randomInit_(d, activityMode::ALLPARAMETERS, gr));

            // Count the number of true and false values
            std::size_t n_true = 0;
            std::size_t n_false = 0;
            for(std::size_t i = 0; i < n_items; i++) {
                p_test->at(i) ? n_true++ : n_false++;
            }

            // We allow a slight deviation, as the initialization is a random process
            double expected_true_min = 0.8 * d * n_items;
            double expected_true_max = 1.2 * d * n_items;

            INFO(
                "\n"
                << "d = " << d << "\n"
                << "Allowed window = " << expected_true_min << " - " << expected_true_max << "\n"
                << "nItems = " << n_items << "\n"
                << "n_true = " << n_true << "\n"
                << "n_false = " << n_false << "\n"
            );
            CHECK((double(n_true) > expected_true_min && double(n_true) < expected_true_max));
        }
    }

    // --------------------------------------------------------------------------

    { // Check that random initialization can be blocked for equal distributions
        std::shared_ptr<GBooleanCollection> p_test1 = this->clone<GBooleanCollection>();
        std::shared_ptr<GBooleanCollection> p_test2 = this->clone<GBooleanCollection>();

        // Make sure the collections are empty
        CHECK_NOTHROW(p_test1->clear());
        CHECK_NOTHROW(p_test2->clear());

        // Add items of fixed value
        for(std::size_t i = 0; i < n_items; i++) {
            p_test1->push_back(false);
        }

        // Block random initialization and cross check
        CHECK_NOTHROW(p_test1->blockRandomInitialization());
        CHECK(p_test1->randomInitializationBlocked() == true);

        // Load the data into p_test2
        CHECK_NOTHROW(p_test2->load(p_test1));

        // Check that both objects are equal
        CHECK(*p_test1 == *p_test2);

        // Check that random initialization is also blocked for p_test2
        CHECK(p_test2->randomInitializationBlocked() == true);

        // Try to randomly initialize, using the *external* function
        CHECK_NOTHROW(p_test1->randomInit(activityMode::ALLPARAMETERS, gr));

        // Check that both objects are still the same
        CHECK(*p_test1 == *p_test2);
    }

    // --------------------------------------------------------------------------

    { // Check that random initialization can be blocked for distributions with a given probability structure
        std::shared_ptr<GBooleanCollection> p_test1 = this->clone<GBooleanCollection>();
        std::shared_ptr<GBooleanCollection> p_test2 = this->clone<GBooleanCollection>();

        // Make sure the collections are empty
        CHECK_NOTHROW(p_test1->clear());
        CHECK_NOTHROW(p_test2->clear());

        // Add items of fixed value
        for(std::size_t i = 0; i < n_items; i++) {
            p_test1->push_back(false);
        }

        // Block random initialization and cross check
        CHECK_NOTHROW(p_test1->blockRandomInitialization());
        CHECK(p_test1->randomInitializationBlocked() == true);

        // Load the data into p_test2
        CHECK_NOTHROW(p_test2->load(p_test1));

        // Check that both objects are equal
        CHECK(*p_test1 == *p_test2);

        // Check that random initialization is also blocked for p_test2
        CHECK(p_test2->randomInitializationBlocked() == true);

        // Try to randomly initialize, using the *external* function
        CHECK_NOTHROW(p_test1->randomInit(0.7, activityMode::ALLPARAMETERS, gr));

        // Check that both objects are still the same
        CHECK(*p_test1 == *p_test2);
    }

    // --------------------------------------------------------------------------

    { // Check that the fp-family of functions doesn't have an effect on this object
        std::shared_ptr<GBooleanCollection> p_test1 = this->GObject::clone<GBooleanCollection>();
        std::shared_ptr<GBooleanCollection> p_test2 = this->GObject::clone<GBooleanCollection>();
        std::shared_ptr<GBooleanCollection> p_test3 = this->GObject::clone<GBooleanCollection>();

        // Add a few items to p_test1
        for(std::size_t i = 0; i < n_items; i++) {
            p_test1->push_back(fixedvalueinit);
        }

        // Load into p_test2 and p_test3 and test equality
        CHECK_NOTHROW(p_test2->load(p_test1));
        CHECK_NOTHROW(p_test3->load(p_test1));
        CHECK(*p_test2 == *p_test1);
        CHECK(*p_test3 == *p_test1);
        CHECK(*p_test3 == *p_test2);

        // Check that initialization with a fixed floating point value has no effect on this object
        CHECK_NOTHROW(p_test2->fixedValueInit<double>(2., activityMode::ALLPARAMETERS));
        CHECK(*p_test2 == *p_test1);

        // Check that multiplication with a fixed floating point value has no effect on this object
        CHECK_NOTHROW(p_test2->multiplyBy<double>(2., activityMode::ALLPARAMETERS));
        CHECK(*p_test2 == *p_test1);

        // Check that a component-wise multiplication with a random fp value in a given range does not have an effect on this object
        CHECK_NOTHROW(p_test2->multiplyByRandom<double>(1., 2., activityMode::ALLPARAMETERS, gr));
        CHECK(*p_test2 == *p_test1);

        // Check that a component-wise multiplication with a random fp value in the range [0:1[ does not have an effect on this object
        CHECK_NOTHROW(p_test2->multiplyByRandom<double>(activityMode::ALLPARAMETERS, gr));
        CHECK(*p_test2 == *p_test1);

        // Check that adding p_test1 to p_test3 does not have an effect
        CHECK_NOTHROW(p_test3->add<double>(p_test1, activityMode::ALLPARAMETERS));
        CHECK(*p_test3 == *p_test2);

        // Check that subtracting p_test1 from p_test3 does not have an effect
        CHECK_NOTHROW(p_test3->subtract<double>(p_test1, activityMode::ALLPARAMETERS));
        CHECK(*p_test3 == *p_test2);
    }

    // --------------------------------------------------------------------------

    // Remove the test adaptor
    this->resetAdaptor();

    // Load the old adaptor, if needed
    if(adaptor_stored) {
        this->addAdaptor(stored_adaptor);
    }

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset(
        "GBooleanCollection::specificTestsNoFailureExpected_GUnitTests",
        "GEM_TESTING"
    );
#endif                  /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GBooleanCollection::specificTestsFailuresExpected_GUnitTests_() {
#ifdef GEM_TESTING
    // A few settings
    std::size_t n_items = 10000;

    // Make sure we have an appropriate adaptor loaded when performing these tests
    bool adaptor_stored = false;
    std::shared_ptr<GAdaptorT<bool>> stored_adaptor;

    if(this->hasAdaptor()) {
        stored_adaptor = this->getAdaptor();
        adaptor_stored = true;
    }

    std::shared_ptr<GBooleanAdaptor> gba_ptr(new GBooleanAdaptor(1.0));
    gba_ptr->setAdaptionThreshold(
        0
    ); // Make sure the adaptor's internal parameters don't change through the adaption
    gba_ptr->setAdaptionMode(adaptionMode::ALWAYS); // Always adapt
    this->addAdaptor(gba_ptr);

    // Call the parent class'es function
    GParameterCollectionT<bool>::specificTestsFailuresExpected_GUnitTests_();

    // A random generator
    Gem::Hap::GRandomT<Gem::Hap::RANDFLAVOURS::RANDOMPROXY> gr;

    // --------------------------------------------------------------------------

    { // Check that random initialization with a probability < 0. throws
        std::shared_ptr<GBooleanCollection> p_test = this->clone<GBooleanCollection>();

        // Make sure the collection is empty
        CHECK_NOTHROW(p_test->clear());

        // Add items of fixed value
        for(std::size_t i = 0; i < n_items; i++) {
            p_test->push_back(true);
        }

        // Randomly initialize, using the internal function
        CHECK_THROWS_AS(
            (p_test->randomInit_(-1., activityMode::ALLPARAMETERS, gr)),
            geneva_exception
        );
    }

    // --------------------------------------------------------------------------

    { // Check that random initialization with a probability > 1. throws
        std::shared_ptr<GBooleanCollection> p_test = this->clone<GBooleanCollection>();

        // Make sure the collection is empty
        CHECK_NOTHROW(p_test->clear());

        // Add items of fixed value
        for(std::size_t i = 0; i < n_items; i++) {
            p_test->push_back(true);
        }

        // Randomly initialize, using the internal function
        CHECK_THROWS_AS(
            (p_test->randomInit_(2., activityMode::ALLPARAMETERS, gr)),
            geneva_exception
        );
    }

    // --------------------------------------------------------------------------

    // Remove the test adaptor
    this->resetAdaptor();

    // Load the old adaptor, if needed
    if(adaptor_stored) {
        this->addAdaptor(stored_adaptor);
    }

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset(
        "GBooleanCollection::specificTestsFailuresExpected_GUnitTests",
        "GEM_TESTING"
    );
#endif                  /* GEM_TESTING */
}

/******************************************************************************/

} /* namespace Gem::Geneva::Parameters */
