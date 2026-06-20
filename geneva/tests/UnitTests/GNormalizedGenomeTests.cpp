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

/********************************************************************************
 *
 * Test harness for the normalized-genome architecture (see
 * prompts/2026-06-20-normalized-genome-architecture.md, §4). It is written FIRST
 * (Phase 1), pinning the behaviour the transition must preserve, then grown as the
 * model lands. The cases here pass against the CURRENT genome and must keep passing:
 *
 *   - the reflecting fold contract (same maths as GConstrainedFPT::transfer);
 *   - faithful value round-trip across magnitudes, incl. offset/narrow boxes
 *     (the cancellation case the normalized model is designed to fix);
 *   - bounded and unbounded parameters coexisting on one genome;
 *   - randomInit staying within a constrained parameter's range.
 *
 * Cases that only become testable once the model lands are added in their phase:
 *   - Phase 2: out-of-range EXTERNAL writes throw (incl. exactly the open upper);
 *              internal writes fold; internal store stays in [-0.5, 0.5).
 *   - Phase 3: a sigma of 0.1 produces an external step ~10% of the range,
 *              independent of the range.
 *   - Phase 5: drift of a neutral unbounded parameter stays within the documented
 *              bound; an unbounded value exceeds its init perimeter during a run.
 *
 ********************************************************************************/

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <memory>
#include <vector>

#include <boost/serialization/base_object.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/nvp.hpp>

#include "geneva/GOptimizationEnums.hpp"
#include "geneva/ind/GGenomeLayout.hpp"
#include "geneva/ind/GFlatGenome.hpp"
#include "geneva/ind/GFlatIndividualT.hpp"
#include "geneva/ind/GGenomeBuilder.hpp"
#include "geneva/oa/GAdaption.hpp"
#include "geneva/oa/GAdaptionConfig.hpp"

using namespace Gem::Geneva;
using namespace Gem::Geneva::Genome;
namespace oa = Gem::Geneva::OptimizationAlgorithms;

namespace Gem::Tests {

/******************************************************************************/
/** A flat individual: n constrained doubles in [lo, hi). Used to exercise the genome
 *  read/write/round-trip contract under offset / narrow boxes. */
class NgBoxIndividual : public GFlatIndividualT<NgBoxIndividual> {
public:
    NgBoxIndividual() { build(3, -10., 10.); }
    NgBoxIndividual(std::size_t n, double lo, double hi) { build(n, lo, hi); }
    NgBoxIndividual(const NgBoxIndividual &) = default;

    std::shared_ptr<oa::GAdaptionConfigBase> getAdaptionConfig() const {
        auto cfg = oa::makeAdaptionConfig<oa::GAdaptionConfigBase>(*this);
        for(std::size_t i = 0; i < cfg->doubleGroups().size(); i++) {
            cfg->groupDouble(i).gauss(0.1, 0.8, 1e-3, 0.5, 1.);
        }
        return cfg;
    }

protected:
    double fitnessCalculation() override {
        std::vector<double> v;
        this->streamline<double>(v);
        double s = 0.;
        for(double x : v) {
            s += x * x;
        }
        return s;
    }

private:
    void build(std::size_t n, double lo, double hi) {
        GGenomeBuilder b;
        b.addDoubleGroup(n, lo, hi);
        this->setGenome(b.build());
    }

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        ar &boost::serialization::make_nvp(
            "GFlatIndividualT",
            boost::serialization::base_object<GFlatIndividualT<NgBoxIndividual>>(*this)
        );
    }
};

/******************************************************************************/
/** A flat individual mixing a constrained group and an unbounded (plain) group on ONE
 *  genome -- exercising bounded/unbounded coexistence (§2.5). */
class NgMixedIndividual : public GFlatIndividualT<NgMixedIndividual> {
public:
    NgMixedIndividual() { build(); }
    NgMixedIndividual(const NgMixedIndividual &) = default;

    std::shared_ptr<oa::GAdaptionConfigBase> getAdaptionConfig() const {
        auto cfg = oa::makeAdaptionConfig<oa::GAdaptionConfigBase>(*this);
        for(std::size_t i = 0; i < cfg->doubleGroups().size(); i++) {
            cfg->groupDouble(i).gauss(0.1, 0.8, 1e-3, 0.5, 1.);
        }
        return cfg;
    }

protected:
    double fitnessCalculation() override {
        std::vector<double> v;
        this->streamline<double>(v);
        double s = 0.;
        for(double x : v) {
            s += x * x;
        }
        return s;
    }

private:
    void build() {
        GGenomeBuilder b;
        b.addDoubleGroup(2, -1., 1.);        // bounded
        b.addDoublePlainGroup(2, -10., 10.); // unbounded (the [min,max] is only the init perimeter)
        this->setGenome(b.build());
    }

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        ar &boost::serialization::make_nvp(
            "GFlatIndividualT",
            boost::serialization::base_object<GFlatIndividualT<NgMixedIndividual>>(*this)
        );
    }
};

} // namespace Gem::Tests

BOOST_CLASS_EXPORT(Gem::Tests::NgBoxIndividual)   // NOLINT
BOOST_CLASS_EXPORT(Gem::Tests::NgMixedIndividual) // NOLINT

using Gem::Tests::NgBoxIndividual;
using Gem::Tests::NgMixedIndividual;

namespace {

/** Faithful round-trip: equal to within a few ULP. Exact under today's identity store;
 *  correctly-rounded (not necessarily bit-identical) under the normalized long-double
 *  transfer to come (§2.1) -- so this assertion is durable across the transition. */
bool faithful(double a, double b) {
    const double tol = 8. * std::numeric_limits<double>::epsilon()
                       * std::max({1.0, std::abs(a), std::abs(b)});
    return std::abs(a - b) <= tol;
}

} // namespace

/******************************************************************************/
TEST_CASE(
    "normalized-genome: reflecting fold contract (provenance: GConstrainedFPT::transfer)",
    "[normalized-genome]"
) {
    // In-range -> identity.
    CHECK(foldConstrainedFP<double>(5., -10., 10.) == 5.);
    CHECK(foldConstrainedFP<double>(25., -10., 10.) == -5.); // reflected

    // Every out-of-range value folds into the half-open range and is idempotent
    // (external-value-preserving: folding an already-folded value is a no-op).
    for(double x : {-37.3, -11., 9.999, 100.25, 10.0, 25.0}) {
        const double f = foldConstrainedFP<double>(x, -10., 10.);
        CHECK(f >= -10.);
        CHECK(f < 10.);
        CHECK(foldConstrainedFP<double>(f, -10., 10.) == f);
    }

    // Closed integer range, also idempotent.
    for(std::int32_t x : {-100, -11, 11, 250}) {
        const std::int32_t f = foldConstrainedInt<std::int32_t>(x, -10, 10);
        CHECK(f >= -10);
        CHECK(f <= 10);
        CHECK(foldConstrainedInt<std::int32_t>(f, -10, 10) == f);
    }

    // Frozen range (lo == hi): any value folds to the single point.
    CHECK(foldConstrainedFP<double>(100., 4., 4.) == 4.);
}

/******************************************************************************/
TEST_CASE(
    "normalized-genome: value round-trip is faithful across magnitudes incl. offset/narrow boxes",
    "[normalized-genome]"
) {
    struct Box {
        double lo;
        double hi;
        std::vector<double> vals;
    };
    const std::vector<Box> boxes = {
        {0., 1., {0.0, 0.5, 0.999}},
        {-5., 5., {-4.9, 0.0, 4.9}},
        {1e6, 1e6 + 1., {1e6 + 0.1, 1e6 + 0.5, 1e6 + 0.9}}, // offset + narrow: the cancellation case
        {-1., 1e4, {-0.9, 1234.5, 9999.0}},
    };
    for(const auto &bx : boxes) {
        NgBoxIndividual ind(bx.vals.size(), bx.lo, bx.hi);
        ind.assignValueVector<double>(bx.vals);
        std::vector<double> out;
        ind.streamline<double>(out);
        REQUIRE(out.size() == bx.vals.size());
        for(std::size_t i = 0; i < out.size(); ++i) {
            CHECK(faithful(out[i], bx.vals[i]));
        }
    }
}

/******************************************************************************/
TEST_CASE("normalized-genome: bounded and unbounded coexist on one genome", "[normalized-genome]") {
    NgMixedIndividual ind;
    REQUIRE(ind.countParameters<double>() == 4);

    // First two are bounded [-1,1); next two unbounded -- an unbounded value beyond its
    // init perimeter is legal (NN-weight reachability) and round-trips.
    const std::vector<double> vals{0.5, -0.5, 42.0, -100.0};
    ind.assignValueVector<double>(vals);
    std::vector<double> out;
    ind.streamline<double>(out);
    REQUIRE(out.size() == 4);
    for(std::size_t i = 0; i < 4; ++i) {
        CHECK(faithful(out[i], vals[i]));
    }
}

/******************************************************************************/
TEST_CASE("normalized-genome: randomInit keeps constrained values in range", "[normalized-genome]") {
    NgBoxIndividual ind(64, -3., 7.);
    ind.randomInit(activityMode::ALLPARAMETERS);
    std::vector<double> out;
    ind.streamline<double>(out);
    REQUIRE(out.size() == 64);
    for(double x : out) {
        CHECK(x >= -3.);
        CHECK(x < 7.);
    }
}

/******************************************************************************/
// --- coordinate-model helpers (Phase 1, not yet wired into the live path) --------------------------

namespace {
struct Box {
    double lo;
    double hi;
};
const std::vector<Box> kBoxes = {
    {0., 1.}, {-5., 5.}, {1e6, 1e6 + 1.}, {-1., 1e4} // last two: offset / narrow (cancellation case)
};
} // namespace

/******************************************************************************/
TEST_CASE(
    "normalized-genome: affine internal<->external round-trip is faithful and centered",
    "[normalized-genome]"
) {
    for(const auto &bx : kBoxes) {
        const double scale = bx.hi - bx.lo;
        const double anchor = (bx.lo + bx.hi) / 2.;
        for(double frac : {-0.5, -0.25, 0.0, 0.25, 0.499}) {
            const double x = anchor + frac * scale; // an in-range external value
            const double u = ngExternalToInternal<double>(x, scale, anchor);
            CHECK(u >= -0.5);
            CHECK(u < 0.5); // in-range external maps into the canonical interval
            const double x2 = ngInternalToExternal<double>(u, scale, anchor);
            CHECK(faithful(x2, x)); // faithful round-trip, incl. offset/narrow boxes
        }
    }
    // A frozen parameter (scale <= 0) maps everything to the interval centre.
    CHECK(ngExternalToInternal<double>(4., 0., 4.) == 0.);
}

/******************************************************************************/
TEST_CASE(
    "normalized-genome: the normalized fold path reproduces the user-coordinate fold",
    "[normalized-genome]"
) {
    // For in- and out-of-range probes, folding in the centered internal interval and mapping back
    // must equal the existing user-coordinate fold -- this is what makes the Phase 2 switch safe.
    // Probes span the REALISTIC regime: with fold-on-write the internal value is folded every step, so
    // the fold never sees more than ~1 step out of range. (Folding a value many box-widths out is
    // precision-meaningless -- the residual is a catastrophic cancellation in either framing -- so for
    // far-out inputs only range-membership is meaningful; that is covered by the fold-contract test.)
    const std::vector<double> fracs = {-2.7, -1.5, -1.0, -0.5, 0.0, 0.25, 0.499, 0.9, 1.5, 2.7};
    for(const auto &bx : kBoxes) {
        const double scale = bx.hi - bx.lo;
        const double anchor = (bx.lo + bx.hi) / 2.;
        for(double frac : fracs) {
            const double x = anchor + frac * scale;
            const double ext_old = foldConstrainedFP<double>(x, bx.lo, bx.hi);

            const double u = ngExternalToInternal<double>(x, scale, anchor);
            const double uf = ngFoldInternal<double>(u);
            // affine map back, then enforce the half-open contract in external coordinates
            const double ext_new =
                ngClampHalfOpen<double>(ngInternalToExternal<double>(uf, scale, anchor), bx.lo, bx.hi);

            CHECK(faithful(ext_new, ext_old));
            CHECK(ext_new >= bx.lo);
            CHECK(ext_new < bx.hi);
        }
    }
}

/******************************************************************************/
TEST_CASE(
    "normalized-genome: ngScale/ngAnchor derive from bounds (constrained) and perimeter (plain), "
    "and scale == the stored range",
    "[normalized-genome]"
) {
    GGenomeBuilder b;
    b.addDoubleGroup(2, -1., 3.);        // constrained: scale = 4, anchor = 1
    b.addDoublePlainGroup(2, -10., 10.); // plain: scale = 20, anchor = 0 (from the init perimeter)
    auto g = b.build();
    REQUIRE(g.layout);
    const ChannelLayout<double> &ch = g.layout->d;
    REQUIRE(ch.size() == 4);

    CHECK(ngScale(ch, 0) == 4.);
    CHECK(ngAnchor(ch, 0) == 1.);
    CHECK(ngScale(ch, 2) == 20.);
    CHECK(ngAnchor(ch, 2) == 0.);

    // The derived scale equals the per-group `range` already in the layout (they are unified in Phase 3).
    REQUIRE(ch.groups.size() == 2);
    CHECK(ch.groups[0].range == ngScale(ch, 0));
    CHECK(ch.groups[1].range == ngScale(ch, 2));
}
