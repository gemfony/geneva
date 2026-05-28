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
#include "geneva/par/GDoubleObject.hpp"
#include "common/GCommonHelperFunctionsT.hpp"
#include "common/GExceptions.hpp"
#include "common/GExpectationChecksT.hpp"
#include "common/GLogger.hpp"
#include "geneva/GOptimizationEnums.hpp"
#include "geneva/par/GAdaptorT.hpp"
#include "geneva/par/GDoubleGaussAdaptor.hpp"
#include "geneva/par/GNumFPT.hpp"
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

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::Parameters::GDoubleObject) // NOLINT
namespace Gem::Geneva::Parameters {

/******************************************************************************/
/**
 * Initialization by contained value
 *
 * @param val A value used for the initialization
 */
GDoubleObject::GDoubleObject(const double &val)
  : GNumFPT<double>(val) { /* nothing */
}

/******************************************************************************/
/**
 * Initialization by random number in a given range
 *
 * @param lower_boundary The lower boundary for the random number used in the initialization
 * @param upper_boundary The upper boundary for the random number used in the initialization
 */
GDoubleObject::GDoubleObject(const double &lower_boundary, const double &upper_boundary)
  : GNumFPT<double>(lower_boundary, upper_boundary) { /* nothing */
}

/******************************************************************************/
/**
 * Initialization with a fixed value and the range for random initialization
 *
 * @param val The value to be assigned to the object
 * @param lower_boundary The lower boundary for random initialization
 * @param upper_boundary The upper boundary for random initialization
 */
GDoubleObject::GDoubleObject(
    const double &val,
    const double &lower_boundary,
    const double &upper_boundary
)
  : GNumFPT<double>(val, lower_boundary, upper_boundary) { /* nothing */
}

/******************************************************************************/
/**
 * An assignment operator
 *
 * @param val The value to be assigned to this object
 * @return The value that was just assigned to this object
 */
GDoubleObject &GDoubleObject::operator=(const double &val) {
    GNumFPT<double>::operator=(val);
    return *this;
}

/******************************************************************************/
/**
 * Creates a deep clone of this object.
 *
 * @return A copy of this object, camouflaged as a GParameterBase
 */
GParameterBase *GDoubleObject::clone_() const {
    return new GDoubleObject(*this);
}

/******************************************************************************/
/**
 * Searches for compliance with expectations with respect to another object
 * of the same type
 *
 * @param cp A constant reference to another GParameterBase object
 * @param e The expected outcome of the comparison
 */
void GDoubleObject::compare_(
    const GParameterBase &cp,
    const Gem::Common::expectation &e,
    [[maybe_unused]] const double & limit
) const {
    using namespace Gem::Common;

    // Check that we are dealing with a GDoubleObject reference independent of this object and convert the pointer
    const GDoubleObject *p_load =
        Gem::Common::g_convert_and_compare<GParameterBase, GDoubleObject>(cp, this);

    GToken token("GDoubleObject", e);

    // Compare our parent data ...
    Gem::Common::compare_base_t<GNumFPT<double>>(*this, *p_load, token);

    // ... no local data

    // React on deviations from the expectation
    token.evaluate();
}

/***********************************************************************************/
/**
 * Emits a name for this class / object
 */
std::string GDoubleObject::name_() const {
    return std::string("GDoubleObject");
}

/******************************************************************************/
/**
 * Attach our local value to the vector. This is used to collect all parameters of this type
 * in the sequence in which they were registered.
 */
void GDoubleObject::doubleStreamline(
    std::vector<double> &par_vec,
    [[maybe_unused]] const activityMode & am
) const {
    par_vec.push_back(this->value());
}

/******************************************************************************/
/**
 * Attach our local value to the map.
 */
void GDoubleObject::doubleStreamline(
    std::map<std::string, std::vector<double>> &par_vec,
    [[maybe_unused]] const activityMode & am
) const {
#ifdef DEBUG
    if((this->getParameterName()).empty()) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GDoubleObject::doubleStreamline(std::map<std::string, std::vector<double>>& "
               "par_vec) const: Error!"
            << '\n'
            << "No name was assigned to the object" << '\n'
        );
    }
#endif /* DEBUG */

    std::vector<double> parameters;
    parameters.push_back(this->value());
    par_vec[this->getParameterName()] = parameters;
}

/******************************************************************************/
/**
 * Attach boundaries of type double to the vectors. Since this is an unbounded type,
 * we use the initialization boundaries as a replacement.
 */
void GDoubleObject::doubleBoundaries(
    std::vector<double> &l_bnd_vec,
    std::vector<double> &u_bnd_vec,
    [[maybe_unused]] const activityMode & am
) const {
    l_bnd_vec.push_back(this->getLowerInitBoundary());
    u_bnd_vec.push_back(this->getUpperInitBoundary());
}

/******************************************************************************/
/**
 * Tell the audience that we own a double value
 *
 * @param am An enum indicating whether only information about active, inactive or all parameters of this type should be extracted
 * @return The number 1, as we own a single double parameter
 */
std::size_t GDoubleObject::countDoubleParameters(
    [[maybe_unused]] const activityMode & am
) const {
    return 1;
}

/******************************************************************************/
/**
 * Assigns part of a value vector to the parameter
 */
void GDoubleObject::assignDoubleValueVector(
    const std::vector<double> &par_vec,
    std::size_t &pos,
    [[maybe_unused]] const activityMode & am
) {
#ifdef DEBUG
    // Do we have a valid position ?
    if(pos >= par_vec.size()) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GDoubleObject::assignDoubleValueVector(const std::vector<double>&, "
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
void GDoubleObject::assignDoubleValueVectors(
    const std::map<std::string, std::vector<double>> &par_map,
    [[maybe_unused]] const activityMode & am
) {
    this->setValue((Gem::Common::getMapItem(par_map, this->getParameterName())).at(0));
}

/******************************************************************************/
/**
 * Multiplication with a random value in a given range
 */
void GDoubleObject::doubleMultiplyByRandom(
    const double &min,
    const double &max,
    [[maybe_unused]] const activityMode & am
    ,
    Gem::Hap::GRandomBase &gr // NOLINT(misc-unused-parameters)
) {
    std::uniform_real_distribution<double> uniform_real_distribution(min, max);
    GParameterT<double>::setValue(GParameterT<double>::value() * uniform_real_distribution(gr));
}

/******************************************************************************/
/**
 * Multiplication with a random value in the range [0,1[
 */
void GDoubleObject::doubleMultiplyByRandom(
    [[maybe_unused]] const activityMode & am
    ,
    Gem::Hap::GRandomBase &gr // NOLINT(misc-unused-parameters)
) {
    std::uniform_real_distribution<double> uniform_real_distribution(0., 1.);
    GParameterT<double>::setValue(GParameterT<double>::value() * uniform_real_distribution(gr));
}

/******************************************************************************/
/**
 * Multiplication with a constant value
 */
void GDoubleObject::doubleMultiplyBy(const double &val, const activityMode &am) {
    GParameterT<double>::setValue(val * GParameterT<double>::value());
}

/******************************************************************************/
/**
 * Initialization with a constant value
 */
void GDoubleObject::doubleFixedValueInit(const double &val, const activityMode &am) {
    GParameterT<double>::setValue(val);
}

/******************************************************************************/
/**
 * Adds the "same-type" parameters of another GParameterBase object to this one
 */
void GDoubleObject::doubleAdd(
    std::shared_ptr<GParameterBase> p_base,
    [[maybe_unused]] const activityMode & am
) {
    // We first need to convert p_base into the local type
    std::shared_ptr<GDoubleObject> p = GParameterBase::parameterbase_cast<GDoubleObject>(p_base);
    GParameterT<double>::setValue(this->value() + p->value());
}

/******************************************************************************/
/**
 * Adds the "same-type" parameters of another GParameterBase object to this one
 */
void GDoubleObject::doubleSubtract(
    std::shared_ptr<GParameterBase> p_base,
    [[maybe_unused]] const activityMode & am
) {
    // We first need to convert p_base into the local type
    std::shared_ptr<GDoubleObject> p = GParameterBase::parameterbase_cast<GDoubleObject>(p_base);
    GParameterT<double>::setValue(this->value() - p->value());
}

/******************************************************************************/
/**
 * Loads the data of another GParameterBase
 *
 * @param cp A copy of another GDoubleObject object, camouflaged as a GParameterBase
 */
void GDoubleObject::load_(const GParameterBase *cp) {
    // Convert the pointer to our target type and check for self-assignment
    const GDoubleObject *p_load =
        Gem::Common::g_convert_and_compare<GParameterBase, GDoubleObject>(cp, this);

    // Load our parent class'es data ...
    GNumFPT<double>::load_(cp);

    // ... no local data
}

/******************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GDoubleObject::modify_GUnitTests_() {
#ifdef GEM_TESTING
    // A random generator
    Gem::Hap::GRandomT<Gem::Hap::RANDFLAVOURS::RANDOMPROXY> gr;

    bool result = false;

    // Call the parent class'es function
    if(GNumFPT<double>::modify_GUnitTests_()) {
        result = true;
    }

    this->randomInit(activityMode::ALLPARAMETERS, gr);
    result = true;

    return result;

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset("GDoubleObject::modify_GUnitTests", "GEM_TESTING");
    return false;
#endif                  /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GDoubleObject::specificTestsNoFailureExpected_GUnitTests_() {
#ifdef GEM_TESTING
    // A few settings
    constexpr std::size_t n_tests = 10000;

    // Make sure we have an appropriate adaptor loaded when performing these tests
    bool adaptor_stored = false;
    std::shared_ptr<GAdaptorT<double>> stored_adaptor;

    if(this->hasAdaptor()) {
        stored_adaptor = this->getAdaptor();
        adaptor_stored = true;
    }

    std::shared_ptr<GDoubleGaussAdaptor> gdga_ptr(
        new GDoubleGaussAdaptor(0.025, 0.1, 0., 0.5, 1.0)
    );
    gdga_ptr->setAdaptionThreshold(
        0
    ); // Make sure the adaptor's internal parameters don't change through the adaption
    gdga_ptr->setAdaptionMode(adaptionMode::ALWAYS); // Always adapt
    this->addAdaptor(gdga_ptr);

    // Call the parent class'es function
    GNumFPT<double>::specificTestsNoFailureExpected_GUnitTests_();

    // Get a random number generator
    Gem::Hap::GRandomT<Gem::Hap::RANDFLAVOURS::RANDOMPROXY> gr;

    // --------------------------------------------------------------------------

    { // Test of GParameterT<T>'s methods for setting and retrieval of values
        std::shared_ptr<GDoubleObject> p_test = this->clone<GDoubleObject>();

        for(int val_i = 0; val_i < 1000; ++val_i) {
            const double d = val_i * 0.01;
            CHECK_NOTHROW((*p_test) = d);       // Setting using operator=()
            CHECK(p_test->value() == d);        // Retrieval through the value() function
            CHECK_NOTHROW(p_test->setValue(d)); // Setting using the setValue() function
            CHECK(p_test->value() == d);        // Retrieval through the value() function
            CHECK_NOTHROW(
                p_test->setValue_(d)
            ); // Setting using the protected constant setValue_() function
            CHECK(p_test->value() == d); // Retrieval through the value() function
        }
    }

    // --------------------------------------------------------------------------

    { // Test automatic conversion to the target type, using GParameterT<T>'s operator T()
        std::shared_ptr<GDoubleObject> p_test = this->clone<GDoubleObject>();

        double target = -1.;
        for(int val_i = 0; val_i < 1000; ++val_i) {
            const double d = val_i * 0.01;
            CHECK_NOTHROW(p_test->setValue(d)); // Setting using the setValue() function
            CHECK_NOTHROW(target = *p_test);    // Automatic conversion
            CHECK(target == d);                 // Cross-check
        }
    }

    // --------------------------------------------------------------------------

    { // Test the GParameterT<T>::adapt() implementation
        std::shared_ptr<GDoubleObject> p_test = this->clone<GDoubleObject>();

        if(p_test->hasAdaptor()) {
            CHECK_NOTHROW(*p_test = 1.);
            double orig_val = *p_test;
            CHECK(*p_test == 1.);
            CHECK(orig_val == 1.);

            for(std::size_t i = 0; i < n_tests; i++) {
                CHECK_NOTHROW(p_test->adapt(gr));
                CHECK(orig_val != *p_test); // Should be different
                CHECK_NOTHROW(orig_val = *p_test);
            }
        }
    }

    // --------------------------------------------------------------------------

    { // Test resetting, adding and retrieval of adaptors in GParameterBaseWithAdaptorsT<T>
        std::shared_ptr<GDoubleObject> p_test = this->clone<GDoubleObject>();

        // Reset the local adaptor to its pristine condition
        CHECK_NOTHROW(p_test->resetAdaptor());

        // Add a new adaptor. This should clone the adaptor
        CHECK_NOTHROW(p_test->addAdaptor(gdga_ptr));

        // Check that we indeed have an adaptor (should always be the case)
        CHECK(p_test->hasAdaptor() == true);

        // Retrieve a pointer to the adaptor
        std::shared_ptr<GAdaptorT<double>> p_adaptor_base;
        CHECK(not p_adaptor_base);
        CHECK_NOTHROW(p_adaptor_base = p_test->getAdaptor());

        // Check that we have indeed received an adaptor
        CHECK(p_adaptor_base);

        // Retrieve another, converted pointer to the adaptor
        std::shared_ptr<GDoubleGaussAdaptor> gdga_clone_ptr;
        CHECK(not gdga_clone_ptr);
        CHECK_NOTHROW(gdga_clone_ptr = p_test->getAdaptor<GDoubleGaussAdaptor>());

        // Check that we have indeed received an adaptor
        CHECK(gdga_clone_ptr);

        // The address of the original adaptor and of this one should differ
        CHECK(gdga_clone_ptr.get() != gdga_ptr.get());

        // The adaptors should otherwise be identical
        CHECK(*gdga_clone_ptr == *gdga_ptr);
    }

    // --------------------------------------------------------------------------

    { // Test that retrieval of adaptor doesn't throw in GParameterBaseWithAdaptorsT<T>::getAdaptor() after calling resetAdaptor() (Note: This is the non-templated version of the function)
        std::shared_ptr<GDoubleObject> p_test = this->clone<GDoubleObject>();

        // Make sure the adaptor is in pristine condition
        CHECK_NOTHROW(p_test->resetAdaptor());
        CHECK(p_test->hasAdaptor() == true);
        CHECK_NOTHROW(p_test->getAdaptor());
    }

    // --------------------------------------------------------------------------

    { // Test that retrieval of an adaptor doesn't throw in GParameterBaseWithAdaptorsT<T>::getAdaptor<>() after calling resetAdaptor() (Note: This is the templated version of the function)
        std::shared_ptr<GDoubleObject> p_test = this->clone<GDoubleObject>();

        // Make sure no adaptor is present
        CHECK_NOTHROW(p_test->resetAdaptor());
        CHECK(p_test->hasAdaptor() == true);
        CHECK_NOTHROW(p_test->getAdaptor<GDoubleGaussAdaptor>());
    }

    // Remove the test adaptor
    this->resetAdaptor();

    // Load the old adaptor, if needed
    if(adaptor_stored) {
        this->addAdaptor(stored_adaptor);
    }

    // --------------------------------------------------------------------------

    { // Check that construction with initialization boundaries leads to random content

        double previous = -1.;
        for(std::size_t i = 0; i < 10; i++) {
            GDoubleObject p(0., 10000000.);
            CHECK(p.value() != previous);
            previous = p.value();
        }
    }

    // --------------------------------------------------------------------------

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset(
        "GDoubleObject::specificTestsNoFailureExpected_GUnitTests",
        "GEM_TESTING"
    );
#endif                  /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GDoubleObject::specificTestsFailuresExpected_GUnitTests_() {
#ifdef GEM_TESTING
    // Make sure we have an appropriate adaptor loaded when performing these tests
    bool adaptor_stored = false;
    std::shared_ptr<GAdaptorT<double>> stored_adaptor;

    if(this->hasAdaptor()) {
        stored_adaptor = this->getAdaptor();
        adaptor_stored = true;
    }

    std::shared_ptr<GDoubleGaussAdaptor> gdga_ptr(
        new GDoubleGaussAdaptor(0.025, 0.1, 0., 0.5, 1.0)
    );
    gdga_ptr->setAdaptionThreshold(
        0
    ); // Make sure the adaptor's internal parameters don't change through the adaption
    gdga_ptr->setAdaptionMode(adaptionMode::ALWAYS); // Always adapt
    this->addAdaptor(gdga_ptr);

    // Call the parent class'es function
    GNumFPT<double>::specificTestsFailuresExpected_GUnitTests_();

    // --------------------------------------------------------------------------

    { // Test of GParameterBaseWithAdaptorsT<T>::addAdaptor() in case of an empty adaptor pointer
        std::shared_ptr<GDoubleObject> p_test = this->clone<GDoubleObject>();

        // Make sure the object is in pristine condition
        CHECK_NOTHROW(p_test->resetAdaptor());

        // Add an empty std::shared_ptr<GDoubleGaussAdaptor>. This should throw
        CHECK_THROWS_AS(
            p_test->addAdaptor(std::shared_ptr<GDoubleGaussAdaptor>()),
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
        "GDoubleObject::specificTestsFailuresExpected_GUnitTests",
        "GEM_TESTING"
    );
#endif                  /* GEM_TESTING */
}

/******************************************************************************/

} /* namespace Gem::Geneva::Parameters */
