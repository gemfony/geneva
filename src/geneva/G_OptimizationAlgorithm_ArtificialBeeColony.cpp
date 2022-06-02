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
, m_random_seed(cp.m_random_seed)
, m_parallel_rule(cp.m_parallel_rule)
, m_best_individual(cp.m_best_individual->clone<GParameterSet>())
{/* nothing */}

void GArtificialBeeColony::addConfigurationOptions_(Common::GParserBuilder &gpb) {
    // Call our parent class'es function
    G_OptimizationAlgorithm_Base::addConfigurationOptions_(gpb);

    gpb.registerFileParameter<std::uint32_t>(
            "maxTrial" // The name of the variable
            , DEFAULTMAXTRIAL // The default value
            , [this](std::uint32_t maxTrial){ this->setMMaxTrial(maxTrial); }
    )
            << "The maximum allowed trial_ to abandon a food source";
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
    m_random_seed = p_load->m_random_seed;
    m_parallel_rule = p_load->m_parallel_rule;
    m_best_individual = p_load->m_best_individual->clone<GParameterSet>();
}

void GArtificialBeeColony::compare_(const GObject &cp, const Common::expectation &e, const double &limit) const {
    using namespace Gem::Common;

    // Check that we are dealing with a GBooleanAdaptor reference independent of this object and convert the pointer
    const GArtificialBeeColony *p_load = Gem::Common::g_convert_and_compare<GObject, GArtificialBeeColony >(cp, this);

    GToken token("GArtificialBeeColony", e);

    // Compare our parent data ...
    Gem::Common::compare_base_t<G_OptimizationAlgorithm_Base>(*this, *p_load, token);

    // ... and then the local data
    compare_t(IDENTITY(m_dbl_lower_parameter_boundaries_cnt, p_load->m_dbl_lower_parameter_boundaries_cnt), token);
    compare_t(IDENTITY(m_dbl_upper_parameter_boundaries_cnt, p_load->m_dbl_upper_parameter_boundaries_cnt), token);
    compare_t(IDENTITY(m_max_trial, p_load->m_max_trial), token);
    compare_t(IDENTITY(m_random_seed, p_load->m_random_seed), token);
    compare_t(IDENTITY(m_parallel_rule, p_load->m_parallel_rule), token);
    compare_t(IDENTITY(m_best_individual, p_load->m_best_individual), token);

    // React on deviations from the expectation
    token.evaluate();
}

void GArtificialBeeColony::resetToOptimizationStart_() {
    m_dbl_lower_parameter_boundaries_cnt.clear();
    m_dbl_upper_parameter_boundaries_cnt.clear();

    G_OptimizationAlgorithm_Base::resetToOptimizationStart_();
}

void GArtificialBeeColony::init() {
    // To be performed before any other action
    G_OptimizationAlgorithm_Base::init();

    if(this->getDefaultPopulationSize() <= 1) {
        this->setDefaultPopulationSize(2); //Needs to have atleast 2 Individuals
    }

    m_random_seed = static_cast<size_t> (time(nullptr));
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

    for (auto& ind_ptr: *this) {
        ind_ptr->randomInit(activityMode::ACTIVEONLY); //assuming the randomInit distributes every parameter evenly between its bounds
    }

    runFitnessCalculation_();

    m_best_individual = this->front()->clone<GParameterSet>();

    findBestIndividual();

    adjustPopulation_();
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
    employeeBeePhase();

    scoutBeePhase();

    onlookerBeePhase();

    runFitnessCalculation_();

    findBestIndividual();

    return m_best_individual->getFitnessTuple();
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
            this->back()->randomInit(activityMode::ACTIVEONLY); //if there are individuals missing for some reason, we fill them up with new random solutions
        }
    }
}

void GArtificialBeeColony::actOnStalls_() {
    /* nothing */
}

void GArtificialBeeColony::runFitnessCalculation_() {
    for(const auto& ind_ptr: this->m_data_cnt) {
        ind_ptr->set_processing_status(Courtier::processingStatus::DO_PROCESS);
    }

    auto status = this->workOn(
            m_data_cnt
            ,true
            ,"GArtificialBeeColony::runFitnessCalculation()");


    if(status.has_errors) {
        std::size_t n_erased = Gem::Common::erase_if(
                this->m_data_cnt, [this](std::shared_ptr<GParameterSet> p) -> bool {
                    return p->has_errors();
                }
        );
#ifdef DEBUG
        glogger
                << "In GSwarmAlgorithm::runFitnessCalculation(): " << std::endl
                << "Removed " << n_erased << " erroneous work items in iteration " << this->getIteration() << std::endl
                << GLOGGING;
#endif
        adjustPopulation_();
    }

    while(not status.is_complete) {
        std::vector<std::shared_ptr<GParameterSet>> m_data_incomplete;
        for (auto& ind_ptr: m_data_cnt) {
            m_data_incomplete.push_back(ind_ptr);
        }

        status = this->workOn(m_data_incomplete
                , true
                , "GArtificialBeeColony::runFitnessCalculation()");

        if(not status.is_complete) {
            Gem::Common::erase_if(
                    this->m_data_cnt
                    , [this](std::shared_ptr<GParameterSet> p) -> bool {
                        return (p->getProcessingStatus() == Gem::Courtier::processingStatus::PROCESSED);
                    }
            );
        }
    }
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

size_t GArtificialBeeColony::getMRandomSeed() const {
    return m_random_seed;
}

uint32_t GArtificialBeeColony::getMMaxTrial() const {
    return m_max_trial;
}

void GArtificialBeeColony::setMMaxTrial(uint32_t mMaxTrial) {
    m_max_trial = mMaxTrial;
}

void GArtificialBeeColony::employeeBeePhase() {
    std::vector<std::shared_ptr<GParameterSet>> new_solutions;

    std::vector<double> values;
    std::vector<double> second_individual_values;

    this->front()->streamline(values, activityMode::ACTIVEONLY);
    std::size_t parameter_count = values.size();

    std::size_t second_individual_i;
    std::size_t random_component_i;

    std::mt19937_64 gen(m_random_seed);
    std::uniform_int_distribution<std::size_t> second_individual_rng(0,this->getDefaultPopulationSize() - 2);
    std::uniform_real_distribution<double> phi_rng(-1., 1.);
    std::uniform_int_distribution<std::size_t> component_rng(0, parameter_count - 1);

    for(std::size_t i = 0; i < this->size(); ++i) {
        new_solutions.push_back(this->at(i)->clone<GParameterSet>());
        if (new_solutions.at(i)->getPersonalityTraits<GArtificialBeeColony_PersonalityTraits>()->getTrial() < m_max_trial) {
            second_individual_i = second_individual_rng(gen);
            if(second_individual_i >= i) {
                ++second_individual_i;
            }
            new_solutions.at(i)->streamline(values, activityMode::ACTIVEONLY);
            this->at(second_individual_i)->streamline(second_individual_values, activityMode::ACTIVEONLY);
            if(values.size() == 1) {
                values.at(0) += phi_rng(gen) * (values.at(0) - second_individual_values.at(0));
            } else {
                random_component_i = component_rng(gen);
                values.at(random_component_i) += phi_rng(gen) * (values.at(random_component_i) - second_individual_values.at(random_component_i));
            }
            new_solutions.at(i)->assignValueVector(values, activityMode::ACTIVEONLY);
        }
    }

    workOn(
            new_solutions
            , true
            , "GArtificialBeeColony::employeePhase");

    for(std::size_t i = 0; i < this->size(); ++i) {
        if (isBetter(new_solutions.at(i), this->at(i))) {
            this->at(i)->load(new_solutions.at(i));
            this->at(i)->getPersonalityTraits<GArtificialBeeColony_PersonalityTraits>()->resetTrial();
        } else {
            this->at(i)->getPersonalityTraits<GArtificialBeeColony_PersonalityTraits>()->increaseTrial();
        }
    }

    findBestIndividual();
}

void GArtificialBeeColony::scoutBeePhase() {
    std::size_t max_trial_index = findMaxTrialIndex();
    if (this->at(max_trial_index)->getPersonalityTraits<GArtificialBeeColony_PersonalityTraits>()->getTrial() > m_max_trial) {
        this->at(max_trial_index)->randomInit(activityMode::ACTIVEONLY);
        this->at(max_trial_index)->getPersonalityTraits<GArtificialBeeColony_PersonalityTraits>()->resetTrial();
        this->at(max_trial_index)->set_processing_status(Courtier::processingStatus::DO_PROCESS);
        this->at(max_trial_index)->process();
    }
}

void GArtificialBeeColony::onlookerBeePhase() {

}

void GArtificialBeeColony::findBestIndividual() {
    for (auto& ind_ptr: m_data_cnt) {
        if (isBetter(ind_ptr, this->m_best_individual)) {
            this->m_best_individual = ind_ptr->clone<GParameterSet>();
        }
    }
}

std::size_t GArtificialBeeColony::findMaxTrialIndex() {
    std::size_t max_trial = 0;
    for (std::size_t i = 0; i < this->size(); ++i) {
        max_trial = this->at(i)->getPersonalityTraits<GArtificialBeeColony_PersonalityTraits>()->getTrial() > max_trial ?
                i : max_trial;
    }
    return max_trial;
}

parallelRule GArtificialBeeColony::getMParallelRule() const {
    return m_parallel_rule;
}

void GArtificialBeeColony::setMParallelRule(parallelRule mParallelRule) {
    m_parallel_rule = mParallelRule;
}

const std::shared_ptr<GParameterSet> &GArtificialBeeColony::getMBestIndividual() const {
    return m_best_individual;
}

void GArtificialBeeColony::setMBestIndividual(const std::shared_ptr<GParameterSet> &mBestIndividual) {
    m_best_individual = mBestIndividual;
}


} /* namespace Geneva */
} /* namespace Gem */