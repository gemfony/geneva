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

#include "geneva/oa/GAntColonyOptimization_PersonalityTraits.hpp"

#include <string>

#ifdef GEM_TESTING
#include <catch2/catch_test_macros.hpp>
#endif /* GEM_TESTING */

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::OptimizationAlgorithms::GAntColonyOptimization_PersonalityTraits) // NOLINT

namespace Gem::Geneva::OptimizationAlgorithms {

/******************************************************************************/
/** A short identifier suitable for storage in a std::map -- the command-line mnemonic. */
const std::string GAntColonyOptimization_PersonalityTraits::nickname = "acor"; // NOLINT

/******************************************************************************/
/**
 * @brief Sets the index of the population slot this individual occupies
 *
 * @param population_position The slot index to store
 */
void GAntColonyOptimization_PersonalityTraits::setPopulationPosition(std::size_t population_position) {
    population_position_ = population_position;
}

/******************************************************************************/
/**
 * @brief Retrieves the index of the population slot this individual occupies
 *
 * @return The stored slot index
 */
std::size_t GAntColonyOptimization_PersonalityTraits::getPopulationPosition() const {
    return population_position_;
}

/******************************************************************************/
/**
 * @brief Retrieves the mnemonic of the optimization algorithm
 *
 * @return The mnemonic ("acor") associated with this personality
 */
std::string GAntColonyOptimization_PersonalityTraits::getMnemonic() const {
    return GAntColonyOptimization_PersonalityTraits::nickname;
}

/******************************************************************************/
/**
 * @brief Searches for compliance with expectations with respect to another object of the same type
 *
 * @param cp A constant reference to another GPersonalityTraits object to compare against
 * @param e The expected outcome of the comparison (e.g. equality or inequality)
 * @param limit The maximum allowed deviation for floating-point comparisons (unused here)
 */
void GAntColonyOptimization_PersonalityTraits::compare_(
    const GPersonalityTraits &cp,
    const Gem::Common::expectation &e,
    [[maybe_unused]] const double &limit
) const {
    using namespace Gem::Common;

    const GAntColonyOptimization_PersonalityTraits *p_load =
        Gem::Common::g_convert_and_compare<GPersonalityTraits, GAntColonyOptimization_PersonalityTraits>(
            cp,
            this
        );

    GToken token("GAntColonyOptimization_PersonalityTraits", e);

    // Compare our parent data ...
    Gem::Common::compare_base_t<GPersonalityTraits>(*this, *p_load, token);

    // ... and then the local data, derived from the single localMembers() declaration
    g_compare_members(localMembers_(*this), localMembers_(*p_load), token);

    token.evaluate();
}

/******************************************************************************/
/**
 * @brief Emits a name for this class / object
 *
 * @return The string "GAntColonyOptimization_PersonalityTraits"
 */
std::string GAntColonyOptimization_PersonalityTraits::name_() const {
    return std::string("GAntColonyOptimization_PersonalityTraits");
}

/******************************************************************************/
/**
 * @brief Creates a deep clone of this object
 *
 * @return A clone of this object, camouflaged as a GPersonalityTraits
 */
GPersonalityTraits *GAntColonyOptimization_PersonalityTraits::clone_() const {
    return new GAntColonyOptimization_PersonalityTraits(*this);
}

/******************************************************************************/
/**
 * @brief Loads the data of another GAntColonyOptimization_PersonalityTraits object
 *
 * @param cp A pointer to another object of this type, camouflaged as a GPersonalityTraits
 */
void GAntColonyOptimization_PersonalityTraits::load_(const GPersonalityTraits *cp) {
    const GAntColonyOptimization_PersonalityTraits *p_load =
        Gem::Common::g_convert_and_compare<GPersonalityTraits, GAntColonyOptimization_PersonalityTraits>(
            cp,
            this
        );

    // Load the parent class'es data
    GPersonalityTraits::load_(cp);

    // Then load our local data, derived from the single localMembers() declaration
    Gem::Common::g_load_members(localMembers_(*this), localMembers_(*p_load));
}

/******************************************************************************/
/**
 * @brief Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GAntColonyOptimization_PersonalityTraits::modify_GUnitTests_() {
#ifdef GEM_TESTING
    bool result = false;

    if(GPersonalityTraits::modify_GUnitTests_()) {
        result = true;
    }

    this->setPopulationPosition(this->getPopulationPosition() + 1);
    result = true;

    return result;
#else /* GEM_TESTING */
    Gem::Common::condnotset(
        "GAntColonyOptimization_PersonalityTraits::modify_GUnitTests",
        "GEM_TESTING"
    );
    return false;
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * @brief Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GAntColonyOptimization_PersonalityTraits::specificTestsNoFailureExpected_GUnitTests_() {
#ifdef GEM_TESTING
    GPersonalityTraits::specificTestsNoFailureExpected_GUnitTests_();

    {
        std::shared_ptr<GAntColonyOptimization_PersonalityTraits> p_test =
            this->clone<GAntColonyOptimization_PersonalityTraits>();

        CHECK_NOTHROW(p_test->setPopulationPosition(42));
        CHECK(p_test->getPopulationPosition() == 42);
    }
#else /* GEM_TESTING */
    Gem::Common::condnotset(
        "GAntColonyOptimization_PersonalityTraits::specificTestsNoFailureExpected_GUnitTests",
        "GEM_TESTING"
    );
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * @brief Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GAntColonyOptimization_PersonalityTraits::specificTestsFailuresExpected_GUnitTests_() {
#ifdef GEM_TESTING
    GPersonalityTraits::specificTestsFailuresExpected_GUnitTests_();
#else /* GEM_TESTING */
    Gem::Common::condnotset(
        "GAntColonyOptimization_PersonalityTraits::specificTestsFailuresExpected_GUnitTests",
        "GEM_TESTING"
    );
#endif /* GEM_TESTING */
}

/******************************************************************************/

} /* namespace Gem::Geneva::OptimizationAlgorithms */
