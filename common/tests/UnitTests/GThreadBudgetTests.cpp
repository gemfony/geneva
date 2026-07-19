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

#include <catch2/catch_test_macros.hpp>

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "common/concurrency/GThreadBudget.hpp"
#include "common/concurrency/GThreadPool.hpp"

using namespace Gem::Common::Concurrency;

// NOTE: threadBudget() is a process-global shared with every other pool alive in this test
// binary, so all assertions are DELTA-based against a baseline taken inside the test case.

/******************************************************************************/
// Reservation accounting

TEST_CASE("GThreadBudget: reserve/release round-trip is delta-exact",
          "[common][concurrency][thread-budget]") {
    auto &budget = threadBudget();
    const unsigned int base = budget.reserved();

    {
        auto r = budget.reserve("test:a", 4, ThreadElasticity::Elastic);
        CHECK(r.granted() == 4);
        CHECK(budget.reserved() == base + 4);

        auto r2 = budget.reserve("test:b", 3, ThreadElasticity::Fixed);
        CHECK(r2.granted() == 3);
        CHECK(budget.reserved() == base + 7);
    }

    CHECK(budget.reserved() == base);
}

TEST_CASE("GThreadBudget: a reservation always grants at least one thread",
          "[common][concurrency][thread-budget]") {
    auto &budget = threadBudget();
    auto r = budget.reserve("test:zero", 0, ThreadElasticity::Elastic);
    CHECK(r.granted() == 1);
}

TEST_CASE("GThreadBudget: moved-from handles release exactly once",
          "[common][concurrency][thread-budget]") {
    auto &budget = threadBudget();
    const unsigned int base = budget.reserved();

    auto r = budget.reserve("test:move", 5, ThreadElasticity::Elastic);
    GThreadBudget::Reservation moved = std::move(r);
    CHECK(moved.granted() == 5);
    CHECK(budget.reserved() == base + 5);

    r.release(); // releasing a moved-from (empty) handle is a no-op
    CHECK(budget.reserved() == base + 5);

    moved.release();
    CHECK(budget.reserved() == base);
    moved.release(); // idempotent
    CHECK(budget.reserved() == base);
}

TEST_CASE("GThreadBudget: oversubscribed() reflects the reserved total vs the hardware ceiling",
          "[common][concurrency][thread-budget]") {
    auto &budget = threadBudget();

    // Reserve well past the ceiling; delta-safe because the handle releases at scope exit.
    auto r = budget.reserve("test:huge", budget.ceiling() * 3 + 1, ThreadElasticity::Fixed);
    CHECK(budget.oversubscribed());
}

/******************************************************************************/
// The budgeted GThreadPool constructor

TEST_CASE("GThreadPool: the budgeted constructor reserves for the pool's lifetime",
          "[common][concurrency][thread-budget]") {
    auto &budget = threadBudget();
    const unsigned int base = budget.reserved();

    {
        GThreadPool const pool("test:pool", 3, ThreadElasticity::Elastic);
        CHECK(pool.getNThreads() == 3);
        CHECK(budget.reserved() == base + 3);
    }

    CHECK(budget.reserved() == base);
}

/******************************************************************************/
// Elastic granting (P1): only a NESTED elastic reservation may be shrunk

TEST_CASE("GThreadBudget: a nested elastic reservation is granted only the remaining budget",
          "[common][concurrency][thread-budget]") {
    auto &budget = threadBudget();

    // Fill the budget to (at least) the ceiling, then ask for a large nested elastic share.
    auto filler = budget.reserve("test:filler", budget.ceiling(), ThreadElasticity::Fixed);
    auto nested =
        budget.reserve("test:nested", budget.ceiling() * 4, ThreadElasticity::Elastic, true);

    // Whatever was reserved before this test, the nested grant cannot exceed the remaining
    // budget -- and with the filler holding >= ceiling it must have shrunk to the minimum.
    CHECK(nested.granted() == 1);
}

TEST_CASE("GThreadBudget: a top-level reservation is always granted in full",
          "[common][concurrency][thread-budget]") {
    auto &budget = threadBudget();

    auto filler = budget.reserve("test:filler", budget.ceiling() * 2, ThreadElasticity::Fixed);
    // Top-level (nested == false): granted in full even though the budget is exhausted --
    // shrinking by construction order would starve a legitimate top-level pool.
    auto top = budget.reserve("test:top", 5, ThreadElasticity::Elastic);
    CHECK(top.granted() == 5);
}

TEST_CASE("GThreadPool: a budgeted pool built inside another pool's worker is budget-bounded",
          "[common][concurrency][thread-budget]") {
    auto &budget = threadBudget();

    // The meta-optimization shape end-to-end: an outer budgeted pool whose worker constructs an
    // inner budgeted pool. The inner construction happens with inWorkerThread() == true, so its
    // grant is bounded by the remaining budget.
    auto filler = budget.reserve("test:filler", budget.ceiling() * 2, ThreadElasticity::Fixed);

    GThreadPool outer("test:outer", 1, ThreadElasticity::Elastic);
    unsigned int inner_threads = 0;
    outer
        .async_schedule([&inner_threads]() {
            GThreadPool const inner("test:inner", 64, ThreadElasticity::Elastic);
            inner_threads = inner.getNThreads();
        })
        .get();

    // The budget was exhausted by the filler, so the nested pool must have shrunk to one worker
    // (it would have started 64 without the budget -- the hw^2 pathology).
    CHECK(inner_threads == 1);
}

/******************************************************************************/
// The oversubscription warning

namespace {

/** @brief A log target capturing every message routed to it */
struct CapturingTarget : Gem::Common::GBaseLogTarget {
    void log(const std::string &msg) const override { messages.push_back(msg); }
    void logWithSource(const std::string &msg, const std::string &) const override {
        messages.push_back(msg);
    }
    mutable std::vector<std::string> messages;
};

} // anonymous namespace

TEST_CASE("GThreadBudget: crossing the warning threshold emits one warning naming the source",
          "[common][concurrency][thread-budget]") {
    auto &budget = threadBudget();

    auto capture = std::make_shared<CapturingTarget>();
    glogger.addLogTarget(capture);

    {
        // Push the total past ceiling * factor in one reservation; the warning must fire once.
        const auto excess = static_cast<unsigned int>(
            static_cast<double>(budget.ceiling()) * GThreadBudget::OVERSUBSCRIPTION_FACTOR
        );
        auto r = budget.reserve("test:overflow", excess + budget.ceiling(), ThreadElasticity::Fixed);

        bool found = false;
        for(const auto &m : capture->messages) {
            if(m.find("test:overflow") != std::string::npos
               && m.find("GThreadBudget") != std::string::npos) {
                found = true;
            }
        }
        CHECK(found);
    }

    glogger.resetLogTargets();
}

/******************************************************************************/
// The device registry (a sibling facility; tested here alongside the thread budget)

#include "common/concurrency/GDeviceRegistry.hpp"

TEST_CASE("GDeviceRegistry: acquire/release round-trip per device id",
          "[common][concurrency][device-registry]") {
    auto &reg = deviceRegistry();
    const unsigned int base0 = reg.users(0);
    const unsigned int base1 = reg.users(1);

    {
        auto u0 = reg.acquire("test:gpu-consumer", 0);
        CHECK(reg.users(0) == base0 + 1);
        CHECK(reg.users(1) == base1);

        auto u0b = reg.acquire("test:rng", 0);
        CHECK(reg.users(0) == base0 + 2);
        CHECK(reg.contended(0));

        auto u1 = reg.acquire("test:other-device", 1);
        CHECK(reg.users(1) == base1 + 1);
    }

    CHECK(reg.users(0) == base0);
    CHECK(reg.users(1) == base1);
}

TEST_CASE("GDeviceRegistry: moved-from handles release exactly once",
          "[common][concurrency][device-registry]") {
    auto &reg = deviceRegistry();
    const unsigned int base = reg.users(0);

    auto u = reg.acquire("test:move", 0);
    GDeviceRegistry::DeviceUse moved = std::move(u);
    CHECK(reg.users(0) == base + 1);

    u.release(); // empty handle: no-op
    CHECK(reg.users(0) == base + 1);
    moved.release();
    CHECK(reg.users(0) == base);
    moved.release(); // idempotent
    CHECK(reg.users(0) == base);
}
