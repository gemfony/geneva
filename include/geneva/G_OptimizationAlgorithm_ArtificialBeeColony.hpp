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

#pragma once

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard header files go here

// Boost header files go here

// Geneva headers go here
#include "common/GExceptions.hpp"
#include "common/GPlotDesigner.hpp"
#include "geneva/GParameterSet.hpp"
#include "geneva/G_OptimizationAlgorithm_Base.hpp"
#include "geneva/GOptimizationEnums.hpp"
#include "geneva/G_OptimizationAlgorithm_ArtificialBeeColony_PersonalityTraits.hpp"

#ifdef GEM_TESTING
#include "geneva/GTestIndividual1.hpp"
#endif /* GEM_TESTING */

namespace Gem {
namespace Geneva {


class GArtificialBeeColony
        : public G_OptimizationAlgorithm_Base {

    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template<typename Archive>
    void serialize(Archive & ar, const unsigned int) {
        using boost::serialization::make_nvp;

        ar
        & make_nvp("G_OptimizationAlgorithm_Base", boost::serialization::base_object<G_OptimizationAlgorithm_Base>(*this))
        & BOOST_SERIALIZATION_NVP(m_dbl_lower_parameter_boundaries_cnt)
        & BOOST_SERIALIZATION_NVP(m_dbl_upper_parameter_boundaries_cnt)
        & BOOST_SERIALIZATION_NVP(m_max_trial)
        & BOOST_SERIALIZATION_NVP(m_random_seed)
        & BOOST_SERIALIZATION_NVP(m_parallel_rule)
        & BOOST_SERIALIZATION_NVP(m_best_individual);
    }
    ///////////////////////////////////////////////////////////////////////


public:
    /** @brief The default constructor */
    G_API_GENEVA GArtificialBeeColony();

    /** @brief A standard copy constructor */
    G_API_GENEVA GArtificialBeeColony(const GArtificialBeeColony&) = default;

    /** @brief The standard destructor */
    G_API_GENEVA ~GArtificialBeeColony() override = default;

    /** @brief Retrieves the random generator seed of the algorithm */
    G_API_GENEVA size_t getMRandomSeed() const;

    /** @brief Retrieves the max trial set for the algorithm */
    G_API_GENEVA uint32_t getMMaxTrial() const;

    /** @brief Sets the max trial for the algorithm */
    G_API_GENEVA void setMMaxTrial(uint32_t mMaxTrial);

    /** @brief Retrieves which algorithm is being used for the Onlooker Phase */
    G_API_GENEVA abcParallelRule getMParallelRule() const;

    /** @brief Sets the algorithm for the Onlooker Phase*/
    G_API_GENEVA void setMParallelRule(abcParallelRule mParallelRule);

    /** @brief Gets the best current individual */
    const std::shared_ptr<GParameterSet> &getMBestIndividual() const;


protected:
    /** @brief Adds local configuration options to a GParserBuilder object */
    G_API_GENEVA void addConfigurationOptions_(Common::GParserBuilder &gpb) override;

    /** @brief Loads the data of another GArtificialBeeColony object, camouflaged as a GObject */
    G_API_GENEVA void load_(const GObject *cp) override;

    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GArtificialBeeColony>(
            GArtificialBeeColony const &
            , GArtificialBeeColony const &
            , Gem::Common::GToken &
    );

    /**
     * Searches for compliance with expectations with respect to another object of the same type
     * @param object The other object
     * @param expectation The expectations for this object, e.g. equality
     * @param d The limit of deviation for comparing floating point types
     */
    G_API_GENEVA void compare_(
            const GObject &cp
            , const Common::expectation &e
            , const double &limit
    ) const override;

    /** @brief Resets the settings of this population to what was configured when the optimize()-call was issued */
    G_API_GENEVA void resetToOptimizationStart_() override;

    /** @brief Does any necessary initialization work before the optimization cycle starts */
    G_API_GENEVA void init() override;

    /** @brief Does any necessary finalization work */
    G_API_GENEVA void finalize() override;

    /** @brief Applies modifications to this object */
    G_API_GENEVA bool modify_GUnitTests_() override;

    /** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
    G_API_GENEVA void specificTestsNoFailureExpected_GUnitTests_() override;

    /** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
    G_API_GENEVA void specificTestsFailuresExpected_GUnitTests_() override;

private:
    /** @brief Emits a name for this class / object */
    G_API_GENEVA std::string name_() const override;

    /** @brief Creates a deep clone of this object */
    G_API_GENEVA GObject *clone_() const override;

    /** @brief The actual business logic to be performed during each iteration; Returns the best achieved fitness */
    G_API_GENEVA std::tuple<double, double> cycleLogic_() override;

    /** @brief Retrieve a GPersonalityTraits object belonging to this algorithm */
    G_API_GENEVA std::shared_ptr<GPersonalityTraits> getPersonalityTraits_() const override;

    /** @brief Gives individuals an opportunity to update their internal structures */
    G_API_GENEVA void actOnStalls_() override;

    /** @brief Resizes the population to the desired level and does some error checks */
    G_API_GENEVA void adjustPopulation_() override;

    /** @brief We submit individuals to the broker connector and wait for processed items */
    G_API_GENEVA void runFitnessCalculation_() override;

    /** @brief Returns information about the type of optimization algorithm. */
    G_API_GENEVA std::string getAlgorithmPersonalityType_() const override;

    /** @brief Returns the name of this optimization algorithm */
    G_API_GENEVA std::string getAlgorithmName_() const override;

    /** @brief The Employee Phase of the ABC Algorithm */
    void employeeBeePhase();

    /** @brief The Scouting Phase of the ABC Algorithm*/
    void scoutBeePhase();

    /** @brief The Onlooker Phase of the ABC Algorithm*/
    void onlookerBeePhase();

    /** @brief Updates the best individual found so far*/
    void findBestIndividual();

    /**
     * Finds the index of the individual with the highest current trial value
     * @return The index of the found individual
     */
    std::size_t findMaxTrialIndex();

    /** @brief The Parallel Variant of the Onlooker Phase */
    void onlookerParallel();

    /** @brief The Sequential Variant of the Onlooker Phase*/
    void onlookerSequential();

    /** @brief The Simplex Variant of the Onlooker Phase */
    void onlookerSimplex();

    /** @brief Sets the best current individual */
    void setMBestIndividual(const std::shared_ptr<GParameterSet> &mBestIndividual);

    /**
     * @brief Calculates the Probabilities for an Individual to be chosen and also sets the onlooker counts accourding to these individuals
     */
    void onlookerProbabilityCalculations();

    /** @brief A countainer for the lower bounds for double parameters */
    std::vector<double> m_dbl_lower_parameter_boundaries_cnt = std::vector<double>();

    /** @brief A countainer for the upper bounds for double parameters */
    std::vector<double> m_dbl_upper_parameter_boundaries_cnt = std::vector<double>();

    /** @brief The current maximum trial */
    std::uint32_t m_max_trial = DEFAULTMAXTRIAL;

    /** @brief The random generator seed */
    std::size_t m_random_seed = 0;

    /** @brief Which onlooker phase algorithm the bee colony uses */
    abcParallelRule m_parallel_rule = DEFAULTPARALLELRULE;

    /** @brief The current best individual */
    std::shared_ptr<GParameterSet> m_best_individual;
};

} /* namespace Geneva */
} /* namespace Gem */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GArtificialBeeColony)