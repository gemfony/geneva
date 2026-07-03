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
 * @brief Turns GLoadableParaboloid into a runtime-loadable Geneva individual plugin.
 *
 * This whole translation unit is the "glue" a problem author writes to make an existing individual
 * loadable at runtime. It is exactly TWO lines of registration:
 *
 *  1. BOOST_CLASS_EXPORT(GLoadableParaboloid): registers the individual's serialization GUID so it can
 *     cross the networked wire and a checkpoint (the SAME registration a compiled-in individual needs; it
 *     lives here so it is compiled into the .so). Every node that (de)serializes this individual must have
 *     this plugin loaded -- which, since server and client are the same binary loading the same .so, is
 *     automatic.
 *
 *  2. GENEVA_INDIVIDUAL_PLUGIN(...): emits the two C entry points the loader resolves -- an ABI-version
 *     marker (baked in at build time, so an incompatible/older plugin is rejected at load) and a factory
 *     that produces the individuals. The factory type is the standard GFlatIndividualFactory<Derived>.
 */

#include <boost/serialization/export.hpp>

#include "geneva/ind/GFlatIndividualFactory.hpp"
#include "geneva/ind/GIndividualPlugin.hpp"

#include "GLoadableParaboloid.hpp"

// (1) Serialization GUID for wire / checkpoint transport of this individual.
BOOST_CLASS_EXPORT(GLoadableParaboloid) // NOLINT

// (2) Make this .so a loadable Geneva individual: its factory is the standard flat-individual factory
// parameterised on the problem type; the second argument is the problem's config file (auto-created with
// the individual's defaults if absent). That is the entire author-facing surface of the plugin mechanism.
GENEVA_INDIVIDUAL_PLUGIN(
    Gem::Geneva::Genome::GFlatIndividualFactory<GLoadableParaboloid>,
    "./config/GLoadableParaboloid.json")
