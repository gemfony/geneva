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

#include "geneva/oa/GAdaptiveEvolutionaryAlgorithm_PersonalityTraits.hpp"
#include "common/GCommonInterfaceT.hpp"
#include "common/GExpectationChecksT.hpp"
#include "geneva/GPersonalityTraits.hpp"
#include "geneva/oa/GBaseParChildPersonalityTraits.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::OptimizationAlgorithms::GAdaptiveEvolutionaryAlgorithm_PersonalityTraits) // NOLINT

namespace Gem::Geneva::OptimizationAlgorithms {

/******************************************************************************/
/** A short identifier suitable for storage in a std::map */
const std::string GAdaptiveEvolutionaryAlgorithm_PersonalityTraits::nickname = "eaa"; // NOLINT

/******************************************************************************/
/**
 * @brief Searches for compliance with expectations with respect to another object
 * of the same type
 *
 * @param cp A constant reference to another GPersonalityTraits object to compare against
 * @param e The expected outcome of the comparison (e.g. equality or inequality)
 * @param limit The maximum allowed deviation for floating-point comparisons (unused here)
 */
void GAdaptiveEvolutionaryAlgorithm_PersonalityTraits::compare_(
    const GPersonalityTraits &cp,
    const Gem::Common::expectation &e,
    [[maybe_unused]] const double & limit
) const {
    using namespace Gem::Common;

    const GAdaptiveEvolutionaryAlgorithm_PersonalityTraits *p_load =
        Gem::Common::g_convert_and_compare<GPersonalityTraits, GAdaptiveEvolutionaryAlgorithm_PersonalityTraits>(
            cp,
            this
        );

    GToken token("GAdaptiveEvolutionaryAlgorithm_PersonalityTraits", e);

    // Compare our parent data ...
    Gem::Common::compare_base_t<GBaseParChildPersonalityTraits>(*this, *p_load, token);

    // ... and then the local data, derived from the single localMembers() declaration
    g_compare_members(localMembers(), p_load->localMembers(), token);

    // React on deviations from the expectation
    token.evaluate();
}

/******************************************************************************/
/**
 * @brief Emits a name for this class / object
 *
 * @return The string "GAdaptiveEvolutionaryAlgorithm_PersonalityTraits"
 */
std::string GAdaptiveEvolutionaryAlgorithm_PersonalityTraits::name_() const {
    return std::string("GAdaptiveEvolutionaryAlgorithm_PersonalityTraits");
}

/******************************************************************************/
/**
 * @brief Retrieves the mnemonic of the optimization algorithm
 *
 * @return The short identifier ("eaa") for the adaptive evolutionary algorithm
 */
std::string GAdaptiveEvolutionaryAlgorithm_PersonalityTraits::getMnemonic() const {
    return GAdaptiveEvolutionaryAlgorithm_PersonalityTraits::nickname;
}

/******************************************************************************/
/**
 * @brief Creates a deep clone of this object
 *
 * @return A clone of this object, camouflaged as a GPersonalityTraits
 */
GPersonalityTraits *GAdaptiveEvolutionaryAlgorithm_PersonalityTraits::clone_() const {
    return new GAdaptiveEvolutionaryAlgorithm_PersonalityTraits(*this);
}

/******************************************************************************/
/**
 * @brief Loads the data of another GAdaptiveEvolutionaryAlgorithm_PersonalityTraits object
 *
 * @param cp A pointer to another such object, camouflaged as a GPersonalityTraits
 */
void GAdaptiveEvolutionaryAlgorithm_PersonalityTraits::load_(const GPersonalityTraits *cp) {
    const GAdaptiveEvolutionaryAlgorithm_PersonalityTraits *p_load =
        Gem::Common::g_convert_and_compare<GPersonalityTraits, GAdaptiveEvolutionaryAlgorithm_PersonalityTraits>(
            cp,
            this
        );

    // Load the parent class'es data
    GBaseParChildPersonalityTraits::load_(cp);

    // Then load our local data, derived from the single localMembers() declaration
    Gem::Common::g_load_members(localMembers(), p_load->localMembers());
}

/******************************************************************************/
/**
 * @brief Allows to check whether this individual lies on the pareto front
 *
 * @return A boolean indicating whether this object lies on the current pareto front
 */
bool GAdaptiveEvolutionaryAlgorithm_PersonalityTraits::isOnParetoFront() const {
    return is_on_pareto_front_;
}

/******************************************************************************/
/**
 * @brief Allows to reset the pareto tag to "true"
 */
void GAdaptiveEvolutionaryAlgorithm_PersonalityTraits::resetParetoTag() {
    is_on_pareto_front_ = true;
}

/******************************************************************************/
/**
 * @brief Allows to specify that this individual does not lie on the pareto front
 * of the current iteration
 */
void GAdaptiveEvolutionaryAlgorithm_PersonalityTraits::setIsNotOnParetoFront() {
    is_on_pareto_front_ = false;
}

/******************************************************************************/
/**
 * @brief Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GAdaptiveEvolutionaryAlgorithm_PersonalityTraits::modify_GUnitTests_() {
#ifdef GEM_TESTING
    bool result = false;

    // Call the parent class'es function
    if(GBaseParChildPersonalityTraits::modify_GUnitTests_()) {
        result = true;
    }

    return result;
#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset(
        "GAdaptiveEvolutionaryAlgorithm_PersonalityTraits::modify_GUnitTests",
        "GEM_TESTING"
    );
    return false;
#endif                  /* GEM_TESTING */
}

/******************************************************************************/
/**
 * @brief Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GAdaptiveEvolutionaryAlgorithm_PersonalityTraits::specificTestsNoFailureExpected_GUnitTests_() {
#ifdef GEM_TESTING

    // Call the parent class'es function
    GBaseParChildPersonalityTraits::specificTestsNoFailureExpected_GUnitTests_();

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset(
        "GAdaptiveEvolutionaryAlgorithm_PersonalityTraits::specificTestsNoFailureExpected_GUnitTests",
        "GEM_TESTING"
    );
#endif                  /* GEM_TESTING */
}

/******************************************************************************/
/**
 * @brief Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GAdaptiveEvolutionaryAlgorithm_PersonalityTraits::specificTestsFailuresExpected_GUnitTests_() {
#ifdef GEM_TESTING

    // Call the parent class'es function
    GBaseParChildPersonalityTraits::specificTestsFailuresExpected_GUnitTests_();

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset(
        "GAdaptiveEvolutionaryAlgorithm_PersonalityTraits::specificTestsFailuresExpected_GUnitTests",
        "GEM_TESTING"
    );
#endif                  /* GEM_TESTING */
}

/******************************************************************************/

} /* namespace Gem::Geneva::OptimizationAlgorithms */
