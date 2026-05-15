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

#include <atomic>
#include <chrono>
#include <future>
#include <stdexcept>
#include <type_traits>

#include "common/GThreadPool.hpp"

using namespace Gem::Common;
using namespace std::chrono_literals;

// ---------------------------------------------------------------------------
// Construction / configuration

TEST_CASE("GThreadPool: getNThreads is zero before first task (threads not yet started)",
          "[common][thread-pool]") {
    // Threads are created lazily on the first async_schedule call, so
    // getNThreads() — which reports the live thread-group size — is zero
    // until a task is submitted.
    GThreadPool tp(3);
    CHECK(tp.getNThreads() == 0u);
}

TEST_CASE("GThreadPool::async_schedule triggers thread creation; getNThreads then reflects pool size",
          "[common][thread-pool]") {
    GThreadPool tp(3);
    auto fut = tp.async_schedule([] {});
    fut.get();
    CHECK(tp.getNThreads() == 3u);
}

TEST_CASE("GThreadPool::setNThreads: setting before any submission leaves getNThreads at 0",
          "[common][thread-pool]") {
    GThreadPool tp(2);
    tp.setNThreads(6);   // No threads have been created yet; this is a no-op for the live pool.
    CHECK(tp.getNThreads() == 0u);
}

TEST_CASE("GThreadPool: non-copyable / non-movable",
          "[common][thread-pool]") {
    static_assert(not std::is_copy_constructible_v<GThreadPool>);
    static_assert(not std::is_move_constructible_v<GThreadPool>);
}

// ---------------------------------------------------------------------------
// async_schedule (void return type): task runs and the future is satisfied.

TEST_CASE("GThreadPool::async_schedule (void task) runs the callable",
          "[common][thread-pool]") {
    GThreadPool tp(2);
    std::atomic<int> counter{0};

    auto fut = tp.async_schedule([&counter] { ++counter; });
    fut.get();           // blocks until done; rethrows on exception
    CHECK(counter.load() == 1);
}

TEST_CASE("GThreadPool: many tasks all complete",
          "[common][thread-pool]") {
    GThreadPool tp(4);
    std::atomic<int> counter{0};
    constexpr int N = 64;

    std::vector<std::future<void>> futs;
    futs.reserve(N);
    for(int i = 0; i < N; ++i) {
        futs.push_back(tp.async_schedule([&counter] { ++counter; }));
    }
    for(auto &f : futs) f.get();

    CHECK(counter.load() == N);
}

// ---------------------------------------------------------------------------
// async_schedule (non-void return type): future carries the result.

TEST_CASE("GThreadPool::async_schedule (non-void task) returns the result via future",
          "[common][thread-pool]") {
    GThreadPool tp(2);
    auto fut = tp.async_schedule([] { return 42; });
    CHECK(fut.get() == 42);
}

TEST_CASE("GThreadPool::async_schedule: arguments are forwarded to the callable",
          "[common][thread-pool]") {
    GThreadPool tp(2);
    auto fut = tp.async_schedule([](int a, int b) { return a + b; }, 10, 32);
    CHECK(fut.get() == 42);
}

// ---------------------------------------------------------------------------
// Exception handling: a throwing task surfaces via the returned future.

TEST_CASE("GThreadPool::async_schedule: exception from task propagates via future",
          "[common][thread-pool]") {
    GThreadPool tp(2);
    auto fut = tp.async_schedule([] {
        throw std::runtime_error("boom");
    });
    CHECK_THROWS_AS(fut.get(), std::runtime_error);
}

// ---------------------------------------------------------------------------
// wait(): blocks until pending tasks have completed.

TEST_CASE("GThreadPool::wait: blocks until in-flight tasks finish",
          "[common][thread-pool]") {
    GThreadPool tp(2);
    std::atomic<int> done{0};

    for(int i = 0; i < 8; ++i) {
        tp.async_schedule([&done] {
            std::this_thread::sleep_for(5ms);
            ++done;
        });
    }
    tp.wait();
    CHECK(done.load() == 8);
}
