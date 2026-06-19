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

#include "common/GPlotDesigner.hpp"
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
#include <cmath>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <ios>
#include <istream>
#include <memory>
#include <ostream>
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
 * @return A suffix string built from the parent id (if secondary) and this object's own id
 */
std::string GBasePlotter::suffix(bool is_secondary, std::size_t p_id) const {
    std::string result; // NOLINT(cppcoreguidelines-init-variables)

    if(not is_secondary) {
        result = std::string("_") + to_string(this->id());
    }
    else {
        result = std::string("_") + to_string(p_id) + std::string("_") + to_string(this->id());
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
    std::ostringstream header_data; // NOLINT(cppcoreguidelines-init-variables)

    // Add this plot's data
    header_data << indent << "// Header data for primary plotter" << '\n'
                << this->headerData_(false, 0, indent);

    // Extract data from the secondary plotters, if any
    std::size_t pos = 0;
    std::vector<std::shared_ptr<GBasePlotter>>::const_iterator cit;
    for(cit = secondary_plotter_.begin(); cit != secondary_plotter_.end(); ++cit) {
        // Give the plotters their own id which will act as a child id in this case
        (*cit)->setId(pos);

        // We parent id 0 is reserved for primary plotters
        header_data << indent << "// Header data for secondary plotter " << pos << " of "
                    << this->getPlotterName() << '\n'
                    << (*cit)->headerData_(true, this->id(), indent) << '\n';

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
    std::ostringstream body_data; // NOLINT(cppcoreguidelines-init-variables)

    // Add this plot's data
    body_data << indent << "// Body data for primary plotter" << '\n'
              << this->bodyData_(false, 0, indent);

    // Extract data from the secondary plotters, if any
    std::size_t pos = 0;
    for(auto const &plotter_ptr : secondary_plotter_) {
        body_data << indent << "// Body data for secondary plotter " << pos << " of "
                  << this->getPlotterName() << '\n'
                  << plotter_ptr->bodyData_(true, this->id(), indent) << '\n';

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
    std::ostringstream footer_data; // NOLINT(cppcoreguidelines-init-variables)

    // Add this plot's data
    footer_data << indent << "// Footer data for primary plotter" << '\n'
                << this->footerData_(false, 0, indent);

    // Extract data from the secondary plotters, if any
    std::size_t pos = 0;
    std::vector<std::shared_ptr<GBasePlotter>>::const_iterator cit;
    for(cit = secondary_plotter_.begin(); cit != secondary_plotter_.end(); ++cit) {
        footer_data << indent << "// Footer data for secondary plotter " << pos << " of "
                    << this->getPlotterName() << '\n'
                    << (*cit)->footerData_(true, this->id(), indent) << '\n';

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
 * @param indent The indentation string prepended to every emitted line
 * @return The header code declaring the x/y data arrays for this graph
 */
std::string
GGraph2D::headerData_(bool is_secondary, std::size_t p_id, const std::string &indent) const {
    std::ostringstream header_data; // NOLINT(cppcoreguidelines-init-variables)

    // Set up suitable arrays for the header
    std::string base_name = suffix(is_secondary, p_id);
    std::string array_base_name = "array_" + base_name;

    std::string x_array_name = "x_" + array_base_name;
    std::string y_array_name = "y_" + array_base_name;

    std::string comment; // NOLINT(cppcoreguidelines-init-variables)
    if(!ds_marker_.empty()) {
        comment = "// " + ds_marker_;
    }

    header_data << indent << "double " << x_array_name << "[" << to_string(data_.size()) << "];"
                << (!comment.empty() ? comment : "") << '\n'
                << indent << "double " << y_array_name << "[" << to_string(data_.size()) << "];"
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
 * @param indent The indentation string prepended to every emitted line
 * @return The body code filling the x/y data arrays with this graph's tuple values
 */
std::string
GGraph2D::bodyData_(bool is_secondary, std::size_t p_id, const std::string &indent) const {
    std::ostringstream body_data; // NOLINT(cppcoreguidelines-init-variables)

    // Set up suitable arrays for the header
    std::string base_name = suffix(is_secondary, p_id);
    std::string array_base_name = "array_" + base_name;

    std::string x_array_name = "x_" + array_base_name;
    std::string y_array_name = "y_" + array_base_name;

    std::string comment; // NOLINT(cppcoreguidelines-init-variables)
    if(!ds_marker_.empty()) {
        body_data << "// " + ds_marker_ << '\n';
    }

    // Fill data from the tuples into the arrays
    std::vector<std::tuple<double, double>>::const_iterator it;
    std::size_t pos_counter = 0;

    for(it = data_.begin(); it != data_.end(); ++it) {
        body_data << indent << x_array_name << "[" << pos_counter << "] = " << std::get<0>(*it)
                  << ";"
                  << "\t" << y_array_name << "[" << pos_counter << "] = " << std::get<1>(*it) << ";"
                  << '\n';

        pos_counter++;
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
 * @param indent The indentation string prepended to every emitted line
 * @return The footer code creating and drawing the ROOT TGraph (and optional arrows) for this graph
 */
std::string
GGraph2D::footerData_(bool is_secondary, std::size_t p_id, const std::string &indent) const {
    std::ostringstream footer_data; // NOLINT(cppcoreguidelines-init-variables)

    // Set up suitable arrays for the header
    std::string base_name = suffix(is_secondary, p_id);
    std::string array_base_name = "array_" + base_name;

    std::string x_array_name = "x_" + array_base_name;
    std::string y_array_name = "y_" + array_base_name;

    std::string graph_name = std::string("graph") + base_name;

    std::string comment; // NOLINT(cppcoreguidelines-init-variables)
    if(!ds_marker_.empty()) {
        footer_data << "// " + ds_marker_ << '\n';
    }

    // Retrieve the current drawing arguments
    std::string d_a = this->drawingArguments(is_secondary);

    // Fill the data in our tuple-vector into a ROOT TGraph object
    footer_data << indent << "TGraph *" << graph_name << " = new TGraph(" << data_.size() << ", "
                << x_array_name << ", " << y_array_name << ");" << '\n'
                << indent << graph_name << "->GetXaxis()->SetTitle(\"" << xAxisLabel() << "\");"
                << '\n'
                << indent << graph_name << "->GetYaxis()->SetTitle(\"" << yAxisLabel() << "\");"
                << '\n';

    if(!plot_label_.empty()) {
        footer_data << indent << graph_name << "->SetTitle(\"" << plot_label_ << "\");" << '\n';
    }
    else {
        footer_data << indent << graph_name << "->SetTitle(\" \");" << '\n';
    }

    footer_data << indent << graph_name << "->Draw(\"" << d_a << "\");" << '\n' << '\n';

    if(draw_arrows_ && data_.size() >= 2) {
        std::vector<std::tuple<double, double>>::const_iterator it;
        std::size_t pos_counter = 0;

        double x1 = std::get<0>(*data_.begin());
        double y1 = std::get<1>(*data_.begin());
        double x2 = 0.;
        double y2 = 0.;

        for(it = data_.begin() + 1; it != data_.end(); ++it) {
            x2 = std::get<0>(*it);
            y2 = std::get<1>(*it);

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
        if(graphPlotMode::SCATTER == p_m_ || true == draw_arrows_) {
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
 * @param indent The indentation string prepended to every emitted line
 * @return The header code declaring the x/ex/y/ey data arrays for this error graph
 */
std::string
GGraph2ED::headerData_(bool is_secondary, std::size_t p_id, const std::string &indent) const {
    std::ostringstream header_data; // NOLINT(cppcoreguidelines-init-variables)

    // Set up suitable arrays for the header
    std::string base_name = suffix(is_secondary, p_id);
    std::string array_base_name = "array_" + base_name;

    std::string x_array_name = "x_" + array_base_name;
    std::string ex_array_name = "ex_" + array_base_name;
    std::string y_array_name = "y_" + array_base_name;
    std::string ey_array_name = "ey_" + array_base_name;

    std::string comment; // NOLINT(cppcoreguidelines-init-variables)
    if(!ds_marker_.empty()) {
        comment = "// " + ds_marker_;
    }

    header_data << indent << "double " << x_array_name << "[" << to_string(data_.size()) << "];"
                << comment << '\n'
                << indent << "double " << ex_array_name << "[" << to_string(data_.size()) << "];"
                << '\n'
                << indent << "double " << y_array_name << "[" << to_string(data_.size()) << "];"
                << '\n'
                << indent << "double " << ey_array_name << "[" << to_string(data_.size()) << "];"
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
 * @param indent The indentation string prepended to every emitted line
 * @return The body code filling the x/ex/y/ey data arrays with this graph's tuple values
 */
std::string
GGraph2ED::bodyData_(bool is_secondary, std::size_t p_id, const std::string &indent) const {
    std::ostringstream body_data; // NOLINT(cppcoreguidelines-init-variables)

    // Set up suitable arrays for the header
    std::string base_name = suffix(is_secondary, p_id);
    std::string array_base_name = "array_" + base_name;

    std::string x_array_name = "x_" + array_base_name;
    std::string ex_array_name = "ex_" + array_base_name;
    std::string y_array_name = "y_" + array_base_name;
    std::string ey_array_name = "ey_" + array_base_name;

    std::string comment; // NOLINT(cppcoreguidelines-init-variables)
    if(!ds_marker_.empty()) {
        body_data << "// " + ds_marker_ << '\n';
    }

    // Fill data from the tuples into the arrays
    std::vector<std::tuple<double, double, double, double>>::const_iterator it;
    std::size_t pos_counter = 0;

    for(it = data_.begin(); it != data_.end(); ++it) {
        body_data << indent << x_array_name << "[" << pos_counter << "] = " << std::get<0>(*it)
                  << ";" << '\n'
                  << indent << ex_array_name << "[" << pos_counter << "] = " << std::get<1>(*it)
                  << ";" << '\n'
                  << indent << y_array_name << "[" << pos_counter << "] = " << std::get<2>(*it)
                  << ";" << '\n'
                  << indent << ey_array_name << "[" << pos_counter << "] = " << std::get<3>(*it)
                  << ";" << '\n';

        pos_counter++;
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
 * @param indent The indentation string prepended to every emitted line
 * @return The footer code creating and drawing the ROOT TGraphErrors object for this graph
 */
std::string
GGraph2ED::footerData_(bool is_secondary, std::size_t p_id, const std::string &indent) const {
    std::ostringstream footer_data; // NOLINT(cppcoreguidelines-init-variables)

    // Set up suitable arrays for the header
    std::string base_name = suffix(is_secondary, p_id);
    std::string array_base_name = "array_" + base_name;

    std::string x_array_name = "x_" + array_base_name;
    std::string ex_array_name = "ex_" + array_base_name;
    std::string y_array_name = "y_" + array_base_name;
    std::string ey_array_name = "ey_" + array_base_name;

    std::string graph_name = std::string("graph_") + base_name;

    std::string comment; // NOLINT(cppcoreguidelines-init-variables)
    if(!ds_marker_.empty()) {
        footer_data << "// " + ds_marker_ << '\n';
    }

    // Check whether custom drawing arguments have been set or whether one
    // of our generic choices has been selected
    std::string d_a = this->drawingArguments(is_secondary);

    // Fill the data in our tuple-vector into a ROOT TGraphErrors object
    footer_data << indent << "TGraphErrors *" << graph_name << " = new TGraphErrors("
                << data_.size() << ", " << x_array_name << ", " << y_array_name << ", "
                << ex_array_name << " ," << ey_array_name << ");" << '\n'
                << indent << graph_name << "->GetXaxis()->SetTitle(\"" << xAxisLabel() << "\");"
                << '\n'
                << indent << graph_name << "->GetYaxis()->SetTitle(\"" << yAxisLabel() << "\");"
                << '\n';

    if(!plot_label_.empty()) {
        footer_data << indent << graph_name << "->SetTitle(\"" << plot_label_ << "\");" << '\n';
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
 * @param indent The indentation string prepended to every emitted line
 * @return The header code declaring the x/y/z data arrays for this 3D graph
 */
std::string
GGraph3D::headerData_(bool is_secondary, std::size_t p_id, const std::string &indent) const {
    std::ostringstream header_data; // NOLINT(cppcoreguidelines-init-variables)

    // Set up suitable arrays for the header
    std::string base_name = suffix(is_secondary, p_id);
    std::string array_base_name = "array_" + base_name;

    std::string x_array_name = "x_" + array_base_name;
    std::string y_array_name = "y_" + array_base_name;
    std::string z_array_name = "z_" + array_base_name;

    std::string comment; // NOLINT(cppcoreguidelines-init-variables)
    if(!ds_marker_.empty()) {
        comment = "// " + ds_marker_;
    }

    header_data << indent << "double " << x_array_name << "[" << to_string(data_.size()) << "];"
                << (!comment.empty() ? comment : "") << '\n'
                << indent << "double " << y_array_name << "[" << to_string(data_.size()) << "];"
                << '\n'
                << indent << "double " << z_array_name << "[" << to_string(data_.size()) << "];"
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
 * @param indent The indentation string prepended to every emitted line
 * @return The body code filling the x/y/z data arrays with this graph's tuple values
 */
std::string
GGraph3D::bodyData_(bool is_secondary, std::size_t p_id, const std::string &indent) const {
    std::ostringstream body_data; // NOLINT(cppcoreguidelines-init-variables)

    // Set up suitable arrays for the header
    std::string base_name = suffix(is_secondary, p_id);
    std::string array_base_name = "array_" + base_name;

    std::string x_array_name = "x_" + array_base_name;
    std::string y_array_name = "y_" + array_base_name;
    std::string z_array_name = "z_" + array_base_name;

    std::string comment; // NOLINT(cppcoreguidelines-init-variables)
    if(!ds_marker_.empty()) {
        body_data << "// " + ds_marker_ << '\n';
    }

    // Fill data from the tuples into the arrays
    std::vector<std::tuple<double, double, double>>::const_iterator it;
    std::size_t pos_counter = 0;

    for(it = data_.begin(); it != data_.end(); ++it) {
        body_data << indent << x_array_name << "[" << pos_counter << "] = " << std::get<0>(*it)
                  << ";"
                  << "\t" << y_array_name << "[" << pos_counter << "] = " << std::get<1>(*it) << ";"
                  << "\t" << z_array_name << "[" << pos_counter << "] = " << std::get<2>(*it) << ";"
                  << '\n';

        pos_counter++;
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
 * @param indent The indentation string prepended to every emitted line
 * @return The footer code creating and drawing the ROOT TGraph2D (and optional poly-line) for this graph
 */
std::string
GGraph3D::footerData_(bool is_secondary, std::size_t p_id, const std::string &indent) const {
    std::ostringstream footer_data; // NOLINT(cppcoreguidelines-init-variables)

    // Set up suitable arrays for the header
    std::string base_name = suffix(is_secondary, p_id);
    std::string array_base_name = "array_" + base_name;

    std::string x_array_name = "x_" + array_base_name;
    std::string y_array_name = "y_" + array_base_name;
    std::string z_array_name = "z_" + array_base_name;

    std::string graph_name = std::string("graph_") + base_name;

    std::string comment; // NOLINT(cppcoreguidelines-init-variables)
    if(!ds_marker_.empty()) {
        footer_data << "// " + ds_marker_ << '\n';
    }

    // Check whether custom drawing arguments have been set or whether one
    // of our generic choices has been selected
    std::string d_a = this->drawingArguments(is_secondary);

    // Fill the data in our tuple-vector into a ROOT TGraph object
    footer_data << indent << "TGraph2D *" << graph_name << " = new TGraph2D(" << data_.size()
                << ", " << x_array_name << ", " << y_array_name << ", " << z_array_name << ");"
                << '\n'
                << indent << graph_name << "->GetXaxis()->SetTitle(\"" << xAxisLabel() << "\");"
                << '\n'
                << indent << graph_name << "->GetXaxis()->SetTitleOffset(1.5);" << '\n'
                << indent << graph_name << "->GetYaxis()->SetTitle(\"" << yAxisLabel() << "\");"
                << '\n'
                << indent << graph_name << "->GetYaxis()->SetTitleOffset(1.5);" << '\n'
                << indent << graph_name << "->GetZaxis()->SetTitle(\"" << zAxisLabel() << "\");"
                << '\n'
                << indent << graph_name << "->GetZaxis()->SetTitleOffset(1.5);" << '\n'
                << indent << graph_name << "->SetMarkerStyle(20);" << '\n'
                << indent << graph_name << "->SetMarkerSize(1);" << '\n'
                << indent << graph_name << "->SetMarkerColor(2);" << '\n';

    if(!plot_label_.empty()) {
        footer_data << indent << graph_name << "->SetTitle(\"" << plot_label_ << "\");" << '\n';
    }
    else {
        footer_data << indent << graph_name << "->SetTitle(\" \");" << '\n';
    }

    footer_data << indent << graph_name << "->Draw(\"" << d_a << "\");" << '\n' << '\n';

    if(draw_lines_ && data_.size() >= 2) {
        std::vector<std::tuple<double, double, double>>::const_iterator it;
        std::size_t pos_counter = 0;

        double x = 0.0;
        double y = 0.0;
        double z = 0.0;

        footer_data << indent << "TPolyLine3D *lines_" << graph_name << " = new TPolyLine3D("
                    << data_.size() << ");" << '\n'
                    << '\n';

        for(it = data_.begin() + 1; it != data_.end(); ++it) {
            x = std::get<0>(*it);
            y = std::get<1>(*it);
            z = std::get<2>(*it);

            footer_data << indent << "lines_" << graph_name << "->SetPoint(" << pos_counter << ", "
                        << x << ", " << y << ", " << z << ");";

            pos_counter++;
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
std::string GGraph4D::headerData_(bool, std::size_t, std::string const &) const {
    std::ostringstream header_data; // NOLINT(cppcoreguidelines-init-variables)

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
std::string GGraph4D::bodyData_(bool, std::size_t, std::string const &) const {
    std::ostringstream body_data; // NOLINT(cppcoreguidelines-init-variables)

    // nothing

    return body_data.str();
}

/******************************************************************************/
/**
 * Retrieves specific draw commands for this plot
 *
 * @param is_secondary Whether this plotter is a secondary plotter (true) or a primary one (false)
 * @param p_id The id of the parent plotter, used to build unique object names for secondary plotters
 * @param indent The indentation string prepended to every emitted line
 * @return The footer code creating the 3D frame and per-point poly-markers (sized by the fourth component)
 */
std::string
GGraph4D::footerData_(bool is_secondary, std::size_t p_id, const std::string &indent) const {
    std::vector<std::tuple<double, double, double, double>> local_data = data_;

    std::string base_name = suffix(is_secondary, p_id);

    // Sort the data, so we can select the n_best_ best more easily
    if(small_w_large_marker_) {
        std::sort(
            local_data.begin(),
            local_data.end(),
            [](std::tuple<double, double, double, double> a,
               std::tuple<double, double, double, double> b) -> bool {
                return (std::get<3>(a) < std::get<3>(b));
            }
        );
    }
    else {
        std::sort(
            local_data.begin(),
            local_data.end(),
            [](std::tuple<double, double, double, double> a,
               std::tuple<double, double, double, double> b) -> bool {
                return (std::get<3>(a) > std::get<3>(b));
            }
        );
    }

    std::ostringstream footer_data; // NOLINT(cppcoreguidelines-init-variables)

    // Find out about the minimum and maximum values of the data vector
    std::tuple<double, double, double, double, double, double, double, double> min_max =
        getMinMax(local_data);

    // Set up TView object for our 3D data, spanning the minimum and maximum values
    footer_data << indent << "TH3F *fr = new TH3F(\"fr\",\"fr\","
                << "10, " << std::get<0>(min_max) << ", " << std::get<1>(min_max) << ", "
                << "10, " << std::get<2>(min_max) << ", " << std::get<3>(min_max) << ", "
                << "10, " << std::get<4>(min_max) << ", " << std::get<5>(min_max) << ");" << '\n'
                << indent << "fr->SetTitle(\" \");" << '\n'
                << indent << "fr->GetXaxis()->SetTitle(\"" << xAxisLabel() << "\");" << '\n'
                << indent << "fr->GetXaxis()->SetTitleOffset(1.6);" << '\n'
                << indent << "fr->GetYaxis()->SetTitle(\"" << yAxisLabel() << "\");" << '\n'
                << indent << "fr->GetYaxis()->SetTitleOffset(1.6);" << '\n'
                << indent << "fr->GetZaxis()->SetTitle(\"" << zAxisLabel() << "\");" << '\n'
                << indent << "fr->GetZaxis()->SetTitleOffset(1.6);" << '\n'
                << '\n'
                << indent << "fr->Draw();" << '\n';

    double w_min = std::get<6>(min_max);
    double w_max = std::get<7>(min_max);

    // Fill data from the tuples into the arrays
    double w_range = w_max - w_min;
    std::size_t pos = 0;
    std::vector<std::tuple<double, double, double, double>>::const_iterator it;
    for(it = local_data.begin(); it != local_data.end(); ++it) {
        std::string poly_marker_name =
            std::string("pm3d_") + base_name + std::string("_") + to_string(pos);

        // create a TPolyMarker3D for a single data point
        footer_data << indent << "TPolyMarker3D *" << poly_marker_name << " = new TPolyMarker3D(1);"
                    << '\n';

        double x = std::get<0>(*it);
        double y = std::get<1>(*it);
        double z = std::get<2>(*it);
        double w = std::get<3>(*it);

        // Translate the fourth component into a marker size. By default,
        // smaller values will yield the largest value
        double marker_size = 0.;
        if(0 == pos) {
            marker_size = 2 * max_marker_size_;
        }
        else {
            if(small_w_large_marker_) {
                marker_size = min_marker_size_ + (max_marker_size_ - min_marker_size_) *
                                                   pow((1. - (w - w_min) / w_range), 8.);
            }
            else {
                marker_size = min_marker_size_ +
                              (max_marker_size_ - min_marker_size_) * pow(((w - w_min) / w_range), 8);
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
std::string GGraph4D::drawingArguments(bool) const {
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
 * @param indent The indentation string prepended to every emitted line
 * @return The header code declaring the ROOT TH1D histogram (using explicit or auto-detected range)
 */
std::string
GHistogram1D::headerData_(bool is_secondary, std::size_t p_id, const std::string &indent) const {
    std::ostringstream header_data; // NOLINT(cppcoreguidelines-init-variables)

    std::string comment; // NOLINT(cppcoreguidelines-init-variables)
    if(!ds_marker_.empty()) {
        comment = "// " + ds_marker_;
    }

    std::string hist_name = "histD" + suffix(is_secondary, p_id);

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
 * @param indent The indentation string prepended to every emitted line
 * @return The body code filling the ROOT TH1D histogram with this plotter's data values
 */
std::string
GHistogram1D::bodyData_(bool is_secondary, std::size_t p_id, const std::string &indent) const {
    std::ostringstream body_data; // NOLINT(cppcoreguidelines-init-variables)

    std::string comment; // NOLINT(cppcoreguidelines-init-variables)
    if(!ds_marker_.empty()) {
        comment = "// " + ds_marker_;
    }
    else {
        comment = "";
    }

    std::string hist_name = "histD" + suffix(is_secondary, p_id);

    std::vector<double>::const_iterator it;
    std::size_t pos_counter = 0;
    for(it = data_.begin(); it != data_.end(); ++it) {
        body_data << indent << hist_name << "->Fill(" << std::showpoint << *it << ");"
                  << (pos_counter == 0 ? comment : ("")) << '\n';
        pos_counter++;
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
 * @param indent The indentation string prepended to every emitted line
 * @return The footer code setting titles and drawing the ROOT TH1D histogram
 */
std::string
GHistogram1D::footerData_(bool is_secondary, std::size_t p_id, const std::string &indent) const {
    std::ostringstream footer_data; // NOLINT(cppcoreguidelines-init-variables)

    std::string hist_name = "histD" + suffix(is_secondary, p_id);

    if(!plot_label_.empty()) {
        footer_data << indent << hist_name << "->SetTitle(\"" << plot_label_ << "\");" << '\n';
    }
    else {
        footer_data << indent << hist_name << "->SetTitle(\" \");" << '\n';
    }

    std::string comment; // NOLINT(cppcoreguidelines-init-variables)
    if(!ds_marker_.empty()) {
        footer_data << "// " + ds_marker_ << '\n';
    }

    // Check whether custom drawing arguments have been set
    std::string d_a = this->drawingArguments(is_secondary);

    footer_data << indent << hist_name << "->GetXaxis()->SetTitle(\"" << xAxisLabel() << "\");"
                << '\n'
                << indent << hist_name << "->GetYaxis()->SetTitle(\"" << yAxisLabel() << "\");"
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
 * @param indent The indentation string prepended to every emitted line
 * @return The header code declaring the ROOT TH1I integer histogram
 */
std::string
GHistogram1I::headerData_(bool is_secondary, std::size_t p_id, const std::string &indent) const {
    std::ostringstream header_data; // NOLINT(cppcoreguidelines-init-variables)

    std::string comment; // NOLINT(cppcoreguidelines-init-variables)
    if(!ds_marker_.empty()) {
        comment = "// " + ds_marker_;
    }

    std::string hist_name = "histI" + suffix(is_secondary, p_id);

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
 * @param indent The indentation string prepended to every emitted line
 * @return The body code filling the ROOT TH1I histogram with this plotter's integer data values
 */
std::string
GHistogram1I::bodyData_(bool is_secondary, std::size_t p_id, const std::string &indent) const {
    std::ostringstream body_data; // NOLINT(cppcoreguidelines-init-variables)

    std::string comment; // NOLINT(cppcoreguidelines-init-variables)
    if(!ds_marker_.empty()) {
        comment = "// " + ds_marker_;
    }
    else {
        comment = "";
    }

    std::string hist_name = "histI" + suffix(is_secondary, p_id);

    std::vector<std::int32_t>::const_iterator it;
    std::size_t pos_counter = 0;
    for(it = data_.begin(); it != data_.end(); ++it) {
        body_data << indent << hist_name << "->Fill(" << *it << ");"
                  << (pos_counter == 0 ? comment : ("")) << '\n';
        pos_counter++;
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
 * @param indent The indentation string prepended to every emitted line
 * @return The footer code setting titles and drawing the ROOT TH1I histogram
 */
std::string
GHistogram1I::footerData_(bool is_secondary, std::size_t p_id, const std::string &indent) const {
    std::ostringstream footer_data; // NOLINT(cppcoreguidelines-init-variables)

    std::string hist_name = "histI" + suffix(is_secondary, p_id);

    if(!plot_label_.empty()) {
        footer_data << indent << hist_name << "->SetTitle(\"" << plot_label_ << "\");" << '\n';
    }
    else {
        footer_data << indent << hist_name << "->SetTitle(\" \");" << '\n';
    }

    std::string comment; // NOLINT(cppcoreguidelines-init-variables)
    if(!ds_marker_.empty()) {
        footer_data << "// " + ds_marker_ << '\n';
    }

    // Check whether custom drawing arguments have been set
    std::string d_a = this->drawingArguments(is_secondary);

    footer_data << indent << hist_name << "->GetXaxis()->SetTitle(\"" << xAxisLabel() << "\");"
                << '\n'
                << indent << hist_name << "->GetYaxis()->SetTitle(\"" << yAxisLabel() << "\");"
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
 * @param indent The indentation string prepended to every emitted line
 * @return The header code declaring the ROOT TH2D histogram (using explicit or auto-detected ranges)
 */
std::string
GHistogram2D::headerData_(bool is_secondary, std::size_t p_id, const std::string &indent) const {
    std::ostringstream header_data; // NOLINT(cppcoreguidelines-init-variables)

    std::string comment; // NOLINT(cppcoreguidelines-init-variables)
    if(!ds_marker_.empty()) {
        comment = "// " + ds_marker_;
    }

    std::string hist_name = "hist2D" + suffix(is_secondary, p_id);

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
 * @param indent The indentation string prepended to every emitted line
 * @return The body code filling the ROOT TH2D histogram with this plotter's (x, y) data values
 */
std::string
GHistogram2D::bodyData_(bool is_secondary, std::size_t p_id, const std::string &indent) const {
    std::ostringstream body_data; // NOLINT(cppcoreguidelines-init-variables)

    std::string comment; // NOLINT(cppcoreguidelines-init-variables)
    if(!ds_marker_.empty()) {
        comment = "// " + ds_marker_;
    }
    else {
        comment = "";
    }

    std::string hist_name = "hist2D" + suffix(is_secondary, p_id);

    std::vector<std::tuple<double, double>>::const_iterator it;
    std::size_t pos_counter = 0;
    for(it = data_.begin(); it != data_.end(); ++it) {
        body_data << indent << hist_name << "->Fill(" << std::showpoint << std::get<0>(*it) << ", "
                  << std::get<1>(*it) << ");" << (pos_counter == 0 ? comment : ("")) << '\n';
        pos_counter++;
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
 * @param indent The indentation string prepended to every emitted line
 * @return The footer code setting titles and drawing the ROOT TH2D histogram
 */
std::string
GHistogram2D::footerData_(bool is_secondary, std::size_t p_id, const std::string &indent) const {
    std::ostringstream footer_data; // NOLINT(cppcoreguidelines-init-variables)

    std::string hist_name = "hist2D" + suffix(is_secondary, p_id);

    if(!plot_label_.empty()) {
        footer_data << indent << hist_name << "->SetTitle(\"" << plot_label_ << "\");" << '\n';
    }
    else {
        footer_data << indent << hist_name << "->SetTitle(\" \");" << '\n';
    }

    std::string comment; // NOLINT(cppcoreguidelines-init-variables)
    if(!ds_marker_.empty()) {
        footer_data << "// " + ds_marker_ << '\n';
    }

    // Check whether custom drawing arguments have been set
    std::string d_a = this->drawingArguments(is_secondary);

    footer_data << indent << hist_name << "->GetXaxis()->SetTitle(\"" << xAxisLabel() << "\");"
                << '\n'
                << indent << hist_name << "->GetYaxis()->SetTitle(\"" << yAxisLabel() << "\");"
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
 * @param indent The indentation string prepended to every emitted line
 * @return The code to be added to the plot header for this function
 */
std::string GFunctionPlotter1D::headerData_(
    bool is_secondary,
    std::size_t p_id,
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

    std::ostringstream result; // NOLINT(cppcoreguidelines-init-variables)

    std::string comment; // NOLINT(cppcoreguidelines-init-variables)
    if(!ds_marker_.empty()) {
        comment = "// " + ds_marker_;
    }

    std::string function_name = "func1D" + suffix(is_secondary, p_id);
    result << indent << "TF1 *" << function_name << " = new TF1(\"" << function_name << "\", \""
           << function_description_ << "\"," << std::get<0>(x_extremes_) << ", "
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
std::string GFunctionPlotter1D::bodyData_(bool, std::size_t, std::string const &) const {
    // No data needs to be added for a function plotter
    return {};
}

/******************************************************************************/
/**
 * Retrieves specific draw commands for this plot
 *
 * @param is_secondary Whether this plotter is a secondary plotter (true) or a primary one (false)
 * @param p_id The id of the parent plotter, used to build a unique function name for secondary plotters
 * @param indent The indentation string prepended to every emitted line
 * @return The draw command to be added to the plot's data for this function
 */
std::string GFunctionPlotter1D::footerData_(
    bool is_secondary,
    std::size_t p_id,
    const std::string &indent
) const {
    std::ostringstream footer_data; // NOLINT(cppcoreguidelines-init-variables)

    std::string comment; // NOLINT(cppcoreguidelines-init-variables)
    if(!ds_marker_.empty()) {
        comment = "// " + ds_marker_;
    }

    std::string function_name = "func1D" + suffix(is_secondary, p_id);
    footer_data << indent << function_name << "->GetXaxis()->SetTitle(\"" << xAxisLabel() << "\");"
                << '\n'
                << indent << function_name << "->GetYaxis()->SetTitle(\"" << yAxisLabel() << "\");"
                << '\n'
                << indent << function_name << "->SetNpx(" << n_samples_x_ << ");" << '\n';

    if(!plot_label_.empty()) {
        footer_data << indent << function_name << "->SetTitle(\"" << plot_label_ << "\");" << '\n';
    }
    else {
        footer_data << indent << function_name << "->SetTitle(\" \");" << '\n';
    }

    std::string d_a = this->drawingArguments(is_secondary);

    footer_data << indent << function_name << "->Draw(" << d_a << ");"
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
 * @param indent The indentation string prepended to every emitted line
 * @return The code to be added to the plot header for this function
 */
std::string GFunctionPlotter2D::headerData_(
    bool is_secondary,
    std::size_t p_id,
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

    std::ostringstream result; // NOLINT(cppcoreguidelines-init-variables)

    std::string comment; // NOLINT(cppcoreguidelines-init-variables)
    if(!ds_marker_.empty()) {
        comment = "// " + ds_marker_;
    }

    std::string function_name = "func2D" + suffix(is_secondary, p_id);
    result << indent << "TF2 *" << function_name << " = new TF2(\"" << function_name << "\", \""
           << function_description_ << "\"," << std::get<0>(x_extremes_) << ", "
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
std::string GFunctionPlotter2D::bodyData_(bool, std::size_t, std::string const &) const {
    // No data needs to be added for a function plotter
    return {};
}

/******************************************************************************/
/**
 * Retrieves specific draw commands for this plot
 *
 * @param is_secondary Whether this plotter is a secondary plotter (true) or a primary one (false)
 * @param p_id The id of the parent plotter, used to build a unique function name for secondary plotters
 * @param indent The indentation string prepended to every emitted line
 * @return The draw command to be added to the plot's data for this function
 */
std::string GFunctionPlotter2D::footerData_(
    bool is_secondary,
    std::size_t p_id,
    std::string const &indent
) const {
    std::ostringstream footer_data; // NOLINT(cppcoreguidelines-init-variables)

    std::string comment; // NOLINT(cppcoreguidelines-init-variables)
    if(!ds_marker_.empty()) {
        comment = "// " + ds_marker_;
    }

    std::string function_name = "func2D" + suffix(is_secondary, p_id);
    footer_data << indent << function_name << "->GetXaxis()->SetTitle(\"" << xAxisLabel() << "\");"
                << '\n'
                << indent << function_name << "->GetYaxis()->SetTitle(\"" << yAxisLabel() << "\");"
                << '\n'
                << indent << function_name << "->GetZaxis()->SetTitle(\"" << zAxisLabel() << "\");"
                << '\n'
                << indent << function_name << "->SetNpx(" << n_samples_x_ << ");" << '\n'
                << indent << function_name << "->SetNpy(" << n_samples_y_ << ");" << '\n';

    if(!plot_label_.empty()) {
        footer_data << indent << function_name << "->SetTitle(\"" << plot_label_ << "\");" << '\n';
    }
    else {
        footer_data << indent << function_name << "->SetTitle(\" \");" << '\n';
    }

    std::string d_a = this->drawingArguments(is_secondary);

    footer_data << indent << function_name << "->Draw(" << d_a << ");"
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
    std::ofstream result(file_name);
    result << plot(file_name);
    result.close();
}

/******************************************************************************/
/**
 * Emits the overall plot
 *
 * @param plot_name The output file name; used to derive the png file name in the emitted print command
 * (the literal string "empty" or an empty path suppresses the print command)
 * @return The complete ROOT macro source code for the canvas and all registered plotters
 */
std::string GPlotDesigner::plot(const std::filesystem::path &plot_name) const {
    std::ostringstream result; // NOLINT(cppcoreguidelines-init-variables)
    std::size_t max_plots = c_x_div_ * c_y_div_;

    if(plotters_cnt_.size() > max_plots) {
        glogger << "In GPlotDesigner::plot() (Canvas label = \"" << this->getCanvasLabel()
                << "\":" << '\n'
                << "Warning! Found more plots than pads (" << plotters_cnt_.size() << " vs. "
                << max_plots << ")" << '\n'
                << "Some of the plots will be ignored" << '\n'
                << GWARNING;
    }

    result << "{" << '\n' << staticHeader(indent()) << '\n';

    // Plot all body sections up to the maximum allowed number
    result << indent() << "//===================  Header Section ====================" << '\n'
           << '\n';

    // Plot all headers up to the maximum allowed number
    std::size_t n_plots = 0;
    std::vector<std::shared_ptr<GBasePlotter>>::const_iterator it;
    for(it = plotters_cnt_.begin(); it != plotters_cnt_.end(); ++it) {
        if(n_plots++ < max_plots) {
            result << (*it)->headerData(indent()) << '\n';
        }
    }

    // Plot all body sections up to the maximum allowed number
    result << indent() << "//===================  Data Section ======================" << '\n'
           << '\n';

    n_plots = 0;
    for(it = plotters_cnt_.begin(); it != plotters_cnt_.end(); ++it) {
        if(n_plots++ < max_plots) {
            result << (*it)->bodyData(indent()) << '\n';
        }
    }

    // Plot all footer data up to the maximum allowed number
    result << indent() << "//===================  Plot Section ======================" << '\n'
           << '\n';

    n_plots = 0;
    for(it = plotters_cnt_.begin(); it != plotters_cnt_.end(); ++it) {
        if(n_plots < max_plots) {
            result << indent() << "graphPad->cd(" << n_plots + 1 << ");"
                   << '\n' /* cd starts at 1 */
                   << (*it)->footerData(indent()) << '\n';

            n_plots++;
        }
    }

    result << indent() << "graphPad->cd();" << '\n' << indent() << "cc->cd();" << '\n';

    // Check if we are supposed to output a png file
    if(add_print_command_ && plot_name.string() != "empty" && not(plot_name.string()).empty()) {
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
               << indent() << "// Print out the data of this file to a png file" << '\n'
               << indent() << "cc->Print(\"" << plot_name_local << ".png\");" << '\n';
    }

    result << "}" << '\n';

    return result.str();
}

/******************************************************************************/
/**
 * A default header for a ROOT file
 *
 * @param indent The indentation string prepended to every emitted line
 * @return The ROOT macro source code setting up the canvas, title and graph pad
 */
std::string GPlotDesigner::staticHeader(const std::string &indent) const {
    std::ostringstream result; // NOLINT(cppcoreguidelines-init-variables)

    result << indent << "gROOT->Reset();" << '\n'
           << indent << "gStyle->SetCanvasColor(0);" << '\n'
           << indent << "gStyle->SetStatBorderSize(1);" << '\n'
           << indent << "gStyle->SetOptStat(0);" << '\n'
           << '\n'
           << indent << "TCanvas *cc = new TCanvas(\"cc\", \"cc\",0,0," << c_x_dim_ << ","
           << c_y_dim_ << ");" << '\n'
           << '\n'
           << indent << "TPaveLabel* canvasTitle = new TPaveLabel(0.2,0.95,0.8,0.99, \""
           << canvas_label_ << "\");" << '\n'
           << indent << "canvasTitle->Draw();" << '\n'
           << '\n'
           << indent << "TPad* graphPad = new TPad(\"Graphs\", \"Graphs\", 0.01, 0.01, 0.99, 0.94);"
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

} /* namespace Gem::Common */
