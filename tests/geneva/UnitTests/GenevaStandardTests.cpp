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
#include "geneva/individuals/GDelayIndividual.hpp"
#include "geneva/individuals/GExternalEvaluatorIndividual.hpp"
#include "geneva/individuals/GFunctionIndividual.hpp"
#include "geneva/GTestIndividual3.hpp"
#include "geneva/par/GBooleanAdaptor.hpp"
#include "geneva/par/GBooleanCollection.hpp"
#include "geneva/par/GBooleanObject.hpp"
#include "geneva/par/GBooleanObjectCollection.hpp"
#include "geneva/par/GConstrainedDoubleCollection.hpp"
#include "geneva/par/GConstrainedDoubleObject.hpp"
#include "geneva/par/GConstrainedDoubleObjectCollection.hpp"
#include "geneva/par/GConstrainedInt32Object.hpp"
#include "geneva/par/GConstrainedInt32ObjectCollection.hpp"
#include "geneva/par/GDoubleBiGaussAdaptor.hpp"
#include "geneva/par/GDoubleCollection.hpp"
#include "geneva/par/GDoubleGaussAdaptor.hpp"
#include "geneva/par/GDoubleObject.hpp"
#include "geneva/par/GDoubleObjectCollection.hpp"
#include "geneva/par/GInt32Collection.hpp"
#include "geneva/par/GInt32FlipAdaptor.hpp"
#include "geneva/par/GInt32GaussAdaptor.hpp"
#include "geneva/par/GInt32Object.hpp"
#include "geneva/par/GInt32ObjectCollection.hpp"
#include "geneva/par/GParameterObjectCollection.hpp"
#include "geneva/GTestIndividual1.hpp"
#include "geneva/oa/GEvolutionaryAlgorithm_PersonalityTraits.hpp"
#include "geneva/oa/GGradientDescent_PersonalityTraits.hpp"
#include "geneva/oa/GParameterScan_PersonalityTraits.hpp"
#include "geneva/oa/GSimulatedAnnealing_PersonalityTraits.hpp"
#include "geneva/oa/GSwarmAlgorithm_PersonalityTraits.hpp"

#include "Geneva_tests.hpp"

using namespace Gem::Geneva;

// ============================================================================
// Standard tests — no failure expected
// ============================================================================

TEMPLATE_TEST_CASE(
    "StandardTests_no_failure_expected — adaptor types",
    "[geneva][standard]",
    gpar::GInt32FlipAdaptor,
    gpar::GBooleanAdaptor,
    gpar::GInt32GaussAdaptor,
    gpar::GDoubleBiGaussAdaptor,
    gpar::GDoubleGaussAdaptor
) {
    Gem::Tests::StandardTests_no_failure_expected<TestType>();
}

TEMPLATE_TEST_CASE(
    "StandardTests_no_failure_expected — data types",
    "[geneva][standard]",
    gpar::GBooleanObject,
    gpar::GInt32Object,
    gpar::GDoubleObject,
    gpar::GConstrainedInt32Object,
    gpar::GConstrainedDoubleObject
) {
    Gem::Tests::StandardTests_no_failure_expected<TestType>();
}

TEMPLATE_TEST_CASE(
    "StandardTests_no_failure_expected — object collection types",
    "[geneva][standard]",
    gpar::GParameterObjectCollection,
    gpar::GBooleanObjectCollection,
    gpar::GInt32ObjectCollection,
    gpar::GConstrainedInt32ObjectCollection,
    gpar::GDoubleObjectCollection,
    gpar::GConstrainedDoubleObjectCollection
) {
    Gem::Tests::StandardTests_no_failure_expected<TestType>();
}

TEMPLATE_TEST_CASE(
    "StandardTests_no_failure_expected — pod collection types",
    "[geneva][standard]",
    gpar::GInt32Collection,
    gpar::GDoubleCollection,
    gpar::GBooleanCollection,
    gpar::GConstrainedDoubleCollection
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
    gind::GFunctionIndividual,
    gind::GDelayIndividual,
    gind::GExternalEvaluatorIndividual
) {
    Gem::Tests::StandardTests_no_failure_expected<TestType>();
}

// ============================================================================
// Standard tests — failures expected
// ============================================================================

TEMPLATE_TEST_CASE(
    "StandardTests_failures_expected — adaptor types",
    "[geneva][standard][failures-expected]",
    gpar::GInt32FlipAdaptor,
    gpar::GBooleanAdaptor,
    gpar::GInt32GaussAdaptor,
    gpar::GDoubleBiGaussAdaptor,
    gpar::GDoubleGaussAdaptor
) {
    Gem::Tests::StandardTests_failures_expected<TestType>();
}

TEMPLATE_TEST_CASE(
    "StandardTests_failures_expected — data types",
    "[geneva][standard][failures-expected]",
    gpar::GBooleanObject,
    gpar::GInt32Object,
    gpar::GDoubleObject,
    gpar::GConstrainedInt32Object,
    gpar::GConstrainedDoubleObject
) {
    Gem::Tests::StandardTests_failures_expected<TestType>();
}

TEMPLATE_TEST_CASE(
    "StandardTests_failures_expected — object collection types",
    "[geneva][standard][failures-expected]",
    gpar::GParameterObjectCollection,
    gpar::GBooleanObjectCollection,
    gpar::GInt32ObjectCollection,
    gpar::GConstrainedInt32ObjectCollection,
    gpar::GDoubleObjectCollection,
    gpar::GConstrainedDoubleObjectCollection
) {
    Gem::Tests::StandardTests_failures_expected<TestType>();
}

TEMPLATE_TEST_CASE(
    "StandardTests_failures_expected — pod collection types",
    "[geneva][standard][failures-expected]",
    gpar::GInt32Collection,
    gpar::GDoubleCollection,
    gpar::GBooleanCollection,
    gpar::GConstrainedDoubleCollection
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
    gind::GFunctionIndividual,
    gind::GDelayIndividual,
    gind::GExternalEvaluatorIndividual
) {
    Gem::Tests::StandardTests_failures_expected<TestType>();
}
