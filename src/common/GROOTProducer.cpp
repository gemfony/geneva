/**
 * @file GROOTProducer.hpp
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

#include "common/GROOTProducer.hpp"

namespace Gem {
namespace Common {

/*****************************************************************************/
/**
 * The standard constructor
 */
GROOTProducer::GROOTProducer(
	const boost::uint32_t& c_x_div
	, const boost::uint32_t& c_y_div
)
	: c_x_dim_(DEFCXDIM)
	, c_y_dim_(DEFCYDIM)
	, c_x_div_(c_x_div)
	, c_y_div_(c_y_div)
	, pM_(DEFPLOTMODE)
	, currentSubCanvas_(0)
	, plotComplete_(false)
{ /* nothing */ }

/*****************************************************************************/
/**
 * Set the dimensions of the output canvas
 *
 * @param c_x_dim The x-dimension of the output canvas
 * @param c_x_dim The y-dimension of the output canvas
 */
void GROOTProducer::setCanvasDimensions(
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
boost::tuple<boost::uint32_t, boost::uint32_t> GROOTProducer::getCanvasDimensions() const {
	return boost::tuple<boost::uint32_t, boost::uint32_t>(c_x_dim_, c_y_dim_);
}

/*****************************************************************************/
/**
 * Allows to retrieve the canvas divisions
 */
boost::tuple<boost::uint32_t, boost::uint32_t> GROOTProducer::getCanvasDivision() const {
	return boost::tuple<boost::uint32_t, boost::uint32_t>(c_x_div_, c_y_div_);
}

/*****************************************************************************/
/**
 * Assign a name to the entire canvas
 *
 * @param canvasName A name to be assigned to the entire canvas
 */
void GROOTProducer::setCanvasName(const std::string& canvasName) {
	canvasName_ = canvasName;
}

/*****************************************************************************/
/**
 * Determines whether a scatter plot or a curve is created
 *
 * @param pM The desired plot mode
 */
void GROOTProducer::setPlotMode(const plotMode& pM) {
	pM_ = pM;
}

/*****************************************************************************/
/**
 * Allows to retrieve the current plotting mode
 *
 * @return The current plot mode
 */
plotMode GROOTProducer::getPlotMode() const {
	return pM_;
}

/*****************************************************************************/
/**
 * Marks the current data entry session as complete and switches to the next drawing
 * area. The maximum number of drawing areas is determined by the divisions set in the
 * constructor. There is no way to switch back to a preceding canvas. Instead, all
 * collected data so far is written to the result stream. No switch takes place if
 * we have gone beyond the last canvas.
 *
 * @return A boolean indicating whether the switch was successful
 *
 * @param xAxisName A name to be assigned to the x-axis of the plot
 * @oaram yAxisName A name to be assigned to the y-axis of the plot
 * @param subCanvasName The name of the current sub-canvas
 */
bool GROOTProducer::completeSubCanvas(
	const std::string& xAxisName
	, const std::string& yAxisName
	, const std::string& subCanvasName
){
	if(!measuredData_.empty()) {
		// Set up suitable arrays for the header
		std::string arrayBaseName =
				"array_" + boost::lexical_cast<std::string>(currentSubCanvas_);

		std::string xArrayName = "x_" + arrayBaseName;
		std::string yArrayName = "y_" + arrayBaseName;

		headerData_ << "  double " << xArrayName << "[" << boost::lexical_cast<std::string>(measuredData_.size()) << "];" << std::endl
				<< "  double " << yArrayName << "[" << boost::lexical_cast<std::string>(measuredData_.size()) << "];" << std::endl
				<< std::endl;

		// Fill data from the tuples into the arrays
		std::vector<boost::tuple<double, double> >::iterator it;
		std::size_t posCounter = 0;
		for(it=measuredData_.begin(); it!=measuredData_.end(); ++it) {
			tupleData_
			<< "  " << xArrayName << "[" << posCounter << "] = " << it->get<0>() << ";" << std::endl
			<< "  " << yArrayName << "[" << posCounter << "] = " << it->get<1>() << ";" << std::endl;

			posCounter++;
		}
		tupleData_ << std::endl;

		std::string graphName =
				(subCanvasName == "")?(std::string("graph_") + boost::lexical_cast<std::string>(currentSubCanvas_)):subCanvasName;

		// Fill the data in our tuple-vector into a ROOT TGraph object
		footerData_ << std::endl
				<< "  " << (canvasName_==""?"cc":canvasName_) << "->cd(" << currentSubCanvas_+1 << ");" << std::endl
				<< "  TGraph *" << graphName << " = new TGraph(" << measuredData_.size() <<", " << xArrayName << ", " << yArrayName << ");" << std::endl
				<< "  " << graphName << "->GetXaxis()->SetTitle(\"" << ((xAxisName=="")?("x"):xAxisName) << "\");" << std::endl
				<< "  " << graphName << "->GetYaxis()->SetTitle(\"" << ((yAxisName=="")?("y"):yAxisName) << "\");" << std::endl
				<< "  " << graphName << "->Draw(\""<< ((pM_==SCATTER)?("AP"):("APL")) << "\");" << std::endl
				<< std::endl;

		// Clear the tuple vector for the next iteration
		measuredData_.clear();
	}

	// Switch to the next canvas
	if(++currentSubCanvas_ >= boost::numeric_cast<std::size_t>(c_x_div_*c_y_div_)) {
		plotComplete_ = true;
		return false;
	}

	return true;
}

/*****************************************************************************/
/**
 * Allows to add external data (such as tuple-data or a function plot) to the current
 * sub canvas. No test is done of the data added here -- everything is added verbatim.
 * No data will be added once the canvas is complete. Note that you have to switch the
 * sub-canvas manually, otherwise the external data will end up in the current plot.
 *
 * @param extHeader The command needed to set up the plot in extPlot
 * @param extPlot A ROOT plot to be added to the canvas
 * @param extFooter The command needed to draw the plot in extPlot
 */
void GROOTProducer::addExternalPlot(
		const std::string& extHeader
		, const std::string& extPlot
		, const std::string& extFooter
) {
	if(!plotComplete_) {
		headerData_ << extHeader << std::endl;
		tupleData_  << extPlot << std::endl;
		footerData_ << extFooter << std::endl;
	}
}

/*****************************************************************************/
/**
 * Retrieves the resulting plot as a std::string
 *
 * @return A string with the resulting plot
 */
std::string GROOTProducer::getResult() const {
	std::ostringstream result;

	result
		<< "{" << std::endl
		<<    getStaticHeader()
		<<    headerData_.str()
		<<    tupleData_.str()
		<<    footerData_.str()
		<< std::endl
		<< "  " << (canvasName_==""?"cc":canvasName_) << "->cd();" << std::endl
		<< "}" << std::endl;

	return result.str();
}

/*****************************************************************************/
/**
 * Writes out the final ROOT file
 *
 * @param resultFile The name of the result file
 */
void GROOTProducer::writeResult(const std::string& resultFile) const {
	std::ofstream result(resultFile.c_str());
	result << getResult();
	result.close();
}

/*****************************************************************************/
/**
 * A default header for a ROOT file
 */
std::string GROOTProducer::getStaticHeader() const {
	std::string cn = (canvasName_==""?"cc":canvasName_);

	std::ostringstream result;

	result
	  << "  gROOT->Reset();" << std::endl
	  << "  gStyle->SetOptTitle(0);" << std::endl
	  << "  TCanvas *" << cn << " = new TCanvas(\"" << cn << "\",\"" << cn << "\",0,0,"<< c_x_dim_ << "," << c_y_dim_ << ");" << std::endl
	  << "  " << cn << "->Divide(" << c_x_div_ << "," << c_y_div_ << ");" << std::endl
	  << std::endl;

	return result.str();
}

/*****************************************************************************/

} /* namespace Common */
} /* namespace Gem */
