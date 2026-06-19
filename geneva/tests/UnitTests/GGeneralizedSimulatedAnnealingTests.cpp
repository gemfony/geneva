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
 * Convergence checks for the from-scratch Generalized (Dual) Simulated Annealing (GSA; Tsallis/Stariolo,
 * Xiang et al. 1997) algorithm on flat-genome individuals. GSA is an FP-only, archive-free global
 * optimizer that runs P independent Markov chains with a heavy-tailed Tsallis visiting distribution, a
 * generalized acceptance rule and a power-law cooling schedule. It needs no adaption config (it mutates
 * by sampling, not via per-parameter adaptors). This is distinct from the classic geometric-cooling
 * Metropolis GSimulatedAnnealing.
 */

#include <catch2/catch_test_macros.hpp>

#include <cmath>
#include <cstddef>
#include <memory>
#include <numbers>
#include <vector>

#include "geneva/ind/GFlatIndividualT.hpp"
#include "geneva/ind/GGenomeBuilder.hpp"
#include "geneva/oa/GGeneralizedSimulatedAnnealing.hpp"

namespace gen = Gem::Geneva::Genome;
namespace oa = Gem::Geneva::OptimizationAlgorithms;

namespace {

/******************************************************************************/
/**
 * A flat-genome sphere individual: a configurable number of constrained doubles in [-5, 5), started at
 * 3.0 (so the initial fitness is n_dim * 9). Authored entirely through the builder; GSA needs NO
 * adaption config. fitness = sum of squares (optimum: the origin, f = 0).
 */
template <std::size_t N_DIM>
class FlatSphereGSA : public gen::GFlatIndividualT<FlatSphereGSA<N_DIM>> {
public:
    FlatSphereGSA() {
        gen::GGenomeBuilder b;
        b.addDoubleGroup(N_DIM, -5., 5.).init(3.0); // structure only; GSA needs no adaptor
        this->setGenome(b.build());
    }
    FlatSphereGSA(const FlatSphereGSA &) = default;

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

/******************************************************************************/
/**
 * A flat-genome Rastrigin individual: a configurable number of constrained doubles in [-5.12, 5.12),
 * started at 3.0. fitness = 10 n + sum_i ( x_i^2 - 10 cos(2 pi x_i) ). The global optimum is the origin
 * with f = 0; the landscape is highly multimodal (a regular lattice of local minima), so it stresses the
 * heavy-tailed visiting jumps and generalized acceptance that let GSA escape local optima.
 */
template <std::size_t N_DIM>
class FlatRastriginGSA : public gen::GFlatIndividualT<FlatRastriginGSA<N_DIM>> {
public:
    FlatRastriginGSA() {
        gen::GGenomeBuilder b;
        b.addDoubleGroup(N_DIM, -5.12, 5.12).init(3.0); // structure only; GSA needs no adaptor
        this->setGenome(b.build());
    }
    FlatRastriginGSA(const FlatRastriginGSA &) = default;

protected:
    double fitnessCalculation() override {
        std::vector<double> v;
        this->template streamline<double>(v);
        double s = 10. * static_cast<double>(v.size());
        for(double x : v) {
            s += x * x - 10. * std::cos(2. * std::numbers::pi * x);
        }
        return s;
    }
};

/** @brief Sphere value of the best individual, also asserting the constraints held. */
template <std::size_t N_DIM>
double bestSphere(const std::shared_ptr<FlatSphereGSA<N_DIM>> &best) {
    std::vector<double> v;
    best->template streamline<double>(v);
    double s = 0.;
    for(double x : v) {
        s += x * x;
        CHECK(x >= -5.0);
        CHECK(x < 5.0);
    }
    return s;
}

/** @brief Rastrigin value of the best individual, also asserting the constraints held. */
template <std::size_t N_DIM>
double bestRastrigin(const std::shared_ptr<FlatRastriginGSA<N_DIM>> &best) {
    std::vector<double> v;
    best->template streamline<double>(v);
    double s = 10. * static_cast<double>(v.size());
    for(double x : v) {
        s += x * x - 10. * std::cos(2. * std::numbers::pi * x);
        CHECK(x >= -5.12);
        CHECK(x < 5.12);
    }
    return s;
}

} /* anonymous namespace */

/******************************************************************************/

TEST_CASE("Generalized Simulated Annealing optimizes a 5-dim flat sphere", "[gsa]") {
    auto pop = std::make_shared<oa::GGeneralizedSimulatedAnnealing>();
    pop->setNChains(8);
    pop->setMaxIteration(3000);
    pop->setMaxStallIteration(0); // 0 == disabled: run the full budget so the chains fully converge
    pop->setReportIteration(100000);
    pop->push_back(FlatSphereGSA<5>().clone_unique());
    pop->optimize();

    auto best = pop->getBestGlobalIndividual<FlatSphereGSA<5>>();
    REQUIRE(best);
    CHECK(bestSphere<5>(best) < 1.e-2); // far below the f = 5 * 9 = 45 start
}

/******************************************************************************/

TEST_CASE("Generalized Simulated Annealing optimizes a 10-dim flat sphere", "[gsa]") {
    auto pop = std::make_shared<oa::GGeneralizedSimulatedAnnealing>();
    pop->setNChains(10);
    pop->setMaxIteration(4000);
    pop->setMaxStallIteration(0); // 0 == disabled: run the full budget so the chains fully converge
    pop->setReportIteration(100000);
    pop->push_back(FlatSphereGSA<10>().clone_unique());
    pop->optimize();

    auto best = pop->getBestGlobalIndividual<FlatSphereGSA<10>>();
    REQUIRE(best);
    CHECK(bestSphere<10>(best) < 1.0); // well below the f = 10 * 9 = 90 start
}

/******************************************************************************/

TEST_CASE("Generalized Simulated Annealing reaches a good value on a multimodal Rastrigin", "[gsa]") {
    auto pop = std::make_shared<oa::GGeneralizedSimulatedAnnealing>();
    pop->setNChains(12);
    pop->setMaxIteration(4000);
    pop->setMaxStallIteration(0);   // run the full budget
    pop->setReannealingSteps(400);  // periodically re-open the cooling clock to escape local optima
    pop->setReportIteration(100000);
    pop->push_back(FlatRastriginGSA<5>().clone_unique());
    pop->optimize();

    auto best = pop->getBestGlobalIndividual<FlatRastriginGSA<5>>();
    REQUIRE(best);
    // Soft bound: the 3.0 start sits at f ~ 5*(9 - 10*cos(6pi)) + 50 = 50 (a high local plateau). A good
    // run should drop well into the low-lying basins, escaping many of the lattice of local minima.
    CHECK(bestRastrigin<5>(best) < 10.0);
}

/******************************************************************************/

TEST_CASE("Generalized Simulated Annealing parameter validation", "[gsa]") {
    auto gsa = std::make_shared<oa::GGeneralizedSimulatedAnnealing>();

    gsa->setNChains(4);
    CHECK(gsa->getNChains() == 4);
    gsa->setQv(2.0);
    CHECK(gsa->getQv() == 2.0);
    gsa->setQa(-3.0);
    CHECK(gsa->getQa() == -3.0);
    gsa->setT0(5.0);
    CHECK(gsa->getT0() == 5.0);
    gsa->setReannealingSteps(50);
    CHECK(gsa->getReannealingSteps() == 50);

    CHECK_THROWS(gsa->setNChains(0)); // need at least 1 chain
    CHECK_THROWS(gsa->setQv(1.0));    // q_v must be in ]1,3[
    CHECK_THROWS(gsa->setQv(3.0));    // q_v must be in ]1,3[
    CHECK_THROWS(gsa->setQa(1.0));    // q_a must not be exactly 1
    CHECK_THROWS(gsa->setT0(-1.0));   // t0 must be >= 0
}

/******************************************************************************/
