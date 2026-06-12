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

#include "geneva/oa/GConjugateGradientDescentFactory.hpp"
#include "geneva/oa/GInitializerT.hpp"
#include "common/GFactoryT.hpp"
#include "common/GParserBuilder.hpp"
#include "geneva/oa/GOptimizationAlgorithmBase.hpp"
#include "geneva/oa/GConjugateGradientDescent.hpp"
#include "geneva/oa/GConjugateGradientDescent_PersonalityTraits.hpp"
#include "geneva/oa/GOAFactoryT.hpp"
#include "geneva/ind/GParameterTree.hpp"
#include <cstddef>
#include <memory>
#include <string>

namespace Gem::Geneva::OptimizationAlgorithms {

/******************************************************************************/
/**
 * The default constructor
 */
GConjugateGradientDescentFactory::GConjugateGradientDescentFactory()
  : GOAFactoryT<GOptimizationAlgorithmBase>(
        "./config/GConjugateGradientDescent.json"
    ) { /* nothing */
}

/******************************************************************************/
/**
 * Initialization with the name of the config file
 */
GConjugateGradientDescentFactory::GConjugateGradientDescentFactory(
    std::filesystem::path const &config_file
)
  : GOAFactoryT<GOptimizationAlgorithmBase>(config_file) { /* nothing */
}

/******************************************************************************/
/**
 * A constructor with the ability to switch the parallelization mode and
 * to add a content creator. It initializes a target item as needed.
 */
GConjugateGradientDescentFactory::GConjugateGradientDescentFactory(
    const std::string &config_file,
    std::shared_ptr<Gem::Common::GFactoryT<gpar::GParameterTree>> content_creator_ptr
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
std::string GConjugateGradientDescentFactory::getMnemonic() const {
    return GConjugateGradientDescent_PersonalityTraits::nickname;
}

/******************************************************************************/
/**
 * Gives access to a clear-text description of the algorithm
 */
std::string GConjugateGradientDescentFactory::getAlgorithmName() const {
    return std::string("Conjugate Gradient Descent");
}

/******************************************************************************/
/**
 * Creates items of this type
 *
 * @return Items of the desired type
 */
std::shared_ptr<GOptimizationAlgorithmBase> GConjugateGradientDescentFactory::getObject_(
    Gem::Common::GParserBuilder &gpb,
    [[maybe_unused]] const std::size_t & id
) {
    std::shared_ptr<GConjugateGradientDescent> target(new GConjugateGradientDescent());

    // Make the local configuration options known (up to the level of GConjugateGradientDescent)
    target->GConjugateGradientDescent::addConfigurationOptions(gpb);

    return target;
}

/******************************************************************************/
/**
 * Allows to act on the configuration options received from the configuration file.
 *
 * @param p_base A smart-pointer to be acted on during post-processing
 */
void GConjugateGradientDescentFactory::postProcess_(
    std::shared_ptr<GOptimizationAlgorithmBase> &p_base
) {
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
GInitializerT<GConjugateGradientDescentFactory> g_oaf_registrant;
} // anonymous namespace

/******************************************************************************/

} /* namespace Gem::Geneva::OptimizationAlgorithms */
