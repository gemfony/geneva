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
 * Phase 5 of the flat-only migration: end-to-end checks that EVERY optimization algorithm can drive a
 * GFlatGenome individual, not just the EA. Each algorithm only ever touches an individual through the
 * storage-agnostic channel interface (streamline / assignValueVector / boundaries / countParameters /
 * adapt / randomInit) + population-level access + personality traits -- and the OA populations already
 * hold a GUniquePtrContainerT<GOptimizableEntity>, so the flat genome drops in with ZERO OA changes.
 * These tests prove it by running each OA serially on a flat-genome sphere and asserting convergence.
 *
 * This is the flat counterpart of the OA validation the full test suite already performs on the tree
 * (GTestIndividual1 etc.) -- the same serial-EA/SA/Swarm/CGD/NM/Scan machinery, the same convergence,
 * a different genome implementation behind the identical §2 seam.
 */

#include <catch2/catch_test_macros.hpp>

#include <cmath>
#include <cstddef>
#include <filesystem>
#include <memory>
#include <tuple>
#include <vector>

#include <boost/serialization/export.hpp>

#include "geneva/ind/GAdaptionAuxKeys.hpp"
#include "geneva/ind/GAdaptionKernels.hpp"
#include "geneva/ind/GFlatGenome.hpp"
#include "geneva/ind/GFlatIndividualT.hpp"
#include "geneva/ind/GGenomeBuilder.hpp"
#include "geneva/individuals/GLineFitIndividual.hpp"
#include "geneva/oa/GAdaptionConfig.hpp"
#include "geneva/oa/GConjugateGradientDescent.hpp"
#include "geneva/oa/GEvolutionaryAlgorithm.hpp"
#include "geneva/oa/GNelderMead.hpp"
#include "geneva/oa/GParameterScan.hpp"
#include "geneva/oa/GSimulatedAnnealing.hpp"
#include "geneva/oa/GSwarmAlgorithm.hpp"

namespace gpar = Gem::Geneva::Parameters;
namespace oa = Gem::Geneva::OptimizationAlgorithms;

using Gem::Geneva::activityMode;

namespace {

constexpr std::size_t N_DIM = 5;

/******************************************************************************/
/**
 * A flat-genome sphere individual: N_DIM constrained doubles in [-5, 5), sharing one Gauss adaptor,
 * started at 3.0 (so the initial fitness is N_DIM * 9 = 45). Authored entirely through the builder;
 * clone/load/compare come from the CRTP base + GFlatGenome.
 */
class FlatSphereOA : public gpar::GFlatIndividualT<FlatSphereOA> {
public:
    FlatSphereOA() {
        gpar::GGenomeBuilder b;
        b.addDoubleGroup(N_DIM, -5., 5.).gaussAdaptor(0.5, 0.8, 1e-3, 2., 1.).init(3.0);
        this->setGenome(b.build());
    }
    FlatSphereOA(const FlatSphereOA &) = default;

protected:
    double fitnessCalculation() override {
        std::vector<double> v;
        this->streamline<double>(v);
        double s = 0.;
        for(double x : v) {
            s += x * x;
        }
        return s;
    }
};

/******************************************************************************/
/**
 * A WIDE-bounds flat sphere individual: N_DIM constrained doubles in [-25, 25), started at 3.0. Used
 * for the gradient method: CONSTRAINED (not unbounded) because conjugate gradient descent scales its
 * finite-difference step by the parameter RANGE (finite_step * (upper-lower)) -- an unbounded
 * parameter would give an infinite range and hence a degenerate step. The bounds are kept wide enough
 * that the sphere optimum (the origin) and the whole descent path stay well in the interior, so the
 * constrained fold never distorts a line-search probe (the same setup real CGD usage employs, e.g. the
 * GFunctionMinimizer example over GConstrainedDoubleObject).
 */
class FlatSphereWideOA : public gpar::GFlatIndividualT<FlatSphereWideOA> {
public:
    FlatSphereWideOA() {
        gpar::GGenomeBuilder b;
        b.addDoubleGroup(N_DIM, -25., 25.).init(3.0);
        this->setGenome(b.build());
    }
    FlatSphereWideOA(const FlatSphereWideOA &) = default;

protected:
    double fitnessCalculation() override {
        std::vector<double> v;
        this->streamline<double>(v);
        double s = 0.;
        for(double x : v) {
            s += x * x;
        }
        return s;
    }
};

/******************************************************************************/
/**
 * A continuous flat sphere driven by a BI-GAUSSIAN adaptor (instead of the single gaussian): N_DIM
 * constrained doubles in [-5, 5), started at 3.0. Demonstrates the bi-gauss kernel end-to-end.
 */
class FlatBiGaussSphereOA : public gpar::GFlatIndividualT<FlatBiGaussSphereOA> {
public:
    FlatBiGaussSphereOA() {
        gpar::GGenomeBuilder b;
        // sigma1, sigmaSigma1, minSigma1, maxSigma1, sigma2, sigmaSigma2, minSigma2, maxSigma2,
        // delta, sigmaDelta, minDelta, maxDelta, adProb
        b.addDoubleGroup(N_DIM, -5., 5.)
            .biGaussAdaptor(0.5, 0.8, 1e-3, 2., 0.5, 0.8, 1e-3, 2., 0.5, 0.8, 0., 2., 1.)
            .init(3.0);
        this->setGenome(b.build());
    }
    FlatBiGaussSphereOA(const FlatBiGaussSphereOA &) = default;

protected:
    double fitnessCalculation() override {
        std::vector<double> v;
        this->streamline<double>(v);
        double s = 0.;
        for(double x : v) {
            s += x * x;
        }
        return s;
    }
};

/******************************************************************************/
/**
 * A combinatorial INTEGER problem driven by a FLIP adaptor: N_INT constrained int32 in [-10, 10],
 * started at 7, minimising the sum of squares (optimum: all zero). Demonstrates the int flip kernel.
 */
constexpr std::size_t N_INT = 5;

class FlatIntSphereOA : public gpar::GFlatIndividualT<FlatIntSphereOA> {
public:
    FlatIntSphereOA() {
        gpar::GGenomeBuilder b;
        b.addInt32Group(N_INT, -10, 10).flipAdaptor(1.0).init(7);
        this->setGenome(b.build());
    }
    FlatIntSphereOA(const FlatIntSphereOA &) = default;

protected:
    double fitnessCalculation() override {
        std::vector<std::int32_t> v;
        this->streamline<std::int32_t>(v);
        double s = 0.;
        for(std::int32_t x : v) {
            s += static_cast<double>(x) * static_cast<double>(x);
        }
        return s;
    }
};

/******************************************************************************/
/**
 * A combinatorial BOOLEAN problem (OneMax) driven by a FLIP adaptor: N_BOOL booleans, minimising the
 * count of false bits (optimum: all true). Demonstrates the bool flip kernel end-to-end.
 */
constexpr std::size_t N_BOOL = 16;

class FlatOneMaxOA : public gpar::GFlatIndividualT<FlatOneMaxOA> {
public:
    FlatOneMaxOA() {
        gpar::GGenomeBuilder b;
        b.addBoolGroup(N_BOOL).flipAdaptor(0.25).init(false);
        this->setGenome(b.build());
    }
    FlatOneMaxOA(const FlatOneMaxOA &) = default;

protected:
    double fitnessCalculation() override {
        std::vector<bool> v;
        this->streamline<bool>(v);
        double false_count = 0.;
        for(bool x : v) {
            if(not x) {
                false_count += 1.;
            }
        }
        return false_count; // minimised -> all true
    }
};

/** @brief Sphere value of the best individual, also asserting the constraints held. */
double bestSphere(const std::shared_ptr<FlatSphereOA> &best) {
    std::vector<double> v;
    best->streamline<double>(v);
    double s = 0.;
    for(double x : v) {
        s += x * x;
        CHECK(x >= -5.0);
        CHECK(x < 5.0);
    }
    return s;
}

} /* anonymous namespace */

/******************************************************************************/

TEST_CASE("Evolutionary algorithm optimizes a flat individual", "[flat][oa]") {
    auto pop = std::make_shared<oa::GEvolutionaryAlgorithm>();
    pop->setPopulationSizes(18, 6);
    pop->setMaxIteration(120);
    pop->setReportIteration(100000);
    pop->push_back(FlatSphereOA().clone_unique());
    pop->setLocalConsumer(oa::local_consumer_kind::serial);
    pop->optimize();

    auto best = pop->getBestGlobalIndividual<FlatSphereOA>();
    REQUIRE(best);
    CHECK(bestSphere(best) < 20.0); // far below the f=45 start
}

/******************************************************************************/

TEST_CASE("EA checkpoint round-trip preserves the per-slot adaption scratch", "[flat][oa]") {
    using gpar::GFlatGenome;
    using Gem::Geneva::Parameters::AUXKEY_GAUSS_DOUBLE;
    using Gem::Geneva::Parameters::GaussState;

    // Phase 10.4: the per-individual OA scratch (adaption sigma/state) lives on the GIndividualSlot and is
    // serialized for check-pointing, so a resumed algorithm keeps its evolved state. Here we seed a slot's
    // scratch (as the EA does at setup), drive its sigma to a known value, round-trip the whole algorithm
    // through the exact checkpoint path (toFile -> loadCheckpoint) and confirm the scratch survives. A
    // fully serialization-registered individual (GLineFitIndividual) is used so the algorithm can be
    // serialized to a file.
    namespace gind = Gem::Geneva::Individuals;
    const std::vector<std::tuple<double, double>> data_points{{0., 0.}, {1., 1.}, {2., 2.}};

    auto pop = std::make_shared<oa::GEvolutionaryAlgorithm>();
    pop->push_back(gind::GLineFitIndividual(data_points).clone_unique());

    auto &slot0 = pop->at(0);
    auto &flat0 = dynamic_cast<GFlatGenome &>(slot0->individual());
    oa::GAdaptionConfigBase cfg(flat0);
    cfg.installInto(slot0->scratch());
    auto states = slot0->scratch().metaRecords<GaussState<double>>(AUXKEY_GAUSS_DOUBLE);
    REQUIRE(not states.empty());
    states[0].sigma = 0.123456;
    states[0].counter = 17;

    // Serialize through the checkpoint mechanism (filename must encode the EA personality so
    // loadCheckpoint's cross-check passes), then resume into a fresh algorithm.
    const std::filesystem::path cp =
        std::filesystem::temp_directory_path() / "checkpoint-PERSONALITY_EA-scratchtest.cp";
    pop->toFile(cp, pop->getCheckpointSerializationMode());

    auto resumed = std::make_shared<oa::GEvolutionaryAlgorithm>();
    resumed->loadCheckpoint(cp);

    REQUIRE(resumed->size() == pop->size());
    auto &rslot0 = resumed->at(0);
    REQUIRE(rslot0->scratch().hasAux(AUXKEY_GAUSS_DOUBLE)); // the POD scratch block rode along
    auto rstates = rslot0->scratch().metaRecords<GaussState<double>>(AUXKEY_GAUSS_DOUBLE);
    REQUIRE(not rstates.empty());
    CHECK(rstates[0].sigma == 0.123456); // the evolved sigma survived the checkpoint intact
    CHECK(rstates[0].counter == 17u);

    // And the resumed algorithm continues optimizing through the scratch-PRESERVING setup path
    // (loadCheckpoint set the resume marker, so setIndividualPersonalities + init() preserve the restored
    // scratch instead of re-seeding it). Exercise it end-to-end and confirm it still converges.
    resumed->setPopulationSizes(18, 6);
    resumed->setMaxIteration(300);
    resumed->setReportIteration(100000);
    resumed->setLocalConsumer(oa::local_consumer_kind::serial);
    resumed->optimize();

    auto best = resumed->getBestGlobalIndividual<gind::GLineFitIndividual>();
    REQUIRE(best);
    const auto [a, b] = best->getLine();
    CHECK(std::abs(a - 0.) < 0.5); // resumed run reaches the y = x fit (offset ~0, slope ~1)
    CHECK(std::abs(b - 1.) < 0.3);

    std::filesystem::remove(cp);
}

/******************************************************************************/

TEST_CASE("EA adopts an externally-provided adaption config", "[flat][oa]") {
    // Phase 8 step 4: an algorithm uses an externally-supplied OA-owned config (Go2 hands it the one it
    // holds for the algorithm's type) instead of deriving a default from the genome layout. Here we author
    // a config explicitly from the genome and confirm the EA drives a converging adaption with it. (That
    // the provided config -- not just any default -- is actually consumed is proven by the companion
    // "rejects an adaption config built for a different genome" test, where a mismatched provided config
    // is validated against the genome and rejected.)
    auto pop = std::make_shared<oa::GEvolutionaryAlgorithm>();
    pop->setPopulationSizes(18, 6);
    pop->setMaxIteration(120);
    pop->setReportIteration(100000);

    auto ind = FlatSphereOA().clone_unique();
    auto &flat = dynamic_cast<gpar::GFlatGenome &>(*ind);
    auto cfg = std::make_shared<oa::GEAAdaptionConfig>(flat);
    cfg->groupDouble(0).gauss(0.5, 0.8, 1e-3, 2., 1.); // author the Gauss settings explicitly

    pop->push_back(std::move(ind));
    pop->setAdaptionConfig(cfg);
    pop->setLocalConsumer(oa::local_consumer_kind::serial);
    pop->optimize();

    auto best = pop->getBestGlobalIndividual<FlatSphereOA>();
    REQUIRE(best);
    CHECK(bestSphere(best) < 20.0); // the provided config drives a converging adaption (far below the f=45 start)
}

/******************************************************************************/

TEST_CASE("EA rejects an adaption config built for a different genome", "[flat][oa]") {
    namespace gind = Gem::Geneva::Individuals;

    auto pop = std::make_shared<oa::GEvolutionaryAlgorithm>();
    pop->setPopulationSizes(6, 2);
    pop->setMaxIteration(2);
    pop->setReportIteration(100000);
    pop->push_back(FlatSphereOA().clone_unique()); // genome: a 5-double group

    // A config built from a structurally DIFFERENT genome (a line fit: 2 doubles) must be rejected when
    // the algorithm validates it against its population's genome at setup.
    gind::GLineFitIndividual other(std::vector<std::tuple<double, double>>{{0., 0.}, {1., 1.}});
    auto &oflat = dynamic_cast<gpar::GFlatGenome &>(other);
    auto cfg = std::make_shared<oa::GEAAdaptionConfig>(oflat);

    pop->setAdaptionConfig(cfg);
    pop->setLocalConsumer(oa::local_consumer_kind::serial);
    CHECK_THROWS(pop->optimize()); // checkConsistency rejects the mismatched config at init()
}

/******************************************************************************/

TEST_CASE("Simulated annealing optimizes a flat individual", "[flat][oa]") {
    auto pop = std::make_shared<oa::GSimulatedAnnealing>();
    pop->setPopulationSizes(18, 6);
    pop->setMaxIteration(120);
    pop->setReportIteration(100000);
    pop->push_back(FlatSphereOA().clone_unique());
    pop->setLocalConsumer(oa::local_consumer_kind::serial);
    pop->optimize();

    auto best = pop->getBestGlobalIndividual<FlatSphereOA>();
    REQUIRE(best);
    CHECK(bestSphere(best) < 20.0);
}

/******************************************************************************/

TEST_CASE("Swarm optimization optimizes a flat individual", "[flat][oa]") {
    auto pop = std::make_shared<oa::GSwarmAlgorithm>();
    pop->setSwarmSizes(3, 6); // 3 neighborhoods x 6 members
    pop->setMaxIteration(120);
    pop->setReportIteration(100000);
    pop->push_back(FlatSphereOA().clone_unique());
    pop->setLocalConsumer(oa::local_consumer_kind::serial);
    pop->optimize();

    auto best = pop->getBestGlobalIndividual<FlatSphereOA>();
    REQUIRE(best);
    CHECK(bestSphere(best) < 20.0);
}

/******************************************************************************/

TEST_CASE("Conjugate gradient descent optimizes a flat individual", "[flat][oa]") {
    auto pop = std::make_shared<oa::GConjugateGradientDescent>();
    pop->setNStartingPoints(1);
    pop->setMaxIteration(500);
    pop->setReportIteration(100000);
    pop->push_back(FlatSphereWideOA().clone_unique());
    pop->setLocalConsumer(oa::local_consumer_kind::serial);
    pop->optimize();

    auto best = pop->getBestGlobalIndividual<FlatSphereWideOA>();
    REQUIRE(best);
    std::vector<double> v;
    best->streamline<double>(v);
    double sphere = 0.;
    for(double x : v) {
        sphere += x * x;
    }
    // CGD drives the flat individual and descends from f=45; its default finite-difference step
    // makes convergence gradual (a CGD-config matter, identical on the tree representation).
    CHECK(sphere < 10.0);
}

/******************************************************************************/

TEST_CASE("Nelder-Mead optimizes a flat individual", "[flat][oa]") {
    auto pop = std::make_shared<oa::GNelderMead>();
    pop->setMaxIteration(200);
    pop->setReportIteration(100000);
    pop->push_back(FlatSphereOA().clone_unique());
    pop->setLocalConsumer(oa::local_consumer_kind::serial);
    pop->optimize();

    auto best = pop->getBestGlobalIndividual<FlatSphereOA>();
    REQUIRE(best);
    CHECK(bestSphere(best) < 5.0); // simplex descent on a sphere converges well
}

/******************************************************************************/

TEST_CASE("Parameter scan sweeps a flat individual", "[flat][oa]") {
    auto pop = std::make_shared<oa::GParameterScan>();
    // A 5-step grid per dimension over [-5, 5] includes 0, so the sphere optimum is on the grid.
    pop->setParameterSpecs(
        "d(0, -5., 5., 5), d(1, -5., 5., 5), d(2, -5., 5., 5), d(3, -5., 5., 5), d(4, -5., 5., 5)"
    );
    pop->setMaxIteration(100000);   // enough iterations to cover the 5^5 grid
    pop->setMaxStallIteration(0);    // 0 == disabled: sweep the WHOLE grid (the origin is on it),
                                     // do not stop early on stall-convergence (order/seed dependent)
    pop->setReportIteration(100000);
    pop->push_back(FlatSphereOA().clone_unique());
    pop->setLocalConsumer(oa::local_consumer_kind::serial);
    pop->optimize();

    auto best = pop->getBestGlobalIndividual<FlatSphereOA>();
    REQUIRE(best);
    // The scan swept the grid and returned the best grid point (near the origin); the grid's
    // resolution -- not the optimum -- bounds how close it gets.
    CHECK(bestSphere(best) < 5.0);
}

/******************************************************************************/
// Worked examples for the bi-gaussian + flip kernels (Phase 6), driven end-to-end by an EA.
/******************************************************************************/

TEST_CASE("EA optimizes a flat individual with a BI-GAUSSIAN adaptor", "[flat][oa][bigauss]") {
    auto pop = std::make_shared<oa::GEvolutionaryAlgorithm>();
    pop->setPopulationSizes(18, 6);
    pop->setMaxIteration(150);
    pop->setReportIteration(100000);
    pop->push_back(FlatBiGaussSphereOA().clone_unique());
    pop->setLocalConsumer(oa::local_consumer_kind::serial);
    pop->optimize();

    auto best = pop->getBestGlobalIndividual<FlatBiGaussSphereOA>();
    REQUIRE(best);
    std::vector<double> v;
    best->streamline<double>(v);
    double sphere = 0.;
    for(double x : v) {
        sphere += x * x;
        CHECK(x >= -5.0);
        CHECK(x < 5.0);
    }
    CHECK(sphere < 20.0); // far below the f=45 start
}

/******************************************************************************/

TEST_CASE("EA optimizes a flat INTEGER individual with a FLIP adaptor", "[flat][oa][flip]") {
    auto pop = std::make_shared<oa::GEvolutionaryAlgorithm>();
    pop->setPopulationSizes(18, 6);
    pop->setMaxIteration(200);
    pop->setReportIteration(100000);
    pop->push_back(FlatIntSphereOA().clone_unique());
    pop->setLocalConsumer(oa::local_consumer_kind::serial);
    pop->optimize();

    auto best = pop->getBestGlobalIndividual<FlatIntSphereOA>();
    REQUIRE(best);
    std::vector<std::int32_t> v;
    best->streamline<std::int32_t>(v);
    double sphere = 0.;
    for(std::int32_t x : v) {
        sphere += static_cast<double>(x) * static_cast<double>(x);
        CHECK(x >= -10);
        CHECK(x <= 10);
    }
    CHECK(sphere < 15.0); // far below the N_INT * 49 = 245 start; flips march toward 0
}

/******************************************************************************/

TEST_CASE("EA optimizes a flat BOOLEAN OneMax with a FLIP adaptor", "[flat][oa][flip]") {
    auto pop = std::make_shared<oa::GEvolutionaryAlgorithm>();
    pop->setPopulationSizes(18, 6);
    pop->setMaxIteration(400);
    pop->setReportIteration(100000);
    pop->push_back(FlatOneMaxOA().clone_unique());
    pop->setLocalConsumer(oa::local_consumer_kind::serial);
    pop->optimize();

    auto best = pop->getBestGlobalIndividual<FlatOneMaxOA>();
    REQUIRE(best);
    std::vector<bool> v;
    best->streamline<bool>(v);
    std::size_t false_count = 0;
    for(bool x : v) {
        if(not x) {
            ++false_count;
        }
    }
    // OneMax converges from the all-false start toward all-true; the last few bits are a heavy
    // (coupon-collector) tail, so allow a small residue rather than demanding a perfect sweep.
    CHECK(false_count <= 3);
}

/******************************************************************************/
// Migrated library individual (Phase 6): GLineFitIndividual is now a flat genome. Beyond the standard
// machinery test, prove it still fits a line end-to-end under an EA.
/******************************************************************************/

TEST_CASE("EA fits a line with the migrated (flat) GLineFitIndividual", "[flat][oa][linefit]") {
    namespace gind = Gem::Geneva::Individuals;

    // Sample points on the line y = x -> the optimum is offset a = 0, slope b = 1, residual 0.
    const std::vector<std::tuple<double, double>> data_points{
        {0., 0.}, {1., 1.}, {2., 2.}, {3., 3.}, {4., 4.}};

    auto pop = std::make_shared<oa::GEvolutionaryAlgorithm>();
    pop->setPopulationSizes(18, 6);
    pop->setMaxIteration(400);
    pop->setReportIteration(100000);
    pop->push_back(gind::GLineFitIndividual(data_points).clone_unique());
    pop->setLocalConsumer(oa::local_consumer_kind::serial);
    pop->optimize();

    auto best = pop->getBestGlobalIndividual<gind::GLineFitIndividual>();
    REQUIRE(best);

    const auto [a, b] = best->getLine();
    CHECK(std::abs(a - 0.) < 0.5); // offset near 0
    CHECK(std::abs(b - 1.) < 0.3); // slope near 1
}
