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
#include "geneva/individuals/GMetaOptimizerIndividualT.hpp"
#include "geneva/individuals/GTestIndividual3.hpp"
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
#include "geneva/individuals/GTestIndividual1.hpp"
#include "geneva/oa/GEvolutionaryAlgorithm_PersonalityTraits.hpp"
#include "geneva/oa/GGradientDescent_PersonalityTraits.hpp"
#include "geneva/oa/GParameterScan_PersonalityTraits.hpp"
#include "geneva/oa/GSimulatedAnnealing_PersonalityTraits.hpp"
#include "geneva/oa/GSwarmAlgorithm_PersonalityTraits.hpp"
// Optimization algorithms
#include "geneva/oa/GEvolutionaryAlgorithm.hpp"
#include "geneva/oa/GSimulatedAnnealing.hpp"
#include "geneva/oa/GSwarmAlgorithm.hpp"
#include "geneva/oa/GGradientDescent.hpp"
#include "geneva/oa/GConjugateGradientDescent.hpp"
#include "geneva/oa/GParameterScan.hpp"
#include "geneva/oa/GNelderMead.hpp"
// Constraints
#include "geneva/par/GParameterSetMultiConstraint.hpp"
// Pluggable optimization monitors
#include "geneva/GPluggableOptimizationMonitors.hpp"

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
    Gem::Geneva::Tests::StandardTests_no_failure_expected<TestType>();
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
    Gem::Geneva::Tests::StandardTests_no_failure_expected<TestType>();
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
    Gem::Geneva::Tests::StandardTests_no_failure_expected<TestType>();
}

TEMPLATE_TEST_CASE(
    "StandardTests_no_failure_expected — pod collection types",
    "[geneva][standard]",
    gpar::GInt32Collection,
    gpar::GDoubleCollection,
    gpar::GBooleanCollection,
    gpar::GConstrainedDoubleCollection
) {
    Gem::Geneva::Tests::StandardTests_no_failure_expected<TestType>();
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
    Gem::Geneva::Tests::StandardTests_no_failure_expected<TestType>();
}

TEMPLATE_TEST_CASE(
    "StandardTests_no_failure_expected — individual types",
    "[geneva][standard]",
    gind::GTestIndividual1,
    gind::GTestIndividual3,
    gind::GFunctionIndividual,
    gind::GDelayIndividual,
    gind::GExternalEvaluatorIndividual,
    gind::GMetaOptimizerIndividualT<gind::GFunctionIndividual>
) {
    Gem::Geneva::Tests::StandardTests_no_failure_expected<TestType>();
}

// NOTE: gpar::GParameterSetFormulaConstraint is the only concrete constraint type
// (GParameterSetConstraint is abstract). It is EXCLUDED here because its standard test
// fails on the XML (de-)serialization round-trip with "Invalid XML tag name" (TEXT and
// BINARY round-trip fine; the failure is independent of the formula content). This is a
// suspected real defect in its XML serialization path. See report.

TEMPLATE_TEST_CASE(
    "StandardTests_no_failure_expected — algorithm types",
    "[geneva][standard]",
    oa::GEvolutionaryAlgorithm,
    oa::GSimulatedAnnealing,
    oa::GSwarmAlgorithm,
    oa::GGradientDescent,
    oa::GConjugateGradientDescent,
    oa::GParameterScan,
    oa::GNelderMead
) {
    Gem::Geneva::Tests::StandardTests_no_failure_expected<TestType>();
}

TEMPLATE_TEST_CASE(
    "StandardTests_no_failure_expected — monitor types",
    "[geneva][standard]",
    GStandardMonitor,
    GFitnessMonitor,
    GCollectiveMonitor,
    GProgressPlotter,
    GAllSolutionFileLogger,
    GIterationResultsFileLogger,
    GNAdpationsLogger,
    GAdaptorPropertyLogger<double>,
    GProcessingTimesLogger
) {
    Gem::Geneva::Tests::StandardTests_no_failure_expected<TestType>();
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
    Gem::Geneva::Tests::StandardTests_failures_expected<TestType>();
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
    Gem::Geneva::Tests::StandardTests_failures_expected<TestType>();
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
    Gem::Geneva::Tests::StandardTests_failures_expected<TestType>();
}

TEMPLATE_TEST_CASE(
    "StandardTests_failures_expected — pod collection types",
    "[geneva][standard][failures-expected]",
    gpar::GInt32Collection,
    gpar::GDoubleCollection,
    gpar::GBooleanCollection,
    gpar::GConstrainedDoubleCollection
) {
    Gem::Geneva::Tests::StandardTests_failures_expected<TestType>();
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
    Gem::Geneva::Tests::StandardTests_failures_expected<TestType>();
}

TEMPLATE_TEST_CASE(
    "StandardTests_failures_expected — individual types",
    "[geneva][standard][failures-expected]",
    gind::GTestIndividual1,
    gind::GTestIndividual3,
    gind::GFunctionIndividual,
    gind::GDelayIndividual,
    gind::GExternalEvaluatorIndividual,
    gind::GMetaOptimizerIndividualT<gind::GFunctionIndividual>
) {
    Gem::Geneva::Tests::StandardTests_failures_expected<TestType>();
}

// NOTE: gpar::GParameterSetFormulaConstraint is excluded here (see no-failure-expected block).

TEMPLATE_TEST_CASE(
    "StandardTests_failures_expected — algorithm types",
    "[geneva][standard][failures-expected]",
    oa::GEvolutionaryAlgorithm,
    oa::GSimulatedAnnealing,
    oa::GSwarmAlgorithm,
    oa::GGradientDescent,
    oa::GConjugateGradientDescent,
    oa::GParameterScan,
    oa::GNelderMead
) {
    Gem::Geneva::Tests::StandardTests_failures_expected<TestType>();
}

TEMPLATE_TEST_CASE(
    "StandardTests_failures_expected — monitor types",
    "[geneva][standard][failures-expected]",
    GStandardMonitor,
    GFitnessMonitor,
    GCollectiveMonitor,
    GProgressPlotter,
    GAllSolutionFileLogger,
    GIterationResultsFileLogger,
    GNAdpationsLogger,
    GAdaptorPropertyLogger<double>,
    GProcessingTimesLogger
) {
    Gem::Geneva::Tests::StandardTests_failures_expected<TestType>();
}

// ============================================================================
// Targeted regression test: (de)serialization of GParameterSetFormulaConstraint
// ============================================================================
//
// GParameterSetConstraint::serialize() used BOOST_SERIALIZATION_BASE_OBJECT_NVP on
// the templated base GPreEvaluationValidityCheckT<GParameterSet>, producing an XML
// tag name containing '<' and '>' -> XML (de)serialization threw "Invalid XML tag
// name" (TEXT and BINARY were unaffected, as they ignore the NVP names). This type
// is not run through the templated standard test (it has no modify_GUnitTests_, so
// that test's round-trip block would be skipped), hence this explicit check that all
// three serialization modes round-trip without throwing.
TEST_CASE(
    "GParameterSetFormulaConstraint round-trips in TEXT, XML and BINARY",
    "[geneva][serialization]"
) {
    using Gem::Common::serializationMode;

    for (auto mode :
         {serializationMode::TEXT, serializationMode::XML, serializationMode::BINARY}) {
        gpar::GParameterSetFormulaConstraint original("1 + 2");
        // The default ctor is private (serialization-only); construct the target
        // through the public ctor with a different formula, then load into it.
        gpar::GParameterSetFormulaConstraint restored("0");

        REQUIRE_NOTHROW(
            restored.GObject::fromString(original.GObject::toString(mode), mode)
        );

        GEqualityPrinter gep(
            "GParameterSetFormulaConstraint-roundtrip",
            pow(10, -7),
            Gem::Common::CE_WITH_MESSAGES
        );
        CHECK(gep.isSimilar(restored, original));
    }
}
