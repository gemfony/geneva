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

#include "geneva/ind/GIndividualSlot.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::Genome::GIndividualSlot) // NOLINT

namespace Gem::Geneva::Genome {

/******************************************************************************/
/**
 * Loads the data of another GIndividualSlot. The wrapped individual is deep-copied through the
 * cloneable-member protocol (clone if the concrete genome type differs, load in place otherwise); the
 * scratch is deep-copied by GAuxiliaryStore's copy assignment (which deep-clones the personality and
 * copies the POD blocks).
 *
 * @param cp A copy of another GIndividualSlot object, camouflaged as a GIndividualSlot
 */
void GIndividualSlot::load_(const GIndividualSlot *cp) {
    // Convert the pointer to our target type and check for self-assignment
    const GIndividualSlot *p_load =
        Gem::Common::g_convert_and_compare<GIndividualSlot, GIndividualSlot>(cp, this);

    // The CRTP base carries no data, so there is no base load.

    // The wrapped individual, derived from the single localMembers() declaration.
    Gem::Common::g_load_members(this->localMembers(), p_load->localMembers());

    // The OA-owned scratch (personality + POD blocks). Not part of localMembers(), copied explicitly.
    scratch_ = p_load->scratch_;
}

/******************************************************************************/
/**
 * Searches for compliance with expectations with respect to another object of the same type. Only the
 * wrapped individual is compared: the scratch is OA-installed and deliberately kept out of the compared
 * identity (two slots holding equal individuals but touched by different algorithms compare equal).
 *
 * @param cp A constant reference to another GIndividualSlot object
 * @param e The expected outcome of the comparison
 * @param limit The maximum deviation for floating point values (unused here)
 */
void GIndividualSlot::compare_(
    GIndividualSlot const &cp,
    Gem::Common::expectation const &e,
    [[maybe_unused]] double const &limit
) const {
    using namespace Gem::Common;

    // Check that we are dealing with a GIndividualSlot reference independent of this object and convert
    const GIndividualSlot *p_load =
        Gem::Common::g_convert_and_compare<GIndividualSlot, GIndividualSlot>(cp, this);

    GToken token("GIndividualSlot", e);

    // Compare our CRTP base data (the common-interface root carries no state) ...
    Gem::Common::compare_base_t<Gem::Common::GCommonInterfaceT<GIndividualSlot>>(*this, *p_load, token);

    // ... and the wrapped individual (the scratch is intentionally NOT compared).
    Gem::Common::g_compare_members(localMembers(), p_load->localMembers(), token);

    // React on deviations from the expectation
    token.evaluate();
}

/******************************************************************************/
/**
 * Emits a name for this class / object
 */
std::string GIndividualSlot::name_() const {
    return std::string("GIndividualSlot");
}

/******************************************************************************/
/**
 * Creates a deep clone of this object
 */
GIndividualSlot *GIndividualSlot::clone_() const {
    return new GIndividualSlot(*this);
}

/******************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GIndividualSlot::modify_GUnitTests_() {
#ifdef GEM_TESTING
    bool result = false;

    // Mutate the wrapped individual, if any -- that is the slot's compared identity.
    if(individual_ && individual_->modify_GUnitTests()) {
        result = true;
    }

    return result;

#else  /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset("GIndividualSlot::modify_GUnitTests", "GEM_TESTING");
    return false;
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GIndividualSlot::specificTestsNoFailureExpected_GUnitTests_() {
#ifdef GEM_TESTING
    // Nothing slot-specific here; the wrapped individual carries its own tests.

#else  /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset(
        "GIndividualSlot::specificTestsNoFailureExpected_GUnitTests",
        "GEM_TESTING"
    );
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GIndividualSlot::specificTestsFailuresExpected_GUnitTests_() {
#ifdef GEM_TESTING
    // Nothing slot-specific here; the wrapped individual carries its own tests.

#else  /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset(
        "GIndividualSlot::specificTestsFailuresExpected_GUnitTests",
        "GEM_TESTING"
    );
#endif /* GEM_TESTING */
}

/******************************************************************************/

} /* namespace Gem::Geneva::Genome */
