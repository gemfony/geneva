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

#include <any>
#include <cstdint>
#include <memory>
#include <vector>

#include <boost/serialization/base_object.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/nvp.hpp>

#include "common/GCommonEnums.hpp"
#include "common/GExceptions.hpp"
#include "common/GExpectationChecksT.hpp"
#include "geneva/GOptimizationEnums.hpp"
#include "geneva/ind/GAdaptionLayout.hpp"
#include "geneva/ind/GFlatGenome.hpp"
#include "geneva/ind/GFlatIndividualT.hpp"
#include "geneva/ind/GGenomeArchitecture.hpp"
#include "geneva/ind/GGenomeBuilder.hpp"
#include "geneva/individuals/GTestIndividual1.hpp"

using namespace Gem::Geneva;
using namespace Gem::Geneva::Parameters;

namespace Gem::Tests {

/******************************************************************************/
/**
 * A minimal flat individual: a sphere over n constrained doubles sharing one Gauss adaptor. It
 * demonstrates the intended authoring effort -- a constructor that builds the genome, a
 * fitnessCalculation(), and the serialize hook; clone/load/compare come from the CRTP base +
 * GFlatGenome.
 */
class FlatSphere : public GFlatIndividualT<FlatSphere> {
public:
    FlatSphere() { buildGenome(5); }
    explicit FlatSphere(std::size_t n) { buildGenome(n); }
    FlatSphere(const FlatSphere &) = default;

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
        b.addDoubleGroup(n, -10., 10.).gaussAdaptor(0.5, 0.8, 1e-3, 2., 1.).init(1.0);
        this->setGenome(b.build());
    }

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        ar &boost::serialization::make_nvp(
            "GFlatIndividualT",
            boost::serialization::base_object<GFlatIndividualT<FlatSphere>>(*this)
        );
    }
};

} // namespace Gem::Tests

BOOST_CLASS_EXPORT(Gem::Tests::FlatSphere) // NOLINT

using Gem::Tests::FlatSphere;

/******************************************************************************/
TEST_CASE("GGenomeBuilder produces the expected shared layout", "[flat]") {
    GGenomeBuilder b;
    b.addDoubleGroup(5, -10., 10.).gaussAdaptor(0.5, 0.8, 1e-3, 2., 1.).init(1.0);
    Genome g = b.build();

    REQUIRE(g.layout);
    const ChannelLayout<double> &ch = g.layout->d;
    REQUIRE(ch.size() == 5);
    REQUIRE(ch.groups.size() == 1);

    const GroupSpec<double> &grp = ch.groups[0];
    CHECK(grp.start == 0u);
    CHECK(grp.len == 5u);
    CHECK(grp.active);
    CHECK(grp.has_gauss);
    CHECK(grp.start_sigma == 0.5);
    CHECK(grp.gauss.sigma_sigma == 0.8);
    CHECK(grp.gauss.min_sigma == 1e-3);
    CHECK(grp.gauss.max_sigma == 2.);
    CHECK(grp.range == 20.); // upper - lower

    for(std::size_t k = 0; k < 5; ++k) {
        CHECK(ch.lower[k] == -10.);
        CHECK(ch.upper[k] == 10.);
        CHECK(ch.kind[k] == ParamKind::Constrained);
        CHECK(g.dv[k] == 1.0);
    }
}

/******************************************************************************/
TEST_CASE("GGenomeBuilder: groups vs arrays vs single parameters", "[flat]") {
    GGenomeBuilder b;
    b.addDouble(0., -1., 1.);          // group of 1
    b.addDoubleGroup(4, -2., 2.);      // ONE group of 4 (shared sigma)
    b.addDoubleArray(3, -3., 3.);      // 3 groups of 1
    Genome g = b.build();

    const ChannelLayout<double> &ch = g.layout->d;
    CHECK(ch.size() == 1 + 4 + 3);
    CHECK(ch.groups.size() == 1 + 1 + 3); // single + grouped + array
}

/******************************************************************************/
TEST_CASE("GFlatGenome: value round-trip (streamline / assignValueVector)", "[flat]") {
    FlatSphere ind(4);

    std::vector<double> v;
    ind.streamline<double>(v);
    REQUIRE(v.size() == 4);
    CHECK(ind.countParameters<double>() == 4);

    const std::vector<double> assigned{2., 3., -4., 5.};
    ind.assignValueVector<double>(assigned);
    ind.streamline<double>(v);
    CHECK(v == assigned); // in-range constrained values round-trip identically
}

/******************************************************************************/
TEST_CASE("Constrained fold helpers map into range", "[flat]") {
    // FP: half-open [lo, hi)
    CHECK(foldConstrainedFP<double>(5., -10., 10.) == 5.);   // in range -> identity
    CHECK(foldConstrainedFP<double>(25., -10., 10.) == -5.); // reflected
    for(double x : {-37.3, -11., 9.999, 100.25, 10.0}) {
        const double f = foldConstrainedFP<double>(x, -10., 10.);
        CHECK(f >= -10.);
        CHECK(f < 10.);
    }

    // Int: closed [lo, hi]
    CHECK(foldConstrainedInt<std::int32_t>(7, -10, 10) == 7);
    for(std::int32_t x : {-100, -11, 11, 250}) {
        const std::int32_t f = foldConstrainedInt<std::int32_t>(x, -10, 10);
        CHECK(f >= -10);
        CHECK(f <= 10);
    }
}

/******************************************************************************/
TEST_CASE("GFlatGenome: clone is independent", "[flat]") {
    FlatSphere ind(6);

    auto twin = ind.clone<FlatSphere>();

    // Equal right after cloning.
    CHECK_NOTHROW(twin->compare(
        ind,
        Gem::Common::expectation::EQUALITY,
        Gem::Common::CE_DEF_SIMILARITY_DIFFERENCE
    ));

    // Mutating the clone leaves the original untouched.
    twin->randomInit(activityMode::ALLPARAMETERS);

    std::vector<double> orig;
    std::vector<double> mutated;
    ind.streamline<double>(orig);
    twin->streamline<double>(mutated);
    CHECK(orig != mutated);

    bool equal = true;
    try {
        twin->compare(
            ind,
            Gem::Common::expectation::EQUALITY,
            Gem::Common::CE_DEF_SIMILARITY_DIFFERENCE
        );
    }
    catch(const g_expectation_violation &) {
        equal = false;
    }
    CHECK_FALSE(equal);
}

/******************************************************************************/
TEST_CASE("GFlatGenome: serialization round-trip", "[flat]") {
    FlatSphere ind(5);
    ind.assignValueVector<double>(std::vector<double>{1., -2., 3., -4., 5.});

    const std::string xml = ind.toString(Gem::Common::serializationMode::XML);

    FlatSphere restored;
    restored.fromString(xml, Gem::Common::serializationMode::XML);

    CHECK_NOTHROW(restored.compare(
        ind,
        Gem::Common::expectation::EQUALITY,
        Gem::Common::CE_DEF_SIMILARITY_DIFFERENCE
    ));

    std::vector<double> v;
    restored.streamline<double>(v);
    CHECK(v == std::vector<double>{1., -2., 3., -4., 5.});
}

/******************************************************************************/
TEST_CASE("GFlatGenome: adapt() mutates within bounds", "[flat]") {
    FlatSphere ind(8);

    std::size_t total = 0;
    for(int round = 0; round < 20; ++round) {
        total += ind.adapt();

        std::vector<double> v;
        ind.streamline<double>(v);
        REQUIRE(v.size() == 8);
        for(double x : v) {
            CHECK(x >= -10.); // streamline always folds back into the external range
            CHECK(x < 10.);
        }
    }
    CHECK(total > 0); // ad_prob == 1 -> every value is adapted every round
}

/******************************************************************************/
TEST_CASE("GFlatGenome: randomInit stays within bounds and changes values", "[flat]") {
    FlatSphere ind(10);

    std::vector<double> before;
    ind.streamline<double>(before);

    CHECK(ind.randomInit(activityMode::ALLPARAMETERS));

    std::vector<double> after;
    ind.streamline<double>(after);
    CHECK(before != after);
    for(double x : after) {
        CHECK(x >= -10.);
        CHECK(x < 10.);
    }
}

/******************************************************************************/
TEST_CASE("GFlatGenome: updateAdaptorsOnStall resets sigma to its seed", "[flat]") {
    FlatSphere ind(3);

    // Drive the sigma self-adaption (adaption_threshold defaults to 1 -> sigma adapts every step).
    for(int i = 0; i < 30; ++i) {
        ind.adapt();
    }

    auto sigmaNow = [&]() {
        std::vector<std::any> data;
        ind.queryAdaptor("GDoubleGaussAdaptor", "sigma", data);
        REQUIRE(data.size() == 1);
        return std::any_cast<double>(data[0]);
    };

    const double drifted = sigmaNow();
    CHECK(drifted != 0.5); // sigma has evolved away from its seed

    ind.updateAdaptorsOnStall(3);
    CHECK(sigmaNow() == 0.5); // reset to the configured seed
}

/******************************************************************************/
TEST_CASE("GGridArchitecture reads any genome through the §2 seam (flat)", "[flat][architecture]") {
    FlatSphere ind(12); // 12 FP values -> a 3x4 grid
    GGridArchitecture grid(3, 4);

    CHECK(grid.name() == "GGridArchitecture");
    CHECK(grid.expectedFPSize() == 12);

    std::vector<double> flat;
    ind.streamlineFP(flat);
    REQUIRE(flat.size() == 12);

    for(std::size_t r = 0; r < grid.rows(); ++r) {
        const std::vector<double> row = grid.row(ind, r);
        REQUIRE(row.size() == grid.cols());
        for(std::size_t c = 0; c < grid.cols(); ++c) {
            CHECK(grid.at(ind, r, c) == flat[r * grid.cols() + c]);
            CHECK(row[c] == flat[r * grid.cols() + c]);
        }
    }
}

/******************************************************************************/
TEST_CASE("GGridArchitecture reads a TREE genome identically (layout-agnostic)", "[architecture]") {
    // The same semantic architecture, applied to a TREE individual, proves it never learns the
    // concrete genome implementation -- it only uses the genome-agnostic streamlineFP() seam.
    Gem::Geneva::Individuals::GTestIndividual1 tree_ind;

    std::vector<double> flat;
    tree_ind.streamlineFP(flat);
    REQUIRE(flat.size() >= 100);

    GGridArchitecture grid(10, 10); // a 10x10 view over the first 100 FP values
    for(std::size_t r = 0; r < grid.rows(); ++r) {
        for(std::size_t c = 0; c < grid.cols(); ++c) {
            CHECK(grid.at(tree_ind, r, c) == flat[r * grid.cols() + c]);
        }
    }
}
