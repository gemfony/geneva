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

TEST_CASE("Broker registry holds at most one broker per kind", "[consumer][sharing][registry]") {
    using Registry = Gem::Courtier::GBrokerRegistryT<gen::GOptimizableEntity>;
    using broker_t = Gem::Courtier::GBrokerT<gen::GOptimizableEntity>;
    using Gem::Courtier::broker_kind;

    auto &reg = Registry::instance();
    reg.clearAll();

    CHECK(reg.get(broker_kind::multithreaded) == nullptr);

    // getOrRegister builds exactly once for a kind; later (and concurrent-shaped) calls share it.
    std::size_t factory_calls = 0;
    auto factory = [&factory_calls]() {
        ++factory_calls;
        return std::make_shared<broker_t>();
    };
    auto first = reg.getOrRegister(broker_kind::multithreaded, factory);
    auto second = reg.getOrRegister(broker_kind::multithreaded, factory);
    CHECK(factory_calls == 1);     // the second call did NOT build a new broker
    CHECK(first == second);        // it returned the same shared broker
    CHECK(reg.get(broker_kind::multithreaded) == first);

    // Different kinds are independent.
    auto serial = reg.getOrRegister(broker_kind::serial, factory);
    CHECK(factory_calls == 2);
    CHECK(serial != first);

    // set() publishes/replaces; clear() drops one kind; clearAll() empties.
    auto replacement = std::make_shared<broker_t>();
    reg.set(broker_kind::multithreaded, replacement);
    CHECK(reg.get(broker_kind::multithreaded) == replacement);
    reg.clear(broker_kind::multithreaded);
    CHECK(reg.get(broker_kind::multithreaded) == nullptr);
    CHECK(reg.get(broker_kind::serial) == serial); // clear is per-kind
    reg.clearAll();
    CHECK(reg.get(broker_kind::serial) == nullptr);
}

/******************************************************************************/

TEST_CASE("buildConsumerSetup publishes the local broker under its kind", "[consumer][sharing][registry]") {
    using Registry = Gem::Courtier::GBrokerRegistryT<gen::GOptimizableEntity>;
    using Gem::Courtier::broker_kind;

    auto &reg = Registry::instance();
    reg.clearAll();

    auto stc = Gem::Geneva::buildConsumerSetup(Gem::Geneva::ConsumerSpec{.mnemonic = "stc"});
    REQUIRE(stc.broker);
    CHECK(reg.get(broker_kind::multithreaded) == stc.broker); // published, so un-injected OAs share it

    auto sc = Gem::Geneva::buildConsumerSetup(Gem::Geneva::ConsumerSpec{.mnemonic = "sc"});
    REQUIRE(sc.broker);
    CHECK(reg.get(broker_kind::serial) == sc.broker);
    CHECK(reg.get(broker_kind::multithreaded) == stc.broker); // the serial build left the stc entry intact

    reg.clearAll();
}
