/**
 * @file GenevaStandardTests.cpp
 */

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

#include <catch2/catch_template_test_macros.hpp>

// All classes that will be tested in this file
#include "geneva-individuals/GDelayIndividual.hpp"
#include "geneva-individuals/GExternalEvaluatorIndividual.hpp"
#include "geneva-individuals/GFunctionIndividual.hpp"
#include "geneva-individuals/GTestIndividual3.hpp"
#include "geneva/GBooleanAdaptor.hpp"
#include "geneva/GBooleanCollection.hpp"
#include "geneva/GBooleanObject.hpp"
#include "geneva/GBooleanObjectCollection.hpp"
#include "geneva/GConstrainedDoubleCollection.hpp"
#include "geneva/GConstrainedDoubleObject.hpp"
#include "geneva/GConstrainedDoubleObjectCollection.hpp"
#include "geneva/GConstrainedInt32Object.hpp"
#include "geneva/GConstrainedInt32ObjectCollection.hpp"
#include "geneva/GDoubleBiGaussAdaptor.hpp"
#include "geneva/GDoubleCollection.hpp"
#include "geneva/GDoubleGaussAdaptor.hpp"
#include "geneva/GDoubleObject.hpp"
#include "geneva/GDoubleObjectCollection.hpp"
#include "geneva/GInt32Collection.hpp"
#include "geneva/GInt32FlipAdaptor.hpp"
#include "geneva/GInt32GaussAdaptor.hpp"
#include "geneva/GInt32Object.hpp"
#include "geneva/GInt32ObjectCollection.hpp"
#include "geneva/GParameterObjectCollection.hpp"
#include "geneva/GTestIndividual1.hpp"
#include "geneva/EvolutionaryAlgorithm_PersonalityTraits.hpp"
#include "geneva/GradientDescent_PersonalityTraits.hpp"
#include "geneva/ParameterScan_PersonalityTraits.hpp"
#include "geneva/SimulatedAnnealing_PersonalityTraits.hpp"
#include "geneva/SwarmAlgorithm_PersonalityTraits.hpp"

#include "Geneva_tests.hpp"

using namespace Gem::Geneva;

// ============================================================================
// Standard tests — no failure expected
// ============================================================================

TEMPLATE_TEST_CASE(
    "StandardTests_no_failure_expected — adaptor types",
    "[geneva][standard]",
    GInt32FlipAdaptor,
    GBooleanAdaptor,
    GInt32GaussAdaptor,
    GDoubleBiGaussAdaptor,
    GDoubleGaussAdaptor
) {
    Gem::Tests::StandardTests_no_failure_expected<TestType>();
}

TEMPLATE_TEST_CASE(
    "StandardTests_no_failure_expected — data types",
    "[geneva][standard]",
    GBooleanObject,
    GInt32Object,
    GDoubleObject,
    GConstrainedInt32Object,
    GConstrainedDoubleObject
) {
    Gem::Tests::StandardTests_no_failure_expected<TestType>();
}

TEMPLATE_TEST_CASE(
    "StandardTests_no_failure_expected — object collection types",
    "[geneva][standard]",
    GParameterObjectCollection,
    GBooleanObjectCollection,
    GInt32ObjectCollection,
    GConstrainedInt32ObjectCollection,
    GDoubleObjectCollection,
    GConstrainedDoubleObjectCollection
) {
    Gem::Tests::StandardTests_no_failure_expected<TestType>();
}

TEMPLATE_TEST_CASE(
    "StandardTests_no_failure_expected — pod collection types",
    "[geneva][standard]",
    GInt32Collection,
    GDoubleCollection,
    GBooleanCollection,
    GConstrainedDoubleCollection
) {
    Gem::Tests::StandardTests_no_failure_expected<TestType>();
}

TEMPLATE_TEST_CASE(
    "StandardTests_no_failure_expected — trait types",
    "[geneva][standard]",
    oa::GEvolutionaryAlgorithm_PersonalityTraits,
    oa::GGradientDescent_PersonalityTraits,
    oa::GSwarmAlgorithm_PersonalityTraits,
    oa::GSimulatedAnnealing_PersonalityTraits,
    oa::GParameterScan_PersonalityTraits
) {
    Gem::Tests::StandardTests_no_failure_expected<TestType>();
}

TEMPLATE_TEST_CASE(
    "StandardTests_no_failure_expected — individual types",
    "[geneva][standard]",
    Gem::Tests::GTestIndividual1,
    // Gem::Tests::GTestIndividual3, // TODO: Add test for GTestIndividual3
    GFunctionIndividual,
    GDelayIndividual,
    GExternalEvaluatorIndividual
) {
    Gem::Tests::StandardTests_no_failure_expected<TestType>();
}

// ============================================================================
// Standard tests — failures expected
// ============================================================================

TEMPLATE_TEST_CASE(
    "StandardTests_failures_expected — adaptor types",
    "[geneva][standard][failures-expected]",
    GInt32FlipAdaptor,
    GBooleanAdaptor,
    GInt32GaussAdaptor,
    GDoubleBiGaussAdaptor,
    GDoubleGaussAdaptor
) {
    Gem::Tests::StandardTests_failures_expected<TestType>();
}

TEMPLATE_TEST_CASE(
    "StandardTests_failures_expected — data types",
    "[geneva][standard][failures-expected]",
    GBooleanObject,
    GInt32Object,
    GDoubleObject,
    GConstrainedInt32Object,
    GConstrainedDoubleObject
) {
    Gem::Tests::StandardTests_failures_expected<TestType>();
}

TEMPLATE_TEST_CASE(
    "StandardTests_failures_expected — object collection types",
    "[geneva][standard][failures-expected]",
    GParameterObjectCollection,
    GBooleanObjectCollection,
    GInt32ObjectCollection,
    GConstrainedInt32ObjectCollection,
    GDoubleObjectCollection,
    GConstrainedDoubleObjectCollection
) {
    Gem::Tests::StandardTests_failures_expected<TestType>();
}

TEMPLATE_TEST_CASE(
    "StandardTests_failures_expected — pod collection types",
    "[geneva][standard][failures-expected]",
    GInt32Collection,
    GDoubleCollection,
    GBooleanCollection,
    GConstrainedDoubleCollection
) {
    Gem::Tests::StandardTests_failures_expected<TestType>();
}

TEMPLATE_TEST_CASE(
    "StandardTests_failures_expected — trait types",
    "[geneva][standard][failures-expected]",
    oa::GEvolutionaryAlgorithm_PersonalityTraits,
    oa::GGradientDescent_PersonalityTraits,
    oa::GSwarmAlgorithm_PersonalityTraits,
    oa::GSimulatedAnnealing_PersonalityTraits,
    oa::GParameterScan_PersonalityTraits
) {
    Gem::Tests::StandardTests_failures_expected<TestType>();
}

TEMPLATE_TEST_CASE(
    "StandardTests_failures_expected — individual types",
    "[geneva][standard][failures-expected]",
    Gem::Tests::GTestIndividual1,
    // Gem::Tests::GTestIndividual3, // TODO: Add test for GTestIndividual3
    GFunctionIndividual,
    GDelayIndividual,
    GExternalEvaluatorIndividual
) {
    Gem::Tests::StandardTests_failures_expected<TestType>();
}
