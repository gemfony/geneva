/**
 * @file GCanvasTests.cpp
 * Catch2 unit tests for GCanvas, GRgb, GColumn, coord2D, t_circle, and related types.
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

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "../GCanvas.hpp"

#include <cmath>
#include <memory>
#include <string>
#include <tuple>
#include <vector>

using Catch::Approx;

/******************************************************************************/
// ============================================================
// GCanvas tests
// ============================================================

namespace {

// Minimal valid P3-PPM: 2×2, depth 8, with four distinct extremal colors.
// Row 0: pixel(0,0)=red,  pixel(1,0)=green
// Row 1: pixel(0,1)=blue, pixel(1,1)=white
const std::string k2x2Ppm = "P3\n"
                            "2 2\n"
                            "255\n"
                            "255 0 0 0 255 0\n"
                            "0 0 255 255 255 255\n";

} // namespace

// --- coord2D ---

TEST_CASE("coord2D: default construction sets x and y to zero", "[common][canvas][coord2D]") {
    Gem::Geneva::coord2D c;
    REQUIRE(c.x == 0.f);
    REQUIRE(c.y == 0.f);
}

TEST_CASE("coord2D: construction with values", "[common][canvas][coord2D]") {
    Gem::Geneva::coord2D c{0.3f, 0.7f};
    REQUIRE(c.x == Approx(0.3f));
    REQUIRE(c.y == Approx(0.7f));
}

TEST_CASE("coord2D: operator- computes component-wise difference", "[common][canvas][coord2D]") {
    Gem::Geneva::coord2D a{0.5f, 0.8f};
    Gem::Geneva::coord2D b{0.2f, 0.3f};
    auto d = a - b;
    REQUIRE(d.x == Approx(0.3f));
    REQUIRE(d.y == Approx(0.5f));
}

TEST_CASE("coord2D: operator* computes dot product", "[common][canvas][coord2D]") {
    Gem::Geneva::coord2D a{1.f, 2.f};
    Gem::Geneva::coord2D b{3.f, 4.f};
    REQUIRE((a * b) == Approx(11.f)); // 1*3 + 2*4
}

TEST_CASE("coord2D: dot product of perpendicular vectors is zero", "[common][canvas][coord2D]") {
    Gem::Geneva::coord2D a{1.f, 0.f};
    Gem::Geneva::coord2D b{0.f, 1.f};
    REQUIRE((a * b) == Approx(0.f));
}

// --- GRgb ---

TEST_CASE("GRgb: default construction yields black", "[common][canvas][GRgb]") {
    Gem::Geneva::GRgb p;
    REQUIRE(p.r == 0.f);
    REQUIRE(p.g == 0.f);
    REQUIRE(p.b == 0.f);
}

TEST_CASE("GRgb: construction with rgb floats", "[common][canvas][GRgb]") {
    Gem::Geneva::GRgb p{0.1f, 0.5f, 0.9f};
    REQUIRE(p.r == Approx(0.1f));
    REQUIRE(p.g == Approx(0.5f));
    REQUIRE(p.b == Approx(0.9f));
}

TEST_CASE("GRgb: construction from tuple", "[common][canvas][GRgb]") {
    Gem::Geneva::GRgb p{std::make_tuple(0.2f, 0.4f, 0.6f)};
    REQUIRE(p.r == Approx(0.2f));
    REQUIRE(p.g == Approx(0.4f));
    REQUIRE(p.b == Approx(0.6f));
}

TEST_CASE("GRgb: setColor with floats", "[common][canvas][GRgb]") {
    Gem::Geneva::GRgb p;
    p.setColor(0.3f, 0.6f, 0.9f);
    REQUIRE(p.r == Approx(0.3f));
    REQUIRE(p.g == Approx(0.6f));
    REQUIRE(p.b == Approx(0.9f));
}

TEST_CASE("GRgb: setColor with tuple", "[common][canvas][GRgb]") {
    Gem::Geneva::GRgb p;
    p.setColor(std::make_tuple(0.1f, 0.2f, 0.3f));
    REQUIRE(p.r == Approx(0.1f));
    REQUIRE(p.g == Approx(0.2f));
    REQUIRE(p.b == Approx(0.3f));
}

TEST_CASE("GRgb: copy preserves values", "[common][canvas][GRgb]") {
    Gem::Geneva::GRgb src{0.7f, 0.8f, 0.9f};
    Gem::Geneva::GRgb copy = src;
    REQUIRE(copy.r == Approx(0.7f));
    REQUIRE(copy.g == Approx(0.8f));
    REQUIRE(copy.b == Approx(0.9f));
}

// --- GColumn ---

TEST_CASE("GColumn: default construction yields empty column", "[common][canvas][GColumn]") {
    Gem::Geneva::GColumn col;
    REQUIRE(col.size() == 0);
}

TEST_CASE("GColumn: construction sets size and uniform color", "[common][canvas][GColumn]") {
    Gem::Geneva::GColumn col{4, std::make_tuple(0.5f, 0.25f, 0.75f)};
    REQUIRE(col.size() == 4);
    for(std::size_t i = 0; i < 4; ++i) {
        REQUIRE(col[i].r == Approx(0.5f));
        REQUIRE(col[i].g == Approx(0.25f));
        REQUIRE(col[i].b == Approx(0.75f));
    }
}

TEST_CASE("GColumn: operator[] allows mutation", "[common][canvas][GColumn]") {
    Gem::Geneva::GColumn col{3, std::make_tuple(0.f, 0.f, 0.f)};
    col[1].r = 1.f;
    REQUIRE(col[1].r == Approx(1.f));
    REQUIRE(col[0].r == Approx(0.f));
    REQUIRE(col[2].r == Approx(0.f));
}

TEST_CASE("GColumn: at() throws on out-of-range access", "[common][canvas][GColumn]") {
    Gem::Geneva::GColumn col{2, std::make_tuple(0.f, 0.f, 0.f)};
    REQUIRE_NOTHROW(col.at(0));
    REQUIRE_NOTHROW(col.at(1));
    REQUIRE_THROWS_AS(col.at(2), std::out_of_range);
}

TEST_CASE("GColumn: const at() throws on out-of-range", "[common][canvas][GColumn]") {
    const Gem::Geneva::GColumn col{2, std::make_tuple(1.f, 0.f, 0.f)};
    REQUIRE(col.at(0).r == Approx(1.f));
    REQUIRE_THROWS_AS(col.at(2), std::out_of_range);
}

TEST_CASE("GColumn: init() resizes and recolors", "[common][canvas][GColumn]") {
    Gem::Geneva::GColumn col{3, std::make_tuple(1.f, 0.f, 0.f)};
    col.init(5, std::make_tuple(0.f, 1.f, 0.f));
    REQUIRE(col.size() == 5);
    for(std::size_t i = 0; i < 5; ++i) {
        REQUIRE(col[i].r == Approx(0.f));
        REQUIRE(col[i].g == Approx(1.f));
        REQUIRE(col[i].b == Approx(0.f));
    }
}

// --- t_circle ---

TEST_CASE("t_circle: default construction yields all-zero fields", "[common][canvas][t_circle]") {
    Gem::Geneva::t_circle tc;
    REQUIRE(tc.middle.x == 0.f);
    REQUIRE(tc.radius == 0.f);
    REQUIRE(tc.a == 0.f);
}

TEST_CASE("t_circle: getAlphaValue returns the alpha field", "[common][canvas][t_circle]") {
    Gem::Geneva::t_circle tc;
    tc.a = 0.75f;
    REQUIRE(tc.getAlphaValue() == Approx(0.75f));
}

TEST_CASE("t_circle: operator== and operator!=", "[common][canvas][t_circle]") {
    Gem::Geneva::t_circle a, b;
    REQUIRE(a == b);
    REQUIRE_FALSE(a != b);
    b.r = 1.f;
    REQUIRE_FALSE(a == b);
    REQUIRE(a != b);
}

TEST_CASE("t_circle: toString returns non-empty string", "[common][canvas][t_circle]") {
    Gem::Geneva::t_circle tc;
    tc.middle = Gem::Geneva::coord2D{0.5f, 0.5f};
    tc.radius = 0.2f;
    REQUIRE_FALSE(tc.toString().empty());
}

// --- GCanvas<8>: construction and accessors ---

TEST_CASE("GCanvas<8>: default construction yields empty canvas", "[common][canvas][GCanvas]") {
    Gem::Geneva::GCanvas<8> c;
    REQUIRE(c.getXDim() == 0);
    REQUIRE(c.getYDim() == 0);
    REQUIRE(c.getNPixels() == 0);
}

TEST_CASE(
    "GCanvas<8>: construction with dimensions and color sets all pixels",
    "[common][canvas][GCanvas]"
) {
    Gem::Geneva::GCanvas<8> c{
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
    Gem::Geneva::GCanvas<8> c;
    REQUIRE(c.getColorDepth() == 8);
    REQUIRE(c.getNColors() == 256);
    REQUIRE(c.getMaxColor() == 255);
}

TEST_CASE("GCanvas<16>: getColorDepth/getNColors/getMaxColor", "[common][canvas][GCanvas16]") {
    Gem::Geneva::GCanvas<16> c;
    REQUIRE(c.getColorDepth() == 16);
    REQUIRE(c.getNColors() == 65536);
    REQUIRE(c.getMaxColor() == 65535);
}

TEST_CASE("GCanvas<24>: getColorDepth/getNColors/getMaxColor", "[common][canvas][GCanvas24]") {
    Gem::Geneva::GCanvas<24> c;
    REQUIRE(c.getColorDepth() == 24);
    REQUIRE(c.getNColors() == 16777216);
    REQUIRE(c.getMaxColor() == 16777215);
}

TEST_CASE("GCanvas<8>: dimensions() returns correct tuple", "[common][canvas][GCanvas]") {
    Gem::Geneva::GCanvas<8> c{
        std::make_tuple(std::size_t{7}, std::size_t{3}),
        std::make_tuple(0.f, 0.f, 0.f)
    };
    auto [x, y] = c.dimensions();
    REQUIRE(x == 7);
    REQUIRE(y == 3);
}

TEST_CASE("GCanvas<8>: operator[] mutable access", "[common][canvas][GCanvas]") {
    Gem::Geneva::GCanvas<8> c{
        std::make_tuple(std::size_t{2}, std::size_t{2}),
        std::make_tuple(0.f, 0.f, 0.f)
    };
    c[0][0].r = 0.5f;
    REQUIRE(c[0][0].r == Approx(0.5f));
    REQUIRE(c[1][0].r == Approx(0.f));
}

TEST_CASE(
    "GCanvas<8>: at() mutable — in-range ok, out-of-range throws",
    "[common][canvas][GCanvas]"
) {
    Gem::Geneva::GCanvas<8> c{
        std::make_tuple(std::size_t{3}, std::size_t{3}),
        std::make_tuple(1.f, 0.f, 0.f)
    };
    REQUIRE_NOTHROW(c.at(0));
    REQUIRE_NOTHROW(c.at(2));
    REQUIRE_THROWS_AS(c.at(3), std::out_of_range);
}

TEST_CASE(
    "GCanvas<8>: at() const — in-range ok, out-of-range throws",
    "[common][canvas][GCanvas]"
) {
    const Gem::Geneva::GCanvas<8> c{
        std::make_tuple(std::size_t{3}, std::size_t{3}),
        std::make_tuple(0.5f, 0.f, 0.f)
    };
    REQUIRE(c.at(0)[0].r == Approx(0.5f));
    REQUIRE_THROWS_AS(c.at(3), std::out_of_range);
}

// --- GCanvas<8>: clear and reset ---

TEST_CASE("GCanvas<8>: clear() resets dimensions to zero", "[common][canvas][GCanvas]") {
    Gem::Geneva::GCanvas<8> c{
        std::make_tuple(std::size_t{4}, std::size_t{4}),
        std::make_tuple(1.f, 1.f, 1.f)
    };
    c.clear();
    REQUIRE(c.getXDim() == 0);
    REQUIRE(c.getYDim() == 0);
    REQUIRE(c.getNPixels() == 0);
}

TEST_CASE(
    "GCanvas<8>: reset(dim, r,g,b) changes size and fills color",
    "[common][canvas][GCanvas]"
) {
    Gem::Geneva::GCanvas<8> c;
    c.reset(std::make_tuple(std::size_t{3}, std::size_t{2}), 0.f, 1.f, 0.f);
    REQUIRE(c.getXDim() == 3);
    REQUIRE(c.getYDim() == 2);
    for(std::size_t x = 0; x < 3; ++x) {
        for(std::size_t y = 0; y < 2; ++y) {
            REQUIRE(c[x][y].r == Approx(0.f));
            REQUIRE(c[x][y].g == Approx(1.f));
            REQUIRE(c[x][y].b == Approx(0.f));
        }
    }
}

TEST_CASE(
    "GCanvas<8>: reset(dim, tuple) changes size and fills color",
    "[common][canvas][GCanvas]"
) {
    Gem::Geneva::GCanvas<8> c;
    c.reset(std::make_tuple(std::size_t{2}, std::size_t{2}), std::make_tuple(0.1f, 0.2f, 0.3f));
    REQUIRE(c[0][0].r == Approx(0.1f));
    REQUIRE(c[1][1].b == Approx(0.3f));
}

TEST_CASE("GCanvas<8>: reset on non-empty canvas replaces old data", "[common][canvas][GCanvas]") {
    Gem::Geneva::GCanvas<8> c{
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
    Gem::Geneva::GCanvas<8> c{
        std::make_tuple(std::size_t{1}, std::size_t{1}),
        std::make_tuple(0.f, 0.f, 0.f)
    };
    REQUIRE(c.toPPM().substr(0, 3) == "P3\n");
}

TEST_CASE(
    "GCanvas<8>: toPPM encodes a 1×1 red canvas correctly",
    "[common][canvas][GCanvas][ppm]"
) {
    Gem::Geneva::GCanvas<8> c{
        std::make_tuple(std::size_t{1}, std::size_t{1}),
        std::make_tuple(1.f, 0.f, 0.f)
    };
    std::string ppm = c.toPPM();
    REQUIRE(ppm.find("1 1") != std::string::npos);
    REQUIRE(ppm.find("255") != std::string::npos);
    REQUIRE(ppm.find("255 0 0") != std::string::npos);
}

TEST_CASE(
    "GCanvas<8>: toPPM encodes a 1×1 black canvas as all zeros",
    "[common][canvas][GCanvas][ppm]"
) {
    Gem::Geneva::GCanvas<8> c{
        std::make_tuple(std::size_t{1}, std::size_t{1}),
        std::make_tuple(0.f, 0.f, 0.f)
    };
    REQUIRE(c.toPPM().find("0 0 0") != std::string::npos);
}

// --- GCanvas<8>: PPM loading ---

TEST_CASE(
    "GCanvas<8>: PPM string constructor parses 2×2 image correctly",
    "[common][canvas][GCanvas][ppm]"
) {
    Gem::Geneva::GCanvas<8> c{k2x2Ppm};
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

TEST_CASE(
    "GCanvas<8>: loadFromPPM handles comments and blank lines",
    "[common][canvas][GCanvas][ppm]"
) {
    const std::string ppm = "# comment\n"
                            "P3\n"
                            "\n"
                            "# another comment\n"
                            "1 1\n"
                            "255\n"
                            "128 64 32\n";
    Gem::Geneva::GCanvas<8> c;
    REQUIRE_NOTHROW(c.loadFromPPM(ppm));
    REQUIRE(c.getXDim() == 1);
    REQUIRE(c.getYDim() == 1);
}

TEST_CASE(
    "GCanvas<8>: toPPM then loadFromPPM round-trips extremal colors",
    "[common][canvas][GCanvas][ppm]"
) {
    Gem::Geneva::GCanvas<8> original{k2x2Ppm};
    std::string serialized = original.toPPM();
    Gem::Geneva::GCanvas<8> restored;
    restored.loadFromPPM(serialized);
    REQUIRE(restored.getXDim() == 2);
    REQUIRE(restored.getYDim() == 2);
    REQUIRE(restored[0][0].r == Approx(original[0][0].r));
    REQUIRE(restored[1][0].g == Approx(original[1][0].g));
    REQUIRE(restored[0][1].b == Approx(original[0][1].b));
    REQUIRE(restored[1][1].r == Approx(original[1][1].r));
}

// --- GCanvas<8>: PPM error cases ---

TEST_CASE(
    "GCanvas<8>: loadFromPPM throws on wrong magic number",
    "[common][canvas][GCanvas][ppm][errors]"
) {
    Gem::Geneva::GCanvas<8> c;
    REQUIRE_THROWS_AS(c.loadFromPPM("P6\n1 1\n255\n"), geneva_exception);
}

TEST_CASE(
    "GCanvas<8>: loadFromPPM throws on zero x-dimension",
    "[common][canvas][GCanvas][ppm][errors]"
) {
    Gem::Geneva::GCanvas<8> c;
    REQUIRE_THROWS_AS(c.loadFromPPM("P3\n0 1\n255\n"), geneva_exception);
}

TEST_CASE(
    "GCanvas<8>: loadFromPPM throws on zero y-dimension",
    "[common][canvas][GCanvas][ppm][errors]"
) {
    Gem::Geneva::GCanvas<8> c;
    REQUIRE_THROWS_AS(c.loadFromPPM("P3\n1 0\n255\n"), geneva_exception);
}

TEST_CASE(
    "GCanvas<8>: loadFromPPM throws on wrong color depth",
    "[common][canvas][GCanvas][ppm][errors]"
) {
    Gem::Geneva::GCanvas<8> c;
    REQUIRE_THROWS_AS(c.loadFromPPM("P3\n1 1\n127\n0 0 0\n"), geneva_exception);
}

TEST_CASE(
    "GCanvas<8>: loadFromPPM throws on too-few pixel values",
    "[common][canvas][GCanvas][ppm][errors]"
) {
    Gem::Geneva::GCanvas<8> c;
    // 2×2 needs 12 values; only 3 given
    REQUIRE_THROWS_AS(c.loadFromPPM("P3\n2 2\n255\n255 0 0\n"), geneva_exception);
}

// --- GCanvas<8>: diff ---

TEST_CASE("GCanvas<8>: diff of a canvas with itself is zero", "[common][canvas][GCanvas][diff]") {
    Gem::Geneva::GCanvas<8> c{
        std::make_tuple(std::size_t{4}, std::size_t{4}),
        std::make_tuple(0.5f, 0.3f, 0.7f)
    };
    REQUIRE(c.diff(c) == Approx(0.f));
}

TEST_CASE(
    "GCanvas<8>: diff of white vs black 1×1 canvas equals sqrt(3)",
    "[common][canvas][GCanvas][diff]"
) {
    auto dim = std::make_tuple(std::size_t{1}, std::size_t{1});
    Gem::Geneva::GCanvas<8> white{dim, std::make_tuple(1.f, 1.f, 1.f)};
    Gem::Geneva::GCanvas<8> black{dim, std::make_tuple(0.f, 0.f, 0.f)};
    REQUIRE(white.diff(black) == Approx(std::sqrt(3.f)));
}

TEST_CASE("GCanvas<8>: diff is symmetric", "[common][canvas][GCanvas][diff]") {
    auto dim = std::make_tuple(std::size_t{3}, std::size_t{3});
    Gem::Geneva::GCanvas<8> a{dim, std::make_tuple(1.f, 0.f, 0.f)};
    Gem::Geneva::GCanvas<8> b{dim, std::make_tuple(0.f, 1.f, 0.f)};
    REQUIRE(a.diff(b) == Approx(b.diff(a)));
}

TEST_CASE("GCanvas<8>: diff scales linearly with pixel count", "[common][canvas][GCanvas][diff]") {
    // 1×1 white vs black: sqrt(3); 1×2 should give 2*sqrt(3)
    Gem::Geneva::GCanvas<8> w1{
        std::make_tuple(std::size_t{1}, std::size_t{1}),
        std::make_tuple(1.f, 1.f, 1.f)
    };
    Gem::Geneva::GCanvas<8> b1{
        std::make_tuple(std::size_t{1}, std::size_t{1}),
        std::make_tuple(0.f, 0.f, 0.f)
    };
    float d1 = w1.diff(b1);
    Gem::Geneva::GCanvas<8> w2{
        std::make_tuple(std::size_t{1}, std::size_t{2}),
        std::make_tuple(1.f, 1.f, 1.f)
    };
    Gem::Geneva::GCanvas<8> b2{
        std::make_tuple(std::size_t{1}, std::size_t{2}),
        std::make_tuple(0.f, 0.f, 0.f)
    };
    REQUIRE(w2.diff(b2) == Approx(2.f * d1));
}

TEST_CASE("GCanvas<8>: diff throws on mismatched dimensions", "[common][canvas][GCanvas][diff]") {
    Gem::Geneva::GCanvas<8> a{
        std::make_tuple(std::size_t{2}, std::size_t{2}),
        std::make_tuple(0.f, 0.f, 0.f)
    };
    Gem::Geneva::GCanvas<8> b{
        std::make_tuple(std::size_t{3}, std::size_t{3}),
        std::make_tuple(0.f, 0.f, 0.f)
    };
    REQUIRE_THROWS_AS(a.diff(b), geneva_exception);
}

TEST_CASE("GCanvas8: operator- is equivalent to diff", "[common][canvas][GCanvas][diff]") {
    Gem::Geneva::GCanvas8 a{
        std::make_tuple(std::size_t{2}, std::size_t{2}),
        std::make_tuple(1.f, 0.f, 0.f)
    };
    Gem::Geneva::GCanvas8 b{
        std::make_tuple(std::size_t{2}, std::size_t{2}),
        std::make_tuple(0.f, 0.f, 1.f)
    };
    REQUIRE((a - b) == Approx(a.diff(b)));
}

// --- GCanvas<8>: addTriangle(t_cart) ---

// Helper: build a t_cart that covers the entire unit square plus margin.
// Triangle (0,0)-(3,0)-(0,3) contains every pixel of a 4×4 canvas.
// For a 4×4 canvas, pixel positions are (i+1)/4 ∈ {0.25,0.5,0.75,1.0}.
// The worst case (1.0,1.0): u=v=1/3, u+v=2/3 < 1 → inside. ✓
namespace {
Gem::Geneva::t_cart full_cover_triangle(float r, float g, float b, float a) {
    Gem::Geneva::t_cart t;
    t.tr_one = {0.f, 0.f};
    t.tr_two = {3.f, 0.f};
    t.tr_three = {0.f, 3.f};
    t.r = r;
    t.g = g;
    t.b = b;
    t.a = a;
    return t;
}
} // namespace

TEST_CASE(
    "GCanvas<8>: addTriangle(t_cart) with alpha=0 leaves canvas unchanged",
    "[common][canvas][GCanvas][triangle]"
) {
    Gem::Geneva::GCanvas<8> c{
        std::make_tuple(std::size_t{4}, std::size_t{4}),
        std::make_tuple(0.5f, 0.5f, 0.5f)
    };
    c.addTriangle(full_cover_triangle(1.f, 0.f, 0.f, 0.f));
    for(std::size_t x = 0; x < 4; ++x) {
        for(std::size_t y = 0; y < 4; ++y) {
            REQUIRE(c[x][y].r == Approx(0.5f));
        }
    }
}

TEST_CASE(
    "GCanvas<8>: addTriangle(t_cart) with alpha=1 fully overwrites covered pixels",
    "[common][canvas][GCanvas][triangle]"
) {
    // White canvas + fully-opaque red triangle covering all pixels.
    // Blend: new_g = 1 + 1*(0-1) = 0; new_b = 0.
    Gem::Geneva::GCanvas<8> c{
        std::make_tuple(std::size_t{4}, std::size_t{4}),
        std::make_tuple(1.f, 1.f, 1.f)
    };
    c.addTriangle(full_cover_triangle(1.f, 0.f, 0.f, 1.f));
    for(std::size_t x = 0; x < 4; ++x) {
        for(std::size_t y = 0; y < 4; ++y) {
            REQUIRE(c[x][y].r == Approx(1.f));
            REQUIRE(c[x][y].g == Approx(0.f));
            REQUIRE(c[x][y].b == Approx(0.f));
        }
    }
}

TEST_CASE(
    "GCanvas<8>: addTriangle(t_cart) with alpha=0.5 blends correctly",
    "[common][canvas][GCanvas][triangle]"
) {
    // Black canvas + 50%-opaque white triangle.
    // new_r = 0 + 0.5*(1-0) = 0.5
    Gem::Geneva::GCanvas<8> c{
        std::make_tuple(std::size_t{4}, std::size_t{4}),
        std::make_tuple(0.f, 0.f, 0.f)
    };
    c.addTriangle(full_cover_triangle(1.f, 1.f, 1.f, 0.5f));
    // Check the pixel at (0,0) — pos_f=(0.25,0.25), provably inside (u=v≈0.083)
    REQUIRE(c[0][0].r == Approx(0.5f));
    REQUIRE(c[0][0].g == Approx(0.5f));
    REQUIRE(c[0][0].b == Approx(0.5f));
}

TEST_CASE(
    "GCanvas<8>: addTriangle(t_cart) leaves pixels outside bounding box unchanged",
    "[common][canvas][GCanvas][triangle]"
) {
    // Triangle with all vertices at x > 0.8; pixels at i_x=0 (pos_f.x=0.25) are
    // entirely to the left of the bounding box and must remain untouched.
    Gem::Geneva::GCanvas<8> c{
        std::make_tuple(std::size_t{4}, std::size_t{4}),
        std::make_tuple(0.5f, 0.5f, 0.5f)
    };
    Gem::Geneva::t_cart t;
    t.tr_one = {0.85f, 0.85f};
    t.tr_two = {0.90f, 0.85f};
    t.tr_three = {0.87f, 0.95f};
    t.r = 1.f;
    t.g = 0.f;
    t.b = 0.f;
    t.a = 1.f;
    c.addTriangle(t);
    // Column 0 is to the left of all triangle vertices — must be gray
    for(std::size_t y = 0; y < 4; ++y) {
        REQUIRE(c[0][y].r == Approx(0.5f));
    }
}

TEST_CASE(
    "GCanvas<8>: addTriangles adds multiple triangles",
    "[common][canvas][GCanvas][triangle]"
) {
    // Two non-overlapping triangles (left vs right half), both red alpha=1.
    // After both: canvas should be all red everywhere.
    Gem::Geneva::GCanvas<8> c{
        std::make_tuple(std::size_t{4}, std::size_t{4}),
        std::make_tuple(0.f, 0.f, 1.f) // blue
    };
    Gem::Geneva::t_cart t1 = full_cover_triangle(1.f, 0.f, 0.f, 1.f);
    c.addTriangles(std::vector<Gem::Geneva::t_circle>{}); // verify empty is a no-op
    REQUIRE(c[0][0].b == Approx(1.f));                    // unchanged
    c.addTriangle(t1);
    REQUIRE(c[0][0].r == Approx(1.f));
    REQUIRE(c[0][0].b == Approx(0.f));
}

// --- GCanvas<8>: addTriangle(t_circle) ---

TEST_CASE(
    "GCanvas<8>: addTriangle(t_circle) colors pixels inside the derived triangle",
    "[common][canvas][GCanvas][triangle]"
) {
    // Circle-based triangle: center(0.5,0.5), radius=0.4, angles 0/0.25/0.5.
    // Derived cartesian vertices: (0.9,0.5), (0.5,0.9), (0.1,0.5).
    // For a 10×10 canvas, pixel(4,4) has pos_f=(0.5,0.5) which lies inside.
    Gem::Geneva::GCanvas<8> c{
        std::make_tuple(std::size_t{10}, std::size_t{10}),
        std::make_tuple(0.f, 0.f, 0.f) // black
    };
    Gem::Geneva::t_circle tc;
    tc.middle = {0.5f, 0.5f};
    tc.radius = 0.4f;
    tc.angle1 = 0.f;
    tc.angle2 = 0.25f;
    tc.angle3 = 0.5f;
    tc.r = 1.f;
    tc.g = 0.f;
    tc.b = 0.f;
    tc.a = 1.f;
    c.addTriangle(tc);
    // pixel (4,4) at pos_f=(0.5,0.5) must now be red
    REQUIRE(c[4][4].r == Approx(1.f));
    REQUIRE(c[4][4].g == Approx(0.f));
    REQUIRE(c[4][4].b == Approx(0.f));
}

#ifdef DEBUG
TEST_CASE(
    "GCanvas<8>: addTriangle(t_circle) throws in DEBUG on non-ascending angles",
    "[common][canvas][GCanvas][triangle]"
) {
    Gem::Geneva::GCanvas<8> c{
        std::make_tuple(std::size_t{4}, std::size_t{4}),
        std::make_tuple(0.f, 0.f, 0.f)
    };
    Gem::Geneva::t_circle tc;
    tc.middle = {0.5f, 0.5f};
    tc.radius = 0.3f;
    tc.r = 1.f;
    tc.g = 0.f;
    tc.b = 0.f;
    tc.a = 1.f;

    // angle2 <= angle1
    tc.angle1 = 0.3f;
    tc.angle2 = 0.2f;
    tc.angle3 = 0.8f;
    REQUIRE_THROWS_AS(c.addTriangle(tc), geneva_exception);

    // angle3 >= 1
    tc.angle1 = 0.1f;
    tc.angle2 = 0.3f;
    tc.angle3 = 1.0f;
    REQUIRE_THROWS_AS(c.addTriangle(tc), geneva_exception);

    // angle1 < 0
    tc.angle1 = -0.1f;
    tc.angle2 = 0.2f;
    tc.angle3 = 0.5f;
    REQUIRE_THROWS_AS(c.addTriangle(tc), geneva_exception);
}
#endif /* DEBUG */

// --- GCanvas<8>: getAverageColors ---

TEST_CASE(
    "GCanvas<8>: getAverageColors on uniform canvas returns that color",
    "[common][canvas][GCanvas]"
) {
    Gem::Geneva::GCanvas<8> c{
        std::make_tuple(std::size_t{4}, std::size_t{4}),
        std::make_tuple(0.5f, 0.25f, 0.75f)
    };
    auto [ar, ag, ab] = c.getAverageColors();
    REQUIRE(ar == Approx(0.5f));
    REQUIRE(ag == Approx(0.25f));
    REQUIRE(ab == Approx(0.75f));
}

TEST_CASE(
    "GCanvas<8>: getAverageColors of half-red half-blue 2×1 canvas",
    "[common][canvas][GCanvas]"
) {
    // 2×1 canvas: pixel(0,0)=red, pixel(1,0)=blue → avg=(0.5,0,0.5)
    Gem::Geneva::GCanvas<8> c{
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
    Gem::Geneva::GCanvas<8> c{
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
    Gem::Geneva::GCanvas8 c{
        std::make_tuple(std::size_t{3}, std::size_t{3}),
        std::make_tuple(0.5f, 0.5f, 0.5f)
    };
    REQUIRE(c.getColorDepth() == 8);
    REQUIRE((c - c) == Approx(0.f));
}

TEST_CASE("GCanvas8: PPM string constructor and round-trip", "[common][canvas][GCanvas8]") {
    Gem::Geneva::GCanvas8 c{k2x2Ppm};
    REQUIRE(c.getXDim() == 2);
    REQUIRE(c.getYDim() == 2);
    std::string ppm = c.toPPM();
    Gem::Geneva::GCanvas8 c2;
    c2.loadFromPPM(ppm);
    REQUIRE(c2[0][0].r == Approx(c[0][0].r));
}

TEST_CASE("GCanvas16: construction and color depth", "[common][canvas][GCanvas16]") {
    Gem::Geneva::GCanvas16 c{
        std::make_tuple(std::size_t{2}, std::size_t{2}),
        std::make_tuple(0.f, 0.f, 0.f)
    };
    REQUIRE(c.getColorDepth() == 16);
    REQUIRE(c.getNColors() == 65536);
}

TEST_CASE("GCanvas24: construction and color depth", "[common][canvas][GCanvas24]") {
    Gem::Geneva::GCanvas24 c{
        std::make_tuple(std::size_t{2}, std::size_t{2}),
        std::make_tuple(1.f, 1.f, 1.f)
    };
    REQUIRE(c.getColorDepth() == 24);
    REQUIRE(c.getMaxColor() == 16777215);
}
