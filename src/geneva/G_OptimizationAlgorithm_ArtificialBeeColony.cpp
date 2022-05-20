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

#include "geneva/G_OptimizationAlgorithm_ArtificialBeeColony.hpp"

/******************************************************************************/

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GArtificialBeeColony) // NOLINT

/******************************************************************************/

namespace Gem {
namespace Geneva {

GArtificialBeeColony::GArtificialBeeColony() {}

GArtificialBeeColony::GArtificialBeeColony(const GArtificialBeeColony &cp)
: G_OptimizationAlgorithm_Base(cp)
, m_dbl_lower_parameter_boundaries_cnt(cp.m_dbl_lower_parameter_boundaries_cnt)
, m_dbl_upper_parameter_boundaries_cnt(cp.m_dbl_upper_parameter_boundaries_cnt)
, m_max_trial(cp.m_max_trial)
{/* nothing */}

void GArtificialBeeColony::addConfigurationOptions_(Common::GParserBuilder &gpb) {
    // Call our parent class'es function
    G_OptimizationAlgorithm_Base::addConfigurationOptions_(gpb);
}

void GArtificialBeeColony::load_(const GObject *cp) {
    // Check that we are dealing with a GSwarmAlgorithm reference independent of this object and convert the pointer
    const GArtificialBeeColony *p_load = Gem::Common::g_convert_and_compare<GObject, GArtificialBeeColony >(cp, this);

    // First load the parent class'es data.
    // This will also take care of copying all individuals.
    G_OptimizationAlgorithm_Base::load_(cp);

    // ... and then our own data
    m_dbl_lower_parameter_boundaries_cnt = p_load->m_dbl_lower_parameter_boundaries_cnt;
    m_dbl_upper_parameter_boundaries_cnt = p_load->m_dbl_upper_parameter_boundaries_cnt;
    m_max_trial = p_load->m_max_trial;
}

void GArtificialBeeColony::compare_(const GObject &cp, const Common::expectation &e, const double &limit) const {
    G_OptimizationAlgorithm_Base::compare_(cp, e, limit);
}

void GArtificialBeeColony::resetToOptimizationStart_() {
    m_dbl_lower_parameter_boundaries_cnt.clear();
    m_dbl_upper_parameter_boundaries_cnt.clear();

    G_OptimizationAlgorithm_Base::resetToOptimizationStart_();
}

void GArtificialBeeColony::init() {
    // To be performed before any other action
    G_OptimizationAlgorithm_Base::init();

    // Extract the boundaries of all parameters
    this->at(0)->boundaries(m_dbl_lower_parameter_boundaries_cnt, m_dbl_upper_parameter_boundaries_cnt, activityMode::ACTIVEONLY);

#ifdef DEBUG
    // Size matters!
    if(m_dbl_lower_parameter_boundaries_cnt.size() != m_dbl_upper_parameter_boundaries_cnt.size()) {
        throw gemfony_exception(
                g_error_streamer(DO_LOG,  time_and_place)
                        << "In GArtificialBeeColony::init(): Error!" << std::endl
                        << "Found invalid sizes: "
                        << m_dbl_lower_parameter_boundaries_cnt.size() << " / " << m_dbl_upper_parameter_boundaries_cnt.size() << std::endl
        );
    }
#endif /* DEBUG */

    //TODO: initialize the starting population according to abc
}

void GArtificialBeeColony::finalize() {

    //Last Action
    G_OptimizationAlgorithm_Base::finalize();
}

bool GArtificialBeeColony::modify_GUnitTests_() {
#ifdef GEM_TESTING
    bool result = false;

    // Call the parent class'es function
    if (G_OptimizationAlgorithm_Base::modify_GUnitTests_()) result = true;

    return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset("GArtificialBeeColony::modify_GUnitTests", "GEM_TESTING");
    return false;
#endif /* GEM_TESTING */
}

void GArtificialBeeColony::specificTestsNoFailureExpected_GUnitTests_() {
#ifdef GEM_TESTING
    // Call the parent class'es function
    G_OptimizationAlgorithm_Base::specificTestsNoFailureExpected_GUnitTests_();

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset("GSwarmAlgorithm::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

void GArtificialBeeColony::specificTestsFailuresExpected_GUnitTests_() {
#ifdef GEM_TESTING
    // Call the parent class'es function
    G_OptimizationAlgorithm_Base::specificTestsFailuresExpected_GUnitTests_();

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset("GSwarmAlgorithm::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

std::tuple<double, double> GArtificialBeeColony::cycleLogic_() {
    //TODO: implement the main abc cycle
    return std::tuple<double, double>();
}

std::shared_ptr<GPersonalityTraits> GArtificialBeeColony::getPersonalityTraits_() const {
    return std::shared_ptr<GArtificialBeeColony_PersonalityTraits>(new GArtificialBeeColony_PersonalityTraits());
}

void GArtificialBeeColony::adjustPopulation_() {
    if(this->size() > getDefaultPopulationSize()) {
        this->resize(getDefaultPopulationSize());
    } else if (this->size() < getDefaultPopulationSize()) {
        for (std::size_t i = this->size(); i < getDefaultPopulationSize(); i++) {
            this->push_back(this->front()->clone<GParameterSet>());
            this->back()->randomInit(activityMode::ACTIVEONLY);
        }
    }
}

void GArtificialBeeColony::actOnStalls_() {
    /* nothing */
}

void GArtificialBeeColony::runFitnessCalculation_() {
    //TODO: implement some way to use this for abc (since we usually dont calculate the current existent individuals)
}

std::string GArtificialBeeColony::getAlgorithmPersonalityType_() const {
    return std::string("PERSONALITY_ABC");
}

std::string GArtificialBeeColony::getAlgorithmName_() const {
    return std::string("GArtificialBeeColony");
}

std::string GArtificialBeeColony::name_() const {
    return std::string("GArtificialBeeColony");
}

GObject *GArtificialBeeColony::clone_() const {
    return new GArtificialBeeColony(*this);
}


} /* namespace Geneva */
} /* namespace Gem */