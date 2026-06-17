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
 * The single-shared-consumer invariant: at most one consumer of a given KIND per process, shared by
 * every algorithm that needs that kind. This file pins the thread-pool (stc) case: a nested
 * EA-in-EA -- an outer EA whose individuals each run an inner EA through the default (un-injected ->
 * thread-pool) submission path -- must, across the whole process, build exactly ONE thread-pool
 * consumer (and hence one worker pool), not one per inner evaluation.
 *
 * TODAY this is violated: GOptimizerExecutionPolicy::ensureExecutor_ builds a fresh per-OA consumer
 * whenever no broker was injected, so every inner EA spawns its own GThreadPool. The test asserts the
 * TARGET (one consumer) and is tagged Catch2 [!shouldfail] so the suite stays green until the
 * process-global broker registry (Workstream A1-A3) makes the assertion pass -- at which point the
 * [!shouldfail] tag is removed.
 */

#include <catch2/catch_test_macros.hpp>

#include <cstddef>
#include <memory>
#include <vector>

#include "courtier/consumers/GStdThreadConsumerT.hpp"
#include "geneva/ind/GFlatIndividualT.hpp"
#include "geneva/ind/GGenomeBuilder.hpp"
#include "geneva/ind/GOptimizableEntity.hpp"
#include "geneva/oa/GAdaption.hpp"
#include "geneva/oa/GAdaptionConfig.hpp"
#include "geneva/oa/GEvolutionaryAlgorithm.hpp"

namespace gen = Gem::Geneva::Genome;
namespace oa = Gem::Geneva::OptimizationAlgorithms;

namespace {

/** @brief A tiny flat sphere: 3 constrained doubles in [-5, 5), started at 3.0. The inner problem. */
class InnerSphere : public gen::GFlatIndividualT<InnerSphere> {
public:
    InnerSphere() {
        gen::GGenomeBuilder b;
        b.addDoubleGroup(3, -5., 5.).init(3.0);
        this->setGenome(b.build());
    }
    InnerSphere(const InnerSphere &) = default;

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

/**
 * @brief A meta individual whose evaluation RUNS an inner EA over InnerSphere through the default
 * (un-injected) submission path -- i.e. exactly the nesting the invariant targets. Its own genome is
 * one constrained double (so the outer EA has something to adapt); the value is unused.
 */
class MetaSphere : public gen::GFlatIndividualT<MetaSphere> {
public:
    MetaSphere() {
        gen::GGenomeBuilder b;
        b.addDoubleGroup(1, -5., 5.).init(3.0);
        this->setGenome(b.build());
    }
    MetaSphere(const MetaSphere &) = default;

    std::shared_ptr<oa::GAdaptionConfigBase> buildAdaptionConfig() const {
        auto cfg = oa::makeAdaptionConfig<oa::GAdaptionConfigBase>(*this);
        cfg->groupDouble(0).gauss(0.5, 0.8, 1e-3, 2., 1.);
        return cfg;
    }

protected:
    double fitnessCalculation() override {
        auto inner = std::make_shared<oa::GEvolutionaryAlgorithm>();
        inner->setPopulationSizes(4, 2);
        inner->setMaxIteration(2);
        inner->setReportIteration(100000);
        InnerSphere src;
        inner->push_back(src.clone_unique());
        inner->setAdaptionConfig(src.buildAdaptionConfig());
        // Deliberately NO setLocalConsumer(): init() defaults to the thread-pool consumer, which is the
        // un-injected path the registry must funnel into the single shared consumer.
        inner->optimize();

        auto best = inner->getBestGlobalIndividual<InnerSphere>();
        std::vector<double> v;
        best->streamline<double>(v);
        double s = 0.;
        for(double x : v) {
            s += x * x;
        }
        return s;
    }
};

} /* anonymous namespace */

/******************************************************************************/

TEST_CASE(
    "Nested EA-in-EA shares a single thread-pool consumer",
    "[consumer][sharing][!shouldfail]") {
    using StcConsumer = Gem::Courtier::GStdThreadConsumerT<gen::GOptimizableEntity>;

    // The outer EA is serial so it builds no thread-pool consumer of its own: the count then reflects
    // ONLY the inner (thread-pool) consumers, isolating the invariant under test.
    StcConsumer::instances_constructed().store(0);

    auto outer = std::make_shared<oa::GEvolutionaryAlgorithm>();
    outer->setPopulationSizes(4, 2);
    outer->setMaxIteration(1);
    outer->setReportIteration(100000);
    MetaSphere src;
    outer->push_back(src.clone_unique());
    outer->setAdaptionConfig(src.buildAdaptionConfig());
    outer->setLocalConsumer(oa::local_consumer_kind::serial);
    outer->optimize();

    const std::size_t built = StcConsumer::instances_constructed().load();
    // Invariant target: every un-injected inner EA converges on ONE shared thread-pool consumer.
    // Pre-A1 this is the number of inner evaluations (far more than one), so the test fails today.
    CHECK(built == 1);
}
