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

#include "geneva/oa/GSepCmaEvolutionStrategy_PersonalityTraits.hpp"

#include <string>

#ifdef GEM_TESTING
#include <catch2/catch_test_macros.hpp>
#endif /* GEM_TESTING */

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::OptimizationAlgorithms::GSepCmaEvolutionStrategy_PersonalityTraits) // NOLINT

namespace Gem::Geneva::OptimizationAlgorithms {

/******************************************************************************/
/** A short identifier suitable for storage in a std::map -- the command-line mnemonic. */
const std::string GSepCmaEvolutionStrategy_PersonalityTraits::nickname = "sepcma"; // NOLINT

/******************************************************************************/
/**
 * @brief Sets the offspring's selection rank in the current generation (0 == best)
 *
 * @param rank The selection rank to store
 */
void GSepCmaEvolutionStrategy_PersonalityTraits::setRank(std::size_t rank) {
    rank_ = rank;
}

/******************************************************************************/
/**
 * @brief Retrieves the offspring's selection rank in the current generation
 *
 * @return The stored selection rank (0 == best)
 */
std::size_t GSepCmaEvolutionStrategy_PersonalityTraits::getRank() const {
    return rank_;
}

/******************************************************************************/
/**
 * @brief Allows to check whether this individual lies on the current pareto front
 *
 * @return A boolean indicating whether this object lies on the current pareto front
 */
bool GSepCmaEvolutionStrategy_PersonalityTraits::isOnParetoFront() const {
    return is_on_pareto_front_;
}

/******************************************************************************/
/**
 * @brief Allows to reset the pareto tag to "true"
 */
void GSepCmaEvolutionStrategy_PersonalityTraits::resetParetoTag() {
    is_on_pareto_front_ = true;
}

/******************************************************************************/
/**
 * @brief Allows to specify that this individual does not lie on the pareto front
 */
void GSepCmaEvolutionStrategy_PersonalityTraits::setIsNotOnParetoFront() {
    is_on_pareto_front_ = false;
}

/******************************************************************************/
/**
 * @brief Retrieves the mnemonic of the optimization algorithm
 *
 * @return The mnemonic ("sepcma") associated with this personality
 */
std::string GSepCmaEvolutionStrategy_PersonalityTraits::getMnemonic() const {
    return GSepCmaEvolutionStrategy_PersonalityTraits::nickname;
}

/******************************************************************************/
/**
 * @brief Searches for compliance with expectations with respect to another object of the same type
 *
 * @param cp A constant reference to another GPersonalityTraits object to compare against
 * @param e The expected outcome of the comparison (e.g. equality or inequality)
 * @param limit The maximum allowed deviation for floating-point comparisons (unused here)
 */
void GSepCmaEvolutionStrategy_PersonalityTraits::compare_(
    const GPersonalityTraits &cp,
    const Gem::Common::expectation &e,
    [[maybe_unused]] const double &limit
) const {
    using namespace Gem::Common;

    const GSepCmaEvolutionStrategy_PersonalityTraits *p_load =
        Gem::Common::g_convert_and_compare<GPersonalityTraits, GSepCmaEvolutionStrategy_PersonalityTraits>(
            cp,
            this
        );

    GToken token("GSepCmaEvolutionStrategy_PersonalityTraits", e);

    // Compare our parent data ...
    Gem::Common::compare_base_t<GPersonalityTraits>(*this, *p_load, token);

    // ... and then the local data, derived from the single localMembers() declaration
    g_compare_members(this->localMembers_(), p_load->localMembers_(), token);

    token.evaluate();
}

/******************************************************************************/
/**
 * @brief Emits a name for this class / object
 *
 * @return The string "GSepCmaEvolutionStrategy_PersonalityTraits"
 */
std::string GSepCmaEvolutionStrategy_PersonalityTraits::name_() const {
    return std::string("GSepCmaEvolutionStrategy_PersonalityTraits");
}

/******************************************************************************/
/**
 * @brief Creates a deep clone of this object
 *
 * @return A clone of this object, camouflaged as a GPersonalityTraits
 */
GPersonalityTraits *GSepCmaEvolutionStrategy_PersonalityTraits::clone_() const {
    return new GSepCmaEvolutionStrategy_PersonalityTraits(*this);
}

/******************************************************************************/
/**
 * @brief Loads the data of another GSepCmaEvolutionStrategy_PersonalityTraits object
 *
 * @param cp A pointer to another object of this type, camouflaged as a GPersonalityTraits
 */
void GSepCmaEvolutionStrategy_PersonalityTraits::load_(const GPersonalityTraits *cp) {
    const GSepCmaEvolutionStrategy_PersonalityTraits *p_load =
        Gem::Common::g_convert_and_compare<GPersonalityTraits, GSepCmaEvolutionStrategy_PersonalityTraits>(
            cp,
            this
        );

    // Load the parent class'es data
    GPersonalityTraits::load_(cp);

    // Then load our local data, derived from the single localMembers() declaration
    Gem::Common::g_load_members(this->localMembers_(), p_load->localMembers_());
}

/******************************************************************************/
/**
 * @brief Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GSepCmaEvolutionStrategy_PersonalityTraits::modify_GUnitTests_() {
#ifdef GEM_TESTING
    bool result = false;

    if(GPersonalityTraits::modify_GUnitTests_()) {
        result = true;
    }

    this->setRank(this->getRank() + 1);
    result = true;

    return result;
#else /* GEM_TESTING */
    Gem::Common::condnotset(
        "GSepCmaEvolutionStrategy_PersonalityTraits::modify_GUnitTests",
        "GEM_TESTING"
    );
    return false;
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * @brief Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GSepCmaEvolutionStrategy_PersonalityTraits::specificTestsNoFailureExpected_GUnitTests_() {
#ifdef GEM_TESTING
    GPersonalityTraits::specificTestsNoFailureExpected_GUnitTests_();

    {
        std::shared_ptr<GSepCmaEvolutionStrategy_PersonalityTraits> p_test =
            this->clone<GSepCmaEvolutionStrategy_PersonalityTraits>();

        CHECK_NOTHROW(p_test->setRank(42));
        CHECK(p_test->getRank() == 42);

        p_test->resetParetoTag();
        CHECK(p_test->isOnParetoFront());
        p_test->setIsNotOnParetoFront();
        CHECK(not p_test->isOnParetoFront());
    }
#else /* GEM_TESTING */
    Gem::Common::condnotset(
        "GSepCmaEvolutionStrategy_PersonalityTraits::specificTestsNoFailureExpected_GUnitTests",
        "GEM_TESTING"
    );
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * @brief Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GSepCmaEvolutionStrategy_PersonalityTraits::specificTestsFailuresExpected_GUnitTests_() {
#ifdef GEM_TESTING
    GPersonalityTraits::specificTestsFailuresExpected_GUnitTests_();
#else /* GEM_TESTING */
    Gem::Common::condnotset(
        "GSepCmaEvolutionStrategy_PersonalityTraits::specificTestsFailuresExpected_GUnitTests",
        "GEM_TESTING"
    );
#endif /* GEM_TESTING */
}

/******************************************************************************/

} /* namespace Gem::Geneva::OptimizationAlgorithms */
