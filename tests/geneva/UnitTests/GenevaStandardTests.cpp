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
#include "geneva/individuals/GLineFitIndividual.hpp"
#include "geneva/individuals/GMetaOptimizerIndividualT.hpp"
#include "geneva/individuals/GTestIndividual2.hpp"
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
// TFactory_GUnitTests specializations for classes without a public default
// constructor. The primary template (in common/GUnitTestFrameworkT.hpp) is a
// free function template at global scope; these specializations must be
// visible at the point the standard test is instantiated below.
// ============================================================================

// gind::GTestIndividual2 — public ctor (size, PERFOBJECTTYPE).
template <>
inline std::shared_ptr<gind::GTestIndividual2>
TFactory_GUnitTests<gind::GTestIndividual2>() {
    std::shared_ptr<gind::GTestIndividual2> p;
    CHECK_NOTHROW(
        p = std::make_shared<gind::GTestIndividual2>(
            std::size_t(100),
            gind::PERFOBJECTTYPE::PERFGDOUBLEOBJECT
        )
    );
    return p;
}

// gind::GLineFitIndividual — public ctor takes a vector of {x,y} sample points.
template <>
inline std::shared_ptr<gind::GLineFitIndividual>
TFactory_GUnitTests<gind::GLineFitIndividual>() {
    std::vector<std::tuple<double, double>> data_points{
        {0., 0.}, {1., 1.}, {2., 2.}, {3., 3.}};
    std::shared_ptr<gind::GLineFitIndividual> p;
    CHECK_NOTHROW(p = std::make_shared<gind::GLineFitIndividual>(data_points));
    return p;
}

// gind::GOptOptMonitorT<gind::GFunctionIndividual> — public ctor takes a file name.
template <>
inline std::shared_ptr<gind::GOptOptMonitorT<gind::GFunctionIndividual>>
TFactory_GUnitTests<gind::GOptOptMonitorT<gind::GFunctionIndividual>>() {
    std::shared_ptr<gind::GOptOptMonitorT<gind::GFunctionIndividual>> p;
    CHECK_NOTHROW(
        p = std::make_shared<gind::GOptOptMonitorT<gind::GFunctionIndividual>>(
            std::string("optOptMonitorResult.C")
        )
    );
    return p;
}

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
    gind::GTestIndividual2,
    gind::GTestIndividual3,
    gind::GLineFitIndividual,
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

// The concrete GParameterSetConstraint subclasses defined in GFunctionIndividual.hpp.
// They have public default + copy ctors and are tie-converted (they declare
// localMembers()), so clone/copy/load/compare are exercised. They do not override
// modify_GUnitTests_, so the standard test's (de)serialization round-trip block is
// skipped here (it is covered separately by the focused round-trip TEST_CASEs below).
TEMPLATE_TEST_CASE(
    "StandardTests_no_failure_expected — constraint types",
    "[geneva][standard]",
    gind::GDoubleSumConstraint,
    gind::GSphereConstraint,
    gind::GDoubleSumGapConstraint
) {
    Gem::Geneva::Tests::StandardTests_no_failure_expected<TestType>();
}

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
    GProcessingTimesLogger,
    gind::GOptOptMonitorT<gind::GFunctionIndividual>
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
    gind::GTestIndividual2,
    gind::GTestIndividual3,
    gind::GLineFitIndividual,
    gind::GFunctionIndividual,
    gind::GDelayIndividual,
    gind::GExternalEvaluatorIndividual,
    gind::GMetaOptimizerIndividualT<gind::GFunctionIndividual>
) {
    Gem::Geneva::Tests::StandardTests_failures_expected<TestType>();
}

// NOTE: gpar::GParameterSetFormulaConstraint is excluded here (see no-failure-expected block).

TEMPLATE_TEST_CASE(
    "StandardTests_failures_expected — constraint types",
    "[geneva][standard][failures-expected]",
    gind::GDoubleSumConstraint,
    gind::GSphereConstraint,
    gind::GDoubleSumGapConstraint
) {
    Gem::Geneva::Tests::StandardTests_failures_expected<TestType>();
}

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
    GProcessingTimesLogger,
    gind::GOptOptMonitorT<gind::GFunctionIndividual>
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

// ============================================================================
// Targeted (de)serialization round-trips for the concrete GParameterSetConstraint
// subclasses in GFunctionIndividual.hpp. These types are run through the templated
// standard test for clone/copy/load/compare, but that test skips the round-trip block
// (they do not override modify_GUnitTests_), so the serialize path is validated here.
// ============================================================================
TEST_CASE(
    "GDoubleSumConstraint round-trips in TEXT, XML and BINARY",
    "[geneva][serialization]"
) {
    using Gem::Common::serializationMode;

    for (auto mode :
         {serializationMode::TEXT, serializationMode::XML, serializationMode::BINARY}) {
        // Non-default value so the round-trip actually exercises the member
        // (the default is 1.0; a serialize that dropped it would still pass at 1.0).
        gind::GDoubleSumConstraint original(3.5);
        gind::GDoubleSumConstraint restored(2.0);

        REQUIRE_NOTHROW(
            restored.GObject::fromString(original.GObject::toString(mode), mode)
        );

        GEqualityPrinter gep(
            "GDoubleSumConstraint-roundtrip",
            pow(10, -7),
            Gem::Common::CE_WITH_MESSAGES
        );
        CHECK(gep.isSimilar(restored, original));
    }
}

TEST_CASE(
    "GSphereConstraint round-trips in TEXT, XML and BINARY",
    "[geneva][serialization]"
) {
    using Gem::Common::serializationMode;

    for (auto mode :
         {serializationMode::TEXT, serializationMode::XML, serializationMode::BINARY}) {
        // Non-default value (default 1.0): catches the previously-latent bug where
        // GSphereConstraint::serialize() did not store diameter_ at all.
        gind::GSphereConstraint original(3.5);
        gind::GSphereConstraint restored(2.0);

        REQUIRE_NOTHROW(
            restored.GObject::fromString(original.GObject::toString(mode), mode)
        );

        GEqualityPrinter gep(
            "GSphereConstraint-roundtrip",
            pow(10, -7),
            Gem::Common::CE_WITH_MESSAGES
        );
        CHECK(gep.isSimilar(restored, original));
    }
}

TEST_CASE(
    "GDoubleSumGapConstraint round-trips in TEXT, XML and BINARY",
    "[geneva][serialization]"
) {
    using Gem::Common::serializationMode;

    for (auto mode :
         {serializationMode::TEXT, serializationMode::XML, serializationMode::BINARY}) {
        // Non-default values (defaults 1.0 / 0.5) so both members are exercised.
        gind::GDoubleSumGapConstraint original(3.5, 1.25);
        gind::GDoubleSumGapConstraint restored(2.0, 0.25);

        REQUIRE_NOTHROW(
            restored.GObject::fromString(original.GObject::toString(mode), mode)
        );

        GEqualityPrinter gep(
            "GDoubleSumGapConstraint-roundtrip",
            pow(10, -7),
            Gem::Common::CE_WITH_MESSAGES
        );
        CHECK(gep.isSimilar(restored, original));
    }
}

// Regression: GExternalEvaluatorIndividual::serialize() previously omitted run_id_,
// so it was silently lost on (de)serialization. A non-default run_id_ must survive a
// round-trip in all three modes.
TEST_CASE(
    "GExternalEvaluatorIndividual round-trips run_id_ in TEXT, XML and BINARY",
    "[geneva][serialization]"
) {
    using Gem::Common::serializationMode;

    for (auto mode :
         {serializationMode::TEXT, serializationMode::XML, serializationMode::BINARY}) {
        gind::GExternalEvaluatorIndividual original;
        original.setRunId("custom-run-id-xyz");
        gind::GExternalEvaluatorIndividual restored;
        restored.setRunId("other-run-id");

        REQUIRE_NOTHROW(
            restored.GObject::fromString(original.GObject::toString(mode), mode)
        );

        GEqualityPrinter gep(
            "GExternalEvaluatorIndividual-runid-roundtrip",
            pow(10, -7),
            Gem::Common::CE_WITH_MESSAGES
        );
        CHECK(gep.isSimilar(restored, original));
    }
}

// NOTE: GNeuralNetworkIndividual's t_f_ (de)serialization fix (split save()/load()
// now route t_f_ through serialize_members(localMembers())) is NOT unit-tested
// here: a round-trip calls load(), which reloads the training data from disk via a
// global singleton (./Datasets/*.dat) that is absent in the unit-test environment,
// so the class cannot be (de)serialised standalone. The fix follows the same proven
// serialize_members pattern and is compile-checked.

// Safety net for the oa::GBase serialize() refactor: oa::GBase is the central
// serialised class for all optimization algorithms (checkpoints + network
// transport). Its split save()/load() was collapsed into a single serialize()
// after std::filesystem::path gained a free serialization. This test sets several
// oa::GBase members -- in particular cp_directory_path_ (the path that forced the
// former split) -- to NON-DEFAULT values and verifies they survive a round-trip in
// all three modes, guarding against silently dropping a member.
TEST_CASE(
    "oa::GBase (via GEvolutionaryAlgorithm) round-trips its members incl. the checkpoint path",
    "[geneva][serialization]"
) {
    using Gem::Common::serializationMode;

    for (auto mode :
         {serializationMode::TEXT, serializationMode::XML, serializationMode::BINARY}) {
        oa::GEvolutionaryAlgorithm original;
        // Set several oa::GBase members to non-default values via public setters.
        // "." is an always-existing directory, so setCheckpointBaseName creates nothing.
        original.setCheckpointBaseName(".", "custom_checkpoint_base");
        original.setCheckpointInterval(7);
        original.setMaxIteration(4242);
        original.setReportIteration(13);
        original.setMaxStallIteration(99);

        oa::GEvolutionaryAlgorithm restored;
        // Deliberately different starting values, so the round-trip has to overwrite them.
        restored.setMaxIteration(1);
        restored.setReportIteration(1);

        REQUIRE_NOTHROW(
            restored.GObject::fromString(original.GObject::toString(mode), mode)
        );

        // Explicit getter checks: these would catch a dropped member that the
        // structural compare might otherwise tolerate.
        CHECK(restored.getCheckpointDirectory() == original.getCheckpointDirectory());
        CHECK(restored.getCheckpointBaseName() == original.getCheckpointBaseName());
        CHECK(restored.getCheckpointInterval() == 7);
        CHECK(restored.getMaxIteration() == 4242);
        CHECK(restored.getReportIteration() == 13);
        CHECK(restored.getMaxStallIteration() == 99);

        GEqualityPrinter gep(
            "oa::GBase-roundtrip",
            pow(10, -7),
            Gem::Common::CE_WITH_MESSAGES
        );
        CHECK(gep.isSimilar(restored, original));
    }
}
