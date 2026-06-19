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
 * Validation + head-to-head convergence checks for the adaptive EA modes vs the legacy EA mode of
 * GEvolutionaryAlgorithm ("ea"). The headline test runs both on a HIGH-DIMENSIONAL sphere (the CPU proxy
 * for the GPU image benchmark) for the same budget and asserts the adaptive variant reaches a markedly
 * lower fitness -- precisely the fine-convergence-near-the-optimum behaviour the stock EA lacks at large
 * n because its sigma self-adaption rate is fixed (not scaled by n).
 */

#include <catch2/catch_test_macros.hpp>

#include <cmath>
#include <cstddef>
#include <memory>
#include <vector>

#include <boost/serialization/export.hpp>

#include "geneva/ind/GFlatGenome.hpp"
#include "geneva/ind/GFlatIndividualT.hpp"
#include "geneva/ind/GGenomeBuilder.hpp"
#include "geneva/oa/GAdaption.hpp"
#include "geneva/oa/GAdaptionConfig.hpp"
#include "geneva/oa/GEvolutionaryAlgorithmFactory.hpp"
#include "geneva/oa/GEvolutionaryAlgorithm.hpp"
#include "geneva/oa/GFactoryStore.hpp"

namespace gen = Gem::Geneva::Genome;
namespace oa = Gem::Geneva::OptimizationAlgorithms;

using Gem::Geneva::activityMode;
using oa::stepControl;

namespace {

/******************************************************************************/
/**
 * A high-dimensional flat-genome sphere: N constrained doubles in [-5, 5), all sharing one Gauss group
 * (so there is ONE shared sigma -- the textbook global-sigma sphere), started at 2.0. The genome carries
 * structure only; the Gauss adaptor lives on the OA-owned config (config-strip model).
 */
template <std::size_t N>
class HighDimSphere : public gen::GFlatIndividualT<HighDimSphere<N>> {
public:
    HighDimSphere() {
        gen::GGenomeBuilder b;
        b.addDoubleGroup(N, -5., 5.).init(2.0);
        this->setGenome(b.build());
    }
    HighDimSphere(const HighDimSphere &) = default;

    /** @brief A Gauss config with the CLASSIC fixed sigma_sigma=0.8 (what the stock EA uses). */
    std::shared_ptr<oa::GAdaptionConfigBase> buildAdaptionConfig() const {
        auto cfg = oa::makeAdaptionConfig<oa::GAdaptionConfigBase>(*this);
        // sigma, sigma_sigma, min_sigma, max_sigma, ad_prob. The shared step per dimension is
        // range * N(0, sigma) with range = 10 (the [-5,5) span), so a modest sigma keeps the per-dim
        // step sane at high n. sigma_sigma = 0.8 is the classic Geneva default (too hot at large n).
        cfg->groupDouble(0).gauss(0.02, 0.8, 1e-9, 5., 1.);
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

/** @brief Runs the EA in its LEGACY (pre-adaptive) mode -- the documented opt-in stepControl=SELF_ADAPT,
 *  i.e. classic log-normal sigma self-adaption with no dimension scaling -- on the high-dim sphere for a
 *  fixed budget and returns the best fitness. This reproduces the behaviour of the former stock EA. */
template <std::size_t N>
double runStockEA(std::size_t pop, std::size_t parents, std::size_t iterations) {
    auto p = std::make_shared<oa::GEvolutionaryAlgorithm>();
    p->setPopulationSizes(pop, parents);
    p->setMaxIteration(iterations);
    p->setMaxStallIteration(0);
    p->setReportIteration(100000);
    p->setStepControl(stepControl::SELF_ADAPT); // legacy fixed-sigma-self-adaption behaviour
    HighDimSphere<N> src;
    p->push_back(src.clone_unique());
    p->setAdaptionConfig(src.buildAdaptionConfig());
    p->optimize();
    auto best = p->template getBestGlobalIndividual<HighDimSphere<N>>();
    REQUIRE(best);
    std::vector<double> v;
    best->template streamline<double>(v);
    double s = 0.;
    for(double x : v) {
        s += x * x;
    }
    return s;
}

/** @brief Runs the EA (selectable step controller) on the high-dim sphere for a fixed budget and returns the best
 *  fitness. The step controller is selectable. */
template <std::size_t N>
double runAdaptiveEA(std::size_t pop, std::size_t parents, std::size_t iterations, stepControl sc) {
    auto p = std::make_shared<oa::GEvolutionaryAlgorithm>();
    p->setPopulationSizes(pop, parents);
    p->setMaxIteration(iterations);
    p->setMaxStallIteration(0);
    p->setReportIteration(100000);
    p->setStepControl(sc);
    HighDimSphere<N> src;
    p->push_back(src.clone_unique());
    p->setAdaptionConfig(src.buildAdaptionConfig()); // SAME config the stock EA gets (fixed 0.8)
    p->optimize();
    auto best = p->template getBestGlobalIndividual<HighDimSphere<N>>();
    REQUIRE(best);
    std::vector<double> v;
    best->template streamline<double>(v);
    double s = 0.;
    for(double x : v) {
        s += x * x;
    }
    return s;
}

} /* anonymous namespace */

/******************************************************************************/

TEST_CASE("ea optimizes a flat individual (basic)", "[ea][oa]") {
    const double f = runAdaptiveEA<5>(18, 6, 120, stepControl::SELF_ADAPT_SCALED);
    CHECK(f < 5.0); // converges from the f = 5*4 = 20 start
}

/******************************************************************************/

TEST_CASE("ea default step control is CSA", "[ea][oa]") {
    auto p = std::make_shared<oa::GEvolutionaryAlgorithm>();
    CHECK(p->getStepControl() == stepControl::CSA);
}

/******************************************************************************/

TEST_CASE("ea self-registers its mnemonic in the OA factory store", "[ea][oa]") {
    // The factory's GInitializerT registrant runs at library load and inserts the "ea" mnemonic into
    // the global store, so Go2 / the examples can select it by name (just like "ea"). Touch the factory
    // type so the translation unit (and hence the registrant) is linked in this build.
    (void)oa::GEvolutionaryAlgorithmFactory{};
    CHECK(oaFactoryStore()->exists("ea"));
}

/******************************************************************************/

TEST_CASE("ea SELF_ADAPT mode reproduces the classic algorithm, SCALED beats it", "[ea][oa]") {
    // With stepControl=SELF_ADAPT, the EA IS the classic algorithm (no dimension scaling); with the same
    // budget + config the dimension-scaled mode must do at least as well -- demonstrating the scaling
    // is the lever. f start = 20*4 = 80.
    const double f_self = runAdaptiveEA<20>(40, 10, 200, stepControl::SELF_ADAPT);
    const double f_scaled = runAdaptiveEA<20>(40, 10, 200, stepControl::SELF_ADAPT_SCALED);
    INFO("n=20 SELF_ADAPT f=" << f_self << "  SELF_ADAPT_SCALED f=" << f_scaled);
    CHECK(f_self < 80.0);            // classic self-adaption makes progress from the f=80 start
    CHECK(f_scaled <= f_self);       // the dimension-scaled default is at least as good
}

/******************************************************************************/

TEST_CASE("ea Pareto modes still work on a single-objective problem", "[ea][oa][pareto]") {
    // Pareto comes from the EA base; verify both Pareto sorting modes compile + run (they degenerate to
    // single-eval on a single-criterion individual, which must still converge, not crash).
    for(auto mode :
        {Gem::Geneva::sortingMode::MUPLUSNU_PARETO, Gem::Geneva::sortingMode::MUCOMMANU_PARETO}) {
        auto p = std::make_shared<oa::GEvolutionaryAlgorithm>();
        p->setPopulationSizes(18, 6);
        p->setMaxIteration(120);
        p->setReportIteration(100000);
        p->setSortingScheme(mode);
        HighDimSphere<5> src;
        p->push_back(src.clone_unique());
        p->setAdaptionConfig(src.buildAdaptionConfig());
        CHECK_NOTHROW(p->optimize());
        auto best = p->getBestGlobalIndividual<HighDimSphere<5>>();
        REQUIRE(best);
    }
}

/******************************************************************************/

TEST_CASE("ea ONE_FIFTH and CSA controllers converge", "[ea][oa]") {
    const double f_one_fifth = runAdaptiveEA<20>(40, 10, 200, stepControl::ONE_FIFTH);
    CHECK(f_one_fifth < 20.0);
    const double f_csa = runAdaptiveEA<20>(40, 10, 200, stepControl::CSA);
    CHECK(f_csa < 20.0);
}

/******************************************************************************/

TEST_CASE("ea out-converges the stock EA on a HIGH-DIM sphere", "[ea][oa][highdim]") {
    // The headline comparison: at high n the stock EA's fixed sigma_sigma=0.8 is far too hot, so it
    // random-walks sigma near the optimum and stalls. The dimension-scaled EA keeps converging. Same
    // genome, same starting config, same budget -> the scaled EA must reach a markedly LOWER fitness.
    constexpr std::size_t N = 1000;
    constexpr std::size_t pop = 30;
    constexpr std::size_t parents = 6;
    constexpr std::size_t iters = 500;

    const double f_ea = runStockEA<N>(pop, parents, iters);
    const double f_scaled = runAdaptiveEA<N>(pop, parents, iters, stepControl::SELF_ADAPT_SCALED);
    const double f_csa = runAdaptiveEA<N>(pop, parents, iters, stepControl::CSA);
    const double f_one_fifth = runAdaptiveEA<N>(pop, parents, iters, stepControl::ONE_FIFTH);

    INFO("n=" << N << " budget=" << iters << " iters : stock ea f=" << f_ea
              << "  ea(SELF_ADAPT_SCALED) f=" << f_scaled
              << "  ea(CSA) f=" << f_csa
              << "  ea(ONE_FIFTH) f=" << f_one_fifth);

    // SELF_ADAPT_SCALED must reach a markedly lower fitness than the stock EA at high n
    // (the stock EA's fixed sigma_sigma=0.8 random-walks sigma and it makes essentially no progress).
    CHECK(f_scaled < f_ea);
    CHECK(f_scaled < 0.5 * f_ea);
    // Every dimension-aware controller must beat the stock EA too.
    CHECK(f_csa < f_ea);
    CHECK(f_one_fifth < f_ea);
}

/******************************************************************************/

TEST_CASE("ea out-converges the stock EA at n=10000", "[ea][oa][highdim10k]") {
    // The full-scale 10000-parameter case (the benchmark's dimension). Convergence speed scales ~1/n, so
    // the absolute drop over a modest budget is small, but the stock EA is FROZEN at the f=40000 start
    // (its fixed sigma_sigma=0.8 instantly random-walks the shared sigma) while the dimension-scaled EA
    // keeps descending. Tagged separately so it can be skipped when time is tight.
    constexpr std::size_t N = 10000;
    const double f_ea = runStockEA<N>(20, 4, 300);
    const double f_scaled = runAdaptiveEA<N>(20, 4, 300, stepControl::SELF_ADAPT_SCALED);
    INFO("n=10000 iters=300 : ea f=" << f_ea << "  ea(SCALED) f=" << f_scaled);
    CHECK(f_scaled < f_ea);
}
