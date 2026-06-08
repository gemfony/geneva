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

#include "geneva/par/GConstrainedInt32Object.hpp"
#include "common/GCommonHelperFunctionsT.hpp"
#include "common/GCommonMathHelperFunctionsT.hpp"
#include "common/GExceptions.hpp"
#include "common/GExpectationChecksT.hpp"
#include "common/GLogger.hpp"
#include "geneva/GOptimizationEnums.hpp"
#include "geneva/par/GAdaptorT.hpp"
#include "geneva/par/GConstrainedIntT.hpp"
#include "geneva/par/GInt32GaussAdaptor.hpp"
#include "geneva/par/GParameterBase.hpp"
#include "geneva/par/GParameterT.hpp"
#include "hap/GRandomBase.hpp"
#include <cstddef>
#include <cstdint>
#include <map>
#include <memory>
#include <random>
#include <vector>

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::Parameters::GConstrainedInt32Object) // NOLINT
namespace Gem::Geneva::Parameters {

/******************************************************************************/
/**
 * Initialization with boundaries only. A random value inside of the allowed ranges will
 * be assigned to the object.
 *
 * @param lower_boundary The lower boundary of the value range
 * @param upper_boundary The upper boundary of the value range
 */
GConstrainedInt32Object::GConstrainedInt32Object(
    const std::int32_t &lower_boundary,
    const std::int32_t &upper_boundary
)
  : GConstrainedIntT<std::int32_t>(lower_boundary, upper_boundary) { /* nothing */
}

/******************************************************************************/
/**
 * Initialization with value and boundaries
 *
 * @param val Initialization value
 * @param lower_boundary The lower boundary of the value range
 * @param upper_boundary The upper boundary of the value range
 */
GConstrainedInt32Object::GConstrainedInt32Object(
    const std::int32_t &val,
    const std::int32_t &lower_boundary,
    const std::int32_t &upper_boundary
)
  : GConstrainedIntT<std::int32_t>(val, lower_boundary, upper_boundary) { /* nothing */
}

/******************************************************************************/
/**
 * Initialization by contained value
 *
 * @param val A value used for the initialization
 */
GConstrainedInt32Object::GConstrainedInt32Object(const std::int32_t &val)
  : GConstrainedIntT<std::int32_t>(val) { /* nothing */
}

/******************************************************************************/
/**
 * An assignment operator for the contained value type
 *
 * @param val The value to be assigned to this object
 * @return The value that was just assigned to this object
 */
GConstrainedInt32Object &GConstrainedInt32Object::operator=(const std::int32_t &val) {
    GConstrainedIntT<std::int32_t>::operator=(val);
    return *this;
}

/******************************************************************************/
/**
 * Creates a deep clone of this object.
 *
 * @return A copy of this object, camouflaged as a GParameterBase
 */
GParameterBase *GConstrainedInt32Object::clone_() const {
    return new GConstrainedInt32Object(*this);
}

/******************************************************************************/
/**
 * Searches for compliance with expectations with respect to another object
 * of the same type
 *
 * @param cp A constant reference to another GParameterBase object
 * @param e The expected outcome of the comparison
 */
void GConstrainedInt32Object::compare_(
    const GParameterBase &cp,
    const Gem::Common::expectation &e,
    [[maybe_unused]] const double & limit
) const {
    using namespace Gem::Common;

    // Check that we are dealing with a GConstrainedInt32Object reference independent of this object and convert the pointer
    const GConstrainedInt32Object *p_load =
        Gem::Common::g_convert_and_compare<GParameterBase, GConstrainedInt32Object>(cp, this);

    GToken token("GConstrainedInt32Object", e);

    // Compare our parent data ...
    Gem::Common::compare_base_t<GConstrainedIntT<std::int32_t>>(*this, *p_load, token);

    // .... no local data

    // React on deviations from the expectation
    token.evaluate();
}

/***********************************************************************************/
/**
 * Emits a name for this class / object
 */
std::string GConstrainedInt32Object::name_() const {
    return std::string("GConstrainedInt32Object");
}

/******************************************************************************/
/**
 * Attach our local value to the vector. This is used to collect all parameters of this type
 * in the sequence in which they were registered.
 */
void GConstrainedInt32Object::int32Streamline(
    std::vector<std::int32_t> &par_vec,
    [[maybe_unused]] const activityMode & am
) const {
    par_vec.push_back(this->value());
}

/******************************************************************************/

/******************************************************************************/
/**
 * Attach boundaries of type std::int32_t to the vectors.
 */
void GConstrainedInt32Object::int32Boundaries(
    std::vector<std::int32_t> &l_bnd_vec,
    std::vector<std::int32_t> &u_bnd_vec,
    [[maybe_unused]] const activityMode & am
) const {
    l_bnd_vec.push_back(this->getLowerBoundary());
    u_bnd_vec.push_back(this->getUpperBoundary());
}

/******************************************************************************/
/**
 * Tell the audience that we own a std::int32_t value
 *
 * @return The number 1, as we own a single std::int32_t parameter
 */
std::size_t GConstrainedInt32Object::countInt32Parameters(
    [[maybe_unused]] const activityMode & am
) const {
    return 1;
}

/******************************************************************************/
/**
 * Assigns part of a value vector to the parameter. Note that we apply a transformation
 * to the assigned value, so that it lies inside of the allowed value range.
 */
void GConstrainedInt32Object::assignInt32ValueVector(
    const std::vector<std::int32_t> &par_vec,
    std::size_t &pos,
    [[maybe_unused]] const activityMode & am
) {
#ifdef DEBUG
    // Do we have a valid position ?
    if(pos >= par_vec.size()) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GConstrainedInt32Object::assignInt32ValueVector(const "
               "std::vector<std::int32_t>&, std::size_t&):"
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
void GConstrainedInt32Object::int32MultiplyByRandom(
    const std::int32_t &min,
    const std::int32_t &max,
    [[maybe_unused]] const activityMode & am
    ,
    Gem::Hap::GRandomBase &gr // NOLINT(misc-unused-parameters)
) {
    std::uniform_int_distribution<std::int32_t> uniform_int_distribution(min, max);
    GParameterT<std::int32_t>::setValue(
        transfer(GParameterT<std::int32_t>::value() * uniform_int_distribution(gr))
    );
}

/******************************************************************************/
/**
 * Multiplication with a random DOUBLE value in the range [0,1[
 */
void GConstrainedInt32Object::int32MultiplyByRandom(
    [[maybe_unused]] const activityMode & am
    ,
    Gem::Hap::GRandomBase &gr // NOLINT(misc-unused-parameters)
) {
    std::uniform_real_distribution<double> uniform_real_distribution(0., 1.);
    GParameterT<std::int32_t>::setValue(transfer(
        Gem::Common::narrow<std::int32_t>(
            Gem::Common::narrow<double>(GParameterT<std::int32_t>::value()) *
            uniform_real_distribution(gr)
        )
    ));
}

/******************************************************************************/
/**
 * Multiplication with a constant value
 */
void GConstrainedInt32Object::int32MultiplyBy(const std::int32_t &val, const activityMode &am) {
    GParameterT<std::int32_t>::setValue(transfer(val * GParameterT<std::int32_t>::value()));
}

/******************************************************************************/
/**
 * Initialization with a constant value
 */
void GConstrainedInt32Object::int32FixedValueInit(const std::int32_t &val, const activityMode &am) {
    GParameterT<std::int32_t>::setValue(transfer(val));
}

/******************************************************************************/
/**
 * Adds the "same-type" parameters of another GParameterBase object to this one
 */
void GConstrainedInt32Object::int32Add(
    std::shared_ptr<GParameterBase> p_base,
    [[maybe_unused]] const activityMode & am
) {
    // We first need to convert p_base into the local type
    std::shared_ptr<GConstrainedInt32Object> p =
        GParameterBase::parameterbase_cast<GConstrainedInt32Object>(p_base);
    GParameterT<std::int32_t>::setValue(transfer(this->value() + p->value()));
}

/******************************************************************************/
/**
 * Adds the "same-type" parameters of another GParameterBase object to this one
 */
void GConstrainedInt32Object::int32Subtract(
    std::shared_ptr<GParameterBase> p_base,
    [[maybe_unused]] const activityMode & am
) {
    // We first need to convert p_base into the local type
    std::shared_ptr<GConstrainedInt32Object> p =
        GParameterBase::parameterbase_cast<GConstrainedInt32Object>(p_base);
    GParameterT<std::int32_t>::setValue(transfer(this->value() - p->value()));
}

/******************************************************************************/
/**
 * Loads the data of another GParameterBase
 *
 * @param cp A copy of another GConstrainedInt32Object object, camouflaged as a GParameterBase
 */
void GConstrainedInt32Object::load_(const GParameterBase *cp) {
    // Convert the pointer to our target type and check for self-assignment
    const GConstrainedInt32Object *p_load =
        Gem::Common::g_convert_and_compare<GParameterBase, GConstrainedInt32Object>(cp, this);

    // Load our parent class'es data ...
    GConstrainedIntT<std::int32_t>::load_(cp);

    // ... no local data
}

/******************************************************************************/
/**
 * Triggers random initialization of the parameter object
 */
bool GConstrainedInt32Object::randomInit_(const activityMode &am, Gem::Hap::GRandomBase &gr) {
    return GConstrainedIntT<std::int32_t>::randomInit_(am, gr);
}

/******************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GConstrainedInt32Object::modify_GUnitTests_() {
#ifdef GEM_TESTING
    bool result = false;

    // Call the parent class'es function
    if(GConstrainedIntT<std::int32_t>::modify_GUnitTests_()) {
        result = true;
    }

    if(this->value() == this->getLowerBoundary()) {
        this->setValue(this->getLowerBoundary() + 1);
    }
    else {
        this->setValue(this->getLowerBoundary());
    }
    result = true;

    return result;

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset("GConstrainedInt32Object::modify_GUnitTests", "GEM_TESTING");
    return false;
#endif                  /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GConstrainedInt32Object::specificTestsNoFailureExpected_GUnitTests_() {
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
    GConstrainedIntT<std::int32_t>::specificTestsNoFailureExpected_GUnitTests_();

    // Remove the test adaptor
    this->resetAdaptor();

    // Load the old adaptor, if needed
    if(adaptor_stored) {
        this->addAdaptor(Gem::Common::nonOwningShared(stored_adaptor));
    }

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset(
        "GConstrainedInt32Object::specificTestsNoFailureExpected_GUnitTests",
        "GEM_TESTING"
    );
#endif                  /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GConstrainedInt32Object::specificTestsFailuresExpected_GUnitTests_() {
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
    GConstrainedIntT<std::int32_t>::specificTestsFailuresExpected_GUnitTests_();

    // Remove the test adaptor
    this->resetAdaptor();

    // Load the old adaptor, if needed
    if(adaptor_stored) {
        this->addAdaptor(Gem::Common::nonOwningShared(stored_adaptor));
    }

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset(
        "GConstrainedInt32Object::specificTestsFailuresExpected_GUnitTests",
        "GEM_TESTING"
    );
#endif                  /* GEM_TESTING */
}

/******************************************************************************/

} /* namespace Gem::Geneva::Parameters */
