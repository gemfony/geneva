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
#include <iostream>
#include <type_traits>

// Boost header files go here

// Geneva headers go here
#include "common/GGlobalOptionsT.hpp"
#include "common/GLogger.hpp"
#include "common/GProviderT.hpp"
#include "geneva/genome/GOptimizableEntity.hpp"
#include "geneva/oa/GOptimizationAlgorithmBase.hpp"
#include "geneva/oa/GFactoryStore.hpp"
#include "geneva/oa/GOAFactoryT.hpp"

namespace Gem::Geneva::OptimizationAlgorithms {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A provider that wraps a config-file-driven optimization-algorithm factory.
 * Each provide() call produces a freshly configured algorithm (the factory
 * re-reads its JSON config and bumps its instance id), which is what algorithm
 * chaining (e.g. "ea,gd,swarm") relies on. This is the factory flavour of the
 * shared Gem::Common::GProviderT abstraction.
 *
 * @tparam oaf_type The concrete optimization-algorithm factory type to wrap; must derive
 *         from GOAFactoryT<GOptimizationAlgorithmBase>.
 */
template <typename oaf_type>
class GOAFactoryProviderT : public Gem::Common::GProviderT<GOptimizationAlgorithmBase> {
    // Make sure oaf_type has the expected type
    static_assert(
        std::is_base_of_v<GOAFactoryT<GOptimizationAlgorithmBase>, oaf_type>,
        "GOAFactoryT<GOptimizationAlgorithmBase> is not a base of oaf_type"
    );

public:
    /**
     * @brief Produces a freshly configured optimization algorithm from the wrapped factory.
     * @return A shared pointer to a newly created, configured optimization algorithm.
     */
    std::shared_ptr<GOptimizationAlgorithmBase> provide() override {
        return factory_->Gem::Common::GFactoryT<GOptimizationAlgorithmBase>::get();
    }
    /**
     * @brief Retrieves the mnemonic of the wrapped algorithm factory.
     * @return The factory's mnemonic (e.g. "ea", "cgd", "swarm").
     */
    [[nodiscard]] std::string getMnemonic() const override { return factory_->getMnemonic(); }
    /**
     * @brief Retrieves the human-readable name of the wrapped algorithm factory.
     * @return The factory's algorithm name.
     */
    [[nodiscard]] std::string getName() const override { return factory_->getAlgorithmName(); }
    /**
     * @brief Adds the wrapped factory's command-line options to the given option descriptions.
     * @param visible The options-description collecting options shown in the help text.
     * @param hidden The options-description collecting options hidden from the help text.
     */
    void addCLOptions(
        boost::program_options::options_description &visible,
        boost::program_options::options_description &hidden
    ) override {
        factory_->addCLOptions(visible, hidden);
    }

private:
    // Stored as the concrete factory base (GOptimizationAlgorithmBase is not dependent here), so the
    // qualified GFactoryT<GOptimizationAlgorithmBase>::get() in provide() is well-formed; virtual
    // dispatch still reaches oaf_type's overrides.
    std::shared_ptr<GOAFactoryT<GOptimizationAlgorithmBase>> factory_{std::make_shared<oaf_type>()};
};

/******************************************************************************/
/**
 * This base class registers an optimization-algorithm factory (wrapped in a
 * GOAFactoryProviderT) with the global algorithm store.
 *
 * @tparam oaf_type The concrete optimization-algorithm factory type to register.
 */
template <typename oaf_type>
class GInitializerT {
public:
    /**
     * @brief The initializing constructor; registers the factory provider with the
     *        global algorithm store (only once per mnemonic).
     */
    GInitializerT() {
        auto provider = std::make_shared<GOAFactoryProviderT<oaf_type>>();
        // Add the provider to the store, if it hasn't been stored there yet
        oaFactoryStore()->setOnce(provider->getMnemonic(), provider);
    }

    /** @brief Defaulted destructor */
    virtual ~GInitializerT() = default;
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Gem::Geneva::OptimizationAlgorithms */

