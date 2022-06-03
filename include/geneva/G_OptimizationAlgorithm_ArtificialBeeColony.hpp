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
    G_API_GENEVA GArtificialBeeColony();
    /** @brief A standard copy constructor */
    G_API_GENEVA GArtificialBeeColony(const GArtificialBeeColony&);

    G_API_GENEVA size_t getMRandomSeed() const;

    G_API_GENEVA uint32_t getMMaxTrial() const;

    G_API_GENEVA void setMMaxTrial(uint32_t mMaxTrial);

    G_API_GENEVA abcParallelRule getMParallelRule() const;

    G_API_GENEVA void setMParallelRule(abcParallelRule mParallelRule);

    const std::shared_ptr<GParameterSet> &getMBestIndividual() const;

    void setMBestIndividual(const std::shared_ptr<GParameterSet> &mBestIndividual);

protected:
    G_API_GENEVA void addConfigurationOptions_(Common::GParserBuilder &gpb) override;

    G_API_GENEVA void load_(const GObject *cp) override;

    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GArtificialBeeColony>(
            GArtificialBeeColony const &
            , GArtificialBeeColony const &
            , Gem::Common::GToken &
    );

    G_API_GENEVA void compare_(
            const GObject &cp
            , const Common::expectation &e
            , const double &limit
    ) const override;

    G_API_GENEVA void resetToOptimizationStart_() override;

    G_API_GENEVA void init() override;

    G_API_GENEVA void finalize() override;

    G_API_GENEVA bool modify_GUnitTests_() override;

    G_API_GENEVA void specificTestsNoFailureExpected_GUnitTests_() override;

    G_API_GENEVA void specificTestsFailuresExpected_GUnitTests_() override;

private:
    /** @brief Emits a name for this class / object */
    G_API_GENEVA std::string name_() const override;
    /** @brief Creates a deep clone of this object */
    G_API_GENEVA GObject *clone_() const override;

    G_API_GENEVA std::tuple<double, double> cycleLogic_() override;

    G_API_GENEVA std::shared_ptr<GPersonalityTraits> getPersonalityTraits_() const override;

    G_API_GENEVA void adjustPopulation_() override;

    G_API_GENEVA void actOnStalls_() override;

    G_API_GENEVA void runFitnessCalculation_() override;

    G_API_GENEVA std::string getAlgorithmPersonalityType_() const override;

    G_API_GENEVA std::string getAlgorithmName_() const override;

    void employeeBeePhase();

    void scoutBeePhase();

    void onlookerBeePhase();

    void findBestIndividual();

    std::size_t findMaxTrialIndex();

    void onlookerParallel();

    void onlookerSequential();

    void onlookerSimplex();

    /**
     * @brief Calculates the Probabilities for an Individual to be chosen and also sets the onlooker counts accourding to these individuals
     */
    void onlookerProbabilityCalculations();

    std::vector<double> m_dbl_lower_parameter_boundaries_cnt = std::vector<double>(); ///< Holds lower boundaries of double parameters
    std::vector<double> m_dbl_upper_parameter_boundaries_cnt = std::vector<double>(); ///< Holds upper boundaries of double parameters

    std::uint32_t m_max_trial = DEFAULTMAXTRIAL;
    std::size_t m_random_seed = 0;

    abcParallelRule m_parallel_rule = DEFAULTPARALLELRULE;

    std::shared_ptr<GParameterSet> m_best_individual;
};

} /* namespace Geneva */
} /* namespace Gem */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GArtificialBeeColony)