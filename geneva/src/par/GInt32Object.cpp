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

#include "geneva/par/GInt32Object.hpp"
#include "common/GCommonHelperFunctionsT.hpp"
#include "common/GCommonMathHelperFunctionsT.hpp"
#include "common/GExceptions.hpp"
#include "common/GExpectationChecksT.hpp"
#include "common/GLogger.hpp"
#include "geneva/GOptimizationEnums.hpp"
#include "geneva/par/GAdaptorT.hpp"
#include "geneva/par/GInt32FlipAdaptor.hpp"
#include "geneva/par/GInt32GaussAdaptor.hpp"
#include "geneva/par/GNumIntT.hpp"
#include "geneva/par/GParameterBase.hpp"
#include "geneva/par/GParameterT.hpp"
#include "hap/GRandomBase.hpp"
#include <cstddef>
#include <cstdint>
#include <map>
#include <memory>
#include <random>
#include <vector>

#ifdef GEM_TESTING
#include <catch2/catch_test_macros.hpp>
#endif /* GEM_TESTING */

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::Parameters::GInt32Object) // NOLINT
namespace Gem::Geneva::Parameters {

/******************************************************************************/
/**
 * Initialization by contained value
 *
 * @param val A value used for the initialization
 */
GInt32Object::GInt32Object(const std::int32_t &val)
  : GNumIntT<std::int32_t>(val) { /* nothing */
}

/******************************************************************************/
/**
 * Initialization by random number in a given range
 *
 * @param lower_boundary The lower boundary for the random number used in the initialization
 * @param upper_boundary The upper boundary for the random number used in the initialization
 */
GInt32Object::GInt32Object(const std::int32_t &lower_boundary, const std::int32_t &upper_boundary)
  : GNumIntT<std::int32_t>(lower_boundary, upper_boundary) { /* nothing */
}

/******************************************************************************/
/**
 * Initialization by a fixed value, plus the boundaries for random initialization. Note
 * that we do not enforce val to be inside of the initialization boundaries
 *
 * @param val The value to be assigned to the object
 * @param lower_boundary The lower boundary for the random number used in the initialization
 * @param upper_boundary The upper boundary for the random number used in the initialization
 */
GInt32Object::GInt32Object(
    const std::int32_t &val,
    const std::int32_t &lower_boundary,
    const std::int32_t &upper_boundary
)
  : GNumIntT<std::int32_t>(val, lower_boundary, upper_boundary) { /* nothing */
}

/******************************************************************************/
/**
 * An assignment operator
 *
 * @param val The value to be assigned to this object
 * @return The value that was just assigned to this object
 */
GInt32Object &GInt32Object::operator=(const std::int32_t &val) {
    GNumIntT<std::int32_t>::operator=(val);
    return *this;
}

/******************************************************************************/
/**
 * Creates a deep clone of this object.
 *
 * @return A copy of this object, camouflaged as a GParameterBase
 */
GParameterBase *GInt32Object::clone_() const {
    return new GInt32Object(*this);
}

/******************************************************************************/
/**
 * Searches for compliance with expectations with respect to another object
 * of the same type
 *
 * @param cp A constant reference to another GParameterBase object
 * @param e The expected outcome of the comparison
 */
void GInt32Object::compare_(
    const GParameterBase &cp,
    const Gem::Common::expectation &e,
    [[maybe_unused]] const double & limit
) const {
    using namespace Gem::Common;

    // Check that we are dealing with a GInt32Object reference independent of this object and convert the pointer
    const GInt32Object *p_load =
        Gem::Common::g_convert_and_compare<GParameterBase, GInt32Object>(cp, this);

    GToken token("GInt32Object", e);

    // Compare our parent data ...
    Gem::Common::compare_base_t<GNumIntT<std::int32_t>>(*this, *p_load, token);

    // ... no local data

    // React on deviations from the expectation
    token.evaluate();
}

/***********************************************************************************/
/**
 * Emits a name for this class / object
 */
std::string GInt32Object::name_() const {
    return std::string("GInt32Object");
}

/******************************************************************************/
/**
 * Attach our local value to the vector. This is used to collect all parameters of this type
 * in the sequence in which they were registered.
 */
void GInt32Object::int32Streamline(
    std::vector<std::int32_t> &par_vec,
    [[maybe_unused]] const activityMode & am
) const {
    par_vec.push_back(this->value());
}

/******************************************************************************/

/******************************************************************************/
/**
 * Attach boundaries of type std::int32_t to the vectors. Since this is an unbounded type,
 * we use the initialization boundaries as a replacement.
 */
void GInt32Object::int32Boundaries(
    std::vector<std::int32_t> &l_bnd_vec,
    std::vector<std::int32_t> &u_bnd_vec,
    [[maybe_unused]] const activityMode & am
) const {
    l_bnd_vec.push_back(this->getLowerInitBoundary());
    u_bnd_vec.push_back(this->getUpperInitBoundary());
}

/******************************************************************************/
/**
 * Tell the audience that we own a std::int32_t value
 *
 * @return The number 1, as we own a single std::int32_t parameter
 */
std::size_t GInt32Object::countInt32Parameters(
    [[maybe_unused]] const activityMode & am
) const {
    return 1;
}

/******************************************************************************/
/**
 * Assigns part of a value vector to the parameter
 */
void GInt32Object::assignInt32ValueVector(
    const std::vector<std::int32_t> &par_vec,
    std::size_t &pos,
    [[maybe_unused]] const activityMode & am
) {
#ifdef DEBUG
    // Do we have a valid position ?
    if(pos >= par_vec.size()) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GBooleanObject::assignInt32ValueVector(const std::vector<std::int32_t>&, "
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

/******************************************************************************/
/**
 * Multiplication with a random value in a given range
 */
void GInt32Object::int32MultiplyByRandom(
    const std::int32_t &min,
    const std::int32_t &max,
    [[maybe_unused]] const activityMode & am
    ,
    Gem::Hap::GRandomBase &gr // NOLINT(misc-unused-parameters)
) {
    std::uniform_int_distribution<std::int32_t> uniform_int_distribution(min, max);
    GParameterT<std::int32_t>::setValue(
        GParameterT<std::int32_t>::value() * uniform_int_distribution(gr)
    );
}

/******************************************************************************/
/**
 * Multiplication with a random DOUBLE value in the range [0,1[
 */
void GInt32Object::int32MultiplyByRandom(
    [[maybe_unused]] const activityMode & am
    ,
    Gem::Hap::GRandomBase &gr // NOLINT(misc-unused-parameters)
) {
    std::uniform_real_distribution<double> uniform_real_distribution(0., 1.);
    GParameterT<std::int32_t>::setValue(
        Gem::Common::narrow<std::int32_t>(
            Gem::Common::narrow<double>(GParameterT<std::int32_t>::value()) *
            uniform_real_distribution(gr)
        )
    );
}

/******************************************************************************/
/**
 * Multiplication with a constant value
 */
void GInt32Object::int32MultiplyBy(const std::int32_t &val, const activityMode &am) {
    GParameterT<std::int32_t>::setValue(val * GParameterT<std::int32_t>::value());
}

/******************************************************************************/
/**
 * Initialization with a constant value
 */
void GInt32Object::int32FixedValueInit(const std::int32_t &val, const activityMode &am) {
    GParameterT<std::int32_t>::setValue(val);
}

/******************************************************************************/
/**
 * Adds the "same-type" parameters of another GParameterBase object to this one
 */
void GInt32Object::int32Add(
    std::shared_ptr<GParameterBase> p_base,
    [[maybe_unused]] const activityMode & am
) {
    // We first need to convert p_base into the local type
    std::shared_ptr<GInt32Object> p = GParameterBase::parameterbase_cast<GInt32Object>(p_base);
    GParameterT<std::int32_t>::setValue(this->value() + p->value());
}

/******************************************************************************/
/**
 * Adds the "same-type" parameters of another GParameterBase object to this one
 */
void GInt32Object::int32Subtract(
    std::shared_ptr<GParameterBase> p_base,
    [[maybe_unused]] const activityMode & am
) {
    // We first need to convert p_base into the local type
    std::shared_ptr<GInt32Object> p = GParameterBase::parameterbase_cast<GInt32Object>(p_base);
    GParameterT<std::int32_t>::setValue(this->value() - p->value());
}

/******************************************************************************/
/**
 * Loads the data of another GParameterBase
 *
 * @param cp A copy of another GInt32Object object, camouflaged as a GParameterBase
 */
void GInt32Object::load_(const GParameterBase *cp) {
    // Convert the pointer to our target type and check for self-assignment
    const GInt32Object *p_load =
        Gem::Common::g_convert_and_compare<GParameterBase, GInt32Object>(cp, this);

    // Load our parent class'es data ...
    GNumIntT<std::int32_t>::load_(cp);

    // ... no local data
}

/******************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GInt32Object::modify_GUnitTests_() {
#ifdef GEM_TESTING

    bool result = false;

    // Call the parent class'es function
    if(GNumIntT<std::int32_t>::modify_GUnitTests_()) {
        result = true;
    }

    this->setValue(this->value() + 1);
    result = true;

    return result;

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset("GInt32Object::modify_GUnitTests", "GEM_TESTING");
    return false;
#endif                  /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GInt32Object::specificTestsNoFailureExpected_GUnitTests_() {
#ifdef GEM_TESTING

    // A few settings
    constexpr std::size_t n_tests = 10000;
    constexpr std::int32_t lowerinitboundary = -10;
    constexpr std::int32_t upperinitboundary = 10;
    constexpr std::int32_t fixedvalueinit = 1;

    // Make sure we have an appropriate adaptor loaded when performing these tests
    bool adaptor_stored = false;
    std::unique_ptr<adaptor_base_t> stored_adaptor;

    if(this->hasAdaptor()) {
        stored_adaptor = this->getAdaptor().clone_unique();
        adaptor_stored = true;
    }

    std::shared_ptr<GInt32GaussAdaptor> giga_ptr(new GInt32GaussAdaptor(0.025, 0.1, 0., 0.5, 1.0));
    giga_ptr->setAdaptionThreshold(
        0
    ); // Make sure the adaptor's internal parameters don't change through the adaption
    giga_ptr->setAdaptionMode(adaptionMode::ALWAYS); // Always adapt
    this->addAdaptor(giga_ptr);

    // Call the parent class'es function
    GNumIntT<std::int32_t>::specificTestsNoFailureExpected_GUnitTests_();

    // --------------------------------------------------------------------------

    { // Test different ways of adding an adaptor (Test of GParameterBaseWithAdaptorsT<T> functions)
        std::shared_ptr<GInt32Object> p_test = this->clone<GInt32Object>();

        // Make sure we start in pristine condition. This will add a GInt32FlipAdaptor.
        CHECK_NOTHROW(p_test->resetAdaptor());

        //********************************
        // Adding an adaptor of different type present should clone the adaptor
        CHECK_NOTHROW(p_test->addAdaptor(giga_ptr));

        // Check that the addresses of both adaptors differ
        GInt32GaussAdaptor* giga_clone_ptr = nullptr;
        CHECK_NOTHROW(giga_clone_ptr = &p_test->getAdaptor<GInt32GaussAdaptor>());
        CHECK(giga_clone_ptr != giga_ptr.get());

        //********************************
        // Adding an adaptor when an adaptor of the same type is present should leave the original address intact

        // Make a note of the stored adaptor's address
        GInt32GaussAdaptor *ptr_store = giga_clone_ptr;

        // Add the "global" adaptor again, should be load()-ed
        CHECK_NOTHROW(p_test->addAdaptor(giga_ptr));

        // Retrieve the adaptor again
        GInt32GaussAdaptor* giga_clone2_ptr = nullptr;
        CHECK_NOTHROW(giga_clone2_ptr = &p_test->getAdaptor<GInt32GaussAdaptor>());

        // Check that the address hasn't changed
        CHECK(ptr_store == giga_clone2_ptr);

        //********************************
    }

    // Reset to the original state
    this->resetAdaptor();

    // Load the old adaptor, if needed
    if(adaptor_stored) {
        this->addAdaptor(Gem::Common::nonOwningShared(stored_adaptor));
    }

    // --------------------------------------------------------------------------

    { // Check that construction with initialization boundaries leads to random content

        std::int32_t previous = -1;
        for(std::size_t i = 0; i < 10; i++) {
            GInt32Object p(0, 10000000);
            CHECK(p.value() != previous);
            previous = p.value();
        }
    }

    // --------------------------------------------------------------------------

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset(
        "GInt32Object::specificTestsNoFailureExpected_GUnitTests",
        "GEM_TESTING"
    );
#endif                  /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GInt32Object::specificTestsFailuresExpected_GUnitTests_() {
#ifdef GEM_TESTING

    // Make sure we have an appropriate adaptor loaded when performing these tests
    bool adaptor_stored = false;
    std::unique_ptr<adaptor_base_t> stored_adaptor;

    if(this->hasAdaptor()) {
        stored_adaptor = this->getAdaptor().clone_unique();
        adaptor_stored = true;
    }

    std::shared_ptr<GInt32GaussAdaptor> giga_ptr(new GInt32GaussAdaptor(0.025, 0.1, 0., 1., 1.0));
    giga_ptr->setAdaptionThreshold(
        0
    ); // Make sure the adaptor's internal parameters don't change through the adaption
    giga_ptr->setAdaptionMode(adaptionMode::ALWAYS); // Always adapt
    this->addAdaptor(giga_ptr);

    // Call the parent class'es function
    GNumIntT<std::int32_t>::specificTestsFailuresExpected_GUnitTests_();

    // --------------------------------------------------------------------------

#ifdef DEBUG
    { // Check that retrieval of the adaptor with simultaneous conversion to an incorrect target type throws (Test of GParameterBaseWithAdaptorsT<T> functions)
        std::shared_ptr<GInt32Object> p_test = this->clone<GInt32Object>();

        // Make sure an adaptor is present
        REQUIRE(p_test->hasAdaptor() == true);

        // Make sure the local adaptor has the type we expect
        CHECK(p_test->getAdaptor()->getAdaptorId() == adaptorId::GINT32GAUSSADAPTOR);

        // Attempted conversion to an invalid target type should throw
        CHECK_THROWS_AS((p_test->getAdaptor<GInt32FlipAdaptor>()), geneva_exception);
    }
#endif /* DEBUG */

    // --------------------------------------------------------------------------

    // Load the old adaptor, if needed
    if(adaptor_stored) {
        this->addAdaptor(Gem::Common::nonOwningShared(stored_adaptor));
    }

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset(
        "GInt32Object::specificTestsFailuresExpected_GUnitTests",
        "GEM_TESTING"
    );
#endif                  /* GEM_TESTING */
}

/******************************************************************************/

} /* namespace Gem::Geneva::Parameters */
