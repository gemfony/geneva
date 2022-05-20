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
 * Geneva was started by Dr. Rüdiger Berlich and was later maintained together
 * with Dr. Ariel Garcia under the auspices of Gemfony scientific. For further
 * information on Gemfony scientific, see http://www.gemfomy.eu .
 *
 * The majority of files in Geneva was released under the Apache license v2.0
 * in February 2020.
 *
 * See the NOTICE file in the top-level directory of the Geneva library
 * collection for a list of contributors and copyright information.
 *
 ********************************************************************************/

#include "geneva/G_OptimizationAlgorithm_ArtificialBeeColony_PersonalityTraits.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GArtificialBeeColony_PersonalityTraits) // NOLINT

namespace Gem {
namespace Geneva {

/******************************************************************************/
/** A short identifier suitable for storage in a std::map */
G_API_GENEVA const std::string GArtificialBeeColony_PersonalityTraits::nickname = "abc"; // NOLINT

GArtificialBeeColony_PersonalityTraits::GArtificialBeeColony_PersonalityTraits(const GArtificialBeeColony_PersonalityTraits &cp)
        : GPersonalityTraits(cp)
        , trial_(cp.trial_)
{ /* nothing */ }

unsigned int GArtificialBeeColony_PersonalityTraits::getTrial() const {
    return trial_;
}

void GArtificialBeeColony_PersonalityTraits::setTrial(unsigned int trial) {
    trial_ = trial;
}

std::string GArtificialBeeColony_PersonalityTraits::getMnemonic() const {
    return GArtificialBeeColony_PersonalityTraits::nickname;
}

void GArtificialBeeColony_PersonalityTraits::load_(const GObject *cp) {
    // Check that we are dealing with a GArtificialBeeColony_PersonalityTraits reference independent of this object and convert the pointer
    const GArtificialBeeColony_PersonalityTraits *p_load = Gem::Common::g_convert_and_compare<GObject, GArtificialBeeColony_PersonalityTraits>(cp, this);

    // Load the parent class'es data
    GPersonalityTraits::load_(cp);

    // and then the local data
    trial_ = p_load->trial_;
}

void GArtificialBeeColony_PersonalityTraits::compare_(
        const GObject &cp, const Common::expectation &e, const double &limit
) const {
    using namespace Gem::Common;

    // Check that we are dealing with a GArtificialBeeColony_PersonalityTraits reference independent of this object and convert the pointer
    const GArtificialBeeColony_PersonalityTraits *p_load = Gem::Common::g_convert_and_compare<GObject, GArtificialBeeColony_PersonalityTraits>(cp, this);

    GToken token("GArtificialBeeColony_PersonalityTraits", e);

    // Compare our parent data ...
    Gem::Common::compare_base_t<GPersonalityTraits>(*this, *p_load, token);

    // ... and then the local data
    compare_t(IDENTITY(trial_, p_load->trial_), token);

    token.evaluate();
}

bool GArtificialBeeColony_PersonalityTraits::modify_GUnitTests_() {
#ifdef GEM_TESTING
    bool result = false;

    // Call the parent class'es function
    if (GPersonalityTraits::modify_GUnitTests_()) result = true;

    this->setTrial(1);
    result = true;

    return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset("GArtificialBeeColony_PersonalityTraits::modify_GUnitTests", "GEM_TESTING");
   return false;
#endif /* GEM_TESTING */
}

void GArtificialBeeColony_PersonalityTraits::specificTestsNoFailureExpected_GUnitTests_() {
#ifdef GEM_TESTING
    using boost::unit_test_framework::test_suite;
    using boost::unit_test_framework::test_case;

    // Call the parent class'es function
    GPersonalityTraits::specificTestsNoFailureExpected_GUnitTests_();
#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset("GArtificialBeeColony_PersonalityTraits::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

void GArtificialBeeColony_PersonalityTraits::specificTestsFailuresExpected_GUnitTests_() {
#ifdef GEM_TESTING
    using boost::unit_test_framework::test_suite;
    using boost::unit_test_framework::test_case;

    // Call the parent class'es function
    GPersonalityTraits::specificTestsFailuresExpected_GUnitTests_();
#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset("GArtificialBeeColony_PersonalityTraits::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

std::string GArtificialBeeColony_PersonalityTraits::name_() const {
    return std::string("GArtificialBeeColony_PersonalityTraits");
}

GObject *GArtificialBeeColony_PersonalityTraits::clone_() const {
    return new GArtificialBeeColony_PersonalityTraits(*this);
}

} /* namespace Geneva */
} /* namespace Gem */