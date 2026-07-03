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

// Geneva headers go here
#include "geneva/ind/GIndividualPlugin.hpp" // the plugin contract (entry-point names + factory type)

namespace Gem::Geneva {

/******************************************************************************/
/**
 * @brief Loads a runtime individual (optimization-problem) plugin from @p plugin_path and returns its
 * content-creator factory, so a problem can be supplied without recompiling Geneva.
 *
 * The plugin is a shared object built with the GENEVA_INDIVIDUAL_PLUGIN() macro. This function:
 *  - loads it with @c RTLD_GLOBAL (one symbol namespace: a single Boost.Serialization registry and single
 *    Geneva singletons across the process) and @c RTLD_NOW (eager resolution);
 *  - **verifies the plugin's ABI version** (its baked-in @c GENEVA_VERSION) against this Geneva's and
 *    throws a precise message on a mismatch or on a missing ABI marker -- BEFORE any individual is
 *    constructed, so an incompatible/older plugin is rejected rather than mis-loaded;
 *  - returns the factory from the plugin's @c geneva_make_individual() entry point.
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
