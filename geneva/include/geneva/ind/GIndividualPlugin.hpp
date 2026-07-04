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
#include "common/GGlobalDefines.hpp" // GENEVA_VERSION (the ABI token)

// Standard headers go here
#include <cstdint>
#include <memory>

// Boost headers go here
#include <boost/config.hpp> // BOOST_SYMBOL_EXPORT

// Geneva headers go here
#include "common/GFactoryT.hpp"
#include "geneva/ind/GOptimizableEntity.hpp"

namespace Gem::Geneva {

/******************************************************************************/
/**
 * @brief The (unmangled) names of the two C entry points a runtime-loadable individual plugin exports.
 * The loader (GIndividualPluginLoader) resolves exactly these; the GENEVA_INDIVIDUAL_PLUGIN() macro emits
 * them. Kept in one place so the plugin side and the loader side can never drift apart.
 */
inline constexpr const char *GENEVA_INDIVIDUAL_ABI_SYMBOL = "geneva_individual_abi_version";
inline constexpr const char *GENEVA_INDIVIDUAL_FACTORY_SYMBOL = "geneva_make_individual";

/** @brief The factory type a plugin hands back: the same content-creator type Go2::registerContentCreator
 *  accepts, so a loaded problem is indistinguishable from a compiled-in one downstream. */
using GIndividualFactoryPtr = std::shared_ptr<Gem::Common::GFactoryT<Genome::GOptimizableEntity>>;

/** @brief The signatures of the two plugin entry points (used by the loader's typed symbol lookup). */
using geneva_individual_abi_version_fn = std::uint32_t();
using geneva_individual_factory_fn = GIndividualFactoryPtr();

/******************************************************************************/

} /* namespace Gem::Geneva */

/******************************************************************************/
/**
 * @brief Turns a translation unit into a runtime-loadable Geneva individual (optimization-problem) plugin.
 *
 * A user who wants their optimization problem loadable at runtime (so Geneva need not be recompiled to run
 * a new problem) writes exactly ONE line in the plugin's source, passing their content-creator FACTORY
 * type (a Gem::Common::GFactoryT<GOptimizableEntity>, e.g. a GFlatIndividualFactory<MyProblem>):
 *
 * @code
 *   GENEVA_INDIVIDUAL_PLUGIN(GFlatIndividualFactory<MyProblem>)
 * @endcode
 *
 * The macro emits, from compile-time constants, the two @c extern @c "C" entry points the loader expects:
 *  - @c geneva_individual_abi_version() returning @c GENEVA_VERSION (the value baked in at PLUGIN build
 *    time). The loader compares it to its own @c GENEVA_VERSION and refuses a mismatch with a precise
 *    message, so a plugin built against an older/incompatible Geneva is rejected at load rather than
 *    mis-loaded (Boost.DLL does not check this itself; a missing symbol is its only free failure mode,
 *    which is the backstop for a .so predating this scheme).
 *  - @c geneva_make_individual() returning a fresh factory, constructed with @p ConfigPath. For the
 *    standard @c GFlatIndividualFactory the path names the problem's config file; a missing file is
 *    auto-created with the individual's in-source defaults, so a self-contained problem still works.
 *
 * The macro does NOT emit the individual's @c BOOST_CLASS_EXPORT: the individual type already carries it
 * (the same registration a compiled-in individual needs for wire/checkpoint transport), so emitting it
 * here would duplicate the symbol.
 *
 * @param FactoryType The content-creator factory type (a GFactoryT<GOptimizableEntity>, e.g.
 *        GFlatIndividualFactory<MyProblem>)
 * @param ConfigPath  The factory's configuration-file path (a string literal), passed to its constructor
 */
// geneva_make_individual() returns a std::shared_ptr (a non-C type) with C linkage. That is deliberate and
// safe here: the extern "C" only buys an un-mangled symbol for the loader's dlsym-style lookup, and a plugin
// is ALWAYS loaded by a host built with the same C++ toolchain and ABI (server and client are the same
// binary loading the same .so). Clang still warns (-Wreturn-type-c-linkage); silence just that false
// positive, at the single macro that emits the entry point, for clang only (gcc does not warn).
#if defined(__clang__)
#define GENEVA_PLUGIN_C_LINKAGE_PUSH                                                                     \
    _Pragma("clang diagnostic push") _Pragma("clang diagnostic ignored \"-Wreturn-type-c-linkage\"")
#define GENEVA_PLUGIN_C_LINKAGE_POP _Pragma("clang diagnostic pop")
#else
#define GENEVA_PLUGIN_C_LINKAGE_PUSH
#define GENEVA_PLUGIN_C_LINKAGE_POP
#endif

#define GENEVA_INDIVIDUAL_PLUGIN(FactoryType, ConfigPath)                                                \
    extern "C" BOOST_SYMBOL_EXPORT std::uint32_t geneva_individual_abi_version();                        \
    extern "C" BOOST_SYMBOL_EXPORT std::uint32_t geneva_individual_abi_version() {                       \
        return static_cast<std::uint32_t>(GENEVA_VERSION);                                               \
    }                                                                                                    \
    GENEVA_PLUGIN_C_LINKAGE_PUSH                                                                          \
    extern "C" BOOST_SYMBOL_EXPORT ::Gem::Geneva::GIndividualFactoryPtr geneva_make_individual();        \
    extern "C" BOOST_SYMBOL_EXPORT ::Gem::Geneva::GIndividualFactoryPtr geneva_make_individual() {       \
        return std::make_shared<FactoryType>(ConfigPath);                                                \
    }                                                                                                    \
    GENEVA_PLUGIN_C_LINKAGE_POP

/******************************************************************************/
