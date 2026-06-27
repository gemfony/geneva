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

#pragma once

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard header files go here
#include <string>

// Boost header files go here

// Geneva headers go here
#include "courtier/GCourtierEnums.hpp"
#include "geneva/ind/GOptimizableEntity.hpp"
#include "geneva/oa/GOptimizationAlgorithmBase.hpp"
#include "geneva/oa/GEvolutionaryAlgorithm.hpp"
#include "geneva/oa/GOAFactoryT.hpp"
#include "geneva/oa/GOptimizationAlgorithmFactoryT.hpp"
#include "geneva/oa/GInitializerT.hpp"
#include "geneva/oa/GEvolutionaryAlgorithm_PersonalityTraits.hpp"

namespace Gem::Geneva::OptimizationAlgorithms {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class is a specialization of the GFactoryT<> class for the dimension-aware adaptive evolutionary
 * algorithm ("ea"). It is a thin twin of GSepCmaEvolutionStrategyFactory carrying its own mnemonic.
 */
class GEvolutionaryAlgorithmFactory // NOLINT(cppcoreguidelines-special-member-functions)
  : public GOptimizationAlgorithmFactoryT<GEvolutionaryAlgorithm, GEvolutionaryAlgorithm_PersonalityTraits> {
    using Base = GOptimizationAlgorithmFactoryT<GEvolutionaryAlgorithm, GEvolutionaryAlgorithm_PersonalityTraits>;

public:
    /** @brief The default constructor */
    GEvolutionaryAlgorithmFactory() = default;
    /** @brief Initialization with the name of the config file. @param config_file Path to the JSON config */
    explicit GEvolutionaryAlgorithmFactory(std::filesystem::path const &config_file)
      : Base(config_file) { /* nothing */ }
    /** @brief Initialization with the config file and a content creator.
     *  @param config_file Name of the configuration file
     *  @param content_creator_ptr A factory that produces the individuals populating the algorithm */
    GEvolutionaryAlgorithmFactory(
        const std::string &config_file,
        std::shared_ptr<Gem::Common::GFactoryT<gen::GOptimizableEntity>> content_creator_ptr
    )
      : Base(config_file, content_creator_ptr) { /* nothing */ }
    /** @brief The copy constructor. @param The object to be copied */
    GEvolutionaryAlgorithmFactory(const GEvolutionaryAlgorithmFactory &) = default;
    /** @brief The destructor */
    ~GEvolutionaryAlgorithmFactory() override = default;
};

/******************************************************************************/

} /* namespace Gem::Geneva::OptimizationAlgorithms */
