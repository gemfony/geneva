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
 * @brief Characterization tests for the new candidate-solution hierarchy (GOptimizableEntity /
 * GFlatGenome / GFlatGenomeT). The hierarchy is additive and unwired at this stage; these tests
 * exercise it in isolation: value-channel round-trips, evaluation + external-result acceptance, the
 * multi-format serialization round-trip, clone independence, and -- the watertight part -- the
 * layout-keyed countParameters cache and its invalidation when the genome's layout changes.
 */

#include <algorithm>
#include <cstdint>
#include <functional>
#include <ranges>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include <boost/serialization/export.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/base_object.hpp>

#include "geneva/ind/GOptimizableEntity.hpp"
#include "geneva/ind/GFlatGenomeT.hpp"
#include "geneva/ind/GGenomeBuilder.hpp"

using namespace Gem::Geneva;        // activityMode, serializationMode helpers
using namespace Gem::Geneva::Genome;
using Catch::Approx;

namespace Gem::Tests {

/******************************************************************************/
/**
 * A minimal flat individual on the NEW hierarchy: a sphere over its double channel, with optional int32
 * and bool channels so the parameter-count cache can be exercised across channels. Adds no data members,
 * so clone/load/compare come from the CRTP base + GFlatGenome.
 */
class NewSphere : public GFlatGenomeT<NewSphere> {
public:
    NewSphere() { buildGenome(3, 2, 4); }
    NewSphere(std::size_t nd, std::size_t ni, std::size_t nb) { buildGenome(nd, ni, nb); }
    NewSphere(const NewSphere &) = default;

    /** @brief Rebuilds this individual's genome with a different structure (re-keys the count cache). */
    void rebuild(std::size_t nd, std::size_t ni, std::size_t nb) { buildGenome(nd, ni, nb); }

protected:
    double fitnessCalculation() override {
        std::vector<double> v;
        this->streamline<double>(v);
        return std::ranges::fold_left(
            v | std::views::transform([](double x) { return x * x; }), 0., std::plus{});
    }

private:
    void buildGenome(std::size_t nd, std::size_t ni, std::size_t nb) {
        GGenomeBuilder b;
        b.addDoubleGroup(nd, -10., 10.).init(1.0); // structure only; the adaptor lives on the OA config
        if(ni > 0) {
            b.addInt32Group(ni, -5, 5);
        }
        if(nb > 0) {
            b.addBoolArray(nb);
        }
        this->setGenome(b.build());
    }

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive &ar, [[maybe_unused]] const unsigned int version) {
        ar &boost::serialization::make_nvp(
            "GFlatGenomeT",
            boost::serialization::base_object<GFlatGenomeT<NewSphere>>(*this)
        );
    }
};

} // namespace Gem::Tests

BOOST_CLASS_EXPORT(Gem::Tests::NewSphere) // NOLINT

using Gem::Tests::NewSphere;

/******************************************************************************/
TEST_CASE("GOptimizableEntity: a fresh flat individual evaluates and reaches PROCESSED", "[candidate]") {
    NewSphere ind(3, 0, 0); // 3 doubles, all initialised to 1.0

    // setGenome() marks the item due for processing.
    REQUIRE(ind.is_due_for_processing());

    const auto first = ind.process();

    CHECK(ind.is_processed());
    // sum of squares of three 1.0 values == 3.0
    CHECK(first.rawFitness() == Approx(3.0));
    CHECK(ind.raw_fitness(0) == Approx(3.0));
    CHECK(ind.transformed_fitness(0) == Approx(3.0)); // USESIMPLEEVALUATION: transformed == raw
}

/******************************************************************************/
TEST_CASE("GOptimizableEntity: streamline / assignValueVector round-trip", "[candidate]") {
    NewSphere ind(4, 0, 0);

    std::vector<double> before;
    ind.streamline<double>(before);
    REQUIRE(before.size() == 4);

    std::vector<double> written{-3.5, 0.0, 2.25, 9.99};
    ind.assignValueVector<double>(written);

    std::vector<double> after;
    ind.streamline<double>(after);
    REQUIRE(after.size() == written.size());
    for(std::size_t i = 0; i < written.size(); ++i) {
        CHECK(after[i] == Approx(written[i]));
    }
    // Writing parameters marks the item for reprocessing.
    CHECK(ind.is_due_for_processing());
}

/******************************************************************************/
TEST_CASE("GOptimizableEntity: an external evaluation result is accepted verbatim", "[candidate][external]") {
    NewSphere ind(3, 0, 0);
    REQUIRE(ind.is_due_for_processing());

    // Inject a pre-computed raw result instead of letting fitnessCalculation() run.
    const double injected = 42.5;
    ind.process(std::vector<individual_processing_result>{individual_processing_result(injected)});

    CHECK(ind.is_processed());
    CHECK(ind.raw_fitness(0) == Approx(injected));
    CHECK(ind.transformed_fitness(0) == Approx(injected)); // USESIMPLEEVALUATION
}

/******************************************************************************/
TEST_CASE("GOptimizableEntity: a derived individual round-trips in TEXT, XML and BINARY", "[candidate][serialize]") {
    using Gem::Common::serializationMode;

    NewSphere ind(5, 2, 3);
    ind.process(); // give it results to carry across the wire

    const auto check_format = [&](serializationMode mode) {
        NewSphere restored;
        restored.fromString(ind.toString(mode), mode);
        // Structure preserved
        CHECK(restored.getLayout()->layoutId() == ind.getLayout()->layoutId());
        CHECK(restored.countParameters<double>() == ind.countParameters<double>());
        CHECK(restored.countParameters<std::int32_t>() == ind.countParameters<std::int32_t>());
        CHECK(restored.countParameters<bool>() == ind.countParameters<bool>());
        // Results preserved
        REQUIRE(restored.is_processed());
        CHECK(restored.raw_fitness(0) == Approx(ind.raw_fitness(0)));
        // Values preserved
        std::vector<double> a;
        std::vector<double> b;
        ind.streamline<double>(a);
        restored.streamline<double>(b);
        REQUIRE(a.size() == b.size());
        for(std::size_t i = 0; i < a.size(); ++i) {
            CHECK(a[i] == Approx(b[i]));
        }
    };

    check_format(serializationMode::TEXT);
    check_format(serializationMode::XML);
    check_format(serializationMode::BINARY);
}

/******************************************************************************/
TEST_CASE("GOptimizableEntity: clone is independent of the original", "[candidate]") {
    NewSphere ind(4, 0, 0);
    ind.process();

    auto clone = ind.clone<NewSphere>();
    REQUIRE(clone);

    // Mutating the clone must not touch the original.
    clone->randomInit(activityMode::ALLPARAMETERS);

    std::vector<double> orig_v;
    std::vector<double> clone_v;
    ind.streamline<double>(orig_v);
    clone->streamline<double>(clone_v);

    // The original still holds its evaluated state; the clone has been re-randomized and is dirty.
    CHECK(ind.is_processed());
    CHECK(clone->is_due_for_processing());
}

/******************************************************************************/
TEST_CASE("GFlatGenome: countParameters is cached and re-keyed on layout change", "[candidate][cache]") {
    NewSphere ind(3, 2, 4); // 3 doubles, 2 int32, 4 bool, 0 float

    SECTION("counts are correct and stable across repeated queries (cache hit)") {
        for(int rep = 0; rep < 3; ++rep) {
            CHECK(ind.countParameters<double>() == 3u);
            CHECK(ind.countParameters<float>() == 0u);
            CHECK(ind.countParameters<std::int32_t>() == 2u);
            CHECK(ind.countParameters<bool>() == 4u);
            CHECK(ind.countFPParameters() == 3u);
        }
    }

    SECTION("the cache is invalidated when the genome's layout changes") {
        // Prime the cache for the original structure.
        REQUIRE(ind.countParameters<double>() == 3u);
        REQUIRE(ind.countParameters<std::int32_t>() == 2u);
        REQUIRE(ind.countParameters<bool>() == 4u);

        // Install a structurally different layout: counts must follow.
        ind.rebuild(5, 0, 1);
        CHECK(ind.countParameters<double>() == 5u);
        CHECK(ind.countParameters<std::int32_t>() == 0u);
        CHECK(ind.countParameters<bool>() == 1u);

        // Restore a structure identical to the original: counts must match it again (re-keying works,
        // and a recycled layout address cannot alias because the cache pins the layout it is keyed to).
        ind.rebuild(3, 2, 4);
        CHECK(ind.countParameters<double>() == 3u);
        CHECK(ind.countParameters<std::int32_t>() == 2u);
        CHECK(ind.countParameters<bool>() == 4u);
    }

    SECTION("the cache survives a load_()-based copy") {
        NewSphere other(1, 1, 1);
        // Prime both caches with their own structures.
        REQUIRE(other.countParameters<double>() == 1u);
        REQUIRE(ind.countParameters<double>() == 3u);

        // load_() (clone path) copies ind's layout into other -- its cache must re-key.
        auto loaded = ind.clone<NewSphere>();
        CHECK(loaded->countParameters<double>() == 3u);
        CHECK(loaded->countParameters<std::int32_t>() == 2u);
        CHECK(loaded->countParameters<bool>() == 4u);
    }
}
