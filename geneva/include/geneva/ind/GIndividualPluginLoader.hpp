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
#include <string>

// Geneva headers go here
#include "common/GModuleManifest.hpp"       // GenevaModuleManifest (the unified module manifest)
#include "geneva/ind/GIndividualPlugin.hpp" // the plugin contract (entry-point names + factory type)

namespace Gem::Geneva {

/******************************************************************************/
/**
 * @brief Loads a runtime Geneva module from @p module_path, validates its toolchain-compatibility
 * fingerprint (`GenevaCompat`) against this host, keeps it resident and returns its manifest.
 *
 * This is the general module entry point (individuals, and -- as further categories are modularised --
 * optimization algorithms and consumers). It:
 *  - loads the shared object with @c RTLD_GLOBAL (one symbol namespace: a single Boost.Serialization
 *    registry and single Geneva singletons) and @c RTLD_NOW (eager resolution);
 *  - resolves @c geneva_module_manifest() and validates its @c GenevaCompat **before touching any C++
 *    contribution** -- a compiler/stdlib/Boost/build-mode mismatch is rejected with a diagnostic naming the
 *    offending axis, rather than being mis-loaded and crashing later;
 *  - keeps the library resident for the process lifetime (its code + Boost.Serialization registrations back
 *    live objects) and never unloads it.
 *
 * The returned manifest (and everything it points to) lives for the process lifetime.
 *
 * @param module_path The filesystem path to the module shared object
 * @return The module's manifest (never nullptr; throws on any failure, incl. a missing manifest)
 */
const GenevaModuleManifest *loadModule(const std::filesystem::path &module_path);

/******************************************************************************/
/** @brief This host's own toolchain-compatibility fingerprint (built from the host's predefined macros).
 *  Exposed for diagnostics and tests. */
const GenevaCompat &thisHostCompat();

/**
 * @brief Returns an empty string if @p moduleCompat is ABI-compatible with this host, otherwise a
 * diagnostic naming the first offending axis. This is the exact check the loader applies to a module's
 * manifest before touching any of its C++ contributions; exposed so tests can verify the gate without a
 * shared object.
 */
std::string moduleCompatMismatch(const GenevaCompat &moduleCompat);

/******************************************************************************/
/**
 * @brief Loads a runtime individual (optimization-problem) plugin from @p plugin_path and returns its
 * content-creator factory, so a problem can be supplied without recompiling Geneva.
 *
 * The plugin is a shared object built with the GENEVA_INDIVIDUAL_PLUGIN() macro. This function:
 *  - loads it with @c RTLD_GLOBAL (one symbol namespace: a single Boost.Serialization registry and single
 *    Geneva singletons across the process) and @c RTLD_NOW (eager resolution);
 *  - if the plugin exports the unified manifest, **validates its full toolchain fingerprint**
 *    (`GenevaCompat`: compiler/stdlib/`_GLIBCXX_USE_CXX11_ABI`/Boost/build-mode/Geneva-version) against
 *    this host and rejects a mismatch with a diagnostic naming the offending axis -- BEFORE any individual
 *    is constructed. A plugin that predates the manifest (legacy two-symbol convention) falls back to the
 *    weaker @c GENEVA_VERSION-only gate for one release;
 *  - returns the factory from the plugin's @c INDIVIDUAL contribution (or the legacy
 *    @c geneva_make_individual() entry point).
 *
 * The loaded library is kept resident for the entire process lifetime (its code backs live individuals and
 * its Boost.Serialization type registrations); it is deliberately never unloaded.
 *
 * @param plugin_path The filesystem path to the individual plugin shared object
 * @return The plugin's content-creator factory (never empty; throws on any failure)
 */
GIndividualFactoryPtr loadIndividualPlugin(const std::filesystem::path &plugin_path);

/******************************************************************************/

} /* namespace Gem::Geneva */
