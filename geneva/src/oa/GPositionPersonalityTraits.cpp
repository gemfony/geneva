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
#include "geneva/oa/GPositionPersonalityTraits.hpp"
#include "common/GCommonInterfaceT.hpp"
#include "common/GExpectationChecksT.hpp"
#include "geneva/GPersonalityTraits.hpp"
#include <typeinfo>

namespace Gem::Geneva::OptimizationAlgorithms {

namespace {

/**
 * @brief Enforces that two traits objects are of the SAME concrete type. The shared position base
 * must not let one algorithm's traits load from / compare against another's merely because both
 * derive it (before the fold, each concrete class's own conversion check provided this guarantee).
 *
 * @param self The object whose member function was invoked
 * @param other The object handed in for loading / comparison
 * @param where The calling function's identity, for the error text
 */
void requireSameConcreteType(
    const GPersonalityTraits &self,
    const GPersonalityTraits &other,
    const char *where
) {
    if(typeid(self) != typeid(other)) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In " << where << ": Error!" << '\n'
            << "An object of type \"" << typeid(other).name() << "\" was handed to an object of"
            << " different type \"" << typeid(self).name() << "\"." << '\n'
        );
    }
}

} // namespace

/******************************************************************************/
/**
 * @brief Searches for compliance with expectations with respect to another object of the SAME
 * concrete traits type.
 *
 * @param cp A constant reference to another GPersonalityTraits object (must be of this object's
 *           exact concrete type) to compare against
 * @param e The expected outcome of the comparison (equality, inequality, etc.)
 * @param limit The maximum acceptable deviation for floating-point comparisons (unused here)
 */
void GPositionPersonalityTraits::compare_(
    const GPersonalityTraits &cp,
    const Gem::Common::expectation &e,
    [[maybe_unused]] const double & limit
) const {
    using namespace Gem::Common;

    requireSameConcreteType(*this, cp, "GPositionPersonalityTraits::compare_()");
    const GPositionPersonalityTraits *p_load =
        Gem::Common::g_convert_and_compare<GPersonalityTraits, GPositionPersonalityTraits>(cp, this);

    // The token carries the CONCRETE class name (name() is virtual), so comparison diagnostics
    // read exactly as they did when each traits class implemented compare_ itself.
    GToken token(this->name(), e);

    // Compare our parent data ...
    Gem::Common::compare_base_t<GPersonalityTraits>(*this, *p_load, token);

    // ... and then the local data
    g_compare_members(this->localMembers_(), p_load->localMembers_(), token);

    // React on deviations from the expectation
    token.evaluate();
}

/******************************************************************************/
/**
 * @brief Loads the data of another object of the SAME concrete traits type.
 *
 * @param cp A pointer to another GPersonalityTraits object (must be of this object's exact
 *           concrete type) whose data is copied into this object
 */
void GPositionPersonalityTraits::load_(const GPersonalityTraits *cp) {
    requireSameConcreteType(*this, *cp, "GPositionPersonalityTraits::load_()");
    const GPositionPersonalityTraits *p_load =
        Gem::Common::g_convert_and_compare<GPersonalityTraits, GPositionPersonalityTraits>(cp, this);

    // Load the parent class'es data
    GPersonalityTraits::load_(cp);

    // and then the local data, derived from the single localMembers() declaration
    Gem::Common::g_load_members(this->localMembers_(), p_load->localMembers_());
}

/******************************************************************************/
/**
 * @brief Applies modifications to this object. This is needed for testing purposes.
 *
 * @return true if the object was modified, false otherwise (or when GEM_TESTING is disabled)
 */
bool GPositionPersonalityTraits::modify_GUnitTests_() {
#ifdef GEM_TESTING
    bool result = false;

    if(GPersonalityTraits::modify_GUnitTests_()) {
        result = true;
    }

    this->setPopulationPosition(this->getPopulationPosition() + 1);
    result = true;

    return result;

#else /* GEM_TESTING */
    Gem::Common::condnotset("GPositionPersonalityTraits::modify_GUnitTests", "GEM_TESTING");
    return false;
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * @brief Performs self tests that are expected to succeed. This is needed for testing purposes.
 */
void GPositionPersonalityTraits::specificTestsNoFailureExpected_GUnitTests_() {
#ifdef GEM_TESTING

    GPersonalityTraits::specificTestsNoFailureExpected_GUnitTests_();

#else /* GEM_TESTING */
    Gem::Common::condnotset(
        "GPositionPersonalityTraits::specificTestsNoFailureExpected_GUnitTests",
        "GEM_TESTING"
    );
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * @brief Performs self tests that are expected to fail. This is needed for testing purposes.
 */
void GPositionPersonalityTraits::specificTestsFailuresExpected_GUnitTests_() {
#ifdef GEM_TESTING

    GPersonalityTraits::specificTestsFailuresExpected_GUnitTests_();

#else /* GEM_TESTING */
    Gem::Common::condnotset(
        "GPositionPersonalityTraits::specificTestsFailuresExpected_GUnitTests",
        "GEM_TESTING"
    );
#endif /* GEM_TESTING */
}

/******************************************************************************/

} /* namespace Gem::Geneva::OptimizationAlgorithms */
