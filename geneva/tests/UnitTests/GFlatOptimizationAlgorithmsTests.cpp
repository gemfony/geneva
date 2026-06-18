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
 * End-to-end checks that EVERY optimization algorithm can drive a
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
#include "geneva/oa/GAdaption.hpp"
#include "geneva/oa/GAdaptionConfig.hpp"
#include "geneva/oa/GConjugateGradientDescent.hpp"
#include "geneva/oa/GEvolutionaryAlgorithm.hpp"
#include "geneva/oa/GNelderMead.hpp"
#include "geneva/oa/GParameterScan.hpp"
#include "geneva/oa/GSimulatedAnnealing.hpp"
#include "geneva/oa/GSwarmAlgorithm.hpp"

namespace gen = Gem::Geneva::Genome;
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
class FlatSphereOA : public gen::GFlatIndividualT<FlatSphereOA> {
public:
    FlatSphereOA() {
        gen::GGenomeBuilder b;
        b.addDoubleGroup(N_DIM, -5., 5.).init(3.0); // structure only; the adaptor lives on the OA config
        this->setGenome(b.build());
    }
    FlatSphereOA(const FlatSphereOA &) = default;

    /** @brief The OA-owned Gauss adaption config for this genome's single double group. */
    std::shared_ptr<oa::GAdaptionConfigBase> buildAdaptionConfig() const {
        auto cfg = oa::makeAdaptionConfig<oa::GAdaptionConfigBase>(*this);
        cfg->groupDouble(0).gauss(0.5, 0.8, 1e-3, 2., 1.);
        return cfg;
    }

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
class FlatSphereWideOA : public gen::GFlatIndividualT<FlatSphereWideOA> {
public:
    FlatSphereWideOA() {
        gen::GGenomeBuilder b;
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
class FlatBiGaussSphereOA : public gen::GFlatIndividualT<FlatBiGaussSphereOA> {
public:
    FlatBiGaussSphereOA() {
        gen::GGenomeBuilder b;
        // sigma1, sigmaSigma1, minSigma1, maxSigma1, sigma2, sigmaSigma2, minSigma2, maxSigma2,
        // delta, sigmaDelta, minDelta, maxDelta, adProb
        b.addDoubleGroup(N_DIM, -5., 5.).init(3.0); // structure only; the adaptor lives on the OA config
        this->setGenome(b.build());
    }
    FlatBiGaussSphereOA(const FlatBiGaussSphereOA &) = default;

    /** @brief The OA-owned bi-gaussian adaption config for this genome's single double group. */
    std::shared_ptr<oa::GAdaptionConfigBase> buildAdaptionConfig() const {
        auto cfg = oa::makeAdaptionConfig<oa::GAdaptionConfigBase>(*this);
        // sigma1, sigmaSigma1, minSigma1, maxSigma1, sigma2, sigmaSigma2, minSigma2, maxSigma2,
        // delta, sigmaDelta, minDelta, maxDelta, adProb
        cfg->groupDouble(0).biGauss(0.5, 0.8, 1e-3, 2., 0.5, 0.8, 1e-3, 2., 0.5, 0.8, 0., 2., 1.);
        return cfg;
    }

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

class FlatIntSphereOA : public gen::GFlatIndividualT<FlatIntSphereOA> {
public:
    FlatIntSphereOA() {
        gen::GGenomeBuilder b;
        b.addInt32Group(N_INT, -10, 10).init(7); // structure only; the adaptor lives on the OA config
        this->setGenome(b.build());
    }
    FlatIntSphereOA(const FlatIntSphereOA &) = default;

    /** @brief The OA-owned flip adaption config for this genome's single int32 group. */
    std::shared_ptr<oa::GAdaptionConfigBase> buildAdaptionConfig() const {
        auto cfg = oa::makeAdaptionConfig<oa::GAdaptionConfigBase>(*this);
        cfg->groupInt32(0).flip(1.0);
        return cfg;
    }

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

class FlatOneMaxOA : public gen::GFlatIndividualT<FlatOneMaxOA> {
public:
    FlatOneMaxOA() {
        gen::GGenomeBuilder b;
        b.addBoolGroup(N_BOOL).init(false); // structure only; the adaptor lives on the OA config
        this->setGenome(b.build());
    }
    FlatOneMaxOA(const FlatOneMaxOA &) = default;

    /** @brief The OA-owned flip adaption config for this genome's single bool group. */
    std::shared_ptr<oa::GAdaptionConfigBase> buildAdaptionConfig() const {
        auto cfg = oa::makeAdaptionConfig<oa::GAdaptionConfigBase>(*this);
        cfg->groupBool(0).flip(0.25);
        return cfg;
    }

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

/**
 * A flat individual with a FROZEN (equal-bound) parameter: two free constrained doubles in [-5, 5) plus
 * one parameter fixed at 4 by equal bounds [4, 4]. Used to confirm an algorithm (here swarm, whose
 * per-dimension velocity range is l*(upper-lower) == 0 for the frozen dim) tolerates a fixed parameter
 * rather than crashing.
 */
class FlatFrozenOA : public gen::GFlatIndividualT<FlatFrozenOA> {
public:
    FlatFrozenOA() {
        gen::GGenomeBuilder b;
        b.addDoubleGroup(2, -5., 5.).init(3.0);
        b.addDouble(4., 4., 4.); // a parameter frozen at 4 (lower == upper)
        this->setGenome(b.build());
    }
    FlatFrozenOA(const FlatFrozenOA &) = default;

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
    FlatSphereOA src;
    pop->push_back(src.clone_unique());
    pop->setAdaptionConfig(src.buildAdaptionConfig());
    pop->optimize();

    auto best = pop->getBestGlobalIndividual<FlatSphereOA>();
    REQUIRE(best);
    CHECK(bestSphere(best) < 20.0); // far below the f=45 start
}

/******************************************************************************/

TEST_CASE("EA in a PARETO mode degenerates safely on a single-objective individual", "[flat][oa]") {
    // B-4: selecting a PARETO sorting mode with a single-criterion individual makes Pareto selection
    // degenerate. The EA must fall back to the corresponding single-eval sort (it warns once) and still
    // optimize correctly rather than mis-selecting or crashing.
    auto pop = std::make_shared<oa::GEvolutionaryAlgorithm>();
    pop->setPopulationSizes(18, 6);
    pop->setMaxIteration(120);
    pop->setReportIteration(100000);
    pop->setSortingScheme(Gem::Geneva::sortingMode::MUPLUSNU_PARETO); // multi-objective mode, single-objective problem
    FlatSphereOA src;
    pop->push_back(src.clone_unique());
    pop->setAdaptionConfig(src.buildAdaptionConfig());
    CHECK_NOTHROW(pop->optimize());

    auto best = pop->getBestGlobalIndividual<FlatSphereOA>();
    REQUIRE(best);
    CHECK(bestSphere(best) < 20.0); // still converges via the single-eval fallback
}

/******************************************************************************/

TEST_CASE("EA checkpoint round-trip preserves the per-slot adaption scratch", "[flat][oa]") {
    using gen::GFlatGenome;
    using Gem::Geneva::Genome::AUXKEY_GAUSS_DOUBLE;
    using Gem::Geneva::Genome::GaussState;

    // The per-individual OA scratch (adaption sigma/state) lives on the GIndividualSlot and is
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
    // The Gauss adaptor lives on the OA-owned config the individual authors (the genome is structure-only).
    auto cfg = dynamic_cast<gind::GLineFitIndividual &>(slot0->individual()).getAdaptionConfig();
    cfg->installInto(slot0->scratch());
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
    // The resumed algorithm adapts through the OA-owned config (the genome is structure-only); the
    // restored per-slot scratch is preserved, not re-seeded, by the resume path.
    resumed->setAdaptionConfig(
        dynamic_cast<gind::GLineFitIndividual &>(resumed->at(0)->individual()).getAdaptionConfig());
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
    // An algorithm uses an externally-supplied OA-owned config (Go2 hands it the one it
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
    auto &flat = dynamic_cast<gen::GFlatGenome &>(*ind);
    auto cfg = std::make_shared<oa::GEAAdaptionConfig>(flat);
    cfg->groupDouble(0).gauss(0.5, 0.8, 1e-3, 2., 1.); // author the Gauss settings explicitly

    pop->push_back(std::move(ind));
    pop->setAdaptionConfig(cfg);
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
    auto &oflat = dynamic_cast<gen::GFlatGenome &>(other);
    auto cfg = std::make_shared<oa::GEAAdaptionConfig>(oflat);

    pop->setAdaptionConfig(cfg);
    CHECK_THROWS(pop->optimize()); // checkConsistency rejects the mismatched config at init()
}

/******************************************************************************/

TEST_CASE("EA with no adaption config is a hard error", "[flat][oa]") {
    // The genome carries only structure; adaption intent is never inferred from it. An adapting algorithm
    // that is given no OA-owned config must fail loudly at setup rather than silently not adapting.
    auto pop = std::make_shared<oa::GEvolutionaryAlgorithm>();
    pop->setPopulationSizes(6, 2);
    pop->setMaxIteration(2);
    pop->setReportIteration(100000);
    pop->push_back(FlatSphereOA().clone_unique());
    CHECK_THROWS(pop->optimize()); // no setAdaptionConfig() -> init() hard-errors
}

/******************************************************************************/

TEST_CASE("Simulated annealing optimizes a flat individual", "[flat][oa]") {
    auto pop = std::make_shared<oa::GSimulatedAnnealing>();
    pop->setPopulationSizes(18, 6);
    pop->setMaxIteration(120);
    pop->setReportIteration(100000);
    FlatSphereOA src;
    pop->push_back(src.clone_unique());
    pop->setAdaptionConfig(src.buildAdaptionConfig());
    pop->optimize();

    auto best = pop->getBestGlobalIndividual<FlatSphereOA>();
    REQUIRE(best);
    CHECK(bestSphere(best) < 20.0);
}

/******************************************************************************/

TEST_CASE("Simulated annealing parameter validation and round-trip", "[flat][oa][sa]") {
    auto sa = std::make_shared<oa::GSimulatedAnnealing>();

    // Valid values round-trip
    sa->setT0(5.0);
    CHECK(sa->getT0() == 5.0);
    sa->setTDegradationStrength(0.7);
    CHECK(sa->getTDegradationStrength() == 0.7);

    // The start temperature must be strictly positive
    CHECK_THROWS(sa->setT0(0.0));
    CHECK_THROWS(sa->setT0(-1.0));

    // The cooling factor alpha must lie strictly inside (0, 1): alpha >= 1 would never cool,
    // alpha <= 0 is meaningless.
    CHECK_THROWS(sa->setTDegradationStrength(0.0));
    CHECK_THROWS(sa->setTDegradationStrength(1.0));
    CHECK_THROWS(sa->setTDegradationStrength(1.5));
    CHECK_THROWS(sa->setTDegradationStrength(-0.1));
}

/******************************************************************************/

TEST_CASE("Simulated annealing cools geometrically and floors above zero", "[flat][oa][sa]") {
    // Geometric cooling: with t0 and alpha fixed, the temperature after the run is t0 * alpha^N,
    // strictly decreasing and still positive. A strictly-positive floor matters because saProb()
    // divides by the temperature -- a temperature of exactly 0 would yield a 0/0 NaN.
    {
        auto pop = std::make_shared<oa::GSimulatedAnnealing>();
        pop->setPopulationSizes(18, 6);
        pop->setT0(10.0);
        pop->setTDegradationStrength(0.5);
        pop->setMaxIteration(40);
        pop->setMaxStallIteration(0); // run all 40 iterations, so the cooling is fully exercised
        pop->setReportIteration(100000);
        FlatSphereOA src;
        pop->push_back(src.clone_unique());
        pop->setAdaptionConfig(src.buildAdaptionConfig());
        pop->optimize();

        const double t = pop->getT();
        CHECK(std::isfinite(t));
        CHECK(t > 0.0);              // floor: never reaches 0 (would NaN saProb)
        CHECK(t < pop->getT0());     // the temperature cooled down
        CHECK(t < 1.0e-3);           // 10 * 0.5^40 is tiny -> substantial cooling after 40 iterations
    }

    // Drive the temperature hard into the subnormal range: it must clamp to a positive floor and
    // never become 0 or non-finite.
    {
        auto pop = std::make_shared<oa::GSimulatedAnnealing>();
        pop->setPopulationSizes(18, 6);
        pop->setT0(1.0e-305);
        pop->setTDegradationStrength(0.1);
        pop->setMaxIteration(50);
        pop->setMaxStallIteration(0);
        pop->setReportIteration(100000);
        FlatSphereOA src;
        pop->push_back(src.clone_unique());
        pop->setAdaptionConfig(src.buildAdaptionConfig());
        pop->optimize();

        const double t = pop->getT();
        CHECK(std::isfinite(t));
        CHECK(t > 0.0); // clamped to the smallest normalised double, not 0
    }
}

/******************************************************************************/

TEST_CASE("Swarm optimization optimizes a flat individual", "[flat][oa]") {
    auto pop = std::make_shared<oa::GSwarmAlgorithm>();
    pop->setSwarmSizes(3, 6); // 3 neighborhoods x 6 members
    pop->setMaxIteration(120);
    pop->setReportIteration(100000);
    pop->push_back(FlatSphereOA().clone_unique());
    pop->optimize();

    auto best = pop->getBestGlobalIndividual<FlatSphereOA>();
    REQUIRE(best);
    CHECK(bestSphere(best) < 20.0);
}

/******************************************************************************/

TEST_CASE("Swarm tolerates a frozen (equal-bound) parameter", "[flat][oa]") {
    // Regression for the PSO crash on a fixed parameter: a dimension with upper == lower has a velocity
    // range of l*(upper-lower) == 0; pruneVelocity() must clamp it to zero instead of throwing, and the
    // constrained fold must map the frozen value to its single point instead of dividing by a zero range.
    auto pop = std::make_shared<oa::GSwarmAlgorithm>();
    pop->setSwarmSizes(3, 6);
    pop->setMaxIteration(60);
    pop->setReportIteration(100000);
    pop->push_back(FlatFrozenOA().clone_unique());
    CHECK_NOTHROW(pop->optimize()); // must NOT crash on the frozen dimension

    auto best = pop->getBestGlobalIndividual<FlatFrozenOA>();
    REQUIRE(best);
    std::vector<double> v;
    best->streamline<double>(v);
    REQUIRE(v.size() == 3);
    CHECK(v[2] == 4.); // the frozen parameter stayed exactly at its fixed value
}

/******************************************************************************/

TEST_CASE("Swarm normalizes a non-canonical user population at setup", "[flat][oa]") {
    // Regression for the two self-admitted adjustPopulation_() bugs: when the user pushed more than
    // n_neighborhoods but not exactly default_pop_size, the population was either resized down to
    // n_neighborhoods (silently discarding the user's extra start individuals) or had ALL surplus dumped
    // into the last neighborhood (corrupting the topology). The setup must instead normalize to exactly
    // default_pop_size, every neighborhood at its default member count, without throwing.
    constexpr std::size_t n_neighborhoods = 3;
    constexpr std::size_t n_members = 6;
    constexpr std::size_t default_pop_size = n_neighborhoods * n_members; // 18

    SECTION("between n_neighborhoods and default_pop_size -> clone-fill up") {
        auto pop = std::make_shared<oa::GSwarmAlgorithm>();
        pop->setSwarmSizes(n_neighborhoods, n_members);
        pop->setMaxIteration(40);
        pop->setReportIteration(100000);
        for(std::size_t i = 0; i < 10; i++) { // 3 < 10 < 18
            pop->push_back(FlatSphereOA().clone_unique());
        }
        CHECK_NOTHROW(pop->optimize());
        CHECK(pop->size() == default_pop_size); // filled to capacity, nothing discarded mid-setup
        auto best = pop->getBestGlobalIndividual<FlatSphereOA>();
        REQUIRE(best);
        CHECK(bestSphere(best) < 20.0);
    }

    SECTION("above default_pop_size -> trim surplus to capacity") {
        auto pop = std::make_shared<oa::GSwarmAlgorithm>();
        pop->setSwarmSizes(n_neighborhoods, n_members);
        pop->setMaxIteration(40);
        pop->setReportIteration(100000);
        for(std::size_t i = 0; i < 25; i++) { // 25 > 18
            pop->push_back(FlatSphereOA().clone_unique());
        }
        CHECK_NOTHROW(pop->optimize());
        CHECK(pop->size() == default_pop_size); // surplus trimmed, topology intact (no last-neighborhood dump)
        auto best = pop->getBestGlobalIndividual<FlatSphereOA>();
        REQUIRE(best);
        CHECK(bestSphere(best) < 20.0);
    }
}

/******************************************************************************/

TEST_CASE("Conjugate gradient descent optimizes a flat individual", "[flat][oa]") {
    auto pop = std::make_shared<oa::GConjugateGradientDescent>();
    pop->setNStartingPoints(1);
    pop->setMaxIteration(500);
    pop->setReportIteration(100000);
    pop->push_back(FlatSphereWideOA().clone_unique());
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

TEST_CASE("Conjugate gradient descent: every beta formula converges", "[flat][oa]") {
    using gm = oa::gradientMethod;
    for(gm method : {gm::CONJUGATE_PR_PLUS, gm::CONJUGATE_FR, gm::CONJUGATE_HS, gm::CONJUGATE_DY}) {
        auto pop = std::make_shared<oa::GConjugateGradientDescent>();
        pop->setNStartingPoints(1);
        pop->setGradientMethod(method);
        pop->setMaxIteration(500);
        pop->setReportIteration(100000);
        pop->push_back(FlatSphereWideOA().clone_unique());
        pop->optimize();

        auto best = pop->getBestGlobalIndividual<FlatSphereWideOA>();
        REQUIRE(best);
        std::vector<double> v;
        best->streamline<double>(v);
        double sphere = 0.;
        for(double x : v) {
            sphere += x * x;
        }
        CHECK(sphere < 10.0); // every conjugate variant (FR / PR+ / HS+ / DY) descends from f=45
    }
}

/******************************************************************************/

TEST_CASE("Conjugate gradient descent: central-difference gradient converges", "[flat][oa]") {
    auto pop = std::make_shared<oa::GConjugateGradientDescent>();
    pop->setNStartingPoints(1);
    pop->setCentralDifferences(true); // O(h^2) gradient: two probe children per direction
    pop->setMaxIteration(500);
    pop->setReportIteration(100000);
    pop->push_back(FlatSphereWideOA().clone_unique());
    pop->optimize();

    CHECK(pop->getCentralDifferences());
    auto best = pop->getBestGlobalIndividual<FlatSphereWideOA>();
    REQUIRE(best);
    std::vector<double> v;
    best->streamline<double>(v);
    double sphere = 0.;
    for(double x : v) {
        sphere += x * x;
    }
    CHECK(sphere < 10.0);
}

/******************************************************************************/

TEST_CASE("Conjugate gradient descent: L-BFGS converges", "[flat][oa]") {
    for(std::size_t m : {std::size_t(3), std::size_t(10)}) { // small and default history sizes
        auto pop = std::make_shared<oa::GConjugateGradientDescent>();
        pop->setNStartingPoints(1);
        pop->setGradientMethod(oa::gradientMethod::LBFGS);
        pop->setLBFGSMemory(m);
        pop->setMaxIteration(500);
        pop->setReportIteration(100000);
        pop->push_back(FlatSphereWideOA().clone_unique());
        pop->optimize();

        CHECK(pop->getLBFGSMemory() == m);
        auto best = pop->getBestGlobalIndividual<FlatSphereWideOA>();
        REQUIRE(best);
        std::vector<double> v;
        best->streamline<double>(v);
        double sphere = 0.;
        for(double x : v) {
            sphere += x * x;
        }
        CHECK(sphere < 10.0); // the quasi-Newton direction descends from f=45 like the CG variants
    }
}

/******************************************************************************/

TEST_CASE("Nelder-Mead optimizes a flat individual", "[flat][oa]") {
    auto pop = std::make_shared<oa::GNelderMead>();
    pop->setMaxIteration(200);
    pop->setReportIteration(100000);
    pop->push_back(FlatSphereOA().clone_unique());
    pop->optimize();

    auto best = pop->getBestGlobalIndividual<FlatSphereOA>();
    REQUIRE(best);
    CHECK(bestSphere(best) < 5.0); // simplex descent on a sphere converges well
}

/******************************************************************************/

TEST_CASE("Nelder-Mead with oriented restart still converges", "[flat][oa]") {
    // Enabling the oriented restart must not break convergence: on a unimodal sphere a
    // restart re-explores around the best vertex and re-converges to the same optimum.
    auto pop = std::make_shared<oa::GNelderMead>();
    pop->setMaxIteration(300);
    pop->setReportIteration(100000);
    pop->setRestartThreshold(5); // restart every 5 stalled iterations
    pop->push_back(FlatSphereOA().clone_unique());
    pop->optimize();

    CHECK(pop->getRestartThreshold() == 5);
    auto best = pop->getBestGlobalIndividual<FlatSphereOA>();
    REQUIRE(best);
    CHECK(bestSphere(best) < 5.0);
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
    pop->optimize();

    auto best = pop->getBestGlobalIndividual<FlatSphereOA>();
    REQUIRE(best);
    // The scan swept the grid and returned the best grid point (near the origin); the grid's
    // resolution -- not the optimum -- bounds how close it gets.
    CHECK(bestSphere(best) < 5.0);
}

/******************************************************************************/
// Worked examples for the bi-gaussian + flip kernels, driven end-to-end by an EA.
/******************************************************************************/

TEST_CASE("EA optimizes a flat individual with a BI-GAUSSIAN adaptor", "[flat][oa][bigauss]") {
    auto pop = std::make_shared<oa::GEvolutionaryAlgorithm>();
    pop->setPopulationSizes(18, 6);
    pop->setMaxIteration(150);
    pop->setReportIteration(100000);
    FlatBiGaussSphereOA src;
    pop->push_back(src.clone_unique());
    pop->setAdaptionConfig(src.buildAdaptionConfig());
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
    FlatIntSphereOA src;
    pop->push_back(src.clone_unique());
    pop->setAdaptionConfig(src.buildAdaptionConfig());
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
    FlatOneMaxOA src;
    pop->push_back(src.clone_unique());
    pop->setAdaptionConfig(src.buildAdaptionConfig());
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
// Library individual: GLineFitIndividual is a flat genome. Beyond the standard
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
    gind::GLineFitIndividual src(data_points);
    pop->push_back(src.clone_unique());
    pop->setAdaptionConfig(src.getAdaptionConfig());
    pop->optimize();

    auto best = pop->getBestGlobalIndividual<gind::GLineFitIndividual>();
    REQUIRE(best);

    const auto [a, b] = best->getLine();
    CHECK(std::abs(a - 0.) < 0.5); // offset near 0
    CHECK(std::abs(b - 1.) < 0.3); // slope near 1
}
