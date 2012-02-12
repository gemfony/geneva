/**
 * @file GPlotDesigner.cpp
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt)
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) com
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
 * http://www.gemfony.com .
 */

#include "common/GPlotDesigner.hpp"

namespace Gem {
namespace Common {

/*****************************************************************************/
///////////////////////////////////////////////////////////////////////////////
/*****************************************************************************/
/**
 * The default constructor
 */
GBasePlotter::GBasePlotter()
	: drawingArguments_("APL")
	, x_axis_label_("x")
	, y_axis_label_("y")
	, z_axis_label_("z")
	, plot_label_("")
	, dsMarker_("")
	, id_(0)
{ /* nothing */ }

/*****************************************************************************/
/**
 * A copy constructor
 *
 * @param cp A copy of another GBasePlotter object
 */
GBasePlotter::GBasePlotter(const GBasePlotter& cp)
	: drawingArguments_(cp.drawingArguments_)
	, x_axis_label_(cp.x_axis_label_)
	, y_axis_label_(cp.y_axis_label_)
	, z_axis_label_(cp.z_axis_label_)
	, plot_label_(cp.plot_label_)
	, dsMarker_(cp.dsMarker_)
	, id_(cp.id_)
{ /* nothing */ }

/*****************************************************************************/
/**
 * The destructor
 */
GBasePlotter::~GBasePlotter()
{ /* nothing */ }

/*****************************************************************************/
/**
 * The assignment operator
 */
void GBasePlotter::operator=(const GBasePlotter& cp) {
	drawingArguments_ = cp.drawingArguments_;
	x_axis_label_ = cp.x_axis_label_;
	y_axis_label_ = cp.y_axis_label_;
	z_axis_label_ = cp.z_axis_label_;
	plot_label_   = cp.plot_label_;
	dsMarker_     = cp.dsMarker_;
	id_           = cp.id_;
}

/*****************************************************************************/
/**
 * Allows to set the drawing arguments for this plot
 *
 * @param drawingArguments The drawing arguments for this plot
 */
void GBasePlotter::setDrawingArguments(std::string drawingArguments) {
	drawingArguments_ = drawingArguments;
}

/*****************************************************************************/
/**
 * Retrieve the current drawing arguments
 */
std::string GBasePlotter::drawingArguments() const {
	return drawingArguments_;
}

/*****************************************************************************/
/**
 * Sets the label for the x-axis
 * */
void GBasePlotter::setXAxisLabel(std::string x_axis_label) {
	x_axis_label_ = x_axis_label;
}

/*****************************************************************************/
/**
 * Retrieve the x-axis label
 */
std::string GBasePlotter::xAxisLabel() const {
	return x_axis_label_;
}

/*****************************************************************************/
/**
 * Sets the label for the y-axis
 */
void GBasePlotter::setYAxisLabel(std::string y_axis_label) {
	y_axis_label_ = y_axis_label;
}

/*****************************************************************************/
/**
 * Retrieve the y-axis label
 */
std::string GBasePlotter::yAxisLabel() const {
	return y_axis_label_;
}

/*****************************************************************************/
/**
 * Sets the label for the z-axis
 */
void GBasePlotter::setZAxisLabel(std::string z_axis_label) {
	z_axis_label_ = z_axis_label;
}

/*****************************************************************************/
/**
 * Retrieve the z-axis label
 */
std::string GBasePlotter::zAxisLabel() const {
	return z_axis_label_;
}

/*****************************************************************************/
/**
 * Allows to assign a label to the entire plot
 *
 * @param pL A label to be assigned to the entire plot
 */
void GBasePlotter::setPlotLabel(std::string pL) {
	plot_label_ = pL;
}

/*****************************************************************************/
/**
 * Allows to retrieve the plot label
 *
 * @return The label that has been assigned to the plot
 */
std::string GBasePlotter::plotLabel() const {
	return plot_label_;
}

/*****************************************************************************/
/**
 * Allows to assign a marker to data structures in the output file
 *
 * @param A marker that has been assigned to the output data structures
 */
void GBasePlotter::setDataStructureMarker(std::string dsMarker) {
	dsMarker_ = dsMarker;
}

/*****************************************************************************/
/**
 * Allows to retrieve the data structure marker
 *
 * @return The marker that has been assigned to the output data structures
 */
std::string GBasePlotter::dsMarker() const {
	return dsMarker_;
}

/*****************************************************************************/
/**
 * Allows to retrieve the id of this object
 */
std::size_t GBasePlotter::id() const {
	return id_;
}


/*****************************************************************************/
/**
 * Sets the id of the object
 *
 * @param id The id to be assigned to this object
 */
void GBasePlotter::setId(const std::size_t& id) {
	id_ = id;
}

/*****************************************************************************/
///////////////////////////////////////////////////////////////////////////////
/*****************************************************************************/
/**
 * The default constructor
 */
GGraph2D::GGraph2D()
	: pM_(DEFPLOTMODE)
{ /* nothing */ }

/*****************************************************************************/
/**
 * A copy constructor
 */
GGraph2D::GGraph2D(const GGraph2D& cp)
	: GDataCollector2T<double,double>(cp)
	, pM_(cp.pM_)
{ /* nothing */ }

/*****************************************************************************/
/**
 * The destructor
 */
GGraph2D::~GGraph2D()
{ /* nothing */ }

/*****************************************************************************/
/**
 * The assignment operator
 */
GGraph2D & GGraph2D::operator=(const GGraph2D& cp) {
	// Copy our parent class'es data
	GDataCollector2T<double,double>::operator=(cp);

	// and then our own
	pM_ = cp.pM_;

	return *this;
}

/*****************************************************************************/
/**
 * Determines whether a scatter plot or a curve is created
 *
 * @param pM The desired plot mode
 */
void GGraph2D::setPlotMode(graphPlotMode pM) {
	pM_ = pM;
}

/*****************************************************************************/
/**
 * Allows to retrieve the current plotting mode
 *
 * @return The current plot mode
 */
graphPlotMode GGraph2D::getPlotMode() const {
	return pM_;
}

/*****************************************************************************/
/**
 * Retrieve specific header settings for this plot
 */
std::string GGraph2D::headerData() const {
	std::ostringstream header_data;

	// Set up suitable arrays for the header
	std::string arrayBaseName =
			"array_" + boost::lexical_cast<std::string>(id());

	std::string xArrayName = "x_" + arrayBaseName;
	std::string yArrayName = "y_" + arrayBaseName;

	std::string comment;
	if(dsMarker_ != "") {
		comment = "// " + dsMarker_;
	}

	header_data
		<< "  double " << xArrayName << "[" << boost::lexical_cast<std::string>(data_.size()) << "];" << (comment!=""?comment:"") << std::endl
		<< "  double " << yArrayName << "[" << boost::lexical_cast<std::string>(data_.size()) << "];" << (comment!=""?comment:"") << std::endl
		<< std::endl;

	return header_data.str();
}

/*****************************************************************************/
/**
 * Retrieves the actual data sets
 */
std::string GGraph2D::bodyData() const {
	std::ostringstream body_data;

	// Set up suitable arrays for the header
	std::string arrayBaseName =
			"array_" + boost::lexical_cast<std::string>(id());

	std::string xArrayName = "x_" + arrayBaseName;
	std::string yArrayName = "y_" + arrayBaseName;

	std::string comment;
	if(dsMarker_ != "") {
		body_data << "// " + dsMarker_ << std::endl;
	}

	// Fill data from the tuples into the arrays
	std::vector<boost::tuple<double, double> >::const_iterator it;
	std::size_t posCounter = 0;

	for(it=data_.begin(); it!=data_.end(); ++it) {
		body_data
			<< "  " << xArrayName << "[" << posCounter << "] = " << it->get<0>() << ";" << std::endl
			<< "  " << yArrayName << "[" << posCounter << "] = " << it->get<1>() << ";" << std::endl;

		posCounter++;
	}

	body_data << std::endl;

	return body_data.str();
}

/*****************************************************************************/
/**
 * Retrieves specific draw commands for this plot
 */
std::string GGraph2D::footerData() const {
	std::ostringstream footer_data;

	// Set up suitable arrays for the header
	std::string arrayBaseName =
			"array_" + boost::lexical_cast<std::string>(id());

	std::string xArrayName = "x_" + arrayBaseName;
	std::string yArrayName = "y_" + arrayBaseName;

	std::string graphName = std::string("graph_") + boost::lexical_cast<std::string>(id());

	std::string comment;
	if(dsMarker_ != "") {
		footer_data << "// " + dsMarker_ << std::endl;
	}

	// Fill the data in our tuple-vector into a ROOT TGraph object
	footer_data
		<< "  TGraph *" << graphName << " = new TGraph(" << data_.size() <<", " << xArrayName << ", " << yArrayName << ");" << std::endl
		<< "  " << graphName << "->GetXaxis()->SetTitle(\"" << xAxisLabel() << "\");" << std::endl
		<< "  " << graphName << "->GetYaxis()->SetTitle(\"" << yAxisLabel() << "\");" << std::endl
		<< "  " << graphName << "->Draw(\""<< ((pM_==SCATTER)?("AP"):("APL")) << "\");" << std::endl
		<< std::endl;

	return footer_data.str();
}

/*****************************************************************************/
///////////////////////////////////////////////////////////////////////////////
/*****************************************************************************/
/**
 * The standard constructor
 *
 * @param c_x_div The number of plots in x-direction
 * @param c_y_div The number of plots in y-direction
 */
GPlotDesigner::GPlotDesigner(
	const std::size_t& c_x_div, const std::size_t& c_y_div
)
	: c_x_div_(c_x_div)
	, c_y_div_(c_y_div)
	, c_x_dim_(DEFCXDIM)
	, c_y_dim_(DEFCYDIM)
	, canvasLabel_("GPlotDesigner")
{ /* nothing */ }

/*****************************************************************************/
/*
 * Emits the overall plot
 */
std::string GPlotDesigner::plot() const {
	std::ostringstream result;
	std::size_t maxPlots = c_x_div_*c_y_div_;

#ifdef DEBUG
	if(plotters_.size() > maxPlots) {
		std::cerr << "In GPlotDesigner::plot(): Warning!" << std::endl
				  << "Found more plots than pads." << std::endl
				  << "Some of the plots will be ignored" << std::endl;
	}
#endif /* DEBUG*/

	result
		<< "{" << std::endl
		<<    staticHeader();

	// Plot all headers up to the maximum allowed number
	std::size_t nPlots = 0;
	std::vector<boost::shared_ptr<GBasePlotter> >::const_iterator it;
	for(it=plotters_.begin(); it!=plotters_.end(); ++it) {
		if(nPlots++ < maxPlots) {
			result << (*it)->headerData()
				   << std::endl;
		}
	}

	// Plot all body sections up to the maximum allowed number
	result << "  //===================  Data Section ======================" << std::endl
		   << std::endl;

	nPlots = 0;
	for(it=plotters_.begin(); it!=plotters_.end(); ++it) {
		if(nPlots++ < maxPlots) {
			result << (*it)->bodyData()
				   << std::endl;
		}
	}

	// Plot all footer data up to the maximum allowed number
	result << "  //===================  Plot Section ======================" << std::endl
		   << std::endl;

	nPlots = 0;
	for(it=plotters_.begin(); it!=plotters_.end(); ++it) {
		if(nPlots < maxPlots) {
			result << "  graphPad->cd(" << nPlots+1 << ");" << std::endl /* cd starts at 1 */
			       << (*it)->footerData()
				   << std::endl;

			nPlots++;
		}
	}

	result
	    << "  graphPad->cd()" << std::endl
		<< "  cc->cd();" << std::endl
		<< "}" << std::endl;

	return result.str();
}

/*****************************************************************************/
/**
 * A default header for a ROOT file
 */
std::string GPlotDesigner::staticHeader() const {
	std::ostringstream result;

	result
	  << "  gROOT->Reset();" << std::endl
	  << "  gStyle->SetOptTitle(0);" << std::endl
	  << "  TCanvas *cc = new TCanvas(\"cc\", \"cc\",0,0,"<< c_x_dim_ << "," << c_y_dim_ << ");" << std::endl
	  << std::endl
	  << "  TPaveLabel* canvasTitle = new TPaveLabel(0.2,0.95,0.8,0.99, \"" << canvasLabel_ << "\");" << std::endl
	  << "  canvasTitle->Draw();" << std::endl
	  << std::endl
	  << "  TPad* graphPad = new TPad(\"Graphs\", \"Graphs\", 0.01, 0.01, 0.99, 0.94);" << std::endl
	  << "  graphPad->Draw();" << std::endl
	  << "  graphPad->Divide(" << c_x_div_ << "," << c_y_div_ << ");" << std::endl
	  << std::endl;

	return result.str();
}

/*****************************************************************************/
/**
 * Allows to add a new plotter object
 *
 * @param plotter_ptr A pointer to a plotter
 */
void GPlotDesigner::registerPlotter(boost::shared_ptr<GBasePlotter> plotter_ptr) {
	if(plotter_ptr) {
		plotter_ptr->setId(plotters_.size());
		plotters_.push_back(plotter_ptr);
	} else {
		raiseException(
			"GPlotDesigner::registerPlotter(): Error!" << std::endl
			<< "Got empty plotter" << std::endl
		);
	}
}

/*****************************************************************************/
/**
 * Allows to assign a global title to the entire canvas
 *
 * @param canvasLabel A title to be assigned to the canvas
 */
void GPlotDesigner::setCanvasLabel(std::string canvasLabel) {
	canvasLabel_ = canvasLabel;
}

/*****************************************************************************/
/**
 * Set the dimensions of the output canvas
 *
 * @param c_x_dim The x-dimension of the output canvas
 * @param c_x_dim The y-dimension of the output canvas
 */
void GPlotDesigner::setCanvasDimensions(
	const boost::uint32_t& c_x_dim
	, const boost::uint32_t& c_y_dim
) {
	c_x_dim_ = c_x_dim;
	c_y_dim_ = c_y_dim;
}

/*****************************************************************************/
/**
 * Allows to retrieve the canvas dimensions
 *
 * @return A boost::tuple holding the canvas dimensions
 */
boost::tuple<boost::uint32_t, boost::uint32_t> GPlotDesigner::getCanvasDimensions() const {
	return boost::tuple<boost::uint32_t, boost::uint32_t>(c_x_dim_, c_y_dim_);
}

/*****************************************************************************/
///////////////////////////////////////////////////////////////////////////////
/*****************************************************************************/

} /* namespace Common */
} /* namespace Gem */
