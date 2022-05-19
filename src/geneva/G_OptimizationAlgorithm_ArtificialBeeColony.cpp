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

GArtificialBeeColony::GArtificialBeeColony(const G_OptimizationAlgorithm_Base &cp) : G_OptimizationAlgorithm_Base(cp) {}

void GArtificialBeeColony::addConfigurationOptions_(Common::GParserBuilder &gpb) {
    // Call our parent class'es function
    G_OptimizationAlgorithm_Base::addConfigurationOptions_(gpb);
}

void GArtificialBeeColony::load_(const GObject *cp) {
    G_OptimizationAlgorithm_Base::load_(cp);
}

void GArtificialBeeColony::compare_(const GObject &cp, const Common::expectation &e, const double &limit) const {
    G_OptimizationAlgorithm_Base::compare_(cp, e, limit);
}

void GArtificialBeeColony::resetToOptimizationStart_() {
    G_OptimizationAlgorithm_Base::resetToOptimizationStart_();
}

void GArtificialBeeColony::init() {
    // To be performed before any other action
    G_OptimizationAlgorithm_Base::init();

    // Extract the boundaries of all parameters
    this->at(0)->boundaries(m_dbl_lower_parameter_boundaries_cnt, m_dbl_upper_parameter_boundaries_cnt, activityMode::ACTIVEONLY);
}

void GArtificialBeeColony::finalize() {
    G_OptimizationAlgorithm_Base::finalize();
}

bool GArtificialBeeColony::modify_GUnitTests_() {
    return G_OptimizationAlgorithm_Base::modify_GUnitTests_();
}

void GArtificialBeeColony::specificTestsNoFailureExpected_GUnitTests_() {
    G_OptimizationAlgorithm_Base::specificTestsNoFailureExpected_GUnitTests_();
}

void GArtificialBeeColony::specificTestsFailuresExpected_GUnitTests_() {
    G_OptimizationAlgorithm_Base::specificTestsFailuresExpected_GUnitTests_();
}

void GArtificialBeeColony::updateGlobalBestsPQ_(GParameterSetFixedSizePriorityQueue &bestIndividuals) {
    G_OptimizationAlgorithm_Base::updateGlobalBestsPQ_(bestIndividuals);
}

void GArtificialBeeColony::updateIterationBestsPQ_(GParameterSetFixedSizePriorityQueue &bestIndividuals) {
    G_OptimizationAlgorithm_Base::updateIterationBestsPQ_(bestIndividuals);
}

std::tuple<double, double> GArtificialBeeColony::cycleLogic_() {
    return std::tuple<double, double>();
}

std::shared_ptr<GPersonalityTraits> GArtificialBeeColony::getPersonalityTraits_() const {
    return std::shared_ptr<GPersonalityTraits>();
}

void GArtificialBeeColony::adjustPopulation_() {

}

void GArtificialBeeColony::actOnStalls_() {

}

void GArtificialBeeColony::runFitnessCalculation_() {

}

std::string GArtificialBeeColony::getAlgorithmPersonalityType_() const {
    return std::string("PERSONALITY_ABC");
}

std::string GArtificialBeeColony::getAlgorithmName_() const {
    return std::string("GArtificialBeeColony");
}

std::string GArtificialBeeColony::name_() const {

}


} /* namespace Geneva */
} /* namespace Gem */