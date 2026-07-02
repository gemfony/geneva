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

#include "geneva/oa/GEvolutionaryAlgorithm_PersonalityTraits.hpp"
#include "common/GCommonInterfaceT.hpp"
#include "common/GExpectationChecksT.hpp"
#include "geneva/GPersonalityTraits.hpp"
#include "geneva/oa/GBaseParChildPersonalityTraits.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::OptimizationAlgorithms::GEvolutionaryAlgorithm_PersonalityTraits) // NOLINT

namespace Gem::Geneva::OptimizationAlgorithms {

/******************************************************************************/
/** A short identifier suitable for storage in a std::map */
const std::string GEvolutionaryAlgorithm_PersonalityTraits::nickname = "ea"; // NOLINT

/******************************************************************************/
/**
 * @brief Searches for compliance with expectations with respect to another object
 * of the same type
 *
 * @param cp A constant reference to another GPersonalityTraits object to compare against
 * @param e The expected outcome of the comparison (e.g. equality or inequality)
 * @param limit The maximum allowed deviation for floating-point comparisons (unused here)
 */
void GEvolutionaryAlgorithm_PersonalityTraits::compare_(
    const GPersonalityTraits &cp,
    const Gem::Common::expectation &e,
    [[maybe_unused]] const double & limit
) const {
    using namespace Gem::Common;

    const GEvolutionaryAlgorithm_PersonalityTraits *p_load =
        Gem::Common::g_convert_and_compare<GPersonalityTraits, GEvolutionaryAlgorithm_PersonalityTraits>(
            cp,
            this
        );

    GToken token("GEvolutionaryAlgorithm_PersonalityTraits", e);

    // Compare our parent data ...
    Gem::Common::compare_base_t<GBaseParChildPersonalityTraits>(*this, *p_load, token);

    // ... and then the local data, derived from the single localMembers() declaration
    g_compare_members(this->localMembers_(), p_load->localMembers_(), token);

    // React on deviations from the expectation
    token.evaluate();
}

/******************************************************************************/
/**
 * @brief Emits a name for this class / object
 *
 * @return The string "GEvolutionaryAlgorithm_PersonalityTraits"
 */
std::string GEvolutionaryAlgorithm_PersonalityTraits::name_() const {
    return std::string("GEvolutionaryAlgorithm_PersonalityTraits");
}

/******************************************************************************/
/**
 * @brief Retrieves the mnemonic of the optimization algorithm
 *
 * @return The short identifier ("ea") for the adaptive evolutionary algorithm
 */
std::string GEvolutionaryAlgorithm_PersonalityTraits::getMnemonic() const {
    return GEvolutionaryAlgorithm_PersonalityTraits::nickname;
}

/******************************************************************************/
/**
 * @brief Creates a deep clone of this object
 *
 * @return A clone of this object, camouflaged as a GPersonalityTraits
 */
GPersonalityTraits *GEvolutionaryAlgorithm_PersonalityTraits::clone_() const {
    return new GEvolutionaryAlgorithm_PersonalityTraits(*this);
}

/******************************************************************************/
/**
 * @brief Loads the data of another GEvolutionaryAlgorithm_PersonalityTraits object
 *
 * @param cp A pointer to another such object, camouflaged as a GPersonalityTraits
 */
void GEvolutionaryAlgorithm_PersonalityTraits::load_(const GPersonalityTraits *cp) {
    const GEvolutionaryAlgorithm_PersonalityTraits *p_load =
        Gem::Common::g_convert_and_compare<GPersonalityTraits, GEvolutionaryAlgorithm_PersonalityTraits>(
            cp,
            this
        );

    // Load the parent class'es data
    GBaseParChildPersonalityTraits::load_(cp);

    // Then load our local data, derived from the single localMembers() declaration
    Gem::Common::g_load_members(this->localMembers_(), p_load->localMembers_());
}

/******************************************************************************/
/**
 * @brief Allows to check whether this individual lies on the pareto front
 *
 * @return A boolean indicating whether this object lies on the current pareto front
 */
bool GEvolutionaryAlgorithm_PersonalityTraits::isOnParetoFront() const {
    return is_on_pareto_front_;
}

/******************************************************************************/
/**
 * @brief Allows to reset the pareto tag to "true"
 */
void GEvolutionaryAlgorithm_PersonalityTraits::resetParetoTag() {
    is_on_pareto_front_ = true;
}

/******************************************************************************/
/**
 * @brief Allows to specify that this individual does not lie on the pareto front
 * of the current iteration
 */
void GEvolutionaryAlgorithm_PersonalityTraits::setIsNotOnParetoFront() {
    is_on_pareto_front_ = false;
}

/******************************************************************************/
/**
 * @brief Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GEvolutionaryAlgorithm_PersonalityTraits::modify_GUnitTests_() {
#ifdef GEM_TESTING
    bool result = false;

    // Call the parent class'es function
    if(GBaseParChildPersonalityTraits::modify_GUnitTests_()) {
        result = true;
    }

    return result;
#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset(
        "GEvolutionaryAlgorithm_PersonalityTraits::modify_GUnitTests",
        "GEM_TESTING"
    );
    return false;
#endif                  /* GEM_TESTING */
}

/******************************************************************************/
/**
 * @brief Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GEvolutionaryAlgorithm_PersonalityTraits::specificTestsNoFailureExpected_GUnitTests_() {
#ifdef GEM_TESTING

    // Call the parent class'es function
    GBaseParChildPersonalityTraits::specificTestsNoFailureExpected_GUnitTests_();

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset(
        "GEvolutionaryAlgorithm_PersonalityTraits::specificTestsNoFailureExpected_GUnitTests",
        "GEM_TESTING"
    );
#endif                  /* GEM_TESTING */
}

/******************************************************************************/
/**
 * @brief Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GEvolutionaryAlgorithm_PersonalityTraits::specificTestsFailuresExpected_GUnitTests_() {
#ifdef GEM_TESTING

    // Call the parent class'es function
    GBaseParChildPersonalityTraits::specificTestsFailuresExpected_GUnitTests_();

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset(
        "GEvolutionaryAlgorithm_PersonalityTraits::specificTestsFailuresExpected_GUnitTests",
        "GEM_TESTING"
    );
#endif                  /* GEM_TESTING */
}

/******************************************************************************/

} /* namespace Gem::Geneva::OptimizationAlgorithms */
