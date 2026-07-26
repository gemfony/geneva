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
 * @brief The one place where Geneva's built-in optimization algorithms make themselves selectable.
 *
 * Every algorithm the library ships registers its factory here, at library-load time, through the same
 * function a runtime-loaded algorithm module goes through: Gem::Geneva::registerOptimizationAlgorithm().
 * There is no per-algorithm registration translation unit any more -- this file is the complete list of
 * mnemonics a stock Geneva offers, and it doubles as the build's algorithm inventory (the optional
 * algorithms appear here exactly under the GENEVA_ALGO_* switch that puts their sources into the library).
 *
 * Geneva is always built as a shared library, so these load-time initializers are never stripped.
 */

// Geneva headers go here
#include "geneva/oa/GInitializerT.hpp"

#include "geneva/oa/GAntColonyOptimizationFactory.hpp"
#include "geneva/oa/GEvolutionaryAlgorithmFactory.hpp"
#include "geneva/oa/GGeneralizedSimulatedAnnealingFactory.hpp"
#include "geneva/oa/GSepCmaEvolutionStrategyFactory.hpp"
#include "geneva/oa/GStandardPSO2011Factory.hpp"

#ifdef GENEVA_ALGO_CGD
#include "geneva/oa/GConjugateGradientDescentFactory.hpp"
#endif
#ifdef GENEVA_ALGO_NM
#include "geneva/oa/GNelderMeadFactory.hpp"
#endif
#ifdef GENEVA_ALGO_PS
#include "geneva/oa/GParameterScanFactory.hpp"
#endif
#ifdef GENEVA_ALGO_SA
#include "geneva/oa/GSimulatedAnnealingFactory.hpp"
#endif
#ifdef GENEVA_ALGO_SWARM
#include "geneva/oa/GSwarmAlgorithmFactory.hpp"
#endif

namespace Gem::Geneva::OptimizationAlgorithms {

namespace {

/******************************************************************************/
// The algorithms that are always part of Geneva.

const GInitializerT<GAntColonyOptimizationFactory> g_aco_registrant;
const GInitializerT<GEvolutionaryAlgorithmFactory> g_ea_registrant;
const GInitializerT<GGeneralizedSimulatedAnnealingFactory> g_gsa_registrant;
const GInitializerT<GSepCmaEvolutionStrategyFactory> g_sepcma_registrant;
const GInitializerT<GStandardPSO2011Factory> g_pso_registrant;

/******************************************************************************/
// The algorithms a slim-core build may leave out. Each switch is set by the build for exactly the
// algorithms whose sources went into the library, so an omitted algorithm's mnemonic simply does not
// exist at runtime.

#ifdef GENEVA_ALGO_CGD
const GInitializerT<GConjugateGradientDescentFactory> g_cgd_registrant;
#endif
#ifdef GENEVA_ALGO_NM
const GInitializerT<GNelderMeadFactory> g_nm_registrant;
#endif
#ifdef GENEVA_ALGO_PS
const GInitializerT<GParameterScanFactory> g_ps_registrant;
#endif
#ifdef GENEVA_ALGO_SA
const GInitializerT<GSimulatedAnnealingFactory> g_sa_registrant;
#endif
#ifdef GENEVA_ALGO_SWARM
const GInitializerT<GSwarmAlgorithmFactory> g_swarm_registrant;
#endif

/******************************************************************************/

} // anonymous namespace

} /* namespace Gem::Geneva::OptimizationAlgorithms */
