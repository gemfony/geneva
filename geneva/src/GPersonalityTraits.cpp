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

#include "geneva/GPersonalityTraits.hpp"
#include "common/GCommonInterfaceT.hpp"
#include "common/GExpectationChecksT.hpp"

namespace Gem::Geneva {

/******************************************************************************/
/**
 * @brief Searches for compliance with expectations with respect to another object
 * of the same type
 *
 * @param cp A constant reference to another GPersonalityTraits object to compare against
 * @param e The expected outcome of the comparison (e.g. equality or inequality)
 * @param limit The maximum allowed deviation for floating point comparisons (unused here, as this root has no local data)
 */
void GPersonalityTraits::compare_(
    const GPersonalityTraits &cp,
    const Gem::Common::expectation &e,
    [[maybe_unused]] const double & limit
) const {
    using namespace Gem::Common;

    // Check that we are dealing with a GPersonalityTraits reference independent of this object and convert the pointer
    const GPersonalityTraits *p_load =
        Gem::Common::g_convert_and_compare<GPersonalityTraits, GPersonalityTraits>(cp, this);

    GToken token("GPersonalityTraits", e);

    // Compare our parent data ...
    Gem::Common::compare_base_t<GCommonInterfaceT<GPersonalityTraits>>(*this, *p_load, token);

    // ... no local data

    // React on deviations from the expectation
    token.evaluate();
}

/******************************************************************************/
/**
 * @brief Emits a name for this class / object
 *
 * @return The string "GPersonalityTraits", identifying this class
 */
std::string GPersonalityTraits::name_() const {
    return std::string("GPersonalityTraits");
}

/******************************************************************************/
/**
 * @brief Loads the data of another GPersonalityTraits object
 *
 * @param cp A pointer to another GPersonalityTraits object whose data is loaded into this one
 */
void GPersonalityTraits::load_(const GPersonalityTraits *cp) {
    // Convert the pointer to our target type and check for self-assignment
    Gem::Common::g_convert_and_compare<GPersonalityTraits, GPersonalityTraits>(cp, this);

    // No parent class with data and no local data
}

/******************************************************************************/
/**
 * @brief Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made (always false here, as this root has no modifiable data)
 */
bool GPersonalityTraits::modify_GUnitTests_() {
#ifdef GEM_TESTING
    // This is the category root; there is no modifiable parent class and no
    // local data, so there is nothing we can do here in this function.

    return false;

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset("GPersonalityTraits::modify_GUnitTests", "GEM_TESTING");
    return false;
#endif                  /* GEM_TESTING */
}

/******************************************************************************/
/**
 * @brief Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GPersonalityTraits::specificTestsNoFailureExpected_GUnitTests_() {
#ifdef GEM_TESTING
    // This is the category root; no parent class and no local data -- nothing to test

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset(
        "GPersonalityTraits::specificTestsNoFailureExpected_GUnitTests",
        "GEM_TESTING"
    );
#endif                  /* GEM_TESTING */
}

/******************************************************************************/
/**
 * @brief Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GPersonalityTraits::specificTestsFailuresExpected_GUnitTests_() {
#ifdef GEM_TESTING
    // This is the category root; no parent class and no local data -- nothing to test

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset(
        "GPersonalityTraits::specificTestsFailuresExpected_GUnitTests",
        "GEM_TESTING"
    );
#endif                  /* GEM_TESTING */
}

/******************************************************************************/

} /* namespace Gem::Geneva */
