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
 * The gnuplot-script file extension.
 *
 * @return The string ".gp"
 */
std::string GnuplotEmitter::fileExtension() const {
    return std::string(".gp");
}

namespace {

/******************************************************************************/
// Shared helpers for reading a GPlotColumn (the type-tagged float64 / int32 view a
// plotter exposes via dataColumns()). These let every non-ROOT backend consume a
// column generically; an int32 column streams as integers and exports as a real
// int32 numpy array, while a double column is unchanged. Declared first so the
// gnuplot, matplotlib and data sections below can all use them.


/** @brief The double-typed value vector behind a column. Only valid for a column known
 *  to be float64 (the graph plotters, whose axes are always double); throws otherwise. */
const std::vector<double> &asDoubleColumn(const GPlotColumn &c) {
    return *std::get<const std::vector<double> *>(c);
}

/******************************************************************************/
// Helpers backing the gnuplot backend. A "dataset" is one plotter's inline data
// block (`x y ...` rows terminated by `e`); a "spec" is the leading
// `'-' with <style> title "..."` fragment naming that dataset inside a (s)plot
// command. The graph plotters expose their columns via the public const
// column<I>() accessor added to GDataCollectorT.

/** @brief The graph-plotter kind, used to group compatible plotters into one (s)plot. */
enum class graphKind { g2d, g2ed, g3d, g4d };

/** @brief Classify a plotter by its reported plotSpec().kind; throws if it is not one of
 *  the four graph plotters. */
graphKind classifyGraph(const GBasePlotter &p) {
    switch(p.plotSpec().kind) {
        case plotKind::graph_2d:     return graphKind::g2d;
        case plotKind::graph_2d_err: return graphKind::g2ed;
        case plotKind::graph_3d:     return graphKind::g3d;
        case plotKind::graph_4d:     return graphKind::g4d;
        default:                     break;
    }
    throw geneva_exception(
        g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
        << "In GnuplotEmitter::emitDocument(): Error!" << '\n'
        << "the gnuplot backend supports only graph plotters; use the ROOT backend for "
        << p.getPlotterName() << '\n'
    );
}

/** @brief Whether a (s)plot groups 2-d (plot) or 3-d (splot) datasets. */
bool isThreeDimensional(graphKind k) {
    return k == graphKind::g3d || k == graphKind::g4d;
}

/** @brief The `with <style> ...` spec for a plotter's dataset (the dataset source -- a named
 *  datablock -- is prepended by the caller). gnuplot cannot read inline `'-'` data inside a
 *  `set multiplot` block, so each dataset is emitted as a `$Dn` datablock and referenced here. */
std::string datasetSpec(const GBasePlotter &p, graphKind k) {
    EmitStream spec; // NOLINT(cppcoreguidelines-init-variables)
    const std::string title = gnuplotEscape(p.plotLabel());
    switch(k) {
        case graphKind::g2d:
            spec << "with linespoints title \"" << title << "\"";
            break;
        case graphKind::g2ed:
            spec << "with xyerrorbars title \"" << title << "\"";
            break;
        case graphKind::g3d:
            spec << "with linespoints title \"" << title << "\"";
            break;
        case graphKind::g4d:
            spec << "using 1:2:3:4 with points palette pointtype 7 title \"" << title << "\"";
            break;
    }
    return spec.str();
}

/** @brief A plotter's data rows (one point per line). Used as the body of a gnuplot `$Dn << EOD`
 *  datablock, so it carries no `e`/`EOD` terminator -- the caller adds it. */
std::string datasetRows(const GBasePlotter &p, graphKind k, const std::string &indent) {
    EmitStream rows; // NOLINT(cppcoreguidelines-init-variables)
    // Columns come back in storage order (see plotSpec().columns); the gnuplot row
    // layout is the same EXCEPT GGraph2ED, whose stored (x, ex, y, ey) is re-ordered
    // to the `x y xdelta ydelta` that the xyerrorbars style expects.
    // Graph plotters are always float64, so each column is read as a double vector.
    const auto cols = p.dataColumns();
    switch(k) {
        case graphKind::g2d: {
            const auto &x = asDoubleColumn(cols[0]);
            const auto &y = asDoubleColumn(cols[1]);
            for(std::size_t i = 0; i < x.size(); ++i) {
                rows << indent << x[i] << ' ' << y[i] << '\n';
            }
        } break;
        case graphKind::g2ed: {
            const auto &x  = asDoubleColumn(cols[0]);
            const auto &ex = asDoubleColumn(cols[1]);
            const auto &y  = asDoubleColumn(cols[2]);
            const auto &ey = asDoubleColumn(cols[3]);
            for(std::size_t i = 0; i < x.size(); ++i) {
                rows << indent << x[i] << ' ' << y[i] << ' ' << ex[i] << ' ' << ey[i] << '\n';
            }
        } break;
        case graphKind::g3d: {
            const auto &x = asDoubleColumn(cols[0]);
            const auto &y = asDoubleColumn(cols[1]);
            const auto &z = asDoubleColumn(cols[2]);
            for(std::size_t i = 0; i < x.size(); ++i) {
                rows << indent << x[i] << ' ' << y[i] << ' ' << z[i] << '\n';
            }
        } break;
        case graphKind::g4d: {
            const auto &x = asDoubleColumn(cols[0]);
            const auto &y = asDoubleColumn(cols[1]);
            const auto &z = asDoubleColumn(cols[2]);
            const auto &w = asDoubleColumn(cols[3]);
            for(std::size_t i = 0; i < x.size(); ++i) {
                rows << indent << x[i] << ' ' << y[i] << ' ' << z[i] << ' ' << w[i] << '\n';
            }
        } break;
    }
    return rows.str();
}

} // anonymous namespace

/******************************************************************************/
/**
 * Emits a gnuplot script for the graph plotters (GGraph2D / GGraph2ED / GGraph3D /
 * GGraph4D). Any other plotter type triggers a clear geneva_exception. The script
 * is terminal-agnostic (the caller / validity harness prepends `set terminal` and
 * `set output`): it lays the pads out as a `set multiplot` grid and, per pad, sets
 * the axis labels and title and emits a `plot`/`splot` with one inline dataset per
 * primary-or-secondary plotter sharing that pad.
 *
 * @param gpd The designer holding the graph plotters and canvas configuration
 * @return The complete gnuplot script as a string
 */
std::string GnuplotEmitter::emitDocument(const GPlotDesigner &gpd) const {
    const std::size_t cols = gpd.c_x_div_;
    const std::size_t rows = gpd.c_y_div_;
    const std::size_t max_plots = cols * rows;

    EmitStream result; // NOLINT(cppcoreguidelines-init-variables)

    // Validate ALL plotters (and their secondaries) up front, so a partial script is
    // never produced for an unsupported plotter type.
    for(const auto &p : gpd.plotters_cnt_) {
        const graphKind k = classifyGraph(*p);
        for(const auto &sp : p->secondaryPlotters()) {
            const graphKind sk = classifyGraph(*sp);
            if(isThreeDimensional(k) != isThreeDimensional(sk)) {
                throw geneva_exception(
                    g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                    << "In GnuplotEmitter::emitDocument(): Error!" << '\n'
                    << "a 2-d and a 3-d graph cannot share a single gnuplot pad ("
                    << p->getPlotterName() << " vs. " << sp->getPlotterName() << ")" << '\n'
                );
            }
        }
    }

    if(gpd.plotters_cnt_.size() > max_plots) {
        glogger << "In GnuplotEmitter::emitDocument() (Canvas label = \"" << gpd.getCanvasLabel()
                << "\":" << '\n'
                << "Warning! Found more plots than pads (" << gpd.plotters_cnt_.size() << " vs. "
                << max_plots << ")" << '\n'
                << "Some of the plots will be ignored" << '\n'
                << GWARNING;
    }

    const std::string &indent = gpd.indent();

    // gnuplot cannot read an inline `'-'` dataset from inside a `set multiplot` block (it warns
    // "Reading from '-' inside a multiplot not supported" and renders nothing), so every dataset is
    // emitted up front as a named `$Dn` datablock and referenced from the plot commands below.
    //
    // Pass 1 -- the datablocks, one per primary-or-secondary graph, in plot order.
    std::size_t db_idx = 0;
    {
        std::size_t n_plots = 0;
        for(const auto &p : gpd.plotters_cnt_) {
            if(n_plots++ >= max_plots) {
                break;
            }
            std::vector<const GBasePlotter *> pad;
            pad.push_back(p.get());
            for(const auto &sp : p->secondaryPlotters()) {
                pad.push_back(sp.get());
            }
            for(const auto *dp : pad) {
                result << "$D" << db_idx << " << EOD" << '\n'
                       << datasetRows(*dp, classifyGraph(*dp), "") << "EOD" << '\n';
                ++db_idx;
            }
        }
    }
    result << '\n';

    // The multiplot grid (rows = c_y_div, cols = c_x_div).
    result << "set multiplot layout " << rows << "," << cols << " title \""
           << gnuplotEscape(gpd.getCanvasLabel()) << "\"" << '\n' << '\n';

    // Pass 2 -- per pad: axis labels / title, then a plot/splot referencing the datablocks. The
    // datablock counter advances in the SAME order as pass 1, so $Dn lines up with its data.
    std::size_t db_ref = 0;
    std::size_t n_plots = 0;
    for(const auto &p : gpd.plotters_cnt_) {
        if(n_plots++ >= max_plots) {
            break;
        }

        const graphKind k = classifyGraph(*p);
        const bool three_d = isThreeDimensional(k);

        // Per-pad axis labels and title.
        result << indent << "set xlabel \"" << gnuplotEscape(p->xAxisLabel()) << "\"" << '\n'
               << indent << "set ylabel \"" << gnuplotEscape(p->yAxisLabel()) << "\"" << '\n';
        if(three_d) {
            result << indent << "set zlabel \"" << gnuplotEscape(p->zAxisLabel()) << "\"" << '\n';
        }
        result << indent << "set title \"" << gnuplotEscape(p->plotLabel()) << "\"" << '\n';

        // GGraph4D maps its w-component to a colour palette.
        if(k == graphKind::g4d) {
            result << indent << "set palette" << '\n';
        }

        // Collect this pad's datasets (primary first, then any secondary plotters).
        std::vector<const GBasePlotter *> pad;
        pad.push_back(p.get());
        for(const auto &sp : p->secondaryPlotters()) {
            pad.push_back(sp.get());
        }

        // The (s)plot command: one `$Dn <style>` reference per dataset, comma-separated.
        result << indent << (three_d ? "splot " : "plot ");
        for(std::size_t i = 0; i < pad.size(); ++i) {
            result << (i == 0 ? "" : ", ") << "$D" << db_ref << ' '
                   << datasetSpec(*pad[i], classifyGraph(*pad[i]));
            ++db_ref;
        }
        result << '\n' << '\n';
    }

    result << "unset multiplot" << '\n';

    return result.str();
}


/******************************************************************************/
} /* namespace Gem::Dietrich */
