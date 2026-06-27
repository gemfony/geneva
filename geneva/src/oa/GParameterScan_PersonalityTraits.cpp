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

#include "geneva/oa/GParameterScan_PersonalityTraits.hpp"
#include "common/GCommonInterfaceT.hpp"
#include "common/GExpectationChecksT.hpp"
#include "geneva/GPersonalityTraits.hpp"
#include <cstddef>
#include <string>

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::OptimizationAlgorithms::GParameterScan_PersonalityTraits) // NOLINT
namespace Gem::Geneva::OptimizationAlgorithms {

/******************************************************************************/
/** A short identifier suitable for storage in a std::map */
const std::string GParameterScan_PersonalityTraits::nickname = "ps";

/******************************************************************************/
/**
 * @brief Searches for compliance with expectations with respect to another object
 * of the same type
 *
 * @param cp A constant reference to another GPersonalityTraits object, expected to be a GParameterScan_PersonalityTraits
 * @param e The expected outcome of the comparison (e.g. equality or inequality)
 * @param limit The maximum allowed deviation for floating point values (unused here, no local FP members)
 */
void GParameterScan_PersonalityTraits::compare_(
    const GPersonalityTraits &cp,
    const Gem::Common::expectation &e,
    [[maybe_unused]] const double & limit
) const {
    using namespace Gem::Common;

    // Check that we are dealing with a GParameterScan_PersonalityTraits reference independent of this object and convert the pointer
    const GParameterScan_PersonalityTraits *p_load =
        Gem::Common::g_convert_and_compare<GPersonalityTraits, GParameterScan_PersonalityTraits>(cp, this);

    GToken token("GParameterScan_PersonalityTraits", e);

    // Compare our parent data ...
    Gem::Common::compare_base_t<GPersonalityTraits>(*this, *p_load, token);

    // ... and then the local data
    g_compare_members(localMembers_(*this), localMembers_(*p_load), token);

    // React on deviations from the expectation
    token.evaluate();
}

/***********************************************************************************/
/**
 * @brief Emits a name for this class / object
 *
 * @return The string "GParameterScan_PersonalityTraits"
 */
std::string GParameterScan_PersonalityTraits::name_() const {
    return std::string("GParameterScan_PersonalityTraits");
}

/******************************************************************************/
/**
 * @brief Retrieves the mnemonic of the optimization algorithm
 *
 * @return The short identifier of the parameter-scan personality ("ps")
 */
std::string GParameterScan_PersonalityTraits::getMnemonic() const {
    return GParameterScan_PersonalityTraits::nickname;
}

/******************************************************************************/
/**
 * @brief Creates a deep clone of this object
 *
 * @return A newly allocated clone of this object, camouflaged as a GPersonalityTraits (ownership passes to the caller)
 */
GPersonalityTraits *GParameterScan_PersonalityTraits::clone_() const {
    return new GParameterScan_PersonalityTraits(*this);
}

/******************************************************************************/
/**
 * @brief Loads the data of another GParameterScan_PersonalityTraits object
 *
 * @param cp A pointer to another GParameterScan_PersonalityTraits object, camouflaged as a GPersonalityTraits
 */
void GParameterScan_PersonalityTraits::load_(const GPersonalityTraits *cp) {
    // Check that we are dealing with a GParameterScan_PersonalityTraits reference independent of this object and convert the pointer
    const GParameterScan_PersonalityTraits *p_load =
        Gem::Common::g_convert_and_compare<GPersonalityTraits, GParameterScan_PersonalityTraits>(cp, this);

    // Load the parent class'es data
    GPersonalityTraits::load_(cp);

    // and then the local data, derived from the single localMembers() declaration
    Gem::Common::g_load_members(localMembers_(*this), localMembers_(*p_load));
}

/******************************************************************************/
/**
 * @brief Sets the position of the individual in the population
 *
 * @param pop_pos The new zero-based position of this individual in the population
 */
void GParameterScan_PersonalityTraits::setPopulationPosition(const std::size_t &pop_pos) {
    pop_pos_ = pop_pos;
}

/* ----------------------------------------------------------------------------------
 * So far untested
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * @brief Retrieves the position of the individual in the population
 *
 * @return The current zero-based position of this individual in the population
 */
std::size_t GParameterScan_PersonalityTraits::getPopulationPosition() const {
    return pop_pos_;
}

/* ----------------------------------------------------------------------------------
 * So far untested
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * @brief Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GParameterScan_PersonalityTraits::modify_GUnitTests_() {
#ifdef GEM_TESTING
    bool result = false;

    // Call the parent class'es function
    if(GPersonalityTraits::modify_GUnitTests_()) {
        result = true;
    }

    this->setPopulationPosition(this->getPopulationPosition() + 1);
    result = true;

    return result;

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset("GParameterScan_PersonalityTraits::modify_GUnitTests", "GEM_TESTING");
    return false;
#endif                  /* GEM_TESTING */
}

/******************************************************************************/
/**
 * @brief Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GParameterScan_PersonalityTraits::specificTestsNoFailureExpected_GUnitTests_() {
#ifdef GEM_TESTING

    // Call the parent class'es function
    GPersonalityTraits::specificTestsNoFailureExpected_GUnitTests_();

    // --------------------------------------------------------------------------

    // --------------------------------------------------------------------------

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset(
        "GParameterScan_PersonalityTraits::specificTestsNoFailureExpected_GUnitTests",
        "GEM_TESTING"
    );
#endif                  /* GEM_TESTING */
}

/******************************************************************************/
/**
 * @brief Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GParameterScan_PersonalityTraits::specificTestsFailuresExpected_GUnitTests_() {
#ifdef GEM_TESTING

    // Call the parent class'es function
    GPersonalityTraits::specificTestsFailuresExpected_GUnitTests_();

    // --------------------------------------------------------------------------

    // --------------------------------------------------------------------------

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset(
        "GParameterScan_PersonalityTraits::specificTestsFailuresExpected_GUnitTests",
        "GEM_TESTING"
    );
#endif                  /* GEM_TESTING */
}

/******************************************************************************/

} /* namespace Gem::Geneva::OptimizationAlgorithms */
