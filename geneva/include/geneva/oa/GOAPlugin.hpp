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
#include <memory>

// Boost headers go here
#include <boost/config.hpp> // BOOST_SYMBOL_EXPORT

// Geneva headers go here
#include "common/GModuleManifest.hpp" // the unified module manifest (GenevaCompat + contributions)
#include "common/GProviderT.hpp"
#include "geneva/oa/GInitializerT.hpp" // GOAFactoryProviderT
#include "geneva/oa/GOptimizationAlgorithmBase.hpp"

namespace Gem::Geneva {

/******************************************************************************/
/**
 * @brief The type a loaded optimization-algorithm contribution hands back: a provider over the OA base,
 * i.e. the exact type @c oaFactoryStore() holds. The module builds the provider (where the concrete factory
 * type is known) and the loader registers it type-erased, so the loader stays free of the concrete OA.
 */
using GOAProviderPtr = std::shared_ptr<Gem::Common::GProviderT<OptimizationAlgorithms::GOptimizationAlgorithmBase>>;

/******************************************************************************/
/**
 * @brief Builds the module manifest for a runtime-loadable optimization algorithm, entirely in C++ (no macro).
 *
 * The OA analogue of @c individualManifest(). A loadable OA's source hand-writes ONE small entry point that
 * delegates here:
 * @code
 *   extern "C" BOOST_SYMBOL_EXPORT const GenevaModuleManifest *geneva_module_manifest() {
 *       return Gem::Geneva::oaManifest<MyOAFactory, "MyOA">();
 *   }
 * @endcode
 * The @c extern @c "C" wrapper is irreducible (the loader resolves the fixed, unmangled symbol
 * @c geneva_module_manifest via dlsym); everything else is this typed template.
 *
 * Unlike an individual (a claim-once single slot), an OA is resolved by mnemonic against a shared store, so
 * the contribution's factory thunk hands back a fully-built @c GOAFactoryProviderT (as a @c GOAProviderPtr on
 * the heap, the plain-C @c void* boundary); the loader moves it out and @c setOnce-registers it into
 * @c oaFactoryStore() under the provider's own mnemonic (the algorithm's personality nickname). Registering a
 * mnemonic a built-in (or another module) already holds is a hard error -- a module cannot shadow one.
 *
 * This function does NOT emit the OA's @c BOOST_CLASS_EXPORT -- the OA's own translation unit carries the
 * @c BOOST_CLASS_EXPORT_IMPLEMENT for the algorithm and its personality traits (the same registrations a
 * compiled-in OA needs), so a checkpoint written with the module loaded resumes with it loaded.
 *
 * @tparam FactoryType The concrete OA factory (a @c GOAFactoryT<GOptimizationAlgorithmBase> subclass, e.g.
 *         @c GOptimizationAlgorithmFactoryT<MyOA, MyOA_PersonalityTraits>); default-constructed by the provider
 * @tparam Name The module/contribution name, for diagnostics (a string literal); the store key is the
 *         provider's mnemonic, not this
 * @return A pointer to this module's process-lifetime manifest
 */
template <typename FactoryType, Gem::Common::GFixedString Name>
const GenevaModuleManifest *oaManifest() {
    // Captureless thunk -> void*(*)(void): build the OA provider (which default-constructs the factory) on
    // the heap; the loader moves-from and deletes it, then registers the provider in oaFactoryStore().
    static constexpr auto factory_thunk = +[]() -> void * {
        return new GOAProviderPtr(
            std::make_shared<OptimizationAlgorithms::GOAFactoryProviderT<FactoryType>>());
    };
    static const GenevaContribution contribution{GENEVA_CONTRIBUTION_OA, Name.c_str(), factory_thunk};
    static const GenevaModuleManifest manifest{
        GENEVA_BUILD_FINGERPRINT, Name.c_str(), GENEVA_VERSION_STRING, &contribution, 1u};
    return &manifest;
}

/******************************************************************************/

} /* namespace Gem::Geneva */
