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
#include <vector>

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Common::GMarker<short>)                        // NOLINT
BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Common::GMarker<std::int32_t>)                 // NOLINT
BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Common::GMarker<std::uint32_t>)                // NOLINT
BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Common::GMarker<float>)                        // NOLINT
BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Common::GMarker<double>)                       // NOLINT
BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Common::GDecoratorContainer_2D<short>)         // NOLINT
BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Common::GDecoratorContainer_2D<std::int32_t>)  // NOLINT
BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Common::GDecoratorContainer_2D<std::uint32_t>) // NOLINT
BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Common::GDecoratorContainer_2D<float>)         // NOLINT
BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Common::GDecoratorContainer_2D<double>)        // NOLINT
BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Common::GDecoratorContainer_3D<short>)         // NOLINT
BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Common::GDecoratorContainer_3D<std::int32_t>)  // NOLINT
BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Common::GDecoratorContainer_3D<std::uint32_t>) // NOLINT
BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Common::GDecoratorContainer_3D<float>)         // NOLINT
BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Common::GDecoratorContainer_3D<double>)        // NOLINT
BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Common::GHistogram1D)                          // NOLINT
BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Common::GHistogram1I)                          // NOLINT
BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Common::GHistogram2D)                          // NOLINT
BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Common::GGraph2D)                              // NOLINT
BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Common::GGraph2ED)                             // NOLINT
BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Common::GGraph3D)                              // NOLINT
BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Common::GGraph4D)                              // NOLINT
BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Common::GFunctionPlotter1D)                    // NOLINT
BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Common::GFunctionPlotter2D)                    // NOLINT
BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Common::GPlotDesigner)                         // NOLINT

namespace Gem::Common {

namespace {

/******************************************************************************/
/**
 * A locale-independent, full-round-trip-precision string builder for emitted ROOT macro text.
 * Every coordinate value must be emitted through such a stream: a default std::ostringstream prints
 * only 6 significant digits (lossy for optimization traces) and honours the global locale (a
 * comma-decimal locale would emit "12,5", which ROOT misparses as two arguments). Imbuing the
 * classic ("C") locale and setting max_digits10 precision once, in the constructor, fixes both for
 * all subsequent insertions.
 */
class EmitStream : public std::ostringstream {
public:
    EmitStream() {
        this->imbue(std::locale::classic());
        *this << std::setprecision(std::numeric_limits<double>::max_digits10);
    }
};

/******************************************************************************/
/**
 * Escape a user-supplied string for safe inclusion inside a ROOT C-string literal (axis/plot/canvas
 * labels, TF1/TF2 formulae) or a // comment. Without this, a label containing a double quote,
 * backslash or newline would produce a non-compiling macro (or, inside a comment, inject code past
 * the comment). Only the C-string metacharacters are touched, so ROOT TLatex markup (#frac, #sqrt,
 * braces) passes through unchanged.
 */
std::string rootEscape(const std::string &in) {
    std::string out;
    out.reserve(in.size());
    for(char c : in) {
        switch(c) {
            case '\\': out += "\\\\"; break;
            case '"':  out += "\\\""; break;
            case '\n': out += "\\n"; break;
            case '\t': out += "\\t"; break;
            case '\r':                break; // drop bare CRs
            default:   out += c;      break;
        }
    }
    return out;
}

/******************************************************************************/
/**
 * Escape a user-supplied string for safe inclusion inside a gnuplot DOUBLE-quoted
 * string literal (labels, titles). gnuplot double-quoted strings interpret C-style
 * backslash escapes (\" \\ \n \t), so the metacharacters must be backslash-escaped
 * exactly as for those literals -- this is deliberately NOT rootEscape (which targets
 * a ROOT C-string literal and additionally drops bare CRs / has its own contract).
 */
std::string gnuplotEscape(const std::string &in) {
    std::string out;
    out.reserve(in.size());
    for(char c : in) {
        switch(c) {
            case '\\': out += "\\\\"; break;
            case '"':  out += "\\\""; break;
            case '\n': out += "\\n"; break;
            case '\t': out += "\\t"; break;
            case '\r': out += "\\r"; break;
            default:   out += c;      break;
        }
    }
    return out;
}

/******************************************************************************/
/**
 * Escape a user-supplied string for safe inclusion inside a Python DOUBLE-quoted
 * string literal (labels, titles). Python double-quoted literals interpret C-style
 * backslash escapes (\" \\ \n \t \r), so the metacharacters are backslash-escaped
 * exactly as for those literals -- this is deliberately NEITHER rootEscape (ROOT
 * C-string contract, drops bare CRs) NOR gnuplotEscape (which targets a gnuplot
 * double-quoted literal). The result is always inserted between two `"` delimiters.
 */
std::string pythonEscape(const std::string &in) {
    std::string out;
    out.reserve(in.size());
    for(char c : in) {
        switch(c) {
            case '\\': out += "\\\\"; break;
            case '"':  out += "\\\""; break;
            case '\n': out += "\\n"; break;
            case '\t': out += "\\t"; break;
            case '\r': out += "\\r"; break;
            default:   out += c;      break;
        }
    }
    return out;
}

} // anonymous namespace

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
// Streamers

/******************************************************************************/
/**
 * Puts a gColor into a stream. Needed for streaming / Gem::Common::fromString<>
 *
 * @param o The output stream the value is written to
 * @param x The gColor enum value to be streamed out
 * @return A reference to the output stream, to allow chaining
 */
std::ostream &operator<<(std::ostream &o, const gColor &x) {
    auto tmp = static_cast<ENUMBASETYPE>(x);
    o << tmp;
    return o;
}

/******************************************************************************/
/**
 * Reads a gColor item from a stream. Needed for streaming / Gem::Common::fromString<>
 *
 * @param i The input stream the value is read from
 * @param x The gColor reference into which the parsed value is stored
 * @return A reference to the input stream, to allow chaining
 */
std::istream &operator>>(std::istream &i, gColor &x) {
    ENUMBASETYPE tmp = 0;
    i >> tmp;

#ifdef DEBUG
    x = Gem::Common::narrow<gColor>(tmp);
#else
    x = static_cast<gColor>(tmp);
#endif /* DEBUG */

    return i;
}

/******************************************************************************/
/**
 * Puts a gMarker into a stream. Needed for streaming / Gem::Common::fromString<>
 *
 * @param o The output stream the value is written to
 * @param x The gMarker enum value to be streamed out
 * @return A reference to the output stream, to allow chaining
 */
std::ostream &operator<<(std::ostream &o, const gMarker &x) {
    auto tmp = static_cast<ENUMBASETYPE>(x);
    o << tmp;
    return o;
}

/******************************************************************************/
/**
 * Reads a gMarker item from a stream. Needed for streaming / Gem::Common::fromString<>
 *
 * @param i The input stream the value is read from
 * @param x The gMarker reference into which the parsed value is stored
 * @return A reference to the input stream, to allow chaining
 */
std::istream &operator>>(std::istream &i, gMarker &x) {
    ENUMBASETYPE tmp = 0;
    i >> tmp;

#ifdef DEBUG
    x = Gem::Common::narrow<gMarker>(tmp);
#else
    x = static_cast<gMarker>(tmp);
#endif /* DEBUG */

    return i;
}

/******************************************************************************/
/**
 * Puts a gLineStyle into a stream. Needed for streaming / Gem::Common::fromString<>
 *
 * @param o The output stream the value is written to
 * @param x The gLineStyle enum value to be streamed out
 * @return A reference to the output stream, to allow chaining
 */
std::ostream &operator<<(std::ostream &o, const gLineStyle &x) {
    auto tmp = static_cast<ENUMBASETYPE>(x);
    o << tmp;
    return o;
}

/******************************************************************************/
/**
 * Reads a gLineStyle item from a stream. Needed for streaming / Gem::Common::fromString<>
 *
 * @param i The input stream the value is read from
 * @param x The gLineStyle reference into which the parsed value is stored
 * @return A reference to the input stream, to allow chaining
 */
std::istream &operator>>(std::istream &i, gLineStyle &x) {
    ENUMBASETYPE tmp = 0;
    i >> tmp;

#ifdef DEBUG
    x = Gem::Common::narrow<gLineStyle>(tmp);
#else
    x = static_cast<gLineStyle>(tmp);
#endif /* DEBUG */

    return i;
}

/******************************************************************************/
/**
 * Puts a graphPlotMode into a stream. Needed for streaming / Gem::Common::fromString<>
 *
 * @param o The output stream the value is written to
 * @param x The graphPlotMode enum value to be streamed out
 * @return A reference to the output stream, to allow chaining
 */
std::ostream &operator<<(std::ostream &o, const graphPlotMode &x) {
    auto tmp = static_cast<ENUMBASETYPE>(x);
    o << tmp;
    return o;
}

/******************************************************************************/
/**
 * Reads a graphPlotMode item from a stream. Needed for streaming / Gem::Common::fromString<>
 *
 * @param i The input stream the value is read from
 * @param x The graphPlotMode reference into which the parsed value is stored
 * @return A reference to the input stream, to allow chaining
 */
std::istream &operator>>(std::istream &i, graphPlotMode &x) {
    ENUMBASETYPE tmp = 0;
    i >> tmp;

#ifdef DEBUG
    x = Gem::Common::narrow<graphPlotMode>(tmp);
#else
    x = static_cast<graphPlotMode>(tmp);
#endif /* DEBUG */

    return i;
}

/******************************************************************************/
/**
 * Puts a tddropt into a stream. Needed for streaming / Gem::Common::fromString<>
 *
 * @param o The output stream the value is written to
 * @param x The tddropt enum value to be streamed out
 * @return A reference to the output stream, to allow chaining
 */
std::ostream &operator<<(std::ostream &o, const tddropt &x) {
    auto tmp = static_cast<ENUMBASETYPE>(x);
    o << tmp;
    return o;
}

/******************************************************************************/
/**
 * Reads a tddropt item from a stream. Needed for streaming / Gem::Common::fromString<>
 *
 * @param i The input stream the value is read from
 * @param x The tddropt reference into which the parsed value is stored
 * @return A reference to the input stream, to allow chaining
 */
std::istream &operator>>(std::istream &i, tddropt &x) {
    ENUMBASETYPE tmp = 0;
    i >> tmp;

#ifdef DEBUG
    x = Gem::Common::narrow<tddropt>(tmp);
#else
    x = static_cast<tddropt>(tmp);
#endif /* DEBUG */

    return i;
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * The copy constructor
 *
 * @param cp A copy of another GBasePlotter object
 */
GBasePlotter::GBasePlotter(const GBasePlotter &cp)
  : Gem::Common::GCommonInterfaceT<GBasePlotter>(cp)
  , drawing_arguments_(cp.drawing_arguments_)
  , x_axis_label_(cp.x_axis_label_)
  , y_axis_label_(cp.y_axis_label_)
  , z_axis_label_(cp.z_axis_label_)
  , plot_label_(cp.plot_label_)
  , ds_marker_(cp.ds_marker_)
  , id_(cp.id_) {
    // Note: Explicit scope needed for name resolution of clone -- compare
    // https://isocpp.org/wiki/faq/templates#nondependent-name-lookup-members

    // Copy secondary plot data over
    for(auto const &plotter_ptr : cp.secondary_plotter_) {
        secondary_plotter_.push_back(plotter_ptr->GCommonInterfaceT<GBasePlotter>::clone());
    }
}

/******************************************************************************/
/**
 * Assignment operator
 *
 * @param cp A constant reference to another GBasePlotter object to copy from
 * @return A reference to this object, to allow chaining
 */
GBasePlotter &GBasePlotter::operator=(GBasePlotter const &cp) {
    if(this == &cp) {
        return *this;
    }
    GCommonInterfaceT<GBasePlotter>::operator=(cp);

    drawing_arguments_ = cp.drawing_arguments_;
    x_axis_label_ = cp.x_axis_label_;
    y_axis_label_ = cp.y_axis_label_;
    z_axis_label_ = cp.z_axis_label_;
    plot_label_ = cp.plot_label_;
    ds_marker_ = cp.ds_marker_;
    id_ = cp.id_;

    Gem::Common::copyCloneableSmartPointerContainer(cp.secondary_plotter_, secondary_plotter_);

    return *this;
}

/******************************************************************************/
/**
 * Allows to set the drawing arguments for this plot
 *
 * @param drawing_arguments The drawing arguments for this plot
 */
void GBasePlotter::setDrawingArguments(std::string drawing_arguments) {
    drawing_arguments_ = drawing_arguments;
}

/******************************************************************************/
/**
 * Sets the label for the x-axis
 *
 * @param x_axis_label The label to be assigned to the x-axis
 * */
void GBasePlotter::setXAxisLabel(std::string x_axis_label) {
    x_axis_label_ = x_axis_label;
}

/******************************************************************************/
/**
 * Retrieve the x-axis label
 *
 * @return The label currently assigned to the x-axis
 */
std::string GBasePlotter::xAxisLabel() const {
    return x_axis_label_;
}

/******************************************************************************/
/**
 * Sets the label for the y-axis
 *
 * @param y_axis_label The label to be assigned to the y-axis
 */
void GBasePlotter::setYAxisLabel(std::string y_axis_label) {
    y_axis_label_ = y_axis_label;
}

/******************************************************************************/
/**
 * Retrieve the y-axis label
 *
 * @return The label currently assigned to the y-axis
 */
std::string GBasePlotter::yAxisLabel() const {
    return y_axis_label_;
}

/******************************************************************************/
/**
 * Sets the label for the z-axis
 *
 * @param z_axis_label The label to be assigned to the z-axis
 */
void GBasePlotter::setZAxisLabel(std::string z_axis_label) {
    z_axis_label_ = z_axis_label;
}

/******************************************************************************/
/**
 * Retrieve the z-axis label
 *
 * @return The label currently assigned to the z-axis
 */
std::string GBasePlotter::zAxisLabel() const {
    return z_axis_label_;
}

/******************************************************************************/
/**
 * Allows to assign a label to the entire plot
 *
 * @param p_l A label to be assigned to the entire plot
 */
void GBasePlotter::setPlotLabel(std::string p_l) {
    plot_label_ = p_l;
}

/******************************************************************************/
/**
 * Allows to retrieve the plot label
 *
 * @return The label that has been assigned to the plot
 */
std::string GBasePlotter::plotLabel() const {
    return plot_label_;
}

/******************************************************************************/
/**
 * Reports this plotter's choice as a GPlotSpec value. The base fills the common
 * fields (name, labels, drawing args) and a default kind; concrete plotters override
 * this to set their kind, their column names and (for histograms) the bin counts.
 * This is a pure const reporter and does not touch serialize()/load_()/compare_().
 *
 * @return A GPlotSpec describing this plotter
 */
GPlotSpec GBasePlotter::plotSpec() const {
    GPlotSpec spec; // kind defaults to graph_2d, role defaulted from it
    spec.name = plot_label_;
    spec.x_label = x_axis_label_;
    spec.y_label = y_axis_label_;
    spec.z_label = z_axis_label_;
    spec.drawing_args = drawing_arguments_;
    return spec;
}

/******************************************************************************/
/**
 * The base reports no exportable columns; the columnar collectors override this (see
 * GDataCollectorT::dataColumns()). A plotter that stores no exportable sample columns
 * -- a function plotter -- keeps this default.
 *
 * @return An empty list of columns
 */
std::vector<GPlotColumn> GBasePlotter::dataColumns() const {
    return {};
}

/******************************************************************************/
/**
 * The base cannot accept a generic data row (a function plotter holds no sample
 * columns); the columnar collectors override this. See GDataCollectorT::appendRow().
 *
 * @param row Unused in the base; always rejected
 */
void GBasePlotter::appendRow([[maybe_unused]] std::span<const double> row) {
    throw geneva_exception(
        g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
        << "In GBasePlotter::appendRow(): Error!" << '\n'
        << getPlotterName() << " does not support generic row append." << '\n'
    );
}

/******************************************************************************/
/**
 * The base has no sortable columns to order; the columnar collectors override this
 * (see GDataCollectorT::sortByFirstColumn()).
 */
void GBasePlotter::sortByFirstColumn() { /* nothing */ }

/******************************************************************************/
/**
 * Constructs an empty plotter of the kind described by a GPlotSpec (the inverse of
 * GBasePlotter::plotSpec()), applying the spec's labels, drawing arguments and (for
 * histograms) bin counts. The function plotters cannot be reconstructed from a spec
 * and therefore throw. See the declaration in GPlots.hpp for the full contract.
 *
 * @param spec The plot specification describing the plotter to build
 * @return A newly-allocated, empty plotter matching the spec
 */
std::unique_ptr<GBasePlotter> makePlotter(const GPlotSpec &spec) {
    std::unique_ptr<GBasePlotter> p;
    switch(spec.kind) {
        case plotKind::graph_2d:
            p = std::make_unique<GGraph2D>();
            break;
        case plotKind::graph_2d_err:
            p = std::make_unique<GGraph2ED>();
            break;
        case plotKind::graph_3d:
            p = std::make_unique<GGraph3D>();
            break;
        case plotKind::graph_4d:
            p = std::make_unique<GGraph4D>();
            break;
        case plotKind::hist_1d:
            // With a fixed range use the (n_bins, min, max) ctor; otherwise the single-bin-count
            // ctor auto-determines the value range from the data.
            if(spec.range_x.has_value()) {
                p = std::make_unique<GHistogram1D>(spec.n_bins_x.value_or(10), *spec.range_x);
            }
            else {
                p = std::make_unique<GHistogram1D>(spec.n_bins_x.value_or(10));
            }
            break;
        case plotKind::hist_2d:
            // A fixed range needs BOTH per-axis ranges (there is no mixed ctor); otherwise auto-range.
            if(spec.range_x.has_value() && spec.range_y.has_value()) {
                p = std::make_unique<GHistogram2D>(
                    spec.n_bins_x.value_or(10), spec.n_bins_y.value_or(10), *spec.range_x, *spec.range_y
                );
            }
            else {
                p = std::make_unique<GHistogram2D>(
                    spec.n_bins_x.value_or(10), spec.n_bins_y.value_or(10)
                );
            }
            break;
        case plotKind::hist_1i:
            // GHistogram1I has no auto-range ctor, so it can only be reconstructed when the spec
            // carries an explicit value range.
            if(spec.range_x.has_value()) {
                p = std::make_unique<GHistogram1I>(spec.n_bins_x.value_or(10), *spec.range_x);
            }
            else {
                throw geneva_exception(
                    g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                    << "In makePlotter(): Error!" << '\n'
                    << "an integer histogram (kind \"" << to_string(spec.kind)
                    << "\") without a fixed value range cannot be reconstructed from a "
                    << "GPlotSpec -- an integer histogram has no auto-range form." << '\n'
                );
            }
            break;
        case plotKind::function_1d:
        case plotKind::function_2d:
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In makePlotter(): Error!" << '\n'
                << "a function plotter (kind \"" << to_string(spec.kind)
                << "\") cannot be reconstructed from a GPlotSpec -- its formula and "
                << "sampling range are not part of the spec." << '\n'
            );
    }

    p->setPlotLabel(spec.name);
    p->setXAxisLabel(spec.x_label);
    p->setYAxisLabel(spec.y_label);
    p->setZAxisLabel(spec.z_label);
    p->setDrawingArguments(spec.drawing_args);

    // The plot mode (scatter vs. curve) is a GGraph2D / GGraph2ED render choice; apply it
    // when the spec carries one and the built plotter supports it.
    if(spec.plot_mode.has_value()) {
        if(auto *g2d = dynamic_cast<GGraph2D *>(p.get())) {
            g2d->setPlotMode(*spec.plot_mode);
        }
        else if(auto *g2ed = dynamic_cast<GGraph2ED *>(p.get())) {
            g2ed->setPlotMode(*spec.plot_mode);
        }
    }
    return p;
}

/******************************************************************************/
/**
 * Allows to assign a marker to data structures in the output file
 *
 * @param ds_marker A marker that has been assigned to the output data structures
 */
void GBasePlotter::setDataStructureMarker(std::string ds_marker) {
    ds_marker_ = ds_marker;
}

/******************************************************************************/
/**
 * Allows to retrieve the data structure marker
 *
 * @return The marker that has been assigned to the output data structures
 */
std::string GBasePlotter::dsMarker() const {
    return ds_marker_;
}

/******************************************************************************/
/**
 * Allows to add secondary plots to be added to the same sub-canvas
 *
 * @param sp A shared pointer to the secondary plotter to register; must be
 * non-empty and compatible with this plotter, otherwise an exception is thrown
 */
void GBasePlotter::registerSecondaryPlotter(std::shared_ptr<GBasePlotter> sp) {
    // Check that the secondary plot isn't empty
    if(not sp) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GBasePlotter::registerSecondaryPlot(): Error!" << '\n'
            << "Got empty secondary plot" << '\n'
        );
    }

    // Check that the secondary plotter is compatible with us
    if(not this->isCompatible(sp)) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GBasePlotter::registerSecondaryPlot(): Error!" << '\n'
            << "Received incompatible secondary plotter" << '\n'
            << sp->getPlotterName() << " in plotter " << this->getPlotterName() << '\n'
        );
    }

    // Add the plotter to our collection
    secondary_plotter_.push_back(sp);
}

/******************************************************************************/
/**
 * Check that a given plotter is compatible with us. By default we only
 * check that the names of both plotters match. If other plot types are
 * compatible with this plotter, you need to overload this function.
 *
 * @param other A shared pointer to the other plotter whose compatibility is checked
 * @return true if the other plotter is compatible (same plotter name by default), false otherwise
 */
bool GBasePlotter::isCompatible(std::shared_ptr<GBasePlotter> other) const {
    return (this->getPlotterName() == other->getPlotterName());
}

/******************************************************************************/
/**
 * calculate a suffix from id and parent ids
 *
 * @param is_secondary Whether this plotter is a secondary plotter (true) or a primary one (false)
 * @param p_id The id of the parent plotter, only used when is_secondary is true
 * @param own_id This plotter's own emit index, threaded in by the caller (replaces the former mutated id_)
 * @return A suffix string built from the parent id (if secondary) and this object's own id
 */
std::string GBasePlotter::suffix(bool is_secondary, std::size_t p_id, std::size_t own_id) const {
    std::string result; // NOLINT(cppcoreguidelines-init-variables)

    if(not is_secondary) {
        result = std::string("_") + to_string(own_id);
    }
    else {
        result = std::string("_") + to_string(p_id) + std::string("_") + to_string(own_id);
    }

    return result;
}

/******************************************************************************/
/**
 * Allows to retrieve the id of this object
 *
 * @return The id currently assigned to this object
 */
std::size_t GBasePlotter::id() const {
    return id_;
}

/******************************************************************************/
/**
 * Read-only access to the secondary plotters sharing this plotter's pad
 *
 * @return A const reference to the list of registered secondary plotters
 */
const std::vector<std::shared_ptr<GBasePlotter>> &GBasePlotter::secondaryPlotters() const {
    return secondary_plotter_;
}

/******************************************************************************/
/**
 * Sets the id of the object
 *
 * @param id The id to be assigned to this object
 */
void GBasePlotter::setId(const std::size_t &id) {
    id_ = id;
}

/******************************************************************************/
/**
 * Returns the name of this class
 *
 * @return The name of this class as a string
 */
std::string GBasePlotter::name_() const {
    return std::string("GBasePlotter");
}

/******************************************************************************/
/**
 * Searches for compliance with expectations with respect to another object
 * of the same type
 *
 * @param cp A constant reference to another GBasePlotter object, passed as a GBasePlotter reference
 * @param e The expectation (equality / inequality) the comparison should fulfil
 * @param limit The acceptable tolerance for floating point comparisons (unused here)
 */
void GBasePlotter::compare_(
    const GBasePlotter &cp,
    const expectation &e,
    [[maybe_unused]] const double & limit
) const {
    // Check that we are dealing with a GBasePlotter reference independent of this object and convert the pointer
    const auto *p_load = g_convert_and_compare(cp, this);

    GToken token("GBasePlotter", e);

    // Compare our parent data ...
    compare_base_t<GCommonInterfaceT<GBasePlotter>>(*this, *p_load, token);

    // ... and then the local data
    g_compare_members(localMembers_(*this), localMembers_(*p_load), token);

    // React on deviations from the expectation
    token.evaluate();
}

/******************************************************************************/
/**
 * Loads the data of another object
 *
 * @param cp A constant pointer to another GBasePlotter object whose data is loaded into this object
 */
void GBasePlotter::load_(const GBasePlotter *cp) {
    // Check that we are dealing with a GBasePlotter reference independent of this object and convert the pointer
    const auto *p_load = g_convert_and_compare(cp, this);

    // No parent class with loadable data

    // Load local data
    g_load_members(localMembers_(*this), localMembers_(*p_load));
}

/******************************************************************************/
/**
 * Retrieve header settings for this plot (and any sub-plots)
 *
 * @param indent The indentation string prepended to every emitted line
 * @return The combined header data of this primary plotter and any secondary plotters
 */
std::string GBasePlotter::headerData(const std::string &indent) const {
    EmitStream header_data; // NOLINT(cppcoreguidelines-init-variables)

    // Add this plot's data
    header_data << indent << "// Header data for primary plotter" << '\n'
                << this->headerData_(false, 0, this->id(), indent);

    // Extract data from the secondary plotters, if any. The secondary plotter's own
    // emit index (pos) is threaded through the call instead of being stashed in its
    // id_, so this const emit no longer mutates the shared child state.
    std::size_t pos = 0;
    for(auto const &plotter_ptr : secondary_plotter_) {
        // We parent id 0 is reserved for primary plotters
        header_data << indent << "// Header data for secondary plotter " << pos << " of "
                    << this->getPlotterName() << '\n'
                    << plotter_ptr->headerData_(true, this->id(), pos, indent) << '\n';

        pos++;
    }

    return header_data.str();
}

/******************************************************************************/
/**
 * Retrieves body / data settings for this plot (and any sub-plots)
 *
 * @param indent The indentation string prepended to every emitted line
 * @return The combined body data of this primary plotter and any secondary plotters
 */
std::string GBasePlotter::bodyData(const std::string &indent) const {
    EmitStream body_data; // NOLINT(cppcoreguidelines-init-variables)

    // Add this plot's data
    body_data << indent << "// Body data for primary plotter" << '\n'
              << this->bodyData_(false, 0, this->id(), indent);

    // Extract data from the secondary plotters, if any. The secondary plotter's own
    // emit index (pos) is threaded through the call instead of being stashed in its
    // id_, so this const emit no longer mutates the shared child state.
    std::size_t pos = 0;
    for(auto const &plotter_ptr : secondary_plotter_) {
        body_data << indent << "// Body data for secondary plotter " << pos << " of "
                  << this->getPlotterName() << '\n'
                  << plotter_ptr->bodyData_(true, this->id(), pos, indent) << '\n';

        pos++;
    }

    return body_data.str();
}

/******************************************************************************/
/**
 * Retrieves footer / drawing settings for this plot (and any sub-plots)
 *
 * @param indent The indentation string prepended to every emitted line
 * @return The combined footer data of this primary plotter and any secondary plotters
 */
std::string GBasePlotter::footerData(const std::string &indent) const {
    EmitStream footer_data; // NOLINT(cppcoreguidelines-init-variables)

    // Add this plot's data
    footer_data << indent << "// Footer data for primary plotter" << '\n'
                << this->footerData_(false, 0, this->id(), indent);

    // Extract data from the secondary plotters, if any. The secondary plotter's own
    // emit index (pos) is threaded through the call instead of being stashed in its
    // id_, so this const emit no longer mutates the shared child state.
    std::size_t pos = 0;
    for(auto const &plotter_ptr : secondary_plotter_) {
        footer_data << indent << "// Footer data for secondary plotter " << pos << " of "
                    << this->getPlotterName() << '\n'
                    << plotter_ptr->footerData_(true, this->id(), pos, indent) << '\n';

        pos++;
    }

    return footer_data.str();
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * Adds arrows to the plots between consecutive points. Note that setting this
 * value to true will force "SCATTER" mode
 *
 * @param d_a The desired value of the draw_arrows_ variable
 */
void GGraph2D::setDrawArrows(bool d_a) {
    draw_arrows_ = d_a;
}

/******************************************************************************/
/**
 * Retrieves the value of the draw_arrows_ variable
 *
 * @return The value of the draw_arrows_ variable
 */
bool GGraph2D::getDrawArrows() const {
    return draw_arrows_;
}

/******************************************************************************/
/**
 * Determines whether a scatter plot or a curve is created
 *
 * @param p_m The desired plot mode
 */
void GGraph2D::setPlotMode(graphPlotMode p_m) {
    p_m_ = p_m;
}

/******************************************************************************/
/**
 * Allows to retrieve the current plotting mode
 *
 * @return The current plot mode
 */
graphPlotMode GGraph2D::getPlotMode() const {
    return p_m_;
}

/******************************************************************************/
/**
 * Retrieves a unique name for this plotter
 *
 * @return A unique name identifying this plotter type
 */
std::string GGraph2D::getPlotterName() const {
    return "GGraph2D";
}

/******************************************************************************/
/**
 * Reports this plotter's choice as a GPlotSpec (a 2-d xy graph).
 *
 * @return A GPlotSpec describing this GGraph2D
 */
GPlotSpec GGraph2D::plotSpec() const {
    GPlotSpec spec = GBasePlotter::plotSpec();
    spec.kind = plotKind::graph_2d;
    spec.role = defaultRole(spec.kind);
    spec.columns = {"x", "y"};
    spec.plot_mode = getPlotMode();
    return spec;
}

/******************************************************************************/
/**
 * Returns the name of this class
 *
 * @return The name of this class as a string
 */
std::string GGraph2D::name_() const {
    return std::string("GGraph2D");
}

/******************************************************************************/
/**
 * Searches for compliance with expectations with respect to another object
 * of the same type
 *
 * @param cp A constant reference to another object, passed as a GBasePlotter reference
 * @param e The expectation (equality / inequality) the comparison should fulfil
 * @param limit The acceptable tolerance for floating point comparisons (unused here)
 */
void GGraph2D::compare_(
    const GBasePlotter &cp,
    const expectation &e,
    [[maybe_unused]] const double & limit
) const {
    // Check that we are dealing with a GGraph2D reference independent of this object and convert the pointer
    const auto *p_load = g_convert_and_compare(cp, this);

    GToken token("GGraph2D", e);

    // Compare our parent data ...
    compare_base_t<GDataCollector2T<double, double>>(*this, *p_load, token);

    // ... and then the local data, derived from the single localMembers() declaration
    g_compare_members(localMembers_(*this), localMembers_(*p_load), token);

    // React on deviations from the expectation
    token.evaluate();
}

/******************************************************************************/
/**
 * Retrieve specific header settings for this plot
 *
 * @param is_secondary Whether this plotter is a secondary plotter (true) or a primary one (false)
 * @param p_id The id of the parent plotter, used to build array names for secondary plotters
 * @param own_id This plotter's own emit index, threaded in by the caller (replaces the former mutated id_)
 * @param indent The indentation string prepended to every emitted line
 * @return The header code declaring the x/y data arrays for this graph
 */
std::string
GGraph2D::headerData_(bool is_secondary, std::size_t p_id, std::size_t own_id, const std::string &indent) const {
    EmitStream header_data; // NOLINT(cppcoreguidelines-init-variables)

    // Set up suitable arrays for the header
    std::string base_name = suffix(is_secondary, p_id, own_id);
    std::string array_base_name = "array_" + base_name;

    std::string x_array_name = "x_" + array_base_name;
    std::string y_array_name = "y_" + array_base_name;

    std::string comment; // NOLINT(cppcoreguidelines-init-variables)
    if(!ds_marker_.empty()) {
        comment = "// " + rootEscape(ds_marker_);
    }

    header_data << indent << "double " << x_array_name << "[" << to_string(this->currentSize()) << "];"
                << (!comment.empty() ? comment : "") << '\n'
                << indent << "double " << y_array_name << "[" << to_string(this->currentSize()) << "];"
                << '\n'
                << '\n';

    return header_data.str();
}

/******************************************************************************/
/**
 * Retrieves the actual data sets
 *
 * @param is_secondary Whether this plotter is a secondary plotter (true) or a primary one (false)
 * @param p_id The id of the parent plotter, used to build array names for secondary plotters
 * @param own_id This plotter's own emit index, threaded in by the caller (replaces the former mutated id_)
 * @param indent The indentation string prepended to every emitted line
 * @return The body code filling the x/y data arrays with this graph's tuple values
 */
std::string
GGraph2D::bodyData_(bool is_secondary, std::size_t p_id, std::size_t own_id, const std::string &indent) const {
    EmitStream body_data; // NOLINT(cppcoreguidelines-init-variables)

    // Set up suitable arrays for the header
    std::string base_name = suffix(is_secondary, p_id, own_id);
    std::string array_base_name = "array_" + base_name;

    std::string x_array_name = "x_" + array_base_name;
    std::string y_array_name = "y_" + array_base_name;

    std::string comment; // NOLINT(cppcoreguidelines-init-variables)
    if(!ds_marker_.empty()) {
        body_data << "// " + rootEscape(ds_marker_) << '\n';
    }

    // Fill data from the columns into the arrays
    const auto &x_col = this->column<0>();
    const auto &y_col = this->column<1>();
    const std::size_t n = this->currentSize();

    for(std::size_t pos_counter = 0; pos_counter < n; ++pos_counter) {
        body_data << indent << x_array_name << "[" << pos_counter << "] = " << x_col[pos_counter]
                  << ";"
                  << "\t" << y_array_name << "[" << pos_counter << "] = " << y_col[pos_counter] << ";"
                  << '\n';
    }
    body_data << '\n';

    return body_data.str();
}

/******************************************************************************/
/**
 * Retrieves specific draw commands for this plot
 *
 * @param is_secondary Whether this plotter is a secondary plotter (true) or a primary one (false)
 * @param p_id The id of the parent plotter, used to build array/object names for secondary plotters
 * @param own_id This plotter's own emit index, threaded in by the caller (replaces the former mutated id_)
 * @param indent The indentation string prepended to every emitted line
 * @return The footer code creating and drawing the ROOT TGraph (and optional arrows) for this graph
 */
std::string
GGraph2D::footerData_(bool is_secondary, std::size_t p_id, std::size_t own_id, const std::string &indent) const {
    EmitStream footer_data; // NOLINT(cppcoreguidelines-init-variables)

    // Set up suitable arrays for the header
    std::string base_name = suffix(is_secondary, p_id, own_id);
    std::string array_base_name = "array_" + base_name;

    std::string x_array_name = "x_" + array_base_name;
    std::string y_array_name = "y_" + array_base_name;

    std::string graph_name = std::string("graph") + base_name;

    std::string comment; // NOLINT(cppcoreguidelines-init-variables)
    if(!ds_marker_.empty()) {
        footer_data << "// " + rootEscape(ds_marker_) << '\n';
    }

    // Retrieve the current drawing arguments
    std::string d_a = this->drawingArguments(is_secondary);

    // Fill the data in our columns into a ROOT TGraph object
    footer_data << indent << "TGraph *" << graph_name << " = new TGraph(" << this->currentSize() << ", "
                << x_array_name << ", " << y_array_name << ");" << '\n'
                << indent << graph_name << "->GetXaxis()->SetTitle(\"" << rootEscape(xAxisLabel()) << "\");"
                << '\n'
                << indent << graph_name << "->GetYaxis()->SetTitle(\"" << rootEscape(yAxisLabel()) << "\");"
                << '\n';

    if(!plot_label_.empty()) {
        footer_data << indent << graph_name << "->SetTitle(\"" << rootEscape(plot_label_) << "\");" << '\n';
    }
    else {
        footer_data << indent << graph_name << "->SetTitle(\" \");" << '\n';
    }

    footer_data << indent << graph_name << "->Draw(\"" << d_a << "\");" << '\n' << '\n';

    if(draw_arrows_ && this->currentSize() >= 2) {
        const auto &x_col = this->column<0>();
        const auto &y_col = this->column<1>();
        const std::size_t n = this->currentSize();
        std::size_t pos_counter = 0;

        double x1 = x_col[0];
        double y1 = y_col[0];
        double x2 = 0.;
        double y2 = 0.;

        for(std::size_t i = 1; i < n; ++i) {
            x2 = x_col[i];
            y2 = y_col[i];

            footer_data << indent << "TArrow * ta_" << graph_name << "_" << pos_counter
                        << " = new TArrow(" << x1 << ", " << y1 << "," << x2 << ", " << y2 << ", "
                        << 0.05 << ", \"|>\");" << '\n'
                        << indent << "ta_" << graph_name << "_" << pos_counter
                        << "->SetArrowSize(0.01);" << '\n'
                        << indent << "ta_" << graph_name << "_" << pos_counter << "->Draw();"
                        << '\n';

            x1 = x2;
            y1 = y2;

            pos_counter++;
        }
        footer_data << '\n';
    }
    footer_data << '\n';

    return footer_data.str();
}

/******************************************************************************/
/**
 * Retrieve the current drawing arguments
 *
 * @param is_secondary Whether this plotter is a secondary plotter (true) or a primary one (false)
 * @return The ROOT draw-option string, derived from custom arguments or the plot mode / arrow setting
 */
std::string GGraph2D::drawingArguments(bool is_secondary) const {
    std::string d_a;

    if(!this->drawing_arguments_.empty()) {
        d_a = this->drawing_arguments_;
    }
    else {
        if(graphPlotMode::SCATTER == p_m_ || draw_arrows_) {
            d_a = "P";
        }
        else {
            d_a = "PL";
        }

        if(is_secondary) {
            d_a = d_a + ",same";
        }
        else {
            d_a = "A" + d_a;
        }
    }

    return d_a;
}

/******************************************************************************/
/**
 * Creates a deep clone of this object
 *
 * @return A deep copy of this object, returned as a GBasePlotter pointer
 */
GBasePlotter *GGraph2D::clone_() const {
    return new GGraph2D(*this);
}

/******************************************************************************/
/**
 * Loads the data of another object
 *
 * @param cp A constant pointer to another object (as a GBasePlotter) whose data is loaded into this one
 */
void GGraph2D::load_(const GBasePlotter *cp) {
    // Check that we are dealing with a GGraph2D reference independent of this object and convert the pointer
    const auto *p_load = g_convert_and_compare(cp, this);

    // Load our parent class'es data ...
    GDataCollector2T<double, double>::load_(cp);

    // ... and then our local data, derived from the single localMembers() declaration
    g_load_members(localMembers_(*this), localMembers_(*p_load));
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * Determines whether a scatter plot or a curve is created
 *
 * @param p_m The desired plot mode
 */
void GGraph2ED::setPlotMode(graphPlotMode p_m) {
    p_m_ = p_m;
}

/******************************************************************************/
/**
 * Allows to retrieve the current plotting mode
 *
 * @return The current plot mode
 */
graphPlotMode GGraph2ED::getPlotMode() const {
    return p_m_;
}

/******************************************************************************/
/**
 * Retrieves a unique name for this plotter
 *
 * @return A unique name identifying this plotter type
 */
std::string GGraph2ED::getPlotterName() const {
    return "GGraph2ED";
}

/******************************************************************************/
/**
 * Reports this plotter's choice as a GPlotSpec (a 2-d xy graph with x/y errors).
 *
 * @return A GPlotSpec describing this GGraph2ED
 */
GPlotSpec GGraph2ED::plotSpec() const {
    GPlotSpec spec = GBasePlotter::plotSpec();
    spec.kind = plotKind::graph_2d_err;
    spec.role = defaultRole(spec.kind);
    spec.columns = {"x", "ex", "y", "ey"};
    spec.plot_mode = getPlotMode();
    return spec;
}

/******************************************************************************/
/**
 * Returns the name of this class
 *
 * @return The name of this class as a string
 */
std::string GGraph2ED::name_() const {
    return std::string("GGraph2ED");
}

/******************************************************************************/
/**
 * Searches for compliance with expectations with respect to another object
 * of the same type
 *
 * @param cp A constant reference to another object, passed as a GBasePlotter reference
 * @param e The expectation (equality / inequality) the comparison should fulfil
 * @param limit The acceptable tolerance for floating point comparisons (unused here)
 */
void GGraph2ED::compare_(
    const GBasePlotter &cp,
    const expectation &e,
    [[maybe_unused]] const double & limit
) const {
    // Check that we are dealing with a GBasePlotter reference independent of this object and convert the pointer
    const auto *p_load = g_convert_and_compare(cp, this);

    GToken token("GGraph2ED", e);

    // Compare our parent data ...
    compare_base_t<GDataCollector2ET<double, double>>(*this, *p_load, token);

    // ... and then the local data, derived from the single localMembers() declaration
    g_compare_members(localMembers_(*this), localMembers_(*p_load), token);

    // React on deviations from the expectation
    token.evaluate();
}

/******************************************************************************/
/**
 * Retrieve specific header settings for this plot
 *
 * @param is_secondary Whether this plotter is a secondary plotter (true) or a primary one (false)
 * @param p_id The id of the parent plotter, used to build array names for secondary plotters
 * @param own_id This plotter's own emit index, threaded in by the caller (replaces the former mutated id_)
 * @param indent The indentation string prepended to every emitted line
 * @return The header code declaring the x/ex/y/ey data arrays for this error graph
 */
std::string
GGraph2ED::headerData_(bool is_secondary, std::size_t p_id, std::size_t own_id, const std::string &indent) const {
    EmitStream header_data; // NOLINT(cppcoreguidelines-init-variables)

    // Set up suitable arrays for the header
    std::string base_name = suffix(is_secondary, p_id, own_id);
    std::string array_base_name = "array_" + base_name;

    std::string x_array_name = "x_" + array_base_name;
    std::string ex_array_name = "ex_" + array_base_name;
    std::string y_array_name = "y_" + array_base_name;
    std::string ey_array_name = "ey_" + array_base_name;

    std::string comment; // NOLINT(cppcoreguidelines-init-variables)
    if(!ds_marker_.empty()) {
        comment = "// " + rootEscape(ds_marker_);
    }

    header_data << indent << "double " << x_array_name << "[" << to_string(this->currentSize()) << "];"
                << comment << '\n'
                << indent << "double " << ex_array_name << "[" << to_string(this->currentSize()) << "];"
                << '\n'
                << indent << "double " << y_array_name << "[" << to_string(this->currentSize()) << "];"
                << '\n'
                << indent << "double " << ey_array_name << "[" << to_string(this->currentSize()) << "];"
                << '\n'
                << '\n';

    return header_data.str();
}

/******************************************************************************/
/**
 * Retrieves the actual data sets
 *
 * @param is_secondary Whether this plotter is a secondary plotter (true) or a primary one (false)
 * @param p_id The id of the parent plotter, used to build array names for secondary plotters
 * @param own_id This plotter's own emit index, threaded in by the caller (replaces the former mutated id_)
 * @param indent The indentation string prepended to every emitted line
 * @return The body code filling the x/ex/y/ey data arrays with this graph's tuple values
 */
std::string
GGraph2ED::bodyData_(bool is_secondary, std::size_t p_id, std::size_t own_id, const std::string &indent) const {
    EmitStream body_data; // NOLINT(cppcoreguidelines-init-variables)

    // Set up suitable arrays for the header
    std::string base_name = suffix(is_secondary, p_id, own_id);
    std::string array_base_name = "array_" + base_name;

    std::string x_array_name = "x_" + array_base_name;
    std::string ex_array_name = "ex_" + array_base_name;
    std::string y_array_name = "y_" + array_base_name;
    std::string ey_array_name = "ey_" + array_base_name;

    std::string comment; // NOLINT(cppcoreguidelines-init-variables)
    if(!ds_marker_.empty()) {
        body_data << "// " + rootEscape(ds_marker_) << '\n';
    }

    // Fill data from the columns into the arrays
    const auto &x_col = this->column<0>();
    const auto &ex_col = this->column<1>();
    const auto &y_col = this->column<2>();
    const auto &ey_col = this->column<3>();
    const std::size_t n = this->currentSize();

    for(std::size_t pos_counter = 0; pos_counter < n; ++pos_counter) {
        body_data << indent << x_array_name << "[" << pos_counter << "] = " << x_col[pos_counter]
                  << ";" << '\n'
                  << indent << ex_array_name << "[" << pos_counter << "] = " << ex_col[pos_counter]
                  << ";" << '\n'
                  << indent << y_array_name << "[" << pos_counter << "] = " << y_col[pos_counter]
                  << ";" << '\n'
                  << indent << ey_array_name << "[" << pos_counter << "] = " << ey_col[pos_counter]
                  << ";" << '\n';
    }
    body_data << '\n';

    return body_data.str();
}

/******************************************************************************/
/**
 * Retrieves specific draw commands for this plot
 *
 * @param is_secondary Whether this plotter is a secondary plotter (true) or a primary one (false)
 * @param p_id The id of the parent plotter, used to build array/object names for secondary plotters
 * @param own_id This plotter's own emit index, threaded in by the caller (replaces the former mutated id_)
 * @param indent The indentation string prepended to every emitted line
 * @return The footer code creating and drawing the ROOT TGraphErrors object for this graph
 */
std::string
GGraph2ED::footerData_(bool is_secondary, std::size_t p_id, std::size_t own_id, const std::string &indent) const {
    EmitStream footer_data; // NOLINT(cppcoreguidelines-init-variables)

    // Set up suitable arrays for the header
    std::string base_name = suffix(is_secondary, p_id, own_id);
    std::string array_base_name = "array_" + base_name;

    std::string x_array_name = "x_" + array_base_name;
    std::string ex_array_name = "ex_" + array_base_name;
    std::string y_array_name = "y_" + array_base_name;
    std::string ey_array_name = "ey_" + array_base_name;

    std::string graph_name = std::string("graph_") + base_name;

    std::string comment; // NOLINT(cppcoreguidelines-init-variables)
    if(!ds_marker_.empty()) {
        footer_data << "// " + rootEscape(ds_marker_) << '\n';
    }

    // Check whether custom drawing arguments have been set or whether one
    // of our generic choices has been selected
    std::string d_a = this->drawingArguments(is_secondary);

    // Fill the data in our tuple-vector into a ROOT TGraphErrors object
    footer_data << indent << "TGraphErrors *" << graph_name << " = new TGraphErrors("
                << this->currentSize() << ", " << x_array_name << ", " << y_array_name << ", "
                << ex_array_name << " ," << ey_array_name << ");" << '\n'
                << indent << graph_name << "->GetXaxis()->SetTitle(\"" << rootEscape(xAxisLabel()) << "\");"
                << '\n'
                << indent << graph_name << "->GetYaxis()->SetTitle(\"" << rootEscape(yAxisLabel()) << "\");"
                << '\n';

    if(!plot_label_.empty()) {
        footer_data << indent << graph_name << "->SetTitle(\"" << rootEscape(plot_label_) << "\");" << '\n';
    }
    else {
        footer_data << indent << graph_name << "->SetTitle(\" \");" << '\n';
    }

    footer_data << indent << graph_name << "->Draw(\"" << d_a << "\");" << '\n' << '\n';

    return footer_data.str();
}

/******************************************************************************/
/**
 * Retrieve the current drawing arguments
 *
 * @param is_secondary Whether this plotter is a secondary plotter (true) or a primary one (false)
 * @return The ROOT draw-option string, derived from custom arguments or the current plot mode
 */
std::string GGraph2ED::drawingArguments(bool is_secondary) const {
    std::string d_a;

    if(!this->drawing_arguments_.empty()) {
        d_a = this->drawing_arguments_;
    }
    else {
        if(graphPlotMode::SCATTER == p_m_) {
            d_a = "P";
        }
        else {
            d_a = "PL";
        }

        if(is_secondary) {
            d_a = d_a + ",same";
        }
        else {
            d_a = "A" + d_a;
        }
    }

    return d_a;
}

/******************************************************************************/
/**
 * Creates a deep clone of this object
 *
 * @return A deep copy of this object, returned as a GBasePlotter pointer
 */
GBasePlotter *GGraph2ED::clone_() const {
    return new GGraph2ED(*this);
}

/******************************************************************************/
/**
 * Loads the data of another object
 *
 * @param cp A constant pointer to another object (as a GBasePlotter) whose data is loaded into this one
 */
void GGraph2ED::load_(const GBasePlotter *cp) {
    // Check that we are dealing with a GGraph2ED reference independent of this object and convert the pointer
    const auto *p_load = g_convert_and_compare(cp, this);

    // Load our parent class'es data ...
    GDataCollector2ET<double, double>::load_(cp);

    // ... and then our local data, derived from the single localMembers() declaration
    g_load_members(localMembers_(*this), localMembers_(*p_load));
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * Adds lines to the plots between consecutive points.
 *
 * @param d_l The desired value of the draw_lines_ variable
 */
void GGraph3D::setDrawLines(bool d_l) {
    draw_lines_ = d_l;
}

/******************************************************************************/
/**
 * Retrieves the value of the draw_lines_ variable
 *
 * @return The value of the draw_lines_ variable
 */
bool GGraph3D::getDrawLines() const {
    return draw_lines_;
}

/******************************************************************************/
/**
 * Retrieves a unique name for this plotter
 *
 * @return A unique name identifying this plotter type
 */
std::string GGraph3D::getPlotterName() const {
    return "GGraph3D";
}

/******************************************************************************/
/**
 * Reports this plotter's choice as a GPlotSpec (a 3-d xyz graph).
 *
 * @return A GPlotSpec describing this GGraph3D
 */
GPlotSpec GGraph3D::plotSpec() const {
    GPlotSpec spec = GBasePlotter::plotSpec();
    spec.kind = plotKind::graph_3d;
    spec.role = defaultRole(spec.kind);
    spec.columns = {"x", "y", "z"};
    return spec;
}

/******************************************************************************/
/**
 * Returns the name of this class
 *
 * @return The name of this class as a string
 */
std::string GGraph3D::name_() const {
    return std::string("GGraph3D");
}

/******************************************************************************/
/**
 * Searches for compliance with expectations with respect to another object
 * of the same type
 *
 * @param cp A constant reference to another object, passed as a GBasePlotter reference
 * @param e The expectation (equality / inequality) the comparison should fulfil
 * @param limit The acceptable tolerance for floating point comparisons (unused here)
 */
void GGraph3D::compare_(
    const GBasePlotter &cp,
    const expectation &e,
    [[maybe_unused]] const double & limit
) const {
    // Check that we are dealing with a GGraph3D reference independent of this object and convert the pointer
    const auto *p_load = g_convert_and_compare(cp, this);

    GToken token("GGraph3D", e);

    // Compare our parent data ...
    compare_base_t<GDataCollector3T<double, double, double>>(*this, *p_load, token);

    // ... and then the local data, derived from the single localMembers() declaration
    g_compare_members(localMembers_(*this), localMembers_(*p_load), token);

    // React on deviations from the expectation
    token.evaluate();
}

/******************************************************************************/
/**
 * Retrieve specific header settings for this plot
 *
 * @param is_secondary Whether this plotter is a secondary plotter (true) or a primary one (false)
 * @param p_id The id of the parent plotter, used to build array names for secondary plotters
 * @param own_id This plotter's own emit index, threaded in by the caller (replaces the former mutated id_)
 * @param indent The indentation string prepended to every emitted line
 * @return The header code declaring the x/y/z data arrays for this 3D graph
 */
std::string
GGraph3D::headerData_(bool is_secondary, std::size_t p_id, std::size_t own_id, const std::string &indent) const {
    EmitStream header_data; // NOLINT(cppcoreguidelines-init-variables)

    // Set up suitable arrays for the header
    std::string base_name = suffix(is_secondary, p_id, own_id);
    std::string array_base_name = "array_" + base_name;

    std::string x_array_name = "x_" + array_base_name;
    std::string y_array_name = "y_" + array_base_name;
    std::string z_array_name = "z_" + array_base_name;

    std::string comment; // NOLINT(cppcoreguidelines-init-variables)
    if(!ds_marker_.empty()) {
        comment = "// " + rootEscape(ds_marker_);
    }

    header_data << indent << "double " << x_array_name << "[" << to_string(this->currentSize()) << "];"
                << (!comment.empty() ? comment : "") << '\n'
                << indent << "double " << y_array_name << "[" << to_string(this->currentSize()) << "];"
                << '\n'
                << indent << "double " << z_array_name << "[" << to_string(this->currentSize()) << "];"
                << '\n'
                << '\n';

    return header_data.str();
}

/******************************************************************************/
/**
 * Retrieves the actual data sets
 *
 * @param is_secondary Whether this plotter is a secondary plotter (true) or a primary one (false)
 * @param p_id The id of the parent plotter, used to build array names for secondary plotters
 * @param own_id This plotter's own emit index, threaded in by the caller (replaces the former mutated id_)
 * @param indent The indentation string prepended to every emitted line
 * @return The body code filling the x/y/z data arrays with this graph's tuple values
 */
std::string
GGraph3D::bodyData_(bool is_secondary, std::size_t p_id, std::size_t own_id, const std::string &indent) const {
    EmitStream body_data; // NOLINT(cppcoreguidelines-init-variables)

    // Set up suitable arrays for the header
    std::string base_name = suffix(is_secondary, p_id, own_id);
    std::string array_base_name = "array_" + base_name;

    std::string x_array_name = "x_" + array_base_name;
    std::string y_array_name = "y_" + array_base_name;
    std::string z_array_name = "z_" + array_base_name;

    std::string comment; // NOLINT(cppcoreguidelines-init-variables)
    if(!ds_marker_.empty()) {
        body_data << "// " + rootEscape(ds_marker_) << '\n';
    }

    // Fill data from the columns into the arrays
    const auto &x_col = this->column<0>();
    const auto &y_col = this->column<1>();
    const auto &z_col = this->column<2>();
    const std::size_t n = this->currentSize();

    for(std::size_t pos_counter = 0; pos_counter < n; ++pos_counter) {
        body_data << indent << x_array_name << "[" << pos_counter << "] = " << x_col[pos_counter]
                  << ";"
                  << "\t" << y_array_name << "[" << pos_counter << "] = " << y_col[pos_counter] << ";"
                  << "\t" << z_array_name << "[" << pos_counter << "] = " << z_col[pos_counter] << ";"
                  << '\n';
    }
    body_data << '\n';

    return body_data.str();
}

/******************************************************************************/
/**
 * Retrieves specific draw commands for this plot
 *
 * @param is_secondary Whether this plotter is a secondary plotter (true) or a primary one (false)
 * @param p_id The id of the parent plotter, used to build array/object names for secondary plotters
 * @param own_id This plotter's own emit index, threaded in by the caller (replaces the former mutated id_)
 * @param indent The indentation string prepended to every emitted line
 * @return The footer code creating and drawing the ROOT TGraph2D (and optional poly-line) for this graph
 */
std::string
GGraph3D::footerData_(bool is_secondary, std::size_t p_id, std::size_t own_id, const std::string &indent) const {
    EmitStream footer_data; // NOLINT(cppcoreguidelines-init-variables)

    // Set up suitable arrays for the header
    std::string base_name = suffix(is_secondary, p_id, own_id);
    std::string array_base_name = "array_" + base_name;

    std::string x_array_name = "x_" + array_base_name;
    std::string y_array_name = "y_" + array_base_name;
    std::string z_array_name = "z_" + array_base_name;

    std::string graph_name = std::string("graph_") + base_name;

    std::string comment; // NOLINT(cppcoreguidelines-init-variables)
    if(!ds_marker_.empty()) {
        footer_data << "// " + rootEscape(ds_marker_) << '\n';
    }

    // Check whether custom drawing arguments have been set or whether one
    // of our generic choices has been selected
    std::string d_a = this->drawingArguments(is_secondary);

    // Fill the data in our columns into a ROOT TGraph object
    footer_data << indent << "TGraph2D *" << graph_name << " = new TGraph2D(" << this->currentSize()
                << ", " << x_array_name << ", " << y_array_name << ", " << z_array_name << ");"
                << '\n'
                << indent << graph_name << "->GetXaxis()->SetTitle(\"" << rootEscape(xAxisLabel()) << "\");"
                << '\n'
                << indent << graph_name << "->GetXaxis()->SetTitleOffset(1.5);" << '\n'
                << indent << graph_name << "->GetYaxis()->SetTitle(\"" << rootEscape(yAxisLabel()) << "\");"
                << '\n'
                << indent << graph_name << "->GetYaxis()->SetTitleOffset(1.5);" << '\n'
                << indent << graph_name << "->GetZaxis()->SetTitle(\"" << rootEscape(zAxisLabel()) << "\");"
                << '\n'
                << indent << graph_name << "->GetZaxis()->SetTitleOffset(1.5);" << '\n'
                << indent << graph_name << "->SetMarkerStyle(20);" << '\n'
                << indent << graph_name << "->SetMarkerSize(1);" << '\n'
                << indent << graph_name << "->SetMarkerColor(2);" << '\n';

    if(!plot_label_.empty()) {
        footer_data << indent << graph_name << "->SetTitle(\"" << rootEscape(plot_label_) << "\");" << '\n';
    }
    else {
        footer_data << indent << graph_name << "->SetTitle(\" \");" << '\n';
    }

    footer_data << indent << graph_name << "->Draw(\"" << d_a << "\");" << '\n' << '\n';

    if(draw_lines_ && this->currentSize() >= 2) {
        const auto &x_col = this->column<0>();
        const auto &y_col = this->column<1>();
        const auto &z_col = this->column<2>();
        const std::size_t n = this->currentSize();

        double x = 0.0;
        double y = 0.0;
        double z = 0.0;

        footer_data << indent << "TPolyLine3D *lines_" << graph_name << " = new TPolyLine3D("
                    << this->currentSize() << ");" << '\n'
                    << '\n';

        for(std::size_t pos_counter = 0; pos_counter < n; ++pos_counter) {
            x = x_col[pos_counter];
            y = y_col[pos_counter];
            z = z_col[pos_counter];

            footer_data << indent << "lines_" << graph_name << "->SetPoint(" << pos_counter << ", "
                        << x << ", " << y << ", " << z << ");";
        }
        footer_data << '\n'
                    << indent << "lines_" << graph_name << "->SetLineWidth(3);" << '\n'
                    << indent << "lines_" << graph_name << "->Draw();" << '\n'
                    << '\n';
    }

    return footer_data.str();
}

/******************************************************************************/
/**
 * Retrieve the current drawing arguments
 *
 * @param is_secondary Whether this plotter is a secondary plotter (true) or a primary one (false)
 * @return The ROOT draw-option string, derived from custom arguments or the default point-draw option
 */
std::string GGraph3D::drawingArguments(bool is_secondary) const {
    std::string d_a;

    if(!this->drawing_arguments_.empty()) {
        d_a = this->drawing_arguments_;
    }
    else {
        d_a = "P";

        if(is_secondary) {
            d_a = d_a + ",same";
        }
    }

    return d_a;
}

/******************************************************************************/
/**
 * Creates a deep clone of this object
 *
 * @return A deep copy of this object, returned as a GBasePlotter pointer
 */
GBasePlotter *GGraph3D::clone_() const {
    return new GGraph3D(*this);
}

/******************************************************************************/
/**
 * Loads the data of another object
 *
 * @param cp A constant pointer to another object (as a GBasePlotter) whose data is loaded into this one
 */
void GGraph3D::load_(const GBasePlotter *cp) {
    // Check that we are dealing with a GGraph3D reference independent of this object and convert the pointer
    const auto *p_load = g_convert_and_compare(cp, this);

    // Load our parent class'es data ...
    GDataCollector3T<double, double, double>::load_(cp);

    // ... and then our local data, derived from the single localMembers() declaration
    g_load_members(localMembers_(*this), localMembers_(*p_load));
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * Allows to set the minimum marker size
 *
 * @param min_marker_size The minimum marker size; must be non-negative, otherwise an exception is thrown
 */
void GGraph4D::setMinMarkerSize(const double &min_marker_size) {
    if(min_marker_size < 0.) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GGraph4D::setMinMarkerSize(): Error!" << '\n'
            << "Received invalid minimum marker size: " << min_marker_size << '\n'
        );
    }

    min_marker_size_ = min_marker_size;
}

/******************************************************************************/
/**
 * Allows to set the maximum marker size
 *
 * @param max_marker_size The maximum marker size; must be non-negative and not smaller than the
 * previously set minimum marker size, otherwise an exception is thrown
 */
void GGraph4D::setMaxMarkerSize(const double &max_marker_size) {
    if(max_marker_size < 0. || max_marker_size < min_marker_size_) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GGraph4D::setMinMarkerSize(): Error!" << '\n'
            << "Received invalid minimum marker size: " << min_marker_size_ << " " << max_marker_size
            << "." << '\n'
            << "Always set the lower boundary first." << '\n'
        );
    }

    max_marker_size_ = max_marker_size;
}

/******************************************************************************/
/**
 * Allows to retrieve the minimum marker size
 *
 * @return The currently configured minimum marker size
 */
double GGraph4D::getMinMarkerSize() const {
    return min_marker_size_;
}

/******************************************************************************/
/**
 * Allows to retrieve the maximum marker size
 *
 * @return The currently configured maximum marker size
 */
double GGraph4D::getMaxMarkerSize() const {
    return max_marker_size_;
}

/******************************************************************************/
/**
 * Allows to specify whether small w yield large markers
 *
 * @param swlm If true, small fourth-component (w) values are mapped to large markers; if false, the reverse
 */
void GGraph4D::setSmallWLargeMarker(const bool &swlm) {
    small_w_large_marker_ = swlm;
}

/******************************************************************************/
/**
 * Allows to check whether small w yield large markers
 *
 * @return true if small fourth-component (w) values are mapped to large markers, false otherwise
 */
bool GGraph4D::getSmallWLargeMarker() const {
    return small_w_large_marker_;
}

/******************************************************************************/
/**
 * Allows to set the number of solutions the class should show. Setting the value
 * to 0 will result in all data being displayed.
 *
 * @param n_best The number of (best) solutions to display; 0 means display all data points
 */
void GGraph4D::setNBest(const std::size_t &n_best) {
    n_best_ = n_best;
}

/******************************************************************************/
/**
 * Allows to retrieve the number of solutions the class should show
 *
 * @return The number of (best) solutions to display; 0 means all data points are shown
 */
std::size_t GGraph4D::getNBest() const {
    return n_best_;
}

/******************************************************************************/
/**
 * Retrieves a unique name for this plotter
 *
 * @return A unique name identifying this plotter type
 */
std::string GGraph4D::getPlotterName() const {
    return "GGraph4D";
}

/******************************************************************************/
/**
 * Reports this plotter's choice as a GPlotSpec (a 4-d xyzw graph).
 *
 * @return A GPlotSpec describing this GGraph4D
 */
GPlotSpec GGraph4D::plotSpec() const {
    GPlotSpec spec = GBasePlotter::plotSpec();
    spec.kind = plotKind::graph_4d;
    spec.role = defaultRole(spec.kind);
    spec.columns = {"x", "y", "z", "w"};
    return spec;
}

/******************************************************************************/
/**
 * Returns the name of this class
 *
 * @return The name of this class as a string
 */
std::string GGraph4D::name_() const {
    return std::string("GGraph4D");
}

/******************************************************************************/
/**
 * Searches for compliance with expectations with respect to another object
 * of the same type
 *
 * @param cp A constant reference to another object, passed as a GBasePlotter reference
 * @param e The expectation (equality / inequality) the comparison should fulfil
 * @param limit The acceptable tolerance for floating point comparisons (unused here)
 */
void GGraph4D::compare_(
    const GBasePlotter &cp,
    const expectation &e,
    [[maybe_unused]] const double & limit
) const {
    // Check that we are dealing with a GGraph4D reference independent of this object and convert the pointer
    const auto *p_load = g_convert_and_compare(cp, this);

    GToken token("GGraph4D", e);

    // Compare our parent data ...
    compare_base_t<GDataCollector4T<double, double, double, double>>(*this, *p_load, token);

    // ... and then the local data, derived from the single localMembers() declaration
    g_compare_members(localMembers_(*this), localMembers_(*p_load), token);

    // React on deviations from the expectation
    token.evaluate();
}

/******************************************************************************/
/**
 * Retrieve specific header settings for this plot. The three positional parameters
 * (is_secondary flag, parent id and indentation string) are unused because this 4D
 * graph emits all of its ROOT code in the footer section.
 *
 * @return An empty string, as this 4D graph emits no header code
 */
std::string GGraph4D::headerData_([[maybe_unused]] bool is_secondary, [[maybe_unused]] std::size_t parent_id, [[maybe_unused]] std::size_t own_id, [[maybe_unused]] std::string const &indent) const {
    EmitStream header_data; // NOLINT(cppcoreguidelines-init-variables)

    // nothing

    return header_data.str();
}

/******************************************************************************/
/**
 * Retrieves the actual data sets. The three positional parameters (is_secondary flag,
 * parent id and indentation string) are unused because this 4D graph emits all of its
 * ROOT code in the footer section.
 *
 * @return An empty string, as this 4D graph emits no body data
 */
std::string GGraph4D::bodyData_([[maybe_unused]] bool is_secondary, [[maybe_unused]] std::size_t parent_id, [[maybe_unused]] std::size_t own_id, [[maybe_unused]] std::string const &indent) const {
    EmitStream body_data; // NOLINT(cppcoreguidelines-init-variables)

    // nothing

    return body_data.str();
}

/******************************************************************************/
/**
 * Retrieves specific draw commands for this plot
 *
 * @param is_secondary Whether this plotter is a secondary plotter (true) or a primary one (false)
 * @param p_id The id of the parent plotter, used to build unique object names for secondary plotters
 * @param own_id This plotter's own emit index, threaded in by the caller (replaces the former mutated id_)
 * @param indent The indentation string prepended to every emitted line
 * @return The footer code creating the 3D frame and per-point poly-markers (sized by the fourth component)
 */
std::string
GGraph4D::footerData_(bool is_secondary, std::size_t p_id, std::size_t own_id, const std::string &indent) const {
    // Read the four columns directly. Rather than copying the whole data set to
    // sort it on every emission, we sort an index permutation by the w-component
    // (axis 3) and read each point through that permutation; the columns stay put.
    const auto &x_col = this->column<0>();
    const auto &y_col = this->column<1>();
    const auto &z_col = this->column<2>();
    const auto &w_col = this->column<3>();
    const std::size_t data_size = this->currentSize();

    std::string base_name = suffix(is_secondary, p_id, own_id);

    // Build the w-ordered index permutation, so we can select the n_best_ best more easily
    std::vector<std::size_t> order(data_size);
    for(std::size_t i = 0; i < data_size; ++i) {
        order[i] = i;
    }
    if(small_w_large_marker_) {
        std::ranges::sort(order, [&w_col](std::size_t a, std::size_t b) -> bool {
            return (w_col[a] < w_col[b]);
        });
    }
    else {
        std::ranges::sort(order, [&w_col](std::size_t a, std::size_t b) -> bool {
            return (w_col[a] > w_col[b]);
        });
    }

    EmitStream footer_data; // NOLINT(cppcoreguidelines-init-variables)

    // Find out about the minimum and maximum values of the data set. This preserves
    // the previous getMinMax(4D) contract, including its requirement of at least two
    // data items, while reading straight from the columns (no copy).
    if(data_size < static_cast<std::size_t>(2)) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GBasePlotter::getMinMax(4D): Error!" << '\n'
            << "Got vector of invalid size " << data_size << '\n'
        );
    }
    std::tuple<double, double, double, double, double, double, double, double> min_max{
        *std::ranges::min_element(x_col),
        *std::ranges::max_element(x_col),
        *std::ranges::min_element(y_col),
        *std::ranges::max_element(y_col),
        *std::ranges::min_element(z_col),
        *std::ranges::max_element(z_col),
        *std::ranges::min_element(w_col),
        *std::ranges::max_element(w_col)
    };

    // Set up TView object for our 3D data, spanning the minimum and maximum values
    footer_data << indent << R"(TH3F *fr = new TH3F("fr","fr",)"
                << "10, " << std::get<0>(min_max) << ", " << std::get<1>(min_max) << ", "
                << "10, " << std::get<2>(min_max) << ", " << std::get<3>(min_max) << ", "
                << "10, " << std::get<4>(min_max) << ", " << std::get<5>(min_max) << ");" << '\n'
                << indent << "fr->SetTitle(\" \");" << '\n'
                << indent << "fr->GetXaxis()->SetTitle(\"" << rootEscape(xAxisLabel()) << "\");" << '\n'
                << indent << "fr->GetXaxis()->SetTitleOffset(1.6);" << '\n'
                << indent << "fr->GetYaxis()->SetTitle(\"" << rootEscape(yAxisLabel()) << "\");" << '\n'
                << indent << "fr->GetYaxis()->SetTitleOffset(1.6);" << '\n'
                << indent << "fr->GetZaxis()->SetTitle(\"" << rootEscape(zAxisLabel()) << "\");" << '\n'
                << indent << "fr->GetZaxis()->SetTitleOffset(1.6);" << '\n'
                << '\n'
                << indent << "fr->Draw();" << '\n';

    double w_min = std::get<6>(min_max);
    double w_max = std::get<7>(min_max);

    // Fill data from the columns into the arrays, following the w-ordered permutation
    double w_range = w_max - w_min;
    std::size_t pos = 0;
    for(std::size_t idx : order) {
        std::string poly_marker_name =
            std::string("pm3d_") + base_name + std::string("_") + to_string(pos);

        // create a TPolyMarker3D for a single data point
        footer_data << indent << "TPolyMarker3D *" << poly_marker_name << " = new TPolyMarker3D(1);"
                    << '\n';

        double x = x_col[idx];
        double y = y_col[idx];
        double z = z_col[idx];
        double w = w_col[idx];

        // Translate the fourth component into a marker size. By default,
        // smaller values will yield the largest value
        double marker_size = 0.;
        if(0 == pos) {
            marker_size = 2 * max_marker_size_;
        }
        else {
            if(small_w_large_marker_) {
                marker_size = min_marker_size_ + ((max_marker_size_ - min_marker_size_) *
                                                   pow((1. - ((w - w_min) / w_range)), 8.));
            }
            else {
                marker_size = min_marker_size_ +
                              ((max_marker_size_ - min_marker_size_) * pow(((w - w_min) / w_range), 8));
            }
        }

        footer_data << indent << poly_marker_name << "->SetPoint(" << pos << ", " << x << ", " << y
                    << ", " << z << "); // w = " << w << '\n'
                    << indent << poly_marker_name << "->SetMarkerSize(" << marker_size << ");"
                    << '\n'
                    << indent << poly_marker_name << "->SetMarkerColor(" << (0 == pos ? 4 : 2)
                    << ");" << '\n'
                    << indent << poly_marker_name << "->SetMarkerStyle(8);" << '\n'
                    << indent << poly_marker_name << "->Draw();" << '\n'
                    << '\n';

        pos++;

        if(n_best_ && pos >= n_best_) {
            break;
        }
    }

    footer_data << '\n';

    return footer_data.str();
}

/******************************************************************************/
/**
 * Retrieve the current drawing arguments. The is_secondary flag parameter is unused,
 * as this 4D graph builds its draw commands entirely in the footer section.
 *
 * @return An empty string, as no generic draw-option string is used by this plotter
 */
std::string GGraph4D::drawingArguments([[maybe_unused]] bool is_secondary) const {
    std::string d_a;

    // nothing

    return d_a;
}

/******************************************************************************/
/**
 * Creates a deep clone of this object
 *
 * @return A deep copy of this object, returned as a GBasePlotter pointer
 */
GBasePlotter *GGraph4D::clone_() const {
    return new GGraph4D(*this);
}

/******************************************************************************/
/**
 * Loads the data of another object
 *
 * @param cp A constant pointer to another object (as a GBasePlotter) whose data is loaded into this one
 */
void GGraph4D::load_(const GBasePlotter *cp) {
    // Check that we are dealing with a GGraph4D reference independent of this object and convert the pointer
    const auto *p_load = g_convert_and_compare(cp, this);

    // Load our parent class'es data ...
    GDataCollector4T<double, double, double, double>::load_(cp);

    // ... and then our local data, derived from the single localMembers() declaration
    g_load_members(localMembers_(*this), localMembers_(*p_load));
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * Initialization with number of bins and automatic range detection
 *
 * @param n_bins_x The number of bins along the x-axis (the range is detected automatically from the data)
 */
GHistogram1D::GHistogram1D(const std::size_t &n_bins_x)
  : n_bins_x_(n_bins_x) { /* nothing */
}

/******************************************************************************/
/**
 * Initialization with the number of bins and an explicit x-range
 *
 * @param n_bins_x The number of bins along the x-axis
 * @param min_x The lower boundary of the x-axis range
 * @param max_x The upper boundary of the x-axis range
 */
GHistogram1D::GHistogram1D(const std::size_t &n_bins_x, const double &min_x, const double &max_x)
  : n_bins_x_(n_bins_x)
  , min_x_(min_x)
  , max_x_(max_x) { /* nothing */
}

/******************************************************************************/
/**
 * Initialization with a range in the form of a tuple
 *
 * @param n_bins_x The number of bins along the x-axis
 * @param range_x A tuple holding the lower (get<0>) and upper (get<1>) boundary of the x-axis range
 */
GHistogram1D::GHistogram1D(const std::size_t &n_bins_x, const std::tuple<double, double> &range_x)
  : n_bins_x_(n_bins_x)
  , min_x_(std::get<0>(range_x))
  , max_x_(std::get<1>(range_x)) { /* nothing */
}

/******************************************************************************/
/**
 * Retrieve specific header settings for this plot
 *
 * @param is_secondary Whether this plotter is a secondary plotter (true) or a primary one (false)
 * @param p_id The id of the parent plotter, used to build a unique histogram name for secondary plotters
 * @param own_id This plotter's own emit index, threaded in by the caller (replaces the former mutated id_)
 * @param indent The indentation string prepended to every emitted line
 * @return The header code declaring the ROOT TH1D histogram (using explicit or auto-detected range)
 */
std::string
GHistogram1D::headerData_(bool is_secondary, std::size_t p_id, std::size_t own_id, const std::string &indent) const {
    EmitStream header_data; // NOLINT(cppcoreguidelines-init-variables)

    std::string comment; // NOLINT(cppcoreguidelines-init-variables)
    if(!ds_marker_.empty()) {
        comment = "// " + rootEscape(ds_marker_);
    }

    std::string hist_name = "histD" + suffix(is_secondary, p_id, own_id);

    if(min_x_ != max_x_) {
        header_data << indent << "TH1D *" << hist_name << " = new TH1D(\"" << hist_name << "\", \""
                    << hist_name << "\"," << n_bins_x_ << ", " << min_x_ << ", " << max_x_ << ");"
                    << (!comment.empty() ? comment : "") << '\n'
                    << '\n';
    }
    else { // automatic range detection
        std::tuple<double, double> minmax = this->getMinMaxElements();
        header_data << indent << "TH1D *" << hist_name << " = new TH1D(\"" << hist_name << "\", \""
                    << hist_name << "\"," << n_bins_x_ << ", " << std::get<0>(minmax) << ", "
                    << std::get<1>(minmax) << ");" << (!comment.empty() ? comment : "") << '\n'
                    << '\n';
    }

    return header_data.str();
}

/******************************************************************************/
/**
 * Retrieves the actual data sets
 *
 * @param is_secondary Whether this plotter is a secondary plotter (true) or a primary one (false)
 * @param p_id The id of the parent plotter, used to build a unique histogram name for secondary plotters
 * @param own_id This plotter's own emit index, threaded in by the caller (replaces the former mutated id_)
 * @param indent The indentation string prepended to every emitted line
 * @return The body code filling the ROOT TH1D histogram with this plotter's data values
 */
std::string
GHistogram1D::bodyData_(bool is_secondary, std::size_t p_id, std::size_t own_id, const std::string &indent) const {
    EmitStream body_data; // NOLINT(cppcoreguidelines-init-variables)

    std::string comment; // NOLINT(cppcoreguidelines-init-variables)
    if(!ds_marker_.empty()) {
        comment = "// " + rootEscape(ds_marker_);
    }
    else {
        comment = "";
    }

    std::string hist_name = "histD" + suffix(is_secondary, p_id, own_id);

    const auto &x_col = this->column<0>();
    const std::size_t n = this->currentSize();
    for(std::size_t pos_counter = 0; pos_counter < n; ++pos_counter) {
        body_data << indent << hist_name << "->Fill(" << std::showpoint << x_col[pos_counter] << ");"
                  << (pos_counter == 0 ? comment : ("")) << '\n';
    }
    body_data << '\n';

    return body_data.str();
}

/******************************************************************************/
/**
 * Retrieves specific draw commands for this plot
 *
 * @param is_secondary Whether this plotter is a secondary plotter (true) or a primary one (false)
 * @param p_id The id of the parent plotter, used to build a unique histogram name for secondary plotters
 * @param own_id This plotter's own emit index, threaded in by the caller (replaces the former mutated id_)
 * @param indent The indentation string prepended to every emitted line
 * @return The footer code setting titles and drawing the ROOT TH1D histogram
 */
std::string
GHistogram1D::footerData_(bool is_secondary, std::size_t p_id, std::size_t own_id, const std::string &indent) const {
    EmitStream footer_data; // NOLINT(cppcoreguidelines-init-variables)

    std::string hist_name = "histD" + suffix(is_secondary, p_id, own_id);

    if(!plot_label_.empty()) {
        footer_data << indent << hist_name << "->SetTitle(\"" << rootEscape(plot_label_) << "\");" << '\n';
    }
    else {
        footer_data << indent << hist_name << "->SetTitle(\" \");" << '\n';
    }

    std::string comment; // NOLINT(cppcoreguidelines-init-variables)
    if(!ds_marker_.empty()) {
        footer_data << "// " + rootEscape(ds_marker_) << '\n';
    }

    // Check whether custom drawing arguments have been set
    std::string d_a = this->drawingArguments(is_secondary);

    footer_data << indent << hist_name << "->GetXaxis()->SetTitle(\"" << rootEscape(xAxisLabel()) << "\");"
                << '\n'
                << indent << hist_name << "->GetYaxis()->SetTitle(\"" << rootEscape(yAxisLabel()) << "\");"
                << '\n'
                << indent << hist_name << "->Draw(\"" << d_a << "\");" << '\n'
                << '\n';

    return footer_data.str();
}

/******************************************************************************/
/**
 * Retrieve the current drawing arguments
 *
 * @param is_secondary Whether this plotter is a secondary plotter (true) or a primary one (false)
 * @return The ROOT draw-option string; secondary plotters get the "same" option appended
 */
std::string GHistogram1D::drawingArguments(bool is_secondary) const {
    std::string d_a;

    if(!drawing_arguments_.empty()) {
        d_a = drawing_arguments_;
    }
    else {
        if(is_secondary) {
            if(d_a.empty()) {
                d_a = "same";
            }
            else {
                d_a = d_a + ",same";
            }
        }
    }

    return d_a;
}

/******************************************************************************/
/**
 * Retrieve the number of bins in x-direction
 *
 * @return The number of bins in x-direction
 */
std::size_t GHistogram1D::getNBinsX() const {
    return n_bins_x_;
}

/******************************************************************************/
/**
 * Retrieve the lower boundary of the plot
 *
 * @return The lower boundary of the plot
 */
double GHistogram1D::getMinX() const {
    return min_x_;
}

/******************************************************************************/
/**
 * Retrieve the upper boundary of the plot
 *
 * @return The upper boundary of the plot
 */
double GHistogram1D::getMaxX() const {
    return max_x_;
}

/******************************************************************************/
/**
 * Retrieves a unique name for this plotter
 *
 * @return A unique name identifying this plotter type
 */
std::string GHistogram1D::getPlotterName() const {
    return "GHistogram1D";
}

/******************************************************************************/
/**
 * Reports this plotter's choice as a GPlotSpec (a 1-d histogram of double samples).
 * The single column holds the raw (unbinned) sample values; n_bins_x carries the
 * histogram's bin count.
 *
 * @return A GPlotSpec describing this GHistogram1D
 */
GPlotSpec GHistogram1D::plotSpec() const {
    GPlotSpec spec = GBasePlotter::plotSpec();
    spec.kind = plotKind::hist_1d;
    spec.role = defaultRole(spec.kind);
    spec.columns = {"value"};
    spec.n_bins_x = n_bins_x_;
    // A fixed value range is signalled by min != max (min == max means auto-range from data).
    if(min_x_ != max_x_) {
        spec.range_x = std::make_tuple(min_x_, max_x_);
    }
    return spec;
}

/******************************************************************************/
/**
 * Returns the name of this class
 *
 * @return The name of this class as a string
 */
std::string GHistogram1D::name_() const {
    return std::string("GHistogram1D");
}

/******************************************************************************/
/**
 * Searches for compliance with expectations with respect to another object
 * of the same type
 *
 * @param cp A constant reference to another object, passed as a GBasePlotter reference
 * @param e The expectation (equality / inequality) the comparison should fulfil
 * @param limit The acceptable tolerance for floating point comparisons (unused here)
 */
void GHistogram1D::compare_(
    const GBasePlotter &cp,
    const expectation &e,
    [[maybe_unused]] const double & limit
) const {
    // Check that we are dealing with a GBasePlotter reference independent of this object and convert the pointer
    const auto *p_load = g_convert_and_compare(cp, this);

    GToken token("GHistogram1D", e);

    // Compare our parent data ...
    compare_base_t<GDataCollector1T<double>>(*this, *p_load, token);

    // ... and then the local data, derived from the single localMembers() declaration
    g_compare_members(localMembers_(*this), localMembers_(*p_load), token);

    // React on deviations from the expectation
    token.evaluate();
}

/******************************************************************************/
/**
 * Creates a deep clone of this object
 *
 * @return A deep copy of this object, returned as a GBasePlotter pointer
 */
GBasePlotter *GHistogram1D::clone_() const {
    return new GHistogram1D(*this);
}

/******************************************************************************/
/**
 * Loads the data of another object
 *
 * @param cp A constant pointer to another object (as a GBasePlotter) whose data is loaded into this one
 */
void GHistogram1D::load_(const GBasePlotter *cp) {
    // Check that we are dealing with a GHistogram1D reference independent of this object and convert the pointer
    const auto *p_load = g_convert_and_compare(cp, this);

    // Load our parent class'es data ...
    GDataCollector1T<double>::load_(cp);

    // ... and then our local data, derived from the single localMembers() declaration
    g_load_members(localMembers_(*this), localMembers_(*p_load));
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * The standard constructor
 *
 * @param n_bins_x The number of bins along the x-axis
 * @param min_x The lower boundary of the x-axis range
 * @param max_x The upper boundary of the x-axis range
 */
GHistogram1I::GHistogram1I(const std::size_t &n_bins_x, const double &min_x, const double &max_x)
  : n_bins_x_(n_bins_x)
  , min_x_(min_x)
  , max_x_(max_x) { /* nothing */
}

/******************************************************************************/
/**
 * Initialization with a range in the form of a tuple
 *
 * @param n_bins_x The number of bins along the x-axis
 * @param range_x A tuple holding the lower (get<0>) and upper (get<1>) boundary of the x-axis range
 */
GHistogram1I::GHistogram1I(const std::size_t &n_bins_x, const std::tuple<double, double> &range_x)
  : n_bins_x_(n_bins_x)
  , min_x_(std::get<0>(range_x))
  , max_x_(std::get<1>(range_x)) { /* nothing */
}

/******************************************************************************/
/**
 * Retrieve specific header settings for this plot
 *
 * @param is_secondary Whether this plotter is a secondary plotter (true) or a primary one (false)
 * @param p_id The id of the parent plotter, used to build a unique histogram name for secondary plotters
 * @param own_id This plotter's own emit index, threaded in by the caller (replaces the former mutated id_)
 * @param indent The indentation string prepended to every emitted line
 * @return The header code declaring the ROOT TH1I integer histogram
 */
std::string
GHistogram1I::headerData_(bool is_secondary, std::size_t p_id, std::size_t own_id, const std::string &indent) const {
    EmitStream header_data; // NOLINT(cppcoreguidelines-init-variables)

    std::string comment; // NOLINT(cppcoreguidelines-init-variables)
    if(!ds_marker_.empty()) {
        comment = "// " + rootEscape(ds_marker_);
    }

    std::string hist_name = "histI" + suffix(is_secondary, p_id, own_id);

    header_data << indent << "TH1I *" << hist_name << " = new TH1I(\"" << hist_name << "\", \""
                << hist_name << "\"," << n_bins_x_ << ", " << min_x_ << ", " << max_x_ << ");"
                << (!comment.empty() ? comment : "") << '\n'
                << '\n';

    return header_data.str();
}

/******************************************************************************/
/**
 * Retrieves the actual data sets
 *
 * @param is_secondary Whether this plotter is a secondary plotter (true) or a primary one (false)
 * @param p_id The id of the parent plotter, used to build a unique histogram name for secondary plotters
 * @param own_id This plotter's own emit index, threaded in by the caller (replaces the former mutated id_)
 * @param indent The indentation string prepended to every emitted line
 * @return The body code filling the ROOT TH1I histogram with this plotter's integer data values
 */
std::string
GHistogram1I::bodyData_(bool is_secondary, std::size_t p_id, std::size_t own_id, const std::string &indent) const {
    EmitStream body_data; // NOLINT(cppcoreguidelines-init-variables)

    std::string comment; // NOLINT(cppcoreguidelines-init-variables)
    if(!ds_marker_.empty()) {
        comment = "// " + rootEscape(ds_marker_);
    }
    else {
        comment = "";
    }

    std::string hist_name = "histI" + suffix(is_secondary, p_id, own_id);

    const auto &x_col = this->column<0>();
    const std::size_t n = this->currentSize();
    for(std::size_t pos_counter = 0; pos_counter < n; ++pos_counter) {
        body_data << indent << hist_name << "->Fill(" << x_col[pos_counter] << ");"
                  << (pos_counter == 0 ? comment : ("")) << '\n';
    }

    body_data << '\n';

    return body_data.str();
}

/******************************************************************************/
/**
 * Retrieves specific draw commands for this plot
 *
 * @param is_secondary Whether this plotter is a secondary plotter (true) or a primary one (false)
 * @param p_id The id of the parent plotter, used to build a unique histogram name for secondary plotters
 * @param own_id This plotter's own emit index, threaded in by the caller (replaces the former mutated id_)
 * @param indent The indentation string prepended to every emitted line
 * @return The footer code setting titles and drawing the ROOT TH1I histogram
 */
std::string
GHistogram1I::footerData_(bool is_secondary, std::size_t p_id, std::size_t own_id, const std::string &indent) const {
    EmitStream footer_data; // NOLINT(cppcoreguidelines-init-variables)

    std::string hist_name = "histI" + suffix(is_secondary, p_id, own_id);

    if(!plot_label_.empty()) {
        footer_data << indent << hist_name << "->SetTitle(\"" << rootEscape(plot_label_) << "\");" << '\n';
    }
    else {
        footer_data << indent << hist_name << "->SetTitle(\" \");" << '\n';
    }

    std::string comment; // NOLINT(cppcoreguidelines-init-variables)
    if(!ds_marker_.empty()) {
        footer_data << "// " + rootEscape(ds_marker_) << '\n';
    }

    // Check whether custom drawing arguments have been set
    std::string d_a = this->drawingArguments(is_secondary);

    footer_data << indent << hist_name << "->GetXaxis()->SetTitle(\"" << rootEscape(xAxisLabel()) << "\");"
                << '\n'
                << indent << hist_name << "->GetYaxis()->SetTitle(\"" << rootEscape(yAxisLabel()) << "\");"
                << '\n'
                << indent << hist_name << "->Draw(\"" << d_a << "\");" << '\n'
                << '\n';

    return footer_data.str();
}

/******************************************************************************/
/**
 * Retrieve the current drawing arguments
 *
 * @param is_secondary Whether this plotter is a secondary plotter (true) or a primary one (false)
 * @return The ROOT draw-option string; secondary plotters get the "same" option appended
 */
std::string GHistogram1I::drawingArguments(bool is_secondary) const {
    std::string d_a;

    if(!drawing_arguments_.empty()) {
        d_a = drawing_arguments_;
    }
    else {
        if(is_secondary) {
            if(d_a.empty()) {
                d_a = "same";
            }
            else {
                d_a = d_a + ",same";
            }
        }
    }

    return d_a;
}

/******************************************************************************/
/**
 * Retrieve the number of bins in x-direction
 *
 * @return The number of bins in x-direction
 */
std::size_t GHistogram1I::getNBinsX() const {
    return n_bins_x_;
}

/******************************************************************************/
/**
 * Retrieve the lower boundary of the plot
 *
 * @return The lower boundary of the plot
 */
double GHistogram1I::getMinX() const {
    return min_x_;
}

/******************************************************************************/
/**
 * Retrieve the upper boundary of the plot
 *
 * @return The upper boundary of the plot
 */
double GHistogram1I::getMaxX() const {
    return max_x_;
}

/******************************************************************************/
/**
 * Retrieves a unique name for this plotter
 *
 * @return A unique name identifying this plotter type
 */
std::string GHistogram1I::getPlotterName() const {
    return "GHistogram1I";
}

/******************************************************************************/
/**
 * Reports this plotter's choice as a GPlotSpec (a 1-d histogram of integer samples).
 *
 * @return A GPlotSpec describing this GHistogram1I
 */
GPlotSpec GHistogram1I::plotSpec() const {
    GPlotSpec spec = GBasePlotter::plotSpec();
    spec.kind = plotKind::hist_1i;
    spec.role = defaultRole(spec.kind);
    spec.columns = {"value"};
    spec.n_bins_x = n_bins_x_;
    // An integer histogram has no auto-range ctor; emit its range whenever it is fixed
    // (min != max), which lets makePlotter reconstruct it.
    if(min_x_ != max_x_) {
        spec.range_x = std::make_tuple(min_x_, max_x_);
    }
    return spec;
}

/******************************************************************************/
/**
 * Returns the name of this class
 *
 * @return The name of this class as a string
 */
std::string GHistogram1I::name_() const {
    return std::string("GHistogram1I");
}

/******************************************************************************/
/**
 * Searches for compliance with expectations with respect to another object
 * of the same type
 *
 * @param cp A constant reference to another object, passed as a GBasePlotter reference
 * @param e The expectation (equality / inequality) the comparison should fulfil
 * @param limit The acceptable tolerance for floating point comparisons (unused here)
 */
void GHistogram1I::compare_(
    const GBasePlotter &cp,
    const expectation &e,
    [[maybe_unused]] const double & limit
) const {
    // Check that we are dealing with a GBasePlotter reference independent of this object and convert the pointer
    const auto *p_load = g_convert_and_compare(cp, this);

    GToken token("GHistogram1I", e);

    // Compare our parent data ...
    compare_base_t<GDataCollector1T<std::int32_t>>(*this, *p_load, token);

    // ... and then the local data, derived from the single localMembers() declaration
    g_compare_members(localMembers_(*this), localMembers_(*p_load), token);

    // React on deviations from the expectation
    token.evaluate();
}

/******************************************************************************/
/**
 * Creates a deep clone of this object
 *
 * @return A deep copy of this object, returned as a GBasePlotter pointer
 */
GBasePlotter *GHistogram1I::clone_() const {
    return new GHistogram1I(*this);
}

/******************************************************************************/
/**
 * Loads the data of another object
 *
 * @param cp A constant pointer to another object (as a GBasePlotter) whose data is loaded into this one
 */
void GHistogram1I::load_(const GBasePlotter *cp) {
    // Check that we are dealing with a GHistogram1I reference independent of this object and convert the pointer
    const auto *p_load = g_convert_and_compare(cp, this);

    // Load our parent class'es data ...
    GDataCollector1T<std::int32_t>::load_(cp);

    // ... and then our local data, derived from the single localMembers() declaration
    g_load_members(localMembers_(*this), localMembers_(*p_load));
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * The standard constructor
 *
 * @param n_bins_x The number of bins along the x-axis
 * @param n_bins_y The number of bins along the y-axis
 * @param min_x The lower boundary of the x-axis range
 * @param max_x The upper boundary of the x-axis range
 * @param min_y The lower boundary of the y-axis range
 * @param max_y The upper boundary of the y-axis range
 */
GHistogram2D::GHistogram2D(
    const std::size_t &n_bins_x,
    const std::size_t &n_bins_y,
    const double &min_x,
    const double &max_x,
    const double &min_y,
    const double &max_y
)
  : n_bins_x_(n_bins_x)
  , n_bins_y_(n_bins_y)
  , min_x_(min_x)
  , max_x_(max_x)
  , min_y_(min_y)
  , max_y_(max_y)
  , dropt_(tddropt::TDEMPTY) { /* nothing */
}

/******************************************************************************/
/**
 * Initialization with ranges
 *
 * @param n_bins_x The number of bins along the x-axis
 * @param n_bins_y The number of bins along the y-axis
 * @param range_x A tuple holding the lower (get<0>) and upper (get<1>) boundary of the x-axis range
 * @param range_y A tuple holding the lower (get<0>) and upper (get<1>) boundary of the y-axis range
 */
GHistogram2D::GHistogram2D(
    const std::size_t &n_bins_x,
    const std::size_t &n_bins_y,
    const std::tuple<double, double> &range_x,
    const std::tuple<double, double> &range_y
)
  : n_bins_x_(n_bins_x)
  , n_bins_y_(n_bins_y)
  , min_x_(std::get<0>(range_x))
  , max_x_(std::get<1>(range_x))
  , min_y_(std::get<0>(range_y))
  , max_y_(std::get<1>(range_y))
  , dropt_(tddropt::TDEMPTY) { /* nothing */
}

/******************************************************************************/
/**
 * Initialization with automatic range detection
 *
 * @param n_bins_x The number of bins along the x-axis (the x-range is detected automatically)
 * @param n_bins_y The number of bins along the y-axis (the y-range is detected automatically)
 */
GHistogram2D::GHistogram2D(const std::size_t &n_bins_x, const std::size_t &n_bins_y)
  : n_bins_x_(n_bins_x)
  , n_bins_y_(n_bins_y)
  , min_x_(0)
  , max_x_(min_x_)
  , min_y_(0)
  , max_y_(min_y_)
  , dropt_(tddropt::TDEMPTY) { /* nothing */
}

/******************************************************************************/
/**
 * Retrieve specific header settings for this plot
 *
 * @param is_secondary Whether this plotter is a secondary plotter (true) or a primary one (false)
 * @param p_id The id of the parent plotter, used to build a unique histogram name for secondary plotters
 * @param own_id This plotter's own emit index, threaded in by the caller (replaces the former mutated id_)
 * @param indent The indentation string prepended to every emitted line
 * @return The header code declaring the ROOT TH2D histogram (using explicit or auto-detected ranges)
 */
std::string
GHistogram2D::headerData_(bool is_secondary, std::size_t p_id, std::size_t own_id, const std::string &indent) const {
    EmitStream header_data; // NOLINT(cppcoreguidelines-init-variables)

    std::string comment; // NOLINT(cppcoreguidelines-init-variables)
    if(!ds_marker_.empty()) {
        comment = "// " + rootEscape(ds_marker_);
    }

    std::string hist_name = "hist2D" + suffix(is_secondary, p_id, own_id);

    if(min_x_ != max_x_ && min_y_ != max_y_) {
        header_data << indent << "TH2D *" << hist_name << " = new TH2D(\"" << hist_name << "\", \""
                    << hist_name << "\"," << n_bins_x_ << ", " << min_x_ << ", " << max_x_ << ","
                    << n_bins_y_ << ", " << min_y_ << ", " << max_y_ << ");"
                    << (!comment.empty() ? comment : "") << '\n'
                    << '\n';
    }
    else { // // automatic range detection
        std::tuple<double, double, double, double> minmax = this->getMinMaxElements();

        header_data << indent << "TH2D *" << hist_name << " = new TH2D(\"" << hist_name << "\", \""
                    << hist_name << "\"," << n_bins_x_ << ", " << std::get<0>(minmax) << ", "
                    << std::get<1>(minmax) << "," << n_bins_y_ << ", " << std::get<2>(minmax) << ", "
                    << std::get<3>(minmax) << ");" << (!comment.empty() ? comment : "") << '\n'
                    << '\n';
    }

    return header_data.str();
}

/******************************************************************************/
/**
 * Retrieves the actual data sets
 *
 * @param is_secondary Whether this plotter is a secondary plotter (true) or a primary one (false)
 * @param p_id The id of the parent plotter, used to build a unique histogram name for secondary plotters
 * @param own_id This plotter's own emit index, threaded in by the caller (replaces the former mutated id_)
 * @param indent The indentation string prepended to every emitted line
 * @return The body code filling the ROOT TH2D histogram with this plotter's (x, y) data values
 */
std::string
GHistogram2D::bodyData_(bool is_secondary, std::size_t p_id, std::size_t own_id, const std::string &indent) const {
    EmitStream body_data; // NOLINT(cppcoreguidelines-init-variables)

    std::string comment; // NOLINT(cppcoreguidelines-init-variables)
    if(!ds_marker_.empty()) {
        comment = "// " + rootEscape(ds_marker_);
    }
    else {
        comment = "";
    }

    std::string hist_name = "hist2D" + suffix(is_secondary, p_id, own_id);

    const auto &x_col = this->column<0>();
    const auto &y_col = this->column<1>();
    const std::size_t n = this->currentSize();
    for(std::size_t pos_counter = 0; pos_counter < n; ++pos_counter) {
        body_data << indent << hist_name << "->Fill(" << std::showpoint << x_col[pos_counter] << ", "
                  << y_col[pos_counter] << ");" << (pos_counter == 0 ? comment : ("")) << '\n';
    }

    body_data << '\n';

    return body_data.str();
}

/******************************************************************************/
/**
 * Retrieves specific draw commands for this plot
 *
 * @param is_secondary Whether this plotter is a secondary plotter (true) or a primary one (false)
 * @param p_id The id of the parent plotter, used to build a unique histogram name for secondary plotters
 * @param own_id This plotter's own emit index, threaded in by the caller (replaces the former mutated id_)
 * @param indent The indentation string prepended to every emitted line
 * @return The footer code setting titles and drawing the ROOT TH2D histogram
 */
std::string
GHistogram2D::footerData_(bool is_secondary, std::size_t p_id, std::size_t own_id, const std::string &indent) const {
    EmitStream footer_data; // NOLINT(cppcoreguidelines-init-variables)

    std::string hist_name = "hist2D" + suffix(is_secondary, p_id, own_id);

    if(!plot_label_.empty()) {
        footer_data << indent << hist_name << "->SetTitle(\"" << rootEscape(plot_label_) << "\");" << '\n';
    }
    else {
        footer_data << indent << hist_name << "->SetTitle(\" \");" << '\n';
    }

    std::string comment; // NOLINT(cppcoreguidelines-init-variables)
    if(!ds_marker_.empty()) {
        footer_data << "// " + rootEscape(ds_marker_) << '\n';
    }

    // Check whether custom drawing arguments have been set
    std::string d_a = this->drawingArguments(is_secondary);

    footer_data << indent << hist_name << "->GetXaxis()->SetTitle(\"" << rootEscape(xAxisLabel()) << "\");"
                << '\n'
                << indent << hist_name << "->GetYaxis()->SetTitle(\"" << rootEscape(yAxisLabel()) << "\");"
                << '\n'
                << indent << hist_name << "->Draw(\"" << d_a << "\");" << '\n'
                << '\n';

    return footer_data.str();
}

/******************************************************************************/
/**
 * Retrieve the current drawing arguments
 *
 * @param is_secondary Whether this plotter is a secondary plotter (true) or a primary one (false)
 * @return The ROOT draw-option string derived from the 2D drawing option; secondary plotters get
 * the "same" option appended
 */
std::string GHistogram2D::drawingArguments(bool is_secondary) const {
    std::string d_a;

    if(!drawing_arguments_.empty()) {
        d_a = drawing_arguments_;
    }
    else {
        switch(dropt_) {
            using enum tddropt;
        case TDEMPTY:
            d_a = "";
            break;

        case SURFONE:
            d_a = "SURF1";
            break;

        case SURFTWOZ:
            d_a = "SURF2Z";
            break;

        case SURFTHREE:
            d_a = "SURF3";
            break;

        case SURFFOUR:
            d_a = "SURF4";
            break;

        case CONTZ:
            d_a = "CONTZ";
            break;

        case CONTONE:
            d_a = "CONT1";
            break;

        case CONTTWO:
            d_a = "CONT2";
            break;

        case CONTTHREE:
            d_a = "CONT3";
            break;

        case TEXT:
            d_a = "TEXT";
            break;

        case SCAT:
            d_a = "SCAT";
            break;

        case BOX:
            d_a = "BOX";
            break;

        case ARR:
            d_a = "ARR";
            break;

        case COLZ:
            d_a = "COLZ";
            break;

        case LEGO:
            d_a = "LEGO";
            break;

        case LEGOONE:
            d_a = "LEGO1";
            break;

        case SURFONEPOL:
            d_a = "SURF1POL";
            break;

        case SURFONECYL:
            d_a = "SURF1CYL";
            break;
        }

        if(is_secondary) {
            d_a = d_a + ",same";
        }
    }

    return d_a;
}

/******************************************************************************/
/**
 * Allows to specify 2d-drawing options
 *
 * @param dropt The 2D drawing option (e.g. surface, contour, lego, ...) to be used when drawing the histogram
 */
void GHistogram2D::set2DOpt(tddropt dropt) {
    dropt_ = dropt;
}

/******************************************************************************/
/**
 * Allows to retrieve 2d-drawing options
 *
 * @return The currently configured 2D drawing option
 */
tddropt GHistogram2D::get2DOpt() const {
    return dropt_;
}

/******************************************************************************/
/**
 * Retrieve the number of bins in x-direction
 *
 * @return The number of bins in x-direction
 */
std::size_t GHistogram2D::getNBinsX() const {
    return n_bins_x_;
}

/******************************************************************************/
/**
 * Retrieve the number of bins in y-direction
 *
 * @return The number of bins in y-direction
 */
std::size_t GHistogram2D::getNBinsY() const {
    return n_bins_y_;
}

/******************************************************************************/
/**
 * Retrieve the lower boundary of the plot in x-direction
 *
 * @return The lower boundary of the plot in x-direction
 */
double GHistogram2D::getMinX() const {
    return min_x_;
}

/******************************************************************************/
/**
 * Retrieve the upper boundary of the plot in x-direction
 *
 * @return The upper boundary of the plot in x-direction
 */
double GHistogram2D::getMaxX() const {
    return max_x_;
}

/******************************************************************************/
/**
 * Retrieve the lower boundary of the plot in y-direction
 *
 * @return The lower boundary of the plot in y-direction
 */
double GHistogram2D::getMinY() const {
    return min_y_;
}

/******************************************************************************/
/**
 * Retrieve the upper boundary of the plot in y-direction
 *
 * @return The upper boundary of the plot in y-direction
 */
double GHistogram2D::getMaxY() const {
    return max_y_;
}

/******************************************************************************/
/**
 * Retrieves a unique name for this plotter
 *
 * @return A unique name identifying this plotter type
 */
std::string GHistogram2D::getPlotterName() const {
    return "GHistogram2D";
}

/******************************************************************************/
/**
 * Reports this plotter's choice as a GPlotSpec (a 2-d histogram). The columns hold
 * the raw (unbinned) x/y sample values; n_bins_x / n_bins_y carry the bin counts.
 *
 * @return A GPlotSpec describing this GHistogram2D
 */
GPlotSpec GHistogram2D::plotSpec() const {
    GPlotSpec spec = GBasePlotter::plotSpec();
    spec.kind = plotKind::hist_2d;
    spec.role = defaultRole(spec.kind);
    spec.columns = {"x", "y"};
    spec.n_bins_x = n_bins_x_;
    spec.n_bins_y = n_bins_y_;
    // Fixed per-axis ranges are signalled by min != max (min == max means auto-range).
    if(min_x_ != max_x_) {
        spec.range_x = std::make_tuple(min_x_, max_x_);
    }
    if(min_y_ != max_y_) {
        spec.range_y = std::make_tuple(min_y_, max_y_);
    }
    return spec;
}

/******************************************************************************/
/**
 * Returns the name of this class
 *
 * @return The name of this class as a string
 */
std::string GHistogram2D::name_() const {
    return std::string("GHistogram2D");
}

/******************************************************************************/
/**
 * Searches for compliance with expectations with respect to another object
 * of the same type
 *
 * @param cp A constant reference to another object, passed as a GBasePlotter reference
 * @param e The expectation (equality / inequality) the comparison should fulfil
 * @param limit The acceptable tolerance for floating point comparisons (unused here)
 */
void GHistogram2D::compare_(
    const GBasePlotter &cp,
    const expectation &e,
    [[maybe_unused]] const double & limit
) const {
    // Check that we are dealing with a GBasePlotter reference independent of this object and convert the pointer
    const auto *p_load = g_convert_and_compare(cp, this);

    GToken token("GHistogram2D", e);

    // Compare our parent data ...
    compare_base_t<GDataCollector2T<double, double>>(*this, *p_load, token);

    // ... and then the local data, derived from the single localMembers() declaration
    g_compare_members(localMembers_(*this), localMembers_(*p_load), token);

    // React on deviations from the expectation
    token.evaluate();
}

/******************************************************************************/
/**
 * Creates a deep clone of this object
 *
 * @return A deep copy of this object, returned as a GBasePlotter pointer
 */
GBasePlotter *GHistogram2D::clone_() const {
    return new GHistogram2D(*this);
}

/******************************************************************************/
/**
 * Loads the data of another object
 *
 * @param cp A constant pointer to another object (as a GBasePlotter) whose data is loaded into this one
 */
void GHistogram2D::load_(const GBasePlotter *cp) {
    // Check that we are dealing with a GHistogram2D reference independent of this object and convert the pointer
    const auto *p_load = g_convert_and_compare(cp, this);

    // Load our parent class'es data ...
    GDataCollector2T<double, double>::load_(cp);

    // ... and then our local data, derived from the single localMembers() declaration
    g_load_members(localMembers_(*this), localMembers_(*p_load));
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * The standard constructor. Some member variables may be initialized in the
 * class body.
 *
 * @param f_d The function description (a ROOT-style formula string) to be plotted
 * @param x_extremes A tuple holding the lower (get<0>) and upper (get<1>) boundary of the x-axis range
 */
GFunctionPlotter1D::GFunctionPlotter1D(
    const std::string &f_d,
    const std::tuple<double, double> &x_extremes
)
  : function_description_(f_d)
  , x_extremes_(x_extremes) { /* nothing */
}

/******************************************************************************/
/**
 * Allows to set the number of sampling points of the function on the x-axis
 *
 * @param n_samples_x The number of sampling points of the function on the x-axis
 */
void GFunctionPlotter1D::setNSamplesX(std::size_t n_samples_x) {
    n_samples_x_ = n_samples_x;
}

/******************************************************************************/
/**
 * Retrieves a unique name for this plotter
 *
 * @return A unique name identifying this plotter type
 */
std::string GFunctionPlotter1D::getPlotterName() const {
    return "GFunctionPlotter1D";
}

/******************************************************************************/
/**
 * Reports this plotter's choice as a GPlotSpec (a sampled 1-d function plot). A
 * function plotter carries no sampled data columns.
 *
 * @return A GPlotSpec describing this GFunctionPlotter1D
 */
GPlotSpec GFunctionPlotter1D::plotSpec() const {
    GPlotSpec spec = GBasePlotter::plotSpec();
    spec.kind = plotKind::function_1d;
    spec.role = defaultRole(spec.kind);
    return spec;
}

/******************************************************************************/
/**
 * Returns the name of this class
 *
 * @return The name of this class as a string
 */
std::string GFunctionPlotter1D::name_() const {
    return std::string("GFunctionPlotter1D");
}

/******************************************************************************/
/**
 * Searches for compliance with expectations with respect to another object
 * of the same type
 *
 * @param cp A constant reference to another object, passed as a GBasePlotter reference
 * @param e The expectation (equality / inequality) the comparison should fulfil
 * @param limit The acceptable tolerance for floating point comparisons (unused here)
 */
void GFunctionPlotter1D::compare_(
    const GBasePlotter &cp,
    const expectation &e,
    [[maybe_unused]] const double & limit
) const {
    // Check that we are dealing with a GFunctionPlotter1D reference independent of this object and convert the pointer
    const auto *p_load = g_convert_and_compare(cp, this);

    GToken token("GFunctionPlotter1D", e);

    // Compare our parent data ...
    compare_base_t<GBasePlotter>(*this, *p_load, token);

    // ... and then the local data, derived from the single localMembers() declaration
    g_compare_members(localMembers_(*this), localMembers_(*p_load), token);

    // React on deviations from the expectation
    token.evaluate();
}

/******************************************************************************/
/**
 * Retrieve specific header settings for this plot
 *
 * @param is_secondary Whether this plotter is a secondary plotter (true) or a primary one (false)
 * @param p_id The id of the parent plotter, used to build a unique function name for secondary plotters
 * @param own_id This plotter's own emit index, threaded in by the caller (replaces the former mutated id_)
 * @param indent The indentation string prepended to every emitted line
 * @return The code to be added to the plot header for this function
 */
std::string GFunctionPlotter1D::headerData_(
    bool is_secondary,
    std::size_t p_id,
    std::size_t own_id,
    const std::string &indent
) const {
    // Check the extreme values for consistency
    if(std::get<0>(x_extremes_) >= std::get<1>(x_extremes_)) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GFunctionPlotter1D::headerData_(): Error!" << '\n'
            << "lower boundary >= upper boundary: " << std::get<0>(x_extremes_) << " / "
            << std::get<1>(x_extremes_) << '\n'
        );
    }

    EmitStream result; // NOLINT(cppcoreguidelines-init-variables)

    std::string comment; // NOLINT(cppcoreguidelines-init-variables)
    if(!ds_marker_.empty()) {
        comment = "// " + rootEscape(ds_marker_);
    }

    std::string function_name = "func1D" + suffix(is_secondary, p_id, own_id);
    result << indent << "TF1 *" << function_name << " = new TF1(\"" << function_name << "\", \""
           << rootEscape(function_description_) << "\"," << std::get<0>(x_extremes_) << ", "
           << std::get<1>(x_extremes_) << ");" << (!comment.empty() ? comment : "") << '\n';

    return result.str();
}

/******************************************************************************/
/**
 * Retrieves the actual data sets. The three positional parameters (is_secondary flag,
 * parent id and indentation string) are unused, as a function plotter contributes no data points.
 *
 * @return The code to be added to the plot's data section for this function (always empty)
 */
std::string GFunctionPlotter1D::bodyData_([[maybe_unused]] bool is_secondary, [[maybe_unused]] std::size_t parent_id, [[maybe_unused]] std::size_t own_id, [[maybe_unused]] std::string const &indent) const {
    // No data needs to be added for a function plotter
    return {};
}

/******************************************************************************/
/**
 * Retrieves specific draw commands for this plot
 *
 * @param is_secondary Whether this plotter is a secondary plotter (true) or a primary one (false)
 * @param p_id The id of the parent plotter, used to build a unique function name for secondary plotters
 * @param own_id This plotter's own emit index, threaded in by the caller (replaces the former mutated id_)
 * @param indent The indentation string prepended to every emitted line
 * @return The draw command to be added to the plot's data for this function
 */
std::string GFunctionPlotter1D::footerData_(
    bool is_secondary,
    std::size_t p_id,
    std::size_t own_id,
    const std::string &indent
) const {
    EmitStream footer_data; // NOLINT(cppcoreguidelines-init-variables)

    std::string comment; // NOLINT(cppcoreguidelines-init-variables)
    if(!ds_marker_.empty()) {
        comment = "// " + rootEscape(ds_marker_);
    }

    std::string function_name = "func1D" + suffix(is_secondary, p_id, own_id);
    footer_data << indent << function_name << "->GetXaxis()->SetTitle(\"" << rootEscape(xAxisLabel()) << "\");"
                << '\n'
                << indent << function_name << "->GetYaxis()->SetTitle(\"" << rootEscape(yAxisLabel()) << "\");"
                << '\n'
                << indent << function_name << "->SetNpx(" << n_samples_x_ << ");" << '\n';

    if(!plot_label_.empty()) {
        footer_data << indent << function_name << "->SetTitle(\"" << rootEscape(plot_label_) << "\");" << '\n';
    }
    else {
        footer_data << indent << function_name << "->SetTitle(\" \");" << '\n';
    }

    std::string d_a = this->drawingArguments(is_secondary);

    footer_data << indent << function_name << "->Draw(\"" << d_a << "\");"
                << (!comment.empty() ? comment : "") << '\n'
                << '\n';

    return footer_data.str();
}

/******************************************************************************/
/**
 * Retrieve the current drawing arguments
 *
 * @param is_secondary Whether this plotter is a secondary plotter (true) or a primary one (false)
 * @return The ROOT draw-option string; secondary plotters get the "same" option appended
 */
std::string GFunctionPlotter1D::drawingArguments(bool is_secondary) const {
    std::string d_a;

    if(!this->drawing_arguments_.empty()) {
        d_a = this->drawing_arguments_;
    }

    if(is_secondary) {
        if(d_a.empty()) {
            d_a = "same";
        }
        else {
            d_a = d_a + ",same";
        }
    }

    return d_a;
}

/******************************************************************************/
/**
 * Creates a deep clone of this object
 *
 * @return A deep copy of this object, returned as a GBasePlotter pointer
 */
GBasePlotter *GFunctionPlotter1D::clone_() const {
    return new GFunctionPlotter1D(*this);
}

/******************************************************************************/
/**
 * Loads the data of another object
 *
 * @param cp A constant pointer to another object (as a GBasePlotter) whose data is loaded into this one
 */
void GFunctionPlotter1D::load_(const GBasePlotter *cp) {
    // Check that we are dealing with a GFunctionPlotter1D reference independent of this object and convert the pointer
    const auto *p_load = g_convert_and_compare(cp, this);

    // Load our parent class'es data ...
    GBasePlotter::load_(cp);

    // ... and then our local data, derived from the single localMembers() declaration
    g_load_members(localMembers_(*this), localMembers_(*p_load));
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * The standard constructor
 *
 * @param f_d The function description (a ROOT-style formula string) to be plotted
 * @param x_extremes A tuple holding the lower (get<0>) and upper (get<1>) boundary of the x-axis range
 * @param y_extremes A tuple holding the lower (get<0>) and upper (get<1>) boundary of the y-axis range
 */
GFunctionPlotter2D::GFunctionPlotter2D(
    const std::string &f_d,
    const std::tuple<double, double> &x_extremes,
    const std::tuple<double, double> &y_extremes
)
  : function_description_(f_d)
  , x_extremes_(x_extremes)
  , y_extremes_(y_extremes) { /* nothing */
}

/******************************************************************************/
/**
 * Allows to set the number of sampling points of the function on the x-axis
 *
 * @param n_samples_x The number of sampling points of the function on the x-axis
 */
void GFunctionPlotter2D::setNSamplesX(std::size_t n_samples_x) {
    n_samples_x_ = n_samples_x;
}

/******************************************************************************/
/**
 * Allows to set the number of sampling points of the function on the y-axis
 *
 * @param n_samples_y The number of sampling points of the function on the y-axis
 */
void GFunctionPlotter2D::setNSamplesY(std::size_t n_samples_y) {
    n_samples_y_ = n_samples_y;
}

/******************************************************************************/
/**
 * Retrieves a unique name for this plotter
 *
 * @return A unique name identifying this plotter type
 */
std::string GFunctionPlotter2D::getPlotterName() const {
    return "GFunctionPlotter2D";
}

/******************************************************************************/
/**
 * Reports this plotter's choice as a GPlotSpec (a sampled 2-d function plot). A
 * function plotter carries no sampled data columns.
 *
 * @return A GPlotSpec describing this GFunctionPlotter2D
 */
GPlotSpec GFunctionPlotter2D::plotSpec() const {
    GPlotSpec spec = GBasePlotter::plotSpec();
    spec.kind = plotKind::function_2d;
    spec.role = defaultRole(spec.kind);
    return spec;
}

/******************************************************************************/
/**
 * Returns the name of this class
 *
 * @return The name of this class as a string
 */
std::string GFunctionPlotter2D::name_() const {
    return std::string("GFunctionPlotter2D");
}

/******************************************************************************/
/**
 * Searches for compliance with expectations with respect to another object
 * of the same type
 *
 * @param cp A constant reference to another object, passed as a GBasePlotter reference
 * @param e The expectation (equality / inequality) the comparison should fulfil
 * @param limit The acceptable tolerance for floating point comparisons (unused here)
 */
void GFunctionPlotter2D::compare_(
    const GBasePlotter &cp,
    const expectation &e,
    [[maybe_unused]] const double & limit
) const {
    // Check that we are dealing with a GFunctionPlotter2D reference independent of this object and convert the pointer
    const auto *p_load = g_convert_and_compare(cp, this);

    GToken token("GFunctionPlotter2D", e);

    // Compare our parent data ...
    compare_base_t<GBasePlotter>(*this, *p_load, token);

    // ... and then the local data, derived from the single localMembers() declaration
    g_compare_members(localMembers_(*this), localMembers_(*p_load), token);

    // React on deviations from the expectation
    token.evaluate();
}

/******************************************************************************/
/**
 * Retrieve specific header settings for this plot
 *
 * @param is_secondary Whether this plotter is a secondary plotter (true) or a primary one (false)
 * @param p_id The id of the parent plotter, used to build a unique function name for secondary plotters
 * @param own_id This plotter's own emit index, threaded in by the caller (replaces the former mutated id_)
 * @param indent The indentation string prepended to every emitted line
 * @return The code to be added to the plot header for this function
 */
std::string GFunctionPlotter2D::headerData_(
    bool is_secondary,
    std::size_t p_id,
    std::size_t own_id,
    const std::string &indent
) const {
    // Check the extreme values for consistency
    if(std::get<0>(x_extremes_) >= std::get<1>(x_extremes_)) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GFunctionPlotter2D::headerData_(): Error!" << '\n'
            << "lower boundary(x) >= upper boundary(x): " << std::get<0>(x_extremes_) << " / "
            << std::get<1>(x_extremes_) << '\n'
        );
    }

    if(std::get<0>(y_extremes_) >= std::get<1>(y_extremes_)) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GFunctionPlotter2D::headerData_(): Error!" << '\n'
            << "lower boundary(y) >= upper boundary(y): " << std::get<0>(y_extremes_) << " / "
            << std::get<1>(y_extremes_) << '\n'
        );
    }

    EmitStream result; // NOLINT(cppcoreguidelines-init-variables)

    std::string comment; // NOLINT(cppcoreguidelines-init-variables)
    if(!ds_marker_.empty()) {
        comment = "// " + rootEscape(ds_marker_);
    }

    std::string function_name = "func2D" + suffix(is_secondary, p_id, own_id);
    result << indent << "TF2 *" << function_name << " = new TF2(\"" << function_name << "\", \""
           << rootEscape(function_description_) << "\"," << std::get<0>(x_extremes_) << ", "
           << std::get<1>(x_extremes_) << ", " << std::get<0>(y_extremes_) << ", "
           << std::get<1>(y_extremes_) << ");" << (!comment.empty() ? comment : "") << '\n';

    return result.str();
}

/******************************************************************************/
/**
 * Retrieves the actual data sets. The three positional parameters (is_secondary flag,
 * parent id and indentation string) are unused, as a function plotter contributes no data points.
 *
 * @return The code to be added to the plot's data section for this function (always empty)
 */
std::string GFunctionPlotter2D::bodyData_([[maybe_unused]] bool is_secondary, [[maybe_unused]] std::size_t parent_id, [[maybe_unused]] std::size_t own_id, [[maybe_unused]] std::string const &indent) const {
    // No data needs to be added for a function plotter
    return {};
}

/******************************************************************************/
/**
 * Retrieves specific draw commands for this plot
 *
 * @param is_secondary Whether this plotter is a secondary plotter (true) or a primary one (false)
 * @param p_id The id of the parent plotter, used to build a unique function name for secondary plotters
 * @param own_id This plotter's own emit index, threaded in by the caller (replaces the former mutated id_)
 * @param indent The indentation string prepended to every emitted line
 * @return The draw command to be added to the plot's data for this function
 */
std::string GFunctionPlotter2D::footerData_(
    bool is_secondary,
    std::size_t p_id,
    std::size_t own_id,
    std::string const &indent
) const {
    EmitStream footer_data; // NOLINT(cppcoreguidelines-init-variables)

    std::string comment; // NOLINT(cppcoreguidelines-init-variables)
    if(!ds_marker_.empty()) {
        comment = "// " + rootEscape(ds_marker_);
    }

    std::string function_name = "func2D" + suffix(is_secondary, p_id, own_id);
    footer_data << indent << function_name << "->GetXaxis()->SetTitle(\"" << rootEscape(xAxisLabel()) << "\");"
                << '\n'
                << indent << function_name << "->GetYaxis()->SetTitle(\"" << rootEscape(yAxisLabel()) << "\");"
                << '\n'
                << indent << function_name << "->GetZaxis()->SetTitle(\"" << rootEscape(zAxisLabel()) << "\");"
                << '\n'
                << indent << function_name << "->SetNpx(" << n_samples_x_ << ");" << '\n'
                << indent << function_name << "->SetNpy(" << n_samples_y_ << ");" << '\n';

    if(!plot_label_.empty()) {
        footer_data << indent << function_name << "->SetTitle(\"" << rootEscape(plot_label_) << "\");" << '\n';
    }
    else {
        footer_data << indent << function_name << "->SetTitle(\" \");" << '\n';
    }

    std::string d_a = this->drawingArguments(is_secondary);

    footer_data << indent << function_name << "->Draw(\"" << d_a << "\");"
                << (!comment.empty() ? comment : "") << '\n'
                << '\n';

    return footer_data.str();
}

/******************************************************************************/
/**
 * Retrieve the current drawing arguments
 *
 * @param is_secondary Whether this plotter is a secondary plotter (true) or a primary one (false)
 * @return The ROOT draw-option string; secondary plotters get the "same" option appended
 */
std::string GFunctionPlotter2D::drawingArguments(bool is_secondary) const {
    std::string d_a;

    if(!this->drawing_arguments_.empty()) {
        d_a = this->drawing_arguments_;
    }

    if(is_secondary) {
        if(d_a.empty()) {
            d_a = "same";
        }
        else {
            d_a = d_a + ",same";
        }
    }

    return d_a;
}

/******************************************************************************/
/**
 * Creates a deep clone of this object
 *
 * @return A deep copy of this object, returned as a GBasePlotter pointer
 */
GBasePlotter *GFunctionPlotter2D::clone_() const {
    return new GFunctionPlotter2D(*this);
}

/******************************************************************************/
/**
 * Loads the data of another object
 *
 * @param cp A constant pointer to another object (as a GBasePlotter) whose data is loaded into this one
 */
void GFunctionPlotter2D::load_(const GBasePlotter *cp) {
    // Check that we are dealing with a GFunctionPlotter2D reference independent of this object and convert the pointer
    const auto *p_load = g_convert_and_compare(cp, this);

    // Load our parent class'es data ...
    GBasePlotter::load_(cp);

    // ... and then our local data, derived from the single localMembers() declaration
    g_load_members(localMembers_(*this), localMembers_(*p_load));
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * The standard constructor. Note that some variables are initialized in the
 * class body.
 *
 * @param canvas_label The label of the canvas
 * @param c_x_div The number of plots in x-direction
 * @param c_y_div The number of plots in y-direction
 */
GPlotDesigner::GPlotDesigner(
    const std::string &canvas_label,
    const std::size_t &c_x_div,
    const std::size_t &c_y_div
)
  : c_x_div_(c_x_div)
  , c_y_div_(c_y_div)
  , canvas_label_(canvas_label) { /* nothing */
}

/******************************************************************************/
/**
 * The copy constructor
 *
 * @param cp A constant reference to another GPlotDesigner object to copy from
 */
GPlotDesigner::GPlotDesigner(const GPlotDesigner &cp)
  : c_x_div_(cp.c_x_div_)
  , c_y_div_(cp.c_y_div_)
  , c_x_dim_(cp.c_x_dim_)
  , c_y_dim_(cp.c_y_dim_)
  , canvas_label_(cp.canvas_label_)
  , add_print_command_(cp.add_print_command_)
  , n_indention_spaces_(cp.n_indention_spaces_) {
    // Copy any secondary plotters over
    copyCloneableSmartPointerContainer<GBasePlotter>(cp.plotters_cnt_, plotters_cnt_);
}

/******************************************************************************/
/**
 * The assignment operator
 *
 * @param cp A constant reference to another GPlotDesigner object to copy from
 * @return A reference to this object, to allow chaining
 */
GPlotDesigner &GPlotDesigner::operator=(GPlotDesigner const &cp) {
    if(this == &cp) {
        return *this;
    }
    c_x_div_ = cp.c_x_div_;
    c_y_div_ = cp.c_y_div_;
    c_x_dim_ = cp.c_x_dim_;
    c_y_dim_ = cp.c_y_dim_;
    canvas_label_ = cp.canvas_label_;
    add_print_command_ = cp.add_print_command_;
    n_indention_spaces_ = cp.n_indention_spaces_;

    // Copy any secondary plotters over
    copyCloneableSmartPointerContainer<GBasePlotter>(cp.plotters_cnt_, plotters_cnt_);

    return *this;
}

/******************************************************************************/
/**
 * Writes the plot to a file
 *
 * @param file_name The name of the file to which the data can be written
 */
void GPlotDesigner::writeToFile(const std::filesystem::path &file_name) {
    // Some backends (the DATA backend's NPZ mode) emit a BINARY document that may contain
    // embedded NUL bytes; a text-mode `<<` would truncate / mangle it. Render once, then
    // write the full byte length binary-safe so every backend round-trips intact (the text
    // backends are unaffected -- their bytes are the same in either mode).
    const std::string document = plot(file_name);
    std::ofstream result(file_name, std::ios::binary);
    result.write(document.data(), static_cast<std::streamsize>(document.size()));
    result.close();
}

/******************************************************************************/
/**
 * Selects the backend the designer emits through, installing the standard emitter
 * for the requested backend. plot() then delegates to it.
 *
 * @param backend The plotting backend to use (ROOT, the default, or GNUPLOT)
 */
void GPlotDesigner::setPlotBackend(plotBackend backend) {
    switch(backend) {
        case plotBackend::ROOT:       emitter_ = std::make_shared<GRootEmitter>(); break;
        case plotBackend::GNUPLOT:    emitter_ = std::make_shared<GnuplotEmitter>(); break;
        case plotBackend::MATPLOTLIB: emitter_ = std::make_shared<MatplotlibEmitter>(); break;
        case plotBackend::DATA:       emitter_ = std::make_shared<GDataEmitter>(); break;
    }
}

/******************************************************************************/
/**
 * Selects the DATA backend in the requested export format, installing a GDataEmitter.
 * A convenience for setEmitter(std::make_shared<GDataEmitter>(format)); plot() then
 * delegates to it, exporting the raw series data rather than a rendered plot.
 *
 * @param format The on-disk format to export (CSV, the default, or NPZ)
 */
void GPlotDesigner::setDataFormat(dataFormat format) {
    emitter_ = std::make_shared<GDataEmitter>(format);
}

/******************************************************************************/
/**
 * Installs a custom plot emitter, overriding the backend selected via
 * setPlotBackend(). plot() delegates to it.
 *
 * @param emitter The emitter to install (must not be empty)
 */
void GPlotDesigner::setEmitter(std::shared_ptr<IPlotEmitter> emitter) {
    if(not emitter) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GPlotDesigner::setEmitter(): Error!" << '\n'
            << "Got empty emitter" << '\n'
        );
    }
    emitter_ = std::move(emitter);
}

/******************************************************************************/
/**
 * Emits the overall plot by delegating to the selected backend emitter. The ROOT
 * backend is the default; its output is byte-identical to the historical generator.
 *
 * @param plot_name The output file name; used to derive the png file name in the emitted print command
 * (the literal string "empty" or an empty path suppresses the print command)
 * @return The complete backend document for the canvas and all registered plotters
 */
std::string GPlotDesigner::plot(const std::filesystem::path &plot_name) const {
    // Thread the plot name to the emitter (used by the ROOT print command).
    pending_plot_name_ = plot_name;

    // Lazily default to the ROOT backend so the historical behaviour is unchanged.
    if(not emitter_) {
        const_cast<GPlotDesigner *>(this)->emitter_ = std::make_shared<GRootEmitter>();
    }

    return emitter_->emitDocument(*this);
}

/******************************************************************************/
/**
 * The ROOT-macro file extension.
 *
 * @return The string ".C"
 */
std::string GRootEmitter::fileExtension() const {
    return std::string(".C");
}

/******************************************************************************/
/**
 * Emits the overall plot as a ROOT macro. This reproduces the historical
 * GPlotDesigner::plot() body verbatim, so the emitted text is unchanged.
 *
 * @param gpd The designer holding the plotters and canvas configuration
 * @return The complete ROOT macro source code for the canvas and all registered plotters
 */
std::string GRootEmitter::emitDocument(const GPlotDesigner &gpd) const {
    const std::filesystem::path &plot_name = gpd.pending_plot_name_;

    EmitStream result; // NOLINT(cppcoreguidelines-init-variables)
    std::size_t max_plots = gpd.c_x_div_ * gpd.c_y_div_;

    if(gpd.plotters_cnt_.size() > max_plots) {
        glogger << "In GPlotDesigner::plot() (Canvas label = \"" << gpd.getCanvasLabel()
                << "\":" << '\n'
                << "Warning! Found more plots than pads (" << gpd.plotters_cnt_.size() << " vs. "
                << max_plots << ")" << '\n'
                << "Some of the plots will be ignored" << '\n'
                << GWARNING;
    }

    result << "{" << '\n' << gpd.staticHeader(gpd.indent()) << '\n';

    // Plot all body sections up to the maximum allowed number
    result << gpd.indent() << "//===================  Header Section ====================" << '\n'
           << '\n';

    // Plot all headers up to the maximum allowed number
    std::size_t n_plots = 0;
    std::vector<std::shared_ptr<GBasePlotter>>::const_iterator it;
    for(it = gpd.plotters_cnt_.begin(); it != gpd.plotters_cnt_.end(); ++it) {
        if(n_plots++ < max_plots) {
            result << (*it)->headerData(gpd.indent()) << '\n';
        }
    }

    // Plot all body sections up to the maximum allowed number
    result << gpd.indent() << "//===================  Data Section ======================" << '\n'
           << '\n';

    n_plots = 0;
    for(it = gpd.plotters_cnt_.begin(); it != gpd.plotters_cnt_.end(); ++it) {
        if(n_plots++ < max_plots) {
            result << (*it)->bodyData(gpd.indent()) << '\n';
        }
    }

    // Plot all footer data up to the maximum allowed number
    result << gpd.indent() << "//===================  Plot Section ======================" << '\n'
           << '\n';

    n_plots = 0;
    for(it = gpd.plotters_cnt_.begin(); it != gpd.plotters_cnt_.end(); ++it) {
        if(n_plots < max_plots) {
            result << gpd.indent() << "graphPad->cd(" << n_plots + 1 << ");"
                   << '\n' /* cd starts at 1 */
                   << (*it)->footerData(gpd.indent()) << '\n';

            n_plots++;
        }
    }

    result << gpd.indent() << "graphPad->cd();" << '\n' << gpd.indent() << "cc->cd();" << '\n';

    // Check if we are supposed to output a png file
    if(gpd.add_print_command_ && plot_name.string() != "empty" && not(plot_name.string()).empty()) {
        std::string plot_name_local = plot_name.string(); // Make sure there are no white spaces
        auto ltrim = plot_name_local.find_first_not_of(" \t\r\n");
        auto rtrim = plot_name_local.find_last_not_of(" \t\r\n");
        if(ltrim != std::string::npos) {
            plot_name_local = plot_name_local.substr(ltrim, rtrim - ltrim + 1);
        }
        else {
            plot_name_local.clear();
        }
        result << '\n'
               << gpd.indent() << "// Print out the data of this file to a png file" << '\n'
               << gpd.indent() << "cc->Print(\"" << plot_name_local << ".png\");" << '\n';
    }

    result << "}" << '\n';

    return result.str();
}

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

/** @brief The number of elements in a column (visits whichever value vector it holds). */
std::size_t columnSize(const GPlotColumn &c) {
    return std::visit([](const auto *v) { return v->size(); }, c);
}

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
            call << ax << ".hist(" << pyListCol(cols[0]) << ", bins="
                 << *spec.n_bins_x << ", label=\"" << label << "\")" << '\n';
        } break;
        case mplKind::hist2d: {
            call << "_h = " << ax << ".hist2d(" << pyListCol(cols[0]) << ", "
                 << pyListCol(cols[1]) << ", bins=[" << *spec.n_bins_x << ", " << *spec.n_bins_y
                 << "])" << '\n'
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
 * GHistogram2D). Any other plotter type (e.g. a function plotter) triggers a clear
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
    std::size_t n_plots = 0;
    for(const auto &p : gpd.plotters_cnt_) {
        if(n_plots >= max_plots) {
            break;
        }
        const std::size_t pad_idx = n_plots + 1; // matplotlib subplot indices start at 1
        ++n_plots;

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
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
// The DATA backend (GDataEmitter): exports the raw columnar series data (NOT a
// rendered plot) as either human-inspectable CSV text or a binary numpy .npz archive.

namespace {

/******************************************************************************/
/**
 * A single exportable series: one plotter's columnar data captured generically. `name`
 * is the plotter's plot label, `kind` its getPlotterName(), `column_names` the per-axis
 * labels (x, y, ...) and `columns` the per-axis value vectors (all equal length). The
 * DATA backend reads everything through the public column<I>() accessor, so the series
 * is decoupled from the concrete plotter type once captured.
 */
struct dataSeries {
    std::string name;                              ///< the plotter's plot label
    std::string kind;                              ///< the plotter's getPlotterName()
    std::vector<std::string> column_names;         ///< per-axis names (x, ex, y, ...)
    std::vector<GPlotColumn> columns;              ///< per-axis value vectors (type-tagged, parallel)
    GPlotSpec spec;                                ///< the plotter's full reported plot spec
    std::size_t pad = 0;                           ///< the canvas pad this series draws into
    bool secondary = false;                        ///< true if it overlays a primary in the same pad
};

/** @brief The canvas-level layout the data export records so an external renderer can
 *  reproduce the multi-pad figure: the canvas title and the pad grid (columns x rows). */
struct canvasInfo {
    std::string label;       ///< the canvas title
    std::size_t c_x_div = 1; ///< number of pad columns
    std::size_t c_y_div = 1; ///< number of pad rows
};

/** @brief Capture a plotter's columns as a dataSeries, or std::nullopt for a plotter that
 *  carries no exportable sampled data (the function plotters). The column data (float64
 *  or int32) and names are read generically through dataColumns() / plotSpec(), so the
 *  capture is decoupled from the concrete plotter type. */
std::optional<dataSeries> captureSeries(const GBasePlotter &p) {
    dataSeries s;
    s.name = p.plotLabel();
    s.kind = p.getPlotterName();
    s.spec = p.plotSpec();
    s.columns = p.dataColumns();

    // A plotter with no exportable columns (a function plotter) reports nothing here --
    // those are dataless and are skipped.
    if(s.columns.empty()) {
        return std::nullopt;
    }

    // The per-axis names are the spec's column labels, in the same storage order as
    // dataColumns() (e.g. {"x","ex","y","ey"} for GGraph2ED, {"value"} for GHistogram1D).
    s.column_names = s.spec.columns;
    return s;
}

/** @brief The number of data rows in a captured series (the common column length). */
std::size_t seriesRows(const dataSeries &s) {
    return s.columns.empty() ? 0 : columnSize(s.columns.front());
}

/******************************************************************************/
// CSV mode.

/** @brief Escape a label for a CSV header comment / a quoted CSV field: drop CRs/newlines
 *  (a comment line and a data row must stay single-line) and double any embedded `"`. */
std::string csvComment(const std::string &in) {
    std::string out;
    out.reserve(in.size());
    for(char c : in) {
        if(c == '\n' || c == '\r') {
            out += ' ';
        } else {
            out += c;
        }
    }
    return out;
}

/** @brief Render all captured series as the CSV document: one section per series separated
 *  by a blank line; each section a `# series ...` comment header, a column-name header row,
 *  then the data rows. Values are full-precision and locale-independent (EmitStream). */
std::string emitCsv(const std::vector<dataSeries> &series, const canvasInfo &canvas) {
    EmitStream out; // NOLINT(cppcoreguidelines-init-variables)

    // A leading canvas comment so an external reader can reproduce the pad grid.
    out << "# canvas: \"" << csvComment(canvas.label) << "\" c_x_div=" << canvas.c_x_div
        << " c_y_div=" << canvas.c_y_div << '\n';

    for(std::size_t si = 0; si < series.size(); ++si) {
        const dataSeries &s = series[si];
        if(si != 0) {
            out << '\n'; // blank line between sections
        }

        // Comma-list of column names for the header comment.
        std::string col_list;
        for(std::size_t c = 0; c < s.column_names.size(); ++c) {
            col_list += (c == 0 ? "" : ",") + s.column_names[c];
        }

        out << "# series " << si << ": \"" << csvComment(s.name) << "\" kind=" << s.kind
            << " plotkind=" << to_string(s.spec.kind) << " role=" << s.spec.role
            << " columns=" << col_list
            << " pad=" << s.pad << " secondary=" << (s.secondary ? 1 : 0) << '\n';

        // Column-name header row.
        for(std::size_t c = 0; c < s.column_names.size(); ++c) {
            out << (c == 0 ? "" : ",") << s.column_names[c];
        }
        out << '\n';

        // Data rows. Each cell streams its column's value at row r -- an int32 column
        // prints integers, a float64 column full-precision doubles.
        const std::size_t rows = seriesRows(s);
        for(std::size_t r = 0; r < rows; ++r) {
            for(std::size_t c = 0; c < s.columns.size(); ++c) {
                out << (c == 0 ? "" : ",");
                std::visit([&out, r](const auto *v) { out << (*v)[r]; }, s.columns[c]);
            }
            out << '\n';
        }
    }
    return out.str();
}

/******************************************************************************/
// NPZ mode: build a numpy .npz (an uncompressed ZIP of float64 .npy members) by hand,
// with no new C++ dependency. Helpers below assume a little-endian host (every platform
// Geneva targets) -- the float64 / uint bytes are emitted in native order.

/** @brief Append a uint16 little-endian to a byte buffer. */
void putU16(std::string &buf, std::uint16_t v) {
    buf.push_back(static_cast<char>(v & 0xFFu));
    buf.push_back(static_cast<char>((v >> 8) & 0xFFu));
}

/** @brief Append a uint32 little-endian to a byte buffer. */
void putU32(std::string &buf, std::uint32_t v) {
    buf.push_back(static_cast<char>(v & 0xFFu));
    buf.push_back(static_cast<char>((v >> 8) & 0xFFu));
    buf.push_back(static_cast<char>((v >> 16) & 0xFFu));
    buf.push_back(static_cast<char>((v >> 24) & 0xFFu));
}

/** @brief A table-based CRC-32 (the ISO-HDLC / ZIP polynomial 0xEDB88320), built once. */
const std::array<std::uint32_t, 256> &crc32Table() {
    static const std::array<std::uint32_t, 256> table = [] {
        std::array<std::uint32_t, 256> t{};
        for(std::uint32_t n = 0; n < 256; ++n) {
            std::uint32_t c = n;
            for(int k = 0; k < 8; ++k) {
                c = (c & 1u) ? (0xEDB88320u ^ (c >> 1)) : (c >> 1);
            }
            t[n] = c;
        }
        return t;
    }();
    return table;
}

/** @brief CRC-32 of a byte range (ZIP local/central-directory checksum). */
std::uint32_t crc32(const std::string &data) {
    const auto &table = crc32Table();
    std::uint32_t crc = 0xFFFFFFFFu;
    for(unsigned char byte : data) {
        crc = table[(crc ^ byte) & 0xFFu] ^ (crc >> 8);
    }
    return crc ^ 0xFFFFFFFFu;
}

/** @brief Build the bytes of a numpy `.npy` (format v1.0) for a 2-D C-order little-endian
 *  array of shape (rows, cols). The dtype is taken from the columns: a series whose
 *  columns are int32 is written as `<i4` (a real numpy int32 array), otherwise `<f8`
 *  float64. A series' columns are uniform dtype (a plotter's axes are all double or all
 *  int32). The column-major `columns` are interleaved into the row-major payload; a
 *  1-column series is still written as shape (rows, 1). */
std::string buildNpy(const std::vector<GPlotColumn> &columns, std::size_t rows) {
    const std::size_t cols = columns.size();

    const bool is_int = !columns.empty()
        && std::holds_alternative<const std::vector<std::int32_t> *>(columns.front());
    const char *descr = is_int ? "<i4" : "<f8";
    const std::size_t elem_size = is_int ? sizeof(std::int32_t) : sizeof(double);

    // The ASCII dict header describing the array.
    std::string dict = std::string("{'descr': '") + descr + "', 'fortran_order': False, 'shape': (";
    dict += std::to_string(rows);
    dict += ", ";
    dict += std::to_string(cols);
    dict += "), }";

    // The header must be padded with spaces so that magic(6)+version(2)+len(2)+header is a
    // multiple of 64, with the final header byte a newline.
    const std::size_t prefix = 6 + 2 + 2; // magic + version + uint16 length field
    std::size_t total = prefix + dict.size() + 1; // +1 for the trailing '\n'
    const std::size_t pad = (64 - (total % 64)) % 64;
    dict.append(pad, ' ');
    dict.push_back('\n');

    std::string npy;
    npy.append("\x93NUMPY", 6);    // magic
    npy.push_back('\x01');          // version major
    npy.push_back('\x00');          // version minor
    putU16(npy, static_cast<std::uint16_t>(dict.size())); // header length (LE)
    npy += dict;

    // The raw payload in C order: row-major, i.e. all columns of row 0, then row 1...
    // Each value is written in its native (little-endian) byte width matching `descr`.
    npy.reserve(npy.size() + rows * cols * elem_size);
    for(std::size_t r = 0; r < rows; ++r) {
        for(std::size_t c = 0; c < cols; ++c) {
            std::visit([&npy, r](const auto *v) {
                const auto val = (*v)[r];
                char bytes[sizeof(val)];
                std::memcpy(bytes, &val, sizeof(val)); // native (little-endian) order
                npy.append(bytes, sizeof(val));
            }, columns[c]);
        }
    }
    return npy;
}

/** @brief One member to be stored in the .npz ZIP: its archive name and raw bytes. */
struct zipMember {
    std::string name;
    std::string data;
};

/** @brief Pack the members into a single uncompressed ("store", method 0) ZIP -- which IS a
 *  .npz. Each member gets a local file header + its bytes; a central directory and an
 *  end-of-central-directory record close the archive. DOS date/time are left zero (numpy
 *  ignores them) so the bytes are deterministic. */
std::string buildZip(const std::vector<zipMember> &members) {
    std::string out;
    struct cdEntry {
        std::string name;
        std::uint32_t crc;
        std::uint32_t size;
        std::uint32_t offset;
    };
    std::vector<cdEntry> directory;

    for(const auto &m : members) {
        const std::uint32_t offset = static_cast<std::uint32_t>(out.size());
        const std::uint32_t crc = crc32(m.data);
        const std::uint32_t size = static_cast<std::uint32_t>(m.data.size());

        // Local file header.
        putU32(out, 0x04034b50u);                                  // local file header signature
        putU16(out, 20);                                            // version needed to extract (2.0)
        putU16(out, 0);                                            // general purpose bit flag
        putU16(out, 0);                                            // compression method 0 (store)
        putU16(out, 0);                                            // last mod file time
        putU16(out, 0);                                            // last mod file date
        putU32(out, crc);                                          // CRC-32
        putU32(out, size);                                         // compressed size (== uncompressed)
        putU32(out, size);                                         // uncompressed size
        putU16(out, static_cast<std::uint16_t>(m.name.size()));    // file name length
        putU16(out, 0);                                            // extra field length
        out += m.name;                                            // file name
        out += m.data;                                            // the member bytes

        directory.push_back({m.name, crc, size, offset});
    }

    // Central directory.
    const std::uint32_t cd_offset = static_cast<std::uint32_t>(out.size());
    for(const auto &e : directory) {
        putU32(out, 0x02014b50u);                                  // central file header signature
        putU16(out, 20);                                           // version made by
        putU16(out, 20);                                           // version needed to extract
        putU16(out, 0);                                            // general purpose bit flag
        putU16(out, 0);                                            // compression method 0 (store)
        putU16(out, 0);                                            // last mod file time
        putU16(out, 0);                                            // last mod file date
        putU32(out, e.crc);                                        // CRC-32
        putU32(out, e.size);                                       // compressed size
        putU32(out, e.size);                                       // uncompressed size
        putU16(out, static_cast<std::uint16_t>(e.name.size()));    // file name length
        putU16(out, 0);                                            // extra field length
        putU16(out, 0);                                            // file comment length
        putU16(out, 0);                                            // disk number start
        putU16(out, 0);                                            // internal file attributes
        putU32(out, 0);                                            // external file attributes
        putU32(out, e.offset);                                     // relative offset of local header
        out += e.name;                                            // file name
    }
    const std::uint32_t cd_size = static_cast<std::uint32_t>(out.size()) - cd_offset;

    // End of central directory record.
    putU32(out, 0x06054b50u);                                      // EOCD signature
    putU16(out, 0);                                                // number of this disk
    putU16(out, 0);                                                // disk where CD starts
    putU16(out, static_cast<std::uint16_t>(directory.size()));     // CD records on this disk
    putU16(out, static_cast<std::uint16_t>(directory.size()));     // total CD records
    putU32(out, cd_size);                                          // size of central directory
    putU32(out, cd_offset);                                        // offset of central directory
    putU16(out, 0);                                                // comment length
    return out;
}

/** @brief Minimal JSON string escaping for a manifest field (quotes, backslashes, control
 *  chars), matching GPlotSpec::toJson()'s own escaping. */
std::string jsonEscapeField(const std::string &in) {
    std::string out;
    out.reserve(in.size() + 2);
    for(char c : in) {
        switch(c) {
            case '"':  out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default:   out += c;      break;
        }
    }
    return out;
}

/** @brief Render all captured series as a numpy .npz: one float64 .npy member per series
 *  (`series_0`, `series_1`, ...) plus a `manifest.json`. The manifest is a self-describing
 *  object -- the canvas (title + pad grid) and an ordered `series` array -- so (data +
 *  manifest) fully describes the multi-pad figure for an external renderer. Each series
 *  entry is the plotter's GPlotSpec augmented with the `pad` it draws into and a
 *  `secondary` flag (true if it overlays a primary in that pad). */
std::string emitNpz(const std::vector<dataSeries> &series, const canvasInfo &canvas) {
    std::vector<zipMember> members;

    std::string manifest = "{\n";
    manifest += "  \"canvas\": {\"label\": \"" + jsonEscapeField(canvas.label)
        + "\", \"c_x_div\": " + std::to_string(canvas.c_x_div)
        + ", \"c_y_div\": " + std::to_string(canvas.c_y_div) + "},\n";
    manifest += "  \"series\": [\n";
    for(std::size_t si = 0; si < series.size(); ++si) {
        const dataSeries &s = series[si];
        const std::size_t rows = seriesRows(s);

        // The .npy member.
        zipMember member;
        member.name = "series_" + std::to_string(si) + ".npy";
        member.data = buildNpy(s.columns, rows);
        members.push_back(std::move(member));

        // The manifest entry: the plotter's GPlotSpec JSON ({...}) augmented in-place
        // with its pad / secondary placement (splice before the closing brace).
        std::string spec_json = s.spec.toJson();
        spec_json.pop_back(); // drop the trailing '}'
        spec_json += ", \"pad\": " + std::to_string(s.pad)
            + ", \"secondary\": " + (s.secondary ? "true" : "false") + "}";
        manifest += "    " + spec_json;
        manifest += (si + 1 == series.size() ? "\n" : ",\n");
    }
    manifest += "  ]\n}\n";

    members.push_back({"manifest.json", manifest});

    return buildZip(members);
}

/** @brief Collect the exportable series from an ordered list of plotters (skipping the
 *  dataless function plotters), including each plotter's secondary plotters. Shared by
 *  both the CSV and NPZ paths; the plotter list is supplied by the friend emitter member
 *  (free functions cannot reach GPlotDesigner's private plotter container). */
std::vector<dataSeries> collectSeries(
    const std::vector<std::shared_ptr<GBasePlotter>> &plotters
) {
    std::vector<dataSeries> series;
    // The pad index is the primary's registration index (matching the render emitters,
    // which place plotter i into pad i); its secondary plotters overlay the same pad. A
    // dataless plotter (function plotter) still consumes its pad index, so the exported
    // pad numbers line up with where ROOT / matplotlib would draw each plot.
    for(std::size_t pad = 0; pad < plotters.size(); ++pad) {
        const auto &p = plotters[pad];
        if(auto s = captureSeries(*p)) {
            s->pad = pad;
            s->secondary = false;
            series.push_back(std::move(*s));
        }
        // Secondary plotters are independent datasets sharing the primary's pad.
        for(const auto &sp : p->secondaryPlotters()) {
            if(auto s = captureSeries(*sp)) {
                s->pad = pad;
                s->secondary = true;
                series.push_back(std::move(*s));
            }
        }
    }
    return series;
}

} // anonymous namespace

/******************************************************************************/
/**
 * Constructs the emitter in the requested export format (CSV or NPZ).
 *
 * @param format The on-disk format to export
 */
GDataEmitter::GDataEmitter(dataFormat format) : format_(format) { /* nothing */ }

/******************************************************************************/
/**
 * The file extension for the selected format.
 *
 * @return ".csv" in CSV mode, ".npz" in NPZ mode
 */
std::string GDataEmitter::fileExtension() const {
    return format_ == dataFormat::NPZ ? std::string(".npz") : std::string(".csv");
}

/******************************************************************************/
/**
 * The export format this emitter was constructed with.
 *
 * @return The current dataFormat
 */
dataFormat GDataEmitter::getDataFormat() const {
    return format_;
}

/******************************************************************************/
/**
 * Exports each registered plotter's raw columnar series data (NOT a rendered plot). The
 * graph plotters (GGraph2D / GGraph2ED / GGraph3D / GGraph4D) and the histogram plotters
 * (GHistogram1D / GHistogram2D) export their axis columns; the function plotters
 * (GFunctionPlotter1D / GFunctionPlotter2D) carry no sampled data and are skipped. The
 * result is either a human-inspectable CSV document or the raw bytes of a numpy .npz
 * archive (which numpy.load() reads back as a dict of float64 arrays).
 *
 * @param gpd The designer holding the plotters
 * @return The CSV text or the raw .npz bytes (a std::string holds embedded NULs intact)
 */
std::string GDataEmitter::emitDocument(const GPlotDesigner &gpd) const {
    const std::vector<dataSeries> series = collectSeries(gpd.plotters_cnt_);
    const canvasInfo canvas{gpd.getCanvasLabel(), gpd.c_x_div_, gpd.c_y_div_};
    return format_ == dataFormat::NPZ ? emitNpz(series, canvas) : emitCsv(series, canvas);
}

/******************************************************************************/
/**
 * A default header for a ROOT file
 *
 * @param indent The indentation string prepended to every emitted line
 * @return The ROOT macro source code setting up the canvas, title and graph pad
 */
std::string GPlotDesigner::staticHeader(const std::string &indent) const {
    EmitStream result; // NOLINT(cppcoreguidelines-init-variables)

    result << indent << "gROOT->Reset();" << '\n'
           << indent << "gStyle->SetCanvasColor(0);" << '\n'
           << indent << "gStyle->SetStatBorderSize(1);" << '\n'
           << indent << "gStyle->SetOptStat(0);" << '\n'
           << '\n'
           << indent << R"(TCanvas *cc = new TCanvas("cc", "cc",0,0,)" << c_x_dim_ << ","
           << c_y_dim_ << ");" << '\n'
           << '\n'
           << indent << "TPaveLabel* canvasTitle = new TPaveLabel(0.2,0.95,0.8,0.99, \""
           << rootEscape(canvas_label_) << "\");" << '\n'
           << indent << "canvasTitle->Draw();" << '\n'
           << '\n'
           << indent << R"(TPad* graphPad = new TPad("Graphs", "Graphs", 0.01, 0.01, 0.99, 0.94);)"
           << '\n'
           << indent << "graphPad->Draw();" << '\n'
           << indent << "graphPad->Divide(" << c_x_div_ << "," << c_y_div_ << ");" << '\n'
           << '\n';

    return result.str();
}

/******************************************************************************/
/**
 * Allows to add a new plotter object
 *
 * @param plotter_ptr A pointer to a plotter
 */
void GPlotDesigner::registerPlotter(std::shared_ptr<GBasePlotter> plotter_ptr) {
    if(plotter_ptr) {
        plotter_ptr->setId(plotters_cnt_.size());
        plotters_cnt_.push_back(plotter_ptr);
    }
    else {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "GPlotDesigner::registerPlotter(): Error!" << '\n'
            << "Got empty plotter" << '\n'
        );
    }
}

/******************************************************************************/
/**
 * Set the dimensions of the output canvas
 *
 * @param c_x_dim The x-dimension of the output canvas
 * @param c_y_dim The y-dimension of the output canvas
 */
void GPlotDesigner::setCanvasDimensions(
    const std::uint32_t &c_x_dim,
    const std::uint32_t &c_y_dim
) {
    c_x_dim_ = c_x_dim;
    c_y_dim_ = c_y_dim;
}

/******************************************************************************/
/**
 * Set the dimensions of the output canvas
 *
 * @param c_dim A tuple holding the x-dimension (get<0>) and y-dimension (get<1>) of the output canvas
 */
void GPlotDesigner::setCanvasDimensions(const std::tuple<std::uint32_t, std::uint32_t> &c_dim) {
    this->setCanvasDimensions(std::get<0>(c_dim), std::get<1>(c_dim));
}

/******************************************************************************/
/**
 * Allows to retrieve the canvas dimensions
 *
 * @return A std::tuple holding the canvas dimensions
 */
std::tuple<std::uint32_t, std::uint32_t> GPlotDesigner::getCanvasDimensions() const {
    return std::tuple<std::uint32_t, std::uint32_t>{c_x_dim_, c_y_dim_};
}

/******************************************************************************/
/**
 * Allows to set the canvas label
 *
 * @param canvas_label The label to be assigned to the output canvas
 */
void GPlotDesigner::setCanvasLabel(const std::string &canvas_label) {
    canvas_label_ = canvas_label;
}

/******************************************************************************/
/**
 * Allows to retrieve the canvas label
 *
 * @return The label currently assigned to the output canvas
 */
std::string GPlotDesigner::getCanvasLabel() const {
    return canvas_label_;
}

/******************************************************************************/
/**
 * Allows to add a "Print" command to the end of the script so that picture files are created
 *
 * @param add_print_command If true, a print command writing a png file is appended to the emitted script
 */
void GPlotDesigner::setAddPrintCommand(bool add_print_command) {
    add_print_command_ = add_print_command;
}

/******************************************************************************/
/**
 * Allows to retrieve the current value of the add_print_command_ variable
 *
 * @return true if a print command is appended to the emitted script, false otherwise
 */
bool GPlotDesigner::getAddPrintCommand() const {
    return add_print_command_;
}

/******************************************************************************/
/**
 * Allows to set the number of spaces used for indention
 *
 * @param n_indention_spaces The number of space characters used for one level of indentation
 */
void GPlotDesigner::setNIndentionSpaces(const std::size_t &n_indention_spaces) {
    n_indention_spaces_ = n_indention_spaces;
}

/******************************************************************************/
/**
 * Allows to retrieve the number spaces used for indention
 *
 * @return The number of space characters used for one level of indentation
 */
std::size_t GPlotDesigner::getNIndentionSpaces() const {
    return n_indention_spaces_;
}

/******************************************************************************/
/**
 * Returns the current number of indention spaces as a string
 *
 * @return A string consisting of the configured number of space characters
 */
std::string GPlotDesigner::indent() const {
    return std::string(n_indention_spaces_, ' ');
}

/******************************************************************************/
/**
 * Resets the plotters
 */
void GPlotDesigner::resetPlotters() {
    plotters_cnt_.clear();
}

/******************************************************************************/
/**
 * Returns the name of this class
 *
 * @return The name of this class as a string
 */
std::string GPlotDesigner::name_() const {
    return std::string("GPlotDesigner");
}

/******************************************************************************/
/**
 * Searches for compliance with expectations with respect to another object
 * of the same type
 *
 * @param cp A constant reference to another GPlotDesigner object to compare against
 * @param e The expectation (equality / inequality) the comparison should fulfil
 * @param limit The acceptable tolerance for floating point comparisons (unused here)
 */
void GPlotDesigner::compare_(
    const GPlotDesigner &cp,
    const expectation &e,
    [[maybe_unused]] const double & limit
) const {
    // Check that we are dealing with a GPlotDesigner reference independent of this object and convert the pointer
    const auto *p_load = g_convert_and_compare(cp, this);

    GToken token("GPlotDesigner", e);

    // Compare our parent data ...
    compare_base_t<GCommonInterfaceT<GPlotDesigner>>(*this, *p_load, token);

    // ... and then the local data
    g_compare_members(localMembers_(*this), localMembers_(*p_load), token);

    // React on deviations from the expectation
    token.evaluate();
}

/******************************************************************************/
/**
 * Creates a deep clone of this object
 *
 * @return A deep copy of this object, returned as a GPlotDesigner pointer
 */
GPlotDesigner *GPlotDesigner::clone_() const {
    return new GPlotDesigner(*this);
}

/******************************************************************************/
/**
 * Loads the data of another object
 *
 * @param cp A constant pointer to another GPlotDesigner object whose data is loaded into this object
 */
void GPlotDesigner::load_(const GPlotDesigner *cp) {
    // Check that we are dealing with a GPlotDesigner reference independent of this object and convert the pointer
    const auto *p_load = g_convert_and_compare(cp, this);

    // No "loadable" parent class

    // Load local data
    g_load_members(localMembers_(*this), localMembers_(*p_load));
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
// GDataLog: the modern, plotter-object-free plot API (declare a GPlotSpec per series +
// push data; realize into a GPlotDesigner via makePlotter() + appendRow()).

/******************************************************************************/
/**
 * Constructs an empty log for the given canvas title and pad grid.
 *
 * @param canvas_label The canvas title
 * @param c_x_div The number of pad columns
 * @param c_y_div The number of pad rows
 */
GDataLog::GDataLog(std::string canvas_label, std::size_t c_x_div, std::size_t c_y_div)
  : canvas_label_(std::move(canvas_label)), c_x_div_(c_x_div), c_y_div_(c_y_div) { /* nothing */ }

/******************************************************************************/
/**
 * Declares a primary series (its own pad) from its plot spec.
 *
 * @param spec The plot specification
 * @return The id of the new series
 */
GDataLog::SeriesId GDataLog::declareSeries(GPlotSpec spec) {
    series_.push_back(Series{std::move(spec), {}, std::nullopt});
    return series_.size() - 1;
}

/******************************************************************************/
/**
 * Declares an overlay series sharing an existing primary series' pad.
 *
 * @param primary The id of the primary series whose pad this overlays
 * @param spec The plot specification for the overlay
 * @return The id of the new overlay series
 */
GDataLog::SeriesId GDataLog::overlaySeries(SeriesId primary, GPlotSpec spec) {
    if(primary >= series_.size() || series_[primary].primary.has_value()) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GDataLog::overlaySeries(): Error!" << '\n'
            << "primary id " << primary << " is not a valid primary series" << '\n'
        );
    }
    series_.push_back(Series{std::move(spec), {}, primary});
    return series_.size() - 1;
}

/******************************************************************************/
/**
 * Appends one data row (one value per column) to a series.
 *
 * @param id The series to append to
 * @param row One value per column
 */
void GDataLog::append(SeriesId id, std::span<const double> row) {
    if(id >= series_.size()) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GDataLog::append(): Error!" << '\n'
            << "series id " << id << " is out of range" << '\n'
        );
    }
    series_[id].rows.emplace_back(row.begin(), row.end());
}

/******************************************************************************/
/** @brief Convenience append for a 2-column (x, y) series */
void GDataLog::append(SeriesId id, double x, double y) {
    const double row[] = {x, y};
    this->append(id, std::span<const double>(row));
}

/******************************************************************************/
/** @brief Convenience append for a 3-column (x, y, z) series */
void GDataLog::append(SeriesId id, double x, double y, double z) {
    const double row[] = {x, y, z};
    this->append(id, std::span<const double>(row));
}

/******************************************************************************/
/** @brief Convenience append for a 4-column (x, y, z, w) series */
void GDataLog::append(SeriesId id, double x, double y, double z, double w) {
    const double row[] = {x, y, z, w};
    this->append(id, std::span<const double>(row));
}

/******************************************************************************/
/** @brief Convenience append for a 3-column series from an (x, y, z) tuple */
void GDataLog::append(SeriesId id, const std::tuple<double, double, double> &row) {
    this->append(id, std::get<0>(row), std::get<1>(row), std::get<2>(row));
}

/******************************************************************************/
/** @brief Convenience append for a 4-column series from an (x, y, z, w) tuple */
void GDataLog::append(SeriesId id, const std::tuple<double, double, double, double> &row) {
    this->append(id, std::get<0>(row), std::get<1>(row), std::get<2>(row), std::get<3>(row));
}

/******************************************************************************/
/**
 * Sets the canvas pixel dimensions, forwarded to the built GPlotDesigner.
 *
 * @param x_dim The canvas width in pixels
 * @param y_dim The canvas height in pixels
 */
void GDataLog::setCanvasDimensions(std::uint32_t x_dim, std::uint32_t y_dim) {
    dims_ = std::make_tuple(x_dim, y_dim);
}

/******************************************************************************/
/**
 * Marks a series so its rows are sorted by the first column before rendering.
 *
 * @param id The series to sort
 */
void GDataLog::sortByFirstColumn(SeriesId id) {
    if(id >= series_.size()) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GDataLog::sortByFirstColumn(): Error!" << '\n'
            << "series id " << id << " is out of range" << '\n'
        );
    }
    series_[id].sort_first_column = true;
}

/******************************************************************************/
/**
 * Forwards the "add a print command" flag to the built GPlotDesigner.
 *
 * @param add_print_command Whether the designer should add a print / save command
 */
void GDataLog::setAddPrintCommand(bool add_print_command) {
    add_print_command_ = add_print_command;
}

/******************************************************************************/
/**
 * Realizes the log into a populated GPlotDesigner: each series' plotter is built from
 * its spec (makePlotter) and filled (appendRow); an overlay is attached as a secondary
 * plotter of its primary. Primaries are registered in declaration order, so the emitted
 * output matches an equivalent hand-built designer.
 *
 * @return The populated GPlotDesigner
 */
GPlotDesigner GDataLog::toDesigner() const {
    GPlotDesigner gpd(canvas_label_, c_x_div_, c_y_div_);
    if(dims_.has_value()) {
        gpd.setCanvasDimensions(*dims_);
    }
    if(add_print_command_.has_value()) {
        gpd.setAddPrintCommand(*add_print_command_);
    }

    // Build every series' plotter (primary or overlay) from its spec + rows, sorting
    // by the first column where requested.
    std::vector<std::shared_ptr<GBasePlotter>> built(series_.size());
    for(std::size_t i = 0; i < series_.size(); ++i) {
        std::shared_ptr<GBasePlotter> p = makePlotter(series_[i].spec);
        for(const auto &row : series_[i].rows) {
            p->appendRow(row);
        }
        if(series_[i].sort_first_column) {
            p->sortByFirstColumn();
        }
        built[i] = std::move(p);
    }

    // Attach overlays to their primary.
    for(std::size_t i = 0; i < series_.size(); ++i) {
        if(series_[i].primary.has_value()) {
            built[*series_[i].primary]->registerSecondaryPlotter(built[i]);
        }
    }

    // Register the primaries in declaration order.
    for(std::size_t i = 0; i < series_.size(); ++i) {
        if(not series_[i].primary.has_value()) {
            gpd.registerPlotter(built[i]);
        }
    }
    return gpd;
}

/******************************************************************************/
/**
 * Realizes the log and writes it to a file through the given backend.
 *
 * @param path The output file path
 * @param backend The rendering backend
 */
void GDataLog::writeToFile(const std::filesystem::path &path, plotBackend backend) const {
    GPlotDesigner gpd = this->toDesigner();
    gpd.setPlotBackend(backend);
    gpd.writeToFile(path);
}

/******************************************************************************/
/**
 * The number of declared series (primaries + overlays).
 *
 * @return The series count
 */
std::size_t GDataLog::nSeries() const {
    return series_.size();
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Gem::Common */
