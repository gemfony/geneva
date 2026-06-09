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
 * End-to-end checks that EVERY optimization algorithm can drive a flat-genome
 * individual (GFlatParameterSet), not just the EA. Each algorithm only ever
 * touches an individual through the storage-agnostic channel interface
 * (streamline / assignValueVector / boundaries / countParameters / adapt /
 * randomInit) + population-level access + personality traits, all of which the
 * flat node supports -- so no algorithm needed code changes; these tests prove it.
 */

#include <catch2/catch_test_macros.hpp>

#include <memory>
#include <vector>

#include "geneva/oa/GConjugateGradientDescent.hpp"
#include "geneva/oa/GEvolutionaryAlgorithm.hpp"
#include "geneva/oa/GNelderMead.hpp"
#include "geneva/oa/GParameterScan.hpp"
#include "geneva/oa/GSimulatedAnnealing.hpp"
#include "geneva/oa/GSwarmAlgorithm.hpp"
#include "geneva/par/GConstrainedDoubleObject.hpp"
#include "geneva/par/GDoubleObject.hpp"
#include "geneva/par/GFlatParameterSet.hpp"

namespace gpar = Gem::Geneva::Parameters;
namespace oa = Gem::Geneva::OptimizationAlgorithms;

namespace {

constexpr std::size_t N_DIM = 5;

/** @brief A flat-genome sphere individual: N_DIM constrained doubles in [-5, 5), start at 3.0. */
class FlatSphereIndividual : public gpar::GFlatParameterSet {
public:
    FlatSphereIndividual() {
        for(std::size_t i = 0; i < N_DIM; ++i) {
            this->push_back(std::make_shared<gpar::GConstrainedDoubleObject>(3.0, -5., 5.));
        }
        this->compileToFlat();
    }
    FlatSphereIndividual(const FlatSphereIndividual &) = default;

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

private:
    gpar::GParameterSet *clone_() const override {
        return new FlatSphereIndividual(*this);
    }
};

/** @brief An UNCONSTRAINED flat sphere individual (plain doubles, no fold). Used for the gradient
 *         method, whose line search probes points freely -- a constrained fold would distort those
 *         probes (a concern shared with the tree representation, not specific to the flat node). */
class FlatSphereUnconstrained : public gpar::GFlatParameterSet {
public:
    FlatSphereUnconstrained() {
        for(std::size_t i = 0; i < N_DIM; ++i) {
            this->push_back(std::make_shared<gpar::GDoubleObject>(3.0, -5., 5.)); // value + init range
        }
        this->compileToFlat();
    }
    FlatSphereUnconstrained(const FlatSphereUnconstrained &) = default;

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

private:
    gpar::GParameterSet *clone_() const override {
        return new FlatSphereUnconstrained(*this);
    }
};

/** @brief Sphere value of the best individual, also asserting the constraints held. */
double bestSphere(const std::shared_ptr<FlatSphereIndividual> &best) {
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

TEST_CASE("Simulated annealing optimizes a flat individual", "[flat][oa]") {
    auto pop = std::make_shared<oa::GSimulatedAnnealing>();
    pop->setPopulationSizes(18, 6);
    pop->setMaxIteration(120);
    pop->setReportIteration(100000);
    pop->push_back(FlatSphereIndividual().clone_unique());
    pop->setLocalConsumer(oa::local_consumer_kind::serial);
    pop->optimize();

    auto best = pop->getBestGlobalIndividual<FlatSphereIndividual>();
    REQUIRE(best);
    CHECK(bestSphere(best) < 20.0); // far below the f=45 start
}

/******************************************************************************/

TEST_CASE("Swarm optimization optimizes a flat individual", "[flat][oa]") {
    auto pop = std::make_shared<oa::GSwarmAlgorithm>();
    pop->setSwarmSizes(3, 6); // 3 neighborhoods x 6 members
    pop->setMaxIteration(120);
    pop->setReportIteration(100000);
    pop->push_back(FlatSphereIndividual().clone_unique());
    pop->setLocalConsumer(oa::local_consumer_kind::serial);
    pop->optimize();

    auto best = pop->getBestGlobalIndividual<FlatSphereIndividual>();
    REQUIRE(best);
    CHECK(bestSphere(best) < 20.0);
}

/******************************************************************************/

TEST_CASE("Conjugate gradient descent optimizes a flat individual", "[flat][oa]") {
    auto pop = std::make_shared<oa::GConjugateGradientDescent>();
    pop->setNStartingPoints(1);
    pop->setMaxIteration(500);
    pop->setReportIteration(100000);
    pop->push_back(FlatSphereUnconstrained().clone_unique());
    pop->setLocalConsumer(oa::local_consumer_kind::serial);
    pop->optimize();

    auto best = pop->getBestGlobalIndividual<FlatSphereUnconstrained>();
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
    pop->push_back(FlatSphereIndividual().clone_unique());
    pop->setLocalConsumer(oa::local_consumer_kind::serial);
    pop->optimize();

    auto best = pop->getBestGlobalIndividual<FlatSphereIndividual>();
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
    pop->setMaxIteration(100000); // enough iterations to cover the 5^5 grid
    pop->setReportIteration(100000);
    pop->push_back(FlatSphereIndividual().clone_unique());
    pop->setLocalConsumer(oa::local_consumer_kind::serial);
    pop->optimize();

    auto best = pop->getBestGlobalIndividual<FlatSphereIndividual>();
    REQUIRE(best);
    // The scan swept the grid and returned the best grid point (near the origin); the grid's
    // resolution -- not the optimum -- bounds how close it gets.
    CHECK(bestSphere(best) < 5.0);
}
