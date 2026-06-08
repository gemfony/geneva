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

#include "geneva/par/GConstrainedDoubleObject.hpp"
#include "common/GCommonHelperFunctionsT.hpp"
#include "common/GExceptions.hpp"
#include "common/GExpectationChecksT.hpp"
#include "common/GLogger.hpp"
#include "geneva/GOptimizationEnums.hpp"
#include "geneva/par/GAdaptorT.hpp"
#include "geneva/par/GConstrainedFPT.hpp"
#include "geneva/par/GDoubleGaussAdaptor.hpp"
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

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::Parameters::GConstrainedDoubleObject) // NOLINT
namespace Gem::Geneva::Parameters {

/******************************************************************************/
/**
 * Initialization with boundaries only. The value is set randomly.
 *
 * @param lower_boundary The lower boundary of the value range
 * @param upper_boundary The upper boundary of the value range
 */
GConstrainedDoubleObject::GConstrainedDoubleObject(
    const double &lower_boundary,
    const double &upper_boundary
)
  : GConstrainedFPT<double>(lower_boundary, upper_boundary) { /* nothing */
}

/******************************************************************************/
/**
 * Initialization with value and boundaries
 *
 * @param val Initialization value
 * @param lower_boundary The lower boundary of the value range
 * @param upper_boundary The upper boundary of the value range
 */
GConstrainedDoubleObject::GConstrainedDoubleObject(
    const double &val,
    const double &lower_boundary,
    const double &upper_boundary
)
  : GConstrainedFPT<double>(val, lower_boundary, upper_boundary) { /* nothing */
}

/******************************************************************************/
/**
 * Initialization by contained value
 *
 * @param val A value used for the initialization
 */
GConstrainedDoubleObject::GConstrainedDoubleObject(const double &val)
  : GConstrainedFPT<double>(val) { /* nothing */
}

/******************************************************************************/
/**
 * An assignment operator for the contained value type
 *
 * @param val The value to be assigned to this object
 * @return The value that was just assigned to this object
 */
GConstrainedDoubleObject &GConstrainedDoubleObject::operator=(const double &val) {
    GConstrainedFPT<double>::operator=(val);
    return *this;
}

/******************************************************************************/
/**
 * Creates a deep clone of this object.
 *
 * @return A copy of this object, camouflaged as a GParameterBase
 */
GParameterBase *GConstrainedDoubleObject::clone_() const {
    return new GConstrainedDoubleObject(*this);
}

/******************************************************************************/
/**
 * Searches for compliance with expectations with respect to another object
 * of the same type
 *
 * @param cp A constant reference to another GParameterBase object
 * @param e The expected outcome of the comparison
 */
void GConstrainedDoubleObject::compare_(
    const GParameterBase &cp,
    const Gem::Common::expectation &e,
    [[maybe_unused]] const double & limit
) const {
    using namespace Gem::Common;

    // Check that we are dealing with a GConstrainedDoubleObject reference independent of this object and convert the pointer
    const GConstrainedDoubleObject *p_load =
        Gem::Common::g_convert_and_compare<GParameterBase, GConstrainedDoubleObject>(cp, this);

    GToken token("GConstrainedDoubleObject", e);

    // Compare our parent data ...
    Gem::Common::compare_base_t<GConstrainedFPT<double>>(*this, *p_load, token);

    // .... no local data

    // React on deviations from the expectation
    token.evaluate();
}

/***********************************************************************************/
/**
 * Emits a name for this class / object
 */
std::string GConstrainedDoubleObject::name_() const {
    return std::string("GConstrainedDoubleObject");
}

/******************************************************************************/
/**
 * Attach our local value to the vector. This is used to collect all parameters of this type
 * in the sequence in which they were registered.
 */
void GConstrainedDoubleObject::doubleStreamline(
    std::vector<double> &par_vec,
    [[maybe_unused]] const activityMode & am
) const {
    // Note: application of the transfer function happens in GConstrainedNumT inside value()
    par_vec.push_back(this->value());
}

/******************************************************************************/

/******************************************************************************/
/**
 * Attach boundaries of type double to the vectors.
 */
void GConstrainedDoubleObject::doubleBoundaries(
    std::vector<double> &l_bnd_vec,
    std::vector<double> &u_bnd_vec,
    [[maybe_unused]] const activityMode & am
) const {
    l_bnd_vec.push_back(this->getLowerBoundary());
    u_bnd_vec.push_back(this->getUpperBoundary());
}

/******************************************************************************/
/**
 * Tell the audience that we own a double value
 *
 * @param am An enum indicating whether only information about active, inactive or all parameters of this type should be extracted
 * @return The number 1, as we own a single double parameter
 */
std::size_t GConstrainedDoubleObject::countDoubleParameters(
    [[maybe_unused]] const activityMode & am
) const {
    return 1;
}

/******************************************************************************/
/**
 * Assigns part of a value vector to the parameter. Note that we apply a transformation to
 * the parameter value, so that it lies inside of the allowed value range.
 */
void GConstrainedDoubleObject::assignDoubleValueVector(
    const std::vector<double> &par_vec,
    std::size_t &pos,
    [[maybe_unused]] const activityMode & am
) {
#ifdef DEBUG
    // Do we have a valid position ?
    if(pos >= par_vec.size()) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GConstrainedDoubleObject::assignDoubleValueVector(const std::vector<double>&, "
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

/******************************************************************************/
/**
 * Multiplication with a random value in a given range
 */
void GConstrainedDoubleObject::doubleMultiplyByRandom(
    const double &min,
    const double &max,
    [[maybe_unused]] const activityMode & am
    ,
    Gem::Hap::GRandomBase &gr // NOLINT(misc-unused-parameters)
) {
    std::uniform_real_distribution<double> uniform_real_distribution(min, max);
    GParameterT<double>::setValue(
        transfer(GParameterT<double>::value() * uniform_real_distribution(gr))
    );
}

/******************************************************************************/
/**
 * Multiplication with a random value in the range [0,1[
 */
void GConstrainedDoubleObject::doubleMultiplyByRandom(
    [[maybe_unused]] const activityMode & am
    ,
    Gem::Hap::GRandomBase &gr // NOLINT(misc-unused-parameters)
) {
    std::uniform_real_distribution<double> uniform_real_distribution(0., 1.);
    GParameterT<double>::setValue(
        transfer(GParameterT<double>::value() * uniform_real_distribution(gr))
    );
}

/******************************************************************************/
/**
 * Multiplication with a constant value
 */
void GConstrainedDoubleObject::doubleMultiplyBy(const double &val, const activityMode &am) {
    GParameterT<double>::setValue(transfer(val * GParameterT<double>::value()));
}

/******************************************************************************/
/**
 * Initialization with a constant value
 */
void GConstrainedDoubleObject::doubleFixedValueInit(const double &val, const activityMode &am) {
    GParameterT<double>::setValue(transfer(val));
}

/******************************************************************************/
/**
 * Adds the "same-type" parameters of another GParameterBase object to this one
 */
void GConstrainedDoubleObject::doubleAdd(
    std::shared_ptr<GParameterBase> p_base,
    [[maybe_unused]] const activityMode & am
) {
    // We first need to convert p_base into the local type
    std::shared_ptr<GConstrainedDoubleObject> p =
        GParameterBase::parameterbase_cast<GConstrainedDoubleObject>(p_base);
    GParameterT<double>::setValue(transfer(this->value() + p->value()));
}

/******************************************************************************/
/**
 * Adds the "same-type" parameters of another GParameterBase object to this one
 */
void GConstrainedDoubleObject::doubleSubtract(
    std::shared_ptr<GParameterBase> p_base,
    [[maybe_unused]] const activityMode & am
) {
    // We first need to convert p_base into the local type
    std::shared_ptr<GConstrainedDoubleObject> p =
        GParameterBase::parameterbase_cast<GConstrainedDoubleObject>(p_base);
    GParameterT<double>::setValue(transfer(this->value() - p->value()));
}

/******************************************************************************/
/**
 * Loads the data of another GParameterBase
 *
 * @param cp A copy of another GConstrainedDoubleObject object, camouflaged as a GParameterBase
 */
void GConstrainedDoubleObject::load_(const GParameterBase *cp) {
    // Convert the pointer to our target type and check for self-assignment
    const GConstrainedDoubleObject *p_load =
        Gem::Common::g_convert_and_compare<GParameterBase, GConstrainedDoubleObject>(cp, this);

    // Load our parent class'es data ...
    GConstrainedFPT<double>::load_(cp);

    // ... no local data
}

/******************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GConstrainedDoubleObject::modify_GUnitTests_() {
#ifdef GEM_TESTING
    // A random generator
    Gem::Hap::GRandomT<Gem::Hap::RANDFLAVOURS::RANDOMPROXY> gr;

    bool result = false;

    // Call the parent class'es function
    if(GConstrainedFPT<double>::modify_GUnitTests_()) {
        result = true;
    }

    this->randomInit(activityMode::ALLPARAMETERS, gr);
    result = true;

    return result;

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset("GConstrainedDoubleObject::modify_GUnitTests", "GEM_TESTING");
    return false;
#endif                  /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GConstrainedDoubleObject::specificTestsNoFailureExpected_GUnitTests_() {
#ifdef GEM_TESTING
    // Some general settings
    constexpr double test_val = 42.;
    constexpr double test_val2 = 17.;
    double test_val3 = 0.;
    constexpr double lower_boundary = 0.;
    constexpr double upper_boundary = 100.;
    constexpr std::size_t ntests = 100;

    // Make sure we have an appropriate adaptor loaded when performing these tests
    bool adaptor_stored = false;
    std::unique_ptr<adaptor_base_t> stored_adaptor;

    if(this->hasAdaptor()) {
        stored_adaptor = this->getAdaptor().clone_unique();
        adaptor_stored = true;
    }

    std::shared_ptr<GDoubleGaussAdaptor> gdga_ptr(new GDoubleGaussAdaptor(0.025, 0.1, 0., 1., 1.0));
    gdga_ptr->setAdaptionThreshold(
        0
    ); // Make sure the adaptor's internal parameters don't change through the adaption
    gdga_ptr->setAdaptionMode(adaptionMode::ALWAYS); // Always adapt
    this->addAdaptor(gdga_ptr);

    // Call the parent class'es function
    GConstrainedFPT<double>::specificTestsNoFailureExpected_GUnitTests_();

    // --------------------------------------------------------------------------

    { // Check that assignment of a value with operator= works both for set and unset boundaries
        std::shared_ptr<GConstrainedDoubleObject> p_test =
            this->clone<GConstrainedDoubleObject>();

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
        std::shared_ptr<GConstrainedDoubleObject> p_test(new GConstrainedDoubleObject(0.3, 0.6));
        CHECK_NOTHROW(test_val3 = p_test->value());
    }

    // --------------------------------------------------------------------------

    { // Check construction with two boundaries and a value and extraction of that value
        constexpr double testval = 0.4;
        std::shared_ptr<GConstrainedDoubleObject> p_test(
            new GConstrainedDoubleObject(0.4, 0.3, 0.6)
        );
        CHECK_NOTHROW(test_val3 = p_test->value());
        CHECK(test_val3 == testval);
    }

    // --------------------------------------------------------------------------

    { // Check that repeated retrieval of the value always yields the same value
        constexpr double testval = 0.4;
        std::shared_ptr<GConstrainedDoubleObject> p_test(
            new GConstrainedDoubleObject(0.4, 0.3, 0.6)
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
        "GConstrainedDoubleObject::specificTestsNoFailureExpected_GUnitTests",
        "GEM_TESTING"
    );
#endif                  /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GConstrainedDoubleObject::specificTestsFailuresExpected_GUnitTests_() {
#ifdef GEM_TESTING
    // Make sure we have an appropriate adaptor loaded when performing these tests
    bool adaptor_stored = false;
    std::unique_ptr<adaptor_base_t> stored_adaptor;

    if(this->hasAdaptor()) {
        stored_adaptor = this->getAdaptor().clone_unique();
        adaptor_stored = true;
    }

    std::shared_ptr<GDoubleGaussAdaptor> gdga_ptr(new GDoubleGaussAdaptor(0.025, 0.1, 0., 1., 1.0));
    gdga_ptr->setAdaptionThreshold(
        0
    ); // Make sure the adaptor's internal parameters don't change through the adaption
    gdga_ptr->setAdaptionMode(adaptionMode::ALWAYS); // Always adapt
    this->addAdaptor(gdga_ptr);

    // Call the parent class'es function
    GConstrainedFPT<double>::specificTestsFailuresExpected_GUnitTests_();

    // Remove the test adaptor
    this->resetAdaptor();

    // Load the old adaptor, if needed
    if(adaptor_stored) {
        this->addAdaptor(Gem::Common::nonOwningShared(stored_adaptor));
    }

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset(
        "GConstrainedDoubleObject::specificTestsFailuresExpected_GUnitTests",
        "GEM_TESTING"
    );
#endif                  /* GEM_TESTING */
}

/******************************************************************************/

} /* namespace Gem::Geneva::Parameters */
