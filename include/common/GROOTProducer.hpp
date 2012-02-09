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

// Standard header files go here
#include <string>
#include <vector>
#include <fstream>
#include <sstream>

// Boost headers go here
#include <boost/utility.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/cstdint.hpp>
#include <boost/cast.hpp>
#include <boost/lexical_cast.hpp>

#ifndef GROOTPRODUCER_HPP_
#define GROOTPRODUCER_HPP_

// Geneva headers go here
#include "common/GExceptions.hpp"

namespace Gem {
namespace Common {

/**
 * Determines whether a scatter plot or a curve should be recorded
 */
enum plotMode {
	SCATTER = 0
	, CURVE = 1
};

/**
 * Some default values
 */
const boost::uint32_t DEFCXDIM = 1024;
const boost::uint32_t DEFCYDIM =  768;
const boost::uint32_t DEFCXDIV =    1;
const boost::uint32_t DEFCYDIV =    1;
const plotMode        DEFPLOTMODE = CURVE;

/*********************************************************************************/
/**
 * A class that facilitates the creation of simple ROOT plots (cmp. http://root.cern.ch).
 */
class GROOTProducer
	:boost::noncopyable
{
public:
	/** @brief The standard constructor */
	GROOTProducer(const boost::uint32_t&, const boost::uint32_t&);

	/** @brief Set the dimensions of the output canvas */
	void setCanvasDimensions(const boost::uint32_t&, const boost::uint32_t&);
	/** @brief Allows to retrieve the canvas dimensions */
	boost::tuple<boost::uint32_t, boost::uint32_t> getCanvasDimensions() const;

	/** @brief Set the divisions of the output canvas */
	void setCanvasDivisions(const boost::uint32_t&, const boost::uint32_t&);
	/** @brief Allows to retrieve the canvas divisions */
	boost::tuple<boost::uint32_t, boost::uint32_t> getCanvasDivision() const;

	/** @brief Assign a name to the entire canvas */
	void setCanvasName(const std::string&);

	/** @brief Determines whether a scatter plot or a curve is created */
	void setPlotMode(const plotMode&);
	/** @brief Allows to retrieve the current plotting mode */
	plotMode getPlotMode() const;

	/** @brief Marks the current sub-canvas as complete and switches to the next drawing area */
	bool completeSubCanvas(
		const std::string& = ""
		, const std::string& = ""
		, const std::string& = ""
	);

	/** @brief Allows to add an external plot to the canvas */
	void addExternalPlot(
			const std::string&   // header
			, const std::string& // data section
			, const std::string& // footer
	);

	/** @brief Retrieves the resulting plot as a std::string */
	std::string getResult() const;

	/** @brief Writes out the final root file */
	void writeResult(const std::string&) const;

	/*****************************************************************************/
	/**
	 * Add data to the plot
	 *
	 * @param point A boost::tuple holding the x- and y-coordinates of a data entry
	 */
	template <typename x_type, typename y_type>
	GROOTProducer& operator&(const boost::tuple<x_type, y_type>& point) {
		// Do nothing if we have gone beyond the last drawing area
		if(plotComplete_) {
			return *this;
		}

		double x=0., y=0.;

		// Make sure the data can be converted to doubles
		try {
			x=boost::lexical_cast<double>(boost::get<0>(point));
			y=boost::lexical_cast<double>(boost::get<1>(point));
		}
		catch(boost::bad_lexical_cast &e) {
			raiseException(
				"In GROOTProducer::operator&(): Error!" << std::endl
				<< "Encountered invalid conversion with boost::lexical_cast," << std::endl
				<< "with the message " << std::endl
				<< e.what() << std::endl
			);
		}

		// Add the data
		measuredData_.push_back(boost::tuple<double, double>(x,y));

		return *this;
	}

private:
	/*****************************************************************************/
	/** @brief A header for static data in a ROOT file */
	std::string getStaticHeader() const;

	GROOTProducer(); ///< The default constructor -- intentionally private and undefined

	/*****************************************************************************/
	// Private data

	std::vector<boost::tuple<double, double> > measuredData_; ///< Holds the data tuples

	boost::uint32_t c_x_dim_, c_y_dim_; ///< Holds the dimensions of the canvas
	boost::uint32_t c_x_div_, c_y_div_; ///< The number of divisions in x- and y-direction
	plotMode pM_; ///< Whether to create scatter plots or a curve, connected by lines

	std::size_t currentSubCanvas_; ///< The current canvas to be drawn to
	bool plotComplete_; ///< Indicates whether the last canvas has been completed

	std::ostringstream headerData_; ///< Dynamic data to be added to the header
	std::ostringstream tupleData_;  ///< The tuple data
	std::ostringstream footerData_; ///< Dynamic data to be added to the footer

	std::string canvasName_; ///< A name to be assigned to the entire canvas
};

/*********************************************************************************/

} /* namespace Common */
} /* namespace Gem */

#endif /* GROOTPRODUCER_HPP_ */
