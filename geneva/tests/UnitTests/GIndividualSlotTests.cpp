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
#include <sstream>
#include <vector>

#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/serialization/nvp.hpp>

#include "common/GCommonEnums.hpp"
#include "common/GExpectationChecksT.hpp"
#include "geneva/ind/GFlatGenome.hpp"
#include "geneva/ind/GGenomeBuilder.hpp"
#include "geneva/ind/GIndividualSlot.hpp"
#include "geneva/ind/GFlatGenomeT.hpp"
#include "geneva/oa/GEvolutionaryAlgorithm_PersonalityTraits.hpp"

using namespace Gem::Geneva;
using namespace Gem::Geneva::Genome;

namespace Gem::Tests {

/******************************************************************************/
/**
 * A minimal flat individual used as the slot's payload: a sphere over n constrained doubles sharing
 * one Gauss adaptor.
 */
class SlotSphere : public Gem::Geneva::Genome::GFlatGenomeT<SlotSphere> {
public:
    SlotSphere() { buildGenome(5); }
    explicit SlotSphere(std::size_t n) { buildGenome(n); }
    SlotSphere(const SlotSphere &) = default;

protected:
    double fitnessCalculation() override {
        std::vector<double> v;
        this->streamline<double>(v);
        double sum = 0.;
        for(double x : v) {
            sum += x * x;
        }
        return sum;
    }

private:
    void buildGenome(std::size_t n) {
        GGenomeBuilder b;
        b.addDoubleGroup(n, -10., 10.).init(1.0); // structure only; slot tests drive scratch directly
        this->setGenome(b.build());
    }

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive &ar, [[maybe_unused]] const unsigned int version) {
        ar &boost::serialization::make_nvp(
            "GFlatGenomeT",
            boost::serialization::base_object<GFlatGenomeT<SlotSphere>>(*this)
        );
    }
};

std::unique_ptr<Gem::Geneva::Genome::GOptimizableEntity> makeIndividual(std::size_t n = 5) {
    return std::make_unique<SlotSphere>(n);
}

} // namespace Gem::Tests

BOOST_CLASS_EXPORT(Gem::Tests::SlotSphere) // NOLINT

using Gem::Tests::makeIndividual;

/******************************************************************************/
TEST_CASE("GIndividualSlot: construction, access and emptiness", "[slot]") {
    GIndividualSlot empty;
    CHECK_FALSE(empty.hasIndividual());

    GIndividualSlot slot(makeIndividual(4));
    REQUIRE(slot.hasIndividual());
    CHECK(slot.individual().countParameters<double>() == 4);

    // The scratch starts empty (no personality, no POD blocks).
    CHECK_FALSE(static_cast<bool>(slot.scratch().personalityRef()));
}

/******************************************************************************/
TEST_CASE("GIndividualSlot: deep clone is independent of the source", "[slot]") {
    GIndividualSlot slot(makeIndividual(3));
    slot.scratch().personalityRef() =
        std::make_shared<OptimizationAlgorithms::GEvolutionaryAlgorithm_PersonalityTraits>();

    auto cloned = slot.clone<GIndividualSlot>();
    REQUIRE(cloned->hasIndividual());

    // The cloned individual is a distinct object ...
    CHECK(&cloned->individual() != &slot.individual());
    // ... and the cloned scratch personality is a distinct object too (deep clone).
    REQUIRE(static_cast<bool>(cloned->scratch().personalityRef()));
    CHECK(cloned->scratch().personalityRef().get() != slot.scratch().personalityRef().get());

    // Mutating the clone's individual values does not affect the source.
    std::vector<double> v(3, 7.0);
    cloned->individual().assignValueVector<double>(v);
    std::vector<double> orig;
    slot.individual().streamline<double>(orig);
    CHECK(orig[0] != 7.0);
}

/******************************************************************************/
TEST_CASE("GIndividualSlot: compare ignores scratch (OA-agnostic identity)", "[slot]") {
    GIndividualSlot a(makeIndividual(5));
    auto b = a.clone<GIndividualSlot>();

    using namespace Gem::Common;
    // Right after a clone the two slots are equal.
    CHECK_NOTHROW(a.compare(*b, expectation::EQUALITY, Gem::Common::CE_DEF_SIMILARITY_DIFFERENCE));

    // Installing a personality on ONLY one of them must NOT make them compare unequal: the scratch is
    // deliberately outside the compared identity.
    a.scratch().personalityRef() =
        std::make_shared<OptimizationAlgorithms::GEvolutionaryAlgorithm_PersonalityTraits>();
    CHECK_NOTHROW(a.compare(*b, expectation::EQUALITY, Gem::Common::CE_DEF_SIMILARITY_DIFFERENCE));

    // But a genome difference IS detected.
    std::vector<double> v(5, 3.0);
    b->individual().assignValueVector<double>(v);
    CHECK_THROWS(a.compare(*b, expectation::EQUALITY, Gem::Common::CE_DEF_SIMILARITY_DIFFERENCE));
}

/******************************************************************************/
TEST_CASE("GIndividualSlot: serialization round-trip keeps individual and personality", "[slot]") {
    GIndividualSlot slot(makeIndividual(6));
    std::vector<double> v(6, 2.5);
    slot.individual().assignValueVector<double>(v);
    slot.scratch().personalityRef() =
        std::make_shared<OptimizationAlgorithms::GEvolutionaryAlgorithm_PersonalityTraits>();

    std::stringstream ss;
    {
        boost::archive::xml_oarchive oa(ss);
        oa << boost::serialization::make_nvp("slot", slot);
    }

    GIndividualSlot restored;
    {
        boost::archive::xml_iarchive ia(ss);
        ia >> boost::serialization::make_nvp("slot", restored);
    }

    REQUIRE(restored.hasIndividual());
    std::vector<double> rv;
    restored.individual().streamline<double>(rv);
    REQUIRE(rv.size() == 6);
    CHECK(rv[0] == 2.5);

    // The personality rode along (full / checkpoint serialization).
    CHECK(static_cast<bool>(restored.scratch().personalityRef()));
}

/******************************************************************************/
TEST_CASE("GIndividualSlot: serialization round-trip keeps the POD scratch blocks", "[slot]") {
    // A POD record mirroring the per-group adaption state the OA stashes on a slot's scratch.
    struct StateRec {
        double sigma = 0.;
        double ad_prob = 0.;
        std::uint32_t counter = 0;
    };
    constexpr AuxKey KEY = 0x5151u;

    GIndividualSlot slot(makeIndividual(4));
    slot.scratch().installAuxBlock<StateRec>(KEY, 3, AuxScope::PerIndividual);
    auto recs = slot.scratch().metaRecords<StateRec>(KEY);
    recs[0] = StateRec{0.42, 0.9, 7};
    recs[1] = StateRec{1.25, 0.5, 0};
    recs[2] = StateRec{0.01, 0.1, 99};

    std::stringstream ss;
    {
        boost::archive::xml_oarchive oa(ss);
        oa << boost::serialization::make_nvp("slot", slot);
    }

    GIndividualSlot restored;
    {
        boost::archive::xml_iarchive ia(ss);
        ia >> boost::serialization::make_nvp("slot", restored);
    }

    // The POD scratch block survived the round-trip with its values intact (checkpoint fidelity), and
    // is still typed-readable (the deserialised block's tag is "unchecked", but the stride matches).
    REQUIRE(restored.scratch().hasAux(KEY));
    auto rr = restored.scratch().metaRecords<StateRec>(KEY);
    REQUIRE(rr.size() == 3);
    CHECK(rr[0].sigma == 0.42);
    CHECK(rr[0].counter == 7u);
    CHECK(rr[1].sigma == 1.25);
    CHECK(rr[2].counter == 99u);
}

/******************************************************************************/
TEST_CASE("GIndividualSlot: swap individual out and back for submission", "[slot]") {
    GIndividualSlot slot(makeIndividual(5));
    slot.scratch().personalityRef() =
        std::make_shared<OptimizationAlgorithms::GEvolutionaryAlgorithm_PersonalityTraits>();
    GOptimizableEntity *raw = &slot.individual();

    // Move the individual out into a "submission span", leaving the scratch behind on the slot.
    std::vector<std::unique_ptr<GOptimizableEntity>> span;
    span.push_back(slot.releaseIndividual());
    CHECK_FALSE(slot.hasIndividual());
    // The scratch is untouched by the swap-out.
    CHECK(static_cast<bool>(slot.scratch().personalityRef()));

    // Put it back after "workOn".
    slot.resetIndividual(std::move(span.front()));
    REQUIRE(slot.hasIndividual());
    CHECK(&slot.individual() == raw); // the same object returned to the same slot
}
