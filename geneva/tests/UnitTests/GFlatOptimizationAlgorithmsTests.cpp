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

#include <cstddef>
#include <memory>
#include <vector>

#include "geneva/ind/GFlatGenome.hpp"
#include "geneva/ind/GFlatIndividualT.hpp"
#include "geneva/ind/GGenomeBuilder.hpp"
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
