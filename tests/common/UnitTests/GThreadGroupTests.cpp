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
#include <memory>
#include <thread>

#include "common/GThreadGroup.hpp"

using namespace Gem::Common;
using namespace std::chrono_literals;

// ---------------------------------------------------------------------------
// Empty group

TEST_CASE("GThreadGroup: empty group reports size 0 and joins cleanly",
          "[common][thread-group]") {
    GThreadGroup g;
    CHECK(g.size() == 0);
    CHECK_NOTHROW(g.join_all());
}

// ---------------------------------------------------------------------------
// create_thread / create_threads / join_all

TEST_CASE("GThreadGroup::create_thread runs the supplied callable",
          "[common][thread-group]") {
    GThreadGroup g;
    std::atomic<int> counter{0};

    auto t = g.create_thread([&counter] { ++counter; });
    REQUIRE(t);
    CHECK(g.size() == 1);

    g.join_all();
    CHECK(counter.load() == 1);
}

TEST_CASE("GThreadGroup::create_threads spawns N threads with the same callable",
          "[common][thread-group]") {
    GThreadGroup g;
    std::atomic<int> counter{0};
    constexpr std::size_t N = 8;

    g.create_threads([&counter] { ++counter; }, N);
    CHECK(g.size() == N);

    g.join_all();
    CHECK(counter.load() == static_cast<int>(N));
}

// ---------------------------------------------------------------------------
// add_thread: takes an externally constructed std::thread

TEST_CASE("GThreadGroup::add_thread accepts a pre-built thread",
          "[common][thread-group]") {
    GThreadGroup g;
    std::atomic<int> counter{0};
    auto t = std::make_shared<std::thread>([&counter] { ++counter; });

    g.add_thread(t);
    CHECK(g.size() == 1);

    g.join_all();
    CHECK(counter.load() == 1);
}

// ---------------------------------------------------------------------------
// join_all waits for in-flight work.

TEST_CASE("GThreadGroup::join_all blocks until in-flight work completes",
          "[common][thread-group]") {
    GThreadGroup g;
    std::atomic<bool> done{false};

    g.create_thread([&done] {
        std::this_thread::sleep_for(20ms);
        done.store(true);
    });

    // Pre-join, the worker should still be executing (small but real delay).
    g.join_all();
    CHECK(done.load());
}
