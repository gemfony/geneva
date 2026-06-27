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
 * @file
 * A small, runnable tour of the Gem::Common::Concurrency facilities -- living documentation of the
 * intended call patterns, and an extra compile+exercise path. Each shared primitive gets a short section;
 * the program prints what it does and returns non-zero if any demonstrated invariant does not hold.
 */

#include <cstdint>
#include <iostream>
#include <string>

#include "common/concurrency/GContentAddressedStoreT.hpp"

using namespace Gem::Common::Concurrency;

namespace {

/** @brief Demonstrates GContentAddressedStoreT: a content-addressed blob cache with LRU eviction and
 *  per-peer "who already holds what" tracking (the heart of the wire layout send-once machinery).
 *  @return true iff every demonstrated invariant held. */
bool demo_content_addressed_store() {
    std::cout << "== GContentAddressedStoreT ==\n";
    bool ok = true;

    // id -> opaque blob. Here ids are ints and blobs are strings.
    GContentAddressedStoreT<int, std::string, int> store;

    // Intern two blobs (recency, most- to least-recently-used: 2, 1). A peer is told it holds blob 1;
    // we resolve blob 2 by id. (We deliberately do NOT read blob 1 -- a read would refresh its recency.)
    store.put(1, "layout-A");
    store.put(2, "layout-B");
    store.markPeerHas(/*peer*/ 42, /*id*/ 1);
    std::string blob;
    ok = ok && store.tryGet(2, blob) && blob == "layout-B";
    ok = ok && store.peerHas(42, 1);
    std::cout << "  interned 2 blobs; peer 42 holds blob 1; tryGet(2) -> \"" << blob << "\"\n";

    // A capacity bound evicts the least-recently-used blob (id 1) -- and crucially clears that id from
    // every peer's ack set, so the source can never reference a blob it can no longer answer.
    store.setCapacity(1);
    ok = ok && !store.has(1) && !store.peerHas(42, 1) && store.has(2);
    std::cout << "  after setCapacity(1): has(1)=" << store.has(1)
              << ", peer 42 still holds blob 1? " << std::boolalpha << store.peerHas(42, 1)
              << " (LRU blob evicted, ack cleared)\n";

    std::cout << "  -> " << (ok ? "OK" : "FAILED") << "\n";
    return ok;
}

} // namespace

int main() {
    bool ok = true;
    ok = demo_content_addressed_store() && ok;

    std::cout << (ok ? "\nAll concurrency-primitive demos passed.\n"
                     : "\nA concurrency-primitive demo FAILED.\n");
    return ok ? 0 : 1;
}
