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
#include "geneva/G_OptimizationAlgorithm_NelderMead_PersonalityTraits.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GNelderMead_PersonalityTraits) // NOLINT
namespace Gem::Geneva {

/******************************************************************************/
/** A short identifier suitable for storage in a std::map */
const std::string GNelderMead_PersonalityTraits::nickname = "nm";

/******************************************************************************/
/**
 * Searches for compliance with expectations with respect to another object
 * of the same type
 */
void GNelderMead_PersonalityTraits::compare_(
    const GObject &cp,
    const Gem::Common::expectation &e,
    const double & /*limit*/
) const {
    using namespace Gem::Common;

    const GNelderMead_PersonalityTraits *p_load =
        Gem::Common::g_convert_and_compare<GObject, GNelderMead_PersonalityTraits>(cp, this);

    GToken token("GNelderMead_PersonalityTraits", e);

    // Compare our parent data ...
    Gem::Common::compare_base_t<GPersonalityTraits>(*this, *p_load, token);

    // ... and then the local data
    compare_t(IDENTITY(popPos_, p_load->popPos_), token);

    // React on deviations from the expectation
    token.evaluate();
}

/***********************************************************************************/
/**
 * Emits a name for this class / object
 */
std::string GNelderMead_PersonalityTraits::name_() const {
    return std::string("GNelderMead_PersonalityTraits");
}

/******************************************************************************/
/**
 * Retrieves the mnemonic of the optimization algorithm
 */
std::string GNelderMead_PersonalityTraits::getMnemonic() const {
    return GNelderMead_PersonalityTraits::nickname;
}

/******************************************************************************/
/**
 * Creates a deep clone of this object
 */
GObject *GNelderMead_PersonalityTraits::clone_() const {
    return new GNelderMead_PersonalityTraits(*this);
}

/******************************************************************************/
/**
 * Loads the data of another GNelderMead_PersonalityTraits object
 */
void GNelderMead_PersonalityTraits::load_(const GObject *cp) {
    const GNelderMead_PersonalityTraits *p_load =
        Gem::Common::g_convert_and_compare<GObject, GNelderMead_PersonalityTraits>(cp, this);

    // Load the parent class'es data
    GPersonalityTraits::load_(cp);

    // and then the local data
    popPos_ = p_load->popPos_;
}

/******************************************************************************/
/**
 * Sets the position of the individual in the population
 */
void GNelderMead_PersonalityTraits::setPopulationPosition(const std::size_t &pop_pos) {
    popPos_ = pop_pos;
}

/******************************************************************************/
/**
 * Retrieves the position of the individual in the population
 */
std::size_t GNelderMead_PersonalityTraits::getPopulationPosition(void) const {
    return popPos_;
}

/******************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
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
 * Performs self tests that are expected to succeed. This is needed for testing purposes
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
 * Performs self tests that are expected to fail. This is needed for testing purposes
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

} /* namespace Gem::Geneva */
