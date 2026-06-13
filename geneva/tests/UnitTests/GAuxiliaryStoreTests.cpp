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

#include <cstdint>
#include <memory>

#include "common/GExpectationChecksT.hpp"
#include "geneva/ind/GAuxiliaryStore.hpp"
#include "geneva/individuals/GTestIndividual3.hpp"

using namespace Gem::Geneva::Parameters;

namespace {

/** @brief A representative per-group EA-Gauss metadata record (POD). */
struct GaussState {
    float sigma = 0.f;
    float adProb = 0.f;
    std::uint32_t counter = 0;
};

constexpr AuxKey KEY = 0x1234u;

} // namespace

/******************************************************************************/
TEST_CASE("GAuxiliaryStore POD blocks: install / typed access / copy-independence / clear", "[aux]") {
    GAuxiliaryStore s;

    CHECK_FALSE(s.hasAux(KEY));
    s.installAuxBlock<GaussState>(KEY, 3);
    REQUIRE(s.hasAux(KEY));

    auto recs = s.metaRecords<GaussState>(KEY);
    REQUIRE(recs.size() == 3);
    CHECK(recs[0].sigma == 0.f); // zero-initialised
    CHECK(recs[0].counter == 0u);

    recs[1].sigma = 0.7f;
    recs[1].counter = 5;
    CHECK(s.metaRecords<GaussState>(KEY)[1].sigma == 0.7f);
    CHECK(s.metaRecords<GaussState>(KEY)[1].counter == 5u);

    // A copy is independent (deep byte copy)
    GAuxiliaryStore t(s);
    REQUIRE(t.hasAux(KEY));
    CHECK(t.metaRecords<GaussState>(KEY)[1].sigma == 0.7f);
    t.metaRecords<GaussState>(KEY)[1].sigma = 0.1f;
    CHECK(s.metaRecords<GaussState>(KEY)[1].sigma == 0.7f); // s unaffected

    // clearScratch drops the blocks
    s.clearScratch();
    CHECK_FALSE(s.hasAux(KEY));
}

/******************************************************************************/
TEST_CASE("GOptimizableEntity POD metadata: clone copies scratch; compare ignores it; clearOAScratch drops it", "[aux]") {
    using Gem::Geneva::Individuals::GTestIndividual3;

    GTestIndividual3 ind;

    // Clone BEFORE installing scratch -> an identical-genome twin with no scratch.
    auto bare = ind.clone<GTestIndividual3>();
    REQUIRE_FALSE(bare->hasAux(KEY));

    // Install per-parameter metadata on ind only.
    ind.installAuxBlock<GaussState>(KEY, 2);
    ind.metaRecords<GaussState>(KEY)[0].sigma = 1.5f;
    REQUIRE(ind.hasAux(KEY));

    // A clone taken AFTER installing copies the scratch, independently.
    auto twin = ind.clone<GTestIndividual3>();
    REQUIRE(twin->hasAux(KEY));
    CHECK(twin->metaRecords<GaussState>(KEY)[0].sigma == 1.5f);
    twin->metaRecords<GaussState>(KEY)[0].sigma = 9.f;
    CHECK(ind.metaRecords<GaussState>(KEY)[0].sigma == 1.5f); // independent

    // compare() ignores the POD scratch: ind (with scratch) equals bare (without).
    CHECK_NOTHROW(ind.compare(
        *bare,
        Gem::Common::expectation::EQUALITY,
        Gem::Common::CE_DEF_SIMILARITY_DIFFERENCE
    ));

    // clearOAScratch drops the POD scratch.
    ind.clearOAScratch();
    CHECK_FALSE(ind.hasAux(KEY));
}
