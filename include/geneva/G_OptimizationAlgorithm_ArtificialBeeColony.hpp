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
#include "common/GParserBuilder.hpp"
#include "G_OptimizationAlgorithm_Base.hpp"
#include "GObject.hpp"
#include "common/GCommonEnums.hpp"
#include "GParameterSetFixedSizePriorityQueue.hpp"

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
        & make_nvp("G_OptimizationAlgorithm_Base", boost::serialization::base_object<G_OptimizationAlgorithm_Base>(*this));
        //TODO: Add more
    }
    ///////////////////////////////////////////////////////////////////////


public:
    G_API_GENEVA GArtificialBeeColony();

    GArtificialBeeColony(const G_OptimizationAlgorithm_Base &cp);

protected:
    G_API_GENEVA void addConfigurationOptions_(Common::GParserBuilder &gpb) override;

    G_API_GENEVA void load_(const GObject *cp) override;

    G_API_GENEVA void compare_(const GObject &cp, const Common::expectation &e, const double &limit) const override;

    G_API_GENEVA void resetToOptimizationStart_() override;

    G_API_GENEVA void init() override;

    G_API_GENEVA void finalize() override;

    G_API_GENEVA bool modify_GUnitTests_() override;

    G_API_GENEVA void specificTestsNoFailureExpected_GUnitTests_() override;

    G_API_GENEVA void specificTestsFailuresExpected_GUnitTests_() override;

    G_API_GENEVA void updateGlobalBestsPQ_(GParameterSetFixedSizePriorityQueue &bestIndividuals) override;

    G_API_GENEVA void updateIterationBestsPQ_(GParameterSetFixedSizePriorityQueue &bestIndividuals) override;

private:
    G_API_GENEVA std::string name_() const override;

    G_API_GENEVA std::tuple<double, double> cycleLogic_() BASE override;

    G_API_GENEVA std::shared_ptr<GPersonalityTraits> getPersonalityTraits_() const BASE override;

    G_API_GENEVA void adjustPopulation_() BASE override;

    G_API_GENEVA void actOnStalls_() BASE override;

    void runFitnessCalculation_() BASE override;

    std::string getAlgorithmPersonalityType_() const BASE override;

    std::string getAlgorithmName_() const BASE override;
};

} /* namespace Geneva */
} /* namespace Gem */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GArtificialBeeColony)