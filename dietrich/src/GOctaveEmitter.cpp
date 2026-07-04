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
 * The Octave / MATLAB script file extension.
 *
 * @return The string ".m"
 */
std::string OctaveEmitter::fileExtension() const {
    return std::string(".m");
}

namespace {

/******************************************************************************/
// Helpers backing the Octave / MATLAB backend. The plotters expose their columns via
// the public const column<I>() accessor on GDataCollectorT; coordinates are emitted
// into Octave row-vector literals at full (round-trippable) precision through an
// EmitStream. Everything emitted stays inside the Octave-and-MATLAB COMMON CORE (base
// functions only -- no toolboxes / packages), so a plain `octave` can run the script.

/** @brief The plotter kind the Octave backend understands. */
enum class octKind { g2d, g2ed, g3d, g4d, hist1d, hist1i, hist2d };

/** @brief Classify a plotter for the Octave backend; throws for unsupported types
 *  (e.g. function plotters), directing the caller to the ROOT backend. */
octKind classifyOct(const GBasePlotter &p) {
    switch(p.plotSpec().kind) {
        case plotKind::graph_2d:     return octKind::g2d;
        case plotKind::graph_2d_err: return octKind::g2ed;
        case plotKind::graph_3d:     return octKind::g3d;
        case plotKind::graph_4d:     return octKind::g4d;
        case plotKind::hist_2d:      return octKind::hist2d;
        case plotKind::hist_1d:      return octKind::hist1d;
        case plotKind::hist_1i:      return octKind::hist1i;
        default:                     break;
    }
    throw geneva_exception(
        g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
        << "In OctaveEmitter::emitDocument(): Error!" << '\n'
        << "the Octave backend does not support " << p.getPlotterName()
        << "; use the ROOT backend for it" << '\n'
    );
}

/** @brief Whether a plotter is drawn into a 3-d Axes (plot3 / scatter3). */
bool isThreeDimensionalOct(octKind k) {
    return k == octKind::g3d || k == octKind::g4d;
}

/** @brief Escape a user string for a single-quoted Octave / MATLAB string literal: a
 *  literal apostrophe doubles to '', and embedded newlines (which cannot appear inside a
 *  single-line literal) become spaces. The result is always inserted between two `'`. */
std::string octaveEscape(const std::string &in) {
    std::string out;
    out.reserve(in.size());
    for(char c : in) {
        switch(c) {
            case '\'':           out += "''"; break;
            case '\n': case '\r': out += ' '; break;
            default:             out += c;    break;
        }
    }
    return out;
}

/** @brief Emit one numeric column as an Octave row-vector literal `[v0, v1, ...]` at full
 *  precision. Works for both float64 and int32 element types. */
template <typename T>
std::string octRow(const std::vector<T> &col) {
    EmitStream out; // NOLINT(cppcoreguidelines-init-variables)
    out << '[';
    for(std::size_t i = 0; i < col.size(); ++i) {
        out << (i == 0 ? "" : ", ") << col[i];
    }
    out << ']';
    return out.str();
}

/** @brief Emit a type-tagged column (float64 or int32) as an Octave row-vector literal. */
std::string octRowCol(const GPlotColumn &c) {
    return std::visit([](const auto *v) { return octRow(*v); }, c);
}

/** @brief Emit the Octave plotting call(s) for ONE plotter into the current subplot (a
 *  `hold on` is already in effect, so secondaries overlay). Data are written as inline
 *  row-vector literals; bin counts come from the plot spec. */
std::string octPlotCall(const GBasePlotter &p, octKind k) {
    EmitStream call; // NOLINT(cppcoreguidelines-init-variables)
    const std::string label = octaveEscape(p.plotLabel());
    // Columns in storage order (see plotSpec().columns). GGraph2ED's stored order is
    // (x, ex, y, ey).
    const auto cols = p.dataColumns();
    const GPlotSpec spec = p.plotSpec();
    switch(k) {
        case octKind::g2d: {
            call << "plot(" << octRowCol(cols[0]) << ", " << octRowCol(cols[1])
                 << ", '-o', 'DisplayName', '" << label << "');" << '\n';
        } break;
        case octKind::g2ed: {
            // Y-error bars only: a single errorbar() carrying BOTH x- and y-error is not
            // portable across base Octave and MATLAB, so the x-error column is omitted here.
            call << "errorbar(" << octRowCol(cols[0]) << ", " << octRowCol(cols[2]) << ", "
                 << octRowCol(cols[3]) << ", 'o', 'DisplayName', '" << label << "');" << '\n';
        } break;
        case octKind::g3d: {
            call << "plot3(" << octRowCol(cols[0]) << ", " << octRowCol(cols[1]) << ", "
                 << octRowCol(cols[2]) << ", 'DisplayName', '" << label << "');" << '\n';
        } break;
        case octKind::g4d: {
            // 3-d scatter coloured by the w-component, with a colourbar.
            call << "scatter3(" << octRowCol(cols[0]) << ", " << octRowCol(cols[1]) << ", "
                 << octRowCol(cols[2]) << ", 36, " << octRowCol(cols[3]) << ", 'filled');" << '\n'
                 << "colorbar;" << '\n';
        } break;
        case octKind::hist1d:
        case octKind::hist1i: {
            // A 1-d histogram of the raw samples (float64 or int32); same base call.
            call << "hist(" << octRowCol(cols[0]) << ", " << *spec.n_bins_x << ");" << '\n';
        } break;
        case octKind::hist2d: {
            // A base-only 2-d histogram: bin x and y with histc over linspace edges, sum
            // the counts with accumarray, and draw the grid with imagesc (hist3 is avoided
            // -- it needs the statistics toolbox / package).
            call << "_x = " << octRowCol(cols[0]) << "; _y = " << octRowCol(cols[1]) << ";"
                 << '\n'
                 << "_nbx = " << *spec.n_bins_x << "; _nby = " << *spec.n_bins_y << ";" << '\n'
                 << "_ex = linspace(min(_x), max(_x), _nbx + 1); "
                 << "_ey = linspace(min(_y), max(_y), _nby + 1);" << '\n'
                 << "[~, _bx] = histc(_x, _ex); [~, _by] = histc(_y, _ey);" << '\n'
                 << "_bx = min(max(_bx, 1), _nbx); _by = min(max(_by, 1), _nby);" << '\n'
                 << "_H = accumarray([_bx(:), _by(:)], 1, [_nbx, _nby]);" << '\n'
                 << "imagesc([min(_x) max(_x)], [min(_y) max(_y)], _H'); axis xy; colorbar;"
                 << '\n';
        } break;
    }
    return call.str();
}

} // anonymous namespace

/******************************************************************************/
/**
 * Emits a self-contained Octave / MATLAB (.m) script for the graph plotters (GGraph2D /
 * GGraph2ED / GGraph3D / GGraph4D) and the histogram plotters (GHistogram1D /
 * GHistogram2D). Any other plotter type (e.g. a function plotter) triggers a clear
 * geneva_exception. The script creates a figure and one `subplot` per pad, sets each
 * pad's axis labels and title and plots the pad's primary-and-secondary plotters into it
 * (with `hold on`). Only base Octave-and-MATLAB functions are used. It deliberately ends
 * WITHOUT a `print` / `saveas`, so it is terminal-agnostic: a validity harness (or the
 * caller) appends its own `print('-dpng', 'out.png')`.
 *
 * @param gpd The designer holding the plotters and canvas configuration
 * @return The complete Octave / MATLAB script as a string
 */
std::string OctaveEmitter::emitDocument(const GPlotDesigner &gpd) const {
    const std::size_t cols = gpd.c_x_div_;
    const std::size_t rows = gpd.c_y_div_;
    const std::size_t max_plots = cols * rows;

    EmitStream result; // NOLINT(cppcoreguidelines-init-variables)

    // Validate ALL plotters (and their secondaries) up front, so a partial script is
    // never produced for an unsupported plotter type. A 2-d and a 3-d plotter cannot
    // share one pad.
    for(const auto &p : gpd.plotters_cnt_) {
        const octKind k = classifyOct(*p);
        for(const auto &sp : p->secondaryPlotters()) {
            const octKind sk = classifyOct(*sp);
            if(isThreeDimensionalOct(k) != isThreeDimensionalOct(sk)) {
                throw geneva_exception(
                    g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                    << "In OctaveEmitter::emitDocument(): Error!" << '\n'
                    << "a 2-d and a 3-d graph cannot share a single Octave pad ("
                    << p->getPlotterName() << " vs. " << sp->getPlotterName() << ")" << '\n'
                );
            }
        }
    }

    if(gpd.plotters_cnt_.size() > max_plots) {
        glogger << "In OctaveEmitter::emitDocument() (Canvas label = \"" << gpd.getCanvasLabel()
                << "\":" << '\n'
                << "Warning! Found more plots than pads (" << gpd.plotters_cnt_.size() << " vs. "
                << max_plots << ")" << '\n'
                << "Some of the plots will be ignored" << '\n'
                << GWARNING;
    }

    // Preamble: the canvas title as a leading comment (sgtitle is version-fragile, so it is
    // not emitted as a call), then the figure.
    result << "% Canvas: " << octaveEscape(gpd.getCanvasLabel()) << '\n'
           << "figure();" << '\n' << '\n';

    // Per pad: select the subplot, set labels/title, then plot the pad's primary-and-
    // secondary plotters into it (secondaries overlay via the active `hold on`).
    for(const auto &[idx, p] :
        gpd.plotters_cnt_ | std::views::enumerate | std::views::take(max_plots)) {
        const std::size_t pad_idx = static_cast<std::size_t>(idx) + 1; // subplot indices start at 1

        const octKind k = classifyOct(*p);
        const bool three_d = isThreeDimensionalOct(k);

        result << "subplot(" << rows << ", " << cols << ", " << pad_idx << "); hold on;" << '\n';

        // Per-pad axis labels and title.
        result << "xlabel('" << octaveEscape(p->xAxisLabel()) << "');" << '\n'
               << "ylabel('" << octaveEscape(p->yAxisLabel()) << "');" << '\n';
        if(three_d) {
            result << "zlabel('" << octaveEscape(p->zAxisLabel()) << "');" << '\n';
        }
        result << "title('" << octaveEscape(p->plotLabel()) << "');" << '\n';

        // The primary plotter, then any secondary plotters overlaid in the same pad.
        result << octPlotCall(*p, k);
        for(const auto &sp : p->secondaryPlotters()) {
            result << octPlotCall(*sp, classifyOct(*sp));
        }
        result << '\n';
    }

    return result.str();
}


/******************************************************************************/
} /* namespace Gem::Dietrich */
