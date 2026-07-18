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

#include "dietrich/GPlotDesigner.hpp"
#include "common/GCommonEnums.hpp"
#include "common/GCommonHelperFunctions.hpp"
#include "common/GCommonHelperFunctionsT.hpp"
#include "common/GCommonInterfaceT.hpp"
#include "common/GCommonMathHelperFunctionsT.hpp"
#include "common/GErrorStreamer.hpp"
#include "common/GExceptions.hpp"
#include "common/GExpectationChecksT.hpp"
#include "common/GLogger.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <ios>
#include <istream>
#include <limits>
#include <locale>
#include <memory>
#include <optional>
#include <ostream>
#include <ranges>
#include <sstream>
#include <tuple>
#include <utility>
#include <vector>
#include "dietrich/plotting/detail/GPlotDetail.hpp"


namespace Gem::Dietrich {

// The plotting library builds on common's facilities (logging, serialization helpers,
// exception types, make_member, EmitStream, ...); make them visible here without
// per-name qualification. This affects lookup only within Gem::Dietrich.
using namespace Gem::Common;
// Dietrich declares its own to_string(plotKind), which would otherwise shadow common's
// numeric to_string(...) for unqualified calls; merge common's overloads back in.
using Gem::Common::to_string;

/******************************************************************************/
/**
 * The matplotlib-script file extension.
 *
 * @return The string ".py"
 */
std::string MatplotlibEmitter::fileExtension() const {
    return std::string(".py");
}

namespace {

/******************************************************************************/
// Helpers backing the matplotlib backend. The plotters expose their columns via the
// public const column<I>() accessor on GDataCollectorT; coordinates are emitted into
// Python list literals at full (round-trippable) precision through an EmitStream.

/** @brief The plotter kind the matplotlib backend understands. */
enum class mplKind { g2d, g2ed, g3d, g4d, hist1d, hist1i, hist2d };

/** @brief Classify a plotter for the matplotlib backend; throws for unsupported types
 *  (e.g. function plotters), directing the caller to the ROOT backend. */
mplKind classifyMpl(const GBasePlotter &p) {
    switch(p.plotSpec().kind) {
        case plotKind::graph_2d:     return mplKind::g2d;
        case plotKind::graph_2d_err: return mplKind::g2ed;
        case plotKind::graph_3d:     return mplKind::g3d;
        case plotKind::graph_4d:     return mplKind::g4d;
        case plotKind::hist_2d:      return mplKind::hist2d;
        case plotKind::hist_1d:      return mplKind::hist1d;
        case plotKind::hist_1i:      return mplKind::hist1i;
        default:                     break;
    }
    throw geneva_exception(
        g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
        << "In MatplotlibEmitter::emitDocument(): Error!" << '\n'
        << "the matplotlib backend does not support " << p.getPlotterName()
        << "; use the ROOT backend for it" << '\n'
    );
}

/** @brief Whether a plotter is drawn into a 3-d (mplot3d) Axes. */
bool isThreeDimensionalMpl(mplKind k) {
    return k == mplKind::g3d || k == mplKind::g4d;
}

/** @brief Emit one numeric column as a Python list literal `[v0, v1, ...]` at full precision.
 *  Works for both float64 and int32 element types (an int column emits integer literals). */
template <typename T>
std::string pyList(const std::vector<T> &col) {
    EmitStream out; // NOLINT(cppcoreguidelines-init-variables)
    out << '[';
    for(std::size_t i = 0; i < col.size(); ++i) {
        out << (i == 0 ? "" : ", ") << col[i];
    }
    out << ']';
    return out.str();
}

/** @brief Emit a type-tagged column (float64 or int32) as a Python list literal. */
std::string pyListCol(const GPlotColumn &c) {
    return std::visit([](const auto *v) { return pyList(*v); }, c);
}

/** @brief Emit the matplotlib plotting call(s) for ONE plotter into the named Axes `ax`. The data are
 *  written as inline Python list literals; `ax` is a 2-d Axes for g2d/g2ed/hist*, a 3-d Axes for
 *  g3d/g4d. `fig` is the figure name (used for the g4d/hist2d colourbar). All statements are emitted at
 *  column 0 -- Python is whitespace-sensitive, so no indentation is applied. */
std::string mplPlotCall(
    const GBasePlotter &p,
    mplKind k,
    const std::string &ax,
    const std::string &fig
) {
    EmitStream call; // NOLINT(cppcoreguidelines-init-variables)
    const std::string label = pythonEscape(p.plotLabel());
    // Columns in storage order (see plotSpec().columns); histogram bin counts come
    // from the same spec. GGraph2ED's stored order is (x, ex, y, ey) -> errorbar wants
    // x, y with xerr/yerr.
    const auto cols = p.dataColumns();
    const GPlotSpec spec = p.plotSpec();
    switch(k) {
        case mplKind::g2d: {
            call << ax << ".plot(" << pyListCol(cols[0]) << ", "
                 << pyListCol(cols[1]) << ", marker=\"o\", label=\"" << label << "\")" << '\n';
        } break;
        case mplKind::g2ed: {
            call << ax << ".errorbar(" << pyListCol(cols[0]) << ", "
                 << pyListCol(cols[2]) << ", xerr=" << pyListCol(cols[1])
                 << ", yerr=" << pyListCol(cols[3]) << ", fmt=\"o\", label=\"" << label << "\")"
                 << '\n';
        } break;
        case mplKind::g3d: {
            call << ax << ".plot(" << pyListCol(cols[0]) << ", "
                 << pyListCol(cols[1]) << ", " << pyListCol(cols[2])
                 << ", label=\"" << label << "\")" << '\n';
        } break;
        case mplKind::g4d: {
            // 3-d scatter coloured by the w-component, with a colourbar.
            call << "_sc = " << ax << ".scatter(" << pyListCol(cols[0]) << ", "
                 << pyListCol(cols[1]) << ", " << pyListCol(cols[2])
                 << ", c=" << pyListCol(cols[3]) << ", cmap=\"viridis\", label=\"" << label
                 << "\")" << '\n'
                 << fig << ".colorbar(_sc, ax=" << ax << ")" << '\n';
        } break;
        case mplKind::hist1d:
        case mplKind::hist1i: {
            // A 1-d histogram of the raw samples (float64 or int32); same matplotlib call.
            // A histogram spec always carries its bin counts; the fallback is unreachable
            call << ax << ".hist(" << pyListCol(cols[0]) << ", bins="
                 << spec.n_bins_x.value_or(Gem::Common::DEFAULTNBINSGPD) << ", label=\"" << label
                 << "\")" << '\n';
        } break;
        case mplKind::hist2d: {
            // A histogram spec always carries its bin counts; the fallbacks are unreachable
            call << "_h = " << ax << ".hist2d(" << pyListCol(cols[0]) << ", "
                 << pyListCol(cols[1]) << ", bins=["
                 << spec.n_bins_x.value_or(Gem::Common::DEFAULTNBINSGPD) << ", "
                 << spec.n_bins_y.value_or(Gem::Common::DEFAULTNBINSGPD) << "])" << '\n'
                 << fig << ".colorbar(_h[3], ax=" << ax << ")" << '\n';
        } break;
    }
    return call.str();
}

} // anonymous namespace

/******************************************************************************/
/**
 * Emits a self-contained Python/matplotlib script for the graph plotters (GGraph2D /
 * GGraph2ED / GGraph3D / GGraph4D) and the histogram plotters (GHistogram1D /
 * GHistogram1I / GHistogram2D). Any other plotter type (e.g. a function plotter) triggers a clear
 * geneva_exception. The script selects the headless Agg backend, creates a `fig` and
 * one Axes per pad (a 3-d Axes for the 3-d graphs), sets each pad's axis labels and
 * title and plots the pad's primary-and-secondary plotters into it. It deliberately
 * ends WITHOUT `fig.savefig(...)`, so it is terminal-agnostic: a validity harness (or
 * the caller) appends its own `fig.savefig('out.png')`.
 *
 * @param gpd The designer holding the plotters and canvas configuration
 * @return The complete matplotlib (Python) script as a string
 */
std::string MatplotlibEmitter::emitDocument(const GPlotDesigner &gpd) const {
    const std::size_t cols = gpd.c_x_div_;
    const std::size_t rows = gpd.c_y_div_;
    const std::size_t max_plots = cols * rows;

    EmitStream result; // NOLINT(cppcoreguidelines-init-variables)

    // Validate ALL plotters (and their secondaries) up front, so a partial script is
    // never produced for an unsupported plotter type. A 2-d and a 3-d plotter cannot
    // share one pad (one is a flat Axes, the other an mplot3d Axes).
    for(const auto &p : gpd.plotters_cnt_) {
        const mplKind k = classifyMpl(*p);
        for(const auto &sp : p->secondaryPlotters()) {
            const mplKind sk = classifyMpl(*sp);
            if(isThreeDimensionalMpl(k) != isThreeDimensionalMpl(sk)) {
                throw geneva_exception(
                    g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                    << "In MatplotlibEmitter::emitDocument(): Error!" << '\n'
                    << "a 2-d and a 3-d graph cannot share a single matplotlib pad ("
                    << p->getPlotterName() << " vs. " << sp->getPlotterName() << ")" << '\n'
                );
            }
        }
    }

    if(gpd.plotters_cnt_.size() > max_plots) {
        glogger << "In MatplotlibEmitter::emitDocument() (Canvas label = \"" << gpd.getCanvasLabel()
                << "\":" << '\n'
                << "Warning! Found more plots than pads (" << gpd.plotters_cnt_.size() << " vs. "
                << max_plots << ")" << '\n'
                << "Some of the plots will be ignored" << '\n'
                << GWARNING;
    }

    // NOTE: Python is whitespace-sensitive, so every statement below is emitted at column 0 (the
    // GPlotDesigner indentation setting does not apply to a Python script).

    // Script preamble: select the headless Agg backend BEFORE importing pyplot, then create the
    // figure. Pads are added individually via fig.add_subplot(rows, cols, idx[, projection='3d']) so a
    // 3-d pad gets an mplot3d Axes while 2-d pads stay flat -- a single plt.subplots() grid cannot mix
    // the two.
    result << "import matplotlib" << '\n'
           << "matplotlib.use(\"Agg\")" << '\n'
           << "import matplotlib.pyplot as plt" << '\n'
           << "from mpl_toolkits.mplot3d import Axes3D  # noqa: F401 (registers the 3d projection)"
           << '\n' << '\n'
           << "fig = plt.figure(figsize=(" << (cols == 0 ? 1 : cols) * 5 << ", "
           << (rows == 0 ? 1 : rows) * 4 << "))" << '\n'
           << "fig.suptitle(\"" << pythonEscape(gpd.getCanvasLabel()) << "\")" << '\n' << '\n';

    // Per pad: create the Axes with the right projection, set labels/title, then plot the pad's
    // primary-and-secondary plotters into it (secondaries overlay on the same Axes).
    for(const auto &[idx, p] :
        gpd.plotters_cnt_ | std::views::enumerate | std::views::take(max_plots)) {
        const std::size_t pad_idx = static_cast<std::size_t>(idx) + 1; // matplotlib subplot indices start at 1

        const mplKind k = classifyMpl(*p);
        const bool three_d = isThreeDimensionalMpl(k);

        result << "ax = fig.add_subplot(" << rows << ", " << cols << ", " << pad_idx
               << (three_d ? ", projection=\"3d\"" : "") << ")" << '\n';

        // Per-pad axis labels and title.
        result << "ax.set_xlabel(\"" << pythonEscape(p->xAxisLabel()) << "\")" << '\n'
               << "ax.set_ylabel(\"" << pythonEscape(p->yAxisLabel()) << "\")" << '\n';
        if(three_d) {
            result << "ax.set_zlabel(\"" << pythonEscape(p->zAxisLabel()) << "\")" << '\n';
        }
        result << "ax.set_title(\"" << pythonEscape(p->plotLabel()) << "\")" << '\n';

        // The primary plotter, then any secondary plotters overlaid on the same Axes.
        result << mplPlotCall(*p, k, "ax", "fig");
        for(const auto &sp : p->secondaryPlotters()) {
            result << mplPlotCall(*sp, classifyMpl(*sp), "ax", "fig");
        }
        result << '\n';
    }

    result << "fig.tight_layout()" << '\n';

    return result.str();
}


/******************************************************************************/
} /* namespace Gem::Dietrich */
