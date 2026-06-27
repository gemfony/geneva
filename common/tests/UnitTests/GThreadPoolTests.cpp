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
#include <chrono>
#include <cstdint>
#include <future>
#include <stdexcept>
#include <thread>
#include <type_traits>
#include <vector>

// Catch2 headers go here
#include <catch2/catch_test_macros.hpp>

// Geneva headers go here
#include "common/concurrency/GThreadPool.hpp"

using Gem::Common::Concurrency::GThreadPool;
using namespace std::chrono_literals;

// NOTE: Catch2 assertion macros are NOT thread-safe, so callables running inside
// the pool never call REQUIRE/CHECK; they return values (checked via futures on
// the main thread) or update std::atomics (checked after wait()).

/******************************************************************************/
// Construction / configuration.

TEST_CASE("GThreadPool construction and getNThreads", "[common][thread-pool]") {
    GThreadPool pool(4);
    // Workers start eagerly in the constructor, so getNThreads() reports the live
    // count right away (this is the modernised behaviour: the old ASIO-based pool
    // created threads lazily on the first submission and reported 0 until then).
    REQUIRE(pool.getNThreads() == 4);

    GThreadPool zero(0); // 0 -> hardware default, never 0
    REQUIRE(zero.getNThreads() > 0);
}

TEST_CASE("GThreadPool is neither copyable nor movable", "[common][thread-pool]") {
    static_assert(not std::is_copy_constructible_v<GThreadPool>);
    static_assert(not std::is_copy_assignable_v<GThreadPool>);
    static_assert(not std::is_move_constructible_v<GThreadPool>);
    static_assert(not std::is_move_assignable_v<GThreadPool>);
    static_assert(not std::is_default_constructible_v<GThreadPool>);
    SUCCEED("compile-time type traits hold");
}

/******************************************************************************/
// async_schedule: non-void return type -> result carried by the future.

TEST_CASE("GThreadPool non-void tasks return their result via the future",
          "[common][thread-pool]") {
    GThreadPool pool(4);
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
// async_schedule: void return type -> future is satisfied, side effects run.

TEST_CASE("GThreadPool void tasks complete and the future is satisfied",
          "[common][thread-pool]") {
    GThreadPool pool(4);
    std::atomic<int> counter{0};
    auto f = pool.async_schedule([&counter]() { counter.fetch_add(1); });
    f.get(); // must not throw / hang (void specialisation sets the value)
    REQUIRE(counter.load() == 1);
}

/******************************************************************************/
// Exception handling: a throwing task surfaces through the returned future.

TEST_CASE("GThreadPool propagates task exceptions through the future",
          "[common][thread-pool]") {
    GThreadPool pool(4);
    SECTION("void task") {
        auto f = pool.async_schedule([]() { throw std::runtime_error("boom"); });
        REQUIRE_THROWS_AS(f.get(), std::runtime_error);
    }
    SECTION("non-void task") {
        auto f = pool.async_schedule([]() -> int { throw std::logic_error("nope"); });
        REQUIRE_THROWS_AS(f.get(), std::logic_error);
    }
    // The pool survives throwing tasks and keeps working.
    auto ok = pool.async_schedule([]() { return 7; });
    REQUIRE(ok.get() == 7);
}

/******************************************************************************/
// wait(): blocks until all submitted work has completed.

TEST_CASE("GThreadPool wait() drains all submitted work", "[common][thread-pool]") {
    GThreadPool pool(8);
    constexpr int N = 5000;
    std::atomic<int> done{0};
    for(int i = 0; i < N; ++i) {
        (void)pool.async_schedule([&done]() { done.fetch_add(1); });
    }
    pool.wait();
    REQUIRE(done.load() == N); // every task ran before wait() returned
}

TEST_CASE("GThreadPool wait() is safe on an empty / already-drained pool",
          "[common][thread-pool]") {
    GThreadPool pool(2);
    pool.wait();             // nothing submitted -> returns immediately
    auto f = pool.async_schedule([]() { return 1; });
    REQUIRE(f.get() == 1);
    pool.wait();             // already drained via .get() -> returns immediately
    pool.wait();             // idempotent
    SUCCEED("repeated wait() calls did not hang");
}

TEST_CASE("GThreadPool genuinely waits for slow in-flight tasks", "[common][thread-pool]") {
    GThreadPool pool(2);
    std::atomic<int> done{0};
    for(int i = 0; i < 8; ++i) {
        (void)pool.async_schedule([&done]() {
            std::this_thread::sleep_for(5ms);
            done.fetch_add(1);
        });
    }
    pool.wait();
    REQUIRE(done.load() == 8); // wait() did not return early
}

/******************************************************************************/
// MPMC conservation: a large number of futures, each carrying a distinct value.

TEST_CASE("GThreadPool MPMC conservation (futures)", "[common][thread-pool]") {
    GThreadPool pool(8);
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
// Multi-producer submission: several threads submit concurrently (exercises the
// shared submission lock and the MPMC queue's producer side under contention).

TEST_CASE("GThreadPool tolerates concurrent submission from many producers",
          "[common][thread-pool]") {
    GThreadPool pool(8);
    constexpr int PRODUCERS = 6;
    constexpr int PER_PRODUCER = 2000;
    constexpr int TOTAL = PRODUCERS * PER_PRODUCER;

    // Pre-sized vector with disjoint index ranges per producer: no future is ever
    // touched by more than one thread, so no extra synchronisation is needed.
    std::vector<std::future<long long>> fs(TOTAL);

    std::vector<std::thread> producers;
    producers.reserve(PRODUCERS);
    for(int p = 0; p < PRODUCERS; ++p) {
        producers.emplace_back([&pool, &fs, p]() {
            for(int i = 0; i < PER_PRODUCER; ++i) {
                const int idx = p * PER_PRODUCER + i;
                fs[static_cast<std::size_t>(idx)] =
                    pool.async_schedule([](long long k) { return k; }, static_cast<long long>(idx));
            }
        });
    }
    for(auto &t : producers) t.join();

    long long sum = 0;
    for(auto &f : fs) sum += f.get();
    REQUIRE(sum == static_cast<long long>(TOTAL) * (TOTAL - 1) / 2);
}

/******************************************************************************/
// setNThreads: grows and shrinks while staying functional. Mirrors the old
// behaviour the manual GThreadPoolTest exercised, but as a fast unit test.

TEST_CASE("GThreadPool setNThreads grows and shrinks, staying functional",
          "[common][thread-pool]") {
    GThreadPool pool(2);
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

    pool.setNThreads(0); // 0 -> hardware default, never 0
    REQUIRE(pool.getNThreads() > 0);
    REQUIRE(run_batch(1000) == 1000);
}

TEST_CASE("GThreadPool survives repeated resize-under-load cycles",
          "[common][thread-pool]") {
    // Condensed analogue of the manual stress test: submit a batch, resize, wait,
    // and confirm every task ran in every cycle.
    GThreadPool pool(2);
    constexpr int CYCLES = 12;
    constexpr int JOBS = 500;
    const unsigned int sizes[] = {4u, 1u, 8u, 3u};

    for(int c = 0; c < CYCLES; ++c) {
        std::atomic<int> done{0};
        for(int i = 0; i < JOBS; ++i) {
            (void)pool.async_schedule([&done]() { done.fetch_add(1); });
        }
        pool.setNThreads(sizes[static_cast<std::size_t>(c) % 4]); // drains, then resizes
        pool.wait();
        REQUIRE(done.load() == JOBS); // resize+wait flushed the whole batch
    }
}

/******************************************************************************/
// Clean shutdown: the destructor must drain pending work and join without hanging.

TEST_CASE("GThreadPool clean shutdown with pending work (no hang)",
          "[common][thread-pool]") {
    std::atomic<int> done{0};
    {
        GThreadPool pool(4);
        for(int i = 0; i < 2000; ++i) {
            (void)pool.async_schedule([&done]() { done.fetch_add(1); });
        }
        // No explicit wait(): the destructor must drain the queue (run all tasks)
        // and join cleanly.
    }
    REQUIRE(done.load() == 2000); // destructor drained everything
}

/******************************************************************************/
// post(): fire-and-forget submission (no future, exceptions logged-not-propagated).

TEST_CASE("GThreadPool post() is fire-and-forget and survives throwing tasks",
          "[common][thread-pool]") {
    GThreadPool pool(4);
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
