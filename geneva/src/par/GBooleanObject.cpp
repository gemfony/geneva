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

#include "geneva/par/GBooleanObject.hpp"
#include "common/GCommonHelperFunctionsT.hpp"
#include "common/GCommonMathHelperFunctionsT.hpp"
#include "common/GDefaultValueT.hpp"
#include "common/GExceptions.hpp"
#include "common/GExpectationChecksT.hpp"
#include "common/GLogger.hpp"
#include "geneva/GOptimizationEnums.hpp"
#include "geneva/par/GAdaptorT.hpp"
#include "geneva/par/GBooleanAdaptor.hpp"
#include "geneva/par/GParameterBase.hpp"
#include "geneva/par/GParameterT.hpp"
#include "hap/GHapEnums.hpp"
#include "hap/GRandomBase.hpp"
#include "hap/GRandomT.hpp"
#include <cstddef>
#include <map>
#include <memory>
#include <random>
#include <vector>

#ifdef GEM_TESTING
#include <catch2/catch_test_macros.hpp>
#endif /* GEM_TESTING */

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::Parameters::GBooleanObject) // NOLINT
namespace Gem::Geneva::Parameters {

/******************************************************************************/
/**
 * Initialization by contained value
 *
 * @param val A value used for the initialization
 */
GBooleanObject::GBooleanObject(const bool &val)
  : GParameterT<bool>(val) { /* nothing */
}

// Tested in this file

/******************************************************************************/
/**
 * Initialization with a given probability for "true". E.g., a probability value
 * of 0.7 results in approimately 70% "true" values.
 *
 * @param probability The probability for the value "true"
 */
GBooleanObject::GBooleanObject(const double &probability) {
    Gem::Hap::GRandomT<Gem::Hap::RANDFLAVOURS::RANDOMLOCAL> gr;
    std::bernoulli_distribution bernoulli_distribution(probability);

    this->setValue(bernoulli_distribution(gr));
}

// Tested in this file

/******************************************************************************/
/**
 * An assignment operator
 *
 * @param val The value to be assigned to this object
 * @return The value that was just assigned to this object
 */
GBooleanObject &GBooleanObject::operator=(const bool &val) {
    GParameterT<bool>::operator=(val);
    return *this;
}

/******************************************************************************/
/**
 * Creates a deep clone of this object.
 *
 * @return A copy of this object, camouflaged as a GParameterBase
 */
GParameterBase *GBooleanObject::clone_() const {
    return new GBooleanObject(*this);
}

/******************************************************************************/
/**
 * Flips the value of this object
 */
void GBooleanObject::flip() {
    this->setValue(not this->value());
}

/******************************************************************************/
/**
 * Random initialization. This is a helper function, without it we'd
 * have to say things like "myGBooleanObject.GParameterBase::randomInit();".
 */
bool GBooleanObject::randomInit(const activityMode &am, Gem::Hap::GRandomBase &gr) {
    return GParameterBase::randomInit(am, gr);
}

/* ----------------------------------------------------------------------------------
 * Tested in GInt32Collection::specificTestsNoFailuresExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Triggers random initialization of the parameter object, with a given likelihood structure
 */
bool GBooleanObject::randomInit(
    const double &probability,
    const activityMode &am,
    Gem::Hap::GRandomBase &gr
) {
    if(not GParameterBase::randomInitializationBlocked() && this->modifiableAmMatchOrHandover(am)) {
        return randomInit_(probability, am, gr);
    }
            return false;
   
}

/* ----------------------------------------------------------------------------------
 * Tested in GInt32Collection::specificTestsNoFailuresExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Triggers random initialization of the parameter object
 */
bool GBooleanObject::randomInit_(
    [[maybe_unused]] const activityMode & am
    ,
    Gem::Hap::GRandomBase &gr
) {
    std::bernoulli_distribution bernoulli_distribution; // defaults to 0.5
    this->setValue(bernoulli_distribution(gr));
    return true;
}

/* ----------------------------------------------------------------------------------
 * Tested in GInt32Collection::specificTestsNoFailuresExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Triggers random initialization of the parameter object, with a given likelihood structure.
 * This function holds the actual initialization logic, used in the publicly accessible
 * GBooleanObject::randomInit(const double& probability) function. A probability value of 0.7
 * results in approimately 70% "true" values.
 */
bool GBooleanObject::randomInit_(
    const double &probability,
    [[maybe_unused]] const activityMode & am
    ,
    Gem::Hap::GRandomBase &gr
) {
    // Do some error checks
    if(not Gem::Common::checkRangeCompliance(
           probability,
           0.,
           1.,
           "GBooleanObject::randomInit_(probability)"
       )) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GBooleanObject::randomInit_(probability): Error!" << '\n'
            << "Probability " << probability << " not in allowed value range [0,1]" << '\n'
        );
    }

    std::bernoulli_distribution bernoulli_distribution(probability);
    this->setValue(bernoulli_distribution(gr));
    return true;
}

/* ----------------------------------------------------------------------------------
 * Tested in GInt32Collection::specificTestsNoFailuresExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/***************************************************************************/
/**
 * Returns a "comparative range". In the case of boolean values this must
 * be considered to be more of a "dummy".
 */
bool GBooleanObject::range() const {
    return true;
}

/******************************************************************************/
/**
 * Searches for compliance with expectations with respect to another object
 * of the same type
 *
 * @param cp A constant reference to another GParameterBase object
 * @param e The expected outcome of the comparison
 */
void GBooleanObject::compare_(
    const GParameterBase &cp,
    const Gem::Common::expectation &e,
    [[maybe_unused]] const double & limit
) const {
    using namespace Gem::Common;

    // Check that we are dealing with a GBooleanObject reference independent of this object and convert the pointer
    const GBooleanObject *p_load =
        Gem::Common::g_convert_and_compare<GParameterBase, GBooleanObject>(cp, this);

    GToken token("GBooleanObject", e);

    // Compare our parent data ...
    Gem::Common::compare_base_t<GParameterT<bool>>(*this, *p_load, token);

    // ... no local data

    // React on deviations from the expectation
    token.evaluate();
}

/***********************************************************************************/
/**
 * Emits a name for this class / object
 */
std::string GBooleanObject::name_() const {
    return std::string("GBooleanObject");
}

/******************************************************************************/
/**
 * Attach our local value to the vector. This is used to collect all parameters of this type
 * in the sequence in which they were registered.
 */
void GBooleanObject::booleanStreamline(
    std::vector<bool> &par_vec,
    [[maybe_unused]] const activityMode & am
) const {
    par_vec.push_back(this->value());
}

/******************************************************************************/
/**
 * Attach our local value to the map.
 */
void GBooleanObject::booleanStreamline(
    std::map<std::string, std::vector<bool>> &par_vec,
    [[maybe_unused]] const activityMode & am
) const {
#ifdef DEBUG
    if((this->getParameterName()).empty()) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GBooleanObject::booleanStreamline(std::map<std::string, std::vector<bool>>& "
               "par_vec) const: Error!"
            << '\n'
            << "No name was assigned to the object" << '\n'
        );
    }
#endif /* DEBUG */

    std::vector<bool> parameters;
    parameters.push_back(this->value());
    par_vec[this->getParameterName()] = parameters;
}

/******************************************************************************/
/**
 * Attach boundaries of type bool to the vectors. This function has been added for
 * completeness reasons only.
 */
void GBooleanObject::booleanBoundaries(
    std::vector<bool> &l_bnd_vec,
    std::vector<bool> &u_bnd_vec,
    [[maybe_unused]] const activityMode & am
) const {
    l_bnd_vec.push_back(false);
    u_bnd_vec.push_back(true);
}

/******************************************************************************/
/**
 * Tell the audience that we own a bool value
 *
 * @return The number of active, incactive or all float parameters
 */
std::size_t GBooleanObject::countBoolParameters(
    [[maybe_unused]] const activityMode & am
) const {
    return 1;
}

/******************************************************************************/
/**
 * Assigns part of a value vector to the parameter
 */
void GBooleanObject::assignBooleanValueVector(
    const std::vector<bool> &par_vec,
    std::size_t &pos,
    [[maybe_unused]] const activityMode & am
) {
#ifdef DEBUG
    // Do we have a valid position ?
    if(pos >= par_vec.size()) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GBooleanObject::assignBooleanValueVector(const std::vector<bool>&, "
               "std::size_t&):"
            << '\n'
            << "Tried to access position beyond end of vector: " << par_vec.size() << "/" << pos
            << '\n'
        );
    }
#endif

    this->setValue(par_vec[pos]);
    pos++;
}

/******************************************************************************/
/**
 * Assigns part of a value map to the parameter
 */
void GBooleanObject::assignBooleanValueVectors(
    const std::map<std::string, std::vector<bool>> &par_map,
    [[maybe_unused]] const activityMode & am
) {
    this->setValue(
        (Gem::Common::getMapItem<std::vector<bool>>(par_map, this->getParameterName())).at(0)
    );
}

/******************************************************************************/
/**
 * Loads the data of another GParameterBase
 *
 * @param cp A copy of another GBooleanObject object, camouflaged as a GParameterBase
 */
void GBooleanObject::load_(const GParameterBase *cp) {
    // Convert the pointer to our target type and check for self-assignment
    const auto *p_load = Gem::Common::g_convert_and_compare<GParameterBase, GBooleanObject>(cp, this);

    // Load our parent class'es data ...
    GParameterT<bool>::load_(cp);

    // ... no local data
}

/******************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GBooleanObject::modify_GUnitTests_() {
#ifdef GEM_TESTING
    bool result = false;

    // Call the parent class'es function
    if(GParameterT<bool>::modify_GUnitTests_()) {
        result = true;
    }

    this->flip();
    result = true;

    return result;

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset("GBooleanObject::modify_GUnitTests", "GEM_TESTING");
    return false;
#endif                  /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GBooleanObject::specificTestsNoFailureExpected_GUnitTests_() {
#ifdef GEM_TESTING
    // Some general settings
    constexpr double lowerbnd = 0.8;
    constexpr double upperbnd = 1.2;
    constexpr std::size_t n_tests = 10000;

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
    GParameterT<bool>::specificTestsNoFailureExpected_GUnitTests_();

    // A random generator
    Gem::Hap::GRandomT<Gem::Hap::RANDFLAVOURS::RANDOMPROXY> gr;

    // --------------------------------------------------------------------------

    { // Test default constructor
        GBooleanObject gbo;
        INFO(
            "\n"
            << "gbo.value() = " << gbo.value()
            << "DEFBOVAL = " << Gem::Common::GDefaultValueT<bool>::value()
        );
        CHECK(gbo.value() == Gem::Common::GDefaultValueT<bool>::value());
    }

    // --------------------------------------------------------------------------

    { // Test copy construction and construction with value
        GBooleanObject gbo1(false);
        // NOLINTNEXTLINE(performance-unnecessary-copy-initialization) -- intentional copy: this test checks copy construction
        GBooleanObject gbo2(gbo1);

        INFO("\n" << "gbo1.value() = " << gbo1.value() << "gbo2.value() = " << gbo2.value());
        CHECK((not gbo1.value() && gbo2.value() == gbo1.value()));
    }

    // --------------------------------------------------------------------------

    { // Check construction with a given probability for the value "true"
        std::size_t n_true = 0;
        std::size_t n_false = 0;
        for(std::size_t i = 0; i < n_tests; i++) {
            GBooleanObject gbo(0.5);
            gbo.value() ? n_true++ : n_false++;
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

    { // Test that random initialization with equal probability for true and false will result in roughly the same amount of corresponding values
        std::shared_ptr<GBooleanObject> p_test = this->clone<GBooleanObject>();

        // Assign a boolean value true
        CHECK_NOTHROW(*p_test = true);
        // Cross-check
        CHECK(p_test->value());

        // Count the number of true and false values for a number of subsequent initializations
        // with the internal randomInit_ function.
        std::size_t n_true = 0;
        std::size_t n_false = 0;
        for(std::size_t i = 0; i < n_tests; i++) {
            p_test->randomInit_(activityMode::ALLPARAMETERS, gr);
            p_test->value() ? n_true++ : n_false++;
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
        CHECK((ratio > 0.8 && ratio < 1.2));
    }

    // --------------------------------------------------------------------------

    { // Test that initialization with a probability of 1 for true will only result in true values
        std::shared_ptr<GBooleanObject> p_test = this->clone<GBooleanObject>();

        // Assign a boolean value true
        CHECK_NOTHROW(*p_test = false);
        // Cross-check
        CHECK(not p_test->value());

        // Count the number of true and false values for a number of subsequent initializations
        // with the internal randomInit_ function.
        std::size_t n_true = 0;
        std::size_t n_false = 0;
        for(std::size_t i = 0; i < n_tests; i++) {
            p_test->randomInit_(1., activityMode::ALLPARAMETERS, gr);
            p_test->value() ? n_true++ : n_false++;
        }

        // We should have received only true values
        CHECK(n_true == n_tests);
    }

    // --------------------------------------------------------------------------

    { // Test that initialization with a probability of 0 for true will only result in false values
        std::shared_ptr<GBooleanObject> p_test = this->clone<GBooleanObject>();

        // Assign a boolean value true
        CHECK_NOTHROW(*p_test = true);
        // Cross-check
        CHECK(p_test->value());

        // Count the number of true and false values for a number of subsequent initializations
        // with the internal randomInit_ function.
        std::size_t n_true = 0;
        std::size_t n_false = 0;
        for(std::size_t i = 0; i < n_tests; i++) {
            p_test->randomInit_(0., activityMode::ALLPARAMETERS, gr);
            p_test->value() ? n_true++ : n_false++;
        }

        // We should have received only true values
        CHECK(n_false == n_tests);
    }

    //-----------------------------------------------------------------------------

    { // Test that random initialization with a given probability for true will result in roughly the expected amount of corresponding values
        for(int prob_i = 1; prob_i <= 8; ++prob_i) {
            const double d = prob_i * 0.1;
            std::shared_ptr<GBooleanObject> p_test = this->clone<GBooleanObject>();

            // Assign a boolean value true
            CHECK_NOTHROW(*p_test = true);
            // Cross-check
            CHECK(p_test->value());

            // Randomly initialize, using the internal function, with the current probability
            CHECK_NOTHROW(p_test->randomInit_(d, activityMode::ALLPARAMETERS, gr));

            // Count the number of true and false values for a number of subsequent initializations
            // with the internal randomInit_ function.
            std::size_t n_true = 0;
            std::size_t n_false = 0;
            for(std::size_t i = 0; i < n_tests; i++) {
                p_test->randomInit_(d, activityMode::ALLPARAMETERS, gr);
                p_test->value() ? n_true++ : n_false++;
            }

            // We allow a slight deviation, as the initialization is a random process
            double expected_true_min = 0.8 * d * n_tests;
            double expected_true_max = 1.2 * d * n_tests;

            INFO(
                "\n"
                << "d = " << d << "\n"
                << "Allowed window = " << expected_true_min << " - " << expected_true_max << "\n"
                << "n_tests = " << n_tests << "\n"
                << "n_true = " << n_true << "\n"
                << "n_false = " << n_false << "\n"
            );
            CHECK((double(n_true) > expected_true_min && double(n_true) < expected_true_max));
        }
    }

    // --------------------------------------------------------------------------

    { // Check that random initialization can be blocked for equal distributions
        std::shared_ptr<GBooleanObject> p_test1 = this->clone<GBooleanObject>();
        std::shared_ptr<GBooleanObject> p_test2 = this->clone<GBooleanObject>();

        // Assign a boolean value true
        CHECK_NOTHROW(*p_test1 = true);
        // Cross-check
        CHECK(p_test1->value());

        // Block random initialization and cross check
        CHECK_NOTHROW(p_test1->blockRandomInitialization());
        CHECK(p_test1->randomInitializationBlocked());

        // Load the data into p_test2
        CHECK_NOTHROW(p_test2->load(p_test1));

        // Check that both objects are equal
        CHECK(*p_test1 == *p_test2);

        // Check that random initialization is also blocked for p_test2
        CHECK(p_test2->randomInitializationBlocked());

        // Try to randomly initialize, using the *external* function
        CHECK_NOTHROW(p_test1->randomInit(activityMode::ALLPARAMETERS, gr));

        // Check that both objects are still the same
        CHECK(*p_test1 == *p_test2);
    }

    // --------------------------------------------------------------------------

    { // Check that random initialization can be blocked for distributions with a given probability structure
        std::shared_ptr<GBooleanObject> p_test1 = this->clone<GBooleanObject>();
        std::shared_ptr<GBooleanObject> p_test2 = this->clone<GBooleanObject>();

        // Assign a boolean value true
        CHECK_NOTHROW(*p_test1 = true);
        // Cross-check
        CHECK(p_test1->value()); // Should be true

        // Block random initialization and cross check
        CHECK_NOTHROW(p_test1->blockRandomInitialization());
        CHECK(p_test1->randomInitializationBlocked());

        // Load the data into p_test2
        CHECK_NOTHROW(p_test2->load(p_test1));

        // Check that both objects are equal
        CHECK(*p_test1 == *p_test2);

        // Check that random initialization is also blocked for p_test2
        CHECK(p_test2->randomInitializationBlocked());

        // Try to randomly initialize, using the *external* function
        CHECK_NOTHROW(p_test1->randomInit(0.7, activityMode::ALLPARAMETERS, gr));

        // Check that both objects are still the same
        CHECK(*p_test1 == *p_test2);
    }

    // --------------------------------------------------------------------------

    { // Check that the fp-family of functions doesn't have an effect on this object
        std::shared_ptr<GBooleanObject> p_test1 = this->clone<GBooleanObject>();
        std::shared_ptr<GBooleanObject> p_test2 = this->clone<GBooleanObject>();
        std::shared_ptr<GBooleanObject> p_test3 = this->clone<GBooleanObject>();

        // Assign a boolean value true
        CHECK_NOTHROW(*p_test1 = true);
        // Cross-check
        CHECK(p_test1->value()); // should be true

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

    // Restore the adaptor to pristine condition
    this->resetAdaptor();

    // Load the old adaptor, if needed
    if(adaptor_stored) {
        this->addAdaptor(stored_adaptor);
    }

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset(
        "GBooleanObject::specificTestsNoFailureExpected_GUnitTests",
        "GEM_TESTING"
    );
#endif                  /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GBooleanObject::specificTestsFailuresExpected_GUnitTests_() {
#ifdef GEM_TESTING

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
    GParameterT<bool>::specificTestsFailuresExpected_GUnitTests_();

    // Restore the adaptor to pristine condition
    this->resetAdaptor();

    // Load the old adaptor, if needed
    if(adaptor_stored) {
        this->addAdaptor(stored_adaptor);
    }

    // A random generator
    Gem::Hap::GRandomT<Gem::Hap::RANDFLAVOURS::RANDOMPROXY> gr;

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset(
        "GBooleanObject::specificTestsFailuresExpected_GUnitTests",
        "GEM_TESTING"
    );
#endif                  /* GEM_TESTING */
}

/******************************************************************************/

} /* namespace Gem::Geneva::Parameters */
