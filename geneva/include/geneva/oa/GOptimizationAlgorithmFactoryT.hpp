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

// Standard headers go here
#include <filesystem>
#include <memory>
#include <string>

// Geneva headers go here
#include "common/GParserBuilder.hpp"
#include "geneva/ind/GOptimizableEntity.hpp"
#include "geneva/oa/GOptimizationAlgorithmBase.hpp"
#include "geneva/oa/GOAFactoryT.hpp"

namespace Gem::Geneva::OptimizationAlgorithms {

/******************************************************************************/
/**
 * A CRTP-style scaffold for the per-algorithm factories. It generates the boilerplate every standard
 * optimization-algorithm factory repeats: the three constructors (the default one derives the config
 * path "./config/<oa_class_name>.json"), getMnemonic() (= the personality nickname), getAlgorithmName()
 * (= the algorithm's human-readable name) and getObject_() (construct the algorithm and register its
 * configuration options). A concrete factory becomes a thin named subclass -- kept so the GInitializerT
 * self-registration has a concrete type and so Go2 / the examples can refer to it by name.
 *
 *   class GFooFactory : public GOptimizationAlgorithmFactoryT<GFoo, GFoo_PersonalityTraits> { ... };
 *
 * @tparam oa_type                 The concrete algorithm. Must expose the GOptimizationAlgorithmT
 *                                 identifiers oa_class_name and oa_algorithm_name.
 * @tparam personality_traits_type The algorithm's personality traits, whose static `nickname` is the
 *                                 command-line mnemonic.
 */
template <typename oa_type, typename personality_traits_type>
class GOptimizationAlgorithmFactoryT : public GOAFactoryT<GOptimizationAlgorithmBase> {
public:
    /** @brief The default constructor: derives the config-file path from the algorithm's class name. */
    GOptimizationAlgorithmFactoryT()
      : GOAFactoryT<GOptimizationAlgorithmBase>(
            std::string("./config/") + std::string(oa_type::oa_class_name) + ".json"
        ) { /* nothing */
    }

    /** @brief Initialization with the name of the config file. */
    explicit GOptimizationAlgorithmFactoryT(std::filesystem::path const &config_file)
      : GOAFactoryT<GOptimizationAlgorithmBase>(config_file) { /* nothing */
    }

    /** @brief Initialization with the config file and a content creator. */
    GOptimizationAlgorithmFactoryT(
        std::filesystem::path const &config_file,
        std::shared_ptr<Gem::Common::GFactoryT<gen::GOptimizableEntity>> content_creator_ptr
    )
      : GOAFactoryT<GOptimizationAlgorithmBase>(config_file, content_creator_ptr) { /* nothing */
    }

    /** @brief The copy constructor. */
    GOptimizationAlgorithmFactoryT(const GOptimizationAlgorithmFactoryT &) = default;
    /** @brief The destructor. */
    ~GOptimizationAlgorithmFactoryT() override = default;

    /** @brief The command-line mnemonic / nickname describing this algorithm. */
    std::string getMnemonic() const override { return personality_traits_type::nickname; }
    /** @brief The clear-text name of this algorithm. */
    std::string getAlgorithmName() const override { return std::string(oa_type::oa_algorithm_name); }

protected:
    /** @brief Creates the algorithm and registers its configuration options. */
    std::shared_ptr<GOptimizationAlgorithmBase>
    getObject_(Gem::Common::GParserBuilder &gpb, const std::size_t & /* id */) override {
        std::shared_ptr<oa_type> target(new oa_type());
        target->addConfigurationOptions(gpb);
        return target;
    }
};

/******************************************************************************/

} /* namespace Gem::Geneva::OptimizationAlgorithms */
