/**
 * @file GCommonStandardTests.cpp
 */

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
#include <catch2/catch_approx.hpp>

#include "common/tests/GCommon_tests.hpp"
#include "common/GCommonMathHelperFunctionsT.hpp"
#include "common/GCommonHelperFunctionsT.hpp"
#include "common/GCanvas.hpp"

#include <cmath>
#include <cstring>
#include <map>
#include <vector>
#include <tuple>
#include <memory>
#include <string>

using Catch::Approx;

/******************************************************************************/
// Minimal class hierarchy used across pointer-related tests

namespace {

struct TBase {
    virtual ~TBase() = default;
    int base_val = 0;
};

struct TDerived : TBase {
    int derived_val = 42;
};

struct TOther : TBase {
    int other_val = 99;
};

enum class ScopedColor : unsigned { Red = 0, Green = 1, Blue = 2 };

} // namespace

/******************************************************************************/
// GBoundedBuffer tests (pre-existing)

TEST_CASE_METHOD(Gem::Common::Tests::GBoundedBufferT_tests,
                 "GBoundedBuffer no_failure_expected",
                 "[common][standard]") {
    no_failure_expected();
}

TEST_CASE_METHOD(Gem::Common::Tests::GBoundedBufferT_tests,
                 "GBoundedBuffer failures_expected",
                 "[common][standard][failures-expected]") {
    failures_expected();
}

/******************************************************************************/
// ============================================================
// GCommonMathHelperFunctionsT tests
// ============================================================

// --- grational_sigmoid ---------------------------------------------------

TEST_CASE("grational_sigmoid<double>: zero input yields zero output", "[common][math][grational_sigmoid]") {
    REQUIRE(Gem::Common::grational_sigmoid(0., 10., 1.) == Approx(0.));
    REQUIRE(Gem::Common::grational_sigmoid(0., -5., 2.) == Approx(0.));
}

TEST_CASE("grational_sigmoid<float>: zero input yields zero output", "[common][math][grational_sigmoid]") {
    REQUIRE(Gem::Common::grational_sigmoid(0.f, 10.f, 1.f) == Approx(0.f));
}

TEST_CASE("grational_sigmoid<double>: output is bounded by ±barrier", "[common][math][grational_sigmoid]") {
    const double barrier = 10., steepness = 1.;
    double large_pos = Gem::Common::grational_sigmoid(1e6, barrier, steepness);
    REQUIRE(large_pos > 0.);
    REQUIRE(large_pos < barrier);
    REQUIRE(large_pos == Approx(barrier).epsilon(0.01));

    double large_neg = Gem::Common::grational_sigmoid(-1e6, barrier, steepness);
    REQUIRE(large_neg < 0.);
    REQUIRE(large_neg > -barrier);
    REQUIRE(large_neg == Approx(-barrier).epsilon(0.01));
}

TEST_CASE("grational_sigmoid<float>: output is bounded by ±barrier", "[common][math][grational_sigmoid]") {
    const float barrier = 5.f, steepness = 1.f;
    float large_pos = Gem::Common::grational_sigmoid(1e5f, barrier, steepness);
    REQUIRE(large_pos > 0.f);
    REQUIRE(large_pos < barrier);
    REQUIRE(large_pos == Approx(barrier).epsilon(0.01));

    float large_neg = Gem::Common::grational_sigmoid(-1e5f, barrier, steepness);
    REQUIRE(large_neg < 0.f);
    REQUIRE(large_neg > -barrier);
    REQUIRE(large_neg == Approx(-barrier).epsilon(0.01));
}

TEST_CASE("grational_sigmoid<double>: antisymmetry f(-x) == -f(x)", "[common][math][grational_sigmoid]") {
    const double barrier = 10., steepness = 1.;
    for (double v : {0.1, 1.0, 5.0, 100.0}) {
        REQUIRE(Gem::Common::grational_sigmoid(-v, barrier, steepness) ==
                Approx(-Gem::Common::grational_sigmoid(v, barrier, steepness)));
    }
}

TEST_CASE("grational_sigmoid<float>: antisymmetry f(-x) == -f(x)", "[common][math][grational_sigmoid]") {
    const float barrier = 3.f, steepness = 2.f;
    for (float v : {0.1f, 1.0f, 5.0f}) {
        REQUIRE(Gem::Common::grational_sigmoid(-v, barrier, steepness) ==
                Approx(-Gem::Common::grational_sigmoid(v, barrier, steepness)));
    }
}

TEST_CASE("grational_sigmoid<double>: steeper curve converges faster", "[common][math][grational_sigmoid]") {
    const double barrier = 10., v = 5.;
    double slow = Gem::Common::grational_sigmoid(v, barrier, 10.);
    double fast = Gem::Common::grational_sigmoid(v, barrier, 1.);
    REQUIRE(slow > 0.);
    REQUIRE(fast > 0.);
    REQUIRE(slow < barrier);
    REQUIRE(fast < barrier);
    REQUIRE(fast > slow);
}

TEST_CASE("grational_sigmoid<double>: known value at v==steepness", "[common][math][grational_sigmoid]") {
    for (double s : {1., 2., 5.}) {
        REQUIRE(Gem::Common::grational_sigmoid(s, 10., s) == Approx(5.));
    }
}

#ifdef DEBUG
TEST_CASE("grational_sigmoid: zero steepness throws in DEBUG mode", "[common][math][grational_sigmoid]") {
    REQUIRE_THROWS_AS(Gem::Common::grational_sigmoid(1., 10., 0.), geneva_exception);
    REQUIRE_THROWS_AS(Gem::Common::grational_sigmoid(1., 10., -1.), geneva_exception);
}
#endif

// --- enforceRangeConstraint ----------------------------------------------

TEST_CASE("enforceRangeConstraint: value in range is unchanged", "[common][math][enforceRangeConstraint]") {
    double v = 5.;
    Gem::Common::enforceRangeConstraint(v, 0., 10.);
    REQUIRE(v == Approx(5.));
}

TEST_CASE("enforceRangeConstraint: value below lower is clamped", "[common][math][enforceRangeConstraint]") {
    double v = -3.;
    Gem::Common::enforceRangeConstraint(v, 0., 10.);
    REQUIRE(v == Approx(0.));
}

TEST_CASE("enforceRangeConstraint: value above upper is clamped", "[common][math][enforceRangeConstraint]") {
    double v = 15.;
    Gem::Common::enforceRangeConstraint(v, 0., 10.);
    REQUIRE(v == Approx(10.));
}

TEST_CASE("enforceRangeConstraint: lower > upper throws", "[common][math][enforceRangeConstraint]") {
    double v = 5.;
    REQUIRE_THROWS_AS(
        Gem::Common::enforceRangeConstraint(v, 10., 0.),
        geneva_exception
    );
}

TEST_CASE("enforceRangeConstraint: boundary values are kept", "[common][math][enforceRangeConstraint]") {
    double lo = 0., hi = 10.;
    Gem::Common::enforceRangeConstraint(lo, 0., 10.);
    Gem::Common::enforceRangeConstraint(hi, 0., 10.);
    REQUIRE(lo == Approx(0.));
    REQUIRE(hi == Approx(10.));
}

// --- checkRangeCompliance ------------------------------------------------

TEST_CASE("checkRangeCompliance<fp>: value in range returns true", "[common][math][checkRangeCompliance]") {
    REQUIRE(Gem::Common::checkRangeCompliance(5., 0., 10.));
    REQUIRE(Gem::Common::checkRangeCompliance(0., 0., 10.));
    REQUIRE(Gem::Common::checkRangeCompliance(10., 0., 10.));
}

TEST_CASE("checkRangeCompliance<fp>: value out of range returns false", "[common][math][checkRangeCompliance]") {
    REQUIRE_FALSE(Gem::Common::checkRangeCompliance(-1., 0., 10.));
    REQUIRE_FALSE(Gem::Common::checkRangeCompliance(11., 0., 10.));
}

TEST_CASE("checkRangeCompliance<fp>: lower > upper throws", "[common][math][checkRangeCompliance]") {
    REQUIRE_THROWS_AS(Gem::Common::checkRangeCompliance(5., 10., 0.), geneva_exception);
}

TEST_CASE("checkRangeCompliance<int>: value in range returns true", "[common][math][checkRangeCompliance]") {
    REQUIRE(Gem::Common::checkRangeCompliance(5, 0, 10));
    REQUIRE(Gem::Common::checkRangeCompliance(0, 0, 10));
    REQUIRE(Gem::Common::checkRangeCompliance(10, 0, 10));
}

TEST_CASE("checkRangeCompliance<int>: value out of range returns false", "[common][math][checkRangeCompliance]") {
    REQUIRE_FALSE(Gem::Common::checkRangeCompliance(-1, 0, 10));
    REQUIRE_FALSE(Gem::Common::checkRangeCompliance(11, 0, 10));
}

TEST_CASE("checkRangeCompliance<int>: lower > upper throws", "[common][math][checkRangeCompliance]") {
    REQUIRE_THROWS_AS(Gem::Common::checkRangeCompliance(5, 10, 0), geneva_exception);
}

// --- getWorstCase / getBestCase ------------------------------------------

TEST_CASE("getWorstCase/getBestCase bool overloads", "[common][math][worstBestCase]") {
    // maxMode=true: maximising — worst is lowest, best is highest
    REQUIRE(Gem::Common::getWorstCase<double>(true) < 0.);
    REQUIRE(Gem::Common::getBestCase<double>(true) > 0.);
    // maxMode=false: minimising — worst is highest, best is lowest
    REQUIRE(Gem::Common::getWorstCase<double>(false) > 0.);
    REQUIRE(Gem::Common::getBestCase<double>(false) < 0.);
}

TEST_CASE("getWorstCase/getBestCase sortOrder overloads", "[common][math][worstBestCase]") {
    using so = Gem::Common::sortOrder;
    REQUIRE(Gem::Common::getWorstCase<double>(so::HIGHERISBETTER) < 0.);
    REQUIRE(Gem::Common::getBestCase<double>(so::HIGHERISBETTER) > 0.);
    REQUIRE(Gem::Common::getWorstCase<double>(so::LOWERISBETTER) > 0.);
    REQUIRE(Gem::Common::getBestCase<double>(so::LOWERISBETTER) < 0.);
}

// --- checkValueRange -----------------------------------------------------

TEST_CASE("checkValueRange<fp>: value in closed range is returned", "[common][math][checkValueRange]") {
    REQUIRE(Gem::Common::checkValueRange(5., 0., 10.) == Approx(5.));
    REQUIRE(Gem::Common::checkValueRange(0., 0., 10.) == Approx(0.));
    REQUIRE(Gem::Common::checkValueRange(10., 0., 10.) == Approx(10.));
}

TEST_CASE("checkValueRange<fp>: value out of range throws by default", "[common][math][checkValueRange]") {
    REQUIRE_THROWS_AS(Gem::Common::checkValueRange(-1., 0., 10.), geneva_exception);
    REQUIRE_THROWS_AS(Gem::Common::checkValueRange(11., 0., 10.), geneva_exception);
}

TEST_CASE("checkValueRange<fp>: warnOnly does not throw", "[common][math][checkValueRange]") {
    REQUIRE_NOTHROW(Gem::Common::checkValueRange(-1., 0., 10., false, false, true));
}

TEST_CASE("checkValueRange<fp>: open boundaries exclude endpoints", "[common][math][checkValueRange]") {
    // Open lower: value == min should be out of range
    REQUIRE_THROWS_AS(
        Gem::Common::checkValueRange(0., 0., 10., GFPLOWEROPEN, GFPUPPERCLOSED),
        geneva_exception
    );
    // Open upper: value == max should be out of range
    REQUIRE_THROWS_AS(
        Gem::Common::checkValueRange(10., 0., 10., GFPLOWERCLOSED, GFPUPPEROPEN),
        geneva_exception
    );
}

TEST_CASE("checkValueRange<int>: value in range is returned", "[common][math][checkValueRange]") {
    REQUIRE(Gem::Common::checkValueRange(5, 0, 10) == 5);
    REQUIRE(Gem::Common::checkValueRange(0, 0, 10) == 0);
    REQUIRE(Gem::Common::checkValueRange(10, 0, 10) == 10);
}

TEST_CASE("checkValueRange<int>: value out of range throws", "[common][math][checkValueRange]") {
    REQUIRE_THROWS_AS(Gem::Common::checkValueRange(-1, 0, 10), geneva_exception);
    REQUIRE_THROWS_AS(Gem::Common::checkValueRange(11, 0, 10), geneva_exception);
}

TEST_CASE("checkValueRange<int>: open boundaries exclude endpoints", "[common][math][checkValueRange]") {
    REQUIRE_THROWS_AS(
        Gem::Common::checkValueRange(0, 0, 10, true /*lowerOpen*/),
        geneva_exception
    );
    REQUIRE_THROWS_AS(
        Gem::Common::checkValueRange(10, 0, 10, false, true /*upperOpen*/),
        geneva_exception
    );
}

// --- getMinMax -----------------------------------------------------------

TEST_CASE("getMinMax 1D: returns correct min and max", "[common][math][getMinMax]") {
    std::vector<double> v{3., 1., 4., 1., 5., 9., 2., 6.};
    auto [lo, hi] = Gem::Common::getMinMax(v);
    REQUIRE(lo == Approx(1.));
    REQUIRE(hi == Approx(9.));
}

TEST_CASE("getMinMax 1D: two-element vector", "[common][math][getMinMax]") {
    std::vector<int> v{7, 3};
    auto [lo, hi] = Gem::Common::getMinMax(v);
    REQUIRE(lo == 3);
    REQUIRE(hi == 7);
}

TEST_CASE("getMinMax 1D: size < 2 throws", "[common][math][getMinMax]") {
    REQUIRE_THROWS_AS(Gem::Common::getMinMax(std::vector<double>{1.}), geneva_exception);
    REQUIRE_THROWS_AS(Gem::Common::getMinMax(std::vector<double>{}),   geneva_exception);
}

TEST_CASE("getMinMax 2D: returns correct extremes for both dimensions", "[common][math][getMinMax]") {
    using T = std::tuple<double, double>;
    std::vector<T> v{{1., 10.}, {3., 2.}, {2., 8.}};
    auto [minX, maxX, minY, maxY] = Gem::Common::getMinMax(v);
    REQUIRE(minX == Approx(1.));
    REQUIRE(maxX == Approx(3.));
    REQUIRE(minY == Approx(2.));
    REQUIRE(maxY == Approx(10.));
}

TEST_CASE("getMinMax 2D: size < 2 throws", "[common][math][getMinMax]") {
    using T = std::tuple<double, double>;
    REQUIRE_THROWS_AS(Gem::Common::getMinMax(std::vector<T>{{1., 2.}}), geneva_exception);
}

TEST_CASE("getMinMax 3D: returns correct extremes for all three dimensions", "[common][math][getMinMax]") {
    using T = std::tuple<double, double, double>;
    std::vector<T> v{{1., 10., 100.}, {3., 2., 50.}, {2., 8., 200.}};
    auto [minX, maxX, minY, maxY, minZ, maxZ] = Gem::Common::getMinMax(v);
    REQUIRE(minX == Approx(1.));
    REQUIRE(maxX == Approx(3.));
    REQUIRE(minY == Approx(2.));
    REQUIRE(maxY == Approx(10.));
    REQUIRE(minZ == Approx(50.));
    REQUIRE(maxZ == Approx(200.));
}

TEST_CASE("getMinMax 4D: returns correct extremes for all four dimensions", "[common][math][getMinMax]") {
    using T = std::tuple<double, double, double, double>;
    std::vector<T> v{{1., 10., 100., 1000.}, {3., 2., 50., 500.}, {2., 8., 200., 2000.}};
    auto [minX, maxX, minY, maxY, minZ, maxZ, minW, maxW] = Gem::Common::getMinMax(v);
    REQUIRE(minX == Approx(1.));
    REQUIRE(maxX == Approx(3.));
    REQUIRE(minY == Approx(2.));
    REQUIRE(maxY == Approx(10.));
    REQUIRE(minZ == Approx(50.));
    REQUIRE(maxZ == Approx(200.));
    REQUIRE(minW == Approx(500.));
    REQUIRE(maxW == Approx(2000.));
}

// --- GMean ---------------------------------------------------------------

TEST_CASE("GMean: single element returns that element", "[common][math][GMean]") {
    std::vector<double> v{7.5};
    REQUIRE(Gem::Common::GMean(v) == Approx(7.5));
}

TEST_CASE("GMean: known arithmetic mean", "[common][math][GMean]") {
    std::vector<double> v{1., 2., 3., 4., 5.};
    REQUIRE(Gem::Common::GMean(v) == Approx(3.));
}

TEST_CASE("GMean: constant vector returns the constant", "[common][math][GMean]") {
    std::vector<double> v(10, 4.2);
    REQUIRE(Gem::Common::GMean(v) == Approx(4.2));
}

#ifdef DEBUG
TEST_CASE("GMean: empty vector throws in DEBUG mode", "[common][math][GMean]") {
    REQUIRE_THROWS_AS(Gem::Common::GMean(std::vector<double>{}), geneva_exception);
}
#endif

// --- GStandardDeviation --------------------------------------------------

TEST_CASE("GStandardDeviation: single element gives sigma=0", "[common][math][GStandardDeviation]") {
    std::vector<double> v{3.7};
    auto [mean, sigma] = Gem::Common::GStandardDeviation(v);
    REQUIRE(mean  == Approx(3.7));
    REQUIRE(sigma == Approx(0.));
}

TEST_CASE("GStandardDeviation: constant vector gives sigma=0", "[common][math][GStandardDeviation]") {
    std::vector<double> v(5, 2.0);
    auto [mean, sigma] = Gem::Common::GStandardDeviation(v);
    REQUIRE(mean  == Approx(2.0));
    REQUIRE(sigma == Approx(0.));
}

TEST_CASE("GStandardDeviation: known values {1,2,3,4,5}", "[common][math][GStandardDeviation]") {
    std::vector<double> v{1., 2., 3., 4., 5.};
    auto [mean, sigma] = Gem::Common::GStandardDeviation(v);
    REQUIRE(mean  == Approx(3.));
    REQUIRE(sigma == Approx(std::sqrt(2.5)));
}

// --- PowSmallPosInt ------------------------------------------------------

TEST_CASE("PowSmallPosInt: compile-time integer powers", "[common][math][PowSmallPosInt]") {
    // Exponent 0 → always 1
    static_assert(Gem::Common::PowSmallPosInt<2, 0>() == 1);
    static_assert(Gem::Common::PowSmallPosInt<5, 0>() == 1);
    // Exponent 1 → base
    static_assert(Gem::Common::PowSmallPosInt<7, 1>() == 7);
    // Powers of 2
    static_assert(Gem::Common::PowSmallPosInt<2, 1>() == 2);
    static_assert(Gem::Common::PowSmallPosInt<2, 2>() == 4);
    static_assert(Gem::Common::PowSmallPosInt<2, 8>() == 256);
    // Powers of 3
    static_assert(Gem::Common::PowSmallPosInt<3, 3>() == 27);
    static_assert(Gem::Common::PowSmallPosInt<3, 4>() == 81);
    // Powers of 10
    static_assert(Gem::Common::PowSmallPosInt<10, 3>() == 1000);
    SUCCEED();
}

// --- subtractVec / addVec ------------------------------------------------

TEST_CASE("subtractVec: element-wise subtraction", "[common][math][subtractVec]") {
    std::vector<double> a{5., 7., 9.};
    std::vector<double> b{1., 2., 3.};
    Gem::Common::subtractVec(a, b);
    REQUIRE(a[0] == Approx(4.));
    REQUIRE(a[1] == Approx(5.));
    REQUIRE(a[2] == Approx(6.));
}

TEST_CASE("addVec: element-wise addition", "[common][math][addVec]") {
    std::vector<double> a{1., 2., 3.};
    std::vector<double> b{4., 5., 6.};
    Gem::Common::addVec(a, b);
    REQUIRE(a[0] == Approx(5.));
    REQUIRE(a[1] == Approx(7.));
    REQUIRE(a[2] == Approx(9.));
}

TEST_CASE("subtractVec/addVec: inverse operations cancel out", "[common][math][subtractVec][addVec]") {
    std::vector<double> original{3., 1., 4., 1., 5.};
    std::vector<double> a = original;
    std::vector<double> b{2., 7., 1., 8., 2.};
    Gem::Common::addVec(a, b);
    Gem::Common::subtractVec(a, b);
    for (std::size_t i = 0; i < a.size(); ++i) {
        REQUIRE(a[i] == Approx(original[i]));
    }
}

#ifdef DEBUG
TEST_CASE("subtractVec: size mismatch throws in DEBUG", "[common][math][subtractVec]") {
    std::vector<double> a{1., 2.}, b{3., 4., 5.};
    REQUIRE_THROWS_AS(Gem::Common::subtractVec(a, b), geneva_exception);
}

TEST_CASE("addVec: size mismatch throws in DEBUG", "[common][math][addVec]") {
    std::vector<double> a{1., 2.}, b{3.};
    REQUIRE_THROWS_AS(Gem::Common::addVec(a, b), geneva_exception);
}
#endif

// --- multVecConst / assignVecConst ---------------------------------------

TEST_CASE("multVecConst: each element is multiplied by the constant", "[common][math][multVecConst]") {
    std::vector<double> a{1., 2., 3.};
    Gem::Common::multVecConst(a, 3.);
    REQUIRE(a[0] == Approx(3.));
    REQUIRE(a[1] == Approx(6.));
    REQUIRE(a[2] == Approx(9.));
}

TEST_CASE("multVecConst: multiply by 0 gives zero vector", "[common][math][multVecConst]") {
    std::vector<double> a{1., 2., 3.};
    Gem::Common::multVecConst(a, 0.);
    for (auto v : a) REQUIRE(v == Approx(0.));
}

TEST_CASE("assignVecConst: each element is set to the constant", "[common][math][assignVecConst]") {
    std::vector<double> a{1., 2., 3., 4.};
    Gem::Common::assignVecConst(a, 7.);
    for (auto v : a) REQUIRE(v == Approx(7.));
}

TEST_CASE("assignVecConst: empty vector is a no-op", "[common][math][assignVecConst]") {
    std::vector<double> a;
    REQUIRE_NOTHROW(Gem::Common::assignVecConst(a, 1.));
    REQUIRE(a.empty());
}

// --- sumTupleVec / squareSumTupleVec / productSumTupleVec ---------------

TEST_CASE("sumTupleVec: sums x- and y-components independently", "[common][math][sumTupleVec]") {
    using T = std::tuple<double, double>;
    std::vector<T> v{{1., 2.}, {3., 4.}, {5., 6.}};
    auto [sx, sy] = Gem::Common::sumTupleVec(v);
    REQUIRE(sx == Approx(9.));
    REQUIRE(sy == Approx(12.));
}

TEST_CASE("squareSumTupleVec: sums squares of x- and y-components", "[common][math][squareSumTupleVec]") {
    using T = std::tuple<double, double>;
    std::vector<T> v{{1., 2.}, {3., 4.}};
    auto [sqx, sqy] = Gem::Common::squareSumTupleVec(v);
    REQUIRE(sqx == Approx(10.));  // 1² + 3² = 1 + 9
    REQUIRE(sqy == Approx(20.));  // 2² + 4² = 4 + 16
}

TEST_CASE("productSumTupleVec: sums x*y products", "[common][math][productSumTupleVec]") {
    using T = std::tuple<double, double>;
    std::vector<T> v{{1., 2.}, {3., 4.}};
    double ps = Gem::Common::productSumTupleVec(v);
    REQUIRE(ps == Approx(14.));  // 1*2 + 3*4 = 2 + 12
}

// --- squareDeviation -----------------------------------------------------

TEST_CASE("squareDeviation: points exactly on a line give zero deviation", "[common][math][squareDeviation]") {
    // Line y = 2x + 1; a=1, b=2
    using T = std::tuple<double, double>;
    std::vector<T> pts{{0., 1.}, {1., 3.}, {2., 5.}, {3., 7.}};
    REQUIRE(Gem::Common::squareDeviation(pts, 1., 2.) == Approx(0.).margin(1e-10));
}

TEST_CASE("squareDeviation: offset points give positive deviation", "[common][math][squareDeviation]") {
    using T = std::tuple<double, double>;
    // All points 1 unit above y = x (a=0, b=1), so each contributes 1² = 1
    std::vector<T> pts{{0., 1.}, {1., 2.}, {2., 3.}};
    REQUIRE(Gem::Common::squareDeviation(pts, 0., 1.) == Approx(3.));
}

// --- getRegressionParameters ---------------------------------------------

TEST_CASE("getRegressionParameters: empty data returns zeros", "[common][math][getRegressionParameters]") {
    using T = std::tuple<double, double>;
    auto [a, sa, b, sb] = Gem::Common::getRegressionParameters(std::vector<T>{});
    REQUIRE(a  == Approx(0.));
    REQUIRE(sa == Approx(0.));
    REQUIRE(b  == Approx(0.));
    REQUIRE(sb == Approx(0.));
}

TEST_CASE("getRegressionParameters: points on y=2x+1 recover a=1, b=2", "[common][math][getRegressionParameters]") {
    using T = std::tuple<double, double>;
    std::vector<T> pts{{0., 1.}, {1., 3.}, {2., 5.}, {3., 7.}};
    auto [a, sa, b, sb] = Gem::Common::getRegressionParameters(pts);
    REQUIRE(a  == Approx(1.).margin(1e-10));
    REQUIRE(b  == Approx(2.).margin(1e-10));
    REQUIRE(sa == Approx(0.).margin(1e-10));
    REQUIRE(sb == Approx(0.).margin(1e-10));
}

// --- getRatioError / getRatioErrors --------------------------------------

TEST_CASE("getRatioError: known ratio and propagated error", "[common][math][getRatioError]") {
    // s=(sleep=2, ε=0, val=6, err=0.3), p=(sleep=2, ε=0, val=2, err=0.1)
    // ratio = 3; s_term = 0.3/2 = 0.15; p_term = 6*0.1/4 = 0.15
    // error = sqrt(0.15² + 0.15²) = 0.15*sqrt(2)
    auto s = std::tuple<double,double,double,double>{2., 0., 6., 0.3};
    auto p = std::tuple<double,double,double,double>{2., 0., 2., 0.1};
    auto [t, te, ratio, err] = Gem::Common::getRatioError(s, p);
    REQUIRE(t     == Approx(2.));
    REQUIRE(te    == Approx(0.));
    REQUIRE(ratio == Approx(3.));
    REQUIRE(err   == Approx(0.15 * std::sqrt(2.)));
}

TEST_CASE("getRatioError: division by zero throws", "[common][math][getRatioError]") {
    auto s = std::tuple<double,double,double,double>{1., 0., 4., 0.1};
    auto p = std::tuple<double,double,double,double>{1., 0., 0., 0.1};
    REQUIRE_THROWS_AS(Gem::Common::getRatioError(s, p), geneva_exception);
}

TEST_CASE("getRatioError: mismatched sleep times throws", "[common][math][getRatioError]") {
    auto s = std::tuple<double,double,double,double>{1., 0., 4., 0.1};
    auto p = std::tuple<double,double,double,double>{2., 0., 2., 0.1};
    REQUIRE_THROWS_AS(Gem::Common::getRatioError(s, p), geneva_exception);
}

TEST_CASE("getRatioErrors: size mismatch throws", "[common][math][getRatioErrors]") {
    using T4 = std::tuple<double,double,double,double>;
    std::vector<T4> sn{{1., 0., 2., 0.1}};
    std::vector<T4> pn{{1., 0., 1., 0.1}, {2., 0., 1., 0.1}};
    REQUIRE_THROWS_AS(Gem::Common::getRatioErrors(sn, pn), geneva_exception);
}

TEST_CASE("getRatioErrors: applies getRatioError element-wise", "[common][math][getRatioErrors]") {
    using T4 = std::tuple<double,double,double,double>;
    // Two identical pairs, each gives ratio=2
    std::vector<T4> sn{{1., 0., 4., 0.}, {1., 0., 4., 0.}};
    std::vector<T4> pn{{1., 0., 2., 0.}, {1., 0., 2., 0.}};
    auto result = Gem::Common::getRatioErrors(sn, pn);
    REQUIRE(result.size() == 2);
    for (const auto& r : result) {
        REQUIRE(std::get<2>(r) == Approx(2.));
    }
}

// --- isClose -------------------------------------------------------------

TEST_CASE("isClose: value within margin returns true", "[common][math][isClose]") {
    REQUIRE(Gem::Common::isClose(0.5, 0.5));
    REQUIRE(Gem::Common::isClose(0.50001, 0.5, 0.001));
    REQUIRE(Gem::Common::isClose(-1e-6, 0., 1e-5));
}

TEST_CASE("isClose: value outside margin returns false", "[common][math][isClose]") {
    REQUIRE_FALSE(Gem::Common::isClose(1.0, 0.0));
    REQUIRE_FALSE(Gem::Common::isClose(0.1, 0.0, 0.05));
}

TEST_CASE("isClose: exact zero target", "[common][math][isClose]") {
    REQUIRE(Gem::Common::isClose(0.));
    REQUIRE_FALSE(Gem::Common::isClose(1.0));
}

/******************************************************************************/
// ============================================================
// GCommonHelperFunctionsT tests
// ============================================================

// --- g_delete ------------------------------------------------------------

TEST_CASE("g_delete: deletes and nullifies a non-null pointer", "[common][helper][g_delete]") {
    int* p = new int(42);
    Gem::Common::g_delete(p);
    REQUIRE(p == nullptr);
}

TEST_CASE("g_delete: null pointer is a no-op", "[common][helper][g_delete]") {
    int* p = nullptr;
    REQUIRE_NOTHROW(Gem::Common::g_delete(p));
    REQUIRE(p == nullptr);
}

// --- g_array_delete ------------------------------------------------------

TEST_CASE("g_array_delete: deletes and nullifies a non-null array pointer", "[common][helper][g_array_delete]") {
    int* p = new int[5]{1, 2, 3, 4, 5};
    Gem::Common::g_array_delete(p);
    REQUIRE(p == nullptr);
}

TEST_CASE("g_array_delete: null pointer is a no-op", "[common][helper][g_array_delete]") {
    int* p = nullptr;
    REQUIRE_NOTHROW(Gem::Common::g_array_delete(p));
    REQUIRE(p == nullptr);
}

// --- ptrDifferenceCheck --------------------------------------------------

TEST_CASE("ptrDifferenceCheck raw: different pointers do not throw", "[common][helper][ptrDifferenceCheck]") {
    int a = 1, b = 2;
    REQUIRE_NOTHROW(Gem::Common::ptrDifferenceCheck(&a, &b));
}

TEST_CASE("ptrDifferenceCheck raw: null first pointer is a no-op", "[common][helper][ptrDifferenceCheck]") {
    int a = 1;
    REQUIRE_NOTHROW(Gem::Common::ptrDifferenceCheck<int>(nullptr, &a));
}

#ifdef DEBUG
TEST_CASE("ptrDifferenceCheck raw: same pointer throws in DEBUG", "[common][helper][ptrDifferenceCheck]") {
    int a = 1;
    REQUIRE_THROWS_AS(Gem::Common::ptrDifferenceCheck(&a, &a), geneva_exception);
}

TEST_CASE("ptrDifferenceCheck shared_ptr: aliasing shared_ptrs throw in DEBUG", "[common][helper][ptrDifferenceCheck]") {
    auto p = std::make_shared<int>(42);
    REQUIRE_THROWS_AS(Gem::Common::ptrDifferenceCheck(p, p), geneva_exception);
}
#endif

TEST_CASE("ptrDifferenceCheck shared_ptr: distinct objects do not throw", "[common][helper][ptrDifferenceCheck]") {
    auto p1 = std::make_shared<int>(1);
    auto p2 = std::make_shared<int>(2);
    REQUIRE_NOTHROW(Gem::Common::ptrDifferenceCheck(p1, p2));
}

// --- g_ptr_conversion ----------------------------------------------------

TEST_CASE("g_ptr_conversion raw: valid downcast returns non-null", "[common][helper][g_ptr_conversion]") {
    TDerived d;
    const TBase* base_ptr = &d;
    const TDerived* derived_ptr = Gem::Common::g_ptr_conversion<TBase, TDerived>(base_ptr);
    REQUIRE(derived_ptr != nullptr);
    REQUIRE(derived_ptr == &d);
}

TEST_CASE("g_ptr_conversion raw: null input returns null", "[common][helper][g_ptr_conversion]") {
    const TDerived* p = Gem::Common::g_ptr_conversion<TBase, TDerived>(static_cast<const TBase*>(nullptr));
    REQUIRE(p == nullptr);
}

#ifdef DEBUG
TEST_CASE("g_ptr_conversion raw: wrong dynamic type throws in DEBUG", "[common][helper][g_ptr_conversion]") {
    TOther other;
    const TBase* base_ptr = &other;
    REQUIRE_THROWS_AS(
        (Gem::Common::g_ptr_conversion<TBase, TDerived>(base_ptr)),
        geneva_exception
    );
}
#endif

TEST_CASE("g_ptr_conversion shared_ptr: valid downcast returns non-null", "[common][helper][g_ptr_conversion]") {
    auto sp = std::make_shared<TDerived>();
    auto base_sp = std::static_pointer_cast<TBase>(sp);
    auto derived_sp = Gem::Common::g_ptr_conversion<TBase, TDerived>(base_sp);
    REQUIRE(derived_sp != nullptr);
    REQUIRE(derived_sp.get() == sp.get());
}

TEST_CASE("g_ptr_conversion shared_ptr: null input returns null", "[common][helper][g_ptr_conversion]") {
    std::shared_ptr<TBase> null_sp;
    auto result = Gem::Common::g_ptr_conversion<TBase, TDerived>(null_sp);
    REQUIRE(result == nullptr);
}

// --- convertSmartPointer -------------------------------------------------

TEST_CASE("convertSmartPointer: valid downcast succeeds", "[common][helper][convertSmartPointer]") {
    auto sp = std::make_shared<TDerived>();
    auto base_sp = std::static_pointer_cast<TBase>(sp);
    auto derived_sp = Gem::Common::convertSmartPointer<TBase, TDerived>(base_sp);
    REQUIRE(derived_sp != nullptr);
}

#ifdef DEBUG
TEST_CASE("convertSmartPointer: null input throws in DEBUG", "[common][helper][convertSmartPointer]") {
    std::shared_ptr<TBase> null_sp;
    REQUIRE_THROWS_AS(
        (Gem::Common::convertSmartPointer<TBase, TDerived>(null_sp)),
        geneva_exception
    );
}

TEST_CASE("convertSmartPointer: wrong dynamic type throws in DEBUG", "[common][helper][convertSmartPointer]") {
    auto sp = std::make_shared<TOther>();
    auto base_sp = std::static_pointer_cast<TBase>(sp);
    REQUIRE_THROWS_AS(
        (Gem::Common::convertSmartPointer<TBase, TDerived>(base_sp)),
        geneva_exception
    );
}
#endif

// --- vecToString ---------------------------------------------------------

TEST_CASE("vecToString: empty vector returns empty string", "[common][helper][vecToString]") {
    REQUIRE(Gem::Common::vecToString(std::vector<int>{}).empty());
}

TEST_CASE("vecToString: integer vector formats correctly", "[common][helper][vecToString]") {
    std::string s = Gem::Common::vecToString(std::vector<int>{1, 2, 3});
    REQUIRE(s == "1 2 3 ");
}

TEST_CASE("vecToString: single-element vector", "[common][helper][vecToString]") {
    REQUIRE(Gem::Common::vecToString(std::vector<int>{42}) == "42 ");
}

// --- copyArrays ----------------------------------------------------------

TEST_CASE("copyArrays: copies values correctly", "[common][helper][copyArrays]") {
    const int from_arr[] = {10, 20, 30};
    const int* from = from_arr;
    const std::size_t nFrom = 3;
    int* to = nullptr;
    std::size_t nTo = 0;

    Gem::Common::copyArrays(from, to, nFrom, nTo);

    REQUIRE(nTo == 3);
    REQUIRE(to[0] == 10);
    REQUIRE(to[1] == 20);
    REQUIRE(to[2] == 30);

    Gem::Common::g_array_delete(to);
}

TEST_CASE("copyArrays: null source clears destination", "[common][helper][copyArrays]") {
    int* to = new int[3]{1, 2, 3};
    std::size_t nTo = 3;

    Gem::Common::copyArrays<int>(nullptr, to, 0, nTo);

    REQUIRE(nTo == 0);
    REQUIRE(to == nullptr);
}

TEST_CASE("copyArrays: reallocates when sizes differ", "[common][helper][copyArrays]") {
    const int from_arr[] = {7, 8};
    const int* from = from_arr;
    const std::size_t nFrom = 2;
    int* to = new int[5]{1, 2, 3, 4, 5};
    std::size_t nTo = 5;

    Gem::Common::copyArrays(from, to, nFrom, nTo);

    REQUIRE(nTo == 2);
    REQUIRE(to[0] == 7);
    REQUIRE(to[1] == 8);

    Gem::Common::g_array_delete(to);
}

TEST_CASE("copyArrays: null from with nFrom>0 throws", "[common][helper][copyArrays]") {
    int* to = nullptr;
    std::size_t nTo = 0;
    REQUIRE_THROWS_AS((Gem::Common::copyArrays<int>(nullptr, to, 1, nTo)), geneva_exception);
}

TEST_CASE("copyArrays: non-null from with nFrom=0 throws", "[common][helper][copyArrays]") {
    const int x = 1;
    int* to = nullptr;
    std::size_t nTo = 0;
    REQUIRE_THROWS_AS((Gem::Common::copyArrays(&x, to, 0, nTo)), geneva_exception);
}

// --- splitStringT (single separator) -------------------------------------

TEST_CASE("splitStringT<int>: splits space-separated integers", "[common][helper][splitStringT]") {
    auto result = Gem::Common::splitStringT<int>("1 2 3 4", " ");
    REQUIRE(result.size() == 4);
    REQUIRE(result[0] == 1);
    REQUIRE(result[3] == 4);
}

TEST_CASE("splitStringT<double>: splits comma-separated doubles", "[common][helper][splitStringT]") {
    auto result = Gem::Common::splitStringT<double>("1.5,2.5,3.5", ",");
    REQUIRE(result.size() == 3);
    REQUIRE(result[0] == Approx(1.5));
    REQUIRE(result[2] == Approx(3.5));
}

TEST_CASE("splitStringT<string>: splits pipe-separated strings", "[common][helper][splitStringT]") {
    auto result = Gem::Common::splitStringT<std::string>("foo|bar|baz", "|");
    REQUIRE(result.size() == 3);
    REQUIRE(result[0] == "foo");
    REQUIRE(result[2] == "baz");
}

// --- splitStringT (two separators) ---------------------------------------

TEST_CASE("splitStringT<int,int>: splits pairs from '0/1 2/3'", "[common][helper][splitStringT2]") {
    auto result = Gem::Common::splitStringT<int, int>("0/1 2/3", " ", "/");
    REQUIRE(result.size() == 2);
    REQUIRE(std::get<0>(result[0]) == 0);
    REQUIRE(std::get<1>(result[0]) == 1);
    REQUIRE(std::get<0>(result[1]) == 2);
    REQUIRE(std::get<1>(result[1]) == 3);
}

TEST_CASE("splitStringT two-sep: identical separators throw", "[common][helper][splitStringT2]") {
    REQUIRE_THROWS_AS(
        (Gem::Common::splitStringT<int, int>("0/1", "/", "/")),
        geneva_exception
    );
}

// --- getMapItem ----------------------------------------------------------

TEST_CASE("getMapItem: returns correct value for existing key", "[common][helper][getMapItem]") {
    std::map<std::string, int> m{{"a", 1}, {"b", 2}, {"c", 3}};
    REQUIRE(Gem::Common::getMapItem(m, "b") == 2);
}

TEST_CASE("getMapItem: modifying returned reference changes map", "[common][helper][getMapItem]") {
    std::map<std::string, int> m{{"x", 10}};
    Gem::Common::getMapItem(m, "x") = 99;
    REQUIRE(m.at("x") == 99);
}

TEST_CASE("getMapItem const: returns correct value for existing key", "[common][helper][getMapItem]") {
    const std::map<std::string, double> m{{"pi", 3.14159}};
    REQUIRE(Gem::Common::getMapItem(m, "pi") == Approx(3.14159));
}

TEST_CASE("getMapItem: missing key throws", "[common][helper][getMapItem]") {
    std::map<std::string, int> m{{"a", 1}};
    REQUIRE_THROWS_AS(Gem::Common::getMapItem(m, "z"), geneva_exception);
}

TEST_CASE("getMapItem: empty map throws", "[common][helper][getMapItem]") {
    std::map<std::string, int> m;
    REQUIRE_THROWS_AS(Gem::Common::getMapItem(m, "a"), geneva_exception);
}

// --- to_string -----------------------------------------------------------

TEST_CASE("to_string: integral types", "[common][helper][to_string]") {
    REQUIRE(Gem::Common::to_string(42)   == "42");
    REQUIRE(Gem::Common::to_string(-7)   == "-7");
    REQUIRE(Gem::Common::to_string(0)    == "0");
    REQUIRE(Gem::Common::to_string(42u)  == "42");
}

TEST_CASE("to_string: floating-point types", "[common][helper][to_string]") {
    // lexical_cast is locale-independent, always uses '.' as decimal separator
    REQUIRE(Gem::Common::to_string(1.5)  == "1.5");
    REQUIRE(Gem::Common::to_string(0.)   == "0");
    REQUIRE(Gem::Common::to_string(1.5f) == "1.5");
}

TEST_CASE("to_string: scoped enum yields its underlying integer", "[common][helper][to_string]") {
    REQUIRE(Gem::Common::to_string(ScopedColor::Red)   == "0");
    REQUIRE(Gem::Common::to_string(ScopedColor::Green) == "1");
    REQUIRE(Gem::Common::to_string(ScopedColor::Blue)  == "2");
}

// --- erase_if ------------------------------------------------------------

TEST_CASE("erase_if: removes matching elements and returns count", "[common][helper][erase_if]") {
    std::vector<int> v{1, 2, 3, 4, 5, 6};
    std::size_t n = Gem::Common::erase_if(v, [](int x){ return x % 2 == 0; });
    REQUIRE(n == 3);
    REQUIRE(v == std::vector<int>{1, 3, 5});
}

TEST_CASE("erase_if: nothing matches returns 0 and leaves container unchanged", "[common][helper][erase_if]") {
    std::vector<int> v{1, 3, 5};
    std::size_t n = Gem::Common::erase_if(v, [](int x){ return x % 2 == 0; });
    REQUIRE(n == 0);
    REQUIRE(v == std::vector<int>{1, 3, 5});
}

TEST_CASE("erase_if: empty container is a no-op", "[common][helper][erase_if]") {
    std::vector<int> v;
    std::size_t n = Gem::Common::erase_if(v, [](int){ return true; });
    REQUIRE(n == 0);
    REQUIRE(v.empty());
}

TEST_CASE("erase_if: all elements match clears the container", "[common][helper][erase_if]") {
    std::vector<int> v{2, 4, 6};
    std::size_t n = Gem::Common::erase_if(v, [](int){ return true; });
    REQUIRE(n == 3);
    REQUIRE(v.empty());
}

// --- environmentVariableAs -----------------------------------------------

TEST_CASE("environmentVariableAs: missing variable returns empty optional", "[common][helper][environmentVariableAs]") {
    // Use a name that is extremely unlikely to be set
    auto result = Gem::Common::environmentVariableAs<int>("GENEVA_TEST_NONEXISTENT_VAR_XYZ_12345");
    REQUIRE_FALSE(result.has_value());
}

TEST_CASE("environmentVariableAs: existing variable is read and converted", "[common][helper][environmentVariableAs]") {
    ::setenv("GENEVA_TEST_VAR", "42", 1);
    auto result = Gem::Common::environmentVariableAs<int>("GENEVA_TEST_VAR");
    REQUIRE(result.has_value());
    REQUIRE(result.value() == 42);
    ::unsetenv("GENEVA_TEST_VAR");
}

TEST_CASE("environmentVariableAs: string variable is returned verbatim", "[common][helper][environmentVariableAs]") {
    ::setenv("GENEVA_TEST_STR", "hello", 1);
    auto result = Gem::Common::environmentVariableAs<std::string>("GENEVA_TEST_STR");
    REQUIRE(result.has_value());
    REQUIRE(result.value() == "hello");
    ::unsetenv("GENEVA_TEST_STR");
}

/******************************************************************************/
// ============================================================
// GCanvas tests
// ============================================================

namespace {

// Minimal valid P3-PPM: 2×2, depth 8, with four distinct extremal colors.
// Row 0: pixel(0,0)=red,  pixel(1,0)=green
// Row 1: pixel(0,1)=blue, pixel(1,1)=white
const std::string k2x2Ppm =
    "P3\n"
    "2 2\n"
    "255\n"
    "255 0 0 0 255 0\n"
    "0 0 255 255 255 255\n";

} // namespace

// --- coord2D ---

TEST_CASE("coord2D: default construction sets x and y to zero", "[common][canvas][coord2D]") {
    Gem::Common::coord2D c;
    REQUIRE(c.x == 0.f);
    REQUIRE(c.y == 0.f);
}

TEST_CASE("coord2D: construction with values", "[common][canvas][coord2D]") {
    Gem::Common::coord2D c{0.3f, 0.7f};
    REQUIRE(c.x == Approx(0.3f));
    REQUIRE(c.y == Approx(0.7f));
}

TEST_CASE("coord2D: operator- computes component-wise difference", "[common][canvas][coord2D]") {
    Gem::Common::coord2D a{0.5f, 0.8f};
    Gem::Common::coord2D b{0.2f, 0.3f};
    auto d = a - b;
    REQUIRE(d.x == Approx(0.3f));
    REQUIRE(d.y == Approx(0.5f));
}

TEST_CASE("coord2D: operator* computes dot product", "[common][canvas][coord2D]") {
    Gem::Common::coord2D a{1.f, 2.f};
    Gem::Common::coord2D b{3.f, 4.f};
    REQUIRE((a * b) == Approx(11.f));  // 1*3 + 2*4
}

TEST_CASE("coord2D: dot product of perpendicular vectors is zero", "[common][canvas][coord2D]") {
    Gem::Common::coord2D a{1.f, 0.f};
    Gem::Common::coord2D b{0.f, 1.f};
    REQUIRE((a * b) == Approx(0.f));
}

// --- GRgb ---

TEST_CASE("GRgb: default construction yields black", "[common][canvas][GRgb]") {
    Gem::Common::GRgb p;
    REQUIRE(p.r == 0.f);
    REQUIRE(p.g == 0.f);
    REQUIRE(p.b == 0.f);
}

TEST_CASE("GRgb: construction with rgb floats", "[common][canvas][GRgb]") {
    Gem::Common::GRgb p{0.1f, 0.5f, 0.9f};
    REQUIRE(p.r == Approx(0.1f));
    REQUIRE(p.g == Approx(0.5f));
    REQUIRE(p.b == Approx(0.9f));
}

TEST_CASE("GRgb: construction from tuple", "[common][canvas][GRgb]") {
    Gem::Common::GRgb p{std::make_tuple(0.2f, 0.4f, 0.6f)};
    REQUIRE(p.r == Approx(0.2f));
    REQUIRE(p.g == Approx(0.4f));
    REQUIRE(p.b == Approx(0.6f));
}

TEST_CASE("GRgb: setColor with floats", "[common][canvas][GRgb]") {
    Gem::Common::GRgb p;
    p.setColor(0.3f, 0.6f, 0.9f);
    REQUIRE(p.r == Approx(0.3f));
    REQUIRE(p.g == Approx(0.6f));
    REQUIRE(p.b == Approx(0.9f));
}

TEST_CASE("GRgb: setColor with tuple", "[common][canvas][GRgb]") {
    Gem::Common::GRgb p;
    p.setColor(std::make_tuple(0.1f, 0.2f, 0.3f));
    REQUIRE(p.r == Approx(0.1f));
    REQUIRE(p.g == Approx(0.2f));
    REQUIRE(p.b == Approx(0.3f));
}

TEST_CASE("GRgb: copy preserves values", "[common][canvas][GRgb]") {
    Gem::Common::GRgb src{0.7f, 0.8f, 0.9f};
    Gem::Common::GRgb copy = src;
    REQUIRE(copy.r == Approx(0.7f));
    REQUIRE(copy.g == Approx(0.8f));
    REQUIRE(copy.b == Approx(0.9f));
}

// --- GColumn ---

TEST_CASE("GColumn: default construction yields empty column", "[common][canvas][GColumn]") {
    Gem::Common::GColumn col;
    REQUIRE(col.size() == 0);
}

TEST_CASE("GColumn: construction sets size and uniform color", "[common][canvas][GColumn]") {
    Gem::Common::GColumn col{4, std::make_tuple(0.5f, 0.25f, 0.75f)};
    REQUIRE(col.size() == 4);
    for (std::size_t i = 0; i < 4; ++i) {
        REQUIRE(col[i].r == Approx(0.5f));
        REQUIRE(col[i].g == Approx(0.25f));
        REQUIRE(col[i].b == Approx(0.75f));
    }
}

TEST_CASE("GColumn: operator[] allows mutation", "[common][canvas][GColumn]") {
    Gem::Common::GColumn col{3, std::make_tuple(0.f, 0.f, 0.f)};
    col[1].r = 1.f;
    REQUIRE(col[1].r == Approx(1.f));
    REQUIRE(col[0].r == Approx(0.f));
    REQUIRE(col[2].r == Approx(0.f));
}

TEST_CASE("GColumn: at() throws on out-of-range access", "[common][canvas][GColumn]") {
    Gem::Common::GColumn col{2, std::make_tuple(0.f, 0.f, 0.f)};
    REQUIRE_NOTHROW(col.at(0));
    REQUIRE_NOTHROW(col.at(1));
    REQUIRE_THROWS_AS(col.at(2), std::out_of_range);
}

TEST_CASE("GColumn: const at() throws on out-of-range", "[common][canvas][GColumn]") {
    const Gem::Common::GColumn col{2, std::make_tuple(1.f, 0.f, 0.f)};
    REQUIRE(col.at(0).r == Approx(1.f));
    REQUIRE_THROWS_AS(col.at(2), std::out_of_range);
}

TEST_CASE("GColumn: init() resizes and recolors", "[common][canvas][GColumn]") {
    Gem::Common::GColumn col{3, std::make_tuple(1.f, 0.f, 0.f)};
    col.init(5, std::make_tuple(0.f, 1.f, 0.f));
    REQUIRE(col.size() == 5);
    for (std::size_t i = 0; i < 5; ++i) {
        REQUIRE(col[i].r == Approx(0.f));
        REQUIRE(col[i].g == Approx(1.f));
        REQUIRE(col[i].b == Approx(0.f));
    }
}

// --- t_circle ---

TEST_CASE("t_circle: default construction yields all-zero fields", "[common][canvas][t_circle]") {
    Gem::Common::t_circle tc;
    REQUIRE(tc.middle.x == 0.f);
    REQUIRE(tc.radius == 0.f);
    REQUIRE(tc.a == 0.f);
}

TEST_CASE("t_circle: getAlphaValue returns the alpha field", "[common][canvas][t_circle]") {
    Gem::Common::t_circle tc;
    tc.a = 0.75f;
    REQUIRE(tc.getAlphaValue() == Approx(0.75f));
}

TEST_CASE("t_circle: operator== and operator!=", "[common][canvas][t_circle]") {
    Gem::Common::t_circle a, b;
    REQUIRE(a == b);
    REQUIRE_FALSE(a != b);
    b.r = 1.f;
    REQUIRE_FALSE(a == b);
    REQUIRE(a != b);
}

TEST_CASE("t_circle: toString returns non-empty string", "[common][canvas][t_circle]") {
    Gem::Common::t_circle tc;
    tc.middle = Gem::Common::coord2D{0.5f, 0.5f};
    tc.radius = 0.2f;
    REQUIRE_FALSE(tc.toString().empty());
}

// --- GCanvas<8>: construction and accessors ---

TEST_CASE("GCanvas<8>: default construction yields empty canvas", "[common][canvas][GCanvas]") {
    Gem::Common::GCanvas<8> c;
    REQUIRE(c.getXDim() == 0);
    REQUIRE(c.getYDim() == 0);
    REQUIRE(c.getNPixels() == 0);
}

TEST_CASE("GCanvas<8>: construction with dimensions and color sets all pixels", "[common][canvas][GCanvas]") {
    Gem::Common::GCanvas<8> c{
        std::make_tuple(std::size_t{10}, std::size_t{8}),
        std::make_tuple(0.5f, 0.25f, 0.1f)
    };
    REQUIRE(c.getXDim() == 10);
    REQUIRE(c.getYDim() == 8);
    REQUIRE(c.getNPixels() == 80);
    REQUIRE(c[0][0].r == Approx(0.5f));
    REQUIRE(c[0][0].g == Approx(0.25f));
    REQUIRE(c[9][7].r == Approx(0.5f));
}

TEST_CASE("GCanvas<8>: getColorDepth/getNColors/getMaxColor", "[common][canvas][GCanvas]") {
    Gem::Common::GCanvas<8> c;
    REQUIRE(c.getColorDepth() == 8);
    REQUIRE(c.getNColors() == 256);
    REQUIRE(c.getMaxColor() == 255);
}

TEST_CASE("GCanvas<16>: getColorDepth/getNColors/getMaxColor", "[common][canvas][GCanvas16]") {
    Gem::Common::GCanvas<16> c;
    REQUIRE(c.getColorDepth() == 16);
    REQUIRE(c.getNColors() == 65536);
    REQUIRE(c.getMaxColor() == 65535);
}

TEST_CASE("GCanvas<24>: getColorDepth/getNColors/getMaxColor", "[common][canvas][GCanvas24]") {
    Gem::Common::GCanvas<24> c;
    REQUIRE(c.getColorDepth() == 24);
    REQUIRE(c.getNColors() == 16777216);
    REQUIRE(c.getMaxColor() == 16777215);
}

TEST_CASE("GCanvas<8>: dimensions() returns correct tuple", "[common][canvas][GCanvas]") {
    Gem::Common::GCanvas<8> c{
        std::make_tuple(std::size_t{7}, std::size_t{3}),
        std::make_tuple(0.f, 0.f, 0.f)
    };
    auto [x, y] = c.dimensions();
    REQUIRE(x == 7);
    REQUIRE(y == 3);
}

TEST_CASE("GCanvas<8>: operator[] mutable access", "[common][canvas][GCanvas]") {
    Gem::Common::GCanvas<8> c{
        std::make_tuple(std::size_t{2}, std::size_t{2}),
        std::make_tuple(0.f, 0.f, 0.f)
    };
    c[0][0].r = 0.5f;
    REQUIRE(c[0][0].r == Approx(0.5f));
    REQUIRE(c[1][0].r == Approx(0.f));
}

TEST_CASE("GCanvas<8>: at() mutable — in-range ok, out-of-range throws", "[common][canvas][GCanvas]") {
    Gem::Common::GCanvas<8> c{
        std::make_tuple(std::size_t{3}, std::size_t{3}),
        std::make_tuple(1.f, 0.f, 0.f)
    };
    REQUIRE_NOTHROW(c.at(0));
    REQUIRE_NOTHROW(c.at(2));
    REQUIRE_THROWS_AS(c.at(3), std::out_of_range);
}

TEST_CASE("GCanvas<8>: at() const — in-range ok, out-of-range throws", "[common][canvas][GCanvas]") {
    const Gem::Common::GCanvas<8> c{
        std::make_tuple(std::size_t{3}, std::size_t{3}),
        std::make_tuple(0.5f, 0.f, 0.f)
    };
    REQUIRE(c.at(0)[0].r == Approx(0.5f));
    REQUIRE_THROWS_AS(c.at(3), std::out_of_range);
}

// --- GCanvas<8>: clear and reset ---

TEST_CASE("GCanvas<8>: clear() resets dimensions to zero", "[common][canvas][GCanvas]") {
    Gem::Common::GCanvas<8> c{
        std::make_tuple(std::size_t{4}, std::size_t{4}),
        std::make_tuple(1.f, 1.f, 1.f)
    };
    c.clear();
    REQUIRE(c.getXDim() == 0);
    REQUIRE(c.getYDim() == 0);
    REQUIRE(c.getNPixels() == 0);
}

TEST_CASE("GCanvas<8>: reset(dim, r,g,b) changes size and fills color", "[common][canvas][GCanvas]") {
    Gem::Common::GCanvas<8> c;
    c.reset(std::make_tuple(std::size_t{3}, std::size_t{2}), 0.f, 1.f, 0.f);
    REQUIRE(c.getXDim() == 3);
    REQUIRE(c.getYDim() == 2);
    for (std::size_t x = 0; x < 3; ++x)
        for (std::size_t y = 0; y < 2; ++y) {
            REQUIRE(c[x][y].r == Approx(0.f));
            REQUIRE(c[x][y].g == Approx(1.f));
            REQUIRE(c[x][y].b == Approx(0.f));
        }
}

TEST_CASE("GCanvas<8>: reset(dim, tuple) changes size and fills color", "[common][canvas][GCanvas]") {
    Gem::Common::GCanvas<8> c;
    c.reset(std::make_tuple(std::size_t{2}, std::size_t{2}), std::make_tuple(0.1f, 0.2f, 0.3f));
    REQUIRE(c[0][0].r == Approx(0.1f));
    REQUIRE(c[1][1].b == Approx(0.3f));
}

TEST_CASE("GCanvas<8>: reset on non-empty canvas replaces old data", "[common][canvas][GCanvas]") {
    Gem::Common::GCanvas<8> c{
        std::make_tuple(std::size_t{5}, std::size_t{5}),
        std::make_tuple(1.f, 0.f, 0.f)
    };
    c.reset(std::make_tuple(std::size_t{2}, std::size_t{2}), std::make_tuple(0.f, 0.f, 1.f));
    REQUIRE(c.getXDim() == 2);
    REQUIRE(c[0][0].r == Approx(0.f));
    REQUIRE(c[0][0].b == Approx(1.f));
}

// --- GCanvas<8>: PPM output ---

TEST_CASE("GCanvas<8>: toPPM begins with 'P3' header", "[common][canvas][GCanvas][ppm]") {
    Gem::Common::GCanvas<8> c{
        std::make_tuple(std::size_t{1}, std::size_t{1}),
        std::make_tuple(0.f, 0.f, 0.f)
    };
    REQUIRE(c.toPPM().substr(0, 3) == "P3\n");
}

TEST_CASE("GCanvas<8>: toPPM encodes a 1×1 red canvas correctly", "[common][canvas][GCanvas][ppm]") {
    Gem::Common::GCanvas<8> c{
        std::make_tuple(std::size_t{1}, std::size_t{1}),
        std::make_tuple(1.f, 0.f, 0.f)
    };
    std::string ppm = c.toPPM();
    REQUIRE(ppm.find("1 1") != std::string::npos);
    REQUIRE(ppm.find("255") != std::string::npos);
    REQUIRE(ppm.find("255 0 0") != std::string::npos);
}

TEST_CASE("GCanvas<8>: toPPM encodes a 1×1 black canvas as all zeros", "[common][canvas][GCanvas][ppm]") {
    Gem::Common::GCanvas<8> c{
        std::make_tuple(std::size_t{1}, std::size_t{1}),
        std::make_tuple(0.f, 0.f, 0.f)
    };
    REQUIRE(c.toPPM().find("0 0 0") != std::string::npos);
}

// --- GCanvas<8>: PPM loading ---

TEST_CASE("GCanvas<8>: PPM string constructor parses 2×2 image correctly", "[common][canvas][GCanvas][ppm]") {
    Gem::Common::GCanvas<8> c{k2x2Ppm};
    REQUIRE(c.getXDim() == 2);
    REQUIRE(c.getYDim() == 2);
    // pixel (0,0) = red
    REQUIRE(c[0][0].r == Approx(1.f));
    REQUIRE(c[0][0].g == Approx(0.f));
    REQUIRE(c[0][0].b == Approx(0.f));
    // pixel (1,0) = green
    REQUIRE(c[1][0].r == Approx(0.f));
    REQUIRE(c[1][0].g == Approx(1.f));
    REQUIRE(c[1][0].b == Approx(0.f));
    // pixel (0,1) = blue
    REQUIRE(c[0][1].r == Approx(0.f));
    REQUIRE(c[0][1].g == Approx(0.f));
    REQUIRE(c[0][1].b == Approx(1.f));
    // pixel (1,1) = white
    REQUIRE(c[1][1].r == Approx(1.f));
    REQUIRE(c[1][1].g == Approx(1.f));
    REQUIRE(c[1][1].b == Approx(1.f));
}

TEST_CASE("GCanvas<8>: loadFromPPM handles comments and blank lines", "[common][canvas][GCanvas][ppm]") {
    const std::string ppm =
        "# comment\n"
        "P3\n"
        "\n"
        "# another comment\n"
        "1 1\n"
        "255\n"
        "128 64 32\n";
    Gem::Common::GCanvas<8> c;
    REQUIRE_NOTHROW(c.loadFromPPM(ppm));
    REQUIRE(c.getXDim() == 1);
    REQUIRE(c.getYDim() == 1);
}

TEST_CASE("GCanvas<8>: toPPM then loadFromPPM round-trips extremal colors", "[common][canvas][GCanvas][ppm]") {
    Gem::Common::GCanvas<8> original{k2x2Ppm};
    std::string serialized = original.toPPM();
    Gem::Common::GCanvas<8> restored;
    restored.loadFromPPM(serialized);
    REQUIRE(restored.getXDim() == 2);
    REQUIRE(restored.getYDim() == 2);
    REQUIRE(restored[0][0].r == Approx(original[0][0].r));
    REQUIRE(restored[1][0].g == Approx(original[1][0].g));
    REQUIRE(restored[0][1].b == Approx(original[0][1].b));
    REQUIRE(restored[1][1].r == Approx(original[1][1].r));
}

// --- GCanvas<8>: PPM error cases ---

TEST_CASE("GCanvas<8>: loadFromPPM throws on wrong magic number", "[common][canvas][GCanvas][ppm][errors]") {
    Gem::Common::GCanvas<8> c;
    REQUIRE_THROWS_AS(c.loadFromPPM("P6\n1 1\n255\n"), geneva_exception);
}

TEST_CASE("GCanvas<8>: loadFromPPM throws on zero x-dimension", "[common][canvas][GCanvas][ppm][errors]") {
    Gem::Common::GCanvas<8> c;
    REQUIRE_THROWS_AS(c.loadFromPPM("P3\n0 1\n255\n"), geneva_exception);
}

TEST_CASE("GCanvas<8>: loadFromPPM throws on zero y-dimension", "[common][canvas][GCanvas][ppm][errors]") {
    Gem::Common::GCanvas<8> c;
    REQUIRE_THROWS_AS(c.loadFromPPM("P3\n1 0\n255\n"), geneva_exception);
}

TEST_CASE("GCanvas<8>: loadFromPPM throws on wrong color depth", "[common][canvas][GCanvas][ppm][errors]") {
    Gem::Common::GCanvas<8> c;
    REQUIRE_THROWS_AS(c.loadFromPPM("P3\n1 1\n127\n0 0 0\n"), geneva_exception);
}

TEST_CASE("GCanvas<8>: loadFromPPM throws on too-few pixel values", "[common][canvas][GCanvas][ppm][errors]") {
    Gem::Common::GCanvas<8> c;
    // 2×2 needs 12 values; only 3 given
    REQUIRE_THROWS_AS(c.loadFromPPM("P3\n2 2\n255\n255 0 0\n"), geneva_exception);
}

// --- GCanvas<8>: diff ---

TEST_CASE("GCanvas<8>: diff of a canvas with itself is zero", "[common][canvas][GCanvas][diff]") {
    Gem::Common::GCanvas<8> c{
        std::make_tuple(std::size_t{4}, std::size_t{4}),
        std::make_tuple(0.5f, 0.3f, 0.7f)
    };
    REQUIRE(c.diff(c) == Approx(0.f));
}

TEST_CASE("GCanvas<8>: diff of white vs black 1×1 canvas equals sqrt(3)", "[common][canvas][GCanvas][diff]") {
    auto dim = std::make_tuple(std::size_t{1}, std::size_t{1});
    Gem::Common::GCanvas<8> white{dim, std::make_tuple(1.f, 1.f, 1.f)};
    Gem::Common::GCanvas<8> black{dim, std::make_tuple(0.f, 0.f, 0.f)};
    REQUIRE(white.diff(black) == Approx(std::sqrt(3.f)));
}

TEST_CASE("GCanvas<8>: diff is symmetric", "[common][canvas][GCanvas][diff]") {
    auto dim = std::make_tuple(std::size_t{3}, std::size_t{3});
    Gem::Common::GCanvas<8> a{dim, std::make_tuple(1.f, 0.f, 0.f)};
    Gem::Common::GCanvas<8> b{dim, std::make_tuple(0.f, 1.f, 0.f)};
    REQUIRE(a.diff(b) == Approx(b.diff(a)));
}

TEST_CASE("GCanvas<8>: diff scales linearly with pixel count", "[common][canvas][GCanvas][diff]") {
    // 1×1 white vs black: sqrt(3); 1×2 should give 2*sqrt(3)
    Gem::Common::GCanvas<8> w1{std::make_tuple(std::size_t{1}, std::size_t{1}), std::make_tuple(1.f, 1.f, 1.f)};
    Gem::Common::GCanvas<8> b1{std::make_tuple(std::size_t{1}, std::size_t{1}), std::make_tuple(0.f, 0.f, 0.f)};
    float d1 = w1.diff(b1);
    Gem::Common::GCanvas<8> w2{std::make_tuple(std::size_t{1}, std::size_t{2}), std::make_tuple(1.f, 1.f, 1.f)};
    Gem::Common::GCanvas<8> b2{std::make_tuple(std::size_t{1}, std::size_t{2}), std::make_tuple(0.f, 0.f, 0.f)};
    REQUIRE(w2.diff(b2) == Approx(2.f * d1));
}

TEST_CASE("GCanvas<8>: diff throws on mismatched dimensions", "[common][canvas][GCanvas][diff]") {
    Gem::Common::GCanvas<8> a{std::make_tuple(std::size_t{2}, std::size_t{2}), std::make_tuple(0.f, 0.f, 0.f)};
    Gem::Common::GCanvas<8> b{std::make_tuple(std::size_t{3}, std::size_t{3}), std::make_tuple(0.f, 0.f, 0.f)};
    REQUIRE_THROWS_AS(a.diff(b), geneva_exception);
}

TEST_CASE("GCanvas8: operator- is equivalent to diff", "[common][canvas][GCanvas][diff]") {
    Gem::Common::GCanvas8 a{std::make_tuple(std::size_t{2}, std::size_t{2}), std::make_tuple(1.f, 0.f, 0.f)};
    Gem::Common::GCanvas8 b{std::make_tuple(std::size_t{2}, std::size_t{2}), std::make_tuple(0.f, 0.f, 1.f)};
    REQUIRE((a - b) == Approx(a.diff(b)));
}

// --- GCanvas<8>: addTriangle(t_cart) ---

// Helper: build a t_cart that covers the entire unit square plus margin.
// Triangle (0,0)-(3,0)-(0,3) contains every pixel of a 4×4 canvas.
// For a 4×4 canvas, pixel positions are (i+1)/4 ∈ {0.25,0.5,0.75,1.0}.
// The worst case (1.0,1.0): u=v=1/3, u+v=2/3 < 1 → inside. ✓
namespace {
Gem::Common::t_cart full_cover_triangle(float r, float g, float b, float a) {
    Gem::Common::t_cart t;
    t.tr_one   = {0.f, 0.f};
    t.tr_two   = {3.f, 0.f};
    t.tr_three = {0.f, 3.f};
    t.r = r; t.g = g; t.b = b; t.a = a;
    return t;
}
} // namespace

TEST_CASE("GCanvas<8>: addTriangle(t_cart) with alpha=0 leaves canvas unchanged", "[common][canvas][GCanvas][triangle]") {
    Gem::Common::GCanvas<8> c{
        std::make_tuple(std::size_t{4}, std::size_t{4}),
        std::make_tuple(0.5f, 0.5f, 0.5f)
    };
    c.addTriangle(full_cover_triangle(1.f, 0.f, 0.f, 0.f));
    for (std::size_t x = 0; x < 4; ++x)
        for (std::size_t y = 0; y < 4; ++y)
            REQUIRE(c[x][y].r == Approx(0.5f));
}

TEST_CASE("GCanvas<8>: addTriangle(t_cart) with alpha=1 fully overwrites covered pixels", "[common][canvas][GCanvas][triangle]") {
    // White canvas + fully-opaque red triangle covering all pixels.
    // Blend: new_g = 1 + 1*(0-1) = 0; new_b = 0.
    Gem::Common::GCanvas<8> c{
        std::make_tuple(std::size_t{4}, std::size_t{4}),
        std::make_tuple(1.f, 1.f, 1.f)
    };
    c.addTriangle(full_cover_triangle(1.f, 0.f, 0.f, 1.f));
    for (std::size_t x = 0; x < 4; ++x)
        for (std::size_t y = 0; y < 4; ++y) {
            REQUIRE(c[x][y].r == Approx(1.f));
            REQUIRE(c[x][y].g == Approx(0.f));
            REQUIRE(c[x][y].b == Approx(0.f));
        }
}

TEST_CASE("GCanvas<8>: addTriangle(t_cart) with alpha=0.5 blends correctly", "[common][canvas][GCanvas][triangle]") {
    // Black canvas + 50%-opaque white triangle.
    // new_r = 0 + 0.5*(1-0) = 0.5
    Gem::Common::GCanvas<8> c{
        std::make_tuple(std::size_t{4}, std::size_t{4}),
        std::make_tuple(0.f, 0.f, 0.f)
    };
    c.addTriangle(full_cover_triangle(1.f, 1.f, 1.f, 0.5f));
    // Check the pixel at (0,0) — pos_f=(0.25,0.25), provably inside (u=v≈0.083)
    REQUIRE(c[0][0].r == Approx(0.5f));
    REQUIRE(c[0][0].g == Approx(0.5f));
    REQUIRE(c[0][0].b == Approx(0.5f));
}

TEST_CASE("GCanvas<8>: addTriangle(t_cart) leaves pixels outside bounding box unchanged", "[common][canvas][GCanvas][triangle]") {
    // Triangle with all vertices at x > 0.8; pixels at i_x=0 (pos_f.x=0.25) are
    // entirely to the left of the bounding box and must remain untouched.
    Gem::Common::GCanvas<8> c{
        std::make_tuple(std::size_t{4}, std::size_t{4}),
        std::make_tuple(0.5f, 0.5f, 0.5f)
    };
    Gem::Common::t_cart t;
    t.tr_one   = {0.85f, 0.85f};
    t.tr_two   = {0.90f, 0.85f};
    t.tr_three = {0.87f, 0.95f};
    t.r = 1.f; t.g = 0.f; t.b = 0.f; t.a = 1.f;
    c.addTriangle(t);
    // Column 0 is to the left of all triangle vertices — must be gray
    for (std::size_t y = 0; y < 4; ++y)
        REQUIRE(c[0][y].r == Approx(0.5f));
}

TEST_CASE("GCanvas<8>: addTriangles adds multiple triangles", "[common][canvas][GCanvas][triangle]") {
    // Two non-overlapping triangles (left vs right half), both red alpha=1.
    // After both: canvas should be all red everywhere.
    Gem::Common::GCanvas<8> c{
        std::make_tuple(std::size_t{4}, std::size_t{4}),
        std::make_tuple(0.f, 0.f, 1.f)  // blue
    };
    Gem::Common::t_cart t1 = full_cover_triangle(1.f, 0.f, 0.f, 1.f);
    c.addTriangles(std::vector<Gem::Common::t_circle>{});  // verify empty is a no-op
    REQUIRE(c[0][0].b == Approx(1.f));  // unchanged
    c.addTriangle(t1);
    REQUIRE(c[0][0].r == Approx(1.f));
    REQUIRE(c[0][0].b == Approx(0.f));
}

// --- GCanvas<8>: addTriangle(t_circle) ---

TEST_CASE("GCanvas<8>: addTriangle(t_circle) colors pixels inside the derived triangle", "[common][canvas][GCanvas][triangle]") {
    // Circle-based triangle: center(0.5,0.5), radius=0.4, angles 0/0.25/0.5.
    // Derived cartesian vertices: (0.9,0.5), (0.5,0.9), (0.1,0.5).
    // For a 10×10 canvas, pixel(4,4) has pos_f=(0.5,0.5) which lies inside.
    Gem::Common::GCanvas<8> c{
        std::make_tuple(std::size_t{10}, std::size_t{10}),
        std::make_tuple(0.f, 0.f, 0.f)  // black
    };
    Gem::Common::t_circle tc;
    tc.middle = {0.5f, 0.5f};
    tc.radius = 0.4f;
    tc.angle1 = 0.f;
    tc.angle2 = 0.25f;
    tc.angle3 = 0.5f;
    tc.r = 1.f; tc.g = 0.f; tc.b = 0.f; tc.a = 1.f;
    c.addTriangle(tc);
    // pixel (4,4) at pos_f=(0.5,0.5) must now be red
    REQUIRE(c[4][4].r == Approx(1.f));
    REQUIRE(c[4][4].g == Approx(0.f));
    REQUIRE(c[4][4].b == Approx(0.f));
}

#ifdef DEBUG
TEST_CASE("GCanvas<8>: addTriangle(t_circle) throws in DEBUG on non-ascending angles", "[common][canvas][GCanvas][triangle]") {
    Gem::Common::GCanvas<8> c{
        std::make_tuple(std::size_t{4}, std::size_t{4}),
        std::make_tuple(0.f, 0.f, 0.f)
    };
    Gem::Common::t_circle tc;
    tc.middle = {0.5f, 0.5f};
    tc.radius = 0.3f;
    tc.r = 1.f; tc.g = 0.f; tc.b = 0.f; tc.a = 1.f;

    // angle2 <= angle1
    tc.angle1 = 0.3f; tc.angle2 = 0.2f; tc.angle3 = 0.8f;
    REQUIRE_THROWS_AS(c.addTriangle(tc), geneva_exception);

    // angle3 >= 1
    tc.angle1 = 0.1f; tc.angle2 = 0.3f; tc.angle3 = 1.0f;
    REQUIRE_THROWS_AS(c.addTriangle(tc), geneva_exception);

    // angle1 < 0
    tc.angle1 = -0.1f; tc.angle2 = 0.2f; tc.angle3 = 0.5f;
    REQUIRE_THROWS_AS(c.addTriangle(tc), geneva_exception);
}
#endif /* DEBUG */

// --- GCanvas<8>: getAverageColors ---

TEST_CASE("GCanvas<8>: getAverageColors on uniform canvas returns that color", "[common][canvas][GCanvas]") {
    Gem::Common::GCanvas<8> c{
        std::make_tuple(std::size_t{4}, std::size_t{4}),
        std::make_tuple(0.5f, 0.25f, 0.75f)
    };
    auto [ar, ag, ab] = c.getAverageColors();
    REQUIRE(ar == Approx(0.5f));
    REQUIRE(ag == Approx(0.25f));
    REQUIRE(ab == Approx(0.75f));
}

TEST_CASE("GCanvas<8>: getAverageColors of half-red half-blue 2×1 canvas", "[common][canvas][GCanvas]") {
    // 2×1 canvas: pixel(0,0)=red, pixel(1,0)=blue → avg=(0.5,0,0.5)
    Gem::Common::GCanvas<8> c{
        std::make_tuple(std::size_t{2}, std::size_t{1}),
        std::make_tuple(0.f, 0.f, 0.f)
    };
    c[0][0].setColor(1.f, 0.f, 0.f);
    c[1][0].setColor(0.f, 0.f, 1.f);
    auto [ar, ag, ab] = c.getAverageColors();
    REQUIRE(ar == Approx(0.5f));
    REQUIRE(ag == Approx(0.f));
    REQUIRE(ab == Approx(0.5f));
}

TEST_CASE("GCanvas<8>: average shifts after adding opaque triangle", "[common][canvas][GCanvas]") {
    // Start with a black canvas; add a fully-opaque red triangle covering all pixels.
    // Average should go from (0,0,0) to (1,0,0).
    Gem::Common::GCanvas<8> c{
        std::make_tuple(std::size_t{4}, std::size_t{4}),
        std::make_tuple(0.f, 0.f, 0.f)
    };
    auto [ar0, ag0, ab0] = c.getAverageColors();
    REQUIRE(ar0 == Approx(0.f));

    c.addTriangle(full_cover_triangle(1.f, 0.f, 0.f, 1.f));
    auto [ar1, ag1, ab1] = c.getAverageColors();
    REQUIRE(ar1 == Approx(1.f));
    REQUIRE(ag1 == Approx(0.f));
    REQUIRE(ab1 == Approx(0.f));
}

// --- GCanvas8/16/24 concrete classes ---

TEST_CASE("GCanvas8: construction and diff with self", "[common][canvas][GCanvas8]") {
    Gem::Common::GCanvas8 c{
        std::make_tuple(std::size_t{3}, std::size_t{3}),
        std::make_tuple(0.5f, 0.5f, 0.5f)
    };
    REQUIRE(c.getColorDepth() == 8);
    REQUIRE((c - c) == Approx(0.f));
}

TEST_CASE("GCanvas8: PPM string constructor and round-trip", "[common][canvas][GCanvas8]") {
    Gem::Common::GCanvas8 c{k2x2Ppm};
    REQUIRE(c.getXDim() == 2);
    REQUIRE(c.getYDim() == 2);
    std::string ppm = c.toPPM();
    Gem::Common::GCanvas8 c2;
    c2.loadFromPPM(ppm);
    REQUIRE(c2[0][0].r == Approx(c[0][0].r));
}

TEST_CASE("GCanvas16: construction and color depth", "[common][canvas][GCanvas16]") {
    Gem::Common::GCanvas16 c{
        std::make_tuple(std::size_t{2}, std::size_t{2}),
        std::make_tuple(0.f, 0.f, 0.f)
    };
    REQUIRE(c.getColorDepth() == 16);
    REQUIRE(c.getNColors() == 65536);
}

TEST_CASE("GCanvas24: construction and color depth", "[common][canvas][GCanvas24]") {
    Gem::Common::GCanvas24 c{
        std::make_tuple(std::size_t{2}, std::size_t{2}),
        std::make_tuple(1.f, 1.f, 1.f)
    };
    REQUIRE(c.getColorDepth() == 24);
    REQUIRE(c.getMaxColor() == 16777215);
}
