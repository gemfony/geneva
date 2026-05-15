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
#include <memory>
#include <set>
#include <thread>
#include <type_traits>
#include <vector>

#include "common/GSingletonT.hpp"

using namespace Gem::Common;

// ---------------------------------------------------------------------------
// Helper payload types. Each TEST_CASE uses a fresh type so the static
// per-template singleton state is isolated — tests cannot pollute each other.

namespace {

struct Payload_Default {
    int marker = 42;
};

struct Payload_Instance {};

struct Payload_Reset {};

struct Payload_Concurrent {
    static std::atomic<int> ctor_count;
    Payload_Concurrent() {
        ++ctor_count;
    }
};
std::atomic<int> Payload_Concurrent::ctor_count{0};

struct Payload_DefaultMode {};

// A type without a default constructor — only reachable via a specialised
// TFactory_GSingletonT<>. Tests the documented customisation point.
struct Payload_FactorySpecialised {
    explicit Payload_FactorySpecialised(int v) : value(v) {}
    int value;
};

} // namespace

// Custom factory for the no-default-ctor type. Must be declared in the same
// namespace as the primary template so ADL/standard-lookup can find it.
namespace Gem::Common {
template <>
std::shared_ptr<Payload_FactorySpecialised> TFactory_GSingletonT<Payload_FactorySpecialised>() {
    return std::make_shared<Payload_FactorySpecialised>(7);
}
} // namespace Gem::Common

// ---------------------------------------------------------------------------
// Static interface contract

TEST_CASE("GSingletonT: is non-instantiable and exposes STYPE", "[common][singleton]") {
    static_assert(not std::is_default_constructible_v<GSingletonT<Payload_Default>>);
    static_assert(not std::is_copy_constructible_v<GSingletonT<Payload_Default>>);
    static_assert(not std::is_move_constructible_v<GSingletonT<Payload_Default>>);
    static_assert(std::is_same_v<GSingletonT<Payload_Default>::STYPE, Payload_Default>);
}

// ---------------------------------------------------------------------------
// Instance(0): default-construct on first call, reuse afterwards

TEST_CASE("GSingletonT::Instance(0) returns a non-null pointer to a default-constructed instance",
          "[common][singleton]") {
    auto sp = GSingletonT<Payload_Instance>::Instance(0);
    REQUIRE(sp);
    CHECK(sp.use_count() >= 1);

    auto sp2 = GSingletonT<Payload_Instance>::Instance(0);
    REQUIRE(sp2);
    CHECK(sp.get() == sp2.get());      // same object across calls
    CHECK(sp == sp2);
}

// ---------------------------------------------------------------------------
// Instance(1): reset path

TEST_CASE("GSingletonT::Instance(1) resets the stored pointer; (0) afterwards builds a fresh one",
          "[common][singleton]") {
    auto first = GSingletonT<Payload_Reset>::Instance(0);
    REQUIRE(first);

    auto reset_result = GSingletonT<Payload_Reset>::Instance(1);
    CHECK_FALSE(reset_result);         // reset returns an empty shared_ptr

    // External users still keep the old object alive — singleton storage is
    // released, but `first` keeps its referent valid.
    CHECK(first);

    auto fresh = GSingletonT<Payload_Reset>::Instance(0);
    REQUIRE(fresh);
    CHECK(fresh.get() != first.get()); // brand-new object after the reset
}

// ---------------------------------------------------------------------------
// Default branch in the switch: any mode other than 0 / 1 yields an empty
// shared_ptr without touching the stored singleton.

TEST_CASE("GSingletonT::Instance(>=2) returns an empty pointer and does not initialise",
          "[common][singleton]") {
    auto sp = GSingletonT<Payload_DefaultMode>::Instance(2);
    CHECK_FALSE(sp);

    auto sp3 = GSingletonT<Payload_DefaultMode>::Instance(17);
    CHECK_FALSE(sp3);

    // First real Instance(0) call must still produce a value.
    auto good = GSingletonT<Payload_DefaultMode>::Instance(0);
    REQUIRE(good);
}

// ---------------------------------------------------------------------------
// Customisation point: TFactory_GSingletonT<T> specialisation

TEST_CASE("GSingletonT: honours TFactory_GSingletonT<T> specialisation",
          "[common][singleton]") {
    auto sp = GSingletonT<Payload_FactorySpecialised>::Instance(0);
    REQUIRE(sp);
    CHECK(sp->value == 7);   // produced by the user-supplied factory above
}

// ---------------------------------------------------------------------------
// Concurrency: a horde of threads racing on the first Instance(0) call must
// see exactly one constructor invocation, and all returned pointers must
// alias the same object.

TEST_CASE("GSingletonT::Instance(0) constructs exactly once under concurrent first access",
          "[common][singleton][concurrency]") {
    // Reset any residual state from previous runs of this TU — the singleton
    // storage is per-type and survives between TEST_CASEs.
    GSingletonT<Payload_Concurrent>::Instance(1);
    Payload_Concurrent::ctor_count.store(0);

    // Workers must not invoke Catch CHECK/REQUIRE — Catch's output-redirect
    // assertion is not thread-safe across simultaneous test threads. Collect
    // pointers from each worker and validate on the main thread.
    constexpr int kThreads = 16;
    std::vector<std::shared_ptr<Payload_Concurrent>> results(kThreads);
    std::vector<std::thread> ts;
    ts.reserve(kThreads);

    // A start gate increases the chance that several threads hit the DCLP
    // window simultaneously, exercising the lock path rather than the cheap
    // post-init load path.
    std::atomic<bool> go{false};
    for(int i = 0; i < kThreads; ++i) {
        ts.emplace_back([i, &results, &go] {
            while(not go.load(std::memory_order_acquire)) {
                std::this_thread::yield();
            }
            results[i] = GSingletonT<Payload_Concurrent>::Instance(0);
        });
    }
    go.store(true, std::memory_order_release);
    for(auto &t : ts) {
        t.join();
    }

    CHECK(Payload_Concurrent::ctor_count.load() == 1);

    REQUIRE(results[0]);
    for(int i = 1; i < kThreads; ++i) {
        REQUIRE(results[i]);
        CHECK(results[i].get() == results[0].get());
    }
}
