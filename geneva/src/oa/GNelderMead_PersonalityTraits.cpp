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
#include "geneva/oa/GNelderMead_PersonalityTraits.hpp"
#include "common/GCommonInterfaceT.hpp"
#include "common/GExpectationChecksT.hpp"
#include "geneva/GPersonalityTraits.hpp"
#include <cstddef>
#include <string>

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::OptimizationAlgorithms::GNelderMead_PersonalityTraits) // NOLINT
namespace Gem::Geneva::OptimizationAlgorithms {

/******************************************************************************/
/** A short identifier suitable for storage in a std::map */
const std::string GNelderMead_PersonalityTraits::nickname = "nm";

/******************************************************************************/
/**
 * @brief Searches for compliance with expectations with respect to another object
 * of the same type.
 *
 * @param cp A constant reference to another GPersonalityTraits object (must actually be a
 *           GNelderMead_PersonalityTraits) to compare against
 * @param e The expected outcome of the comparison (equality, inequality, etc.)
 * @param limit The maximum acceptable deviation for floating-point comparisons (unused here)
 */
void GNelderMead_PersonalityTraits::compare_(
    const GPersonalityTraits &cp,
    const Gem::Common::expectation &e,
    [[maybe_unused]] const double & limit
) const {
    using namespace Gem::Common;

    const GNelderMead_PersonalityTraits *p_load =
        Gem::Common::g_convert_and_compare<GPersonalityTraits, GNelderMead_PersonalityTraits>(cp, this);

    GToken token("GNelderMead_PersonalityTraits", e);

    // Compare our parent data ...
    Gem::Common::compare_base_t<GPersonalityTraits>(*this, *p_load, token);

    // ... and then the local data
    g_compare_members(this->localMembers_(), p_load->localMembers_(), token);

    // React on deviations from the expectation
    token.evaluate();
}

/***********************************************************************************/
/**
 * @brief Emits a name for this class / object.
 *
 * @return The string "GNelderMead_PersonalityTraits"
 */
std::string GNelderMead_PersonalityTraits::name_() const {
    return std::string("GNelderMead_PersonalityTraits");
}

/******************************************************************************/
/**
 * @brief Retrieves the mnemonic of the optimization algorithm.
 *
 * @return The short identifier ("nm") of the Nelder-Mead algorithm
 */
std::string GNelderMead_PersonalityTraits::getMnemonic() const {
    return GNelderMead_PersonalityTraits::nickname;
}

/******************************************************************************/
/**
 * @brief Creates a deep clone of this object.
 *
 * @return A pointer to a newly allocated, deep copy of this object (caller takes ownership)
 */
GPersonalityTraits *GNelderMead_PersonalityTraits::clone_() const {
    return new GNelderMead_PersonalityTraits(*this);
}

/******************************************************************************/
/**
 * @brief Loads the data of another GNelderMead_PersonalityTraits object.
 *
 * @param cp A pointer to another GPersonalityTraits object (must actually be a
 *           GNelderMead_PersonalityTraits) whose data is copied into this object
 */
void GNelderMead_PersonalityTraits::load_(const GPersonalityTraits *cp) {
    const GNelderMead_PersonalityTraits *p_load =
        Gem::Common::g_convert_and_compare<GPersonalityTraits, GNelderMead_PersonalityTraits>(cp, this);

    // Load the parent class'es data
    GPersonalityTraits::load_(cp);

    // and then the local data, derived from the single localMembers() declaration
    Gem::Common::g_load_members(this->localMembers_(), p_load->localMembers_());
}

/******************************************************************************/
/**
 * @brief Sets the position of the individual in the population.
 *
 * @param pop_pos The index of the individual within the population
 */
void GNelderMead_PersonalityTraits::setPopulationPosition(const std::size_t &pop_pos) {
    pop_pos_ = pop_pos;
}

/******************************************************************************/
/**
 * @brief Retrieves the position of the individual in the population.
 *
 * @return The index of the individual within the population
 */
std::size_t GNelderMead_PersonalityTraits::getPopulationPosition() const {
    return pop_pos_;
}

/******************************************************************************/
/**
 * @brief Applies modifications to this object. This is needed for testing purposes.
 *
 * @return true if the object was modified, false otherwise (or when GEM_TESTING is disabled)
 */
bool GNelderMead_PersonalityTraits::modify_GUnitTests_() {
#ifdef GEM_TESTING
    bool result = false;

    if(GPersonalityTraits::modify_GUnitTests_()) {
        result = true;
    }

    this->setPopulationPosition(this->getPopulationPosition() + 1);
    result = true;

    return result;

#else /* GEM_TESTING */
    Gem::Common::condnotset("GNelderMead_PersonalityTraits::modify_GUnitTests", "GEM_TESTING");
    return false;
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * @brief Performs self tests that are expected to succeed. This is needed for testing purposes.
 */
void GNelderMead_PersonalityTraits::specificTestsNoFailureExpected_GUnitTests_() {
#ifdef GEM_TESTING

    GPersonalityTraits::specificTestsNoFailureExpected_GUnitTests_();

#else /* GEM_TESTING */
    Gem::Common::condnotset(
        "GNelderMead_PersonalityTraits::specificTestsNoFailureExpected_GUnitTests",
        "GEM_TESTING"
    );
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * @brief Performs self tests that are expected to fail. This is needed for testing purposes.
 */
void GNelderMead_PersonalityTraits::specificTestsFailuresExpected_GUnitTests_() {
#ifdef GEM_TESTING

    GPersonalityTraits::specificTestsFailuresExpected_GUnitTests_();

#else /* GEM_TESTING */
    Gem::Common::condnotset(
        "GNelderMead_PersonalityTraits::specificTestsFailuresExpected_GUnitTests",
        "GEM_TESTING"
    );
#endif /* GEM_TESTING */
}

/******************************************************************************/

} /* namespace Gem::Geneva::OptimizationAlgorithms */
