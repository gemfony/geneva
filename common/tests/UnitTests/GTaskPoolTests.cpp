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

// Standard headers go here
#include <atomic>
#include <cstdint>
#include <future>
#include <stdexcept>
#include <vector>

// Catch2 headers go here
#include <catch2/catch_test_macros.hpp>

// Geneva headers go here
#include "common/GTaskPool.hpp"

using Gem::Common::GTaskPool;

// NOTE: Catch2 assertion macros are NOT thread-safe, so tasks running in the pool
// never call REQUIRE/CHECK; they return values (checked via futures on the main
// thread) or update std::atomics (checked after wait()).

/******************************************************************************/

TEST_CASE("GTaskPool construction and getNThreads", "[GTaskPool]") {
    GTaskPool pool(4);
    REQUIRE(pool.getNThreads() == 4); // workers start eagerly -> live count

    GTaskPool zero(0); // 0 -> hardware default, never 0
    REQUIRE(zero.getNThreads() > 0);
}

/******************************************************************************/

TEST_CASE("GTaskPool non-void tasks return their result via the future", "[GTaskPool]") {
    GTaskPool pool(4);
    auto f = pool.async_schedule([](int x) { return x * 2; }, 21);
    REQUIRE(f.get() == 42);

    // Several at once, with bound arguments forwarded.
    std::vector<std::future<int>> fs;
    for(int i = 0; i < 100; ++i) {
        fs.push_back(pool.async_schedule([](int a, int b) { return a + b; }, i, 1000));
    }
    for(int i = 0; i < 100; ++i) {
        REQUIRE(fs[static_cast<std::size_t>(i)].get() == i + 1000);
    }
}

/******************************************************************************/

TEST_CASE("GTaskPool void tasks complete and the future is satisfied", "[GTaskPool]") {
    GTaskPool pool(4);
    std::atomic<int> counter{0};
    auto f = pool.async_schedule([&counter]() { counter.fetch_add(1); });
    f.get(); // must not throw / hang (void specialisation sets the value)
    REQUIRE(counter.load() == 1);
}

/******************************************************************************/

TEST_CASE("GTaskPool propagates task exceptions through the future", "[GTaskPool]") {
    GTaskPool pool(4);
    SECTION("void task") {
        auto f = pool.async_schedule([]() { throw std::runtime_error("boom"); });
        REQUIRE_THROWS_AS(f.get(), std::runtime_error);
    }
    SECTION("non-void task") {
        auto f = pool.async_schedule([]() -> int { throw std::logic_error("nope"); });
        REQUIRE_THROWS_AS(f.get(), std::logic_error);
    }
}

/******************************************************************************/

TEST_CASE("GTaskPool wait() drains all submitted work", "[GTaskPool]") {
    GTaskPool pool(8);
    constexpr int N = 5000;
    std::atomic<int> done{0};
    for(int i = 0; i < N; ++i) {
        (void)pool.async_schedule([&done]() { done.fetch_add(1); });
    }
    pool.wait();
    REQUIRE(done.load() == N); // every task ran before wait() returned
}

/******************************************************************************/

TEST_CASE("GTaskPool MPMC conservation (futures)", "[GTaskPool]") {
    GTaskPool pool(8);
    constexpr int N = 20000;
    std::vector<std::future<long long>> fs;
    fs.reserve(N);
    for(int i = 0; i < N; ++i) {
        fs.push_back(pool.async_schedule([](long long k) { return k; }, static_cast<long long>(i)));
    }
    long long sum = 0;
    for(auto &f : fs) sum += f.get();
    REQUIRE(sum == static_cast<long long>(N) * (N - 1) / 2);
}

/******************************************************************************/

TEST_CASE("GTaskPool setNThreads grows and shrinks, staying functional", "[GTaskPool]") {
    GTaskPool pool(2);
    REQUIRE(pool.getNThreads() == 2);

    auto run_batch = [&pool](int n) {
        std::atomic<int> done{0};
        for(int i = 0; i < n; ++i) {
            (void)pool.async_schedule([&done]() { done.fetch_add(1); });
        }
        pool.wait();
        return done.load();
    };

    REQUIRE(run_batch(1000) == 1000);

    pool.setNThreads(6); // grow
    REQUIRE(pool.getNThreads() == 6);
    REQUIRE(run_batch(1000) == 1000);

    pool.setNThreads(1); // shrink (recreates the queue + workers)
    REQUIRE(pool.getNThreads() == 1);
    REQUIRE(run_batch(1000) == 1000);

    pool.setNThreads(1); // no-op
    REQUIRE(pool.getNThreads() == 1);
}

/******************************************************************************/

TEST_CASE("GTaskPool clean shutdown with pending work (no hang)", "[GTaskPool]") {
    std::atomic<int> done{0};
    {
        GTaskPool pool(4);
        for(int i = 0; i < 2000; ++i) {
            (void)pool.async_schedule([&done]() { done.fetch_add(1); });
        }
        // No explicit wait(): the destructor must drain the queue (run all tasks)
        // and join cleanly.
    }
    REQUIRE(done.load() == 2000); // destructor drained everything
}

/******************************************************************************/

TEST_CASE("GTaskPool post() is fire-and-forget and survives throwing tasks", "[GTaskPool]") {
    GTaskPool pool(4);
    std::atomic<int> done{0};
    constexpr int N = 3000;
    for(int i = 0; i < N; ++i) {
        pool.post([&done]() { done.fetch_add(1); }); // no future returned
    }
    // Throwing fire-and-forget tasks must be logged-and-ignored, not crash the pool.
    pool.post([]() { throw std::runtime_error("ignored on purpose"); });
    pool.post([]() { throw 42; }); // non-std exception

    pool.wait(); // post() tasks participate in wait()
    REQUIRE(done.load() == N); // all non-throwing tasks ran; throwers did not crash the pool

    // The pool remains usable after a thrown-and-logged task.
    auto f = pool.async_schedule([]() { return 7; });
    REQUIRE(f.get() == 7);
}
