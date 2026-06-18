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
 * thread-pool) submission path -- builds, across the whole process, exactly ONE thread-pool consumer
 * (and hence one worker pool), not one per inner evaluation.
 *
 * This holds because GOptimizerExecutionPolicy::ensureExecutor_ resolves an un-injected local consumer
 * through the process-global per-kind broker registry: the first inner EA builds and registers the
 * thread-pool consumer; every later one shares it. The test clears the registry first so the count
 * reflects only the consumers built during this test.
 */

#include <catch2/catch_test_macros.hpp>

#include <cstddef>
#include <functional>
#include <memory>
#include <thread>
#include <vector>

#include "courtier/GBrokerRegistry.hpp"
#include "courtier/GBrokerT.hpp"
#include "courtier/consumers/GStdThreadConsumerT.hpp"
#include "geneva/GConsumerSetup.hpp"
#include "geneva/ind/GFlatIndividualT.hpp"
#include "geneva/ind/GGenomeBuilder.hpp"
#include "geneva/ind/GOptimizableEntity.hpp"
#include "geneva/oa/GAdaption.hpp"
#include "geneva/oa/GAdaptionConfig.hpp"
#include "geneva/oa/GEvolutionaryAlgorithm.hpp"
#include "geneva/oa/GMetaEvolutionaryAlgorithm.hpp"

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
    "[consumer][sharing]") {
    using StcConsumer = Gem::Courtier::GStdThreadConsumerT<gen::GOptimizableEntity>;

    // Start from an empty registry so the count reflects only the consumers built during this test,
    // and so the shared thread-pool consumer is genuinely built here (not inherited from an earlier test).
    Gem::Courtier::GBrokerRegistryT<gen::GOptimizableEntity>::instance().clearAll();

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
    // Every un-injected inner EA converges on ONE shared thread-pool consumer.
    CHECK(built == 1);
}

/******************************************************************************/

TEST_CASE("Broker registry holds one shared work broker", "[consumer][sharing][registry]") {
    using Registry = Gem::Courtier::GBrokerRegistryT<gen::GOptimizableEntity>;
    using broker_t = Gem::Courtier::GBrokerT<gen::GOptimizableEntity>;
    using Gem::Courtier::broker_kind;

    auto &reg = Registry::instance();
    reg.clearAll();

    CHECK(reg.sharedWorkBroker() == nullptr);

    // ensureSharedWork builds exactly once; later (and concurrent-shaped) calls share the same broker.
    std::size_t factory_calls = 0;
    auto factory = [&factory_calls]() {
        ++factory_calls;
        return std::make_shared<broker_t>();
    };
    auto first = reg.ensureSharedWork(broker_kind::multithreaded, factory);
    auto second = reg.ensureSharedWork(broker_kind::multithreaded, factory);
    CHECK(factory_calls == 1);     // the second call did NOT build a new broker
    CHECK(first == second);        // it returned the same shared work broker
    CHECK(reg.sharedWorkBroker() == first);
    CHECK(reg.sharedWorkKind() == broker_kind::multithreaded);

    // publishSharedWork replaces it (e.g. Go2 publishing the consumer it built); clearAll empties.
    auto replacement = std::make_shared<broker_t>();
    reg.publishSharedWork(broker_kind::networked, replacement);
    CHECK(reg.sharedWorkBroker() == replacement);
    CHECK(reg.sharedWorkKind() == broker_kind::networked);
    reg.clearAll();
    CHECK(reg.sharedWorkBroker() == nullptr);
}

/******************************************************************************/

TEST_CASE(
    "Concurrent algorithms fan in to one shared thread-pool consumer",
    "[consumer][sharing][stress]") {
    using StcConsumer = Gem::Courtier::GStdThreadConsumerT<gen::GOptimizableEntity>;

    // The shared thread-pool consumer is meant to serve several algorithms at once (the fan-in case):
    // each submitter waits on its OWN per-batch counter in dispatch_, not a global pool drain. Here K
    // independent EAs run on K separate threads, all un-injected so all converge on the ONE registered
    // thread-pool consumer. This exercises concurrent dispatch_ calls on the shared pool and asserts
    // (a) every algorithm still converges and (b) only ONE pool was built (bounded threads, no
    // per-algorithm oversubscription). The submitting threads are NOT pool workers, so they block on
    // their batches while the pool drains them -- no pool-reentrancy (a nested EA-in-EA on the SAME pool
    // would instead be split across kinds; see the two-tier meta-optimization model).
    Gem::Courtier::GBrokerRegistryT<gen::GOptimizableEntity>::instance().clearAll();
    StcConsumer::instances_constructed().store(0);

    constexpr int K = 4;
    std::vector<double> results(K, 1.0e9); // each thread writes only its own slot -> no data race

    std::vector<std::thread> threads;
    threads.reserve(K);
    for(int t = 0; t < K; ++t) {
        threads.emplace_back([t, &results]() {
            auto ea = std::make_shared<oa::GEvolutionaryAlgorithm>();
            ea->setPopulationSizes(12, 4);
            ea->setMaxIteration(40);
            ea->setReportIteration(100000);
            InnerSphere src;
            ea->push_back(src.clone_unique());
            ea->setAdaptionConfig(src.buildAdaptionConfig());
            // No setLocalConsumer(): the default thread-pool consumer is resolved (and shared) via the registry.
            ea->optimize();

            auto best = ea->getBestGlobalIndividual<InnerSphere>();
            std::vector<double> v;
            best->streamline<double>(v);
            double s = 0.;
            for(double x : v) {
                s += x * x;
            }
            results[t] = s;
        });
    }
    for(auto &th : threads) {
        th.join();
    }

    // Assertions on the main thread only (Catch2 macros are not thread-safe).
    for(double s : results) {
        CHECK(s < 20.0); // every concurrent algorithm converged (well below the f=27 start)
    }
    CHECK(StcConsumer::instances_constructed().load() == 1); // one shared pool for all K algorithms

    Gem::Courtier::GBrokerRegistryT<gen::GOptimizableEntity>::instance().clearAll();
}

/******************************************************************************/

TEST_CASE(
    "Meta-EA evaluates umbrella-individuals on its own pool; sub-EAs share the one work consumer",
    "[consumer][sharing][metaea]") {
    using StcConsumer = Gem::Courtier::GStdThreadConsumerT<gen::GOptimizableEntity>;

    // The meta-EA evaluates its umbrella-individuals (MetaSphere -- each runs an inner EA) on its OWN
    // orchestration pool, NOT the work consumer. The inner EAs, un-injected, all converge on the one
    // shared work consumer. Because the orchestration pool and the work consumer are distinct pools, an
    // umbrella-individual blocking on its inner EA never starves the pool it runs on -- no deadlock. (A
    // plain EA here would instead evaluate the umbrellas on the work consumer, whose workers would then
    // block on inner work posted back to the same pool -- the deadlock the meta-EA exists to avoid.)
    Gem::Courtier::GBrokerRegistryT<gen::GOptimizableEntity>::instance().clearAll();
    StcConsumer::instances_constructed().store(0);

    auto meta = std::make_shared<oa::GMetaEvolutionaryAlgorithm>();
    meta->setPopulationSizes(4, 2);
    meta->setMaxIteration(1);
    meta->setReportIteration(100000);
    meta->setNOrchestrationThreads(2);
    MetaSphere src;
    meta->push_back(src.clone_unique());
    meta->setAdaptionConfig(src.buildAdaptionConfig());
    meta->optimize();

    auto best = meta->getBestGlobalIndividual<MetaSphere>();
    REQUIRE(best); // completed without deadlock
    // Exactly one work consumer was built and shared by every inner EA; the meta-EA's own orchestration
    // pool is not a consumer, so it does not add to this count.
    CHECK(StcConsumer::instances_constructed().load() == 1);

    Gem::Courtier::GBrokerRegistryT<gen::GOptimizableEntity>::instance().clearAll();
}

/******************************************************************************/

TEST_CASE("buildConsumerSetup publishes the shared work broker", "[consumer][sharing][registry]") {
    using Registry = Gem::Courtier::GBrokerRegistryT<gen::GOptimizableEntity>;
    using Gem::Courtier::broker_kind;

    auto &reg = Registry::instance();
    reg.clearAll();

    auto stc = Gem::Geneva::buildConsumerSetup(Gem::Geneva::ConsumerSpec{.mnemonic = "stc"});
    REQUIRE(stc.broker);
    CHECK(reg.sharedWorkBroker() == stc.broker); // published, so un-injected algorithms submit through it
    CHECK(reg.sharedWorkKind() == broker_kind::multithreaded);

    // A later build replaces the single shared work broker (the process has one work endpoint).
    auto sc = Gem::Geneva::buildConsumerSetup(Gem::Geneva::ConsumerSpec{.mnemonic = "sc"});
    REQUIRE(sc.broker);
    CHECK(reg.sharedWorkBroker() == sc.broker);
    CHECK(reg.sharedWorkKind() == broker_kind::serial);

    reg.clearAll();
}
