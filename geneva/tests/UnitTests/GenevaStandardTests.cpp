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

#include <cmath>

// All classes that will be tested in this file
#include "geneva/individuals/GDelayIndividual.hpp"
#include "geneva/individuals/GExternalEvaluatorIndividual.hpp"
#include "geneva/individuals/GFunctionIndividual.hpp"
#include "geneva/individuals/GLineFitIndividual.hpp"
#include "geneva/individuals/GMetaOptimizerIndividualT.hpp"
#include "geneva/individuals/GTestIndividual2.hpp"
#include "geneva/individuals/GTestIndividual3.hpp"
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
#include "geneva/oa/GConjugateGradientDescent.hpp"
#include "geneva/oa/GParameterScan.hpp"
#include "geneva/oa/GNelderMead.hpp"
// Constraints
#include "geneva/par/GOptimizableEntityMultiConstraint.hpp"
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

// The concrete GOptimizableEntityConstraint subclasses defined in GFunctionIndividual.hpp.
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
// Targeted (de)serialization round-trips for the concrete GOptimizableEntityConstraint
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
            restored.fromString(original.toString(mode), mode)
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
            restored.fromString(original.toString(mode), mode)
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
            restored.fromString(original.toString(mode), mode)
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
            restored.fromString(original.toString(mode), mode)
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

// Safety net for the oa::GOptimizationAlgorithmBase serialize() refactor: oa::GOptimizationAlgorithmBase is the central
// serialised class for all optimization algorithms (checkpoints + network
// transport). Its split save()/load() was collapsed into a single serialize()
// after std::filesystem::path gained a free serialization. This test sets several
// oa::GOptimizationAlgorithmBase members -- in particular cp_directory_path_ (the path that forced the
// former split) -- to NON-DEFAULT values and verifies they survive a round-trip in
// all three modes, guarding against silently dropping a member.
TEST_CASE(
    "oa::GOptimizationAlgorithmBase (via GEvolutionaryAlgorithm) round-trips its members incl. the checkpoint path",
    "[geneva][serialization]"
) {
    using Gem::Common::serializationMode;

    for (auto mode :
         {serializationMode::TEXT, serializationMode::XML, serializationMode::BINARY}) {
        oa::GEvolutionaryAlgorithm original;
        // Set several oa::GOptimizationAlgorithmBase members to non-default values via public setters.
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
            restored.fromString(original.toString(mode), mode)
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
            "oa::GOptimizationAlgorithmBase-roundtrip",
            pow(10, -7),
            Gem::Common::CE_WITH_MESSAGES
        );
        CHECK(gep.isSimilar(restored, original));
    }
}

// Safety net for the GOptimizableEntity serialize()/load_()/compare_() unification
// onto a single localMembers() declaration. GOptimizableEntity is the central serialised
// base for all individuals. This test sets several of its members to non-default
// values -- in particular use_random_crash_ / random_crash_prob_, which serialize()
// stored but the old load_()/compare_() silently ignored (a real member-drop bug,
// fixed by deriving all three from localMembers()) -- and checks they survive both a
// wire round-trip AND an in-memory load() (clone path). Exercised on the concrete
// GTestIndividual1 (a GFlatGenome subclass).
TEST_CASE(
    "GOptimizableEntity (via GTestIndividual1) round-trips its members incl. the random-crash settings",
    "[geneva][serialization]"
) {
    using Gem::Common::serializationMode;

    // --- in-memory load() (clone path) ---
    {
        gind::GTestIndividual1 original;
        original.setRandomCrash(true, 0.25);
        original.setMaxMode(maxMode::MAXIMIZE);

        gind::GTestIndividual1 restored;
        restored.setRandomCrash(false, 0.0);

        // In-memory load goes through load_(); before the fix this dropped the
        // random-crash members.
        REQUIRE_NOTHROW(restored.load(original));

        auto [use_rc, rc_prob] = restored.getRandomCrash();
        CHECK(use_rc == true);
        CHECK(rc_prob == 0.25);
        CHECK(restored.getMaxMode() == maxMode::MAXIMIZE);

        GEqualityPrinter gep(
            "GFlatGenome-load-roundtrip",
            pow(10, -7),
            Gem::Common::CE_WITH_MESSAGES
        );
        // compare_() must now see the two objects as equal (before the fix it
        // ignored the crash members and would have reported "equal" even when they
        // differed -- i.e. it could not tell them apart).
        CHECK(gep.isSimilar(restored, original));
    }

    // --- wire round-trip in all three modes ---
    for (auto mode :
         {serializationMode::TEXT, serializationMode::XML, serializationMode::BINARY}) {
        gind::GTestIndividual1 original;
        original.setRandomCrash(true, 0.5);
        original.setMaxMode(maxMode::MAXIMIZE);

        gind::GTestIndividual1 restored;
        restored.setRandomCrash(false, 0.0);

        REQUIRE_NOTHROW(
            restored.fromString(original.toString(mode), mode)
        );

        auto [use_rc, rc_prob] = restored.getRandomCrash();
        CHECK(use_rc == true);
        CHECK(rc_prob == 0.5);
        CHECK(restored.getMaxMode() == maxMode::MAXIMIZE);

        GEqualityPrinter gep(
            "GFlatGenome-roundtrip",
            pow(10, -7),
            Gem::Common::CE_WITH_MESSAGES
        );
        CHECK(gep.isSimilar(restored, original));
    }
}

// Safety net for the oa::GSwarmAlgorithm serialize()/load_()/compare_() unification
// onto a single localMembers() declaration (the unconditional members) plus a manual
// tail for the conditionally-reconstructed neighborhood/global-best members. This test
// sets several of the about-to-be-tied members to NON-DEFAULT values and verifies they
// survive a wire round-trip AND an in-memory load() (clone path), guarding against the
// default-value masking that hides a dropped member.
TEST_CASE(
    "oa::GSwarmAlgorithm round-trips its members in TEXT, XML, BINARY and via load()",
    "[geneva][serialization]"
) {
    using Gem::Common::serializationMode;

    auto makeOriginal = []() {
        oa::GSwarmAlgorithm a;
        // Non-default values for several of the localMembers()-tied members.
        a.setCPersonal(1.75);
        a.setCNeighborhood(2.25);
        a.setCGlobal(3.5);
        a.setCVelocity(0.875);
        a.setVelocityRangePercentage(0.42);
        a.setUpdateRule(updateRule::SWARM_UPDATERULE_LINEAR);
        a.setRepulsionThreshold(17);
        a.setNeighborhoodsRandomFillUp(false);
        return a;
    };

    auto checkGetters = [](const oa::GSwarmAlgorithm &restored) {
        CHECK(restored.getCPersonal() == 1.75);
        CHECK(restored.getCNeighborhood() == 2.25);
        CHECK(restored.getCGlobal() == 3.5);
        CHECK(restored.getCVelocity() == 0.875);
        CHECK(restored.getVelocityRangePercentage() == 0.42);
        CHECK(restored.getUpdateRule() == updateRule::SWARM_UPDATERULE_LINEAR);
        CHECK(restored.getRepulsionThreshold() == 17);
        CHECK(restored.neighborhoodsFilledUpRandomly() == false);
    };

    // --- in-memory load() (clone path) ---
    {
        oa::GSwarmAlgorithm original = makeOriginal();
        oa::GSwarmAlgorithm restored;
        restored.setCPersonal(0.1);
        restored.setNeighborhoodsRandomFillUp(true);

        REQUIRE_NOTHROW(restored.load(original));
        checkGetters(restored);

        GEqualityPrinter gep(
            "GSwarmAlgorithm-load-roundtrip",
            pow(10, -7),
            Gem::Common::CE_WITH_MESSAGES
        );
        CHECK(gep.isSimilar(restored, original));
    }

    // --- wire round-trip in all three modes ---
    for (auto mode :
         {serializationMode::TEXT, serializationMode::XML, serializationMode::BINARY}) {
        oa::GSwarmAlgorithm original = makeOriginal();
        oa::GSwarmAlgorithm restored;
        restored.setCPersonal(0.1);
        restored.setNeighborhoodsRandomFillUp(true);

        REQUIRE_NOTHROW(
            restored.fromString(original.toString(mode), mode)
        );
        checkGetters(restored);

        GEqualityPrinter gep(
            "GSwarmAlgorithm-roundtrip",
            pow(10, -7),
            Gem::Common::CE_WITH_MESSAGES
        );
        CHECK(gep.isSimilar(restored, original));
    }
}

// Safety net for the oa::GParameterScan serialize()/load_()/compare_() unification
// onto a single localMembers() declaration (the plain members) plus a manual tail
// (the scan-parameter vectors, whose element type lacks the Gemfony common interface,
// and the load-only transient cycle_logic_halt_). This test sets several of the
// about-to-be-tied plain members to NON-DEFAULT values and verifies they survive a
// wire round-trip AND an in-memory load() (clone path).
TEST_CASE(
    "oa::GParameterScan round-trips its members in TEXT, XML, BINARY and via load()",
    "[geneva][serialization]"
) {
    using Gem::Common::serializationMode;

    auto makeOriginal = []() {
        oa::GParameterScan a;
        // Non-default values for the localMembers()-tied plain members.
        a.setScanRandomly(false);    // default is true
        a.setNMonitorInds(42);       // default is DEFAULTNMONITORINDS (10)
        a.setNSimpleScans(7);        // sets simple_scan_items_ (default 0)
        return a;
    };

    auto checkGetters = [](const oa::GParameterScan &restored) {
        CHECK(restored.getScanRandomly() == false);
        CHECK(restored.getNMonitorInds() == 42);
        CHECK(restored.getNSimpleScans() == 7);
    };

    // --- in-memory load() (clone path) ---
    {
        oa::GParameterScan original = makeOriginal();
        oa::GParameterScan restored;
        restored.setScanRandomly(true);
        restored.setNMonitorInds(1);

        REQUIRE_NOTHROW(restored.load(original));
        checkGetters(restored);

        GEqualityPrinter gep(
            "GParameterScan-load-roundtrip",
            pow(10, -7),
            Gem::Common::CE_WITH_MESSAGES
        );
        CHECK(gep.isSimilar(restored, original));
    }

    // --- wire round-trip in all three modes ---
    for (auto mode :
         {serializationMode::TEXT, serializationMode::XML, serializationMode::BINARY}) {
        oa::GParameterScan original = makeOriginal();
        oa::GParameterScan restored;
        restored.setScanRandomly(true);
        restored.setNMonitorInds(1);

        REQUIRE_NOTHROW(
            restored.fromString(original.toString(mode), mode)
        );
        checkGetters(restored);

        GEqualityPrinter gep(
            "GParameterScan-roundtrip",
            pow(10, -7),
            Gem::Common::CE_WITH_MESSAGES
        );
        CHECK(gep.isSimilar(restored, original));
    }
}

// ============================================================================
// Meta-optimizer: the search genome is built from the EA tunable manifest and
// read back by name (no MOT_* index math). This pins that the manifest -> genome
// -> readTuned() chain reproduces the manifest's values under their names, and
// that the derived getters (ad_prob etc.) compute correctly.
// ============================================================================

TEST_CASE("Meta-optimizer genome is built from the tunable manifest and read by name", "[geneva][meta]") {
    using Meta = gind::GMetaOptimizerIndividualT<gind::GFunctionIndividual>;
    namespace n = oa::ea_tunable;

    auto p = std::make_shared<Meta>();
    const auto manifest = oa::GEvolutionaryAlgorithm::tunableManifest();
    Meta::addContent(p, manifest);

    // The defaults come straight from the manifest, addressed by name.
    CHECK(p->getNParents() == 1);
    CHECK(p->getNChildren() == 100);
    CHECK(std::abs(p->getMinSigma() - 0.001) < 1e-9);
    CHECK(std::abs(p->getSigmaRange() - 0.2) < 1e-9);
    CHECK(std::abs(p->getSigmaSigma() - 0.1) < 1e-9);
    // getAdProb = min_ad_prob + ad_prob_start_pct * ad_prob_range = 0 + 1 * 0.9.
    CHECK(std::abs(p->getAdProb() - 0.9) < 1e-9);

    // A reordered / re-valued manifest still reads back correctly by name: build with custom values and
    // confirm the named getters follow the values, not any fixed position.
    auto custom = manifest;
    auto setKnob = [&custom](const char *name, double init, double lo, double hi) {
        for(auto &tp : custom) {
            if(tp.name == name) {
                tp.init = init;
                tp.lower = lo;
                tp.upper = hi;
            }
        }
    };
    setKnob(n::n_parents, 3, 1, 6);
    setKnob(n::n_children, 42, 5, 250);
    setKnob(n::min_sigma, 0.01, 0.001, 0.1);
    setKnob(n::sigma_sigma, 0.5, 0., 1.);

    auto q = std::make_shared<Meta>();
    Meta::addContent(q, custom);
    CHECK(q->getNParents() == 3);
    CHECK(q->getNChildren() == 42);
    CHECK(std::abs(q->getMinSigma() - 0.01) < 1e-9);
    CHECK(std::abs(q->getSigmaSigma() - 0.5) < 1e-9);
}
