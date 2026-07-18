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

/**
 * @file
 * @brief Module "glue" that makes GParaboloidIndividual2D a runtime-loadable Geneva individual.
 *
 * This builds the GParaboloid2D-module artifact (libGParaboloid2D-module.so), the loadable-module packaging
 * of the same problem the GParaboloid2D-fixed executable compiles in; the generic optimizer (example 19's
 * GGenericOptimizer) runs it via --individual. This translation unit holds only the fixed extern "C" entry
 * point geneva_module_manifest(); it does NOT re-emit GParaboloidIndividual2D's BOOST_CLASS_EXPORT (that
 * lives in GParaboloidIndividual2D.cpp, compiled into both artifacts).
 */

#include <boost/config.hpp> // BOOST_SYMBOL_EXPORT

#include "common/GModuleManifest.hpp" // GenevaModuleManifest
#include "geneva/ind/GIndividualFactory.hpp"
#include "geneva/ind/GIndividualPlugin.hpp" // Gem::Geneva::individualManifest<>

#include "GParaboloidIndividual2D.hpp"

extern "C" BOOST_SYMBOL_EXPORT const GenevaModuleManifest *geneva_module_manifest();
extern "C" BOOST_SYMBOL_EXPORT const GenevaModuleManifest *geneva_module_manifest() {
    return Gem::Geneva::individualManifest<
        Gem::Geneva::Genome::GIndividualFactory<Gem::Geneva::GParaboloidIndividual2D>,
        "./config/GParaboloidIndividual2D.json", "GParaboloidIndividual2D">();
}
