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
#include "geneva/oa/GConjugateGradientDescent_PersonalityTraits.hpp"
#include "common/GCommonInterfaceT.hpp"
#include "common/GExpectationChecksT.hpp"
#include "geneva/GPersonalityTraits.hpp"
#include <cstddef>
#include <string>

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::OptimizationAlgorithms::GConjugateGradientDescent_PersonalityTraits) // NOLINT
namespace Gem::Geneva::OptimizationAlgorithms {

/******************************************************************************/
/** @brief A short identifier suitable for storage in a std::map (the algorithm mnemonic "cgd") */
const std::string GConjugateGradientDescent_PersonalityTraits::nickname = "cgd";

/******************************************************************************/
/**
 * @brief Searches for compliance with expectations with respect to another object
 * of the same type
 *
 * @param cp A constant reference to another GPersonalityTraits object to compare against
 * @param e The expected outcome of the comparison (e.g. equality or inequality)
 * @param limit The maximum allowed deviation for floating point comparisons (unused here, as this class holds no floating point members)
 */
void GConjugateGradientDescent_PersonalityTraits::compare_(
    const GPersonalityTraits &cp,
    const Gem::Common::expectation &e,
    [[maybe_unused]] const double & limit
) const {
    using namespace Gem::Common;

    // Check that we are dealing with a GConjugateGradientDescent_PersonalityTraits
    // reference independent of this object and convert the pointer
    const GConjugateGradientDescent_PersonalityTraits *p_load =
        Gem::Common::g_convert_and_compare<GPersonalityTraits, GConjugateGradientDescent_PersonalityTraits>(
            cp,
            this
        );

    GToken token("GConjugateGradientDescent_PersonalityTraits", e);

    // Compare our parent data ...
    Gem::Common::compare_base_t<GPersonalityTraits>(*this, *p_load, token);

    // ... and then the local data, derived from the single localMembers() declaration
    g_compare_members(localMembers(), p_load->localMembers(), token);

    // React on deviations from the expectation
    token.evaluate();
}

/***********************************************************************************/
/**
 * @brief Emits a name for this class / object
 *
 * @return The string "GConjugateGradientDescent_PersonalityTraits"
 */
std::string GConjugateGradientDescent_PersonalityTraits::name_() const {
    return std::string("GConjugateGradientDescent_PersonalityTraits");
}

/******************************************************************************/
/**
 * @brief Retrieves the mnemonic of the optimization algorithm
 *
 * @return The short identifier of this algorithm (the nickname "cgd")
 */
std::string GConjugateGradientDescent_PersonalityTraits::getMnemonic() const {
    return GConjugateGradientDescent_PersonalityTraits::nickname;
}

/******************************************************************************/
/**
 * @brief Creates a deep clone of this object
 *
 * @return A newly allocated deep copy of this object, camouflaged as a GPersonalityTraits pointer
 */
GPersonalityTraits *GConjugateGradientDescent_PersonalityTraits::clone_() const {
    return new GConjugateGradientDescent_PersonalityTraits(*this);
}

/******************************************************************************/
/**
 * @brief Loads the data of another GConjugateGradientDescent_PersonalityTraits object
 *
 * @param cp A pointer to another GConjugateGradientDescent_PersonalityTraits object, camouflaged as a GPersonalityTraits
 */
void GConjugateGradientDescent_PersonalityTraits::load_(const GPersonalityTraits *cp) {
    // Check that we are dealing with a GConjugateGradientDescent_PersonalityTraits
    // reference independent of this object and convert the pointer
    const GConjugateGradientDescent_PersonalityTraits *p_load =
        Gem::Common::g_convert_and_compare<GPersonalityTraits, GConjugateGradientDescent_PersonalityTraits>(
            cp,
            this
        );

    // Load the parent class'es data
    GPersonalityTraits::load_(cp);

    // and then the local data, derived from the single localMembers() declaration
    Gem::Common::g_load_members(localMembers(), p_load->localMembers());
}

/******************************************************************************/
/**
 * @brief Sets the position of the individual in the population
 *
 * @param pop_pos The new position (index) of this individual in the population
 */
void GConjugateGradientDescent_PersonalityTraits::setPopulationPosition(
    const std::size_t &pop_pos
) {
    pop_pos_ = pop_pos;
}

/******************************************************************************/
/**
 * @brief Retrieves the position of the individual in the population
 *
 * @return The current position (index) of this individual in the population
 */
std::size_t GConjugateGradientDescent_PersonalityTraits::getPopulationPosition() const {
    return pop_pos_;
}

/******************************************************************************/
/**
 * @brief Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GConjugateGradientDescent_PersonalityTraits::modify_GUnitTests_() {
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
    Gem::Common::condnotset(
        "GConjugateGradientDescent_PersonalityTraits::modify_GUnitTests",
        "GEM_TESTING"
    );
    return false;
#endif                  /* GEM_TESTING */
}

/******************************************************************************/
/**
 * @brief Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GConjugateGradientDescent_PersonalityTraits::specificTestsNoFailureExpected_GUnitTests_() {
#ifdef GEM_TESTING

    // Call the parent class'es function
    GPersonalityTraits::specificTestsNoFailureExpected_GUnitTests_();

    // --------------------------------------------------------------------------

    // --------------------------------------------------------------------------

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset(
        "GConjugateGradientDescent_PersonalityTraits::specificTestsNoFailureExpected_GUnitTests",
        "GEM_TESTING"
    );
#endif                  /* GEM_TESTING */
}

/******************************************************************************/
/**
 * @brief Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GConjugateGradientDescent_PersonalityTraits::specificTestsFailuresExpected_GUnitTests_() {
#ifdef GEM_TESTING

    // Call the parent class'es function
    GPersonalityTraits::specificTestsFailuresExpected_GUnitTests_();

    // --------------------------------------------------------------------------

    // --------------------------------------------------------------------------

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset(
        "GConjugateGradientDescent_PersonalityTraits::specificTestsFailuresExpected_GUnitTests",
        "GEM_TESTING"
    );
#endif                  /* GEM_TESTING */
}

/******************************************************************************/

} /* namespace Gem::Geneva::OptimizationAlgorithms */
