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

#include "geneva/par/GConstrainedFloatObject.hpp"
#include "common/GCommonHelperFunctionsT.hpp"
#include "common/GExceptions.hpp"
#include "common/GExpectationChecksT.hpp"
#include "common/GLogger.hpp"
#include "geneva/GOptimizationEnums.hpp"
#include "geneva/par/GAdaptorT.hpp"
#include "geneva/par/GConstrainedFPT.hpp"
#include "geneva/par/GFloatGaussAdaptor.hpp"
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

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::Parameters::GConstrainedFloatObject) // NOLINT
namespace Gem::Geneva::Parameters {

/******************************************************************************/
/**
 * Initialization with boundaries only. The value is set randomly.
 *
 * @param lower_boundary The lower boundary of the value range
 * @param upper_boundary The upper boundary of the value range
 */
GConstrainedFloatObject::GConstrainedFloatObject(
    const float &lower_boundary,
    const float &upper_boundary
)
  : GConstrainedFPT<float>(lower_boundary, upper_boundary) { /* nothing */
}

/******************************************************************************/
/**
 * Initialization with value and boundaries
 *
 * @param val Initialization value
 * @param lower_boundary The lower boundary of the value range
 * @param upper_boundary The upper boundary of the value range
 */
GConstrainedFloatObject::GConstrainedFloatObject(
    const float &val,
    const float &lower_boundary,
    const float &upper_boundary
)
  : GConstrainedFPT<float>(val, lower_boundary, upper_boundary) { /* nothing */
}

/******************************************************************************/
/**
 * Initialization by contained value
 *
 * @param val A value used for the initialization
 */
GConstrainedFloatObject::GConstrainedFloatObject(const float &val)
  : GConstrainedFPT<float>(val) { /* nothing */
}

/******************************************************************************/
/**
 * An assignment operator for the contained value type
 *
 * @param val The value to be assigned to this object
 * @return The value that was just assigned to this object
 */
GConstrainedFloatObject &GConstrainedFloatObject::operator=(const float &val) {
    GConstrainedFPT<float>::operator=(val);
    return *this;
}

/******************************************************************************/
/**
 * Creates a deep clone of this object.
 *
 * @return A copy of this object, camouflaged as a GParameterBase
 */
GParameterBase *GConstrainedFloatObject::clone_() const {
    return new GConstrainedFloatObject(*this);
}

/******************************************************************************/
/**
 * Searches for compliance with expectations with respect to another object
 * of the same type
 *
 * @param cp A constant reference to another GParameterBase object
 * @param e The expected outcome of the comparison
 */
void GConstrainedFloatObject::compare_(
    const GParameterBase &cp,
    const Gem::Common::expectation &e,
    [[maybe_unused]] const double & limit
) const {
    using namespace Gem::Common;

    // Check that we are dealing with a GConstrainedFloatObject reference independent of this object and convert the pointer
    const GConstrainedFloatObject *p_load =
        Gem::Common::g_convert_and_compare<GParameterBase, GConstrainedFloatObject>(cp, this);

    GToken token("GConstrainedFloatObject", e);

    // Compare our parent data ...
    Gem::Common::compare_base_t<GConstrainedFPT<float>>(*this, *p_load, token);

    // .... no local data

    // React on deviations from the expectation
    token.evaluate();
}

/***********************************************************************************/
/**
 * Emits a name for this class / object
 */
std::string GConstrainedFloatObject::name_() const {
    return std::string("GConstrainedFloatObject");
}

/******************************************************************************/
/**
 * Attach our local value to the vector. This is used to collect all parameters of this type
 * in the sequence in which they were registered.
 */
void GConstrainedFloatObject::floatStreamline(
    std::vector<float> &par_vec,
    [[maybe_unused]] const activityMode & am
) const {
    // Note: application of the transfer function happens in GConstrainedNumT inside value()
    par_vec.push_back(this->value());
}

/******************************************************************************/
/**
 * Attach our local value to the map.
 */
void GConstrainedFloatObject::floatStreamline(
    std::map<std::string, std::vector<float>> &par_vec,
    [[maybe_unused]] const activityMode & am
) const {
    std::vector<float> parameters;
    // Note: application of the transfer function happens in GConstrainedNumT inside value()
    parameters.push_back(this->value());
    par_vec[this->getParameterName()] = parameters;
}

/******************************************************************************/
/**
 * Attach boundaries of type float to the vectors.
 */
void GConstrainedFloatObject::floatBoundaries(
    std::vector<float> &l_bnd_vec,
    std::vector<float> &u_bnd_vec,
    [[maybe_unused]] const activityMode & am
) const {
    l_bnd_vec.push_back(this->getLowerBoundary());
    u_bnd_vec.push_back(this->getUpperBoundary());
}

/******************************************************************************/
/**
 * Tell the audience that we own a float value
 *
 * @param am An enum indicating whether only information about active, inactive or all parameters of this type should be extracted
 * @return The number 1, as we own a single float parameter
 */
std::size_t GConstrainedFloatObject::countFloatParameters(
    [[maybe_unused]] const activityMode & am
) const {
    return 1;
}

/******************************************************************************/
/**
 * Assigns part of a value vector to the parameter. Note that we apply a transformation to
 * the parameter value, so that it lies inside of the allowed value range.
 */
void GConstrainedFloatObject::assignFloatValueVector(
    const std::vector<float> &par_vec,
    std::size_t &pos,
    [[maybe_unused]] const activityMode & am
) {
#ifdef DEBUG
    // Do we have a valid position ?
    if(pos >= par_vec.size()) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GConstrainedFloatObject::assignFloatValueVector(const std::vector<float>&, "
               "std::size_t&):"
            << '\n'
            << "Tried to access position beyond end of vector: " << par_vec.size() << "/" << pos
            << '\n'
        );
    }
#endif

    this->setValue(this->transfer(par_vec[pos]));
    pos++;
}

/******************************************************************************/
/**
 * Assigns part of a value map to the parameter
 */
void GConstrainedFloatObject::assignFloatValueVectors(
    const std::map<std::string, std::vector<float>> &par_map,
    [[maybe_unused]] const activityMode & am
) {
    this->setValue(
        this->transfer(Gem::Common::getMapItem(par_map, this->getParameterName()).at(0))
    );
}

/******************************************************************************/
/**
 * Multiplication with a random value in a given range
 */
void GConstrainedFloatObject::floatMultiplyByRandom(
    const float &min,
    const float &max,
    [[maybe_unused]] const activityMode & am
    ,
    Gem::Hap::GRandomBase &gr // NOLINT(misc-unused-parameters)
) {
    std::uniform_real_distribution<float> uniform_real_distribution(min, max);
    GParameterT<float>::setValue(
        transfer(GParameterT<float>::value() * uniform_real_distribution(gr))
    );
}

/******************************************************************************/
/**
 * Multiplication with a random value in the range [0,1[
 */
void GConstrainedFloatObject::floatMultiplyByRandom(
    [[maybe_unused]] const activityMode & am
    ,
    Gem::Hap::GRandomBase &gr // NOLINT(misc-unused-parameters)
) {
    std::uniform_real_distribution<float> uniform_real_distribution(0.f, 1.f);
    GParameterT<float>::setValue(
        transfer(GParameterT<float>::value() * uniform_real_distribution(gr))
    );
}

/******************************************************************************/
/**
 * Multiplication with a constant value
 */
void GConstrainedFloatObject::floatMultiplyBy(const float &val, const activityMode &am) {
    GParameterT<float>::setValue(transfer(val * GParameterT<float>::value()));
}

/******************************************************************************/
/**
 * Initialization with a constant value
 */
void GConstrainedFloatObject::floatFixedValueInit(const float &val, const activityMode &am) {
    GParameterT<float>::setValue(transfer(val));
}

/******************************************************************************/
/**
 * Adds the "same-type" parameters of another GParameterBase object to this one
 */
void GConstrainedFloatObject::floatAdd(
    std::shared_ptr<GParameterBase> p_base,
    [[maybe_unused]] const activityMode & am
) {
    // We first need to convert p_base into the local type
    std::shared_ptr<GConstrainedFloatObject> p =
        GParameterBase::parameterbase_cast<GConstrainedFloatObject>(p_base);
    GParameterT<float>::setValue(transfer(this->value() + p->value()));
}

/******************************************************************************/
/**
 * Adds the "same-type" parameters of another GParameterBase object to this one
 */
void GConstrainedFloatObject::floatSubtract(
    std::shared_ptr<GParameterBase> p_base,
    [[maybe_unused]] const activityMode & am
) {
    // We first need to convert p_base into the local type
    std::shared_ptr<GConstrainedFloatObject> p =
        GParameterBase::parameterbase_cast<GConstrainedFloatObject>(p_base);
    GParameterT<float>::setValue(transfer(this->value() - p->value()));
}

/******************************************************************************/
/**
 * Loads the data of another GParameterBase
 *
 * @param cp A copy of another GConstrainedFloatObject object, camouflaged as a GParameterBase
 */
void GConstrainedFloatObject::load_(const GParameterBase *cp) {
    // Convert the pointer to our target type and check for self-assignment
    const GConstrainedFloatObject *p_load =
        Gem::Common::g_convert_and_compare<GParameterBase, GConstrainedFloatObject>(cp, this);

    // Load our parent class'es data ...
    GConstrainedFPT<float>::load_(cp);

    // ... no local data
}

/******************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GConstrainedFloatObject::modify_GUnitTests_() {
#ifdef GEM_TESTING
    // A random generator
    Gem::Hap::GRandomT<Gem::Hap::RANDFLAVOURS::RANDOMPROXY> gr;

    bool result = false;

    // Call the parent class'es function
    if(GConstrainedFPT<float>::modify_GUnitTests_()) {
        result = true;
    }

    this->randomInit(activityMode::ALLPARAMETERS, gr);
    result = true;

    return result;

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset("GConstrainedFloatObject::modify_GUnitTests", "GEM_TESTING");
    return false;
#endif                  /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GConstrainedFloatObject::specificTestsNoFailureExpected_GUnitTests_() {
#ifdef GEM_TESTING
    // Some general settings
    constexpr float test_val = 42.f;
    constexpr float test_val2 = 17.f;
    float test_val3 = 0.f;
    constexpr float lower_boundary = 0.f;
    constexpr float upper_boundary = 100.f;
    constexpr std::size_t ntests = 100;

    // Make sure we have an appropriate adaptor loaded when performing these tests
    bool adaptor_stored = false;
    std::unique_ptr<adaptor_base_t> stored_adaptor;

    if(this->hasAdaptor()) {
        stored_adaptor = this->getAdaptor().clone_unique();
        adaptor_stored = true;
    }

    std::shared_ptr<GFloatGaussAdaptor> gfga_ptr(new GFloatGaussAdaptor(0.025f, 0.1f, 0.f, 1.f, 1.0));
    gfga_ptr->setAdaptionThreshold(
        0
    ); // Make sure the adaptor's internal parameters don't change through the adaption
    gfga_ptr->setAdaptionMode(adaptionMode::ALWAYS); // Always adapt
    this->addAdaptor(gfga_ptr);

    // Call the parent class'es function
    GConstrainedFPT<float>::specificTestsNoFailureExpected_GUnitTests_();

    // --------------------------------------------------------------------------

    { // Check that assignment of a value with operator= works both for set and unset boundaries
        std::shared_ptr<GConstrainedFloatObject> p_test =
            this->clone<GConstrainedFloatObject>();

        // Reset the boundaries so we are free to do what we want
        CHECK_NOTHROW(p_test->resetBoundaries());

        // Assign a value with operator=
        CHECK_NOTHROW(*p_test = test_val2);

        // Check the value
        CHECK(p_test->value() == test_val2);

        // Assign boundaries and values
        CHECK_NOTHROW(p_test->setValue(test_val2, lower_boundary, upper_boundary));

        // Check the value again
        CHECK(p_test->value() == test_val2);

        // Assign a value with operator=
        CHECK_NOTHROW(*p_test = test_val);

        // Check the value again, should have changed
        CHECK(p_test->value() == test_val);
    }

    // --------------------------------------------------------------------------

    { // Check construction with two boundaries plus initialization with a random value and extraction of that value
        std::shared_ptr<GConstrainedFloatObject> p_test(new GConstrainedFloatObject(0.3f, 0.6f));
        CHECK_NOTHROW(test_val3 = p_test->value());
    }

    // --------------------------------------------------------------------------

    { // Check construction with two boundaries and a value and extraction of that value
        constexpr float testval = 0.4f;
        std::shared_ptr<GConstrainedFloatObject> p_test(
            new GConstrainedFloatObject(0.4f, 0.3f, 0.6f)
        );
        CHECK_NOTHROW(test_val3 = p_test->value());
        CHECK(test_val3 == testval);
    }

    // --------------------------------------------------------------------------

    { // Check that repeated retrieval of the value always yields the same value
        constexpr float testval = 0.4f;
        std::shared_ptr<GConstrainedFloatObject> p_test(
            new GConstrainedFloatObject(0.4f, 0.3f, 0.6f)
        );
        for(std::size_t i = 0; i < ntests; i++) {
            CHECK_NOTHROW(test_val3 = p_test->value());
            INFO("The value has changed: " << test_val3 << " / " << testval);
            CHECK(test_val3 == testval);
        }
    }

    // --------------------------------------------------------------------------

    // Remove the test adaptor
    this->resetAdaptor();

    // Load the old adaptor, if needed
    if(adaptor_stored) {
        this->addAdaptor(Gem::Common::nonOwningShared(stored_adaptor));
    }

    // --------------------------------------------------------------------------

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset(
        "GConstrainedFloatObject::specificTestsNoFailureExpected_GUnitTests",
        "GEM_TESTING"
    );
#endif                  /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GConstrainedFloatObject::specificTestsFailuresExpected_GUnitTests_() {
#ifdef GEM_TESTING
    // Make sure we have an appropriate adaptor loaded when performing these tests
    bool adaptor_stored = false;
    std::unique_ptr<adaptor_base_t> stored_adaptor;

    if(this->hasAdaptor()) {
        stored_adaptor = this->getAdaptor().clone_unique();
        adaptor_stored = true;
    }

    std::shared_ptr<GFloatGaussAdaptor> gfga_ptr(new GFloatGaussAdaptor(0.025f, 0.1f, 0.f, 1.f, 1.0));
    gfga_ptr->setAdaptionThreshold(
        0
    ); // Make sure the adaptor's internal parameters don't change through the adaption
    gfga_ptr->setAdaptionMode(adaptionMode::ALWAYS); // Always adapt
    this->addAdaptor(gfga_ptr);

    // Call the parent class'es function
    GConstrainedFPT<float>::specificTestsFailuresExpected_GUnitTests_();

    // Remove the test adaptor
    this->resetAdaptor();

    // Load the old adaptor, if needed
    if(adaptor_stored) {
        this->addAdaptor(Gem::Common::nonOwningShared(stored_adaptor));
    }

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset(
        "GConstrainedFloatObject::specificTestsFailuresExpected_GUnitTests",
        "GEM_TESTING"
    );
#endif                  /* GEM_TESTING */
}

/******************************************************************************/

} /* namespace Gem::Geneva::Parameters */
