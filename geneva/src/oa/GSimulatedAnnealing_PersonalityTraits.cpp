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

#include "geneva/oa/GSimulatedAnnealing_PersonalityTraits.hpp"
#include "common/GExpectationChecksT.hpp"
#include "geneva/GPersonalityTraits.hpp"
#include "geneva/oa/GBaseParChildPersonalityTraits.hpp"
#include <string>

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::OptimizationAlgorithms::GSimulatedAnnealing_PersonalityTraits) // NOLINT
namespace Gem::Geneva::OptimizationAlgorithms {

/******************************************************************************/
/** @brief A short identifier ("sa") suitable for storage in a std::map */
const std::string GSimulatedAnnealing_PersonalityTraits::nickname = "sa";

/******************************************************************************/
/**
 * @brief Searches for compliance with expectations with respect to another object
 * of the same type.
 *
 * @param cp A constant reference to another GPersonalityTraits object to compare against
 * @param e The expected outcome of the comparison (e.g. equality or inequality)
 * @param limit The maximum acceptable deviation for floating-point comparisons (unused here, as this class has no local data)
 */
void GSimulatedAnnealing_PersonalityTraits::compare_(
    const GPersonalityTraits &cp,
    const Gem::Common::expectation &e,
    [[maybe_unused]] const double & limit
) const {
    using namespace Gem::Common;

    // Check that we are dealing with a GSimulatedAnnealing_PersonalityTraits reference independent of this object and convert the pointer
    const GSimulatedAnnealing_PersonalityTraits *p_load =
        Gem::Common::g_convert_and_compare<GPersonalityTraits, GSimulatedAnnealing_PersonalityTraits>(
            cp,
            this
        );

    GToken token("GSimulatedAnnealing_PersonalityTraits", e);

    // Compare our parent data ...
    Gem::Common::compare_base_t<GBaseParChildPersonalityTraits>(*this, *p_load, token);

    // ... no local data

    // React on deviations from the expectation
    token.evaluate();
}

/******************************************************************************/
/**
 * @brief Emits a name for this class / object.
 *
 * @return The class name "GSimulatedAnnealing_PersonalityTraits" as a std::string
 */
std::string GSimulatedAnnealing_PersonalityTraits::name_() const {
    return std::string("GSimulatedAnnealing_PersonalityTraits");
}

/******************************************************************************/
/**
 * @brief Retrieves the mnemonic of the optimization algorithm.
 *
 * @return The short identifier ("sa") of the simulated-annealing algorithm
 */
std::string GSimulatedAnnealing_PersonalityTraits::getMnemonic() const {
    return GSimulatedAnnealing_PersonalityTraits::nickname;
}

/******************************************************************************/
/**
 * @brief Creates a deep clone of this object.
 *
 * @return A clone of this object, camouflaged as a GPersonalityTraits
 */
GPersonalityTraits *GSimulatedAnnealing_PersonalityTraits::clone_() const {
    return new GSimulatedAnnealing_PersonalityTraits(*this);
}

/******************************************************************************/
/**
 * @brief Loads the data of another GSimulatedAnnealing_PersonalityTraits object.
 *
 * @param cp A pointer to another GSimulatedAnnealing_PersonalityTraits object, camouflaged as a GPersonalityTraits
 */
void GSimulatedAnnealing_PersonalityTraits::load_(const GPersonalityTraits *cp) {
    // Check that we are dealing with a GSimulatedAnnealing_PersonalityTraits reference independent of this object and convert the pointer
    const GSimulatedAnnealing_PersonalityTraits *p_load =
        Gem::Common::g_convert_and_compare<GPersonalityTraits, GSimulatedAnnealing_PersonalityTraits>(
            cp,
            this
        );

    // Load the parent class'es data
    GBaseParChildPersonalityTraits::load_(cp);

    // Then load our local data
    // no local data ...
}

/******************************************************************************/
/**
 * @brief Applies modifications to this object. This is needed for testing purposes.
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GSimulatedAnnealing_PersonalityTraits::modify_GUnitTests_() {
#ifdef GEM_TESTING
    bool result = false;

    // Call the parent class'es function
    if(GBaseParChildPersonalityTraits::modify_GUnitTests_()) {
        result = true;
    }

    return result;
#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset(
        "GSimulatedAnnealing_PersonalityTraits::modify_GUnitTests",
        "GEM_TESTING"
    );
    return false;
#endif                  /* GEM_TESTING */
}

/******************************************************************************/
/**
 * @brief Performs self tests that are expected to succeed. This is needed for testing purposes.
 */
void GSimulatedAnnealing_PersonalityTraits::specificTestsNoFailureExpected_GUnitTests_() {
#ifdef GEM_TESTING

    // Call the parent class'es function
    GBaseParChildPersonalityTraits::specificTestsNoFailureExpected_GUnitTests_();

    // --------------------------------------------------------------------------
    // --------------------------------------------------------------------------

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset(
        "GSimulatedAnnealing_PersonalityTraits::specificTestsNoFailureExpected_GUnitTests",
        "GEM_TESTING"
    );
#endif                  /* GEM_TESTING */
}

/******************************************************************************/
/**
 * @brief Performs self tests that are expected to fail. This is needed for testing purposes.
 */
void GSimulatedAnnealing_PersonalityTraits::specificTestsFailuresExpected_GUnitTests_() {
#ifdef GEM_TESTING

    // Call the parent class'es function
    GBaseParChildPersonalityTraits::specificTestsFailuresExpected_GUnitTests_();

    // --------------------------------------------------------------------------
    // --------------------------------------------------------------------------

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset(
        "GSimulatedAnnealing_PersonalityTraits::specificTestsFailuresExpected_GUnitTests",
        "GEM_TESTING"
    );
#endif                  /* GEM_TESTING */
}

/******************************************************************************/

} /* namespace Gem::Geneva::OptimizationAlgorithms */
