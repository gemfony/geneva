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

// Boost header files go here

// Geneva headers go here
#include "geneva/oa/GStandardPSO2011.hpp"
#include "geneva/oa/GStandardPSO2011_PersonalityTraits.hpp"
#include "geneva/oa/GOptimizationAlgorithmFactoryT.hpp"

namespace Gem::Geneva::OptimizationAlgorithms {

/******************************************************************************/
/**
 * @brief The factory that builds the standard particle-swarm optimizer (PSO 2011) from a configuration file.
 *
 * The GOptimizationAlgorithmFactoryT scaffold generates everything such a factory needs -- the three
 * constructors (the default one deriving the config path "./config/GStandardPSO2011.json"), getMnemonic()
 * (the personality nickname), getAlgorithmName() and getObject_() -- so the factory is that instantiation,
 * under a name callers can spell. Registration with the global algorithm store happens once, for every
 * built-in algorithm together, in src/oa/GBuiltinAlgorithms.cpp.
 */
using GStandardPSO2011Factory = GOptimizationAlgorithmFactoryT<GStandardPSO2011, GStandardPSO2011_PersonalityTraits>;

/******************************************************************************/

} /* namespace Gem::Geneva::OptimizationAlgorithms */
