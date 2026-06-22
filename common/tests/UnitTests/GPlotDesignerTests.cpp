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
 * Regression tests for the GPlotDesigner ROOT-macro code generator. These pin the
 * emission-correctness fixes from the 2026-06-21 review (see
 * prompts/2026-06-21-gplotdesigner-root-review.md) by asserting on the generated
 * macro text directly -- no ROOT installation required. The complementary
 * "does ROOT actually accept the macro" check is wired into ctest as
 * GPlotDesignerRootValidity (only when root + xvfb-run are available).
 ********************************************************************************/

#include <catch2/catch_test_macros.hpp>

#include <string>
#include <tuple>

#include "common/GPlotDesigner.hpp"

using namespace Gem::Common;

namespace {
/** Concatenate a plotter's three emission sections for substring assertions. */
std::string emit(const GBasePlotter &p) {
    return p.headerData("") + p.bodyData("") + p.footerData("");
}
} // namespace

/******************************************************************************/
// BUG-1: user strings must be escaped before they go into a ROOT C-string literal.
TEST_CASE("GPlotDesigner escapes user strings in emitted ROOT literals", "[plotting]") {
    GGraph2D g;
    g & std::tuple<double, double>(1.0, 2.0);
    g.setXAxisLabel(std::string("a\"b"));         // an embedded double quote
    g.setPlotLabel(std::string("line1\nline2"));   // an embedded newline

    const std::string s = emit(g);

    // The quote is backslash-escaped (a\"b), never a bare a"b that closes the literal early.
    CHECK(s.find("a\\\"b") != std::string::npos);
    // The newline is emitted as the two characters backslash-n, not a raw line break.
    CHECK(s.find("line1\\nline2") != std::string::npos);
    CHECK(s.find("line1\nline2") == std::string::npos);
}

/******************************************************************************/
// BUG-3: coordinates are emitted at full (round-trippable) precision, not the
// default 6 significant digits, and locale-independently.
TEST_CASE("GPlotDesigner emits full-precision coordinates", "[plotting]") {
    GGraph2D g;
    g & std::tuple<double, double>(1.0 / 3.0, 2.0 / 3.0);

    const std::string s = emit(g);

    // 6-significant-digit default would render 0.333333; full precision keeps many more digits.
    CHECK(s.find("3333333333") != std::string::npos);
    // Locale-independent: the decimal separator is a dot, never a comma.
    CHECK(s.find("0,33333") == std::string::npos);
}

/******************************************************************************/
// BUG-2: the GGraph3D poly-line must receive every data point, including the first
// (the pre-fix loop started at begin()+1, dropping point 0 and leaving an origin vertex).
TEST_CASE("GGraph3D poly-line emits every point including the first", "[plotting]") {
    GGraph3D g;
    g.setDrawLines(true);
    g & std::tuple<double, double, double>(1., 1., 1.);
    g & std::tuple<double, double, double>(2., 2., 2.);
    g & std::tuple<double, double, double>(3., 3., 3.);

    const std::string s = g.footerData("");

    const std::string::size_type poly = s.find("TPolyLine3D");
    REQUIRE(poly != std::string::npos);

    // Count the poly-line's SetPoint() calls (those after the TPolyLine3D declaration).
    std::size_t n = 0;
    for(std::string::size_type pos = s.find("->SetPoint(", poly); pos != std::string::npos;
        pos = s.find("->SetPoint(", pos + 1)) {
        ++n;
    }
    CHECK(n == 3); // all three points (pre-fix: only two)
    // Index 0 is the FIRST data point (1,1,1), not the second.
    CHECK(s.find("->SetPoint(0, 1, 1, 1)", poly) != std::string::npos);
}

/******************************************************************************/
// BUG-5/6: function plotters emit a QUOTED draw option, and the 2D plotter honours
// its configured drawing arguments (pre-fix it emitted an unquoted, and for 2D empty, option).
TEST_CASE("function plotters emit a quoted, honoured draw option", "[plotting]") {
    const std::tuple<double, double> rx(-1., 1.);
    const std::tuple<double, double> ry(-1., 1.);

    GFunctionPlotter2D f2("x*y", rx, ry);
    f2.setDrawingArguments("surf1");
    CHECK(f2.footerData("").find("->Draw(\"surf1\")") != std::string::npos);

    GFunctionPlotter1D f1("sin(x)", rx);
    f1.setDrawingArguments("L");
    CHECK(f1.footerData("").find("->Draw(\"L\")") != std::string::npos);
}

/******************************************************************************/
// BUG-4: an auto-ranged histogram with no data must throw (not dereference end()).
TEST_CASE("auto-ranged histogram with no data throws instead of UB", "[plotting]") {
    GHistogram1D h(10); // 10 bins, no data added -> default min==max -> auto-range path
    CHECK_THROWS(h.headerData(""));
}

/******************************************************************************/
// The gnuplot backend emits a valid gnuplot script for graph plotters.
TEST_CASE("gnuplot backend emits a multiplot script for a GGraph2D", "[plotting]") {
    auto g = std::make_shared<GGraph2D>();
    (*g) & std::tuple<double, double>(1.0, 2.0);
    (*g) & std::tuple<double, double>(3.0, 4.0);
    g->setPlotLabel(std::string("a\"b"));        // an embedded double quote (must be escaped)
    g->setXAxisLabel(std::string("the x axis"));

    GPlotDesigner gpd("gnuplot graphs", 1, 1);
    gpd.setPlotBackend(plotBackend::GNUPLOT);
    gpd.registerPlotter(g);

    const std::string s = gpd.plot();

    // The multiplot grid is present; the data is a named datablock (NOT an inline '-' inside multiplot,
    // which gnuplot cannot read) referenced by a 2-d plot command.
    CHECK(s.find("set multiplot") != std::string::npos);
    CHECK(s.find("$D0 << EOD") != std::string::npos);
    CHECK(s.find("plot $D0 ") != std::string::npos);
    CHECK(s.find("plot '-'") == std::string::npos);
    CHECK(s.find("unset multiplot") != std::string::npos);
    // The data rows are present (in the datablock).
    CHECK(s.find("1 2") != std::string::npos);
    CHECK(s.find("3 4") != std::string::npos);
    // Labels are gnuplot-escaped (a\"b), never a bare a"b that closes the literal early.
    CHECK(s.find("a\\\"b") != std::string::npos);
    CHECK(s.find("the x axis") != std::string::npos);
}

/******************************************************************************/
// The gnuplot backend uses splot for a GGraph3D.
TEST_CASE("gnuplot backend uses splot for a GGraph3D", "[plotting]") {
    auto g = std::make_shared<GGraph3D>();
    (*g) & std::tuple<double, double, double>(1.0, 2.0, 3.0);

    GPlotDesigner gpd("gnuplot 3d", 1, 1);
    gpd.setPlotBackend(plotBackend::GNUPLOT);
    gpd.registerPlotter(g);

    const std::string s = gpd.plot();
    CHECK(s.find("$D0 << EOD") != std::string::npos);
    CHECK(s.find("splot $D0 ") != std::string::npos);
    CHECK(s.find("1 2 3") != std::string::npos);
}

/******************************************************************************/
// The gnuplot backend rejects non-graph plotters (histograms, functions).
TEST_CASE("gnuplot backend throws for non-graph plotters", "[plotting]") {
    {
        auto h = std::make_shared<GHistogram1D>(10, 0.0, 1.0);
        (*h) & 0.5;
        GPlotDesigner gpd("gnuplot hist", 1, 1);
        gpd.setPlotBackend(plotBackend::GNUPLOT);
        gpd.registerPlotter(h);
        CHECK_THROWS(gpd.plot());
    }
    {
        const std::tuple<double, double> rx(-1., 1.);
        auto f = std::make_shared<GFunctionPlotter1D>("sin(x)", rx);
        GPlotDesigner gpd("gnuplot func", 1, 1);
        gpd.setPlotBackend(plotBackend::GNUPLOT);
        gpd.registerPlotter(f);
        CHECK_THROWS(gpd.plot());
    }
}
