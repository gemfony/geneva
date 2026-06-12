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

#include "geneva/oa/GNelderMeadFactory.hpp"
#include "geneva/oa/GInitializerT.hpp"
#include "common/GFactoryT.hpp"
#include "common/GParserBuilder.hpp"
#include "geneva/oa/GOptimizationAlgorithmBase.hpp"
#include "geneva/oa/GNelderMead.hpp"
#include "geneva/oa/GNelderMead_PersonalityTraits.hpp"
#include "geneva/oa/GOAFactoryT.hpp"
#include "geneva/ind/GOptimizableEntity.hpp"
#include <cstddef>
#include <memory>
#include <string>

namespace Gem::Geneva::OptimizationAlgorithms {

/******************************************************************************/
/**
 * The default constructor
 */
GNelderMeadFactory::GNelderMeadFactory()
  : GOAFactoryT<GOptimizationAlgorithmBase>(
        "./config/GNelderMead.json"
    ) { /* nothing */
}

/******************************************************************************/
/**
 * Initialization with the name of the config file
 */
GNelderMeadFactory::GNelderMeadFactory(std::filesystem::path const &config_file)
  : GOAFactoryT<GOptimizationAlgorithmBase>(config_file) { /* nothing */
}

/******************************************************************************/
/**
 * A constructor with the ability to switch the parallelization mode and
 * to add a content creator. It initializes a target item as needed.
 */
GNelderMeadFactory::GNelderMeadFactory(
    const std::string &config_file,
    std::shared_ptr<Gem::Common::GFactoryT<gpar::GOptimizableEntity>> content_creator_ptr
)
  : GOAFactoryT<GOptimizationAlgorithmBase>(
        config_file,
        content_creator_ptr
    ) { /* nothing */
}

/******************************************************************************/
/**
 * Gives access to the mnemonics / nickname describing an algorithm
 */
std::string GNelderMeadFactory::getMnemonic() const {
    return GNelderMead_PersonalityTraits::nickname;
}

/******************************************************************************/
/**
 * Gives access to a clear-text description of the algorithm
 */
std::string GNelderMeadFactory::getAlgorithmName() const {
    return std::string("Nelder-Mead Simplex");
}

/******************************************************************************/
/**
 * Creates items of this type
 *
 * @return Items of the desired type
 */
std::shared_ptr<GOptimizationAlgorithmBase> GNelderMeadFactory::getObject_(
    Gem::Common::GParserBuilder &gpb,
    [[maybe_unused]] const std::size_t & id
) {
    std::shared_ptr<GNelderMead> target(new GNelderMead());

    // Make the local configuration options known (up to the level of GNelderMead)
    target->GNelderMead::addConfigurationOptions(gpb);

    return target;
}

/******************************************************************************/
/**
 * Allows to act on the configuration options received from the configuration file.
 *
 * @param p_base A smart-pointer to be acted on during post-processing
 */
void GNelderMeadFactory::postProcess_(std::shared_ptr<GOptimizationAlgorithmBase> &p_base) {
    // Call our parent class'es function
    GOAFactoryT<GOptimizationAlgorithmBase>::postProcess_(p_base);
}

/******************************************************************************/

/******************************************************************************/
/**
 * Self-registration of this optimization-algorithm factory with the global
 * factory store at library-load time, so that Go2 needs no explicit
 * registration call. (Geneva is always built as a shared library, so these
 * load-time initializers are never stripped.)
 */
namespace {
GInitializerT<GNelderMeadFactory> g_oaf_registrant;
} // anonymous namespace

/******************************************************************************/

} /* namespace Gem::Geneva::OptimizationAlgorithms */
