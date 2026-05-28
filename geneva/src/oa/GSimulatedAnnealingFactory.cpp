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

#include "geneva/oa/GSimulatedAnnealingFactory.hpp"
#include "geneva/oa/GInitializerT.hpp"
#include "common/GFactoryT.hpp"
#include "common/GParserBuilder.hpp"
#include "geneva/oa/GBase.hpp"
#include "geneva/oa/GOAFactoryT.hpp"
#include "geneva/oa/GSimulatedAnnealing.hpp"
#include "geneva/oa/GSimulatedAnnealing_PersonalityTraits.hpp"
#include "geneva/par/GParameterSet.hpp"
#include <cstddef>
#include <memory>
#include <string>

namespace Gem::Geneva::OptimizationAlgorithms {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * The default constructor
 */
GSimulatedAnnealingFactory::GSimulatedAnnealingFactory()
  : GOAFactoryT<GBase>(
        "./config/GSimulatedAnnealing.json"
    ) { /* nothing */
}

/******************************************************************************/
/**
 * Initialization with the name of the config file
 */
GSimulatedAnnealingFactory::GSimulatedAnnealingFactory(std::filesystem::path const &config_file)
  : GOAFactoryT<GBase>(config_file) { /* nothing */
}

/******************************************************************************/
/**
 * A constructor with the ability to switch the parallelization mode and
 * to add a content creator. It initializes a target item as needed.
 */
GSimulatedAnnealingFactory::GSimulatedAnnealingFactory(
    std::filesystem::path const &config_file,
    std::shared_ptr<Gem::Common::GFactoryT<gpar::GParameterSet>> content_creator_ptr
)
  : GOAFactoryT<GBase>(
        config_file,
        content_creator_ptr
    ) { /* nothing */
}

/******************************************************************************/
/**
 * Gives access to the mnemonics / nickname describing an algorithm
 */
std::string GSimulatedAnnealingFactory::getMnemonic() const {
    return GSimulatedAnnealing_PersonalityTraits::nickname;
}

/******************************************************************************/
/**
 * Gives access to a clear-text description of the algorithm
 */
std::string GSimulatedAnnealingFactory::getAlgorithmName() const {
    return std::string("Simulated Annealing");
}

/******************************************************************************/
/**
 * Creates items of this type
 *
 * @return Items of the desired type
 */
std::shared_ptr<GBase> GSimulatedAnnealingFactory::getObject_(
    Gem::Common::GParserBuilder &gpb,
    [[maybe_unused]] const std::size_t & id
) {
    std::shared_ptr<GSimulatedAnnealing> target(new GSimulatedAnnealing());

    // Make the local configuration options known (up to the level of GSimulatedAnnealing)
    target->GSimulatedAnnealing::addConfigurationOptions(gpb);

    return target;
}

/******************************************************************************/
/**
 * Allows to act on the configuration options received from the configuration file. Here
 * we can add the options described in describeLocalOptions to the object.
 *
 * @param p_base A smart-pointer to be acted on during post-processing
 */
void GSimulatedAnnealingFactory::postProcess_(
    std::shared_ptr<GBase> &p_base
) {
    // Call our parent class'es function
    GOAFactoryT<GBase>::postProcess_(p_base);
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

/******************************************************************************/
/**
 * Self-registration of this optimization-algorithm factory with the global
 * factory store at library-load time, so that Go2 needs no explicit
 * registration call. (Geneva is always built as a shared library, so these
 * load-time initializers are never stripped.)
 */
namespace {
GInitializerT<GSimulatedAnnealingFactory> g_oaf_registrant;
} // anonymous namespace

/******************************************************************************/

} /* namespace Gem::Geneva::OptimizationAlgorithms */
