/**
 * @file
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt)
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) eu
 *
 * This file is part of the Geneva library collection.
 *
 * Geneva was developed with kind support from Karlsruhe Institute of
 * Technology (KIT) and Steinbuch Centre for Computing (SCC). Further
 * information about KIT and SCC can be found at http://www.kit.edu/english
 * and http://scc.kit.edu .
 *
 * Geneva is free software: you can redistribute and/or modify it under
 * the terms of version 3 of the GNU Affero General Public License
 * as published by the Free Software Foundation.
 *
 * Geneva is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with the Geneva library. If not, see <http://www.gnu.org/licenses/>.
 *
 * For further information on Gemfony scientific and Geneva, visit
 * http://www.gemfony.eu .
 */

#include "common/GPlotDesigner.hpp"

namespace Gem {
namespace Common {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
// Streamers

/******************************************************************************/
/**
 * Puts a Gem::Common::gColor into a stream. Needed also for boost::lexical_cast<>
 */
std::ostream &operator<<(std::ostream &o, const Gem::Common::gColor &x) {
	Gem::Common::ENUMBASETYPE tmp = static_cast<Gem::Common::ENUMBASETYPE>(x);
	o << tmp;
	return o;
}

/******************************************************************************/
/**
 * Reads a Gem::Common::gColor item from a stream. Needed also for boost::lexical_cast<>
 */
std::istream &operator>>(std::istream &i, Gem::Common::gColor &x) {
	Gem::Common::ENUMBASETYPE tmp;
	i >> tmp;

#ifdef DEBUG
	x = boost::numeric_cast<Gem::Common::gColor>(tmp);
#else
	x = static_cast<Gem::Common::gColor>(tmp);
#endif /* DEBUG */

	return i;
}

/******************************************************************************/
/**
 * Puts a Gem::Common::gMarker into a stream. Needed also for boost::lexical_cast<>
 */
std::ostream &operator<<(std::ostream &o, const Gem::Common::gMarker &x) {
	Gem::Common::ENUMBASETYPE tmp = static_cast<Gem::Common::ENUMBASETYPE>(x);
	o << tmp;
	return o;
}

/******************************************************************************/
/**
 * Reads a Gem::Common::gMarker item from a stream. Needed also for boost::lexical_cast<>
 */
std::istream &operator>>(std::istream &i, Gem::Common::gMarker &x) {
	Gem::Common::ENUMBASETYPE tmp;
	i >> tmp;

#ifdef DEBUG
	x = boost::numeric_cast<Gem::Common::gMarker>(tmp);
#else
	x = static_cast<Gem::Common::gMarker>(tmp);
#endif /* DEBUG */

	return i;
}

/******************************************************************************/
/**
 * Puts a Gem::Common::gLineStyle into a stream. Needed also for boost::lexical_cast<>
 */
std::ostream &operator<<(std::ostream &o, const Gem::Common::gLineStyle &x) {
	Gem::Common::ENUMBASETYPE tmp = static_cast<Gem::Common::ENUMBASETYPE>(x);
	o << tmp;
	return o;
}

/******************************************************************************/
/**
 * Reads a Gem::Common::gLineStyle item from a stream. Needed also for boost::lexical_cast<>
 */
std::istream &operator>>(std::istream &i, Gem::Common::gLineStyle &x) {
	Gem::Common::ENUMBASETYPE tmp;
	i >> tmp;

#ifdef DEBUG
	x = boost::numeric_cast<Gem::Common::gLineStyle>(tmp);
#else
	x = static_cast<Gem::Common::gLineStyle>(tmp);
#endif /* DEBUG */

	return i;
}

/******************************************************************************/
/**
 * Puts a Gem::Common::graphPlotMode into a stream. Needed also for boost::lexical_cast<>
 */
std::ostream &operator<<(std::ostream &o, const Gem::Common::graphPlotMode &x) {
	Gem::Common::ENUMBASETYPE tmp = static_cast<Gem::Common::ENUMBASETYPE>(x);
	o << tmp;
	return o;
}

/******************************************************************************/
/**
 * Reads a Gem::Common::graphPlotMode item from a stream. Needed also for boost::lexical_cast<>
 */
std::istream &operator>>(std::istream &i, Gem::Common::graphPlotMode &x) {
	Gem::Common::ENUMBASETYPE tmp;
	i >> tmp;

#ifdef DEBUG
	x = boost::numeric_cast<Gem::Common::graphPlotMode>(tmp);
#else
	x = static_cast<Gem::Common::graphPlotMode>(tmp);
#endif /* DEBUG */

	return i;
}

/******************************************************************************/
/**
 * Puts a Gem::Common::tddropt into a stream. Needed also for boost::lexical_cast<>
 */
std::ostream &operator<<(std::ostream &o, const Gem::Common::tddropt &x) {
	Gem::Common::ENUMBASETYPE tmp = static_cast<Gem::Common::ENUMBASETYPE>(x);
	o << tmp;
	return o;
}

/******************************************************************************/
/**
 * Reads a Gem::Common::tddropt item from a stream. Needed also for boost::lexical_cast<>
 */
std::istream &operator>>(std::istream &i, Gem::Common::tddropt &x) {
	Gem::Common::ENUMBASETYPE tmp;
	i >> tmp;

#ifdef DEBUG
	x = boost::numeric_cast<Gem::Common::tddropt>(tmp);
#else
	x = static_cast<Gem::Common::tddropt>(tmp);
#endif /* DEBUG */

	return i;
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * The default constructor. Data members are initialized in the class body.
 */
GBasePlotter::GBasePlotter()
{ /* nothing */ }

/******************************************************************************/
/**
 * The copy constructor
 *
 * @param cp A copy of another GBasePlotter object
 */
GBasePlotter::GBasePlotter(const GBasePlotter &cp)
	: m_drawingArguments(cp.m_drawingArguments)
	  , m_x_axis_label(cp.m_x_axis_label)
	  , m_y_axis_label(cp.m_y_axis_label)
	  , m_z_axis_label(cp.m_z_axis_label)
	  , m_plot_label(cp.m_plot_label)
	  , dsMarker_(cp.dsMarker_)
	  , secondaryPlotter_()
	  , id_(cp.id_)
{
	// Note: Explicit scope needed for name resolution of clone -- compare
	// https://isocpp.org/wiki/faq/templates#nondependent-name-lookup-members

	// Copy secondary plot data over
	for(auto plotter_ptr: cp.secondaryPlotter_) { // std::shared_ptr may be copied
		secondaryPlotter_.push_back(plotter_ptr->GCommonInterfaceT<GBasePlotter>::clone());
	}
}

/******************************************************************************/
/**
 * Allows to set the drawing arguments for this plot
 *
 * @param drawingArguments The drawing arguments for this plot
 */
void GBasePlotter::setDrawingArguments(std::string drawingArguments) {
	m_drawingArguments = drawingArguments;
}

/******************************************************************************/
/**
 * Sets the label for the x-axis
 * */
void GBasePlotter::setXAxisLabel(std::string x_axis_label) {
	m_x_axis_label = x_axis_label;
}

/******************************************************************************/
/**
 * Retrieve the x-axis label
 */
std::string GBasePlotter::xAxisLabel() const {
	return m_x_axis_label;
}

/******************************************************************************/
/**
 * Sets the label for the y-axis
 */
void GBasePlotter::setYAxisLabel(std::string y_axis_label) {
	m_y_axis_label = y_axis_label;
}

/******************************************************************************/
/**
 * Retrieve the y-axis label
 */
std::string GBasePlotter::yAxisLabel() const {
	return m_y_axis_label;
}

/******************************************************************************/
/**
 * Sets the label for the z-axis
 */
void GBasePlotter::setZAxisLabel(std::string z_axis_label) {
	m_z_axis_label = z_axis_label;
}

/******************************************************************************/
/**
 * Retrieve the z-axis label
 */
std::string GBasePlotter::zAxisLabel() const {
	return m_z_axis_label;
}

/******************************************************************************/
/**
 * Allows to assign a label to the entire plot
 *
 * @param pL A label to be assigned to the entire plot
 */
void GBasePlotter::setPlotLabel(std::string pL) {
	m_plot_label = pL;
}

/******************************************************************************/
/**
 * Allows to retrieve the plot label
 *
 * @return The label that has been assigned to the plot
 */
std::string GBasePlotter::plotLabel() const {
	return m_plot_label;
}

/******************************************************************************/
/**
 * Allows to assign a marker to data structures in the output file
 *
 * @param A marker that has been assigned to the output data structures
 */
void GBasePlotter::setDataStructureMarker(std::string dsMarker) {
	dsMarker_ = dsMarker;
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
	if (not sp) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GBasePlotter::registerSecondaryPlot(): Error!" << std::endl
				<< "Got empty secondary plot" << std::endl
		);
	}

	// Check that the secondary plotter is compatible with us
	if (not this->isCompatible(sp)) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GBasePlotter::registerSecondaryPlot(): Error!" << std::endl
				<< "Received incompatible secondary plotter" << std::endl
				<< sp->getPlotterName() << " in plotter " << this->getPlotterName() << std::endl
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
bool GBasePlotter::isCompatible(std::shared_ptr < GBasePlotter > other) const {
	return (this->getPlotterName() == other->getPlotterName());
}

/******************************************************************************/
/**
 * calculate a suffix from id and parent ids
 */
std::string GBasePlotter::suffix(bool isSecondary, std::size_t pId) const {
	std::string result;

	if (not isSecondary) {
		result = std::string("_") + Gem::Common::to_string(this->id());
	} else {
		result = std::string("_") + Gem::Common::to_string(pId) + std::string("_") +
					Gem::Common::to_string(this->id());
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
void GBasePlotter::compare(
	const GBasePlotter& cp
	, const Gem::Common::expectation& e
	, const double& limit
) const {
	using namespace Gem::Common;

	// Check that we are dealing with a GBasePlotter reference independent of this object and convert the pointer
	const GBasePlotter *p_load = Gem::Common::g_convert_and_compare(cp, this);

	GToken token("GBasePlotter", e);

	// Compare our parent data ...
	Gem::Common::compare_base<GCommonInterfaceT<GBasePlotter>>(IDENTITY(*this, *p_load), token);

	// ... and then the local data
	compare_t(IDENTITY(m_drawingArguments, p_load->m_drawingArguments), token);
	compare_t(IDENTITY(m_x_axis_label, p_load->m_x_axis_label), token);
	compare_t(IDENTITY(m_y_axis_label, p_load->m_y_axis_label), token);
	compare_t(IDENTITY(m_z_axis_label, p_load->m_z_axis_label), token);
	compare_t(IDENTITY(m_plot_label, p_load->m_plot_label), token);
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
void GBasePlotter::load_(const GBasePlotter* cp) {
	// Check that we are dealing with a GBasePlotter reference independent of this object and convert the pointer
	const GBasePlotter *p_load = Gem::Common::g_convert_and_compare(cp, this);

	// No parent class with loadable data

	// Load local data
	m_drawingArguments = p_load->m_drawingArguments;
	m_x_axis_label     = p_load->m_x_axis_label;
	m_y_axis_label     = p_load->m_y_axis_label;
	m_z_axis_label     = p_load->m_z_axis_label;
	m_plot_label       = p_load->m_plot_label;
	dsMarker_         = p_load->dsMarker_;
	id_               = p_load->id_;

	copyCloneableSmartPointerContainer(p_load->secondaryPlotter_, secondaryPlotter_);
}

/******************************************************************************/
/**
 * Retrieve header settings for this plot (and any sub-plots)
 */
std::string GBasePlotter::headerData(const std::string& indent) const {
	std::ostringstream header_data;

	// Add this plot's data
	header_data
		<< indent << "// Header data for primary plotter" << std::endl
		<< this->headerData_(false, 0, indent);

	// Extract data from the secondary plotters, if any
	std::size_t pos = 0;
	std::vector<std::shared_ptr < GBasePlotter>> ::const_iterator
		cit;
	for (cit = secondaryPlotter_.begin(); cit != secondaryPlotter_.end(); ++cit) {
		// Give the plotters their own id which will act as a child id in this case
		(*cit)->setId(pos);

		// We parent id 0 is reserved for primary plotters
		header_data
			<< indent << "// Header data for secondary plotter " << pos << " of " << this->getPlotterName() << std::endl
			<< (*cit)->headerData_(true, this->id(), indent) << std::endl;

		pos++;
	}

	return header_data.str();
}

/******************************************************************************/
/**
 * Retrieves body / data settings for this plot (and any sub-plots)
 */
std::string GBasePlotter::bodyData(const std::string& indent) const {
	std::ostringstream body_data;

	// Add this plot's data
	body_data
		<< indent << "// Body data for primary plotter" << std::endl
		<< this->bodyData_(false, 0, indent);

	// Extract data from the secondary plotters, if any
	std::size_t pos = 0;
	for (auto plotter_ptr: secondaryPlotter_) { // std::shared_ptr may be copied
		body_data
			<< indent << "// Body data for secondary plotter " << pos << " of " << this->getPlotterName() << std::endl
			<< plotter_ptr->bodyData_(true, this->id(), indent) << std::endl;

		pos++;
	}

	return body_data.str();
}

/******************************************************************************/
/**
 * Retrieves footer / drawing settings for this plot (and any sub-plots)
 */
std::string GBasePlotter::footerData(const std::string& indent) const {
	std::ostringstream footer_data;

	// Add this plot's data
	footer_data
		<< indent << "// Footer data for primary plotter" << std::endl
		<< this->footerData_(false, 0, indent);

	// Extract data from the secondary plotters, if any
	std::size_t pos = 0;
	std::vector<std::shared_ptr<GBasePlotter>>::const_iterator cit;
	for (cit = secondaryPlotter_.begin(); cit != secondaryPlotter_.end(); ++cit) {
		footer_data
			<< indent << "// Footer data for secondary plotter " << pos << " of " << this->getPlotterName() << std::endl
			<< (*cit)->footerData_(true, this->id(), indent) << std::endl;

		pos++;
	}

	return footer_data.str();
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * The default constructor. Member variables are initialized in the class body.
 */
GGraph2D::GGraph2D()
{ /* nothing */ }

/******************************************************************************/
/**
 * A copy constructor
 */
GGraph2D::GGraph2D(const GGraph2D &cp)
	: GDataCollector2T<double, double>(cp)
	  , pM_(cp.pM_)
	  , drawArrows_(cp.drawArrows_)
{ /* nothing */ }

/******************************************************************************/
/**
 * Adds arrows to the plots between consecutive points. Note that setting this
 * value to true will force "SCATTER" mode
 *
 * @param dA The desired value of the drawArrows_ variable
 */
void GGraph2D::setDrawArrows(bool dA) {
	drawArrows_ = dA;
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
void GGraph2D::setPlotMode(graphPlotMode pM) {
	pM_ = pM;
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
void GGraph2D::compare(
	const GBasePlotter& cp
	, const Gem::Common::expectation& e
	, const double& limit
) const {
	using namespace Gem::Common;

	// Check that we are dealing with a GGraph2D reference independent of this object and convert the pointer
	const GGraph2D *p_load = Gem::Common::g_convert_and_compare(cp, this);

	GToken token("GGraph2D", e);

	// Compare our parent data ...
	Gem::Common::compare_base<GDataCollector2T<double, double>>(IDENTITY(*this, *p_load), token);

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
std::string GGraph2D::headerData_(
	bool isSecondary
	, std::size_t pId
	, const std::string& indent
) const {
	std::ostringstream header_data;

	// Set up suitable arrays for the header
	std::string baseName = suffix(isSecondary, pId);
	std::string arrayBaseName = "array_" + baseName;

	std::string xArrayName = "x_" + arrayBaseName;
	std::string yArrayName = "y_" + arrayBaseName;

	std::string comment;
	if (dsMarker_ != "") {
		comment = "// " + dsMarker_;
	}

	header_data
		<< indent << "double " << xArrayName << "[" << Gem::Common::to_string(m_data.size()) << "];" << (comment != "" ? comment : "") << std::endl
		<< indent << "double " << yArrayName << "[" << Gem::Common::to_string(m_data.size()) << "];" << std::endl
		<< std::endl;

	return header_data.str();
}

/******************************************************************************/
/**
 * Retrieves the actual data sets
 */
std::string GGraph2D::bodyData_(
	bool isSecondary
	, std::size_t pId
	, const std::string& indent
) const {
	std::ostringstream body_data;

	// Set up suitable arrays for the header
	std::string baseName = suffix(isSecondary, pId);
	std::string arrayBaseName = "array_" + baseName;

	std::string xArrayName = "x_" + arrayBaseName;
	std::string yArrayName = "y_" + arrayBaseName;

	std::string comment;
	if (dsMarker_ != "") {
		body_data << "// " + dsMarker_ << std::endl;
	}

	// Fill data from the tuples into the arrays
	std::vector<std::tuple<double, double>>::const_iterator it;
	std::size_t posCounter = 0;

	for (it = m_data.begin(); it != m_data.end(); ++it) {
		body_data << indent << xArrayName << "[" << posCounter << "] = " << std::get<0>(*it) << ";" << "\t" << yArrayName << "[" << posCounter << "] = " << std::get<1>(*it) << ";" << std::endl;

		posCounter++;
	}
	body_data << std::endl;

	return body_data.str();
}

/******************************************************************************/
/**
 * Retrieves specific draw commands for this plot
 */
std::string GGraph2D::footerData_(
	bool isSecondary
	, std::size_t pId
	, const std::string& indent
) const {
	std::ostringstream footer_data;

	// Set up suitable arrays for the header
	std::string baseName = suffix(isSecondary, pId);
	std::string arrayBaseName = "array_" + baseName;

	std::string xArrayName = "x_" + arrayBaseName;
	std::string yArrayName = "y_" + arrayBaseName;

	std::string graphName = std::string("graph") + baseName;

	std::string comment;
	if (dsMarker_ != "") {
		footer_data << "// " + dsMarker_ << std::endl;
	}

	// Retrieve the current drawing arguments
	std::string dA = this->drawingArguments(isSecondary);

	// Fill the data in our tuple-vector into a ROOT TGraph object
	footer_data
		<< indent << "TGraph *" << graphName << " = new TGraph(" << m_data.size() << ", " << xArrayName << ", " << yArrayName << ");" << std::endl
		<< indent << graphName << "->GetXaxis()->SetTitle(\"" << xAxisLabel() << "\");" << std::endl
		<< indent << graphName << "->GetYaxis()->SetTitle(\"" << yAxisLabel() << "\");" << std::endl;

	if (m_plot_label != "") {
		footer_data
			<< indent << graphName << "->SetTitle(\"" << m_plot_label << "\");" << std::endl;
	} else {
		footer_data
			<< indent << graphName << "->SetTitle(\" \");" << std::endl;
	}

	footer_data
		<< indent << graphName << "->Draw(\"" << dA << "\");" << std::endl
		<< std::endl;

	if (drawArrows_ && m_data.size() >= 2) {
		std::vector<std::tuple<double, double>>::const_iterator it;
		std::size_t posCounter = 0;

		double x1 = std::get<0>(*m_data.begin());
		double y1 = std::get<1>(*m_data.begin());
		double x2, y2;

		for (it = m_data.begin() + 1; it != m_data.end(); ++it) {
			x2 = std::get<0>(*it);
			y2 = std::get<1>(*it);

			footer_data
				<< indent << "TArrow * ta_" << graphName << "_" << posCounter << " = new TArrow(" << x1 << ", " << y1 << "," << x2 << ", " << y2 << ", " << 0.05 << ", \"|>\");" << std::endl
				<< indent << "ta_" << graphName << "_" << posCounter << "->SetArrowSize(0.01);" << std::endl
				<< indent << "ta_" << graphName << "_" << posCounter << "->Draw();" << std::endl;

			x1 = x2;
			y1 = y2;

			posCounter++;
		}
		footer_data
			<< std::endl;
	}
	footer_data << std::endl;

	return footer_data.str();
}


/******************************************************************************/
/**
 * Retrieve the current drawing arguments
 */
std::string GGraph2D::drawingArguments(bool isSecondary) const {
	std::string dA = "";

	if (this->m_drawingArguments != "") {
		dA = this->m_drawingArguments;
	} else {
		if (graphPlotMode::SCATTER == pM_ || true == drawArrows_) {
			dA = "P";
		} else {
			dA = "PL";
		}

		if (isSecondary) {
			dA = dA + ",same";
		} else {
			dA = "A" + dA;
		}
	}

	return dA;
}

/******************************************************************************/
/**
 * Creates a deep clone of this object
 */
GBasePlotter* GGraph2D::clone_() const {
	return new GGraph2D(*this);
}

/******************************************************************************/
/**
 * Loads the data of another object
 */
void GGraph2D::load_(const GBasePlotter* cp) {
	// Check that we are dealing with a GGraph2D reference independent of this object and convert the pointer
	const GGraph2D *p_load = Gem::Common::g_convert_and_compare<GBasePlotter, GGraph2D>(cp, this);

	// Load our parent class'es data ...
	GDataCollector2T<double, double>::load_(cp);

	// ... and then our local data
	pM_         = p_load->pM_;
	drawArrows_ = p_load->drawArrows_;
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * The default constructor. Data members are initialized in the class body.
 */
GGraph2ED::GGraph2ED()
{ /* nothing */ }

/******************************************************************************/
/**
 * A copy constructor
 */
GGraph2ED::GGraph2ED(const GGraph2ED &cp)
	: GDataCollector2ET<double, double>(cp)
	  , pM_(cp.pM_)
{ /* nothing */ }

/******************************************************************************/
/**
 * Determines whether a scatter plot or a curve is created
 *
 * @param pM The desired plot mode
 */
void GGraph2ED::setPlotMode(graphPlotMode pM) {
	pM_ = pM;
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
void GGraph2ED::compare(
	const GBasePlotter& cp
	, const Gem::Common::expectation& e
	, const double& limit
) const {
	using namespace Gem::Common;

	// Check that we are dealing with a GBasePlotter reference independent of this object and convert the pointer
	const GGraph2ED *p_load = Gem::Common::g_convert_and_compare(cp, this);

	GToken token("GGraph2ED", e);

	// Compare our parent data ...
	Gem::Common::compare_base<GDataCollector2ET<double, double>>(IDENTITY(*this, *p_load), token);

	// ... and then the local data
	compare_t(IDENTITY(pM_, p_load->pM_), token);

	// React on deviations from the expectation
	token.evaluate();
}

/******************************************************************************/
/**
 * Retrieve specific header settings for this plot
 */
std::string GGraph2ED::headerData_(
	bool isSecondary
	, std::size_t pId
	, const std::string& indent
) const {
	std::ostringstream header_data;

	// Set up suitable arrays for the header
	std::string baseName = suffix(isSecondary, pId);
	std::string arrayBaseName = "array_" + baseName;

	std::string xArrayName = "x_" + arrayBaseName;
	std::string exArrayName = "ex_" + arrayBaseName;
	std::string yArrayName = "y_" + arrayBaseName;
	std::string eyArrayName = "ey_" + arrayBaseName;

	std::string comment;
	if (dsMarker_ != "") {
		comment = "// " + dsMarker_;
	}

	header_data
		<< indent << "double " << xArrayName << "[" << Gem::Common::to_string(m_data.size()) << "];" << comment << std::endl
		<< indent << "double " << exArrayName << "[" << Gem::Common::to_string(m_data.size()) << "];" << std::endl
		<< indent << "double " << yArrayName << "[" << Gem::Common::to_string(m_data.size()) << "];" << std::endl
		<< indent << "double " << eyArrayName << "[" << Gem::Common::to_string(m_data.size()) << "];" << std::endl
		<< std::endl;

	return header_data.str();
}

/******************************************************************************/
/**
 * Retrieves the actual data sets
 */
std::string GGraph2ED::bodyData_(
	bool isSecondary
	, std::size_t pId
	, const std::string& indent
) const {
	std::ostringstream body_data;

	// Set up suitable arrays for the header
	std::string baseName = suffix(isSecondary, pId);
	std::string arrayBaseName = "array_" + baseName;

	std::string xArrayName = "x_" + arrayBaseName;
	std::string exArrayName = "ex_" + arrayBaseName;
	std::string yArrayName = "y_" + arrayBaseName;
	std::string eyArrayName = "ey_" + arrayBaseName;

	std::string comment;
	if (dsMarker_ != "") {
		body_data << "// " + dsMarker_ << std::endl;
	}

	// Fill data from the tuples into the arrays
	std::vector<std::tuple<double, double, double, double>>::const_iterator it;
	std::size_t posCounter = 0;

	for (it = m_data.begin(); it != m_data.end(); ++it) {
		body_data
			<< indent << xArrayName << "[" << posCounter << "] = " << std::get<0>(*it) << ";" << std::endl
			<< indent << exArrayName << "[" << posCounter << "] = " << std::get<1>(*it) << ";" << std::endl
			<< indent << yArrayName << "[" << posCounter << "] = " << std::get<2>(*it) << ";" << std::endl
			<< indent << eyArrayName << "[" << posCounter << "] = " << std::get<3>(*it) << ";" << std::endl;

		posCounter++;
	}
	body_data << std::endl;

	return body_data.str();
}

/******************************************************************************/
/**
 * Retrieves specific draw commands for this plot
 */
std::string GGraph2ED::footerData_(
	bool isSecondary
	, std::size_t pId
	, const std::string& indent
) const {
	std::ostringstream footer_data;

	// Set up suitable arrays for the header
	std::string baseName = suffix(isSecondary, pId);
	std::string arrayBaseName = "array_" + baseName;

	std::string xArrayName = "x_" + arrayBaseName;
	std::string exArrayName = "ex_" + arrayBaseName;
	std::string yArrayName = "y_" + arrayBaseName;
	std::string eyArrayName = "ey_" + arrayBaseName;

	std::string graphName = std::string("graph_") + baseName;

	std::string comment;
	if (dsMarker_ != "") {
		footer_data << "// " + dsMarker_ << std::endl;
	}

	// Check whether custom drawing arguments have been set or whether one
	// of our generic choices has been selected
	std::string dA = this->drawingArguments(isSecondary);

	// Fill the data in our tuple-vector into a ROOT TGraphErrors object
	footer_data
		<< indent << "TGraphErrors *" << graphName << " = new TGraphErrors(" << m_data.size() << ", " << xArrayName << ", " << yArrayName << ", " << exArrayName << " ," << eyArrayName << ");" << std::endl
		<< indent << graphName << "->GetXaxis()->SetTitle(\"" << xAxisLabel() << "\");" << std::endl
		<< indent << graphName << "->GetYaxis()->SetTitle(\"" << yAxisLabel() << "\");" << std::endl;

	if (m_plot_label != "") {
		footer_data
			<< indent << graphName << "->SetTitle(\"" << m_plot_label << "\");" << std::endl;
	} else {
		footer_data
			<< indent << graphName << "->SetTitle(\" \");" << std::endl;
	}

	footer_data
		<< indent << graphName << "->Draw(\"" << dA << "\");" << std::endl
		<< std::endl;

	return footer_data.str();
}

/******************************************************************************/
/**
 * Retrieve the current drawing arguments
 */
std::string GGraph2ED::drawingArguments(bool isSecondary) const {
	std::string dA = "";

	if (this->m_drawingArguments != "") {
		dA = this->m_drawingArguments;
	} else {
		if (graphPlotMode::SCATTER == pM_) {
			dA = "P";
		} else {
			dA = "PL";
		}

		if (isSecondary) {
			dA = dA + ",same";
		} else {
			dA = "A" + dA;
		}
	}

	return dA;
}

/******************************************************************************/
/**
 * Creates a deep clone of this object
 */
GBasePlotter* GGraph2ED::clone_() const {
	return new GGraph2ED(*this);
}

/******************************************************************************/
/**
 * Loads the data of another object
 */
void GGraph2ED::load_(const GBasePlotter* cp) {
	// Check that we are dealing with a GGraph2ED reference independent of this object and convert the pointer
	const GGraph2ED *p_load = Gem::Common::g_convert_and_compare<GBasePlotter, GGraph2ED>(cp, this);

	// Load our parent class'es data ...
	GDataCollector2ET<double, double>::load_(cp);

	// ... and then our local data
	pM_ = p_load->pM_;
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * The default constructor. Data members are initialized in the class body.
 */
GGraph3D::GGraph3D()
{ /* nothing */ }

/******************************************************************************/
/**
 * A copy constructor
 */
GGraph3D::GGraph3D(const GGraph3D &cp)
	: GDataCollector3T<double, double, double>(cp)
	  , drawLines_(cp.drawLines_)
{ /* nothing */ }

/******************************************************************************/
/**
 * Adds lines to the plots between consecutive points.
 *
 * @param dL The desired value of the drawLines_ variable
 */
void GGraph3D::setDrawLines(bool dL) {
	drawLines_ = dL;
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
void GGraph3D::compare(
	const GBasePlotter& cp
	, const Gem::Common::expectation& e
	, const double& limit
) const {
	using namespace Gem::Common;

	// Check that we are dealing with a GGraph3D reference independent of this object and convert the pointer
	const GGraph3D *p_load = Gem::Common::g_convert_and_compare(cp, this);

	GToken token("GGraph3D", e);

	// Compare our parent data ...
	Gem::Common::compare_base<GDataCollector3T<double, double, double>>(IDENTITY(*this, *p_load), token);

	// ... and then the local data
	compare_t(IDENTITY(drawLines_, p_load->drawLines_), token);

	// React on deviations from the expectation
	token.evaluate();
}

/******************************************************************************/
/**
 * Retrieve specific header settings for this plot
 */
std::string GGraph3D::headerData_(
	bool isSecondary
	, std::size_t pId
	, const std::string& indent
) const {
	std::ostringstream header_data;

	// Set up suitable arrays for the header
	std::string baseName = suffix(isSecondary, pId);
	std::string arrayBaseName = "array_" + baseName;

	std::string xArrayName = "x_" + arrayBaseName;
	std::string yArrayName = "y_" + arrayBaseName;
	std::string zArrayName = "z_" + arrayBaseName;

	std::string comment;
	if (dsMarker_ != "") {
		comment = "// " + dsMarker_;
	}

	header_data
		<< indent << "double " << xArrayName << "[" << Gem::Common::to_string(m_data.size()) << "];" << (comment != "" ? comment : "") << std::endl
		<< indent << "double " << yArrayName << "[" << Gem::Common::to_string(m_data.size()) << "];" << std::endl
		<< indent << "double " << zArrayName << "[" << Gem::Common::to_string(m_data.size()) << "];" << std::endl
		<< std::endl;

	return header_data.str();
}

/******************************************************************************/
/**
 * Retrieves the actual data sets
 */
std::string GGraph3D::bodyData_(
	bool isSecondary
	, std::size_t pId
	, const std::string& indent
) const {
	std::ostringstream body_data;

	// Set up suitable arrays for the header
	std::string baseName = suffix(isSecondary, pId);
	std::string arrayBaseName = "array_" + baseName;

	std::string xArrayName = "x_" + arrayBaseName;
	std::string yArrayName = "y_" + arrayBaseName;
	std::string zArrayName = "z_" + arrayBaseName;

	std::string comment;
	if (dsMarker_ != "") {
		body_data << "// " + dsMarker_ << std::endl;
	}

	// Fill data from the tuples into the arrays
	std::vector<std::tuple<double, double, double>>::const_iterator it;
	std::size_t posCounter = 0;

	for (it = m_data.begin(); it != m_data.end(); ++it) {
		body_data
			<< indent << xArrayName << "[" << posCounter << "] = " << std::get<0>(*it) << ";" << "\t"
			<< yArrayName << "[" << posCounter << "] = " << std::get<1>(*it) << ";" << "\t"
			<< zArrayName << "[" << posCounter << "] = " << std::get<2>(*it) << ";" << std::endl;

		posCounter++;
	}
	body_data << std::endl;

	return body_data.str();
}

/******************************************************************************/
/**
 * Retrieves specific draw commands for this plot
 */
std::string GGraph3D::footerData_(
	bool isSecondary
	, std::size_t pId
	, const std::string& indent
) const {
	std::ostringstream footer_data;

	// Set up suitable arrays for the header
	std::string baseName = suffix(isSecondary, pId);
	std::string arrayBaseName = "array_" + baseName;

	std::string xArrayName = "x_" + arrayBaseName;
	std::string yArrayName = "y_" + arrayBaseName;
	std::string zArrayName = "z_" + arrayBaseName;

	std::string graphName = std::string("graph_") + baseName;

	std::string comment;
	if (dsMarker_ != "") {
		footer_data << "// " + dsMarker_ << std::endl;
	}

	// Check whether custom drawing arguments have been set or whether one
	// of our generic choices has been selected
	std::string dA = this->drawingArguments(isSecondary);

	// Fill the data in our tuple-vector into a ROOT TGraph object
	footer_data
		<< indent << "TGraph2D *" << graphName << " = new TGraph2D(" << m_data.size() << ", " << xArrayName << ", " << yArrayName <<  ", " << zArrayName << ");" << std::endl
		<< indent << graphName << "->GetXaxis()->SetTitle(\"" << xAxisLabel() << "\");" << std::endl
		<< indent << graphName << "->GetXaxis()->SetTitleOffset(1.5);" << std::endl
		<< indent << graphName << "->GetYaxis()->SetTitle(\"" << yAxisLabel() << "\");" << std::endl
		<< indent << graphName << "->GetYaxis()->SetTitleOffset(1.5);" << std::endl
		<< indent << graphName << "->GetZaxis()->SetTitle(\"" << zAxisLabel() << "\");" << std::endl
		<< indent << graphName << "->GetZaxis()->SetTitleOffset(1.5);" << std::endl
		<< indent << graphName << "->SetMarkerStyle(20);" << std::endl
		<< indent << graphName << "->SetMarkerSize(1);" << std::endl
		<< indent << graphName << "->SetMarkerColor(2);" << std::endl;

	if (m_plot_label != "") {
		footer_data << indent << graphName << "->SetTitle(\"" << m_plot_label << "\");" << std::endl;
	} else {
		footer_data << indent << graphName << "->SetTitle(\" \");" << std::endl;
	}

	footer_data
		<< indent << graphName << "->Draw(\"" << dA << "\");" << std::endl
		<< std::endl;

	if (drawLines_ && m_data.size() >= 2) {
		std::vector<std::tuple<double, double, double>>::const_iterator it;
		std::size_t posCounter = 0;

		double x, y, z;

		footer_data
			<< indent << "TPolyLine3D *lines_" << graphName << " = new TPolyLine3D(" << m_data.size() << ");" << std::endl
			<< std::endl;

		for (it = m_data.begin() + 1; it != m_data.end(); ++it) {
			x = std::get<0>(*it);
			y = std::get<1>(*it);
			z = std::get<2>(*it);

			footer_data
				<< indent << "lines_" << graphName << "->SetPoint(" << posCounter << ", " << x << ", " << y << ", " << z << ");";

			posCounter++;
		}
		footer_data
			<< std::endl
			<< indent << "lines_" << graphName << "->SetLineWidth(3);" << std::endl
			<< indent << "lines_" << graphName << "->Draw();" << std::endl
			<< std::endl;
	}

	return footer_data.str();
}

/******************************************************************************/
/**
 * Retrieve the current drawing arguments
 */
std::string GGraph3D::drawingArguments(bool isSecondary) const {
	std::string dA = "";

	if (this->m_drawingArguments != "") {
		dA = this->m_drawingArguments;
	} else {
		dA = "P";

		if (isSecondary) {
			dA = dA + ",same";
		}
	}

	return dA;
}

/******************************************************************************/
/**
 * Creates a deep clone of this object
 */
GBasePlotter* GGraph3D::clone_() const {
	return new GGraph3D(*this);
}

/******************************************************************************/
/**
 * Loads the data of another object
 */
void GGraph3D::load_(const GBasePlotter* cp) {
	// Check that we are dealing with a GGraph3D reference independent of this object and convert the pointer
	const GGraph3D *p_load = Gem::Common::g_convert_and_compare<GBasePlotter, GGraph3D>(cp, this);

	// Load our parent class'es data ...
	GDataCollector3T<double, double, double>::load_(cp);

	// ... and then our local data
	drawLines_ = p_load->drawLines_;
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * The default constructor. Data members are initialized in the class body.
 */
GGraph4D::GGraph4D()
{ /* nothing */ }

/******************************************************************************/
/**
 * A copy constructor
 */
GGraph4D::GGraph4D(const GGraph4D &cp)
	: GDataCollector4T<double, double, double, double>(cp)
	  , minMarkerSize_(cp.minMarkerSize_)
	  , maxMarkerSize_(cp.maxMarkerSize_)
	  , smallWLargeMarker_(cp.smallWLargeMarker_)
	  , nBest_(cp.nBest_)
{ /* nothing */ }

/******************************************************************************/
/**
 * Allows to set the minimum marker size
 */
void GGraph4D::setMinMarkerSize(const double &minMarkerSize) {
	if (minMarkerSize < 0.) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GGraph4D::setMinMarkerSize(): Error!" << std::endl
				<< "Received invalid minimum marker size: " << minMarkerSize << std::endl
		);
	}

	minMarkerSize_ = minMarkerSize;
}

/******************************************************************************/
/**
 * Allows to set the maximum marker size
 */
void GGraph4D::setMaxMarkerSize(const double &maxMarkerSize) {
	if (maxMarkerSize < 0. || maxMarkerSize < minMarkerSize_) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GGraph4D::setMinMarkerSize(): Error!" << std::endl
				<< "Received invalid minimum marker size: " << minMarkerSize_ << " " << maxMarkerSize << "." << std::endl
				<< "Always set the lower boundary first." << std::endl
		);
	}

	maxMarkerSize_ = maxMarkerSize;
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
void GGraph4D::setNBest(const std::size_t &nBest) {
	nBest_ = nBest;
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
void GGraph4D::compare(
	const GBasePlotter& cp
	, const Gem::Common::expectation& e
	, const double& limit
) const {
	using namespace Gem::Common;

	// Check that we are dealing with a GGraph3D reference independent of this object and convert the pointer
	const GGraph4D *p_load = Gem::Common::g_convert_and_compare(cp, this);

	GToken token("GGraph4D", e);

	// Compare our parent data ...
	Gem::Common::compare_base<GDataCollector4T<double, double, double, double>>(IDENTITY(*this, *p_load), token);

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
std::string GGraph4D::headerData_(
	bool isSecondary
	, std::size_t pId
	, const std::string& indent
) const {
	std::ostringstream header_data;

	// nothing

	return header_data.str();
}

/******************************************************************************/
/**
 * Retrieves the actual data sets
 */
std::string GGraph4D::bodyData_(
	bool isSecondary
	, std::size_t pId
	, const std::string& indent
) const {
	std::ostringstream body_data;

	// nothing

	return body_data.str();
}

/******************************************************************************/
/**
 * Retrieves specific draw commands for this plot
 */
std::string GGraph4D::footerData_(
	bool isSecondary
	, std::size_t pId
	, const std::string& indent
) const {
	std::vector<std::tuple<double, double, double, double>> localData = m_data;

	std::string baseName = suffix(isSecondary, pId);

	// Sort the data, so we can select the nBest_ best more easily
	if (smallWLargeMarker_) {
		std::sort(
			localData.begin(), localData.end(), [](
				std::tuple<double, double, double, double> a, std::tuple<double, double, double, double> b
			) -> bool {
				return (std::get<3>(a) < std::get<3>(b));
			}
		);
	} else {
		std::sort(
			localData.begin(), localData.end(), [](
				std::tuple<double, double, double, double> a, std::tuple<double, double, double, double> b
			) -> bool {
				return (std::get<3>(a) > std::get<3>(b));
			}
		);
	}

	std::ostringstream footer_data;

	// Find out about the minimum and maximum values of the data vector
	std::tuple<double, double, double, double, double, double, double, double> minMax = Gem::Common::getMinMax(
		localData);

	// Set up TView object for our 3D data, spanning the minimum and maximum values
	footer_data
		<< indent << "TH3F *fr = new TH3F(\"fr\",\"fr\","
		<< "10, " << std::get<0>(minMax) << ", " << std::get<1>(minMax) << ", "
		<< "10, " << std::get<2>(minMax) << ", " << std::get<3>(minMax) << ", "
		<< "10, " << std::get<4>(minMax) << ", " << std::get<5>(minMax) << ");" << std::endl
		<< indent << "fr->SetTitle(\" \");" << std::endl
		<< indent << "fr->GetXaxis()->SetTitle(\"" << xAxisLabel() << "\");" << std::endl
		<< indent << "fr->GetXaxis()->SetTitleOffset(1.6);" << std::endl
		<< indent << "fr->GetYaxis()->SetTitle(\"" << yAxisLabel() << "\");" << std::endl
		<< indent << "fr->GetYaxis()->SetTitleOffset(1.6);" << std::endl
		<< indent << "fr->GetZaxis()->SetTitle(\"" << zAxisLabel() << "\");" << std::endl
		<< indent << "fr->GetZaxis()->SetTitleOffset(1.6);" << std::endl
		<< std::endl
		<< indent << "fr->Draw();" << std::endl;

	double wMin = std::get<6>(minMax);
	double wMax = std::get<7>(minMax);

	// Fill data from the tuples into the arrays
	double wRange = wMax - wMin;
	std::size_t pos = 0;
	std::vector<std::tuple<double, double, double, double>>::const_iterator it;
	for (it = localData.begin(); it != localData.end(); ++it) {
		std::string polyMarkerName =
			std::string("pm3d_") + baseName + std::string("_") + Gem::Common::to_string(pos);

		// create a TPolyMarker3D for a single data point
		footer_data
			<< indent << "TPolyMarker3D *" << polyMarkerName << " = new TPolyMarker3D(1);" << std::endl;

		double x = std::get<0>(*it);
		double y = std::get<1>(*it);
		double z = std::get<2>(*it);
		double w = std::get<3>(*it);

		// Translate the fourth component into a marker size. By default,
		// smaller values will yield the largest value
		double markerSize = 0.;
		if (0 == pos) {
			markerSize = 2 * maxMarkerSize_;
		} else {
			if (smallWLargeMarker_) {
				markerSize = minMarkerSize_ + (maxMarkerSize_ - minMarkerSize_) * pow((1. - (w - wMin) / wRange), 8.);
			} else {
				markerSize = minMarkerSize_ + (maxMarkerSize_ - minMarkerSize_) * pow(((w - wMin) / wRange), 8);
			}
		}

		footer_data
			<< indent << polyMarkerName << "->SetPoint(" << pos << ", " << x << ", " << y << ", " << z << "); // w = " << w << std::endl
			<< indent << polyMarkerName << "->SetMarkerSize(" << markerSize << ");" << std::endl
			<< indent << polyMarkerName << "->SetMarkerColor(" << (0 == pos ? 4 : 2) << ");" << std::endl
			<< indent << polyMarkerName << "->SetMarkerStyle(8);" << std::endl
			<< indent << polyMarkerName << "->Draw();" << std::endl
			<< std::endl;

		pos++;

		if (nBest_ && pos >= nBest_) {
			break;
		}
	}

	footer_data << std::endl;

	return footer_data.str();
}

/******************************************************************************/
/**
 * Retrieve the current drawing arguments
 */
std::string GGraph4D::drawingArguments(bool isSecondary) const {
	std::string dA = "";

	// nothing

	return dA;
}

/******************************************************************************/
/**
 * Creates a deep clone of this object
 */
GBasePlotter* GGraph4D::clone_() const {
	return new GGraph4D(*this);
}

/******************************************************************************/
/**
 * Loads the data of another object
 */
void GGraph4D::load_(const GBasePlotter* cp) {
	// Check that we are dealing with a GGraph4D reference independent of this object and convert the pointer
	const GGraph4D *p_load = Gem::Common::g_convert_and_compare<GBasePlotter, GGraph4D>(cp, this);

	// Load our parent class'es data ...
	GDataCollector4T<double, double, double, double>::load_(cp);

	// ... and then our local data
	minMarkerSize_     = p_load->minMarkerSize_;
	maxMarkerSize_     = p_load->maxMarkerSize_;
	smallWLargeMarker_ = p_load->smallWLargeMarker_;
	nBest_             = p_load->nBest_;
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * The default constructor -- intentionally private as it is only needed for (de-)serialization
 */
GHistogram1D::GHistogram1D(){ /* nothing */ }

/******************************************************************************/
/**
 * Initialization with number of bins and automatic range detection
 */
GHistogram1D::GHistogram1D(
	const std::size_t &nBinsX
)
	: nBinsX_(nBinsX)
	  , minX_(0)
	  , maxX_(minX_)
{ /* nothing */ }

/******************************************************************************/
/**
 * Initialization with a range in the form of a tuple
 */
GHistogram1D::GHistogram1D(
	const std::size_t &nBinsX
	, const double &minX
	, const double &maxX
)
	: nBinsX_(nBinsX)
	  , minX_(minX)
	  , maxX_(maxX)
{ /* nothing */ }

/******************************************************************************/
/**
 * Initialization with a range in the form of a tuple
 */
GHistogram1D::GHistogram1D(
	const std::size_t &nBinsX
	, const std::tuple<double, double> &rangeX
)
	: nBinsX_(nBinsX)
	  , minX_(std::get<0>(rangeX))
	  , maxX_(std::get<1>(rangeX))
{ /* nothing */ }

/******************************************************************************/
/**
 * A copy constructor
 *
 * @param cp a copy of another GHistogram1D object
 */
GHistogram1D::GHistogram1D(const GHistogram1D &cp)
	: GDataCollector1T<double>(cp)
	  , nBinsX_(cp.nBinsX_)
	  , minX_(cp.minX_)
	  , maxX_(cp.maxX_)
{ /* nothing */ }

/******************************************************************************/
/**
 * Retrieve specific header settings for this plot
 */
std::string GHistogram1D::headerData_(
	bool isSecondary
	, std::size_t pId
	, const std::string& indent
) const {
	std::ostringstream header_data;

	std::string comment;
	if (dsMarker_ != "") {
		comment = "// " + dsMarker_;
	}

	std::string histName = "histD" + suffix(isSecondary, pId);

	if(minX_ != maxX_) {
		header_data
			<< indent << "TH1D *" << histName << " = new TH1D(\"" << histName << "\", \"" << histName << "\"," << nBinsX_
			<< ", " <<
			minX_ << ", " << maxX_ << ");" << (comment != "" ? comment : "") << std::endl
			<< std::endl;
	} else { // automatic range detection
		std::tuple<double,double> minmax = this->getMinMaxElements();
		header_data
			<< indent << "TH1D *" << histName << " = new TH1D(\"" << histName << "\", \"" << histName << "\"," << nBinsX_
			<< ", " <<
			std::get<0>(minmax) << ", " << std::get<1>(minmax) << ");" << (comment != "" ? comment : "") << std::endl
			<< std::endl;

	}

	return header_data.str();
}

/******************************************************************************/
/**
 * Retrieves the actual data sets
 */
std::string GHistogram1D::bodyData_(
	bool isSecondary
	, std::size_t pId
	, const std::string& indent
) const {
	std::ostringstream body_data;

	std::string comment;
	if (dsMarker_ != "") {
		comment = "// " + dsMarker_;
	} else {
		comment = "";
	}

	std::string histName = "histD" + suffix(isSecondary, pId);

	std::vector<double>::const_iterator it;
	std::size_t posCounter = 0;
	for (it = m_data.begin(); it != m_data.end(); ++it) {
		body_data
			<< indent << histName << "->Fill(" << std::showpoint << *it << ");" << (posCounter == 0 ? comment : ("")) << std::endl;
		posCounter++;
	}
	body_data << std::endl;

	return body_data.str();
}

/******************************************************************************/
/**
 * Retrieves specific draw commands for this plot
 */
std::string GHistogram1D::footerData_(
	bool isSecondary
	, std::size_t pId
	, const std::string& indent
) const {
	std::ostringstream footer_data;

	std::string histName = "histD" + suffix(isSecondary, pId);

	if (m_plot_label != "") {
		footer_data << indent << histName << "->SetTitle(\"" << m_plot_label << "\");" << std::endl;
	} else {
		footer_data << indent << histName << "->SetTitle(\" \");" << std::endl;
	}

	std::string comment;
	if (dsMarker_ != "") {
		footer_data << "// " + dsMarker_ << std::endl;
	}

	// Check whether custom drawing arguments have been set
	std::string dA = this->drawingArguments(isSecondary);

	footer_data
		<< indent << histName << "->GetXaxis()->SetTitle(\"" << xAxisLabel() << "\");" << std::endl
		<< indent << histName << "->GetYaxis()->SetTitle(\"" << yAxisLabel() << "\");" << std::endl
		<< indent << histName << "->Draw(\"" << dA << "\");" << std::endl
		<< std::endl;

	return footer_data.str();
}

/******************************************************************************/
/**
 * Retrieve the current drawing arguments
 */
std::string GHistogram1D::drawingArguments(bool isSecondary) const {
	std::string dA = "";

	if (m_drawingArguments != "") {
		dA = m_drawingArguments;
	} else {
		if (isSecondary) {
			if ("" == dA) {
				dA = "same";
			} else {
				dA = dA + ",same";
			}
		}
	}

	return dA;
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
void GHistogram1D::compare(
	const GBasePlotter& cp
	, const Gem::Common::expectation& e
	, const double& limit
) const {
	using namespace Gem::Common;

	// Check that we are dealing with a GBasePlotter reference independent of this object and convert the pointer
	const GHistogram1D *p_load = Gem::Common::g_convert_and_compare(cp, this);

	GToken token("GHistogram1D", e);

	// Compare our parent data ...
	Gem::Common::compare_base<GDataCollector1T<double>>(IDENTITY(*this, *p_load), token);

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
GBasePlotter* GHistogram1D::clone_() const {
	return new GHistogram1D(*this);
}

/******************************************************************************/
/**
 * Loads the data of another object
 */
void GHistogram1D::load_(const GBasePlotter* cp) {
	// Check that we are dealing with a GHistogram1D reference independent of this object and convert the pointer
	const GHistogram1D *p_load = Gem::Common::g_convert_and_compare<GBasePlotter, GHistogram1D>(cp, this);

	// Load our parent class'es data ...
	GDataCollector1T<double>::load_(cp);

	// ... and then our local data
	nBinsX_ = p_load->nBinsX_;
	minX_     = p_load->minX_;
	maxX_     = p_load->maxX_;
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * The default constructor -- intentionally private as it is only needed for (de-)serialization
 */
GHistogram1I::GHistogram1I()
{ /* nothing */ }

/******************************************************************************/
/**
 * The standard constructor
 */
GHistogram1I::GHistogram1I(
	const std::size_t &nBinsX
	, const double &minX
	, const double &maxX
)
	: GDataCollector1T<std::int32_t>()
	  , nBinsX_(nBinsX)
	  , minX_(minX)
	  , maxX_(maxX)
{ /* nothing */ }

/******************************************************************************/
/**
 * Initialization with a range in the form of a tuple
 */
GHistogram1I::GHistogram1I(
	const std::size_t &nBinsX
	, const std::tuple<double, double> &rangeX
)
	: GDataCollector1T<std::int32_t>()
	  , nBinsX_(nBinsX)
	  , minX_(std::get<0>(rangeX))
	  , maxX_(std::get<1>(rangeX))
{ /* nothing */ }

/******************************************************************************/
/**
 * A copy constructor
 *
 * @param cp a copy of another GHistogram1I object
 */
GHistogram1I::GHistogram1I(const GHistogram1I &cp)
	: GDataCollector1T<std::int32_t>(cp)
	  , nBinsX_(cp.nBinsX_)
	  , minX_(cp.minX_)
	  , maxX_(cp.maxX_)
{ /* nothing */ }

/******************************************************************************/
/**
 * Retrieve specific header settings for this plot
 */
std::string GHistogram1I::headerData_(
	bool isSecondary
	, std::size_t pId
	, const std::string& indent
) const {
	std::ostringstream header_data;

	std::string comment;
	if (dsMarker_ != "") {
		comment = "// " + dsMarker_;
	}

	std::string histName = "histI" + suffix(isSecondary, pId);

	header_data
		<< indent << "TH1I *" << histName << " = new TH1I(\"" << histName << "\", \"" << histName << "\"," << nBinsX_ << ", " <<
		minX_ << ", " << maxX_ << ");" << (comment != "" ? comment : "") << std::endl
		<< std::endl;

	return header_data.str();
}

/******************************************************************************/
/**
 * Retrieves the actual data sets
 */
std::string GHistogram1I::bodyData_(
	bool isSecondary
	, std::size_t pId
	, const std::string& indent
) const {
	std::ostringstream body_data;

	std::string comment;
	if (dsMarker_ != "") {
		comment = "// " + dsMarker_;
	} else {
		comment = "";
	}

	std::string histName = "histI" + suffix(isSecondary, pId);

	std::vector<std::int32_t>::const_iterator it;
	std::size_t posCounter = 0;
	for (it = m_data.begin(); it != m_data.end(); ++it) {
		body_data
			<< indent << histName << "->Fill(" << *it << ");" << (posCounter == 0 ? comment : ("")) << std::endl;
		posCounter++;
	}

	body_data << std::endl;

	return body_data.str();
}

/******************************************************************************/
/**
 * Retrieves specific draw commands for this plot
 */
std::string GHistogram1I::footerData_(
	bool isSecondary
	, std::size_t pId
	, const std::string& indent
) const {
	std::ostringstream footer_data;

	std::string histName = "histI" + suffix(isSecondary, pId);

	if (m_plot_label != "") {
		footer_data
			<< indent << histName << "->SetTitle(\"" << m_plot_label << "\");" << std::endl;
	} else {
		footer_data
			<< indent << histName << "->SetTitle(\" \");" << std::endl;
	}

	std::string comment;
	if (dsMarker_ != "") {
		footer_data << "// " + dsMarker_ << std::endl;
	}

	// Check whether custom drawing arguments have been set
	std::string dA = this->drawingArguments(isSecondary);

	footer_data
		<< indent << histName << "->GetXaxis()->SetTitle(\"" << xAxisLabel() << "\");" << std::endl
		<< indent << histName << "->GetYaxis()->SetTitle(\"" << yAxisLabel() << "\");" << std::endl
		<< indent << histName << "->Draw(\"" << dA << "\");" << std::endl
		<< std::endl;

	return footer_data.str();
}

/******************************************************************************/
/**
 * Retrieve the current drawing arguments
 */
std::string GHistogram1I::drawingArguments(bool isSecondary) const {
	std::string dA = "";

	if (m_drawingArguments != "") {
		dA = m_drawingArguments;
	} else {
		if (isSecondary) {
			if ("" == dA) {
				dA = "same";
			} else {
				dA = dA + ",same";
			}
		}
	}

	return dA;
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
void GHistogram1I::compare(
	const GBasePlotter& cp
	, const Gem::Common::expectation& e
	, const double& limit
) const {
	using namespace Gem::Common;

	// Check that we are dealing with a GBasePlotter reference independent of this object and convert the pointer
	const GHistogram1I *p_load = Gem::Common::g_convert_and_compare(cp, this);

	GToken token("GHistogram1I", e);

	// Compare our parent data ...
	Gem::Common::compare_base<GDataCollector1T<std::int32_t>>(IDENTITY(*this, *p_load), token);

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
GBasePlotter* GHistogram1I::clone_() const {
	return new GHistogram1I(*this);
}

/******************************************************************************/
/**
 * Loads the data of another object
 */
void GHistogram1I::load_(const GBasePlotter* cp) {
	// Check that we are dealing with a GHistogram1I reference independent of this object and convert the pointer
	const GHistogram1I *p_load = Gem::Common::g_convert_and_compare<GBasePlotter, GHistogram1I>(cp, this);

	// Load our parent class'es data ...
	GDataCollector1T<std::int32_t>::load_(cp);

	// ... and then our local data
	nBinsX_   = p_load->nBinsX_;
	minX_     = p_load->minX_;
	maxX_     = p_load->maxX_;
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * The default constructor -- intentionally private, as it is only needed for (de-)serialization
 */
GHistogram2D::GHistogram2D() { /* nothing */ }

/******************************************************************************/
/**
 * The standard constructor
 */
GHistogram2D::GHistogram2D(
	const std::size_t &nBinsX
	, const std::size_t &nBinsY
	, const double &minX
	, const double &maxX
	, const double &minY
	, const double &maxY
)
	: nBinsX_(nBinsX)
	  , nBinsY_(nBinsY)
	  , minX_(minX)
	  , maxX_(maxX)
	  , minY_(minY)
	  , maxY_(maxY)
	  , dropt_(tddropt::TDEMPTY)
{ /* nothing */ }

/******************************************************************************/
/**
 * Initialization with ranges
 */
GHistogram2D::GHistogram2D(
	const std::size_t &nBinsX
	, const std::size_t &nBinsY
	, const std::tuple<double, double> &rangeX
	, const std::tuple<double, double> &rangeY
)
	: nBinsX_(nBinsX)
	  , nBinsY_(nBinsY)
	  , minX_(std::get<0>(rangeX))
	  , maxX_(std::get<1>(rangeX))
	  , minY_(std::get<0>(rangeY))
	  , maxY_(std::get<1>(rangeY))
	  , dropt_(tddropt::TDEMPTY)
{ /* nothing */ }

/******************************************************************************/
/**
 * Initialization with automatic range detection
 */
GHistogram2D::GHistogram2D(
	const std::size_t &nBinsX
	, const std::size_t &nBinsY
)
	: nBinsX_(nBinsX)
	  , nBinsY_(nBinsY)
	  , minX_(0)
	  , maxX_(minX_)
	  , minY_(0)
	  , maxY_(minY_)
	  , dropt_(tddropt::TDEMPTY)
{ /* nothing */ }

/******************************************************************************/
/**
 * A copy constructor
 *
 * @param cp a copy of another GHistogram2D object
 */
GHistogram2D::GHistogram2D(const GHistogram2D &cp)
	: GDataCollector2T<double, double>(cp)
	  , nBinsX_(cp.nBinsX_)
	  , nBinsY_(cp.nBinsY_)
	  , minX_(cp.minX_)
	  , maxX_(cp.maxX_)
	  , minY_(cp.minY_)
	  , maxY_(cp.maxY_)
	  , dropt_(cp.dropt_)
{ /* nothing */ }

/******************************************************************************/
/**
 * Retrieve specific header settings for this plot
 */
std::string GHistogram2D::headerData_(
	bool isSecondary
	, std::size_t pId
	, const std::string& indent
) const {
	std::ostringstream header_data;

	std::string comment;
	if (dsMarker_ != "") {
		comment = "// " + dsMarker_;
	}

	std::string histName = "hist2D" + suffix(isSecondary, pId);

	if(minX_!=maxX_ && minY_!=maxY_) {
		header_data
			<< indent << "TH2D *"
			<< histName
			<< " = new TH2D(\""
			<< histName
			<< "\", \""
			<< histName
			<< "\","
			<< nBinsX_
			<< ", "
			<< minX_
			<< ", "
			<< maxX_
			<< ","
			<< nBinsY_
			<< ", "
			<< minY_
			<< ", "
			<< maxY_
			<< ");"
			<< (comment != "" ? comment : "")
			<< std::endl
			<< std::endl;
	} else { // // automatic range detection
		std::tuple<double,double,double,double> minmax = this->getMinMaxElements();

		header_data
			<< indent << "TH2D *"
			<< histName
			<< " = new TH2D(\""
			<< histName
			<< "\", \""
			<< histName
			<< "\","
			<< nBinsX_
			<< ", "
			<< std::get<0>(minmax)
			<< ", "
			<< std::get<1>(minmax)
			<< ","
			<< nBinsY_
			<< ", "
			<< std::get<2>(minmax)
			<< ", "
			<< std::get<3>(minmax)
			<< ");"
			<< (comment != "" ? comment : "")
			<< std::endl
			<< std::endl;
	}

	return header_data.str();
}

/******************************************************************************/
/**
 * Retrieves the actual data sets
 */
std::string GHistogram2D::bodyData_(
	bool isSecondary
	, std::size_t pId
	, const std::string& indent
) const {
	std::ostringstream body_data;

	std::string comment;
	if (dsMarker_ != "") {
		comment = "// " + dsMarker_;
	} else {
		comment = "";
	}

	std::string histName = "hist2D" + suffix(isSecondary, pId);

	std::vector<std::tuple<double, double>>::const_iterator it;
	std::size_t posCounter = 0;
	for (it = m_data.begin(); it != m_data.end(); ++it) {
		body_data
			<< indent << histName << "->Fill(" << std::showpoint << std::get<0>(*it) << ", " << std::get<1>(*it) << ");" <<
			(posCounter == 0 ? comment : ("")) << std::endl;
		posCounter++;
	}

	body_data << std::endl;

	return body_data.str();
}

/******************************************************************************/
/**
 * Retrieves specific draw commands for this plot
 */
std::string GHistogram2D::footerData_(
	bool isSecondary
	, std::size_t pId
	, const std::string& indent
) const {
	std::ostringstream footer_data;

	std::string histName = "hist2D" + suffix(isSecondary, pId);

	if (m_plot_label != "") {
		footer_data
			<< indent << histName << "->SetTitle(\"" << m_plot_label << "\");" << std::endl;
	} else {
		footer_data
			<< indent << histName << "->SetTitle(\" \");" << std::endl;
	}

	std::string comment;
	if (dsMarker_ != "") {
		footer_data << "// " + dsMarker_ << std::endl;
	}

	// Check whether custom drawing arguments have been set
	std::string dA = this->drawingArguments(isSecondary);

	footer_data
		<< indent << histName << "->GetXaxis()->SetTitle(\"" << xAxisLabel() << "\");" << std::endl
		<< indent << histName << "->GetYaxis()->SetTitle(\"" << yAxisLabel() << "\");" << std::endl
		<< indent << histName << "->Draw(\"" << dA << "\");" << std::endl
		<< std::endl;

	return footer_data.str();
}

/******************************************************************************/
/**
 * Retrieve the current drawing arguments
 */
std::string GHistogram2D::drawingArguments(bool isSecondary) const {
	std::string dA = "";

	if (m_drawingArguments != "") {
		dA = m_drawingArguments;
	} else {
		switch (dropt_) {
			case tddropt::TDEMPTY:
				dA = "";
				break;

			case tddropt::SURFONE:
				dA = "SURF1";
				break;

			case tddropt::SURFTWOZ:
				dA = "SURF2Z";
				break;

			case tddropt::SURFTHREE:
				dA = "SURF3";
				break;

			case tddropt::SURFFOUR:
				dA = "SURF4";
				break;

			case tddropt::CONTZ:
				dA = "CONTZ";
				break;

			case tddropt::CONTONE:
				dA = "CONT1";
				break;

			case tddropt::CONTTWO:
				dA = "CONT2";
				break;

			case tddropt::CONTTHREE:
				dA = "CONT3";
				break;

			case tddropt::TEXT:
				dA = "TEXT";
				break;

			case tddropt::SCAT:
				dA = "SCAT";
				break;

			case tddropt::BOX:
				dA = "BOX";
				break;

			case tddropt::ARR:
				dA = "ARR";
				break;

			case tddropt::COLZ:
				dA = "COLZ";
				break;

			case tddropt::LEGO:
				dA = "LEGO";
				break;

			case tddropt::LEGOONE:
				dA = "LEGO1";
				break;

			case tddropt::SURFONEPOL:
				dA = "SURF1POL";
				break;

			case tddropt::SURFONECYL:
				dA = "SURF1CYL";
				break;
		}

		if (isSecondary) {
			dA = dA + ",same";
		}
	}

	return dA;
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
void GHistogram2D::compare(
	const GBasePlotter& cp
	, const Gem::Common::expectation& e
	, const double& limit
) const {
	using namespace Gem::Common;

	// Check that we are dealing with a GBasePlotter reference independent of this object and convert the pointer
	const GHistogram2D *p_load = Gem::Common::g_convert_and_compare(cp, this);

	GToken token("GHistogram2D", e);

	// Compare our parent data ...
	Gem::Common::compare_base<GDataCollector2T<double, double>>(IDENTITY(*this, *p_load), token);

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
GBasePlotter* GHistogram2D::clone_() const {
	return new GHistogram2D(*this);
}

/******************************************************************************/
/**
 * Loads the data of another object
 */
void GHistogram2D::load_(const GBasePlotter* cp) {
	// Check that we are dealing with a GHistogram2D reference independent of this object and convert the pointer
	const GHistogram2D *p_load = Gem::Common::g_convert_and_compare<GBasePlotter, GHistogram2D>(cp, this);

	// Load our parent class'es data ...
	GDataCollector2T<double, double>::load_(cp);

	// ... and then our local data
	nBinsX_   = p_load->nBinsX_;
	nBinsY_   = p_load->nBinsY_;
	minX_     = p_load->minX_;
	maxX_     = p_load->maxX_;
	minY_     = p_load->minY_;
	maxY_     = p_load->maxY_;
	dropt_    = p_load->dropt_;
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * The default constructor. Intentionally private, as it is only needed for (de-)serialization
 */
GFunctionPlotter1D::GFunctionPlotter1D() { /* nothing */ }

/******************************************************************************/
/**
 * The standard constructor. Some member variables may be initialized in the
 * class body.
 *
 * @param fD A descriptor for the function to be plotted
 */
GFunctionPlotter1D::GFunctionPlotter1D(
	const std::string &fD
	, const std::tuple<double, double> &xExtremes
)
	: functionDescription_(fD)
	  , xExtremes_(xExtremes)
{ /* nothing */ }

/******************************************************************************/
/**
 * A copy constructor
 *
 * @param cp A reference to another GFunctionPlotter1D object
 */
GFunctionPlotter1D::GFunctionPlotter1D(const GFunctionPlotter1D &cp)
	: GBasePlotter(cp)
	  , functionDescription_(cp.functionDescription_)
	  , xExtremes_(cp.xExtremes_)
	  , nSamplesX_(cp.nSamplesX_)
{ /* nothing */ }

/******************************************************************************/
/**
 * Allows to set the number of sampling points of the function on the x-axis
 *
 * @param nSamplesX The number of sampling points of the function on the x-axis
 */
void GFunctionPlotter1D::setNSamplesX(std::size_t nSamplesX) {
	nSamplesX_ = nSamplesX;
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
void GFunctionPlotter1D::compare(
	const GBasePlotter& cp
	, const Gem::Common::expectation& e
	, const double& limit
) const {
	using namespace Gem::Common;

	// Check that we are dealing with a GGraph3D reference independent of this object and convert the pointer
	const GFunctionPlotter1D *p_load = Gem::Common::g_convert_and_compare(cp, this);

	GToken token("GFunctionPlotter1D", e);

	// Compare our parent data ...
	Gem::Common::compare_base<GBasePlotter>(IDENTITY(*this, *p_load), token);

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
	bool isSecondary
	, std::size_t pId
	, const std::string& indent
) const {
	// Check the extreme values for consistency
	if (std::get<0>(xExtremes_) >= std::get<1>(xExtremes_)) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GFunctionPlotter1D::headerData_(): Error!" << std::endl
				<< "lower boundary >= upper boundary: " << std::get<0>(xExtremes_) << " / " << std::get<1>(xExtremes_) <<
				std::endl
		);
	}

	std::ostringstream result;

	std::string comment;
	if (dsMarker_ != "") {
		comment = "// " + dsMarker_;
	}

	std::string functionName = "func1D" + suffix(isSecondary, pId);
	result << indent << "TF1 *" << functionName << " = new TF1(\"" << functionName << "\", \"" << functionDescription_ <<
			 "\"," << std::get<0>(xExtremes_) << ", " << std::get<1>(xExtremes_) << ");" << (comment != "" ? comment : "") <<
			 std::endl;

	return result.str();
}

/******************************************************************************/
/**
 * Retrieves the actual data sets
 *
 * @return The code to be added to the plot's data section for this function
 */
std::string GFunctionPlotter1D::bodyData_(
	bool isSecondary
	, std::size_t pId
	, const std::string& indent
) const {
	// No data needs to be added for a function plotter
	return std::string();
}

/******************************************************************************/
/**
 * Retrieves specific draw commands for this plot
 *
 * @return The draw command to be added to the plot's data for this function
 */
std::string GFunctionPlotter1D::footerData_(
	bool isSecondary
	, std::size_t pId
	, const std::string& indent
) const {
	std::ostringstream footer_data;

	std::string comment;
	if (dsMarker_ != "") {
		comment = "// " + dsMarker_;
	}

	std::string functionName = "func1D" + suffix(isSecondary, pId);
	footer_data
		<< indent << functionName << "->GetXaxis()->SetTitle(\"" << xAxisLabel() << "\");" << std::endl
		<< indent << functionName << "->GetYaxis()->SetTitle(\"" << yAxisLabel() << "\");" << std::endl
		<< indent << functionName << "->SetNpx(" << nSamplesX_ << ");" << std::endl;

	if (m_plot_label != "") {
		footer_data
			<< indent << functionName << "->SetTitle(\"" << m_plot_label << "\");" << std::endl;
	} else {
		footer_data
			<< indent << functionName << "->SetTitle(\" \");" << std::endl;
	}

	std::string dA = this->drawingArguments(isSecondary);

	footer_data
		<< indent << functionName << "->Draw(" << dA << ");" << (comment != "" ? comment : "") << std::endl
		<< std::endl;

	return footer_data.str();
}

/******************************************************************************/
/**
 * Retrieve the current drawing arguments
 */
std::string GFunctionPlotter1D::drawingArguments(bool isSecondary) const {
	std::string dA = "";

	if (this->m_drawingArguments != "") {
		dA = this->m_drawingArguments;
	}

	if (isSecondary) {
		if ("" == dA) {
			dA = "same";
		} else {
			dA = dA + ",same";
		}
	}

	return dA;
}

/******************************************************************************/
/**
 * Creates a deep clone of this object
 */
GBasePlotter* GFunctionPlotter1D::clone_() const {
	return new GFunctionPlotter1D(*this);
}

/******************************************************************************/
/**
 * Loads the data of another object
 */
void GFunctionPlotter1D::load_(const GBasePlotter* cp) {
	// Check that we are dealing with a GFunctionPlotter1D reference independent of this object and convert the pointer
	const GFunctionPlotter1D *p_load = Gem::Common::g_convert_and_compare<GBasePlotter, GFunctionPlotter1D>(cp, this);

	// Load our parent class'es data ...
	GBasePlotter::load_(cp);

	// ... and then our local data
	functionDescription_ = p_load->functionDescription_;
	xExtremes_           = p_load->xExtremes_;
	nSamplesX_           = p_load->nSamplesX_;
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * The default constructor -- intentionally private, as it is only needed for (de-)serialization
 */
GFunctionPlotter2D::GFunctionPlotter2D() { /* nothing */ }

/******************************************************************************/
/**
 * The standard constructor
 *
 * @param fD A descriptor for the function to be plotted
 */
GFunctionPlotter2D::GFunctionPlotter2D(
	const std::string &fD
	, const std::tuple<double, double> &xExtremes
	, const std::tuple<double, double> &yExtremes
)
	: functionDescription_(fD)
	  , xExtremes_(xExtremes)
	  , yExtremes_(yExtremes)
{ /* nothing */ }

/******************************************************************************/
/**
 * A copy constructor
 *
 * @param cp A reference to another GFunctionPlotter2D object
 */
GFunctionPlotter2D::GFunctionPlotter2D(const GFunctionPlotter2D &cp)
	: GBasePlotter(cp)
	  , functionDescription_(cp.functionDescription_)
	  , xExtremes_(cp.xExtremes_)
	  , yExtremes_(cp.yExtremes_)
	  , nSamplesX_(cp.nSamplesX_)
	  , nSamplesY_(cp.nSamplesY_)
{ /* nothing */ }

/******************************************************************************/
/**
 * Allows to set the number of sampling points of the function on the x-axis
 *
 * @param nSamplesX The number of sampling points of the function on the x-axis
 */
void GFunctionPlotter2D::setNSamplesX(std::size_t nSamplesX) {
	nSamplesX_ = nSamplesX;
}

/******************************************************************************/
/**
 * Allows to set the number of sampling points of the function on the y-axis
 *
 * @param nSamplesY The number of sampling points of the function on the y-axis
 */
void GFunctionPlotter2D::setNSamplesY(std::size_t nSamplesY) {
	nSamplesY_ = nSamplesY;
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
void GFunctionPlotter2D::compare(
	const GBasePlotter& cp
	, const Gem::Common::expectation& e
	, const double& limit
) const {
	using namespace Gem::Common;

	// Check that we are dealing with a GGraph3D reference independent of this object and convert the pointer
	const GFunctionPlotter2D *p_load = Gem::Common::g_convert_and_compare(cp, this);

	GToken token("GFunctionPlotter2D", e);

	// Compare our parent data ...
	Gem::Common::compare_base<GBasePlotter>(IDENTITY(*this, *p_load), token);

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
	bool isSecondary
	, std::size_t pId
	, const std::string& indent
) const {
	// Check the extreme values for consistency
	if (std::get<0>(xExtremes_) >= std::get<1>(xExtremes_)) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GFunctionPlotter2D::headerData_(): Error!" << std::endl
				<< "lower boundary(x) >= upper boundary(x): " << std::get<0>(xExtremes_) << " / " <<
				std::get<1>(xExtremes_) << std::endl
		);
	}

	if (std::get<0>(yExtremes_) >= std::get<1>(yExtremes_)) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GFunctionPlotter2D::headerData_(): Error!" << std::endl
				<< "lower boundary(y) >= upper boundary(y): " << std::get<0>(yExtremes_) << " / " <<
				std::get<1>(yExtremes_) << std::endl
		);
	}

	std::ostringstream result;

	std::string comment;
	if (dsMarker_ != "") {
		comment = "// " + dsMarker_;
	}

	std::string functionName = "func2D" + suffix(isSecondary, pId);
	result
		<< indent << "TF2 *"
		<< functionName
		<< " = new TF2(\""
		<< functionName << "\", \""
		<< functionDescription_
		<< "\","
		<< std::get<0>(xExtremes_)
		<< ", "
		<< std::get<1>(xExtremes_)
		<< ", "
		<< std::get<0>(yExtremes_)
		<< ", "
		<< std::get<1>(yExtremes_)
		<< ");"
		<< (comment != "" ? comment : "")
		<< std::endl;

	return result.str();
}

/******************************************************************************/
/**
 * Retrieves the actual data sets
 *
 * @return The code to be added to the plot's data section for this function
 */
std::string GFunctionPlotter2D::bodyData_(
	bool isSecondary
	, std::size_t pId
	, const std::string& indent
) const {
	// No data needs to be added for a function plotter
	return std::string();
}

/******************************************************************************/
/**
 * Retrieves specific draw commands for this plot
 *
 * @return The draw command to be added to the plot's data for this function
 */
std::string GFunctionPlotter2D::footerData_(
	bool isSecondary
	, std::size_t pId
	, const std::string& indent
) const {
	std::ostringstream footer_data;

	std::string comment;
	if (dsMarker_ != "") {
		comment = "// " + dsMarker_;
	}

	std::string functionName = "func2D" + suffix(isSecondary, pId);
	footer_data
		<< indent << functionName << "->GetXaxis()->SetTitle(\"" << xAxisLabel() << "\");" << std::endl
		<< indent << functionName << "->GetYaxis()->SetTitle(\"" << yAxisLabel() << "\");" << std::endl
		<< indent << functionName << "->GetZaxis()->SetTitle(\"" << zAxisLabel() << "\");" << std::endl
		<< indent << functionName << "->SetNpx(" << nSamplesX_ << ");" << std::endl
		<< indent << functionName << "->SetNpy(" << nSamplesY_ << ");" << std::endl;

	if (m_plot_label != "") {
		footer_data
			<< indent << functionName << "->SetTitle(\"" << m_plot_label << "\");" << std::endl;
	} else {
		footer_data
			<< indent << functionName << "->SetTitle(\" \");" << std::endl;
	}

	std::string dA = this->drawingArguments(isSecondary);

	footer_data
		<< indent << functionName << "->Draw(" << dA << ");" << (comment != "" ? comment : "") << std::endl
		<< std::endl;

	return footer_data.str();
}

/******************************************************************************/
/**
 * Retrieve the current drawing arguments
 */
std::string GFunctionPlotter2D::drawingArguments(bool isSecondary) const {
	std::string dA = "";

	if (isSecondary) {
		if ("" == dA) {
			dA = "same";
		} else {
			dA = dA + ",same";
		}
	}

	return dA;
}

/******************************************************************************/
/**
 * Creates a deep clone of this object
 */
GBasePlotter* GFunctionPlotter2D::clone_() const {
	return new GFunctionPlotter2D(*this);
}

/******************************************************************************/
/**
 * Loads the data of another object
 */
void GFunctionPlotter2D::load_(const GBasePlotter* cp) {
	// Check that we are dealing with a GFunctionPlotter2D reference independent of this object and convert the pointer
	const GFunctionPlotter2D *p_load = Gem::Common::g_convert_and_compare<GBasePlotter, GFunctionPlotter2D>(cp, this);

	// Load our parent class'es data ...
	GBasePlotter::load_(cp);

	// ... and then our local data
	functionDescription_ = p_load->functionDescription_;
	xExtremes_           = p_load->xExtremes_;
	yExtremes_           = p_load->yExtremes_;
	nSamplesX_           = p_load->nSamplesX_;
	nSamplesY_           = p_load->nSamplesY_;
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
	const std::string &canvasLabel
	, const std::size_t &c_x_div
	, const std::size_t &c_y_div
)
	: c_x_div_(c_x_div)
	  , c_y_div_(c_y_div)
	  , canvasLabel_(canvasLabel)
{ /* nothing */ }

/******************************************************************************/
/**
 * The default constructor -- only needed for (de-)serialization
 */
GPlotDesigner::GPlotDesigner()
{ /* nothing */ }

/******************************************************************************/
/**
 * The copy constructor
 */
GPlotDesigner::GPlotDesigner(const GPlotDesigner& cp)
	: c_x_div_(cp.c_x_div_)
	  , c_y_div_(cp.c_y_div_)
	  , c_x_dim_(cp.c_x_dim_)
	  , c_y_dim_(cp.c_y_dim_)
	  , canvasLabel_(cp.canvasLabel_)
	  , addPrintCommand_(cp.addPrintCommand_)
	  , nIndentionSpaces_(cp.nIndentionSpaces_)
{
	// Copy any secondary plotters over
	Gem::Common::copyCloneableSmartPointerContainer<GBasePlotter>(cp.plotters_, plotters_);
}

/******************************************************************************/
/**
 * Writes the plot to a file
 *
 * @param fileName The name of the file to which the data can be written
 */
void GPlotDesigner::writeToFile(const boost::filesystem::path &fileName) {
	boost::filesystem::ofstream result(fileName);
	result << plot(fileName);
	result.close();
}

/******************************************************************************/
/*
 * Emits the overall plot
 */
std::string GPlotDesigner::plot(const boost::filesystem::path &plotName) const {
	std::ostringstream result;
	std::size_t maxPlots = c_x_div_ * c_y_div_;

	if (plotters_.size() > maxPlots) {
		glogger
			<< "In GPlotDesigner::plot() (Canvas label = \"" << this->getCanvasLabel() << "\":" << std::endl
			<< "Warning! Found more plots than pads (" << plotters_.size() << " vs. " << maxPlots << ")" << std::endl
			<< "Some of the plots will be ignored" << std::endl
			<< GWARNING;
	}

	result
		<< "{" << std::endl
		<< staticHeader(indent())
		<< std::endl;

	// Plot all body sections up to the maximum allowed number
	result
		<< indent() << "//===================  Header Section ====================" << std::endl
		<< std::endl;

	// Plot all headers up to the maximum allowed number
	std::size_t nPlots = 0;
	std::vector<std::shared_ptr < GBasePlotter>> ::const_iterator
		it;
	for (it = plotters_.begin(); it != plotters_.end(); ++it) {
		if (nPlots++ < maxPlots) {
			result
				<< (*it)->headerData(indent()) << std::endl;
		}
	}

	// Plot all body sections up to the maximum allowed number
	result
		<< indent() << "//===================  Data Section ======================" << std::endl
		<< std::endl;

	nPlots = 0;
	for (it = plotters_.begin(); it != plotters_.end(); ++it) {
		if (nPlots++ < maxPlots) {
			result
				<< (*it)->bodyData(indent()) << std::endl;
		}
	}

	// Plot all footer data up to the maximum allowed number
	result
		<< indent() << "//===================  Plot Section ======================" << std::endl
		<< std::endl;

	nPlots = 0;
	for (it = plotters_.begin(); it != plotters_.end(); ++it) {
		if (nPlots < maxPlots) {
			result
				<< indent() << "graphPad->cd(" << nPlots + 1 << ");" << std::endl /* cd starts at 1 */
				<< (*it)->footerData(indent()) << std::endl;

			nPlots++;
		}
	}

	result
		<< indent() << "graphPad->cd();" << std::endl
		<< indent() << "cc->cd();" << std::endl;

	// Check if we are supposed to output a png file
	if (addPrintCommand_ && plotName.string() != "empty" && not (plotName.string()).empty()) {
		std::string plotName_local = plotName.string(); // Make sure there are no white spaces
		boost::trim(plotName_local);
		result
			<< std::endl
			<< indent() << "// Print out the data of this file to a png file" << std::endl
			<< indent() << "cc->Print(\"" << plotName_local << ".png\");" << std::endl;
	}

	result
		<< "}" << std::endl;

	return result.str();
}

/******************************************************************************/
/**
 * A default header for a ROOT file
 */
std::string GPlotDesigner::staticHeader(const std::string& indent) const {
	std::ostringstream result;

	result
		<< indent << "gROOT->Reset();" << std::endl
		<< indent << "gStyle->SetCanvasColor(0);" << std::endl
		<< indent << "gStyle->SetStatBorderSize(1);" << std::endl
		<< indent << "gStyle->SetOptStat(0);" << std::endl
		<< std::endl
		<< indent << "TCanvas *cc = new TCanvas(\"cc\", \"cc\",0,0," << c_x_dim_ << "," << c_y_dim_ << ");" << std::endl
		<< std::endl
		<< indent << "TPaveLabel* canvasTitle = new TPaveLabel(0.2,0.95,0.8,0.99, \"" << canvasLabel_ << "\");" << std::endl
		<< indent << "canvasTitle->Draw();" << std::endl
		<< std::endl
		<< indent << "TPad* graphPad = new TPad(\"Graphs\", \"Graphs\", 0.01, 0.01, 0.99, 0.94);" << std::endl
		<< indent << "graphPad->Draw();" << std::endl
		<< indent << "graphPad->Divide(" << c_x_div_ << "," << c_y_div_ << ");" << std::endl
		<< std::endl;

	return result.str();
}

/******************************************************************************/
/**
 * Allows to add a new plotter object
 *
 * @param plotter_ptr A pointer to a plotter
 */
void GPlotDesigner::registerPlotter(std::shared_ptr < GBasePlotter > plotter_ptr) {
	if (plotter_ptr) {
		plotter_ptr->setId(plotters_.size());
		plotters_.push_back(plotter_ptr);
	} else {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "GPlotDesigner::registerPlotter(): Error!" << std::endl
				<< "Got empty plotter" << std::endl
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
	const std::uint32_t &c_x_dim, const std::uint32_t &c_y_dim
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
void GPlotDesigner::setCanvasDimensions(
	const std::tuple<std::uint32_t, std::uint32_t> &c_dim
) {
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
void GPlotDesigner::setCanvasLabel(const std::string &canvasLabel) {
	canvasLabel_ = canvasLabel;
}

/******************************************************************************/
/**
 * Allows to retrieve the canvas label
 */
std::string GPlotDesigner::getCanvasLabel() const {
	return canvasLabel_;
}

/******************************************************************************/
/**
 * Allows to add a "Print" command to the end of the script so that picture files are created
 */
void GPlotDesigner::setAddPrintCommand(bool addPrintCommand) {
	addPrintCommand_ = addPrintCommand;
}

/******************************************************************************/
/**
 * Allows to retrieve the current value of the addPrintCommand_ variable
 */
bool GPlotDesigner::getAddPrintCommand() const {
	return addPrintCommand_;
}

/******************************************************************************/
/**
 * Allows to set the number of spaces used for indention
 */
void GPlotDesigner::setNIndentionSpaces(
	const std::size_t& nIndentionSpaces
) {
	nIndentionSpaces_ = nIndentionSpaces;
}

/******************************************************************************/
/**
 * Allows to retrieve the number spaces used for indention
 */
std::size_t GPlotDesigner::getNIndentionSpaces() const {
	return nIndentionSpaces_;
}

/******************************************************************************/
/**
 * Returns the current number of indention spaces as a string
 */
std::string GPlotDesigner::indent() const {
	return std::string(nIndentionSpaces_, ' ');
}

/******************************************************************************/
/**
 * Resets the plotters
 */
void GPlotDesigner::resetPlotters() {
	plotters_.clear();
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
void GPlotDesigner::compare(
	const GPlotDesigner& cp
	, const Gem::Common::expectation& e
	, const double& limit
) const {
	using namespace Gem::Common;

	// Check that we are dealing with a GPlotDesigner reference independent of this object and convert the pointer
	const GPlotDesigner *p_load = Gem::Common::g_convert_and_compare(cp, this);

	GToken token("GPlotDesigner", e);

	// Compare our parent data ...
	Gem::Common::compare_base<GCommonInterfaceT<GPlotDesigner>>(IDENTITY(*this, *p_load), token);

	// ... and then the local data
	compare_t(IDENTITY(plotters_, p_load->plotters_), token);
	compare_t(IDENTITY(c_x_div_, p_load->c_x_div_), token);
	compare_t(IDENTITY(c_y_div_, p_load->c_y_div_), token);
	compare_t(IDENTITY(c_x_dim_, p_load->c_x_dim_), token);
	compare_t(IDENTITY(c_y_dim_, p_load->c_y_dim_), token);
	compare_t(IDENTITY(canvasLabel_, p_load->canvasLabel_), token);
	compare_t(IDENTITY(addPrintCommand_, p_load->addPrintCommand_), token);
	compare_t(IDENTITY(nIndentionSpaces_, p_load->nIndentionSpaces_), token);

	// React on deviations from the expectation
	token.evaluate();
}

/******************************************************************************/
/**
 * Creates a deep clone of this object
 */
GPlotDesigner* GPlotDesigner::clone_() const {
	return new GPlotDesigner(*this);
}

/******************************************************************************/
/**
 * Loads the data of another object
 */
void GPlotDesigner::load_(const GPlotDesigner* cp) {
	// Check that we are dealing with a GBasePlotter reference independent of this object and convert the pointer
	const GPlotDesigner *p_load = Gem::Common::g_convert_and_compare(cp, this);

	// No "loadable" parent class

	// Load local data
	copyCloneableSmartPointerContainer(p_load->plotters_, plotters_);
	c_x_div_           = p_load->c_x_div_;
	c_y_div_           = p_load->c_y_div_;
	c_x_dim_           = p_load->c_x_dim_;
	c_y_dim_           = p_load->c_y_dim_;
	canvasLabel_       = p_load->canvasLabel_;
	addPrintCommand_   = p_load->addPrintCommand_;
	nIndentionSpaces_  = p_load->nIndentionSpaces_;
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Common */
} /* namespace Gem */
