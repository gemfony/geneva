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

#include "geneva/G_OptimizationAlgorithm_ArtificialBeeColony_Factory.hpp"

namespace Gem {
namespace Geneva {

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

#include "geneva/G_OptimizationAlgorithm_ArtificialBeeColony_Factory.hpp"

/******************************************************************************/
/**
 * The default constructor
 */
GArtificialBeeColonyFactory::GArtificialBeeColonyFactory()
        : G_OptimizationAlgorithm_FactoryT<G_OptimizationAlgorithm_Base>(
        "./config/GArtificialBeeColony.json")
{ /* nothing */ }

/******************************************************************************/
/**
 * Initialization with the name of the config file
 */
GArtificialBeeColonyFactory::GArtificialBeeColonyFactory(
        std::filesystem::path const& configFile
)
        : G_OptimizationAlgorithm_FactoryT<G_OptimizationAlgorithm_Base>(configFile)
{ /* nothing */ }

/******************************************************************************/
/**
 * A constructor with the ability to switch the parallelization mode and
 * to add a content creator. It initializes a target item as needed.
 */
GArtificialBeeColonyFactory::GArtificialBeeColonyFactory(const std::filesystem::path &configFile, std::shared_ptr<
        GFactoryT <GParameterSet>>contentCreatorPtr) : G_OptimizationAlgorithm_FactoryT(configFile, contentCreatorPtr)
{ /* nothing */ }

/******************************************************************************/
/**
 * Gives access to the mnemonics / nickname describing an algorithm
 */
std::string GArtificialBeeColonyFactory::getMnemonic() const {
    return GArtificialBeeColony_PersonalityTraits::nickname;
}

/******************************************************************************/
/**
 * Gives access to a clear-text description of the algorithm
 */
std::string GArtificialBeeColonyFactory::getAlgorithmName() const {
    return std::string("Swarm Algorithm");
}

/******************************************************************************/
/**
 * Creates items of this type
 *
 * @return Items of the desired type
 */
std::shared_ptr<G_OptimizationAlgorithm_Base> GArtificialBeeColonyFactory::getObject_(
        Common::GParserBuilder &gpb
        , const std::size_t &id
) {
    std::shared_ptr<GArtificialBeeColony> target(
            new GArtificialBeeColony()
    );

    // Make the local configuration options known (up to the level of GArtificialBeeColony)
    target->GArtificialBeeColony::addConfigurationOptions(gpb);

    return target;
}

/******************************************************************************/
/**
 * Allows to act on the configuration options received from the configuration file. Here
 * we can add the options described in describeLocalOptions to the object.
 *
 * @param p A smart-pointer to be acted on during post-processing
 */
void GArtificialBeeColonyFactory::postProcess_(
        std::shared_ptr<G_OptimizationAlgorithm_Base>& p_base
) {
    // Call our parent class'es function
    G_OptimizationAlgorithm_FactoryT<G_OptimizationAlgorithm_Base>::postProcess_(p_base);
}
/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */


