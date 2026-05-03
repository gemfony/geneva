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

using Catch::Approx;

/********************************************************************************************/
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

/********************************************************************************************/
// grational_sigmoid tests

TEST_CASE("grational_sigmoid<double>: zero input yields zero output", "[common][math][grational_sigmoid]") {
    // f(0) = barrier * 0 / (steepness + 0) = 0
    REQUIRE(Gem::Common::grational_sigmoid(0., 10., 1.) == Approx(0.));
    REQUIRE(Gem::Common::grational_sigmoid(0., -5., 2.) == Approx(0.));
}

TEST_CASE("grational_sigmoid<float>: zero input yields zero output", "[common][math][grational_sigmoid]") {
    REQUIRE(Gem::Common::grational_sigmoid(0.f, 10.f, 1.f) == Approx(0.f));
}

TEST_CASE("grational_sigmoid<double>: output is bounded by ±barrier", "[common][math][grational_sigmoid]") {
    const double barrier = 10.;
    const double steepness = 1.;

    // For large positive var the output approaches +barrier
    double large_pos = Gem::Common::grational_sigmoid(1e6, barrier, steepness);
    REQUIRE(large_pos > 0.);
    REQUIRE(large_pos < barrier);
    REQUIRE(large_pos == Approx(barrier).epsilon(0.01)); // within 1%

    // For large negative var the output approaches -barrier
    double large_neg = Gem::Common::grational_sigmoid(-1e6, barrier, steepness);
    REQUIRE(large_neg < 0.);
    REQUIRE(large_neg > -barrier);
    REQUIRE(large_neg == Approx(-barrier).epsilon(0.01));
}

TEST_CASE("grational_sigmoid<float>: output is bounded by ±barrier", "[common][math][grational_sigmoid]") {
    const float barrier = 5.f;
    const float steepness = 1.f;

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
    const double barrier = 10.;
    const double steepness = 1.;

    for (double v : {0.1, 1.0, 5.0, 100.0}) {
        REQUIRE(Gem::Common::grational_sigmoid(-v, barrier, steepness) ==
                Approx(-Gem::Common::grational_sigmoid(v, barrier, steepness)));
    }
}

TEST_CASE("grational_sigmoid<float>: antisymmetry f(-x) == -f(x)", "[common][math][grational_sigmoid]") {
    const float barrier = 3.f;
    const float steepness = 2.f;

    for (float v : {0.1f, 1.0f, 5.0f}) {
        REQUIRE(Gem::Common::grational_sigmoid(-v, barrier, steepness) ==
                Approx(-Gem::Common::grational_sigmoid(v, barrier, steepness)));
    }
}

TEST_CASE("grational_sigmoid<double>: steeper curve converges faster", "[common][math][grational_sigmoid]") {
    // A larger steepness parameter means slower convergence to the barrier
    const double barrier = 10.;
    const double v = 5.;

    double slow = Gem::Common::grational_sigmoid(v, barrier, 10.);   // high steepness → slow
    double fast = Gem::Common::grational_sigmoid(v, barrier, 1.);    // low steepness → fast

    // Both should be positive and below the barrier
    REQUIRE(slow > 0.);
    REQUIRE(fast > 0.);
    REQUIRE(slow < barrier);
    REQUIRE(fast < barrier);
    // Lower steepness = faster convergence to barrier = larger value at same v
    REQUIRE(fast > slow);
}

TEST_CASE("grational_sigmoid<double>: known value at v==steepness", "[common][math][grational_sigmoid]") {
    // f(steepness, barrier, steepness) = barrier * s / (s + s) = barrier / 2
    for (double s : {1., 2., 5.}) {
        REQUIRE(Gem::Common::grational_sigmoid(s, 10., s) == Approx(5.));
    }
}
