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

// Geneva headers go here
#include "geneva/oa/GOptimizationAlgorithmFactoryT.hpp"

#include "GRandomSearch.hpp"
#include "GRandomSearch_PersonalityTraits.hpp"

namespace Gem::Geneva::OptimizationAlgorithms {

/******************************************************************************/
/**
 * @brief The factory that builds GRandomSearch algorithms from a configuration file.
 *
 * This is the whole factory an optimization-algorithm module needs. The GOptimizationAlgorithmFactoryT
 * scaffold generates the config path ("./config/GRandomSearch.json"), getMnemonic() ("rsearch", from the
 * personality traits), getAlgorithmName(), getObject_() and the three constructors; and because a factory
 * IS the provider the algorithm store holds (GOAFactoryT implements Gem::Common::GProviderT), nothing has
 * to be wrapped to register it. An algorithm that needs more -- extra command-line options, a
 * postProcess_() step -- makes this instantiation its base class instead (see GParameterScanFactory in the
 * Geneva library).
 */
using GRandomSearchFactory = GOptimizationAlgorithmFactoryT<GRandomSearch>;

/******************************************************************************/

} /* namespace Gem::Geneva::OptimizationAlgorithms */
