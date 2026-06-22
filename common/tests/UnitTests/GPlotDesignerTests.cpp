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

/******************************************************************************/
// The matplotlib backend emits a headless, Agg-selected Python script for a GGraph2D,
// with escaped labels and an axes plot call.
TEST_CASE("matplotlib backend emits a headless plot script for a GGraph2D", "[plotting]") {
    auto g = std::make_shared<GGraph2D>();
    (*g) & std::tuple<double, double>(1.0, 2.0);
    (*g) & std::tuple<double, double>(3.0, 4.0);
    g->setPlotLabel(std::string("a\"b"));        // an embedded double quote (must be escaped)
    g->setXAxisLabel(std::string("the x axis"));

    GPlotDesigner gpd("matplotlib graphs", 1, 1);
    gpd.setPlotBackend(plotBackend::MATPLOTLIB);
    gpd.registerPlotter(g);

    const std::string s = gpd.plot();

    // The script selects the headless Agg backend BEFORE importing pyplot, builds a figure with a
    // subplot, and plots into the axes -- but never calls savefig (terminal-agnostic).
    CHECK(s.find("matplotlib.use(\"Agg\")") != std::string::npos);
    CHECK(s.find("add_subplot(") != std::string::npos);
    CHECK(s.find(".plot(") != std::string::npos);
    CHECK(s.find("savefig") == std::string::npos);
    // The data values are present in the emitted list literals.
    CHECK(s.find("1") != std::string::npos);
    // Labels are python-escaped (a\"b), never a bare a"b that closes the literal early.
    CHECK(s.find("a\\\"b") != std::string::npos);
    CHECK(s.find("the x axis") != std::string::npos);
}

/******************************************************************************/
// The matplotlib backend uses a 3-d projection for a GGraph3D.
TEST_CASE("matplotlib backend uses a 3d projection for a GGraph3D", "[plotting]") {
    auto g = std::make_shared<GGraph3D>();
    (*g) & std::tuple<double, double, double>(1.0, 2.0, 3.0);

    GPlotDesigner gpd("matplotlib 3d", 1, 1);
    gpd.setPlotBackend(plotBackend::MATPLOTLIB);
    gpd.registerPlotter(g);

    const std::string s = gpd.plot();
    CHECK(s.find("projection=\"3d\"") != std::string::npos);
    CHECK(s.find("set_zlabel(") != std::string::npos);
}

/******************************************************************************/
// The matplotlib backend emits ax.hist / ax.hist2d for the histogram plotters.
TEST_CASE("matplotlib backend emits hist calls for histograms", "[plotting]") {
    {
        auto h = std::make_shared<GHistogram1D>(7, 0.0, 1.0);
        (*h) & 0.25;
        (*h) & 0.75;
        GPlotDesigner gpd("matplotlib hist1d", 1, 1);
        gpd.setPlotBackend(plotBackend::MATPLOTLIB);
        gpd.registerPlotter(h);

        const std::string s = gpd.plot();
        // A 1-d histogram is a flat (non-3d) Axes with an ax.hist(...) call honouring the bin count.
        CHECK(s.find(".hist(") != std::string::npos);
        CHECK(s.find("bins=7") != std::string::npos);
        CHECK(s.find("projection=\"3d\"") == std::string::npos);
    }
    {
        auto h = std::make_shared<GHistogram2D>(5, 3, 0.0, 1.0, 0.0, 1.0);
        (*h) & std::tuple<double, double>(0.25, 0.5);
        GPlotDesigner gpd("matplotlib hist2d", 1, 1);
        gpd.setPlotBackend(plotBackend::MATPLOTLIB);
        gpd.registerPlotter(h);

        const std::string s = gpd.plot();
        // A 2-d histogram uses ax.hist2d(...) with the per-axis bin counts and a colourbar.
        CHECK(s.find(".hist2d(") != std::string::npos);
        CHECK(s.find("bins=[5, 3]") != std::string::npos);
        CHECK(s.find("colorbar(") != std::string::npos);
    }
}

/******************************************************************************/
// The matplotlib backend still rejects function plotters (deferred to a later pass).
TEST_CASE("matplotlib backend throws for function plotters", "[plotting]") {
    const std::tuple<double, double> rx(-1., 1.);
    auto f = std::make_shared<GFunctionPlotter1D>("sin(x)", rx);
    GPlotDesigner gpd("matplotlib func", 1, 1);
    gpd.setPlotBackend(plotBackend::MATPLOTLIB);
    gpd.registerPlotter(f);
    CHECK_THROWS(gpd.plot());
}

/******************************************************************************/
// The DATA backend in CSV mode exports the raw series data: a `# series` header comment,
// a column-name row and the data rows for a GGraph2D (NOT a rendered plot).
TEST_CASE("data backend (CSV) emits the raw series data for a GGraph2D", "[plotting]") {
    auto g = std::make_shared<GGraph2D>();
    (*g) & std::tuple<double, double>(1.0, 2.0);
    (*g) & std::tuple<double, double>(3.0, 4.0);
    g->setPlotLabel(std::string("my series"));

    GPlotDesigner gpd("data csv", 1, 1);
    gpd.setDataFormat(dataFormat::CSV);
    gpd.registerPlotter(g);

    const std::string s = gpd.plot();

    // A leading canvas comment records the title and the pad grid for an external renderer.
    CHECK(s.find("# canvas: \"data csv\" c_x_div=1 c_y_div=1") != std::string::npos);
    // The section header comment names the series, its class name, canonical plot kind,
    // (spec) role, columns and its pad / overlay placement.
    CHECK(s.find("# series 0: \"my series\" kind=GGraph2D plotkind=graph_2d role=xy "
                 "columns=x,y pad=0 secondary=0") != std::string::npos);
    // The column-name header row.
    CHECK(s.find("x,y") != std::string::npos);
    // The data rows, full precision, dot decimal separator.
    CHECK(s.find("1,2") != std::string::npos);
    CHECK(s.find("3,4") != std::string::npos);
    // No rendering / script content leaks into the data export.
    CHECK(s.find("import") == std::string::npos);
    CHECK(s.find("TCanvas") == std::string::npos);
}

/******************************************************************************/
// GPlotSpec: a GGraph2D reports kind==graph_2d with columns {"x","y"}, and its
// toJson() carries the kind string and the (escaped) label.
TEST_CASE("GGraph2D reports a graph_2d GPlotSpec", "[plotting]") {
    GGraph2D g;
    g.setPlotLabel(std::string("a\"b")); // an embedded double quote (must be escaped in JSON)
    g.setXAxisLabel(std::string("the x axis"));

    const GPlotSpec spec = g.plotSpec();
    CHECK(spec.kind == plotKind::graph_2d);
    CHECK(spec.role == std::string("xy"));
    CHECK(spec.columns == std::vector<std::string>({"x", "y"}));
    CHECK(spec.name == std::string("a\"b"));
    CHECK(spec.x_label == std::string("the x axis"));
    CHECK_FALSE(spec.n_bins_x.has_value());

    const std::string j = spec.toJson();
    // The kind string is present.
    CHECK(j.find("graph_2d") != std::string::npos);
    // The embedded quote is JSON-escaped (a\"b), never a bare a"b closing the string early.
    CHECK(j.find("a\\\"b") != std::string::npos);
    // The column names appear in the JSON.
    CHECK(j.find("\"x\"") != std::string::npos);
    CHECK(j.find("\"y\"") != std::string::npos);
}

/******************************************************************************/
// GPlotSpec: a GHistogram1D reports kind==hist_1d and carries its bin count in n_bins_x.
TEST_CASE("GHistogram1D reports a hist_1d GPlotSpec with n_bins_x", "[plotting]") {
    GHistogram1D h(7, 0.0, 1.0);

    const GPlotSpec spec = h.plotSpec();
    CHECK(spec.kind == plotKind::hist_1d);
    CHECK(spec.role == std::string("distribution"));
    REQUIRE(spec.n_bins_x.has_value());
    CHECK(*spec.n_bins_x == 7);

    const std::string j = spec.toJson();
    CHECK(j.find("hist_1d") != std::string::npos);
    CHECK(j.find("\"n_bins_x\": 7") != std::string::npos);
}

/******************************************************************************/
// The DATA backend in NPZ mode produces a non-empty binary numpy .npz archive: the bytes
// begin with the ZIP local-file-header signature "PK\x03\x04".
TEST_CASE("data backend (NPZ) emits a non-empty ZIP/.npz archive", "[plotting]") {
    auto g = std::make_shared<GGraph2D>();
    (*g) & std::tuple<double, double>(1.0, 2.0);
    (*g) & std::tuple<double, double>(3.0, 4.0);

    GPlotDesigner gpd("data npz", 1, 1);
    gpd.setDataFormat(dataFormat::NPZ);
    gpd.registerPlotter(g);

    const std::string s = gpd.plot();

    REQUIRE(s.size() > 4);
    // The .npz is an uncompressed ZIP: it begins with the local file header signature.
    CHECK(s.compare(0, 4, std::string("PK\x03\x04", 4)) == 0);
    // It is a real archive: an end-of-central-directory record ("PK\x05\x06") is present.
    CHECK(s.find(std::string("PK\x05\x06", 4)) != std::string::npos);
    // The .npy member magic and the manifest member name are embedded.
    CHECK(s.find(std::string("\x93NUMPY", 6)) != std::string::npos);
    CHECK(s.find("manifest.json") != std::string::npos);
}

/******************************************************************************/
// dataColumns(): a plotter reports type-tagged (float64 / int32) views of its columns in
// storage order; a dataless plotter reports an empty list.
TEST_CASE("dataColumns reports columns generically in storage order", "[plotting]") {
    // GGraph2ED stores four float64 columns (x, ex, y, ey) -- the tricky reorder case.
    GGraph2ED g;
    g & std::tuple<double, double, double, double>(1.0, 0.1, 2.0, 0.2);
    g & std::tuple<double, double, double, double>(3.0, 0.3, 4.0, 0.4);

    const auto cols = g.dataColumns();
    REQUIRE(cols.size() == 4); // matches plotSpec().columns {"x","ex","y","ey"}
    CHECK(g.plotSpec().columns.size() == cols.size());
    // Each column is the float64 alternative of the type-tagged variant.
    const auto *x = std::get<const std::vector<double> *>(cols[0]);
    REQUIRE(x->size() == 2);
    CHECK((*x)[0] == 1.0); // x
    CHECK((*std::get<const std::vector<double> *>(cols[1]))[0] == 0.1); // ex
    CHECK((*std::get<const std::vector<double> *>(cols[2]))[0] == 2.0); // y
    CHECK((*std::get<const std::vector<double> *>(cols[3]))[1] == 0.4); // ey

    // The integer histogram now exports its samples as a TRUE int32 column.
    GHistogram1I hi(5, 0.0, 10.0);
    hi & std::int32_t(2);
    hi & std::int32_t(7);
    const auto icols = hi.dataColumns();
    REQUIRE(icols.size() == 1);
    const auto *iv = std::get<const std::vector<std::int32_t> *>(icols[0]); // int32 alternative
    REQUIRE(iv->size() == 2);
    CHECK((*iv)[0] == 2);
    CHECK((*iv)[1] == 7);

    // A function plotter holds no sample columns at all.
    GFunctionPlotter1D fp(std::string("x^2"), std::tuple<double, double>(-1.0, 1.0));
    CHECK(fp.dataColumns().empty());
}

/******************************************************************************/
// makePlotter(): the inverse of plotSpec(). A plotter -> spec -> makePlotter round-trip
// reproduces the same plotSpec() (kind, role, labels, columns, bins).
TEST_CASE("makePlotter reconstructs a plotter from its GPlotSpec", "[plotting]") {
    SECTION("graph round-trips kind/labels/columns") {
        GGraph2ED orig;
        orig.setPlotLabel(std::string("err series"));
        orig.setXAxisLabel(std::string("xx"));
        orig.setYAxisLabel(std::string("yy"));
        orig.setDrawingArguments(std::string("AP"));

        const GPlotSpec spec = orig.plotSpec();
        const auto rebuilt = makePlotter(spec);
        REQUIRE(rebuilt != nullptr);

        const GPlotSpec rspec = rebuilt->plotSpec();
        CHECK(rspec.kind == plotKind::graph_2d_err);
        CHECK(rspec.role == spec.role);
        CHECK(rspec.name == std::string("err series"));
        CHECK(rspec.x_label == std::string("xx"));
        CHECK(rspec.y_label == std::string("yy"));
        CHECK(rspec.drawing_args == std::string("AP"));
        CHECK(rspec.columns == spec.columns);
        // The rebuilt plotter is empty (no data carried by the spec).
        CHECK(std::get<const std::vector<double> *>(rebuilt->dataColumns()[0])->empty());
    }

    SECTION("histogram round-trips its bin counts") {
        GHistogram2D orig(6, 8);
        const GPlotSpec spec = orig.plotSpec();
        const auto rebuilt = makePlotter(spec);

        const GPlotSpec rspec = rebuilt->plotSpec();
        CHECK(rspec.kind == plotKind::hist_2d);
        REQUIRE(rspec.n_bins_x.has_value());
        REQUIRE(rspec.n_bins_y.has_value());
        CHECK(*rspec.n_bins_x == 6);
        CHECK(*rspec.n_bins_y == 8);
    }

    SECTION("function and integer-histogram kinds cannot be rebuilt from a spec") {
        GPlotSpec fspec(plotKind::function_1d);
        CHECK_THROWS(makePlotter(fspec));
        GPlotSpec ispec(plotKind::hist_1i);
        CHECK_THROWS(makePlotter(ispec));
    }
}
