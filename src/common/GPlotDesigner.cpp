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

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * The default constructor
 */
GBasePlotter::GBasePlotter()
	: drawingArguments_("")
	, x_axis_label_("x")
	, y_axis_label_("y")
	, z_axis_label_("z")
	, plot_label_("")
	, dsMarker_("")
	, id_(0)
{ /* nothing */ }

/******************************************************************************/
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

/******************************************************************************/
/**
 * The destructor
 */
GBasePlotter::~GBasePlotter()
{ /* nothing */ }

/******************************************************************************/
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

/******************************************************************************/
/**
 * Allows to set the drawing arguments for this plot
 *
 * @param drawingArguments The drawing arguments for this plot
 */
void GBasePlotter::setDrawingArguments(std::string drawingArguments) {
	drawingArguments_ = drawingArguments;
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
void GBasePlotter::setPlotLabel(std::string pL) {
	plot_label_ = pL;
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
void GBasePlotter::registerSecondaryPlotter(boost::shared_ptr<GBasePlotter> sp) {
   // Check that the secondary plot isn't empty
   if(!sp) {
      glogger
      << "In GBasePlotter::registerSecondaryPlot(): Error!" << std::endl
      << "Got empty secondary plot" << std::endl
      << GEXCEPTION;
   }

   // Check that the secondary plotter is compatible with us
   if(!this->isCompatible(sp)) {
      glogger
      << "In GBasePlotter::registerSecondaryPlot(): Error!" << std::endl
      << "Received incompatible secondary plotter" << std::endl
      << sp->getPlotterName() << " in plotter " << this->getPlotterName() << std::endl
      << GEXCEPTION;
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
bool GBasePlotter::isCompatible(boost::shared_ptr<GBasePlotter> other) const {
   return (this->getPlotterName() == other->getPlotterName());
}

/******************************************************************************/
/**
 * calculate a suffix from id and parent ids
 */
std::string GBasePlotter::suffix(bool isSecondary, std::size_t pId) const {
   std::string result;

   if(!isSecondary) {
      result = std::string("_") + boost::lexical_cast<std::string>(this->id());
   } else {
      result = std::string("_") + boost::lexical_cast<std::string>(pId) + std::string("_") + boost::lexical_cast<std::string>(this->id());
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
void GBasePlotter::setId(const std::size_t& id) {
	id_ = id;
}

/******************************************************************************/
/**
 * Retrieve specific header settings for this plot
 */
std::string GBasePlotter::headerData_() const {
   std::ostringstream header_data;

   // Add this plot's data
   header_data << this->headerData(false, 0);

   // Extract data from the secondary plotters, if any
   std::size_t pos = 0;
   std::vector<boost::shared_ptr<GBasePlotter> >::const_iterator cit;
   for(cit=secondaryPlotter_.begin(); cit!=secondaryPlotter_.end(); ++cit) {
      // Give the plotters their own id which will act as a child id in this case
      (*cit)->setId(pos);

      // We parent id 0 is reserved for primary plotters
      header_data
      << "  // Header data for secondary plotter " << pos << " of " << this->getPlotterName() << std::endl
      << (*cit)->headerData(true, this->id()) << std::endl;

      pos++;
   }

   return header_data.str();
}

/******************************************************************************/
/**
 * Retrieves the actual data sets
 */
std::string GBasePlotter::bodyData_() const {
   std::ostringstream body_data;

   // Add this plot's data
   body_data << this->bodyData(false, 0);

   // Extract data from the secondary plotters, if any
   std::size_t pos = 0;
   std::vector<boost::shared_ptr<GBasePlotter> >::const_iterator cit;
   for(cit=secondaryPlotter_.begin(); cit!=secondaryPlotter_.end(); ++cit) {
      body_data
      << "  // Body data for secondary plotter " << pos << " of " << this->getPlotterName() << std::endl
      << (*cit)->bodyData(true, this->id()) << std::endl;

      pos++;
   }

   return body_data.str();
}

/******************************************************************************/
/**
 * Retrieves specific draw commands for this plot
 */
std::string GBasePlotter::footerData_() const {
   std::ostringstream footer_data;

   // Add this plot's data
   footer_data << this->footerData(false, 0);

   // Extract data from the secondary plotters, if any
   std::size_t pos = 0;
   std::vector<boost::shared_ptr<GBasePlotter> >::const_iterator cit;
   for(cit=secondaryPlotter_.begin(); cit!=secondaryPlotter_.end(); ++cit) {
      footer_data
      << "  // Footer data for secondary plotter " << pos << " of " << this->getPlotterName() << std::endl
      << (*cit)->footerData(true, this->id()) << std::endl;

      pos++;
   }

   return footer_data.str();
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * Specialization of projectX for <x_type, y_type> = <double, double>, that will return a
 * GHistogram1D object, wrapped into a boost::shared_ptr<GHistogram1D>. In case of a
 * default-constructed range, the function will attempt to determine suitable parameters
 * for the range settings.
 *
 * @param nBins The number of bins of the histogram
 * @param range The minimum and maximum boundaries of the histogram
 */
template<>
boost::shared_ptr<GDataCollector1T<double> >
GDataCollector2T<double, double>::projectX(std::size_t nBinsX, boost::tuple<double, double> rangeX) const {
	boost::tuple<double, double> myRangeX;
	if(rangeX == boost::tuple<double, double>()) {
		// Find out about the minimum and maximum values in the data_ array
		boost::tuple<double,double,double,double> extremes = Gem::Common::getMinMax(this->data_);
		myRangeX = boost::tuple<double,double>(boost::get<0>(extremes), boost::get<1>(extremes));
	} else {
		myRangeX = rangeX;
	}

	// Construct the result object
	boost::shared_ptr<GHistogram1D> result(new GHistogram1D(nBinsX, myRangeX));
	result->setXAxisLabel(this->xAxisLabel());
	result->setYAxisLabel("Number of entries");
	result->setPlotLabel(this->plotLabel() + " / x-projection");

	// Add data to the object
	for(std::size_t i=0; i<data_.size(); i++) {
		(*result) & boost::get<0>(data_.at(i));
	}

	// Return the data
	return result;
}

/******************************************************************************/
/**
 * Specialization of projectY for <x_type, y_type> = <double, double>, that will return a
 * GHistogram1D object, wrapped into a boost::shared_ptr<GHistogram1D>. In case of a
 * default-constructed range, the function will attempt to determine suitable parameters
 * for the range settings.
 *
 * @param nBins The number of bins of the histogram
 * @param range The minimum and maximum boundaries of the histogram
 */
template<>
boost::shared_ptr<GDataCollector1T<double> >
GDataCollector2T<double, double>::projectY(std::size_t nBinsY, boost::tuple<double, double> rangeY) const {
	boost::tuple<double, double> myRangeY;
	if(rangeY == boost::tuple<double, double>()) {
		// Find out about the minimum and maximum values in the data_ array
		boost::tuple<double,double,double,double> extremes = Gem::Common::getMinMax(data_);
		myRangeY = boost::tuple<double,double>(boost::get<2>(extremes), boost::get<3>(extremes));
	} else {
		myRangeY = rangeY;
	}

	// Construct the result object
	boost::shared_ptr<GHistogram1D> result(new GHistogram1D(nBinsY, myRangeY));
	result->setXAxisLabel(this->yAxisLabel());
	result->setYAxisLabel("Number of entries");
	result->setPlotLabel(this->plotLabel() + " / y-projection");

	// Add data to the object
	for(std::size_t i=0; i<data_.size(); i++) {
		(*result) & boost::get<1>(data_.at(i));
	}

	// Return the data
	return result;
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * Specialization of projectX for <x_type, y_type, z_type> = <double, double, double>, that will return a
 * GHistogram1D object, wrapped into a boost::shared_ptr<GHistogram1D>. In case of a
 * default-constructed range, the function will attempt to determine suitable parameters
 * for the range settings.
 *
 * @param nBins The number of bins of the histogram
 * @param range The minimum and maximum boundaries of the histogram
 */
template<>
boost::shared_ptr<GDataCollector1T<double> >
GDataCollector3T<double, double,double>::projectX(std::size_t nBinsX, boost::tuple<double, double> rangeX) const {
   boost::tuple<double, double> myRangeX;
   if(rangeX == boost::tuple<double, double>()) {
      // Find out about the minimum and maximum values in the data_ array
      boost::tuple<double,double,double,double,double,double> extremes = Gem::Common::getMinMax(this->data_);
      myRangeX = boost::tuple<double,double>(boost::get<0>(extremes), boost::get<1>(extremes));
   } else {
      myRangeX = rangeX;
   }

   // Construct the result object
   boost::shared_ptr<GHistogram1D> result(new GHistogram1D(nBinsX, myRangeX));
   result->setXAxisLabel(this->xAxisLabel());
   result->setYAxisLabel("Number of entries");
   result->setPlotLabel(this->plotLabel() + " / x-projection");

   // Add data to the object
   for(std::size_t i=0; i<data_.size(); i++) {
      (*result) & boost::get<0>(data_.at(i));
   }

   // Return the data
   return result;
}

/******************************************************************************/
/**
 * Specialization of projectY for <x_type, y_type, z_type> = <double, double, double>, that will return a
 * GHistogram1D object, wrapped into a boost::shared_ptr<GHistogram1D>. In case of a
 * default-constructed range, the function will attempt to determine suitable parameters
 * for the range settings.
 *
 * @param nBins The number of bins of the histogram
 * @param range The minimum and maximum boundaries of the histogram
 */
template<>
boost::shared_ptr<GDataCollector1T<double> >
GDataCollector3T<double, double, double>::projectY(std::size_t nBinsY, boost::tuple<double, double> rangeY) const {
   boost::tuple<double, double> myRangeY;
   if(rangeY == boost::tuple<double, double>()) {
      // Find out about the minimum and maximum values in the data_ array
      boost::tuple<double,double,double,double,double,double> extremes = Gem::Common::getMinMax(data_);
      myRangeY = boost::tuple<double,double>(boost::get<2>(extremes), boost::get<3>(extremes));
   } else {
      myRangeY = rangeY;
   }

   // Construct the result object
   boost::shared_ptr<GHistogram1D> result(new GHistogram1D(nBinsY, myRangeY));
   result->setXAxisLabel(this->yAxisLabel());
   result->setYAxisLabel("Number of entries");
   result->setPlotLabel(this->plotLabel() + " / y-projection");

   // Add data to the object
   for(std::size_t i=0; i<data_.size(); i++) {
      (*result) & boost::get<1>(data_.at(i));
   }

   // Return the data
   return result;
}

/******************************************************************************/
/**
 * Specialization of projectZ for <x_type, y_type, z_type> = <double, double, double>, that will return a
 * GHistogram1D object, wrapped into a boost::shared_ptr<GHistogram1D>. In case of a
 * default-constructed range, the function will attempt to determine suitable parameters
 * for the range settings.
 *
 * @param nBins The number of bins of the histogram
 * @param range The minimum and maximum boundaries of the histogram
 */
template<>
boost::shared_ptr<GDataCollector1T<double> >
GDataCollector3T<double, double, double>::projectZ(std::size_t nBinsZ, boost::tuple<double, double> rangeZ) const {
   boost::tuple<double, double> myRangeZ;
   if(rangeZ == boost::tuple<double, double>()) {
      // Find out about the minimum and maximum values in the data_ array
      boost::tuple<double,double,double,double,double,double> extremes = Gem::Common::getMinMax(data_);
      myRangeZ = boost::tuple<double,double>(boost::get<4>(extremes), boost::get<5>(extremes));
   } else {
      myRangeZ = rangeZ;
   }

   // Construct the result object
   boost::shared_ptr<GHistogram1D> result(new GHistogram1D(nBinsZ, myRangeZ));
   result->setXAxisLabel(this->zAxisLabel());
   result->setYAxisLabel("Number of entries");
   result->setPlotLabel(this->plotLabel() + " / z-projection");

   // Add data to the object
   for(std::size_t i=0; i<data_.size(); i++) {
      (*result) & boost::get<2>(data_.at(i));
   }

   // Return the data
   return result;
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * Specialization of projectX for <x_type, y_type, z_type, w_type> = <double, double, double, double>,
 * that will return a GHistogram1D object, wrapped into a boost::shared_ptr<GHistogram1D>. In case of a
 * default-constructed range, the function will attempt to determine suitable parameters
 * for the range settings.
 *
 * @param nBins The number of bins of the histogram
 * @param range The minimum and maximum boundaries of the histogram
 */
template<>
boost::shared_ptr<GDataCollector1T<double> >
GDataCollector4T<double, double,double, double>::projectX(std::size_t nBinsX, boost::tuple<double, double> rangeX) const {
   boost::tuple<double, double> myRangeX;
   if(rangeX == boost::tuple<double, double>()) {
      // Find out about the minimum and maximum values in the data_ array
      boost::tuple<double,double,double,double,double,double,double,double> extremes = Gem::Common::getMinMax(this->data_);
      myRangeX = boost::tuple<double,double>(boost::get<0>(extremes), boost::get<1>(extremes));
   } else {
      myRangeX = rangeX;
   }

   // Construct the result object
   boost::shared_ptr<GHistogram1D> result(new GHistogram1D(nBinsX, myRangeX));
   result->setXAxisLabel(this->xAxisLabel());
   result->setYAxisLabel("Number of entries");
   result->setPlotLabel(this->plotLabel() + " / x-projection");

   // Add data to the object
   for(std::size_t i=0; i<data_.size(); i++) {
      (*result) & boost::get<0>(data_.at(i));
   }

   // Return the data
   return result;
}

/******************************************************************************/
/**
 * Specialization of projectY for <x_type, y_type, z_type, w_type> = <double, double, double, double>,
 * that will return a GHistogram1D object, wrapped into a boost::shared_ptr<GHistogram1D>. In case of a
 * default-constructed range, the function will attempt to determine suitable parameters
 * for the range settings.
 *
 * @param nBins The number of bins of the histogram
 * @param range The minimum and maximum boundaries of the histogram
 */
template<>
boost::shared_ptr<GDataCollector1T<double> >
GDataCollector4T<double, double, double, double>::projectY(std::size_t nBinsY, boost::tuple<double, double> rangeY) const {
   boost::tuple<double, double> myRangeY;
   if(rangeY == boost::tuple<double, double>()) {
      // Find out about the minimum and maximum values in the data_ array
      boost::tuple<double,double,double,double,double,double,double,double> extremes = Gem::Common::getMinMax(this->data_);
      myRangeY = boost::tuple<double,double>(boost::get<2>(extremes), boost::get<3>(extremes));
   } else {
      myRangeY = rangeY;
   }

   // Construct the result object
   boost::shared_ptr<GHistogram1D> result(new GHistogram1D(nBinsY, myRangeY));
   result->setXAxisLabel(this->yAxisLabel());
   result->setYAxisLabel("Number of entries");
   result->setPlotLabel(this->plotLabel() + " / y-projection");

   // Add data to the object
   for(std::size_t i=0; i<data_.size(); i++) {
      (*result) & boost::get<1>(data_.at(i));
   }

   // Return the data
   return result;
}

/******************************************************************************/
/**
 * Specialization of projectZ for <x_type, y_type, z_type, w_type> = <double, double, double, double>,
 * that will return a GHistogram1D object, wrapped into a boost::shared_ptr<GHistogram1D>. In case of a
 * default-constructed range, the function will attempt to determine suitable parameters
 * for the range settings.
 *
 * @param nBins The number of bins of the histogram
 * @param range The minimum and maximum boundaries of the histogram
 */
template<>
boost::shared_ptr<GDataCollector1T<double> >
GDataCollector4T<double, double, double, double>::projectZ(std::size_t nBinsZ, boost::tuple<double, double> rangeZ) const {
   boost::tuple<double, double> myRangeZ;
   if(rangeZ == boost::tuple<double, double>()) {
      // Find out about the minimum and maximum values in the data_ array
      boost::tuple<double,double,double,double,double,double,double,double> extremes = Gem::Common::getMinMax(this->data_);
      myRangeZ = boost::tuple<double,double>(boost::get<4>(extremes), boost::get<5>(extremes));
   } else {
      myRangeZ = rangeZ;
   }


   // Construct the result object
   boost::shared_ptr<GHistogram1D> result(new GHistogram1D(nBinsZ, myRangeZ));
   result->setXAxisLabel(this->zAxisLabel());
   result->setYAxisLabel("Number of entries");
   result->setPlotLabel(this->plotLabel() + " / z-projection");

   // Add data to the object
   for(std::size_t i=0; i<data_.size(); i++) {
      (*result) & boost::get<2>(data_.at(i));
   }

   // Return the data
   return result;
}

/******************************************************************************/
/**
 * Specialization of projectZ for <x_type, y_type, z_type, w_type> = <double, double, double, double>,
 * that will return a GHistogram1D object, wrapped into a boost::shared_ptr<GHistogram1D>. In case of a
 * default-constructed range, the function will attempt to determine suitable parameters
 * for the range settings.
 *
 * @param nBins The number of bins of the histogram
 * @param range The minimum and maximum boundaries of the histogram
 */
template<>
boost::shared_ptr<GDataCollector1T<double> >
GDataCollector4T<double, double, double, double>::projectW(std::size_t nBinsW, boost::tuple<double, double> rangeW) const {
   boost::tuple<double, double> myRangeW;
   if(rangeW == boost::tuple<double, double>()) {
      // Find out about the minimum and maximum values in the data_ array
      boost::tuple<double,double,double,double,double,double,double,double> extremes = Gem::Common::getMinMax(this->data_);
      myRangeW = boost::tuple<double,double>(boost::get<6>(extremes), boost::get<7>(extremes));
   } else {
      myRangeW = rangeW;
   }


   // Construct the result object
   boost::shared_ptr<GHistogram1D> result(new GHistogram1D(nBinsW, myRangeW));
   result->setXAxisLabel("w");
   result->setYAxisLabel("Number of entries");
   result->setPlotLabel(this->plotLabel() + " / w-projection");

   // Add data to the object
   for(std::size_t i=0; i<data_.size(); i++) {
      (*result) & boost::get<3>(data_.at(i));
   }

   // Return the data
   return result;
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * The default constructor
 */
GGraph2D::GGraph2D()
	: pM_(DEFPLOTMODE)
	, drawArrows_(false)
{ /* nothing */ }

/******************************************************************************/
/**
 * A copy constructor
 */
GGraph2D::GGraph2D(const GGraph2D& cp)
	: GDataCollector2T<double,double>(cp)
	, pM_(cp.pM_)
	, drawArrows_(cp.drawArrows_)
{ /* nothing */ }

/******************************************************************************/
/**
 * The destructor
 */
GGraph2D::~GGraph2D()
{ /* nothing */ }

/******************************************************************************/
/**
 * The assignment operator
 */
const GGraph2D & GGraph2D::operator=(const GGraph2D& cp) {
	// Copy our parent class'es data
	GDataCollector2T<double,double>::operator=(cp);

	// and then our own
	pM_ = cp.pM_;
	drawArrows_ = cp.drawArrows_;

	return *this;
}

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
 * Retrieve specific header settings for this plot
 */
std::string GGraph2D::headerData(bool isSecondary, std::size_t pId) const {
	std::ostringstream header_data;

	// Set up suitable arrays for the header
	std::string baseName = suffix(isSecondary, pId);
	std::string arrayBaseName = "array_" + baseName;

	std::string xArrayName = "x_" + arrayBaseName;
	std::string yArrayName = "y_" + arrayBaseName;

	std::string comment;
	if(dsMarker_ != "") {
		comment = "// " + dsMarker_;
	}

	header_data
		<< "  double " << xArrayName << "[" << boost::lexical_cast<std::string>(data_.size()) << "];" << (comment!=""?comment:"") << std::endl
		<< "  double " << yArrayName << "[" << boost::lexical_cast<std::string>(data_.size()) << "];" << std::endl
		<< std::endl;

	return header_data.str();
}

/******************************************************************************/
/**
 * Retrieves the actual data sets
 */
std::string GGraph2D::bodyData(bool isSecondary, std::size_t pId) const {
	std::ostringstream body_data;

	// Set up suitable arrays for the header
   std::string baseName = suffix(isSecondary, pId);
   std::string arrayBaseName = "array_" + baseName;

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
			<< "  " << xArrayName << "[" << posCounter << "] = " << boost::get<0>(*it) << ";" << "\t" << yArrayName << "[" << posCounter << "] = " << boost::get<1>(*it) << ";" << std::endl;

		posCounter++;
	}
	body_data << std::endl;

	return body_data.str();
}

/******************************************************************************/
/**
 * Retrieves specific draw commands for this plot
 */
std::string GGraph2D::footerData(bool isSecondary, std::size_t pId) const {
	std::ostringstream footer_data;

	// Set up suitable arrays for the header
   std::string baseName = suffix(isSecondary, pId);
   std::string arrayBaseName = "array_" + baseName;

	std::string xArrayName = "x_" + arrayBaseName;
	std::string yArrayName = "y_" + arrayBaseName;

	std::string graphName = std::string("graph") + baseName;

	std::string comment;
	if(dsMarker_ != "") {
		footer_data << "// " + dsMarker_ << std::endl;
	}

	// Retrieve the current drawing arguments
	std::string dA = this->drawingArguments(isSecondary);

	// Fill the data in our tuple-vector into a ROOT TGraph object
	footer_data
		<< "  TGraph *" << graphName << " = new TGraph(" << data_.size() <<", " << xArrayName << ", " << yArrayName << ");" << std::endl
		<< "  " << graphName << "->GetXaxis()->SetTitle(\"" << xAxisLabel() << "\");" << std::endl
		<< "  " << graphName << "->GetYaxis()->SetTitle(\"" << yAxisLabel() << "\");" << std::endl;

	if(plot_label_ != "") {
		footer_data
			<< "  " << graphName << "->SetTitle(\"" << plot_label_ << "\");" << std::endl;
	}  else {
		footer_data
			<< "  " << graphName << "->SetTitle(\" \");" << std::endl;
	}

	footer_data
		<< "  " << graphName << "->Draw(\""<< dA << "\");" << std::endl
		<< std::endl;

	if(drawArrows_ && data_.size() >= 2) {
		std::vector<boost::tuple<double, double> >::const_iterator it;
		std::size_t posCounter = 0;

		double x1 = boost::get<0>(*data_.begin());
		double y1 = boost::get<1>(*data_.begin());
		double x2,y2;

		for(it=data_.begin()+1; it!=data_.end(); ++it) {
			x2 = boost::get<0>(*it);
			y2 = boost::get<1>(*it);

			footer_data
				<< "  TArrow * ta_" << graphName << "_" << posCounter << " = new TArrow(" << x1 << ", " << y1 << "," << x2 << ", " << y2 << ", " << 0.05 << ", \"|>\");" << std::endl
				<< "  ta_" << graphName << "_" << posCounter << "->SetArrowSize(0.01);" << std::endl
				<< "  ta_" << graphName << "_" << posCounter << "->Draw();" << std::endl;

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

   if(this->drawingArguments_ != "") {
      dA = this->drawingArguments_;
   } else {
      if(SCATTER == pM_ || true==drawArrows_) {
         dA = "P";
      } else {
         dA = "PL";
      }

      if(isSecondary) {
         dA = dA + ",same";
      } else {
         dA = "A" + dA;
      }
   }

   return dA;
}

/******************************************************************************/
/**
 * Retrieve a clone of this object
 */
boost::shared_ptr<GBasePlotter> GGraph2D::clone() const {
   return boost::shared_ptr<GBasePlotter>(new GGraph2D(*this));
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * The default constructor
 */
GGraph2ED::GGraph2ED()
	: pM_(DEFPLOTMODE)
{ /* nothing */ }

/******************************************************************************/
/**
 * A copy constructor
 */
GGraph2ED::GGraph2ED(const GGraph2ED& cp)
	: GDataCollector2ET<double,double>(cp)
	, pM_(cp.pM_)
{ /* nothing */ }

/******************************************************************************/
/**
 * The destructor
 */
GGraph2ED::~GGraph2ED()
{ /* nothing */ }

/******************************************************************************/
/**
 * The assignment operator
 */
const GGraph2ED & GGraph2ED::operator=(const GGraph2ED& cp) {
	// Copy our parent class'es data
	GDataCollector2ET<double,double>::operator=(cp);

	// and then our own
	pM_ = cp.pM_;

	return *this;
}

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
 * Retrieve specific header settings for this plot
 */
std::string GGraph2ED::headerData(bool isSecondary, std::size_t pId) const {
	std::ostringstream header_data;

	// Set up suitable arrays for the header
   std::string baseName = suffix(isSecondary, pId);
   std::string arrayBaseName = "array_" + baseName;

	std::string  xArrayName = "x_" + arrayBaseName;
	std::string exArrayName = "ex_" + arrayBaseName;
	std::string  yArrayName = "y_" + arrayBaseName;
	std::string eyArrayName = "ey_" + arrayBaseName;

	std::string comment;
	if(dsMarker_ != "") {
		comment = "// " + dsMarker_;
	}

	header_data
		<< "  double " <<  xArrayName << "[" << boost::lexical_cast<std::string>(data_.size()) << "];" << (comment!=""?comment:"") << std::endl
		<< "  double " << exArrayName << "[" << boost::lexical_cast<std::string>(data_.size()) << "];" << std::endl
		<< "  double " <<  yArrayName << "[" << boost::lexical_cast<std::string>(data_.size()) << "];" << std::endl
		<< "  double " << eyArrayName << "[" << boost::lexical_cast<std::string>(data_.size()) << "];" << std::endl
		<< std::endl;

	return header_data.str();
}

/******************************************************************************/
/**
 * Retrieves the actual data sets
 */
std::string GGraph2ED::bodyData(bool isSecondary, std::size_t pId) const {
	std::ostringstream body_data;

	// Set up suitable arrays for the header
   std::string baseName = suffix(isSecondary, pId);
   std::string arrayBaseName = "array_" + baseName;

	std::string  xArrayName = "x_" + arrayBaseName;
	std::string exArrayName = "ex_" + arrayBaseName;
	std::string  yArrayName = "y_" + arrayBaseName;
	std::string eyArrayName = "ey_" + arrayBaseName;

	std::string comment;
	if(dsMarker_ != "") {
		body_data << "// " + dsMarker_ << std::endl;
	}

	// Fill data from the tuples into the arrays
	std::vector<boost::tuple<double, double, double, double> >::const_iterator it;
	std::size_t posCounter = 0;

	for(it=data_.begin(); it!=data_.end(); ++it) {
		body_data
			<< "  " <<  xArrayName << "[" << posCounter << "] = " << boost::get<0>(*it) << ";" << std::endl
			<< "  " << exArrayName << "[" << posCounter << "] = " << boost::get<1>(*it) << ";" << std::endl
			<< "  " <<  yArrayName << "[" << posCounter << "] = " << boost::get<2>(*it) << ";" << std::endl
			<< "  " << eyArrayName << "[" << posCounter << "] = " << boost::get<3>(*it) << ";" << std::endl;

		posCounter++;
	}
	body_data << std::endl;

	return body_data.str();
}

/******************************************************************************/
/**
 * Retrieves specific draw commands for this plot
 */
std::string GGraph2ED::footerData(bool isSecondary, std::size_t pId) const {
	std::ostringstream footer_data;

	// Set up suitable arrays for the header
   std::string baseName = suffix(isSecondary, pId);
   std::string arrayBaseName = "array_" + baseName;

	std::string  xArrayName = "x_" + arrayBaseName;
	std::string exArrayName = "ex_" + arrayBaseName;
	std::string  yArrayName = "y_" + arrayBaseName;
	std::string eyArrayName = "ey_" + arrayBaseName;

	std::string graphName = std::string("graph_") + baseName;

	std::string comment;
	if(dsMarker_ != "") {
		footer_data << "// " + dsMarker_ << std::endl;
	}

	// Check whether custom drawing arguments have been set or whether one
	// of our generic choices has been selected
	std::string dA = this->drawingArguments(isSecondary);

	// Fill the data in our tuple-vector into a ROOT TGraphErrors object
	footer_data
		<< "  TGraphErrors *" << graphName << " = new TGraphErrors(" << data_.size() <<", " << xArrayName << ", " << yArrayName << ", " << exArrayName << " ," << eyArrayName << ");" << std::endl
		<< "  " << graphName << "->GetXaxis()->SetTitle(\"" << xAxisLabel() << "\");" << std::endl
		<< "  " << graphName << "->GetYaxis()->SetTitle(\"" << yAxisLabel() << "\");" << std::endl;

	if(plot_label_ != "") {
		footer_data
			<< "  " << graphName << "->SetTitle(\"" << plot_label_ << "\");" << std::endl;
	}  else {
		footer_data
			<< "  " << graphName << "->SetTitle(\" \");" << std::endl;
	}

	footer_data
		<< "  " << graphName << "->Draw(\""<< dA << "\");" << std::endl
		<< std::endl;

	return footer_data.str();
}

/******************************************************************************/
/**
 * Retrieve the current drawing arguments
 */
std::string GGraph2ED::drawingArguments(bool isSecondary) const {
   std::string dA = "";

   if(this->drawingArguments_ != "") {
      dA = this->drawingArguments_;
   } else {
      if(SCATTER == pM_) {
         dA = "P";
      } else {
         dA = "PL";
      }

      if(isSecondary) {
         dA = dA + ",same";
      } else {
         dA = "A" + dA;
      }
   }

   return dA;
}

/******************************************************************************/
/**
 * Retrieve a clone of this object
 */
boost::shared_ptr<GBasePlotter> GGraph2ED::clone() const {
   return boost::shared_ptr<GBasePlotter>(new GGraph2ED(*this));
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * The default constructor
 */
GGraph3D::GGraph3D()
   : drawLines_(false)
{ /* nothing */ }

/******************************************************************************/
/**
 * A copy constructor
 */
GGraph3D::GGraph3D(const GGraph3D& cp)
   : GDataCollector3T<double,double,double>(cp)
   , drawLines_(cp.drawLines_)
{ /* nothing */ }

/******************************************************************************/
/**
 * The destructor
 */
GGraph3D::~GGraph3D()
{ /* nothing */ }

/******************************************************************************/
/**
 * The assignment operator
 */
const GGraph3D & GGraph3D::operator=(const GGraph3D& cp) {
   // Copy our parent class'es data
   GDataCollector3T<double,double,double>::operator=(cp);

   // and then our own
   drawLines_ = cp.drawLines_;

   return *this;
}

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
 * Retrieve specific header settings for this plot
 */
std::string GGraph3D::headerData(bool isSecondary, std::size_t pId) const {
   std::ostringstream header_data;

   // Set up suitable arrays for the header
   std::string baseName = suffix(isSecondary, pId);
   std::string arrayBaseName = "array_" + baseName;

   std::string xArrayName = "x_" + arrayBaseName;
   std::string yArrayName = "y_" + arrayBaseName;
   std::string zArrayName = "z_" + arrayBaseName;

   std::string comment;
   if(dsMarker_ != "") {
      comment = "// " + dsMarker_;
   }

   header_data
      << "  double " << xArrayName << "[" << boost::lexical_cast<std::string>(data_.size()) << "];" << (comment!=""?comment:"") << std::endl
      << "  double " << yArrayName << "[" << boost::lexical_cast<std::string>(data_.size()) << "];" << std::endl
      << "  double " << zArrayName << "[" << boost::lexical_cast<std::string>(data_.size()) << "];" << std::endl
      << std::endl;

   return header_data.str();
}

/******************************************************************************/
/**
 * Retrieves the actual data sets
 */
std::string GGraph3D::bodyData(bool isSecondary, std::size_t pId) const {
   std::ostringstream body_data;

   // Set up suitable arrays for the header
   std::string baseName = suffix(isSecondary, pId);
   std::string arrayBaseName = "array_" + baseName;

   std::string xArrayName = "x_" + arrayBaseName;
   std::string yArrayName = "y_" + arrayBaseName;
   std::string zArrayName = "z_" + arrayBaseName;

   std::string comment;
   if(dsMarker_ != "") {
      body_data << "// " + dsMarker_ << std::endl;
   }

   // Fill data from the tuples into the arrays
   std::vector<boost::tuple<double, double, double> >::const_iterator it;
   std::size_t posCounter = 0;

   for(it=data_.begin(); it!=data_.end(); ++it) {
      body_data
         << "  " << xArrayName << "[" << posCounter << "] = " << boost::get<0>(*it) << ";" << "\t"
                 << yArrayName << "[" << posCounter << "] = " << boost::get<1>(*it) << ";" << "\t"
                 << zArrayName << "[" << posCounter << "] = " << boost::get<2>(*it) << ";" << std::endl;

      posCounter++;
   }
   body_data << std::endl;

   return body_data.str();
}

/******************************************************************************/
/**
 * Retrieves specific draw commands for this plot
 */
std::string GGraph3D::footerData(bool isSecondary, std::size_t pId) const {
   std::ostringstream footer_data;

   // Set up suitable arrays for the header
   std::string baseName = suffix(isSecondary, pId);
   std::string arrayBaseName = "array_" + baseName;

   std::string xArrayName = "x_" + arrayBaseName;
   std::string yArrayName = "y_" + arrayBaseName;
   std::string zArrayName = "z_" + arrayBaseName;

   std::string graphName = std::string("graph_") + baseName;

   std::string comment;
   if(dsMarker_ != "") {
      footer_data << "// " + dsMarker_ << std::endl;
   }

   // Check whether custom drawing arguments have been set or whether one
   // of our generic choices has been selected
   std::string dA = this->drawingArguments(isSecondary);

   // Fill the data in our tuple-vector into a ROOT TGraph object
   footer_data
      << "  TGraph2D *" << graphName << " = new TGraph2D(" << data_.size() <<", " << xArrayName << ", " << yArrayName << ", " << zArrayName << ");" << std::endl
      << "  " << graphName << "->GetXaxis()->SetTitle(\"" << xAxisLabel() << "\");" << std::endl
      << "  " << graphName << "->GetXaxis()->SetTitleOffset(1.5);" << std::endl
      << "  " << graphName << "->GetYaxis()->SetTitle(\"" << yAxisLabel() << "\");" << std::endl
      << "  " << graphName << "->GetYaxis()->SetTitleOffset(1.5);" << std::endl
      << "  " << graphName << "->GetZaxis()->SetTitle(\"" << zAxisLabel() << "\");" << std::endl
      << "  " << graphName << "->GetZaxis()->SetTitleOffset(1.5);" << std::endl
      << "  " << graphName << "->SetMarkerStyle(20);" << std::endl
      << "  " << graphName << "->SetMarkerSize(1);" << std::endl
      << "  " << graphName << "->SetMarkerColor(2);" << std::endl;

   if(plot_label_ != "") {
      footer_data
         << "  " << graphName << "->SetTitle(\"" << plot_label_ << "\");" << std::endl;
   }  else {
      footer_data
         << "  " << graphName << "->SetTitle(\" \");" << std::endl;
   }

   footer_data
      << "  " << graphName << "->Draw(\""<< dA << "\");" << std::endl
      << std::endl;

   if(drawLines_ && data_.size() >= 2) {
      std::vector<boost::tuple<double, double, double> >::const_iterator it;
      std::size_t posCounter = 0;

      double x,y,z;

      footer_data
      << "  TPolyLine3D *lines_" << graphName << " = new TPolyLine3D(" << data_.size() << ");" << std::endl
      << std::endl;

      for(it=data_.begin()+1; it!=data_.end(); ++it) {
         x = boost::get<0>(*it);
         y = boost::get<1>(*it);
         z = boost::get<2>(*it);

         footer_data
         << "  lines_" << graphName << "->SetPoint(" << posCounter << ", " << x << ", " << y << ", " << z << ");";

         posCounter++;
      }
      footer_data
      << std::endl
      << "  lines_" << graphName << "->SetLineWidth(3);" << std::endl
      << "  lines_" << graphName << "->Draw();" << std::endl
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

   if(this->drawingArguments_ != "") {
      dA = this->drawingArguments_;
   } else {
      dA = "P";

      if(isSecondary) {
         dA = dA + ",same";
      } else {
         dA = "A" + dA;
      }
   }

   return dA;
}

/******************************************************************************/
/**
 * Retrieve a clone of this object
 */
boost::shared_ptr<GBasePlotter> GGraph3D::clone() const {
   return boost::shared_ptr<GBasePlotter>(new GGraph3D(*this));
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * The default constructor
 */
GGraph4D::GGraph4D()
   : minMarkerSize_(0.001)
   , maxMarkerSize_(1.)
   , smallWLargeMarker_(true)
   , nBest_(0)
{ /* nothing */ }

/******************************************************************************/
/**
 * A copy constructor
 */
GGraph4D::GGraph4D(const GGraph4D& cp)
   : GDataCollector4T<double,double,double,double>(cp)
   , minMarkerSize_(cp.minMarkerSize_)
   , maxMarkerSize_(cp.maxMarkerSize_)
   , smallWLargeMarker_(cp.smallWLargeMarker_)
   , nBest_(cp.nBest_)
{ /* nothing */ }

/******************************************************************************/
/**
 * The destructor
 */
GGraph4D::~GGraph4D()
{ /* nothing */ }

/******************************************************************************/
/**
 * The assignment operator
 */
const GGraph4D & GGraph4D::operator=(const GGraph4D& cp) {
   // Copy our parent class'es data
   GDataCollector4T<double,double,double,double>::operator=(cp);

   // copy local data
   minMarkerSize_     = cp.minMarkerSize_;
   maxMarkerSize_     = cp.maxMarkerSize_;
   smallWLargeMarker_ = cp.smallWLargeMarker_;
   nBest_             = cp.nBest_;

   return *this;
}

/******************************************************************************/
/**
 * Allows to set the minimum marker size
 */
void GGraph4D::setMinMarkerSize(const double& minMarkerSize) {
   if(minMarkerSize < 0.) {
      glogger
      << "In GGraph4D::setMinMarkerSize(): Error!" << std::endl
      << "Received invalid minimum marker size: " << minMarkerSize << std::endl
      << GEXCEPTION;
   }

   minMarkerSize_ = minMarkerSize;
}

/******************************************************************************/
/**
 * Allows to set the maximum marker size
 */
void GGraph4D::setMaxMarkerSize(const double& maxMarkerSize) {
   if(maxMarkerSize < 0. || maxMarkerSize < minMarkerSize_) {
      glogger
      << "In GGraph4D::setMinMarkerSize(): Error!" << std::endl
      << "Received invalid minimum marker size: " << minMarkerSize_ << " " << maxMarkerSize << "." << std::endl
      << "Always set the lower boundary first." << std::endl
      << GEXCEPTION;
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
void GGraph4D::setSmallWLargeMarker(const bool& swlm) {
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
void GGraph4D::setNBest(const std::size_t& nBest) {
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
 * Retrieve specific header settings for this plot
 */
std::string GGraph4D::headerData(bool isSecondary, std::size_t pId) const {
   std::ostringstream header_data;

   // nothing

   return header_data.str();
}

/******************************************************************************/
/**
 * Retrieves the actual data sets
 */
std::string GGraph4D::bodyData(bool isSecondary, std::size_t pId) const {
   std::ostringstream body_data;

   // nothing

   return body_data.str();
}

/******************************************************************************/
/**
 * Retrieves specific draw commands for this plot
 */
std::string GGraph4D::footerData(bool isSecondary, std::size_t pId) const {
   std::vector<boost::tuple<double,double,double,double> > localData = data_;

   std::string baseName = suffix(isSecondary, pId);

   // Sort the data, so we can select the nBest_ best more easily
   if(smallWLargeMarker_) {
      std::sort(
            localData.begin()
            , localData.end()
            , &GGraph4D::comp4Asc
       );
   } else {
      std::sort(
            localData.begin()
            , localData.end()
            , &GGraph4D::comp4Desc
       );
   }

   std::ostringstream footer_data;

   // Find out about the minimum and maximum values of the data vector
   boost::tuple<double,double,double,double,double,double,double,double> minMax = Gem::Common::getMinMax(localData);

   // Set up TView object for our 3D data, spanning the minimum and maximum values
   footer_data
   << "  TH3F *fr = new TH3F(\"fr\",\"fr\","
   <<               "10, " << boost::get<0>(minMax) << ", " << boost::get<1>(minMax) << ", "
   <<               "10, " << boost::get<2>(minMax) << ", " << boost::get<3>(minMax) << ", "
   <<               "10, " << boost::get<4>(minMax) << ", " << boost::get<5>(minMax) << ");" << std::endl
   << "  fr->SetTitle(\" \");" << std::endl
   << "  fr->GetXaxis()->SetTitle(\"" << xAxisLabel() << "\");" << std::endl
   << "  fr->GetXaxis()->SetTitleOffset(1.6);" << std::endl
   << "  fr->GetYaxis()->SetTitle(\"" << yAxisLabel() << "\");" << std::endl
   << "  fr->GetYaxis()->SetTitleOffset(1.6);" << std::endl
   << "  fr->GetZaxis()->SetTitle(\"" << zAxisLabel() << "\");" << std::endl
   << "  fr->GetZaxis()->SetTitleOffset(1.6);" << std::endl
   << std::endl
   << "  fr->Draw();" << std::endl;

   double wMin = boost::get<6>(minMax);
   double wMax = boost::get<7>(minMax);

   // Fill data from the tuples into the arrays
   double wRange = wMax-wMin;
   std::size_t pos = 0;
   std::vector<boost::tuple<double, double, double, double> >::const_iterator it;
   for(it=localData.begin(); it!=localData.end(); ++it) {
      std::string polyMarkerName = std::string("pm3d_") + baseName + std::string("_") + boost::lexical_cast<std::string>(pos);

      // create a TPolyMarker3D for a single data point
      footer_data
      << "  TPolyMarker3D *" << polyMarkerName << " = new TPolyMarker3D(1);" << std::endl;

      double x = boost::get<0>(*it);
      double y = boost::get<1>(*it);
      double z = boost::get<2>(*it);
      double w = boost::get<3>(*it);

      // Translate the fourth component into a marker size. By default,
      // smaller values will yield the largest value
      double markerSize = 0.;
      if(0==pos) {
         markerSize = 2*maxMarkerSize_;
      } else {
         if(smallWLargeMarker_) {
            markerSize = minMarkerSize_ + (maxMarkerSize_-minMarkerSize_)*pow((1. - (w-wMin)/wRange),8.);
         } else {
            markerSize = minMarkerSize_ + (maxMarkerSize_-minMarkerSize_)*pow(((w-wMin)/wRange), 8);
         }
      }

      footer_data
      << polyMarkerName << "->SetPoint(" << pos << ", " << x << ", " << y << ", " << z << "); // w = " << w << std::endl
      << polyMarkerName << "->SetMarkerSize(" << markerSize << ");" << std::endl
      << polyMarkerName << "->SetMarkerColor(" << (0==pos?4:2) << ");" << std::endl
      << polyMarkerName << "->SetMarkerStyle(8);" << std::endl
      << polyMarkerName << "->Draw();" << std::endl;

      pos++;

      if(nBest_ && pos >= nBest_) {
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
 * Retrieve a clone of this object
 */
boost::shared_ptr<GBasePlotter> GGraph4D::clone() const {
   return boost::shared_ptr<GBasePlotter>(new GGraph4D(*this));
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * The standard constructor
 */
GHistogram1D::GHistogram1D(
	const std::size_t& nBinsX
	, const double& minX
	, const double& maxX
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
	const std::size_t& nBinsX
	, const boost::tuple<double,double>& rangeX
)
	: nBinsX_(nBinsX)
	, minX_(boost::get<0>(rangeX))
	, maxX_(boost::get<1>(rangeX))
{ /* nothing */ }

/******************************************************************************/
/**
 * A copy constructor
 *
 * @param cp a copy of another GHistogram1D object
 */
GHistogram1D::GHistogram1D(const GHistogram1D& cp)
	: GDataCollector1T<double>(cp)
	, nBinsX_(cp.nBinsX_)
	, minX_(cp. minX_)
	, maxX_(cp.maxX_)
{ /* nothing */ }

/******************************************************************************/
/**
 * The destructor
 */
GHistogram1D::~GHistogram1D()
{ /* nothing */ }

/******************************************************************************/
/**
 * The assignment operator
 *
 * @param cp A copy of another GHistogram1D object
 */
const GHistogram1D &GHistogram1D::operator=(const GHistogram1D& cp) {
	// Copy our parent class'es data ...
	GDataCollector1T<double>::operator=(cp);

	// ... and then our local data
	nBinsX_ = cp.nBinsX_;
	minX_ = cp. minX_;
	maxX_ = cp.maxX_;

	return *this;
}

/******************************************************************************/
/**
 * Retrieve specific header settings for this plot
 */
std::string GHistogram1D::headerData(bool isSecondary, std::size_t pId) const {
	std::ostringstream header_data;

	std::string comment;
	if(dsMarker_ != "") {
		comment = "// " + dsMarker_;
	}

	std::string histName = "histD" + suffix(isSecondary, pId);

	header_data
		<< "  TH1D *" << histName << " = new TH1D(\"" << histName << "\", \"" << histName << "\"," << nBinsX_ << ", " << minX_ << ", " << maxX_ << ");" << (comment!=""?comment:"") << std::endl
		<< std::endl;

	return header_data.str();
}

/******************************************************************************/
/**
 * Retrieves the actual data sets
 */
std::string GHistogram1D::bodyData(bool isSecondary, std::size_t pId) const {
	std::ostringstream body_data;

	std::string comment;
	if(dsMarker_ != "") {
		comment = "// " + dsMarker_;
	} else {
		comment = "";
	}

	std::string histName = "histD" + suffix(isSecondary, pId);

	std::vector<double>::const_iterator it;
	std::size_t posCounter = 0;
	for(it=data_.begin(); it!=data_.end(); ++it) {
		body_data
			<< "  " << histName << "->Fill(" << *it << ");" << (posCounter==0?comment:("")) << std::endl;
		posCounter++;
	}
	body_data << std::endl;

	return body_data.str();
}

/******************************************************************************/
/**
 * Retrieves specific draw commands for this plot
 */
std::string GHistogram1D::footerData(bool isSecondary, std::size_t pId) const {
	std::ostringstream footer_data;

	std::string histName = "histD" + suffix(isSecondary, pId);

	if(plot_label_ != "") {
		footer_data
			<< "  " << histName << "->SetTitle(\"" << plot_label_ << "\");" << std::endl;
	}  else {
		footer_data
			<< "  " << histName << "->SetTitle(\" \");" << std::endl;
	}

	std::string comment;
	if(dsMarker_ != "") {
		footer_data << "// " + dsMarker_ << std::endl;
	}

	// Check whether custom drawing arguments have been set
	std::string dA = this->drawingArguments(isSecondary);

	footer_data
	    << "  " << histName << "->GetXaxis()->SetTitle(\"" << xAxisLabel() << "\");" << std::endl
	    << "  " << histName << "->GetYaxis()->SetTitle(\"" << yAxisLabel() << "\");" << std::endl
		<< "  " << histName << "->Draw(\""<< dA << "\");" << std::endl
		<< std::endl;

	return footer_data.str();
}

/******************************************************************************/
/**
 * Retrieve the current drawing arguments
 */
std::string GHistogram1D::drawingArguments(bool isSecondary) const {
   std::string dA = "";

   if(drawingArguments_ != "") {
      dA = drawingArguments_;
   } else {
      if(isSecondary) {
         if("" == dA) {
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
 * Retrieve a clone of this object
 */
boost::shared_ptr<GBasePlotter> GHistogram1D::clone() const {
   return boost::shared_ptr<GBasePlotter>(new GHistogram1D(*this));
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * The standard constructor
 */
GHistogram1I::GHistogram1I(
   const std::size_t& nBinsX
   , const double& minX
   , const double& maxX
)
   : GDataCollector1T<boost::int32_t>()
   , nBinsX_(nBinsX)
   , minX_(minX)
   , maxX_(maxX)
{ /* nothing */ }

/******************************************************************************/
/**
 * Initialization with a range in the form of a tuple
 */
GHistogram1I::GHistogram1I(
   const std::size_t& nBinsX
   , const boost::tuple<double,double>& rangeX
)
   : GDataCollector1T<boost::int32_t>()
   , nBinsX_(nBinsX)
   , minX_(boost::get<0>(rangeX))
   , maxX_(boost::get<1>(rangeX))
{ /* nothing */ }

/******************************************************************************/
/**
 * A copy constructor
 *
 * @param cp a copy of another GHistogram1I object
 */
GHistogram1I::GHistogram1I(const GHistogram1I& cp)
   : GDataCollector1T<boost::int32_t>(cp)
   , nBinsX_(cp.nBinsX_)
   , minX_(cp. minX_)
   , maxX_(cp.maxX_)
{ /* nothing */ }

/******************************************************************************/
/**
 * The destructor
 */
GHistogram1I::~GHistogram1I()
{ /* nothing */ }

/******************************************************************************/
/**
 * The assignment operator
 *
 * @param cp A copy of another GHistogram1I object
 */
const GHistogram1I& GHistogram1I::operator=(const GHistogram1I& cp) {
   // Copy our parent class'es data ...
   GDataCollector1T<boost::int32_t>::operator=(cp);

   // ... and then our local data
   nBinsX_ = cp.nBinsX_;
   minX_ = cp. minX_;
   maxX_ = cp.maxX_;

   return *this;
}

/******************************************************************************/
/**
 * Retrieve specific header settings for this plot
 */
std::string GHistogram1I::headerData(bool isSecondary, std::size_t pId) const {
   std::ostringstream header_data;

   std::string comment;
   if(dsMarker_ != "") {
      comment = "// " + dsMarker_;
   }

   std::string histName = "histI" + suffix(isSecondary, pId);

   header_data
      << "  TH1I *" << histName << " = new TH1I(\"" << histName << "\", \"" << histName << "\"," << nBinsX_ << ", " << minX_ << ", " << maxX_ << ");" << (comment!=""?comment:"") << std::endl
      << std::endl;

   return header_data.str();
}

/******************************************************************************/
/**
 * Retrieves the actual data sets
 */
std::string GHistogram1I::bodyData(bool isSecondary, std::size_t pId) const {
   std::ostringstream body_data;

   std::string comment;
   if(dsMarker_ != "") {
      comment = "// " + dsMarker_;
   } else {
      comment = "";
   }

   std::string histName = "histI" + suffix(isSecondary, pId);

   std::vector<boost::int32_t>::const_iterator it;
   std::size_t posCounter = 0;
   for(it=data_.begin(); it!=data_.end(); ++it) {
      body_data
         << "  " << histName << "->Fill(" << *it << ");" << (posCounter==0?comment:("")) << std::endl;
      posCounter++;
   }

   body_data << std::endl;

   return body_data.str();
}

/******************************************************************************/
/**
 * Retrieves specific draw commands for this plot
 */
std::string GHistogram1I::footerData(bool isSecondary, std::size_t pId) const {
   std::ostringstream footer_data;

   std::string histName = "histI" + suffix(isSecondary, pId);

   if(plot_label_ != "") {
      footer_data
         << "  " << histName << "->SetTitle(\"" << plot_label_ << "\");" << std::endl;
   }  else {
      footer_data
         << "  " << histName << "->SetTitle(\" \");" << std::endl;
   }

   std::string comment;
   if(dsMarker_ != "") {
      footer_data << "// " + dsMarker_ << std::endl;
   }

   // Check whether custom drawing arguments have been set
   std::string dA = this->drawingArguments(isSecondary);

   footer_data
       << "  " << histName << "->GetXaxis()->SetTitle(\"" << xAxisLabel() << "\");" << std::endl
       << "  " << histName << "->GetYaxis()->SetTitle(\"" << yAxisLabel() << "\");" << std::endl
      << "  " << histName << "->Draw(\""<< dA << "\");" << std::endl
      << std::endl;

   return footer_data.str();
}

/******************************************************************************/
/**
 * Retrieve the current drawing arguments
 */
std::string GHistogram1I::drawingArguments(bool isSecondary) const {
   std::string dA = "";

   if(drawingArguments_ != "") {
      dA = drawingArguments_;
   } else {
      if(isSecondary) {
         if("" == dA) {
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
 * Retrieve a clone of this object
 */
boost::shared_ptr<GBasePlotter> GHistogram1I::clone() const {
   return boost::shared_ptr<GBasePlotter>(new GHistogram1I(*this));
}


/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * The standard constructor
 */
GHistogram2D::GHistogram2D(
	const std::size_t& nBinsX
	, const std::size_t& nBinsY
	, const double& minX
	, const double& maxX
	, const double& minY
	, const double& maxY
)
	: nBinsX_(nBinsX)
	, nBinsY_(nBinsY)
	, minX_(minX)
	, maxX_(maxX)
	, minY_(minY)
	, maxY_(maxY)
	, dropt_(TDEMPTY)
{ /* nothing */ }

/******************************************************************************/
/**
 * Initialization with ranges
 */
GHistogram2D::GHistogram2D(
	const std::size_t& nBinsX
	, const std::size_t& nBinsY
	, const boost::tuple<double,double>& rangeX
	, const boost::tuple<double,double>& rangeY
)
	: nBinsX_(nBinsX)
	, nBinsY_(nBinsY)
	, minX_(boost::get<0>(rangeX))
	, maxX_(boost::get<1>(rangeX))
	, minY_(boost::get<0>(rangeY))
	, maxY_(boost::get<1>(rangeY))
	, dropt_(TDEMPTY)
{ /* nothing */ }

/******************************************************************************/
/**
 * A copy constructor
 *
 * @param cp a copy of another GHistogram2D object
 */
GHistogram2D::GHistogram2D(const GHistogram2D& cp)
	: GDataCollector2T<double, double>(cp)
	, nBinsX_(cp.nBinsX_)
	, nBinsY_(cp.nBinsY_)
	, minX_(cp. minX_)
	, maxX_(cp.maxX_)
	, minY_(cp. minY_)
	, maxY_(cp.maxY_)
	, dropt_(cp.dropt_)
{ /* nothing */ }

/******************************************************************************/
/**
 * The destructor
 */
GHistogram2D::~GHistogram2D()
{ /* nothing */ }

/******************************************************************************/
/**
 * The assignment operator
 *
 * @param cp A copy of another GHistogram2D object
 */
const GHistogram2D &GHistogram2D::operator=(const GHistogram2D& cp) {
	// Copy our parent class'es data ...
	GDataCollector2T<double,double>::operator=(cp);

	// ... and then our local data
	nBinsX_ = cp.nBinsX_;
	nBinsY_ = cp.nBinsY_;
	minX_ = cp. minX_;
	maxX_ = cp.maxX_;
	minY_ = cp. minY_;
	maxY_ = cp.maxY_;
	dropt_ = cp.dropt_;

	return *this;
}

/******************************************************************************/
/**
 * Retrieve specific header settings for this plot
 */
std::string GHistogram2D::headerData(bool isSecondary, std::size_t pId) const {
	std::ostringstream header_data;

	std::string comment;
	if(dsMarker_ != "") {
		comment = "// " + dsMarker_;
	}

	std::string histName = "hist2D" + suffix(isSecondary, pId);

	header_data
		<< "  TH2D *"
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
		<< (comment!=""?comment:"")
		<< std::endl
		<< std::endl;

	return header_data.str();
}

/******************************************************************************/
/**
 * Retrieves the actual data sets
 */
std::string GHistogram2D::bodyData(bool isSecondary, std::size_t pId) const {
	std::ostringstream body_data;

	std::string comment;
	if(dsMarker_ != "") {
		comment = "// " + dsMarker_;
	} else {
		comment = "";
	}

	std::string histName = "hist2D" + suffix(isSecondary, pId);

	std::vector<boost::tuple<double,double> >::const_iterator it;
	std::size_t posCounter = 0;
	for(it=data_.begin(); it!=data_.end(); ++it) {
		body_data
			<< "  " << histName << "->Fill(" << boost::get<0>(*it) << ", " << boost::get<1>(*it) << ");" << (posCounter==0?comment:("")) << std::endl;
		posCounter++;
	}

	body_data << std::endl;

	return body_data.str();
}

/******************************************************************************/
/**
 * Retrieves specific draw commands for this plot
 */
std::string GHistogram2D::footerData(bool isSecondary, std::size_t pId) const {
	std::ostringstream footer_data;

	std::string histName = "hist2D" + suffix(isSecondary, pId);

	if(plot_label_ != "") {
		footer_data
			<< "  " << histName << "->SetTitle(\"" << plot_label_ << "\");" << std::endl;
	}  else {
		footer_data
			<< "  " << histName << "->SetTitle(\" \");" << std::endl;
	}

	std::string comment;
	if(dsMarker_ != "") {
		footer_data << "// " + dsMarker_ << std::endl;
	}

	// Check whether custom drawing arguments have been set
	std::string dA = this->drawingArguments(isSecondary);

	footer_data
	<< "  " << histName << "->GetXaxis()->SetTitle(\"" << xAxisLabel() << "\");" << std::endl
	<< "  " << histName << "->GetYaxis()->SetTitle(\"" << yAxisLabel() << "\");" << std::endl
	<< "  " << histName << "->Draw(\""<< dA << "\");" << std::endl
	<< std::endl;

	return footer_data.str();
}

/******************************************************************************/
/**
 * Retrieve the current drawing arguments
 */
std::string GHistogram2D::drawingArguments(bool isSecondary) const {
   std::string dA = "";

   if(drawingArguments_ != "") {
      dA = drawingArguments_;
   } else {
      switch(dropt_) {
      case TDEMPTY:
         dA = "";
         break;

      case SURFONE:
         dA = "SURF1";
         break;

      case SURFTWOZ:
         dA = "SURF2Z";
         break;

      case SURFTHREE:
         dA = "SURF3";
         break;

      case SURFFOUR:
         dA = "SURF4";
         break;

      case CONTZ:
         dA = "CONTZ";
         break;

      case CONTONE:
         dA = "CONT1";
         break;

      case CONTTWO:
         dA = "CONT2";
         break;

      case CONTTHREE:
         dA = "CONT3";
         break;

      case TEXT:
         dA = "TEXT";
         break;

      case SCAT:
         dA = "SCAT";
         break;

      case BOX:
         dA = "BOX";
         break;

      case ARR:
         dA = "ARR";
         break;

      case COLZ:
         dA = "COLZ";
         break;

      case LEGO:
         dA = "LEGO";
         break;

      case LEGOONE:
         dA = "LEGO1";
         break;

      case SURFONEPOL:
         dA = "SURF1POL";
         break;

      case SURFONECYL:
         dA = "SURF1CYL";
         break;

      default:
      {
         glogger
         << "In GHistogram2D::drawingArguments(): Error!" << std::endl
         << "Encountered unknown drawing option " << dropt_ << std::endl
         << GEXCEPTION;
      }
      break;
      }

      if(isSecondary) {
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
 * Retrieve a clone of this object
 */
boost::shared_ptr<GBasePlotter> GHistogram2D::clone() const {
   return boost::shared_ptr<GBasePlotter>(new GHistogram2D(*this));
}


/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * The standard constructor
 *
 * @param fD A descriptor for the function to be plotted
 */
GFunctionPlotter1D::GFunctionPlotter1D(
		const std::string& fD
		, const boost::tuple<double,double>& xExtremes
)
	: functionDescription_(fD)
	, xExtremes_(xExtremes)
	, nSamplesX_(DEFNSAMPLES)
{ /* nothing */ }

/******************************************************************************/
/**
 * A copy constructor
 *
 * @param cp A reference to another GFunctionPlotter1D object
 */
GFunctionPlotter1D::GFunctionPlotter1D(const GFunctionPlotter1D& cp)
	: GBasePlotter(cp)
	, functionDescription_(cp.functionDescription_)
	, xExtremes_(cp.xExtremes_)
	, nSamplesX_(cp.nSamplesX_)
{ /* nothing */ }

/******************************************************************************/
/**
 * The destructor
 */
GFunctionPlotter1D::~GFunctionPlotter1D()
{ /* nothing */ }

/******************************************************************************/
/**
 * The assignment operator
 *
 * @param cp A reference to another GFunctionPlotter1D object
 */
const GFunctionPlotter1D & GFunctionPlotter1D::operator=(const GFunctionPlotter1D& cp) {
	// Copy our parent class'es data
	GBasePlotter::operator=(cp);

	// and then our local data
	functionDescription_ = cp.functionDescription_;
	xExtremes_ = cp.xExtremes_;
	nSamplesX_ = cp.nSamplesX_;

	return *this;
}

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
 * Retrieve specific header settings for this plot
 *
 * @return The code to be added to the plot header for this function
 */
std::string GFunctionPlotter1D::headerData(bool isSecondary, std::size_t pId) const {
	// Check the extreme values for consistency
	if(boost::get<0>(xExtremes_) >= boost::get<1>(xExtremes_)) {
	   glogger
	   << "In GFunctionPlotter1D::headerData(): Error!" << std::endl
      << "lower boundary >= upper boundary: " << boost::get<0>(xExtremes_) << " / " << boost::get<1>(xExtremes_) << std::endl
      << GEXCEPTION;
	}

	std::ostringstream result;

	std::string comment;
	if(dsMarker_ != "") {
		comment = "// " + dsMarker_;
	}

	std::string functionName = "func1D" + suffix(isSecondary, pId);
	result << "  TF1 *" << functionName << " = new TF1(\"" << functionName << "\", \"" << functionDescription_ << "\"," << boost::get<0>(xExtremes_) << ", " << boost::get<1>(xExtremes_) << ");" << (comment!=""?comment:"") << std::endl;

	return result.str();
}

/******************************************************************************/
/**
 * Retrieves the actual data sets
 *
 * @return The code to be added to the plot's data section for this function
 */
std::string GFunctionPlotter1D::bodyData(bool isSecondary, std::size_t pId) const {
	// No data needs to be added for a function plotter
	return std::string();
}

/******************************************************************************/
/**
 * Retrieves specific draw commands for this plot
 *
 * @return The draw command to be added to the plot's data for this function
 */
std::string GFunctionPlotter1D::footerData(bool isSecondary, std::size_t pId) const {
	std::ostringstream footer_data;

	std::string comment;
	if(dsMarker_ != "") {
		comment = "// " + dsMarker_;
	}

   std::string functionName = "func1D" + suffix(isSecondary, pId);
	footer_data
		<< "  " << functionName << "->GetXaxis()->SetTitle(\"" << xAxisLabel() << "\");" << std::endl
		<< "  " << functionName << "->GetYaxis()->SetTitle(\"" << yAxisLabel() << "\");" << std::endl
	    << "  " << functionName << "->SetNpx(" << nSamplesX_ << ");" << std::endl;

	if(plot_label_ != "") {
		footer_data
			<< "  " << functionName << "->SetTitle(\"" << plot_label_ << "\");" << std::endl;
	} else {
		footer_data
			<< "  " << functionName << "->SetTitle(\" \");" << std::endl;
	}

	std::string dA = this->drawingArguments(isSecondary);

	footer_data
		<< "  " << functionName << "->Draw(" << dA << ");" << (comment!=""?comment:"") << std::endl
		<< std::endl;

	return footer_data.str();
}

/******************************************************************************/
/**
 * Retrieve the current drawing arguments
 */
std::string GFunctionPlotter1D::drawingArguments(bool isSecondary) const {
   std::string dA = "";

   if(this->drawingArguments_ != "") {
      dA = this->drawingArguments_;
   }

   if(isSecondary) {
      if("" == dA) {
         dA = "same";
      } else {
         dA = dA + ",same";
      }
   }

   return dA;
}

/******************************************************************************/
/**
 * Retrieve a clone of this object
 */
boost::shared_ptr<GBasePlotter> GFunctionPlotter1D::clone() const {
   return boost::shared_ptr<GBasePlotter>(new GFunctionPlotter1D(*this));
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
		const std::string& fD
		, const boost::tuple<double,double>& xExtremes
		, const boost::tuple<double,double>& yExtremes
)
	: functionDescription_(fD)
	, xExtremes_(xExtremes)
	, yExtremes_(yExtremes)
	, nSamplesX_(DEFNSAMPLES)
	, nSamplesY_(DEFNSAMPLES)
{ /* nothing */ }

/******************************************************************************/
/**
 * A copy constructor
 *
 * @param cp A reference to another GFunctionPlotter2D object
 */
GFunctionPlotter2D::GFunctionPlotter2D(const GFunctionPlotter2D& cp)
	: GBasePlotter(cp)
	, functionDescription_(cp.functionDescription_)
	, xExtremes_(cp.xExtremes_)
	, yExtremes_(cp.yExtremes_)
	, nSamplesX_(cp.nSamplesX_)
	, nSamplesY_(cp.nSamplesY_)
{ /* nothing */ }

/******************************************************************************/
/**
 * The destructor
 */
GFunctionPlotter2D::~GFunctionPlotter2D()
{ /* nothing */ }

/******************************************************************************/
/**
 * The assignment operator
 *
 * @param cp A reference to another GFunctionPlotter2D object
 */
const GFunctionPlotter2D & GFunctionPlotter2D::operator=(const GFunctionPlotter2D& cp) {
	// Copy our parent class'es data
	GBasePlotter::operator=(cp);

	// and then our local data
	functionDescription_ = cp.functionDescription_;
	xExtremes_ = cp.xExtremes_;
	yExtremes_ = cp.yExtremes_;
	nSamplesX_ = cp.nSamplesX_;
	nSamplesY_ = cp.nSamplesY_;

	return *this;
}

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
 * Retrieve specific header settings for this plot
 *
 * @return The code to be added to the plot header for this function
 */
std::string GFunctionPlotter2D::headerData(bool isSecondary, std::size_t pId) const {
	// Check the extreme values for consistency
	if(boost::get<0>(xExtremes_) >= boost::get<1>(xExtremes_)) {
	   glogger
	   << "In GFunctionPlotter2D::headerData(): Error!" << std::endl
      << "lower boundary(x) >= upper boundary(x): " << boost::get<0>(xExtremes_) << " / " << boost::get<1>(xExtremes_) << std::endl
      << GEXCEPTION;
	}

	if(boost::get<0>(yExtremes_) >= boost::get<1>(yExtremes_)) {
	   glogger
	   << "In GFunctionPlotter2D::headerData(): Error!" << std::endl
      << "lower boundary(y) >= upper boundary(y): " << boost::get<0>(yExtremes_) << " / " << boost::get<1>(yExtremes_) << std::endl
      << GEXCEPTION;
	}

	std::ostringstream result;

	std::string comment;
	if(dsMarker_ != "") {
		comment = "// " + dsMarker_;
	}

   std::string functionName = "func2D" + suffix(isSecondary, pId);
	result
		<< "  TF2 *"
		<< functionName
		<< " = new TF2(\""
		<< functionName << "\", \""
		<< functionDescription_
		<< "\","
		<< boost::get<0>(xExtremes_)
		<< ", "
		<< boost::get<1>(xExtremes_)
		<< ", "
		<< boost::get<0>(yExtremes_)
		<< ", "
		<< boost::get<1>(yExtremes_)
		<< ");"
		<< (comment!=""?comment:"")
		<< std::endl;

	return result.str();
}

/******************************************************************************/
/**
 * Retrieves the actual data sets
 *
 * @return The code to be added to the plot's data section for this function
 */
std::string GFunctionPlotter2D::bodyData(bool isSecondary, std::size_t pId) const {
	// No data needs to be added for a function plotter
	return std::string();
}

/******************************************************************************/
/**
 * Retrieves specific draw commands for this plot
 *
 * @return The draw command to be added to the plot's data for this function
 */
std::string GFunctionPlotter2D::footerData(bool isSecondary, std::size_t pId) const {
	std::ostringstream footer_data;

	std::string comment;
	if(dsMarker_ != "") {
		comment = "// " + dsMarker_;
	}

	std::string functionName = "func2D" + suffix(isSecondary, pId);
	footer_data
		<< "  " << functionName << "->GetXaxis()->SetTitle(\"" << xAxisLabel() << "\");" << std::endl
		<< "  " << functionName << "->GetYaxis()->SetTitle(\"" << yAxisLabel() << "\");" << std::endl
		<< "  " << functionName << "->GetZaxis()->SetTitle(\"" << zAxisLabel() << "\");" << std::endl
	    << "  " << functionName << "->SetNpx(" << nSamplesX_ << ");" << std::endl
	    << "  " << functionName << "->SetNpy(" << nSamplesY_ << ");" << std::endl;

	if(plot_label_ != "") {
		footer_data
			<< "  " << functionName << "->SetTitle(\"" << plot_label_ << "\");" << std::endl;
	} else {
		footer_data
			<< "  " << functionName << "->SetTitle(\" \");" << std::endl;
	}

	std::string dA = this->drawingArguments(isSecondary);

	footer_data
		<< "  " << functionName << "->Draw(" << dA << ");" << (comment!=""?comment:"") << std::endl
		<< std::endl;

	return footer_data.str();
}

/******************************************************************************/
/**
 * Retrieve the current drawing arguments
 */
std::string GFunctionPlotter2D::drawingArguments(bool isSecondary) const {
   std::string dA = "";

   if(isSecondary) {
      if("" == dA) {
         dA = "same";
      } else {
         dA = dA + ",same";
      }
   }

   return dA;
}

/******************************************************************************/
/**
 * Retrieve a clone of this object
 */
boost::shared_ptr<GBasePlotter> GFunctionPlotter2D::clone() const {
   return boost::shared_ptr<GBasePlotter>(new GFunctionPlotter2D(*this));
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * The default constructor
 */
GFreeFormPlotter::GFreeFormPlotter()
	: headerData_("")
	, bodyData_("")
	, footerData_("")
{ /* nothing */ }

/******************************************************************************/
/**
 * A copy constructor
 *
 * @param cp A copy of another GFreeFormPlotter object
 */
GFreeFormPlotter::GFreeFormPlotter(const GFreeFormPlotter& cp)
	: GBasePlotter(cp)
	, headerData_(cp.headerData_)
	, bodyData_(cp.bodyData_)
	, footerData_(cp.footerData_)
{ /* nothing */ }

/******************************************************************************/
/**
 * The destructor
 */
GFreeFormPlotter::~GFreeFormPlotter()
{ /* nothing */ }

/******************************************************************************/
/**
 * The assignment operator
 *
 * @param cp A copy of another GFreeFormPlotter object
 */
const GFreeFormPlotter& GFreeFormPlotter::operator=(const GFreeFormPlotter& cp)
{
	// Copy our parent class'es data ...
	GBasePlotter::operator=(cp);

	// ... and then our local data
	headerData_ = cp.headerData_;
	bodyData_ = cp.bodyData_;
	footerData_ = cp.footerData_;

	// We do not copy the free form functions

	return *this;
}

/******************************************************************************/
/**
 * Retrieves a unique name for this plotter
 */
std::string GFreeFormPlotter::getPlotterName() const {
   return "GFreeFormPlotter";
}

/******************************************************************************/
/**
 * Retrieve specific header settings for this plot
 *
 * @return A string holding the header data
 */
std::string GFreeFormPlotter::headerData(bool isSecondary, std::size_t pId) const {
   if(headerFunction_) {
      return headerFunction_(isSecondary, pId);
   }
   else {
      return headerData_;
   }
}

/******************************************************************************/
/**
 * Retrieves the actual data sets
 *
 * @return A string holding the body data
 */
std::string GFreeFormPlotter::bodyData(bool isSecondary, std::size_t pId) const {
   if(bodyFunction_) {
      return bodyFunction_(isSecondary, pId);
   }
   else {
      return bodyData_;
   }
}

/******************************************************************************/
/**
 * Retrieves specific draw commands for this plot
 *
 * @return A string holding the footer data
 */
std::string GFreeFormPlotter::footerData(bool isSecondary, std::size_t pId) const {
   if(footerFunction_) {
      return footerFunction_(isSecondary, pId);
   }
   else {
      return footerData_;
   }
}

/******************************************************************************/
/**
 * Adds a string with header data
 */
void GFreeFormPlotter::setHeaderData(const std::string& hD) {
	headerData_ = hD;
}

/******************************************************************************/
/**
 * Adds a string with body data
 */
void GFreeFormPlotter::setBodyData(const std::string& bD){
	bodyData_ = bD;
}

/******************************************************************************/
/**
 * Adds a string with footer data
 */
void GFreeFormPlotter::setFooterData(const std::string& fD) {
	footerData_ = fD;
}

/******************************************************************************/
/**
 * Registers a function that returns the desired header data
 */
void GFreeFormPlotter::registerHeaderFunction(boost::function<std::string(bool, std::size_t)> hf) {
   if(!hf) {
      glogger
      << "In GFreeFormPlotter::registerHeaderFunction(): Error!" << std::endl
      << "Received empty function" << std::endl
      << GEXCEPTION;
   }

   headerFunction_ = hf;
}

/******************************************************************************/
/**
 * Registers a function that returns the desired body data
 */
void GFreeFormPlotter::registerBodyFunction(boost::function<std::string(bool, std::size_t)> bf) {
   if(!bf) {
      glogger
      << "In GFreeFormPlotter::registerBodyFunction(): Error!" << std::endl
      << "Received empty function" << std::endl
      << GEXCEPTION;
   }

   bodyFunction_ = bf;
}

/******************************************************************************/
/**
 * Registers a function that returns the desired footer data
 */
void GFreeFormPlotter::registerFooterFunction(boost::function<std::string(bool, std::size_t)> ff) {
   if(!ff) {
      glogger
      << "In GFreeFormPlotter::registerFooterFunction(): Error!" << std::endl
      << "Received empty function" << std::endl
      << GEXCEPTION;
   }

   footerFunction_ = ff;
}

/******************************************************************************/
/**
 * Retrieve the current drawing arguments
 */
std::string GFreeFormPlotter::drawingArguments(bool isSecondary) const {
   std::string dA = "";

   // nothing

   return dA;
}

/******************************************************************************/
/**
 * Retrieve a clone of this object
 */
boost::shared_ptr<GBasePlotter> GFreeFormPlotter::clone() const {
   return boost::shared_ptr<GBasePlotter>(new GFreeFormPlotter(*this));
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * The standard constructor
 *
 * @param The label of the canvas
 * @param c_x_div The number of plots in x-direction
 * @param c_y_div The number of plots in y-direction
 */
GPlotDesigner::GPlotDesigner(
	const std::string& canvasLabel
	, const std::size_t& c_x_div
	, const std::size_t& c_y_div
)
	: c_x_div_(c_x_div)
	, c_y_div_(c_y_div)
	, c_x_dim_(DEFCXDIM)
	, c_y_dim_(DEFCYDIM)
	, canvasLabel_(canvasLabel)
{ /* nothing */ }

/******************************************************************************/
/**
 * Writes the plot to a file
 *
 * @param fileName The name of the file to which the data can be written
 */
void GPlotDesigner::writeToFile(const std::string& fileName) {
	std::ofstream result(fileName.c_str());
	result << plot();
	result.close();
}

/******************************************************************************/
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
		<<    staticHeader()
		<< std::endl;

	// Plot all body sections up to the maximum allowed number
	result << "  //===================  Header Section ====================" << std::endl
		   << std::endl;

	// Plot all headers up to the maximum allowed number
	std::size_t nPlots = 0;
	std::vector<boost::shared_ptr<GBasePlotter> >::const_iterator it;
	for(it=plotters_.begin(); it!=plotters_.end(); ++it) {
		if(nPlots++ < maxPlots) {
			result << (*it)->headerData_()  << std::endl;
		}
	}

	// Plot all body sections up to the maximum allowed number
	result << "  //===================  Data Section ======================" << std::endl
		   << std::endl;

	nPlots = 0;
	for(it=plotters_.begin(); it!=plotters_.end(); ++it) {
		if(nPlots++ < maxPlots) {
			result << (*it)->bodyData_() << std::endl;
		}
	}

	// Plot all footer data up to the maximum allowed number
	result << "  //===================  Plot Section ======================" << std::endl
		   << std::endl;

	nPlots = 0;
	for(it=plotters_.begin(); it!=plotters_.end(); ++it) {
		if(nPlots < maxPlots) {
			result
				<< "  graphPad->cd(" << nPlots+1 << ");" << std::endl /* cd starts at 1 */
				<< (*it)->footerData_()	<< std::endl;

			nPlots++;
		}
	}

	result
	    << "  graphPad->cd();" << std::endl
		<< "  cc->cd();" << std::endl
		<< "}" << std::endl;

	return result.str();
}

/******************************************************************************/
/**
 * A default header for a ROOT file
 */
std::string GPlotDesigner::staticHeader() const {
	std::ostringstream result;

	result
	  << "  gROOT->Reset();" << std::endl
	  << "  gStyle->SetCanvasColor(0);" << std::endl
	  << "  gStyle->SetStatBorderSize(1);" << std::endl
	  << "  gStyle->SetOptStat(0);" << std::endl
	  << std::endl
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

/******************************************************************************/
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
	   glogger
	   << "GPlotDesigner::registerPlotter(): Error!" << std::endl
      << "Got empty plotter" << std::endl
      << GEXCEPTION;
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
	const boost::uint32_t& c_x_dim
	, const boost::uint32_t& c_y_dim
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
   const boost::tuple<boost::uint32_t, boost::uint32_t>& c_dim
) {
   this->setCanvasDimensions(boost::get<0>(c_dim), boost::get<1>(c_dim));
}

/******************************************************************************/
/**
 * Allows to retrieve the canvas dimensions
 *
 * @return A boost::tuple holding the canvas dimensions
 */
boost::tuple<boost::uint32_t, boost::uint32_t> GPlotDesigner::getCanvasDimensions() const {
	return boost::tuple<boost::uint32_t, boost::uint32_t>(c_x_dim_, c_y_dim_);
}

/******************************************************************************/
/**
 * Allows to set the canvas label
 */
void GPlotDesigner::setCanvasLabel(const std::string& canvasLabel) {
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
 * Resets the plotters
 */
void GPlotDesigner::resetPlotters() {
   plotters_.clear();
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Common */
} /* namespace Gem */
