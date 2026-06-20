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

#include <algorithm>
#include <cmath>
#include <limits>
#include <utility>
#include <cstddef>
#include <filesystem>
#include <memory>
#include <mutex>
#include <set>
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
#include "geneva/oa/GSepCmaEvolutionStrategy.hpp"
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

/******************************************************************************/
// Probe individuals for the parameter-scan tests. Their fitnessCalculation() records, thread-safely,
// the parameter values it is handed (evaluation runs on the local thread-pool consumer), so a test can
// inspect -- after optimize() has joined all work -- exactly which values the scan actually evaluated.

std::mutex g_scan_probe_mutex;
std::vector<std::int32_t> g_scan_int_samples;            ///< every int value the int probe was evaluated at
std::vector<std::pair<double, double>> g_scan_pair_samples; ///< every (x, y) the 2-double probe was evaluated at

/** @brief A single-int32 probe (genome bound [-10, 10]) recording each evaluated int value. */
class FlatScanIntProbe : public gen::GFlatIndividualT<FlatScanIntProbe> {
public:
    FlatScanIntProbe() {
        gen::GGenomeBuilder b;
        b.addInt32Group(1, -10, 10).init(0);
        this->setGenome(b.build());
    }
    FlatScanIntProbe(const FlatScanIntProbe &) = default;

protected:
    double fitnessCalculation() override {
        std::vector<std::int32_t> v;
        this->streamline<std::int32_t>(v);
        {
            std::lock_guard<std::mutex> lock(g_scan_probe_mutex);
            g_scan_int_samples.push_back(v[0]);
        }
        return static_cast<double>(v[0]) * static_cast<double>(v[0]);
    }
};

/** @brief A two-double probe (genome bounds [-5, 5)) recording each evaluated (x, y) pair. */
class FlatScanPairProbe : public gen::GFlatIndividualT<FlatScanPairProbe> {
public:
    FlatScanPairProbe() {
        gen::GGenomeBuilder b;
        b.addDoubleGroup(2, -5., 5.).init(0.0);
        this->setGenome(b.build());
    }
    FlatScanPairProbe(const FlatScanPairProbe &) = default;

protected:
    double fitnessCalculation() override {
        std::vector<double> v;
        this->streamline<double>(v);
        {
            std::lock_guard<std::mutex> lock(g_scan_probe_mutex);
            g_scan_pair_samples.emplace_back(v[0], v[1]);
        }
        double s = 0.;
        for(double x : v) {
            s += x * x;
        }
        return s;
    }
};

/** @brief A ONE-dimensional flat sphere: a single constrained double in [-5, 5), started at 3.0. Used to
 *  exercise the n_vert == 2 (1-D) Nelder-Mead simplex path. */
class FlatSphere1D : public gen::GFlatIndividualT<FlatSphere1D> {
public:
    FlatSphere1D() {
        gen::GGenomeBuilder b;
        b.addDoubleGroup(1, -5., 5.).init(3.0);
        this->setGenome(b.build());
    }
    FlatSphere1D(const FlatSphere1D &) = default;

protected:
    double fitnessCalculation() override {
        std::vector<double> v;
        this->streamline<double>(v);
        return v[0] * v[0];
    }
};

/** @brief A flat individual with INVERTED bounds (lower > upper): three constrained doubles declared as
 *  [5, -5]. The genome builder does not reject this, so it is used to confirm the swarm rejects the
 *  resulting negative velocity range with a clear error instead of hitting undefined behaviour. */
class FlatInvertedBoundsOA : public gen::GFlatIndividualT<FlatInvertedBoundsOA> {
public:
    FlatInvertedBoundsOA() {
        gen::GGenomeBuilder b;
        b.addDoubleGroup(3, 5., -5.); // lower = 5 > upper = -5 (intentionally inverted)
        this->setGenome(b.build());
    }
    FlatInvertedBoundsOA(const FlatInvertedBoundsOA &) = default;

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

/** @brief A MIXED flat individual: 3 constrained doubles in [-5, 5) plus 2 constrained int32 in [-10, 10].
 *  Used to confirm sep-CMA-ES (an FP-only evolution strategy) optimizes the doubles, leaves the integers
 *  at their start values, and warns rather than crashing on the non-FP parameters. */
class FlatMixedOA : public gen::GFlatIndividualT<FlatMixedOA> {
public:
    FlatMixedOA() {
        gen::GGenomeBuilder b;
        b.addDoubleGroup(3, -5., 5.).init(3.0);
        b.addInt32Group(2, -10, 10).init(7);
        this->setGenome(b.build());
    }
    FlatMixedOA(const FlatMixedOA &) = default;

protected:
    double fitnessCalculation() override {
        std::vector<double> v;
        this->streamline<double>(v);
        std::vector<std::int32_t> iv;
        this->streamline<std::int32_t>(iv);
        double s = 0.;
        for(double x : v) {
            s += x * x;
        }
        for(std::int32_t x : iv) {
            s += static_cast<double>(x) * static_cast<double>(x);
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

TEST_CASE("Inverted parameter bounds are rejected at genome build instead of UB", "[flat][oa]") {
    // A bounded floating-point parameter with lower > upper is malformed: the normalized coordinate model
    // requires a non-negative scale (upper - lower). setGenome() rejects inverted bounds up front with a
    // clear exception (at genome construction), rather than deferring to an OA that happens to trip over
    // the resulting negative range. (lower == upper stays valid -- a frozen parameter.)
    CHECK_THROWS(FlatInvertedBoundsOA()); // inverted bounds -> clean throw in setGenome(), not UB
}

/******************************************************************************/

TEST_CASE("Swarm with many neighborhoods does not overflow its bookkeeping", "[flat][oa]") {
    // Regression for a heap-buffer-overflow in fillUpNeighborhood1(): a default-constructed swarm sizes
    // n_neighborhood_members_cnt_ to the DEFAULT neighborhood count, but setSwarmSizes() can grow
    // n_neighborhoods_ beyond that; adjustPopulation_()->fillUpNeighborhood1() then indexed the (too
    // small) vector by neighborhood and wrote past its end. setSwarmSizes() now resizes the per-
    // neighborhood vectors in lockstep. Using more neighborhoods than the default (5) triggers the old
    // overflow (deterministically caught under AddressSanitizer; here it must simply run and converge).
    auto pop = std::make_shared<oa::GSwarmAlgorithm>();
    pop->setSwarmSizes(8, 8); // 8 neighborhoods (> the default), 64 particles
    pop->setMaxIteration(40);
    pop->setReportIteration(100000);
    pop->push_back(FlatSphereOA().clone_unique());
    CHECK_NOTHROW(pop->optimize());

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

TEST_CASE("Nelder-Mead optimizes a 1-D individual", "[flat][oa][nm]") {
    // A 1-D problem makes the simplex a 2-vertex segment (n_vert == 2), where the second-worst vertex IS
    // the best vertex (f_second == f_best) and the "accept reflection between best and second-worst"
    // branch correctly collapses. This exercises that degenerate path end-to-end and confirms it still
    // converges (rather than stalling).
    auto pop = std::make_shared<oa::GNelderMead>();
    pop->setMaxIteration(200);
    pop->setReportIteration(100000);
    pop->push_back(FlatSphere1D().clone_unique());
    pop->optimize();

    auto best = pop->getBestGlobalIndividual<FlatSphere1D>();
    REQUIRE(best);
    std::vector<double> v;
    best->streamline<double>(v);
    REQUIRE(v.size() == 1);
    CHECK(v[0] * v[0] < 1.0e-3); // the 1-D simplex descends from f = 9 to the optimum at the origin
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

TEST_CASE("Parameter scan grid clone round-trip preserves the grid", "[flat][oa][ps]") {
    // Regression for the copy ctor that dropped the GPodContainerT base: a cloned GRID scan
    // (scan_randomly = false) used to lose its pre-computed grid, so getCurrentItem -> at(step_) threw
    // during the sweep. We build a grid scan, CLONE it (via the copy ctor that clone() uses), and run the
    // clone: it must sweep the intact grid and find the origin (which is on a 3-step [-5, 5] grid).
    auto pop = std::make_shared<oa::GParameterScan>();
    pop->setScanRandomly(false); // GRID scan -> the grid points are pre-computed and must survive a clone
    pop->setParameterSpecs(
        "d(0, -5., 5., 3), d(1, -5., 5., 3), d(2, -5., 5., 3), d(3, -5., 5., 3), d(4, -5., 5., 3)"
    );
    pop->setMaxIteration(100000);
    pop->setMaxStallIteration(0);
    pop->setReportIteration(100000);
    pop->push_back(FlatSphereOA().clone_unique());

    // Deep-copy through the copy constructor (exactly what clone() / load_() use).
    auto clone = std::make_shared<oa::GParameterScan>(*pop);
    CHECK_NOTHROW(clone->optimize()); // would throw before the fix (empty grid -> at(step_) out of range)

    auto best = clone->getBestGlobalIndividual<FlatSphereOA>();
    REQUIRE(best);
    CHECK(bestSphere(best) < 1.0e-6); // the 3-step grid (-5, 0, 5) per dim includes the origin
}

/******************************************************************************/

TEST_CASE("Parameter scan random int stays within the inclusive bounds", "[flat][oa][ps]") {
    // Regression for the int32 random off-by-one: uniform_int_distribution treats both bounds as
    // inclusive, so passing upper + 1 let draws exceed the configured upper bound. A random scan over the
    // inclusive range [3, 5] must therefore never produce a 6.
    {
        std::lock_guard<std::mutex> lock(g_scan_probe_mutex);
        g_scan_int_samples.clear();
    }

    auto pop = std::make_shared<oa::GParameterScan>();
    pop->setScanRandomly(true); // random scan -> getRandomItem (inclusive bounds)
    pop->setParameterSpecs("i(0, 3, 5, 60)"); // 60 random draws in the inclusive range [3, 5]
    pop->setMaxIteration(100000);
    pop->setMaxStallIteration(0);
    pop->setReportIteration(100000);
    pop->push_back(FlatScanIntProbe().clone_unique());
    pop->optimize();

    std::lock_guard<std::mutex> lock(g_scan_probe_mutex);
    REQUIRE(g_scan_int_samples.size() >= 3);
    std::set<std::int32_t> seen;
    for(std::int32_t s : g_scan_int_samples) {
        CHECK(s >= 3);
        CHECK(s <= 5); // never 6 -- the former upper_ + 1 off-by-one
        seen.insert(s);
    }
    CHECK(seen.size() > 1); // the draws actually vary across the range (randomness happened)
}

/******************************************************************************/

TEST_CASE("Parameter scan grid covers exactly the product of the per-dimension steps", "[flat][oa][ps]") {
    // A 3x3 grid over two doubles must evaluate exactly 9 distinct combinations and cover the four
    // corners -- no missing points, no duplicates.
    {
        std::lock_guard<std::mutex> lock(g_scan_probe_mutex);
        g_scan_pair_samples.clear();
    }

    auto pop = std::make_shared<oa::GParameterScan>();
    pop->setScanRandomly(false); // grid scan
    pop->setParameterSpecs("d(0, -1., 1., 3), d(1, -1., 1., 3)"); // 3 steps each -> {-1, 0, 1}
    pop->setMaxIteration(100000);
    pop->setMaxStallIteration(0);
    pop->setReportIteration(100000);
    pop->push_back(FlatScanPairProbe().clone_unique());
    pop->optimize();

    std::lock_guard<std::mutex> lock(g_scan_probe_mutex);
    std::set<std::pair<double, double>> combos(g_scan_pair_samples.begin(), g_scan_pair_samples.end());
    CHECK(combos.size() == 9);                 // exactly product(n_steps) = 3 * 3
    CHECK(g_scan_pair_samples.size() == 9);    // and each grid point evaluated exactly once (no duplicates)
    // The four corners are present.
    CHECK(combos.count(std::make_pair(-1., -1.)) == 1);
    CHECK(combos.count(std::make_pair(-1., 1.)) == 1);
    CHECK(combos.count(std::make_pair(1., -1.)) == 1);
    CHECK(combos.count(std::make_pair(1., 1.)) == 1);
}

/******************************************************************************/

TEST_CASE("Parameter scan simple-scan evaluates exactly N random items", "[flat][oa][ps]") {
    // Regression for the simple-scan over-count + stale individual: the counter was incremented after the
    // population-full break (under-counting) and the population was trimmed to one too many (keeping an
    // un-initialized clone). setNSimpleScans(k) must evaluate exactly k random items.
    constexpr std::size_t k = 7;
    {
        std::lock_guard<std::mutex> lock(g_scan_probe_mutex);
        g_scan_pair_samples.clear();
    }

    auto pop = std::make_shared<oa::GParameterScan>();
    pop->setNSimpleScans(k); // simple-scan mode: randomly (re-)initialize whole individuals
    pop->setMaxIteration(100000);
    pop->setMaxStallIteration(0);
    pop->setReportIteration(100000);
    pop->push_back(FlatScanPairProbe().clone_unique());
    pop->optimize();

    CHECK(pop->getNScansPerformed() == k); // exact count, not k-1 (the former under-count)

    std::lock_guard<std::mutex> lock(g_scan_probe_mutex);
    CHECK(g_scan_pair_samples.size() == k); // exactly k items evaluated, no stale extra
}

/******************************************************************************/

TEST_CASE("Parameter scan compare detects scan-state differences", "[flat][oa][ps]") {
    using Gem::Common::expectation;

    // Two grid scans that differ ONLY in their parameter specs (same everything else) must compare
    // unequal -- which is only detectable if compare_ actually compares the scan-parameter vectors.
    auto a = std::make_shared<oa::GParameterScan>();
    a->setScanRandomly(false);
    a->setParameterSpecs("d(0, -5., 5., 3)");

    auto b = std::make_shared<oa::GParameterScan>();
    b->setScanRandomly(false);
    b->setParameterSpecs("d(0, -3., 3., 3)"); // same arity / steps, different bounds -> different grid

    CHECK_THROWS(a->compare(*b, expectation::EQUALITY, 0.)); // grids differ -> not equal

    // A clone compares equal (the grid and all scan state round-trip through the copy ctor).
    auto a_clone = std::make_shared<oa::GParameterScan>(*a);
    CHECK_NOTHROW(a->compare(*a_clone, expectation::EQUALITY, 0.));
}

/******************************************************************************/

TEST_CASE("Parameter scan survives a Boost serialization round-trip", "[flat][oa][ps]") {
    using Gem::Common::expectation;
    using Gem::Common::serializationMode;

    // The scan-parameter vectors are now folded into the single-source localMembers() route (they carry
    // the Gemfony common interface). This pins that serialize_members() (de)serializes them: a grid scan
    // written to a string and read back must compare EQUAL -- including its pre-computed grid points.
    auto a = std::make_shared<oa::GParameterScan>();
    a->setScanRandomly(false);
    a->setParameterSpecs("d(0, -5., 5., 3), i(1, 0, 4, 5), b(2)"); // mixed double / int32 / bool grids

    for(auto mode : {serializationMode::TEXT, serializationMode::XML, serializationMode::BINARY}) {
        const std::string archived = a->toString(mode);
        auto restored = std::make_shared<oa::GParameterScan>();
        restored->fromString(archived, mode);
        // Equal on every scan parameter (the grids included) only if the vectors round-tripped.
        CHECK_NOTHROW(a->compare(*restored, expectation::EQUALITY, 0.));
    }
}

/******************************************************************************/

TEST_CASE("Parameter scan with no scanned parameters does not crash", "[flat][oa][ps]") {
    // Regression for the empty-vector deref in switchToNextParameterSet(): with no scanned parameters
    // (and not in simple-scan mode) the central parameter vector is empty; advancing it must halt
    // cleanly instead of dereferencing all_par_cnt_.begin() on an empty container.
    auto pop = std::make_shared<oa::GParameterScan>();
    pop->setMaxIteration(10);
    pop->setMaxStallIteration(0);
    pop->setReportIteration(100000);
    pop->push_back(FlatSphereOA().clone_unique()); // no setParameterSpecs / setNSimpleScans
    CHECK_NOTHROW(pop->optimize());
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

/******************************************************************************/
// Separable CMA / CSA evolution strategy ("sepcma"): a from-scratch high-dimensional optimizer.
// O(n) per generation, dimension-scaled CSA + diagonal covariance constants -- it out-converges the
// stock self-adaptive EA at high n on the same fixed evaluation budget.
/******************************************************************************/

namespace {

/** @brief A high-dimensional flat sphere: N constrained doubles in [-5, 5), started at 3.0. */
template <std::size_t N>
class FlatHighDimSphere : public gen::GFlatIndividualT<FlatHighDimSphere<N>> {
public:
    FlatHighDimSphere() {
        gen::GGenomeBuilder b;
        b.addDoubleGroup(N, -5., 5.).init(3.0);
        this->setGenome(b.build());
    }
    FlatHighDimSphere(const FlatHighDimSphere &) = default;

    /** @brief Gauss adaption config for the stock EA (sepcma needs none -- it owns its own distribution). */
    std::shared_ptr<oa::GAdaptionConfigBase> buildAdaptionConfig() const {
        auto cfg = oa::makeAdaptionConfig<oa::GAdaptionConfigBase>(*this);
        cfg->groupDouble(0).gauss(0.5, 0.8, 1e-3, 2., 1.);
        return cfg;
    }

protected:
    double fitnessCalculation() override {
        std::vector<double> v;
        this->template streamline<double>(v);
        double s = 0.;
        for(double x : v) {
            s += x * x;
        }
        return s;
    }
};

/** @brief Sum of squares of a flat individual's double parameters (its sphere value). */
double sphereValue(const std::shared_ptr<gen::GOptimizableEntity> &best) {
    std::vector<double> v;
    best->streamline<double>(v);
    double s = 0.;
    for(double x : v) {
        s += x * x;
    }
    return s;
}

/**
 * @brief A two-objective flat individual: minimise f1 = sum x_i^2 and f2 = sum (x_i - 2)^2 over N_DIM
 * constrained doubles in [-5, 5). The Pareto front is the segment x_i in [0, 2]; used for the NSGA-II
 * selection smoke test.
 */
class FlatBiObjective : public gen::GFlatIndividualT<FlatBiObjective> {
public:
    FlatBiObjective() {
        gen::GGenomeBuilder b;
        b.addDoubleGroup(N_DIM, -5., 5.).init(3.0);
        this->setGenome(b.build());
        this->setNStoredResults(2); // two evaluation criteria
    }
    FlatBiObjective(const FlatBiObjective &) = default;

    /** @brief A Gauss adaption config for the EA (sep-CMA needs none). */
    std::shared_ptr<oa::GAdaptionConfigBase> buildAdaptionConfig() const {
        auto cfg = oa::makeAdaptionConfig<oa::GAdaptionConfigBase>(*this);
        cfg->groupDouble(0).gauss(0.5, 0.8, 1e-3, 2., 1.);
        return cfg;
    }

protected:
    double fitnessCalculation() override {
        std::vector<double> v;
        this->streamline<double>(v);
        double f1 = 0.;
        double f2 = 0.;
        for(double x : v) {
            f1 += x * x;
            f2 += (x - 2.) * (x - 2.);
        }
        this->setResult(1, f2);
        return f1; // criterion 0
    }
};

} /* anonymous namespace */

/******************************************************************************/

TEST_CASE("Separable CMA-ES optimizes a flat individual", "[flat][oa][sepcma]") {
    auto pop = std::make_shared<oa::GSepCmaEvolutionStrategy>();
    pop->setMaxIteration(200);
    pop->setReportIteration(100000);
    pop->push_back(FlatSphereOA().clone_unique());
    pop->optimize(); // sepcma owns its own distribution: NO adaption config needed

    auto best = pop->getBestGlobalIndividual<FlatSphereOA>();
    REQUIRE(best);
    CHECK(bestSphere(best) < 1.0e-3); // sep-CMA drives the sphere far below the f=45 start
}

/******************************************************************************/

TEST_CASE("Pure CSA-ES (no diagonal covariance) optimizes a flat individual", "[flat][oa][sepcma]") {
    auto pop = std::make_shared<oa::GSepCmaEvolutionStrategy>();
    pop->setUseDiagonalCMA(false); // step-size control only
    pop->setMaxIteration(300);
    pop->setReportIteration(100000);
    pop->push_back(FlatSphereOA().clone_unique());
    pop->optimize();

    auto best = pop->getBestGlobalIndividual<FlatSphereOA>();
    REQUIRE(best);
    CHECK(bestSphere(best) < 1.0e-2); // CSA alone still converges the isotropic sphere
}

/******************************************************************************/

TEST_CASE("Separable CMA-ES out-converges the stock EA at high dimension", "[flat][oa][sepcma]") {
    // The headline claim: on a high-dimensional sphere with a FIXED evaluation budget, the
    // dimension-scaled sep-CMA-ES reaches a far better fitness than the stock self-adaptive EA, whose
    // per-individual sigma adaption lacks the 1/n / 1/sqrt(n) scaling.
    constexpr std::size_t N = 200;
    using Ind = FlatHighDimSphere<N>;

    // Stock EA: (6 + 24) population, 120 generations -> 6 + 119*24 = 2862 evaluations.
    double ea_best = 0.;
    {
        auto pop = std::make_shared<oa::GEvolutionaryAlgorithm>();
        pop->setPopulationSizes(30, 6);
        pop->setMaxIteration(120);
        pop->setMaxStallIteration(0);
        pop->setReportIteration(100000);
        Ind src;
        pop->push_back(src.clone_unique());
        pop->setAdaptionConfig(src.buildAdaptionConfig());
        pop->optimize();
        auto best = pop->getBestGlobalIndividual<Ind>();
        REQUIRE(best);
        ea_best = sphereValue(best);
    }

    // sepcma: auto lambda (4 + floor(3 ln 200) = 19) over the same kind of budget (~150 gens), no config.
    double eab_best = 0.;
    {
        auto pop = std::make_shared<oa::GSepCmaEvolutionStrategy>();
        pop->setMaxIteration(150);
        pop->setMaxStallIteration(0);
        pop->setReportIteration(100000);
        pop->push_back(Ind().clone_unique());
        pop->optimize();
        auto best = pop->getBestGlobalIndividual<Ind>();
        REQUIRE(best);
        eab_best = sphereValue(best);
    }

    INFO("n=" << N << " stock-ea best=" << ea_best << "  sepcma best=" << eab_best);
    // The start fitness is N * 9 = 1800. sepcma must end well below the EA -- by a clear margin, not a
    // hair. (Observed: stock-ea ~1100, sepcma ~75 -- roughly a 15x lead at n=200.)
    CHECK(eab_best < ea_best);
    CHECK(eab_best < ea_best / 3.0); // a decisive margin, not a coin-flip win
    CHECK(eab_best < 0.1 * 1800.0);  // also an absolute bar: well below 10% of the f=1800 start
}

/******************************************************************************/

TEST_CASE("Separable CMA-ES tolerates (and warns about) non-FP parameters", "[flat][oa][sepcma]") {
    // sep-CMA-ES optimizes only the floating-point parameters. On a mixed individual (doubles + int32) it
    // must not crash: it warns at setup, optimizes the doubles, and leaves the integers at their start
    // value (7). This exercises the non-FP guard path end-to-end.
    auto pop = std::make_shared<oa::GSepCmaEvolutionStrategy>();
    pop->setMaxIteration(150);
    pop->setReportIteration(100000);
    pop->push_back(FlatMixedOA().clone_unique());
    CHECK_NOTHROW(pop->optimize()); // warns about the 2 int parameters, does not throw

    auto best = pop->getBestGlobalIndividual<FlatMixedOA>();
    REQUIRE(best);
    std::vector<double> v;
    best->streamline<double>(v);
    double fp_sphere = 0.;
    for(double x : v) {
        fp_sphere += x * x;
    }
    CHECK(fp_sphere < 1.0); // the FP parameters were driven down from the f=27 (doubles) start
    std::vector<std::int32_t> iv;
    best->streamline<std::int32_t>(iv);
    REQUIRE(iv.size() == 2);
    CHECK(iv[0] == 7); // the integer parameters were left untouched at their start value
    CHECK(iv[1] == 7);
}

/******************************************************************************/

TEST_CASE("Separable CMA-ES Pareto mode runs on a two-objective individual", "[flat][oa][sepcma][pareto]") {
    auto pop = std::make_shared<oa::GSepCmaEvolutionStrategy>();
    pop->setParetoMode(true); // NSGA-II non-dominated sort + crowding distance as the ranking key
    pop->setMaxIteration(120);
    pop->setReportIteration(100000);
    pop->push_back(FlatBiObjective().clone_unique());
    CHECK_NOTHROW(pop->optimize());

    auto best = pop->getBestGlobalIndividual<FlatBiObjective>();
    REQUIRE(best);
    // The Pareto front for (sum x^2, sum (x-2)^2) is the box x_i in [0, 2]. The global-best individual is
    // selected by criterion 0 (f1), so it sits at the f1 end of the front (x near 0): f1 is tiny while f2
    // is correspondingly large -- the textbook trade-off. We assert the run stays inside the genome box
    // and that the criterion-0-best really reached the f1 corner of the front, far below its f1=45 start.
    std::vector<double> v;
    best->streamline<double>(v);
    double f1 = 0.;
    for(double x : v) {
        f1 += x * x;
        CHECK(x >= -5.0);
        CHECK(x < 5.0);
    }
    CHECK(f1 < 10.0); // the criterion-0 best reached the f1 corner of the front (far below the f1=45 start)
}

/******************************************************************************/

TEST_CASE("ea NSGA-II Pareto selection spreads the survivors across the front", "[flat][oa][ea][pareto]") {
    // The two-objective front of (f1 = sum x^2, f2 = sum (x-2)^2) over N_DIM=5 dims runs from (0,20) to
    // (20,0). With NSGA-II selection (non-dominated front + crowding distance) the mu survivors should
    // spread ALONG that front rather than cluster -- exactly what the former first-front + random-shuffle
    // selection failed to do. We measure the spread (the f1 range the survivors cover) and the 2-D
    // hypervolume of the surviving front (which rewards BOTH convergence and coverage).
    constexpr std::size_t MU = 20;
    auto pop = std::make_shared<oa::GEvolutionaryAlgorithm>();
    pop->setPopulationSizes(3 * MU, MU);
    pop->setMaxIteration(200);
    pop->setMaxStallIteration(0);
    pop->setReportIteration(100000);
    pop->setSortingScheme(Gem::Geneva::sortingMode::MUPLUSNU_PARETO);
    FlatBiObjective src;
    pop->push_back(src.clone_unique());
    pop->setAdaptionConfig(src.buildAdaptionConfig());
    pop->optimize();

    // Read the mu surviving parents' two objective values.
    std::vector<std::pair<double, double>> pts; // (f1, f2)
    pts.reserve(pop->getNParents());
    for(std::size_t i = 0; i < pop->getNParents(); ++i) {
        const auto &ind = pop->at(i)->individual();
        pts.emplace_back(ind.transformed_fitness(0), ind.transformed_fitness(1));
    }
    REQUIRE(pts.size() == MU);

    // Spread: the f1-range covered by the survivors (the front's f1-extent is 20).
    double min_f1 = pts.front().first;
    double max_f1 = pts.front().first;
    for(const auto &p : pts) {
        min_f1 = (std::min)(min_f1, p.first);
        max_f1 = (std::max)(max_f1, p.first);
    }
    const double spread = max_f1 - min_f1;

    // 2-D hypervolume w.r.t. reference (20,20), minimisation: keep the non-dominated points (f1 ascending,
    // f2 strictly decreasing), then sum the dominated rectangles up to the reference.
    auto hypervolume = [](std::vector<std::pair<double, double>> p, double rx, double ry) {
        std::sort(p.begin(), p.end()); // by f1 ascending, then f2
        std::vector<std::pair<double, double>> nd;
        double best_f2 = std::numeric_limits<double>::infinity();
        for(const auto &q : p) {
            if(q.first >= rx || q.second >= ry) {
                continue; // outside the reference box -> contributes nothing
            }
            if(q.second < best_f2) {
                nd.push_back(q);
                best_f2 = q.second;
            }
        }
        double area = 0.;
        for(std::size_t i = 0; i < nd.size(); ++i) {
            const double next_f1 = (i + 1 < nd.size()) ? nd[i + 1].first : rx;
            area += (next_f1 - nd[i].first) * (ry - nd[i].second);
        }
        return area;
    };
    const double hv = hypervolume(pts, 20.0, 20.0);

    INFO("ea NSGA-II survivors: spread(f1 range)=" << spread << "  hypervolume=" << hv
         << "  (front f1-extent=20, ref=(20,20))");
    // Crowding always retains the two boundary points (infinite crowding distance), so the survivors
    // cover essentially the whole f1-extent of the front; a clustering selection would not. (Observed
    // spread ~20.0, hypervolume ~319 of the 400 reference box, with very low run-to-run variance.)
    CHECK(spread > 16.0); // near-full coverage of the 20-wide front (boundary retention), not a cluster
    CHECK(hv > 280.0);    // strong convergence AND spread (ideal ~ the 400 reference box)
}
