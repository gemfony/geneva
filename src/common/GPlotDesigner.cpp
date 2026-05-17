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
 * Puts a gColor into a stream. Needed also for boost::lexical_cast<>
 */
std::ostream &operator<<(std::ostream &o, const gColor &x) {
    ENUMBASETYPE tmp = static_cast<ENUMBASETYPE>(x);
    o << tmp;
    return o;
}

/******************************************************************************/
/**
 * Reads a gColor item from a stream. Needed also for boost::lexical_cast<>
 */
std::istream &operator>>(std::istream &i, gColor &x) {
    ENUMBASETYPE tmp = 0;
    i >> tmp;

#ifdef DEBUG
    x = Gem::Common::narrow_cast<gColor>(tmp);
#else
    x = static_cast<gColor>(tmp);
#endif /* DEBUG */

    return i;
}

/******************************************************************************/
/**
 * Puts a gMarker into a stream. Needed also for boost::lexical_cast<>
 */
std::ostream &operator<<(std::ostream &o, const gMarker &x) {
    ENUMBASETYPE tmp = static_cast<ENUMBASETYPE>(x);
    o << tmp;
    return o;
}

/******************************************************************************/
/**
 * Reads a gMarker item from a stream. Needed also for boost::lexical_cast<>
 */
std::istream &operator>>(std::istream &i, gMarker &x) {
    ENUMBASETYPE tmp = 0;
    i >> tmp;

#ifdef DEBUG
    x = Gem::Common::narrow_cast<gMarker>(tmp);
#else
    x = static_cast<gMarker>(tmp);
#endif /* DEBUG */

    return i;
}

/******************************************************************************/
/**
 * Puts a gLineStyle into a stream. Needed also for boost::lexical_cast<>
 */
std::ostream &operator<<(std::ostream &o, const gLineStyle &x) {
    ENUMBASETYPE tmp = static_cast<ENUMBASETYPE>(x);
    o << tmp;
    return o;
}

/******************************************************************************/
/**
 * Reads a gLineStyle item from a stream. Needed also for boost::lexical_cast<>
 */
std::istream &operator>>(std::istream &i, gLineStyle &x) {
    ENUMBASETYPE tmp = 0;
    i >> tmp;

#ifdef DEBUG
    x = Gem::Common::narrow_cast<gLineStyle>(tmp);
#else
    x = static_cast<gLineStyle>(tmp);
#endif /* DEBUG */

    return i;
}

/******************************************************************************/
/**
 * Puts a graphPlotMode into a stream. Needed also for boost::lexical_cast<>
 */
std::ostream &operator<<(std::ostream &o, const graphPlotMode &x) {
    ENUMBASETYPE tmp = static_cast<ENUMBASETYPE>(x);
    o << tmp;
    return o;
}

/******************************************************************************/
/**
 * Reads a graphPlotMode item from a stream. Needed also for boost::lexical_cast<>
 */
std::istream &operator>>(std::istream &i, graphPlotMode &x) {
    ENUMBASETYPE tmp = 0;
    i >> tmp;

#ifdef DEBUG
    x = Gem::Common::narrow_cast<graphPlotMode>(tmp);
#else
    x = static_cast<graphPlotMode>(tmp);
#endif /* DEBUG */

    return i;
}

/******************************************************************************/
/**
 * Puts a tddropt into a stream. Needed also for boost::lexical_cast<>
 */
std::ostream &operator<<(std::ostream &o, const tddropt &x) {
    ENUMBASETYPE tmp = static_cast<ENUMBASETYPE>(x);
    o << tmp;
    return o;
}

/******************************************************************************/
/**
 * Reads a tddropt item from a stream. Needed also for boost::lexical_cast<>
 */
std::istream &operator>>(std::istream &i, tddropt &x) {
    ENUMBASETYPE tmp = 0;
    i >> tmp;

#ifdef DEBUG
    x = Gem::Common::narrow_cast<tddropt>(tmp);
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
  , drawingArguments_(cp.drawingArguments_)
  , x_axis_label_(cp.x_axis_label_)
  , y_axis_label_(cp.y_axis_label_)
  , z_axis_label_(cp.z_axis_label_)
  , plot_label_(cp.plot_label_)
  , dsMarker_(cp.dsMarker_)
  , id_(cp.id_) {
    // Note: Explicit scope needed for name resolution of clone -- compare
    // https://isocpp.org/wiki/faq/templates#nondependent-name-lookup-members

    // Copy secondary plot data over
    for(auto const &plotter_ptr : cp.secondaryPlotter_) {
        secondaryPlotter_.push_back(plotter_ptr->GCommonInterfaceT<GBasePlotter>::clone());
    }
}

/******************************************************************************/
/**
 * Assignment operator
 */
GBasePlotter &GBasePlotter::operator=(GBasePlotter const &cp) {
    if(this == &cp) {
        return *this;
    }
    GCommonInterfaceT<GBasePlotter>::operator=(cp);

    drawingArguments_ = cp.drawingArguments_;
    x_axis_label_ = cp.x_axis_label_;
    y_axis_label_ = cp.y_axis_label_;
    z_axis_label_ = cp.z_axis_label_;
    plot_label_ = cp.plot_label_;
    dsMarker_ = cp.dsMarker_;
    id_ = cp.id_;

    Gem::Common::copyCloneableSmartPointerContainer(cp.secondaryPlotter_, secondaryPlotter_);

    return *this;
}

/******************************************************************************/
/**
 * Allows to set the drawing arguments for this plot
 *
 * @param drawingArguments The drawing arguments for this plot
 */
void GBasePlotter::setDrawingArguments(std::string drawing_arguments) {
    drawingArguments_ = drawing_arguments;
}

/******************************************************************************/
/**
 * Sets the label for the x-axis
 * */
void GBasePlotter::setXAxisLabel(std::string x_axis_label) {
    x_axis_label_ = x_axis_label;
}

/******************************************************************************/
/**
 * Retrieve the x-axis label
 */
std::string GBasePlotter::xAxisLabel() const {
    return x_axis_label_;
}

/******************************************************************************/
/**
 * Sets the label for the y-axis
 */
void GBasePlotter::setYAxisLabel(std::string y_axis_label) {
    y_axis_label_ = y_axis_label;
}

/******************************************************************************/
/**
 * Retrieve the y-axis label
 */
std::string GBasePlotter::yAxisLabel() const {
    return y_axis_label_;
}

/******************************************************************************/
/**
 * Sets the label for the z-axis
 */
void GBasePlotter::setZAxisLabel(std::string z_axis_label) {
    z_axis_label_ = z_axis_label;
}

/******************************************************************************/
/**
 * Retrieve the z-axis label
 */
std::string GBasePlotter::zAxisLabel() const {
    return z_axis_label_;
}

/******************************************************************************/
/**
 * Allows to assign a label to the entire plot
 *
 * @param pL A label to be assigned to the entire plot
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
 * @param A marker that has been assigned to the output data structures
 */
void GBasePlotter::setDataStructureMarker(std::string ds_marker) {
    dsMarker_ = ds_marker;
}

/******************************************************************************/
/**
 * Allows to retrieve the data structure marker
 *
 * @return The marker that has been assigned to the output data structures
 */
std::string GBasePlotter::dsMarker() const {
    return dsMarker_;
}

/******************************************************************************/
/**
 * Allows to add secondary plots to be added to the same sub-canvas
 */
void GBasePlotter::registerSecondaryPlotter(std::shared_ptr<GBasePlotter> sp) {
    // Check that the secondary plot isn't empty
    if(not sp) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In GBasePlotter::registerSecondaryPlot(): Error!" << '\n'
            << "Got empty secondary plot" << '\n'
        );
    }

    // Check that the secondary plotter is compatible with us
    if(not this->isCompatible(sp)) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In GBasePlotter::registerSecondaryPlot(): Error!" << '\n'
            << "Received incompatible secondary plotter" << '\n'
            << sp->getPlotterName() << " in plotter " << this->getPlotterName() << '\n'
        );
    }

    // Add the plotter to our collection
    secondaryPlotter_.push_back(sp);
}

/******************************************************************************/
/**
 * Check that a given plotter is compatible with us. By default we only
 * check that the names of both plotters match. If other plot types are
 * compatible with this plotter, you need to overload this function.
 */
bool GBasePlotter::isCompatible(std::shared_ptr<GBasePlotter> other) const {
    return (this->getPlotterName() == other->getPlotterName());
}

/******************************************************************************/
/**
 * calculate a suffix from id and parent ids
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
 */
std::string GBasePlotter::name_() const {
    return std::string("GBasePlotter");
}

/******************************************************************************/
/**
 * Searches for compliance with expectations with respect to another object
 * of the same type
 */
void GBasePlotter::compare_(
    const GBasePlotter &cp,
    const expectation &e,
    const double & /*limit*/
) const {
    // Check that we are dealing with a GBasePlotter reference independent of this object and convert the pointer
    const auto *p_load = g_convert_and_compare(cp, this);

    GToken token("GBasePlotter", e);

    // Compare our parent data ...
    compare_base_t<GCommonInterfaceT<GBasePlotter>>(*this, *p_load, token);

    // ... and then the local data
    compare_t(IDENTITY(drawingArguments_, p_load->drawingArguments_), token);
    compare_t(IDENTITY(x_axis_label_, p_load->x_axis_label_), token);
    compare_t(IDENTITY(y_axis_label_, p_load->y_axis_label_), token);
    compare_t(IDENTITY(z_axis_label_, p_load->z_axis_label_), token);
    compare_t(IDENTITY(plot_label_, p_load->plot_label_), token);
    compare_t(IDENTITY(dsMarker_, p_load->dsMarker_), token);
    compare_t(IDENTITY(secondaryPlotter_, p_load->secondaryPlotter_), token);
    compare_t(IDENTITY(id_, p_load->id_), token);

    // React on deviations from the expectation
    token.evaluate();
}

/******************************************************************************/
/**
 * Loads the data of another object
 */
void GBasePlotter::load_(const GBasePlotter *cp) {
    // Check that we are dealing with a GBasePlotter reference independent of this object and convert the pointer
    const auto *p_load = g_convert_and_compare(cp, this);

    // No parent class with loadable data

    // Load local data
    drawingArguments_ = p_load->drawingArguments_;
    x_axis_label_ = p_load->x_axis_label_;
    y_axis_label_ = p_load->y_axis_label_;
    z_axis_label_ = p_load->z_axis_label_;
    plot_label_ = p_load->plot_label_;
    dsMarker_ = p_load->dsMarker_;
    id_ = p_load->id_;

    copyCloneableSmartPointerContainer(p_load->secondaryPlotter_, secondaryPlotter_);
}

/******************************************************************************/
/**
 * Retrieve header settings for this plot (and any sub-plots)
 */
std::string GBasePlotter::headerData(const std::string &indent) const {
    std::ostringstream header_data; // NOLINT(cppcoreguidelines-init-variables)

    // Add this plot's data
    header_data << indent << "// Header data for primary plotter" << '\n'
                << this->headerData_(false, 0, indent);

    // Extract data from the secondary plotters, if any
    std::size_t pos = 0;
    std::vector<std::shared_ptr<GBasePlotter>>::const_iterator cit;
    for(cit = secondaryPlotter_.begin(); cit != secondaryPlotter_.end(); ++cit) {
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
 */
std::string GBasePlotter::bodyData(const std::string &indent) const {
    std::ostringstream body_data; // NOLINT(cppcoreguidelines-init-variables)

    // Add this plot's data
    body_data << indent << "// Body data for primary plotter" << '\n'
              << this->bodyData_(false, 0, indent);

    // Extract data from the secondary plotters, if any
    std::size_t pos = 0;
    for(auto const &plotter_ptr : secondaryPlotter_) {
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
 */
std::string GBasePlotter::footerData(const std::string &indent) const {
    std::ostringstream footer_data; // NOLINT(cppcoreguidelines-init-variables)

    // Add this plot's data
    footer_data << indent << "// Footer data for primary plotter" << '\n'
                << this->footerData_(false, 0, indent);

    // Extract data from the secondary plotters, if any
    std::size_t pos = 0;
    std::vector<std::shared_ptr<GBasePlotter>>::const_iterator cit;
    for(cit = secondaryPlotter_.begin(); cit != secondaryPlotter_.end(); ++cit) {
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
 * @param dA The desired value of the drawArrows_ variable
 */
void GGraph2D::setDrawArrows(bool d_a) {
    drawArrows_ = d_a;
}

/******************************************************************************/
/**
 * Retrieves the value of the drawArrows_ variable
 *
 * @return The value of the drawArrows_ variable
 */
bool GGraph2D::getDrawArrows() const {
    return drawArrows_;
}

/******************************************************************************/
/**
 * Determines whether a scatter plot or a curve is created
 *
 * @param pM The desired plot mode
 */
void GGraph2D::setPlotMode(graphPlotMode p_m) {
    pM_ = p_m;
}

/******************************************************************************/
/**
 * Allows to retrieve the current plotting mode
 *
 * @return The current plot mode
 */
graphPlotMode GGraph2D::getPlotMode() const {
    return pM_;
}

/******************************************************************************/
/**
 * Retrieves a unique name for this plotter
 */
std::string GGraph2D::getPlotterName() const {
    return "GGraph2D";
}

/******************************************************************************/
/**
 * Returns the name of this class
 */
std::string GGraph2D::name_() const {
    return std::string("GGraph2D");
}

/******************************************************************************/
/**
 * Searches for compliance with expectations with respect to another object
 * of the same type
 */
void GGraph2D::compare_(
    const GBasePlotter &cp,
    const expectation &e,
    const double & /*limit*/
) const {
    // Check that we are dealing with a GGraph2D reference independent of this object and convert the pointer
    const auto *p_load = g_convert_and_compare(cp, this);

    GToken token("GGraph2D", e);

    // Compare our parent data ...
    compare_base_t<GDataCollector2T<double, double>>(*this, *p_load, token);

    // ... and then the local data
    compare_t(IDENTITY(pM_, p_load->pM_), token);
    compare_t(IDENTITY(drawArrows_, p_load->drawArrows_), token);

    // React on deviations from the expectation
    token.evaluate();
}

/******************************************************************************/
/**
 * Retrieve specific header settings for this plot
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
    if(dsMarker_ != "") {
        comment = "// " + dsMarker_;
    }

    header_data << indent << "double " << x_array_name << "[" << to_string(data_.size()) << "];"
                << (comment != "" ? comment : "") << '\n'
                << indent << "double " << y_array_name << "[" << to_string(data_.size()) << "];"
                << '\n'
                << '\n';

    return header_data.str();
}

/******************************************************************************/
/**
 * Retrieves the actual data sets
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
    if(dsMarker_ != "") {
        body_data << "// " + dsMarker_ << '\n';
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
    if(dsMarker_ != "") {
        footer_data << "// " + dsMarker_ << '\n';
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

    if(plot_label_ != "") {
        footer_data << indent << graph_name << "->SetTitle(\"" << plot_label_ << "\");" << '\n';
    }
    else {
        footer_data << indent << graph_name << "->SetTitle(\" \");" << '\n';
    }

    footer_data << indent << graph_name << "->Draw(\"" << d_a << "\");" << '\n' << '\n';

    if(drawArrows_ && data_.size() >= 2) {
        std::vector<std::tuple<double, double>>::const_iterator it;
        std::size_t pos_counter = 0;

        double x1 = std::get<0>(*data_.begin());
        double y1 = std::get<1>(*data_.begin());
        double x2 = 0., y2 = 0.;

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
 */
std::string GGraph2D::drawingArguments(bool is_secondary) const {
    std::string d_a;

    if(this->drawingArguments_ != "") {
        d_a = this->drawingArguments_;
    }
    else {
        if(graphPlotMode::SCATTER == pM_ || true == drawArrows_) {
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
 */
GBasePlotter *GGraph2D::clone_() const {
    return new GGraph2D(*this);
}

/******************************************************************************/
/**
 * Loads the data of another object
 */
void GGraph2D::load_(const GBasePlotter *cp) {
    // Check that we are dealing with a GGraph2D reference independent of this object and convert the pointer
    const auto *p_load = g_convert_and_compare(cp, this);

    // Load our parent class'es data ...
    GDataCollector2T<double, double>::load_(cp);

    // ... and then our local data
    pM_ = p_load->pM_;
    drawArrows_ = p_load->drawArrows_;
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * Determines whether a scatter plot or a curve is created
 *
 * @param pM The desired plot mode
 */
void GGraph2ED::setPlotMode(graphPlotMode p_m) {
    pM_ = p_m;
}

/******************************************************************************/
/**
 * Allows to retrieve the current plotting mode
 *
 * @return The current plot mode
 */
graphPlotMode GGraph2ED::getPlotMode() const {
    return pM_;
}

/******************************************************************************/
/**
 * Retrieves a unique name for this plotter
 */
std::string GGraph2ED::getPlotterName() const {
    return "GGraph2ED";
}

/******************************************************************************/
/**
 * Returns the name of this class
 */
std::string GGraph2ED::name_() const {
    return std::string("GGraph2ED");
}

/******************************************************************************/
/**
 * Searches for compliance with expectations with respect to another object
 * of the same type
 */
void GGraph2ED::compare_(
    const GBasePlotter &cp,
    const expectation &e,
    const double & /*limit*/
) const {
    // Check that we are dealing with a GBasePlotter reference independent of this object and convert the pointer
    const auto *p_load = g_convert_and_compare(cp, this);

    GToken token("GGraph2ED", e);

    // Compare our parent data ...
    compare_base_t<GDataCollector2ET<double, double>>(*this, *p_load, token);

    // ... and then the local data
    compare_t(IDENTITY(pM_, p_load->pM_), token);

    // React on deviations from the expectation
    token.evaluate();
}

/******************************************************************************/
/**
 * Retrieve specific header settings for this plot
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
    if(dsMarker_ != "") {
        comment = "// " + dsMarker_;
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
    if(dsMarker_ != "") {
        body_data << "// " + dsMarker_ << '\n';
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
    if(dsMarker_ != "") {
        footer_data << "// " + dsMarker_ << '\n';
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

    if(plot_label_ != "") {
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
 */
std::string GGraph2ED::drawingArguments(bool is_secondary) const {
    std::string d_a;

    if(this->drawingArguments_ != "") {
        d_a = this->drawingArguments_;
    }
    else {
        if(graphPlotMode::SCATTER == pM_) {
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
 */
GBasePlotter *GGraph2ED::clone_() const {
    return new GGraph2ED(*this);
}

/******************************************************************************/
/**
 * Loads the data of another object
 */
void GGraph2ED::load_(const GBasePlotter *cp) {
    // Check that we are dealing with a GGraph2ED reference independent of this object and convert the pointer
    const auto *p_load = g_convert_and_compare(cp, this);

    // Load our parent class'es data ...
    GDataCollector2ET<double, double>::load_(cp);

    // ... and then our local data
    pM_ = p_load->pM_;
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * Adds lines to the plots between consecutive points.
 *
 * @param dL The desired value of the drawLines_ variable
 */
void GGraph3D::setDrawLines(bool d_l) {
    drawLines_ = d_l;
}

/******************************************************************************/
/**
 * Retrieves the value of the drawLines_ variable
 *
 * @return The value of the drawLines_ variable
 */
bool GGraph3D::getDrawLines() const {
    return drawLines_;
}

/******************************************************************************/
/**
 * Retrieves a unique name for this plotter
 */
std::string GGraph3D::getPlotterName() const {
    return "GGraph3D";
}

/******************************************************************************/
/**
 * Returns the name of this class
 */
std::string GGraph3D::name_() const {
    return std::string("GGraph3D");
}

/******************************************************************************/
/**
 * Searches for compliance with expectations with respect to another object
 * of the same type
 */
void GGraph3D::compare_(
    const GBasePlotter &cp,
    const expectation &e,
    const double & /*limit*/
) const {
    // Check that we are dealing with a GGraph3D reference independent of this object and convert the pointer
    const auto *p_load = g_convert_and_compare(cp, this);

    GToken token("GGraph3D", e);

    // Compare our parent data ...
    compare_base_t<GDataCollector3T<double, double, double>>(*this, *p_load, token);

    // ... and then the local data
    compare_t(IDENTITY(drawLines_, p_load->drawLines_), token);

    // React on deviations from the expectation
    token.evaluate();
}

/******************************************************************************/
/**
 * Retrieve specific header settings for this plot
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
    if(dsMarker_ != "") {
        comment = "// " + dsMarker_;
    }

    header_data << indent << "double " << x_array_name << "[" << to_string(data_.size()) << "];"
                << (comment != "" ? comment : "") << '\n'
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
    if(dsMarker_ != "") {
        body_data << "// " + dsMarker_ << '\n';
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
    if(dsMarker_ != "") {
        footer_data << "// " + dsMarker_ << '\n';
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

    if(plot_label_ != "") {
        footer_data << indent << graph_name << "->SetTitle(\"" << plot_label_ << "\");" << '\n';
    }
    else {
        footer_data << indent << graph_name << "->SetTitle(\" \");" << '\n';
    }

    footer_data << indent << graph_name << "->Draw(\"" << d_a << "\");" << '\n' << '\n';

    if(drawLines_ && data_.size() >= 2) {
        std::vector<std::tuple<double, double, double>>::const_iterator it;
        std::size_t pos_counter = 0;

        double x = 0.0, y = 0.0, z = 0.0;

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
 */
std::string GGraph3D::drawingArguments(bool is_secondary) const {
    std::string d_a;

    if(this->drawingArguments_ != "") {
        d_a = this->drawingArguments_;
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
 */
GBasePlotter *GGraph3D::clone_() const {
    return new GGraph3D(*this);
}

/******************************************************************************/
/**
 * Loads the data of another object
 */
void GGraph3D::load_(const GBasePlotter *cp) {
    // Check that we are dealing with a GGraph3D reference independent of this object and convert the pointer
    const auto *p_load = g_convert_and_compare(cp, this);

    // Load our parent class'es data ...
    GDataCollector3T<double, double, double>::load_(cp);

    // ... and then our local data
    drawLines_ = p_load->drawLines_;
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * Allows to set the minimum marker size
 */
void GGraph4D::setMinMarkerSize(const double &min_marker_size) {
    if(min_marker_size < 0.) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In GGraph4D::setMinMarkerSize(): Error!" << '\n'
            << "Received invalid minimum marker size: " << min_marker_size << '\n'
        );
    }

    minMarkerSize_ = min_marker_size;
}

/******************************************************************************/
/**
 * Allows to set the maximum marker size
 */
void GGraph4D::setMaxMarkerSize(const double &max_marker_size) {
    if(max_marker_size < 0. || max_marker_size < minMarkerSize_) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In GGraph4D::setMinMarkerSize(): Error!" << '\n'
            << "Received invalid minimum marker size: " << minMarkerSize_ << " " << max_marker_size
            << "." << '\n'
            << "Always set the lower boundary first." << '\n'
        );
    }

    maxMarkerSize_ = max_marker_size;
}

/******************************************************************************/
/**
 * Allows to retrieve the minimum marker size
 */
double GGraph4D::getMinMarkerSize() const {
    return minMarkerSize_;
}

/******************************************************************************/
/**
 * Allows to retrieve the maximum marker size
 */
double GGraph4D::getMaxMarkerSize() const {
    return maxMarkerSize_;
}

/******************************************************************************/
/**
 * Allows to specify whether small w yield large markers
 */
void GGraph4D::setSmallWLargeMarker(const bool &swlm) {
    smallWLargeMarker_ = swlm;
}

/******************************************************************************/
/**
 * Allows to check whether small w yield large markers
 */
bool GGraph4D::getSmallWLargeMarker() const {
    return smallWLargeMarker_;
}

/******************************************************************************/
/**
 * Allows to set the number of solutions the class should show. Setting the value
 * to 0 will result in all data being displayed.
 */
void GGraph4D::setNBest(const std::size_t &n_best) {
    nBest_ = n_best;
}

/******************************************************************************/
/**
 * Allows to retrieve the number of solutions the class should show
 */
std::size_t GGraph4D::getNBest() const {
    return nBest_;
}

/******************************************************************************/
/**
 * Retrieves a unique name for this plotter
 */
std::string GGraph4D::getPlotterName() const {
    return "GGraph4D";
}

/******************************************************************************/
/**
 * Returns the name of this class
 */
std::string GGraph4D::name_() const {
    return std::string("GGraph4D");
}

/******************************************************************************/
/**
 * Searches for compliance with expectations with respect to another object
 * of the same type
 */
void GGraph4D::compare_(
    const GBasePlotter &cp,
    const expectation &e,
    const double & /*limit*/
) const {
    // Check that we are dealing with a GGraph3D reference independent of this object and convert the pointer
    const auto *p_load = g_convert_and_compare(cp, this);

    GToken token("GGraph4D", e);

    // Compare our parent data ...
    compare_base_t<GDataCollector4T<double, double, double, double>>(*this, *p_load, token);

    // ... and then the local data
    compare_t(IDENTITY(minMarkerSize_, p_load->minMarkerSize_), token);
    compare_t(IDENTITY(maxMarkerSize_, p_load->maxMarkerSize_), token);
    compare_t(IDENTITY(smallWLargeMarker_, p_load->smallWLargeMarker_), token);
    compare_t(IDENTITY(nBest_, p_load->nBest_), token);

    // React on deviations from the expectation
    token.evaluate();
}

/******************************************************************************/
/**
 * Retrieve specific header settings for this plot
 */
std::string GGraph4D::headerData_(bool, std::size_t, std::string const &) const {
    std::ostringstream header_data; // NOLINT(cppcoreguidelines-init-variables)

    // nothing

    return header_data.str();
}

/******************************************************************************/
/**
 * Retrieves the actual data sets
 */
std::string GGraph4D::bodyData_(bool, std::size_t, std::string const &) const {
    std::ostringstream body_data; // NOLINT(cppcoreguidelines-init-variables)

    // nothing

    return body_data.str();
}

/******************************************************************************/
/**
 * Retrieves specific draw commands for this plot
 */
std::string
GGraph4D::footerData_(bool is_secondary, std::size_t p_id, const std::string &indent) const {
    std::vector<std::tuple<double, double, double, double>> local_data = data_;

    std::string base_name = suffix(is_secondary, p_id);

    // Sort the data, so we can select the nBest_ best more easily
    if(smallWLargeMarker_) {
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
            marker_size = 2 * maxMarkerSize_;
        }
        else {
            if(smallWLargeMarker_) {
                marker_size = minMarkerSize_ + (maxMarkerSize_ - minMarkerSize_) *
                                                   pow((1. - (w - w_min) / w_range), 8.);
            }
            else {
                marker_size = minMarkerSize_ +
                              (maxMarkerSize_ - minMarkerSize_) * pow(((w - w_min) / w_range), 8);
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

        if(nBest_ && pos >= nBest_) {
            break;
        }
    }

    footer_data << '\n';

    return footer_data.str();
}

/******************************************************************************/
/**
 * Retrieve the current drawing arguments
 */
std::string GGraph4D::drawingArguments(bool) const {
    std::string d_a;

    // nothing

    return d_a;
}

/******************************************************************************/
/**
 * Creates a deep clone of this object
 */
GBasePlotter *GGraph4D::clone_() const {
    return new GGraph4D(*this);
}

/******************************************************************************/
/**
 * Loads the data of another object
 */
void GGraph4D::load_(const GBasePlotter *cp) {
    // Check that we are dealing with a GGraph4D reference independent of this object and convert the pointer
    const auto *p_load = g_convert_and_compare(cp, this);

    // Load our parent class'es data ...
    GDataCollector4T<double, double, double, double>::load_(cp);

    // ... and then our local data
    minMarkerSize_ = p_load->minMarkerSize_;
    maxMarkerSize_ = p_load->maxMarkerSize_;
    smallWLargeMarker_ = p_load->smallWLargeMarker_;
    nBest_ = p_load->nBest_;
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * Initialization with number of bins and automatic range detection
 */
GHistogram1D::GHistogram1D(const std::size_t &n_bins_x)
  : nBinsX_(n_bins_x) { /* nothing */
}

/******************************************************************************/
/**
 * Initialization with a range in the form of a tuple
 */
GHistogram1D::GHistogram1D(const std::size_t &n_bins_x, const double &min_x, const double &max_x)
  : nBinsX_(n_bins_x)
  , minX_(min_x)
  , maxX_(max_x) { /* nothing */
}

/******************************************************************************/
/**
 * Initialization with a range in the form of a tuple
 */
GHistogram1D::GHistogram1D(const std::size_t &n_bins_x, const std::tuple<double, double> &range_x)
  : nBinsX_(n_bins_x)
  , minX_(std::get<0>(range_x))
  , maxX_(std::get<1>(range_x)) { /* nothing */
}

/******************************************************************************/
/**
 * Retrieve specific header settings for this plot
 */
std::string
GHistogram1D::headerData_(bool is_secondary, std::size_t p_id, const std::string &indent) const {
    std::ostringstream header_data; // NOLINT(cppcoreguidelines-init-variables)

    std::string comment; // NOLINT(cppcoreguidelines-init-variables)
    if(dsMarker_ != "") {
        comment = "// " + dsMarker_;
    }

    std::string hist_name = "histD" + suffix(is_secondary, p_id);

    if(minX_ != maxX_) {
        header_data << indent << "TH1D *" << hist_name << " = new TH1D(\"" << hist_name << "\", \""
                    << hist_name << "\"," << nBinsX_ << ", " << minX_ << ", " << maxX_ << ");"
                    << (comment != "" ? comment : "") << '\n'
                    << '\n';
    }
    else { // automatic range detection
        std::tuple<double, double> minmax = this->getMinMaxElements();
        header_data << indent << "TH1D *" << hist_name << " = new TH1D(\"" << hist_name << "\", \""
                    << hist_name << "\"," << nBinsX_ << ", " << std::get<0>(minmax) << ", "
                    << std::get<1>(minmax) << ");" << (comment != "" ? comment : "") << '\n'
                    << '\n';
    }

    return header_data.str();
}

/******************************************************************************/
/**
 * Retrieves the actual data sets
 */
std::string
GHistogram1D::bodyData_(bool is_secondary, std::size_t p_id, const std::string &indent) const {
    std::ostringstream body_data; // NOLINT(cppcoreguidelines-init-variables)

    std::string comment; // NOLINT(cppcoreguidelines-init-variables)
    if(dsMarker_ != "") {
        comment = "// " + dsMarker_;
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
 */
std::string
GHistogram1D::footerData_(bool is_secondary, std::size_t p_id, const std::string &indent) const {
    std::ostringstream footer_data; // NOLINT(cppcoreguidelines-init-variables)

    std::string hist_name = "histD" + suffix(is_secondary, p_id);

    if(plot_label_ != "") {
        footer_data << indent << hist_name << "->SetTitle(\"" << plot_label_ << "\");" << '\n';
    }
    else {
        footer_data << indent << hist_name << "->SetTitle(\" \");" << '\n';
    }

    std::string comment; // NOLINT(cppcoreguidelines-init-variables)
    if(dsMarker_ != "") {
        footer_data << "// " + dsMarker_ << '\n';
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
 */
std::string GHistogram1D::drawingArguments(bool is_secondary) const {
    std::string d_a;

    if(drawingArguments_ != "") {
        d_a = drawingArguments_;
    }
    else {
        if(is_secondary) {
            if("" == d_a) {
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
    return nBinsX_;
}

/******************************************************************************/
/**
 * Retrieve the lower boundary of the plot
 *
 * @return The lower boundary of the plot
 */
double GHistogram1D::getMinX() const {
    return minX_;
}

/******************************************************************************/
/**
 * Retrieve the upper boundary of the plot
 *
 * @return The upper boundary of the plot
 */
double GHistogram1D::getMaxX() const {
    return maxX_;
}

/******************************************************************************/
/**
 * Retrieves a unique name for this plotter
 */
std::string GHistogram1D::getPlotterName() const {
    return "GHistogram1D";
}

/******************************************************************************/
/**
 * Returns the name of this class
 */
std::string GHistogram1D::name_() const {
    return std::string("GHistogram1D");
}

/******************************************************************************/
/**
 * Searches for compliance with expectations with respect to another object
 * of the same type
 */
void GHistogram1D::compare_(
    const GBasePlotter &cp,
    const expectation &e,
    const double & /*limit*/
) const {
    // Check that we are dealing with a GBasePlotter reference independent of this object and convert the pointer
    const auto *p_load = g_convert_and_compare(cp, this);

    GToken token("GHistogram1D", e);

    // Compare our parent data ...
    compare_base_t<GDataCollector1T<double>>(*this, *p_load, token);

    // ... and then the local data
    compare_t(IDENTITY(nBinsX_, p_load->nBinsX_), token);
    compare_t(IDENTITY(minX_, p_load->minX_), token);
    compare_t(IDENTITY(maxX_, p_load->maxX_), token);

    // React on deviations from the expectation
    token.evaluate();
}

/******************************************************************************/
/**
 * Creates a deep clone of this object
 */
GBasePlotter *GHistogram1D::clone_() const {
    return new GHistogram1D(*this);
}

/******************************************************************************/
/**
 * Loads the data of another object
 */
void GHistogram1D::load_(const GBasePlotter *cp) {
    // Check that we are dealing with a GHistogram1D reference independent of this object and convert the pointer
    const auto *p_load = g_convert_and_compare(cp, this);

    // Load our parent class'es data ...
    GDataCollector1T<double>::load_(cp);

    // ... and then our local data
    nBinsX_ = p_load->nBinsX_;
    minX_ = p_load->minX_;
    maxX_ = p_load->maxX_;
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * The standard constructor
 */
GHistogram1I::GHistogram1I(const std::size_t &n_bins_x, const double &min_x, const double &max_x)
  : nBinsX_(n_bins_x)
  , minX_(min_x)
  , maxX_(max_x) { /* nothing */
}

/******************************************************************************/
/**
 * Initialization with a range in the form of a tuple
 */
GHistogram1I::GHistogram1I(const std::size_t &n_bins_x, const std::tuple<double, double> &range_x)
  : nBinsX_(n_bins_x)
  , minX_(std::get<0>(range_x))
  , maxX_(std::get<1>(range_x)) { /* nothing */
}

/******************************************************************************/
/**
 * Retrieve specific header settings for this plot
 */
std::string
GHistogram1I::headerData_(bool is_secondary, std::size_t p_id, const std::string &indent) const {
    std::ostringstream header_data; // NOLINT(cppcoreguidelines-init-variables)

    std::string comment; // NOLINT(cppcoreguidelines-init-variables)
    if(dsMarker_ != "") {
        comment = "// " + dsMarker_;
    }

    std::string hist_name = "histI" + suffix(is_secondary, p_id);

    header_data << indent << "TH1I *" << hist_name << " = new TH1I(\"" << hist_name << "\", \""
                << hist_name << "\"," << nBinsX_ << ", " << minX_ << ", " << maxX_ << ");"
                << (comment != "" ? comment : "") << '\n'
                << '\n';

    return header_data.str();
}

/******************************************************************************/
/**
 * Retrieves the actual data sets
 */
std::string
GHistogram1I::bodyData_(bool is_secondary, std::size_t p_id, const std::string &indent) const {
    std::ostringstream body_data; // NOLINT(cppcoreguidelines-init-variables)

    std::string comment; // NOLINT(cppcoreguidelines-init-variables)
    if(dsMarker_ != "") {
        comment = "// " + dsMarker_;
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
 */
std::string
GHistogram1I::footerData_(bool is_secondary, std::size_t p_id, const std::string &indent) const {
    std::ostringstream footer_data; // NOLINT(cppcoreguidelines-init-variables)

    std::string hist_name = "histI" + suffix(is_secondary, p_id);

    if(plot_label_ != "") {
        footer_data << indent << hist_name << "->SetTitle(\"" << plot_label_ << "\");" << '\n';
    }
    else {
        footer_data << indent << hist_name << "->SetTitle(\" \");" << '\n';
    }

    std::string comment; // NOLINT(cppcoreguidelines-init-variables)
    if(dsMarker_ != "") {
        footer_data << "// " + dsMarker_ << '\n';
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
 */
std::string GHistogram1I::drawingArguments(bool is_secondary) const {
    std::string d_a;

    if(drawingArguments_ != "") {
        d_a = drawingArguments_;
    }
    else {
        if(is_secondary) {
            if("" == d_a) {
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
    return nBinsX_;
}

/******************************************************************************/
/**
 * Retrieve the lower boundary of the plot
 *
 * @return The lower boundary of the plot
 */
double GHistogram1I::getMinX() const {
    return minX_;
}

/******************************************************************************/
/**
 * Retrieve the upper boundary of the plot
 *
 * @return The upper boundary of the plot
 */
double GHistogram1I::getMaxX() const {
    return maxX_;
}

/******************************************************************************/
/**
 * Retrieves a unique name for this plotter
 */
std::string GHistogram1I::getPlotterName() const {
    return "GHistogram1I";
}

/******************************************************************************/
/**
 * Returns the name of this class
 */
std::string GHistogram1I::name_() const {
    return std::string("GHistogram1I");
}

/******************************************************************************/
/**
 * Searches for compliance with expectations with respect to another object
 * of the same type
 */
void GHistogram1I::compare_(
    const GBasePlotter &cp,
    const expectation &e,
    const double & /*limit*/
) const {
    // Check that we are dealing with a GBasePlotter reference independent of this object and convert the pointer
    const auto *p_load = g_convert_and_compare(cp, this);

    GToken token("GHistogram1I", e);

    // Compare our parent data ...
    compare_base_t<GDataCollector1T<std::int32_t>>(*this, *p_load, token);

    // ... and then the local data
    compare_t(IDENTITY(nBinsX_, p_load->nBinsX_), token);
    compare_t(IDENTITY(minX_, p_load->minX_), token);
    compare_t(IDENTITY(maxX_, p_load->maxX_), token);

    // React on deviations from the expectation
    token.evaluate();
}

/******************************************************************************/
/**
 * Creates a deep clone of this object
 */
GBasePlotter *GHistogram1I::clone_() const {
    return new GHistogram1I(*this);
}

/******************************************************************************/
/**
 * Loads the data of another object
 */
void GHistogram1I::load_(const GBasePlotter *cp) {
    // Check that we are dealing with a GHistogram1I reference independent of this object and convert the pointer
    const auto *p_load = g_convert_and_compare(cp, this);

    // Load our parent class'es data ...
    GDataCollector1T<std::int32_t>::load_(cp);

    // ... and then our local data
    nBinsX_ = p_load->nBinsX_;
    minX_ = p_load->minX_;
    maxX_ = p_load->maxX_;
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * The standard constructor
 */
GHistogram2D::GHistogram2D(
    const std::size_t &n_bins_x,
    const std::size_t &n_bins_y,
    const double &min_x,
    const double &max_x,
    const double &min_y,
    const double &max_y
)
  : nBinsX_(n_bins_x)
  , nBinsY_(n_bins_y)
  , minX_(min_x)
  , maxX_(max_x)
  , minY_(min_y)
  , maxY_(max_y)
  , dropt_(tddropt::TDEMPTY) { /* nothing */
}

/******************************************************************************/
/**
 * Initialization with ranges
 */
GHistogram2D::GHistogram2D(
    const std::size_t &n_bins_x,
    const std::size_t &n_bins_y,
    const std::tuple<double, double> &range_x,
    const std::tuple<double, double> &range_y
)
  : nBinsX_(n_bins_x)
  , nBinsY_(n_bins_y)
  , minX_(std::get<0>(range_x))
  , maxX_(std::get<1>(range_x))
  , minY_(std::get<0>(range_y))
  , maxY_(std::get<1>(range_y))
  , dropt_(tddropt::TDEMPTY) { /* nothing */
}

/******************************************************************************/
/**
 * Initialization with automatic range detection
 */
GHistogram2D::GHistogram2D(const std::size_t &n_bins_x, const std::size_t &n_bins_y)
  : nBinsX_(n_bins_x)
  , nBinsY_(n_bins_y)
  , minX_(0)
  , maxX_(minX_)
  , minY_(0)
  , maxY_(minY_)
  , dropt_(tddropt::TDEMPTY) { /* nothing */
}

/******************************************************************************/
/**
 * Retrieve specific header settings for this plot
 */
std::string
GHistogram2D::headerData_(bool is_secondary, std::size_t p_id, const std::string &indent) const {
    std::ostringstream header_data; // NOLINT(cppcoreguidelines-init-variables)

    std::string comment; // NOLINT(cppcoreguidelines-init-variables)
    if(dsMarker_ != "") {
        comment = "// " + dsMarker_;
    }

    std::string hist_name = "hist2D" + suffix(is_secondary, p_id);

    if(minX_ != maxX_ && minY_ != maxY_) {
        header_data << indent << "TH2D *" << hist_name << " = new TH2D(\"" << hist_name << "\", \""
                    << hist_name << "\"," << nBinsX_ << ", " << minX_ << ", " << maxX_ << ","
                    << nBinsY_ << ", " << minY_ << ", " << maxY_ << ");"
                    << (comment != "" ? comment : "") << '\n'
                    << '\n';
    }
    else { // // automatic range detection
        std::tuple<double, double, double, double> minmax = this->getMinMaxElements();

        header_data << indent << "TH2D *" << hist_name << " = new TH2D(\"" << hist_name << "\", \""
                    << hist_name << "\"," << nBinsX_ << ", " << std::get<0>(minmax) << ", "
                    << std::get<1>(minmax) << "," << nBinsY_ << ", " << std::get<2>(minmax) << ", "
                    << std::get<3>(minmax) << ");" << (comment != "" ? comment : "") << '\n'
                    << '\n';
    }

    return header_data.str();
}

/******************************************************************************/
/**
 * Retrieves the actual data sets
 */
std::string
GHistogram2D::bodyData_(bool is_secondary, std::size_t p_id, const std::string &indent) const {
    std::ostringstream body_data; // NOLINT(cppcoreguidelines-init-variables)

    std::string comment; // NOLINT(cppcoreguidelines-init-variables)
    if(dsMarker_ != "") {
        comment = "// " + dsMarker_;
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
 */
std::string
GHistogram2D::footerData_(bool is_secondary, std::size_t p_id, const std::string &indent) const {
    std::ostringstream footer_data; // NOLINT(cppcoreguidelines-init-variables)

    std::string hist_name = "hist2D" + suffix(is_secondary, p_id);

    if(plot_label_ != "") {
        footer_data << indent << hist_name << "->SetTitle(\"" << plot_label_ << "\");" << '\n';
    }
    else {
        footer_data << indent << hist_name << "->SetTitle(\" \");" << '\n';
    }

    std::string comment; // NOLINT(cppcoreguidelines-init-variables)
    if(dsMarker_ != "") {
        footer_data << "// " + dsMarker_ << '\n';
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
 */
std::string GHistogram2D::drawingArguments(bool is_secondary) const {
    std::string d_a;

    if(drawingArguments_ != "") {
        d_a = drawingArguments_;
    }
    else {
        switch(dropt_) {
        case tddropt::TDEMPTY:
            d_a = "";
            break;

        case tddropt::SURFONE:
            d_a = "SURF1";
            break;

        case tddropt::SURFTWOZ:
            d_a = "SURF2Z";
            break;

        case tddropt::SURFTHREE:
            d_a = "SURF3";
            break;

        case tddropt::SURFFOUR:
            d_a = "SURF4";
            break;

        case tddropt::CONTZ:
            d_a = "CONTZ";
            break;

        case tddropt::CONTONE:
            d_a = "CONT1";
            break;

        case tddropt::CONTTWO:
            d_a = "CONT2";
            break;

        case tddropt::CONTTHREE:
            d_a = "CONT3";
            break;

        case tddropt::TEXT:
            d_a = "TEXT";
            break;

        case tddropt::SCAT:
            d_a = "SCAT";
            break;

        case tddropt::BOX:
            d_a = "BOX";
            break;

        case tddropt::ARR:
            d_a = "ARR";
            break;

        case tddropt::COLZ:
            d_a = "COLZ";
            break;

        case tddropt::LEGO:
            d_a = "LEGO";
            break;

        case tddropt::LEGOONE:
            d_a = "LEGO1";
            break;

        case tddropt::SURFONEPOL:
            d_a = "SURF1POL";
            break;

        case tddropt::SURFONECYL:
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
 */
void GHistogram2D::set2DOpt(tddropt dropt) {
    dropt_ = dropt;
}

/******************************************************************************/
/**
 * Allows to retrieve 2d-drawing options
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
    return nBinsX_;
}

/******************************************************************************/
/**
 * Retrieve the number of bins in y-direction
 *
 * @return The number of bins in y-direction
 */
std::size_t GHistogram2D::getNBinsY() const {
    return nBinsY_;
}

/******************************************************************************/
/**
 * Retrieve the lower boundary of the plot in x-direction
 *
 * @return The lower boundary of the plot in x-direction
 */
double GHistogram2D::getMinX() const {
    return minX_;
}

/******************************************************************************/
/**
 * Retrieve the upper boundary of the plot in x-direction
 *
 * @return The upper boundary of the plot in x-direction
 */
double GHistogram2D::getMaxX() const {
    return maxX_;
}

/******************************************************************************/
/**
 * Retrieve the lower boundary of the plot in y-direction
 *
 * @return The lower boundary of the plot in y-direction
 */
double GHistogram2D::getMinY() const {
    return minY_;
}

/******************************************************************************/
/**
 * Retrieve the upper boundary of the plot in y-direction
 *
 * @return The upper boundary of the plot in y-direction
 */
double GHistogram2D::getMaxY() const {
    return maxY_;
}

/******************************************************************************/
/**
 * Retrieves a unique name for this plotter
 */
std::string GHistogram2D::getPlotterName() const {
    return "GHistogram2D";
}

/******************************************************************************/
/**
 * Returns the name of this class
 */
std::string GHistogram2D::name_() const {
    return std::string("GHistogram2D");
}

/******************************************************************************/
/**
 * Searches for compliance with expectations with respect to another object
 * of the same type
 */
void GHistogram2D::compare_(
    const GBasePlotter &cp,
    const expectation &e,
    const double & /*limit*/
) const {
    // Check that we are dealing with a GBasePlotter reference independent of this object and convert the pointer
    const auto *p_load = g_convert_and_compare(cp, this);

    GToken token("GHistogram2D", e);

    // Compare our parent data ...
    compare_base_t<GDataCollector2T<double, double>>(*this, *p_load, token);

    // ... and then the local data
    compare_t(IDENTITY(nBinsX_, p_load->nBinsX_), token);
    compare_t(IDENTITY(nBinsY_, p_load->nBinsY_), token);
    compare_t(IDENTITY(minX_, p_load->minX_), token);
    compare_t(IDENTITY(maxX_, p_load->maxX_), token);
    compare_t(IDENTITY(minY_, p_load->minY_), token);
    compare_t(IDENTITY(maxY_, p_load->maxY_), token);
    compare_t(IDENTITY(dropt_, p_load->dropt_), token);

    // React on deviations from the expectation
    token.evaluate();
}

/******************************************************************************/
/**
 * Creates a deep clone of this object
 */
GBasePlotter *GHistogram2D::clone_() const {
    return new GHistogram2D(*this);
}

/******************************************************************************/
/**
 * Loads the data of another object
 */
void GHistogram2D::load_(const GBasePlotter *cp) {
    // Check that we are dealing with a GHistogram2D reference independent of this object and convert the pointer
    const auto *p_load = g_convert_and_compare(cp, this);

    // Load our parent class'es data ...
    GDataCollector2T<double, double>::load_(cp);

    // ... and then our local data
    nBinsX_ = p_load->nBinsX_;
    nBinsY_ = p_load->nBinsY_;
    minX_ = p_load->minX_;
    maxX_ = p_load->maxX_;
    minY_ = p_load->minY_;
    maxY_ = p_load->maxY_;
    dropt_ = p_load->dropt_;
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * The standard constructor. Some member variables may be initialized in the
 * class body.
 *
 * @param fD A descriptor for the function to be plotted
 */
GFunctionPlotter1D::GFunctionPlotter1D(
    const std::string &f_d,
    const std::tuple<double, double> &x_extremes
)
  : functionDescription_(f_d)
  , xExtremes_(x_extremes) { /* nothing */
}

/******************************************************************************/
/**
 * Allows to set the number of sampling points of the function on the x-axis
 *
 * @param n_samples_x The number of sampling points of the function on the x-axis
 */
void GFunctionPlotter1D::setNSamplesX(std::size_t n_samples_x) {
    nSamplesX_ = n_samples_x;
}

/******************************************************************************/
/**
 * Retrieves a unique name for this plotter
 */
std::string GFunctionPlotter1D::getPlotterName() const {
    return "GFunctionPlotter1D";
}

/******************************************************************************/
/**
 * Returns the name of this class
 */
std::string GFunctionPlotter1D::name_() const {
    return std::string("GFunctionPlotter1D");
}

/******************************************************************************/
/**
 * Searches for compliance with expectations with respect to another object
 * of the same type
 */
void GFunctionPlotter1D::compare_(
    const GBasePlotter &cp,
    const expectation &e,
    const double & /*limit*/
) const {
    // Check that we are dealing with a GGraph3D reference independent of this object and convert the pointer
    const auto *p_load = g_convert_and_compare(cp, this);

    GToken token("GFunctionPlotter1D", e);

    // Compare our parent data ...
    compare_base_t<GBasePlotter>(*this, *p_load, token);

    // ... and then the local data
    compare_t(IDENTITY(functionDescription_, p_load->functionDescription_), token);
    compare_t(IDENTITY(xExtremes_, p_load->xExtremes_), token);
    compare_t(IDENTITY(nSamplesX_, p_load->nSamplesX_), token);

    // React on deviations from the expectation
    token.evaluate();
}

/******************************************************************************/
/**
 * Retrieve specific header settings for this plot
 *
 * @return The code to be added to the plot header for this function
 */
std::string GFunctionPlotter1D::headerData_(
    bool is_secondary,
    std::size_t p_id,
    const std::string &indent
) const {
    // Check the extreme values for consistency
    if(std::get<0>(xExtremes_) >= std::get<1>(xExtremes_)) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In GFunctionPlotter1D::headerData_(): Error!" << '\n'
            << "lower boundary >= upper boundary: " << std::get<0>(xExtremes_) << " / "
            << std::get<1>(xExtremes_) << '\n'
        );
    }

    std::ostringstream result; // NOLINT(cppcoreguidelines-init-variables)

    std::string comment; // NOLINT(cppcoreguidelines-init-variables)
    if(dsMarker_ != "") {
        comment = "// " + dsMarker_;
    }

    std::string function_name = "func1D" + suffix(is_secondary, p_id);
    result << indent << "TF1 *" << function_name << " = new TF1(\"" << function_name << "\", \""
           << functionDescription_ << "\"," << std::get<0>(xExtremes_) << ", "
           << std::get<1>(xExtremes_) << ");" << (comment != "" ? comment : "") << '\n';

    return result.str();
}

/******************************************************************************/
/**
 * Retrieves the actual data sets
 *
 * @return The code to be added to the plot's data section for this function
 */
std::string GFunctionPlotter1D::bodyData_(bool, std::size_t, std::string const &) const {
    // No data needs to be added for a function plotter
    return {};
}

/******************************************************************************/
/**
 * Retrieves specific draw commands for this plot
 *
 * @return The draw command to be added to the plot's data for this function
 */
std::string GFunctionPlotter1D::footerData_(
    bool is_secondary,
    std::size_t p_id,
    const std::string &indent
) const {
    std::ostringstream footer_data; // NOLINT(cppcoreguidelines-init-variables)

    std::string comment; // NOLINT(cppcoreguidelines-init-variables)
    if(dsMarker_ != "") {
        comment = "// " + dsMarker_;
    }

    std::string function_name = "func1D" + suffix(is_secondary, p_id);
    footer_data << indent << function_name << "->GetXaxis()->SetTitle(\"" << xAxisLabel() << "\");"
                << '\n'
                << indent << function_name << "->GetYaxis()->SetTitle(\"" << yAxisLabel() << "\");"
                << '\n'
                << indent << function_name << "->SetNpx(" << nSamplesX_ << ");" << '\n';

    if(plot_label_ != "") {
        footer_data << indent << function_name << "->SetTitle(\"" << plot_label_ << "\");" << '\n';
    }
    else {
        footer_data << indent << function_name << "->SetTitle(\" \");" << '\n';
    }

    std::string d_a = this->drawingArguments(is_secondary);

    footer_data << indent << function_name << "->Draw(" << d_a << ");"
                << (comment != "" ? comment : "") << '\n'
                << '\n';

    return footer_data.str();
}

/******************************************************************************/
/**
 * Retrieve the current drawing arguments
 */
std::string GFunctionPlotter1D::drawingArguments(bool is_secondary) const {
    std::string d_a;

    if(this->drawingArguments_ != "") {
        d_a = this->drawingArguments_;
    }

    if(is_secondary) {
        if("" == d_a) {
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
 */
GBasePlotter *GFunctionPlotter1D::clone_() const {
    return new GFunctionPlotter1D(*this);
}

/******************************************************************************/
/**
 * Loads the data of another object
 */
void GFunctionPlotter1D::load_(const GBasePlotter *cp) {
    // Check that we are dealing with a GFunctionPlotter1D reference independent of this object and convert the pointer
    const auto *p_load = g_convert_and_compare(cp, this);

    // Load our parent class'es data ...
    GBasePlotter::load_(cp);

    // ... and then our local data
    functionDescription_ = p_load->functionDescription_;
    xExtremes_ = p_load->xExtremes_;
    nSamplesX_ = p_load->nSamplesX_;
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * The standard constructor
 *
 * @param fD A descriptor for the function to be plotted
 */
GFunctionPlotter2D::GFunctionPlotter2D(
    const std::string &f_d,
    const std::tuple<double, double> &x_extremes,
    const std::tuple<double, double> &y_extremes
)
  : functionDescription_(f_d)
  , xExtremes_(x_extremes)
  , yExtremes_(y_extremes) { /* nothing */
}

/******************************************************************************/
/**
 * Allows to set the number of sampling points of the function on the x-axis
 *
 * @param n_samples_x The number of sampling points of the function on the x-axis
 */
void GFunctionPlotter2D::setNSamplesX(std::size_t n_samples_x) {
    nSamplesX_ = n_samples_x;
}

/******************************************************************************/
/**
 * Allows to set the number of sampling points of the function on the y-axis
 *
 * @param n_samples_y The number of sampling points of the function on the y-axis
 */
void GFunctionPlotter2D::setNSamplesY(std::size_t n_samples_y) {
    nSamplesY_ = n_samples_y;
}

/******************************************************************************/
/**
 * Retrieves a unique name for this plotter
 */
std::string GFunctionPlotter2D::getPlotterName() const {
    return "GFunctionPlotter2D";
}

/******************************************************************************/
/**
 * Returns the name of this class
 */
std::string GFunctionPlotter2D::name_() const {
    return std::string("GFunctionPlotter2D");
}

/******************************************************************************/
/**
 * Searches for compliance with expectations with respect to another object
 * of the same type
 */
void GFunctionPlotter2D::compare_(
    const GBasePlotter &cp,
    const expectation &e,
    const double & /*limit*/
) const {
    // Check that we are dealing with a GGraph3D reference independent of this object and convert the pointer
    const auto *p_load = g_convert_and_compare(cp, this);

    GToken token("GFunctionPlotter2D", e);

    // Compare our parent data ...
    compare_base_t<GBasePlotter>(*this, *p_load, token);

    // ... and then the local data
    compare_t(IDENTITY(functionDescription_, p_load->functionDescription_), token);
    compare_t(IDENTITY(xExtremes_, p_load->xExtremes_), token);
    compare_t(IDENTITY(yExtremes_, p_load->yExtremes_), token);
    compare_t(IDENTITY(nSamplesX_, p_load->nSamplesX_), token);
    compare_t(IDENTITY(nSamplesY_, p_load->nSamplesY_), token);

    // React on deviations from the expectation
    token.evaluate();
}

/******************************************************************************/
/**
 * Retrieve specific header settings for this plot
 *
 * @return The code to be added to the plot header for this function
 */
std::string GFunctionPlotter2D::headerData_(
    bool is_secondary,
    std::size_t p_id,
    const std::string &indent
) const {
    // Check the extreme values for consistency
    if(std::get<0>(xExtremes_) >= std::get<1>(xExtremes_)) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In GFunctionPlotter2D::headerData_(): Error!" << '\n'
            << "lower boundary(x) >= upper boundary(x): " << std::get<0>(xExtremes_) << " / "
            << std::get<1>(xExtremes_) << '\n'
        );
    }

    if(std::get<0>(yExtremes_) >= std::get<1>(yExtremes_)) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In GFunctionPlotter2D::headerData_(): Error!" << '\n'
            << "lower boundary(y) >= upper boundary(y): " << std::get<0>(yExtremes_) << " / "
            << std::get<1>(yExtremes_) << '\n'
        );
    }

    std::ostringstream result; // NOLINT(cppcoreguidelines-init-variables)

    std::string comment; // NOLINT(cppcoreguidelines-init-variables)
    if(dsMarker_ != "") {
        comment = "// " + dsMarker_;
    }

    std::string function_name = "func2D" + suffix(is_secondary, p_id);
    result << indent << "TF2 *" << function_name << " = new TF2(\"" << function_name << "\", \""
           << functionDescription_ << "\"," << std::get<0>(xExtremes_) << ", "
           << std::get<1>(xExtremes_) << ", " << std::get<0>(yExtremes_) << ", "
           << std::get<1>(yExtremes_) << ");" << (comment != "" ? comment : "") << '\n';

    return result.str();
}

/******************************************************************************/
/**
 * Retrieves the actual data sets
 *
 * @return The code to be added to the plot's data section for this function
 */
std::string GFunctionPlotter2D::bodyData_(bool, std::size_t, std::string const &) const {
    // No data needs to be added for a function plotter
    return {};
}

/******************************************************************************/
/**
 * Retrieves specific draw commands for this plot
 *
 * @return The draw command to be added to the plot's data for this function
 */
std::string GFunctionPlotter2D::footerData_(
    bool is_secondary,
    std::size_t p_id,
    std::string const &indent
) const {
    std::ostringstream footer_data; // NOLINT(cppcoreguidelines-init-variables)

    std::string comment; // NOLINT(cppcoreguidelines-init-variables)
    if(dsMarker_ != "") {
        comment = "// " + dsMarker_;
    }

    std::string function_name = "func2D" + suffix(is_secondary, p_id);
    footer_data << indent << function_name << "->GetXaxis()->SetTitle(\"" << xAxisLabel() << "\");"
                << '\n'
                << indent << function_name << "->GetYaxis()->SetTitle(\"" << yAxisLabel() << "\");"
                << '\n'
                << indent << function_name << "->GetZaxis()->SetTitle(\"" << zAxisLabel() << "\");"
                << '\n'
                << indent << function_name << "->SetNpx(" << nSamplesX_ << ");" << '\n'
                << indent << function_name << "->SetNpy(" << nSamplesY_ << ");" << '\n';

    if(plot_label_ != "") {
        footer_data << indent << function_name << "->SetTitle(\"" << plot_label_ << "\");" << '\n';
    }
    else {
        footer_data << indent << function_name << "->SetTitle(\" \");" << '\n';
    }

    std::string d_a = this->drawingArguments(is_secondary);

    footer_data << indent << function_name << "->Draw(" << d_a << ");"
                << (comment != "" ? comment : "") << '\n'
                << '\n';

    return footer_data.str();
}

/******************************************************************************/
/**
 * Retrieve the current drawing arguments
 */
std::string GFunctionPlotter2D::drawingArguments(bool is_secondary) const {
    std::string d_a;

    if(is_secondary) {
        if("" == d_a) {
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
 */
GBasePlotter *GFunctionPlotter2D::clone_() const {
    return new GFunctionPlotter2D(*this);
}

/******************************************************************************/
/**
 * Loads the data of another object
 */
void GFunctionPlotter2D::load_(const GBasePlotter *cp) {
    // Check that we are dealing with a GFunctionPlotter2D reference independent of this object and convert the pointer
    const auto *p_load = g_convert_and_compare(cp, this);

    // Load our parent class'es data ...
    GBasePlotter::load_(cp);

    // ... and then our local data
    functionDescription_ = p_load->functionDescription_;
    xExtremes_ = p_load->xExtremes_;
    yExtremes_ = p_load->yExtremes_;
    nSamplesX_ = p_load->nSamplesX_;
    nSamplesY_ = p_load->nSamplesY_;
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * The standard constructor. Note that some variables are initialized in the
 * class body.
 *
 * @param The label of the canvas
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
/*
 * Emits the overall plot
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
            g_error_streamer(DO_LOG, time_and_place)
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
 * @param c_x_dim The y-dimension of the output canvas
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
 * @param c_x_dim The x-dimension of the output canvas
 * @param c_x_dim The y-dimension of the output canvas
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
 */
void GPlotDesigner::setCanvasLabel(const std::string &canvas_label) {
    canvas_label_ = canvas_label;
}

/******************************************************************************/
/**
 * Allows to retrieve the canvas label
 */
std::string GPlotDesigner::getCanvasLabel() const {
    return canvas_label_;
}

/******************************************************************************/
/**
 * Allows to add a "Print" command to the end of the script so that picture files are created
 */
void GPlotDesigner::setAddPrintCommand(bool add_print_command) {
    add_print_command_ = add_print_command;
}

/******************************************************************************/
/**
 * Allows to retrieve the current value of the addPrintCommand_ variable
 */
bool GPlotDesigner::getAddPrintCommand() const {
    return add_print_command_;
}

/******************************************************************************/
/**
 * Allows to set the number of spaces used for indention
 */
void GPlotDesigner::setNIndentionSpaces(const std::size_t &n_indention_spaces) {
    n_indention_spaces_ = n_indention_spaces;
}

/******************************************************************************/
/**
 * Allows to retrieve the number spaces used for indention
 */
std::size_t GPlotDesigner::getNIndentionSpaces() const {
    return n_indention_spaces_;
}

/******************************************************************************/
/**
 * Returns the current number of indention spaces as a string
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
 */
std::string GPlotDesigner::name_() const {
    return std::string("GPlotDesigner");
}

/******************************************************************************/
/**
 * Searches for compliance with expectations with respect to another object
 * of the same type
 */
void GPlotDesigner::compare_(
    const GPlotDesigner &cp,
    const expectation &e,
    const double & /*limit*/
) const {
    // Check that we are dealing with a GPlotDesigner reference independent of this object and convert the pointer
    const auto *p_load = g_convert_and_compare(cp, this);

    GToken token("GPlotDesigner", e);

    // Compare our parent data ...
    compare_base_t<GCommonInterfaceT<GPlotDesigner>>(*this, *p_load, token);

    // ... and then the local data
    compare_t(IDENTITY(plotters_cnt_, p_load->plotters_cnt_), token);
    compare_t(IDENTITY(c_x_div_, p_load->c_x_div_), token);
    compare_t(IDENTITY(c_y_div_, p_load->c_y_div_), token);
    compare_t(IDENTITY(c_x_dim_, p_load->c_x_dim_), token);
    compare_t(IDENTITY(c_y_dim_, p_load->c_y_dim_), token);
    compare_t(IDENTITY(canvas_label_, p_load->canvas_label_), token);
    compare_t(IDENTITY(add_print_command_, p_load->add_print_command_), token);
    compare_t(IDENTITY(n_indention_spaces_, p_load->n_indention_spaces_), token);

    // React on deviations from the expectation
    token.evaluate();
}

/******************************************************************************/
/**
 * Creates a deep clone of this object
 */
GPlotDesigner *GPlotDesigner::clone_() const {
    return new GPlotDesigner(*this);
}

/******************************************************************************/
/**
 * Loads the data of another object
 */
void GPlotDesigner::load_(const GPlotDesigner *cp) {
    // Check that we are dealing with a GBasePlotter reference independent of this object and convert the pointer
    const auto *p_load = g_convert_and_compare(cp, this);

    // No "loadable" parent class

    // Load local data
    copyCloneableSmartPointerContainer(p_load->plotters_cnt_, plotters_cnt_);
    c_x_div_ = p_load->c_x_div_;
    c_y_div_ = p_load->c_y_div_;
    c_x_dim_ = p_load->c_x_dim_;
    c_y_dim_ = p_load->c_y_dim_;
    canvas_label_ = p_load->canvas_label_;
    add_print_command_ = p_load->add_print_command_;
    n_indention_spaces_ = p_load->n_indention_spaces_;
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Gem::Common */
