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
#include "geneva/par/GParameterSet.hpp"
#include "geneva/oa/GBase.hpp"
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
 */
template <typename oaf_type>
class GOAFactoryProviderT : public Gem::Common::GProviderT<GBase> {
    // Make sure oaf_type has the expected type
    static_assert(
        std::is_base_of_v<GOAFactoryT<GBase>, oaf_type>,
        "GOAFactoryT<GBase> is not a base of oaf_type"
    );

public:
    std::shared_ptr<GBase> provide() override {
        return factory_->Gem::Common::GFactoryT<GBase>::get();
    }
    std::string getMnemonic() const override { return factory_->getMnemonic(); }
    std::string getName() const override { return factory_->getAlgorithmName(); }
    void addCLOptions(
        boost::program_options::options_description &visible,
        boost::program_options::options_description &hidden
    ) override {
        factory_->addCLOptions(visible, hidden);
    }

private:
    // Stored as the concrete factory base (GBase is not dependent here), so the
    // qualified GFactoryT<GBase>::get() in provide() is well-formed; virtual
    // dispatch still reaches oaf_type's overrides.
    std::shared_ptr<GOAFactoryT<GBase>> factory_{std::make_shared<oaf_type>()};
};

/******************************************************************************/
/**
 * This base class registers an optimization-algorithm factory (wrapped in a
 * GOAFactoryProviderT) with the global algorithm store.
 */
template <typename oaf_type>
class GInitializerT {
public:
    /** @brief The initializing constructor */
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

