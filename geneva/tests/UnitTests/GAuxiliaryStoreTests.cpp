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
#include "geneva/ind/GOptimizableEntity.hpp"
#include "geneva/individuals/GTestIndividual3.hpp"

using namespace Gem::Geneva::Genome;

namespace {

/** @brief A representative per-group EA-Gauss metadata record (POD). */
struct AuxGaussRecord {
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
    s.installAuxBlock<AuxGaussRecord>(KEY, 3);
    REQUIRE(s.hasAux(KEY));

    auto recs = s.metaRecords<AuxGaussRecord>(KEY);
    REQUIRE(recs.size() == 3);
    CHECK(recs[0].sigma == 0.f); // zero-initialised
    CHECK(recs[0].counter == 0u);

    recs[1].sigma = 0.7f;
    recs[1].counter = 5;
    CHECK(s.metaRecords<AuxGaussRecord>(KEY)[1].sigma == 0.7f);
    CHECK(s.metaRecords<AuxGaussRecord>(KEY)[1].counter == 5u);

    // A copy is independent (deep byte copy)
    GAuxiliaryStore t(s);
    REQUIRE(t.hasAux(KEY));
    CHECK(t.metaRecords<AuxGaussRecord>(KEY)[1].sigma == 0.7f);
    t.metaRecords<AuxGaussRecord>(KEY)[1].sigma = 0.1f;
    CHECK(s.metaRecords<AuxGaussRecord>(KEY)[1].sigma == 0.7f); // s unaffected

    // clearScratch drops the blocks
    s.clearScratch();
    CHECK_FALSE(s.hasAux(KEY));
}

/******************************************************************************/
TEST_CASE("Individual scratch: clone copies it; compare ignores it; resetPersonality drops it", "[aux]") {
    using Gem::Geneva::Individuals::GTestIndividual3;

    // The per-group POD adaption scratch is OA-owned and rides on the individual itself (in its
    // GAuxiliaryStore). Its clone-copies / compare-ignores / boundary-clears contract is therefore a
    // property of the individual.
    auto ind = std::make_unique<GTestIndividual3>();

    // A clone taken BEFORE installing scratch -> an identical-genome twin with no scratch.
    auto bare = ind->clone_unique();
    REQUIRE_FALSE(bare->scratch().hasAux(KEY));

    // Install per-group metadata on the individual's scratch only.
    ind->scratch().installAuxBlock<AuxGaussRecord>(KEY, 2);
    ind->scratch().metaRecords<AuxGaussRecord>(KEY)[0].sigma = 1.5f;
    REQUIRE(ind->scratch().hasAux(KEY));

    // A clone taken AFTER installing copies the scratch, independently.
    auto twin = ind->clone_unique();
    REQUIRE(twin->scratch().hasAux(KEY));
    CHECK(twin->scratch().metaRecords<AuxGaussRecord>(KEY)[0].sigma == 1.5f);
    twin->scratch().metaRecords<AuxGaussRecord>(KEY)[0].sigma = 9.f;
    CHECK(ind->scratch().metaRecords<AuxGaussRecord>(KEY)[0].sigma == 1.5f); // independent

    // compare() ignores the scratch: ind (with scratch) equals bare (without).
    CHECK_NOTHROW(ind->compare(
        *bare,
        Gem::Common::expectation::EQUALITY,
        Gem::Common::CE_DEF_SIMILARITY_DIFFERENCE
    ));

    // resetPersonality() drops the whole OA-owned scratch (personality + POD blocks).
    ind->resetPersonality();
    CHECK_FALSE(ind->scratch().hasAux(KEY));
}
