/**
 * @file GPlotDesigner.hpp
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
#include <iostream>
#include <sstream>

// Boost headers go here
#include <boost/utility.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/cstdint.hpp>
#include <boost/cast.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/shared_ptr.hpp>

#ifndef GPLOTDESIGNER_HPP_
#define GPLOTDESIGNER_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif

// Geneva headers go here
#include "common/GExceptions.hpp"

namespace Gem {
namespace Common {

/*********************************************************************************/
/**
 * Determines whether a scatter plot or a curve should be recorded
 */
enum graphPlotMode {
	SCATTER = 0
	, CURVE = 1
};

//Some default values

const boost::uint32_t DEFCXDIM    = 1024;
const boost::uint32_t DEFCYDIM    =  768;

const boost::uint32_t DEFCXDIV    =    1;
const boost::uint32_t DEFCYDIV    =    1;

const std::size_t     DEFNSAMPLES = 100;

const graphPlotMode        DEFPLOTMODE = CURVE;

// Easier access to the header-, body- and footer-data
typedef boost::tuple<std::string,std::string,std::string> plotData;

// Forward declaration in order to allow a friend statement in GBasePlotter
class GPlotDesigner;

/*********************************************************************************/
/**
 * An abstract base class that defines functions for plots. Concrete plotters
 * derive from this class. They can be added to a master canvas, which takes care
 * to plot them into sub-pads.
 */
class GBasePlotter {
	friend class GPlotDesigner;

public:
	/** @brief The default constructor */
	GBasePlotter();
	/** @brief A copy constructor */
	GBasePlotter(const GBasePlotter&);
	/** @brief The destructor */
	virtual ~GBasePlotter();

	/** @brief The assignment operator */
	void operator=(const GBasePlotter&);

	//----------------------------------------------------------------------------
	// Functions to be specified in derived classes

	/** @brief Retrieve specific header settings for this plot */
	virtual std::string headerData() const = 0;
	/** @brief Retrieves the actual data sets */
	virtual std::string bodyData() const = 0;
	/** @brief Retrieves specific draw commands for this plot */
	virtual std::string footerData() const = 0;

	//----------------------------------------------------------------------------

	/** @brief Allows to set the drawing arguments for this plot */
	virtual void setDrawingArguments(std::string);
	/** @brief Retrieve the current drawing arguments */
	std::string drawingArguments() const;

	/** @brief Sets the label for the x-axis */
	void setXAxisLabel(std::string);
	/** @brief Retrieve the x-axis label */
	std::string xAxisLabel() const;
	/** @brief Sets the label for the y-axis */
	void setYAxisLabel(std::string);
	/** @brief Retrieve the y-axis label */
	std::string yAxisLabel() const;
	/** @brief Sets the label for the z-axis */
	void setZAxisLabel(std::string);
	/** @brief Retrieve the z-axis label */
	std::string zAxisLabel() const;

	/** @brief Allows to assign a label to the entire plot */
	void setPlotLabel(std::string);
	/** @brief Allows to retrieve the plot label */
	std::string plotLabel() const;

	/** @brief Allows to assign a marker to data structures */
	void setDataStructureMarker(std::string);
	/** @brief Allows to retrieve the data structure marker */
	std::string dsMarker() const;

protected:
	std::string drawingArguments_; ///< Holds the drawing arguments for this plot

	std::string x_axis_label_; ///< A label for the x-axis
	std::string y_axis_label_; ///< A label for the y-axis
	std::string z_axis_label_; ///< A label for the z-axis (if available)

	std::string plot_label_;   ///< A label to be assigned to the entire plot
	std::string dsMarker_;     ///< A marker to make the origin of data structures clear in the output file

	/** @brief Allows to retrieve the id of this object */
	std::size_t id() const;

private:
	/** @brief Sets the id of the object */
	void setId(const std::size_t&);

	std::size_t id_; ///< The id of this object
};

/*********************************************************************************/
/**
 * A data collector for 1-d data of user-defined type. This will usually be
 * data of a histogram type.
 */
template <typename x_type>
class GDataCollector1T :public GBasePlotter
{
public:
	/*****************************************************************************/
	/**
	 * The default constructor
	 */
	GDataCollector1T()
		: GBasePlotter()
		, data_()
	{ /* nothing */ }

	/*****************************************************************************/
	/**
	 * A copy constructor
	 *
	 * @param cp A copy of another GDataCollector1T<x_type> object
	 */
	GDataCollector1T(const GDataCollector1T<x_type>& cp)
		: GBasePlotter(cp)
		, data_(cp.data_)
	{ /* nothing */ }

	/*****************************************************************************/
	/**
	 * The destructor
	 */
	virtual ~GDataCollector1T() {
		data_.clear();
	}

	/*****************************************************************************/
	/**
	 * The assignment operator
	 */
	void operator=(const GDataCollector1T<x_type>& cp) {
		// Assign our parent class'es data
		GBasePlotter::operator=(cp);

		// and then our own
		data_ = cp.data_;
	}

	/*****************************************************************************/
	/**
	 * Allows to add data of arbitrary type, provided it can be converted
	 * safely to the target type.
	 *
	 * @param x_undet The data item to be added to the collection
	 */
	template <typename x_type_undet>
	void operator&(const x_type_undet& x_undet) {
		using boost::numeric::bad_numeric_cast;

		x_type x = x_type(0);

		// Make sure the data can be converted to doubles
		try {
			x=boost::numeric_cast<x_type>(x_undet);
		}
		catch(bad_numeric_cast &e) {
			raiseException(
				"In GDataCollector1T::operator&(const T&): Error!" << std::endl
				<< "Encountered invalid cast with boost::numeric_cast," << std::endl
				<< "with the message " << std::endl
				<< e.what() << std::endl
			);
		}

		// Add the converted data to our collection
		data_.push_back(x);
	}

	/*****************************************************************************/
	/**
	 * Allows to add data of type "x_type
	 *
	 * @param x The data item to be added to the collection
	 */
	void operator&(const x_type& x) {
		// Add the data item to our collection
		data_.push_back(x);
	}

	/*****************************************************************************/
	/**
	 * Allows to add a collection of data items of undetermined type in one go,
	 * provided the type can be converted safely into the target type
	 *
	 * @param x_vec_undet A collection of data items of undetermined type, to be added to the collection
	 */
	template <typename x_type_undet>
	void operator&(const std::vector<x_type_undet>& x_vec_undet) {
		using boost::numeric::bad_numeric_cast;

		x_type x = x_type(0);

		typename std::vector<x_type_undet>::const_iterator cit;
		for(cit=x_vec_undet.begin(); cit!=x_vec_undet.end(); ++cit) {
			// Make sure the data can be converted to doubles
			try {
				x=boost::numeric_cast<x_type>(*cit);
			}
			catch(bad_numeric_cast &e) {
				raiseException(
					"In GDataCollector1T::operator&(const std::vector<T>&): Error!" << std::endl
					<< "Encountered invalid cast with boost::numeric_cast," << std::endl
					<< "with the message " << std::endl
					<< e.what() << std::endl
				);
			}

			// Add the converted data to our collection
			data_.push_back(x);
		}
	}

	/*****************************************************************************/
	/**
	 * Allows to add a collection of data items of type x_type to our data_ vector.
	 *
	 * @param x_vec A vector of data items to be added to the data_ vector
	 */
	void operator&(const std::vector<x_type>& x_vec) {
		typename std::vector<x_type>::const_iterator cit;
		for(cit=x_vec.begin(); cit!=x_vec.end(); ++cit) {
			// Add the data item to our collection
			data_.push_back(*cit);
		}
	}

protected:
	/*****************************************************************************/

	std::vector<x_type> data_; ///< Holds the actual data
};

/*********************************************************************************/
/**
 * A data collector for 2-d data of user-defined type
 */
template <typename x_type, typename y_type>
class GDataCollector2T :public GBasePlotter
{
public:
	/*****************************************************************************/
	/**
	 * The default constructor
	 */
	GDataCollector2T()
		: GBasePlotter()
		, data_()
	{ /* nothing */ }

	/*****************************************************************************/
	/**
	 * A copy constructor
	 *
	 * @param cp A copy of another GDataCollector1T<x_type> object
	 */
	GDataCollector2T(const GDataCollector2T<x_type, y_type>& cp)
		: GBasePlotter(cp)
		, data_(cp.data_)
	{ /* nothing */ }

	/*****************************************************************************/
	/**
	 * The destructor
	 */
	virtual ~GDataCollector2T() {
		data_.clear();
	}

	/*****************************************************************************/
	/**
	 * The assignment operator
	 */
	void operator=(const GDataCollector2T<x_type, y_type>& cp) {
		// Assign our parent class'es data
		GBasePlotter::operator=(cp);

		// and then our own
		data_ = cp.data_;
	}

	/*****************************************************************************/
	/**
	 * Allows to add data of undetermined type to the collection in an intuitive way,
	 * provided that it can be converted safely to the target type.
	 *
	 * @param point_undet The data item to be added to the collection
	 */
	template <typename x_type_undet, typename y_type_undet>
	void operator&(const boost::tuple<x_type_undet,y_type_undet>& point_undet) {
		using boost::numeric::bad_numeric_cast;

		x_type x = x_type(0);
		y_type y = y_type(0);

		// Make sure the data can be converted to doubles
		try {
			x=boost::numeric_cast<x_type>(boost::get<0>(point_undet));
			y=boost::numeric_cast<y_type>(boost::get<1>(point_undet));
		}
		catch(bad_numeric_cast &e) {
			raiseException(
				"In GDataCollector2T::operator&(const boost::tuple<S,T>&): Error!" << std::endl
				<< "Encountered invalid cast with boost::numeric_cast," << std::endl
				<< "with the message " << std::endl
				<< e.what() << std::endl
			);
		}

		data_.push_back(boost::tuple<x_type,y_type>(x,y));
	}

	/*****************************************************************************/
	/**
	 * Allows to add data of type boost::tuple<x_type, x_type> to the collection in
	 * an intuitive way.
	 *
	 * @param point The data item to be added to the collection
	 */
	void operator&(const boost::tuple<x_type,y_type>& point) {
		// Add the data item to the collection
		data_.push_back(point);
	}

	/*****************************************************************************/
	/**
	 * Allows to add a collection of data items of undetermined type to the
	 * collection in an intuitive way, provided they can be converted safely
	 * to the target type.
	 *
	 * @param point_vec_undet The collection of data items to be added to the collection
	 */
	template <typename x_type_undet, typename y_type_undet>
	void operator&(const std::vector<boost::tuple<x_type_undet,y_type_undet> >& point_vec_undet) {
		using boost::numeric::bad_numeric_cast;

		x_type x = x_type(0);
		y_type y = y_type(0);

		typename std::vector<boost::tuple<x_type_undet,y_type_undet> >::const_iterator cit;
		for(cit=point_vec_undet.begin(); cit!=point_vec_undet.end(); ++cit) {
			// Make sure the data can be converted to doubles
			try {
				x=boost::numeric_cast<x_type>(boost::get<0>(*cit));
				y=boost::numeric_cast<y_type>(boost::get<1>(*cit));
			}
			catch(bad_numeric_cast &e) {
				raiseException(
					"In GDataCollector2T::operator&(const std::vector<boost::tuple<S,T> >&): Error!" << std::endl
					<< "Encountered invalid cast with boost::numeric_cast," << std::endl
					<< "with the message " << std::endl
					<< e.what() << std::endl
				);
			}

			data_.push_back(boost::tuple<x_type,y_type>(x,y));
		}
	}

	/*****************************************************************************/
	/**
	 * Allows to add a collection of data items of type boost::tuple<x_type, y_type>
	 * to the collection in an intuitive way, provided they can be converted safely
	 * to the target type.
	 *
	 * @param point_vec The collection of data items to be added to the collection
	 */
	void operator&(const std::vector<boost::tuple<x_type,y_type> >& point_vec) {
		typename std::vector<boost::tuple<x_type,y_type> >::const_iterator cit;
		for(cit=point_vec.begin(); cit!=point_vec.end(); ++cit) {
			// Add the data item to the collection
			data_.push_back(*cit);
		}
	}

protected:
	/*****************************************************************************/

	std::vector<boost::tuple<x_type, y_type> > data_; ///< Holds the actual data
};

/*********************************************************************************/
/**
 * A data collector for 2-d data of user-defined type, with the ability to
 * additionally specify an error component for both dimensions.
 */
template <typename x_type, typename y_type>
class GDataCollector2ET :public GBasePlotter
{
public:
	/*****************************************************************************/
	/**
	 * The default constructor
	 */
	GDataCollector2ET()
		: GBasePlotter()
		, data_()
	{ /* nothing */ }

	/*****************************************************************************/
	/**
	 * A copy constructor
	 *
	 * @param cp A copy of another GDataCollector1T<x_type> object
	 */
	GDataCollector2ET(const GDataCollector2ET<x_type, y_type>& cp)
		: GBasePlotter(cp)
		, data_(cp.data_)
	{ /* nothing */ }

	/*****************************************************************************/
	/**
	 * The destructor
	 */
	virtual ~GDataCollector2ET() {
		data_.clear();
	}

	/*****************************************************************************/
	/**
	 * The assignment operator
	 */
	void operator=(const GDataCollector2ET<x_type, y_type>& cp) {
		// Assign our parent class'es data
		GBasePlotter::operator=(cp);

		// and then our own
		data_ = cp.data_;
	}

	/*****************************************************************************/
	/**
	 * Allows to add data of undetermined type to the collection in an intuitive way,
	 * provided that it can be converted safely to the target type.
	 *
	 * @param point_undet The data item to be added to the collection
	 */
	template <typename x_type_undet, typename y_type_undet>
	void operator&(const boost::tuple<x_type_undet, x_type_undet, y_type_undet, y_type_undet>& point_undet) {
		using boost::numeric::bad_numeric_cast;

		x_type x  = x_type(0);
		x_type ex = x_type(0);
		y_type y  = y_type(0);
		y_type ey = y_type(0);

		// Make sure the data can be converted to doubles
		try {
			x =boost::numeric_cast<x_type>(boost::get<0>(point_undet));
			ex=boost::numeric_cast<x_type>(boost::get<1>(point_undet));
			y =boost::numeric_cast<y_type>(boost::get<2>(point_undet));
			ey=boost::numeric_cast<y_type>(boost::get<3>(point_undet));
		}
		catch(bad_numeric_cast &e) {
			raiseException(
				"In GDataCollector2ET::operator&(const boost::tuple<S,S,T,T>&): Error!" << std::endl
				<< "Encountered invalid cast with boost::numeric_cast," << std::endl
				<< "with the message " << std::endl
				<< e.what() << std::endl
			);
		}

		data_.push_back(boost::tuple<x_type,x_type,y_type,y_type>(x,ex,y,ey));
	}

	/*****************************************************************************/
	/**
	 * Allows to add data of type boost::tuple<x_type, x_type> to the collection in
	 * an intuitive way.
	 *
	 * @param point The data item to be added to the collection
	 */
	void operator&(const boost::tuple<x_type,x_type,y_type,y_type>& point) {
		// Add the data item to the collection
		data_.push_back(point);
	}

	/*****************************************************************************/
	/**
	 * Allows to add a collection of data items of undetermined type to the
	 * collection in an intuitive way, provided they can be converted safely
	 * to the target type.
	 *
	 * @param point_vec_undet The collection of data items to be added to the collection
	 */
	template <typename x_type_undet, typename y_type_undet>
	void operator&(const std::vector<boost::tuple<x_type_undet,x_type_undet,y_type_undet,y_type_undet> >& point_vec_undet) {
		using boost::numeric::bad_numeric_cast;

		x_type x  = x_type(0);
		x_type ex = x_type(0);
		y_type y  = y_type(0);
		y_type ey = y_type(0);

		typename std::vector<boost::tuple<x_type_undet,x_type_undet,y_type_undet,y_type_undet> >::const_iterator cit;
		for(cit=point_vec_undet.begin(); cit!=point_vec_undet.end(); ++cit) {
			// Make sure the data can be converted to doubles
			try {
				x  = boost::numeric_cast<x_type>(boost::get<0>(*cit));
				ex = boost::numeric_cast<x_type>(boost::get<1>(*cit));
				y  = boost::numeric_cast<y_type>(boost::get<2>(*cit));
				ey = boost::numeric_cast<y_type>(boost::get<3>(*cit));
			}
			catch(bad_numeric_cast &e) {
				raiseException(
					"In GDataCollector2ET::operator&(const std::vector<boost::tuple<S,S,T,T> >&): Error!" << std::endl
					<< "Encountered invalid cast with boost::numeric_cast," << std::endl
					<< "with the message " << std::endl
					<< e.what() << std::endl
				);
			}

			data_.push_back(boost::tuple<x_type,x_type,y_type,y_type>(x,ex,y,ey));
		}
	}

	/*****************************************************************************/
	/**
	 * Allows to add a collection of data items of type boost::tuple<x_type, x_type, y_type, y_type>
	 * to the collection in an intuitive way, provided they can be converted safely
	 * to the target type.
	 *
	 * @param point_vec The collection of data items to be added to the collection
	 */
	void operator&(const std::vector<boost::tuple<x_type,x_type,y_type,y_type> >& point_vec) {
		typename std::vector<boost::tuple<x_type,x_type,y_type,y_type> >::const_iterator cit;
		for(cit=point_vec.begin(); cit!=point_vec.end(); ++cit) {
			// Add the data item to the collection
			data_.push_back(*cit);
		}
	}

protected:
	/*****************************************************************************/

	std::vector<boost::tuple<x_type, x_type, y_type, y_type> > data_; ///< Holds the actual data
};

/*********************************************************************************/
/**
 * A wrapper for the ROOT TGraph class (2d data and curve-like structures)
 */
class GGraph2D : public GDataCollector2T<double,double> {
public:
	/** @brief The default constructor */
	GGraph2D();

	/** @brief A copy constructor */
	GGraph2D(const GGraph2D&);

	/** @brief The destructor */
	~GGraph2D();

	/** @brief The assignment operator */
	const GGraph2D &operator=(const GGraph2D&);

	/** @brief Determines whether a scatter plot or a curve is created */
	void setPlotMode(graphPlotMode);
	/** @brief Allows to retrieve the current plotting mode */
	graphPlotMode getPlotMode() const;

	/** @brief Retrieve specific header settings for this plot */
	virtual std::string headerData() const;
	/** @brief Retrieves the actual data sets */
	virtual std::string bodyData() const;
	/** @brief Retrieves specific draw commands for this plot */
	virtual std::string footerData() const;

private:
	graphPlotMode pM_; ///< Whether to create scatter plots or a curve, connected by lines
};

/*********************************************************************************/
/**
 * A wrapper for the ROOT TGraphErrors class (2d data and curve-like structures)
 */
class GGraph2ED : public GDataCollector2ET<double,double> {
public:
	/** @brief The default constructor */
	GGraph2ED();

	/** @brief A copy constructor */
	GGraph2ED(const GGraph2ED&);

	/** @brief The destructor */
	~GGraph2ED();

	/** @brief The assignment operator */
	const GGraph2ED &operator=(const GGraph2ED&);

	/** @brief Determines whether a scatter plot or a curve is created */
	void setPlotMode(graphPlotMode);
	/** @brief Allows to retrieve the current plotting mode */
	graphPlotMode getPlotMode() const;

	/** @brief Retrieve specific header settings for this plot */
	virtual std::string headerData() const;
	/** @brief Retrieves the actual data sets */
	virtual std::string bodyData() const;
	/** @brief Retrieves specific draw commands for this plot */
	virtual std::string footerData() const;

private:
	graphPlotMode pM_; ///< Whether to create scatter plots or a curve, connected by lines
};

/*********************************************************************************/
/**
 * A wrapper for ROOT's TH1D class (1-d double data)
 */
class GHistogram1D : public GDataCollector1T<double> {
public:
	/** @brief The default constructor */
	GHistogram1D(
		const std::size_t&
		, const double&
		, const double&
	);
	/** @brief A copy constructor */
	GHistogram1D(const GHistogram1D&);

	/** @brief The destructor */
	~GHistogram1D();

	/** @brief The assignment operator */
	const GHistogram1D &operator=(const GHistogram1D&);

	/** @brief Retrieve specific header settings for this plot */
	virtual std::string headerData() const;
	/** @brief Retrieves the actual data sets */
	virtual std::string bodyData() const;
	/** @brief Retrieves specific draw commands for this plot */
	virtual std::string footerData() const;

	/** @brief Retrieve the number of bins in x-direction */
	std::size_t getNBinsX() const;

	/** @brief Retrieve the lower boundary of the plot */
	double getMinX() const;
	/** @brief Retrieve the upper boundary of the plot */
	double getMaxX() const;

private:
	GHistogram1D(); ///< The default constructor -- intentionally private and undefined

	std::size_t nBinsX_; ///< The number of bins in the histogram

	double minX_; ///< The lower boundary of the histogram
	double maxX_; ///< The upper boundary of the histogram
};

/*********************************************************************************/
/**
 * An enum for 2D-drawing options
 */
enum tddropt {
	TDEMPTY = 0
	, SURFONE = 1
	, SURFTWOZ = 2
	, SURFTHREE = 3
	, SURFFOUR = 4
	, CONTZ = 5
	, CONTONE = 6
	, CONTTWO = 7
	, CONTTHREE = 8
	, TEXT = 9
	, SCAT = 10
	, BOX = 11
	, ARR = 12
	, COLZ = 13
	, LEGO = 14
	, LEGOONE = 15
	, SURFONEPOL = 16
	, SURFONECYL = 17
};

/*********************************************************************************/
/**
 * A wrapper for ROOT's TH2D class (2-d double data)
 */
class GHistogram2D : public GDataCollector2T<double, double> {
public:
	/** @brief The default constructor */
	GHistogram2D(
		const std::size_t&
		, const std::size_t&
		, const double&
		, const double&
		, const double&
		, const double&
	);
	/** @brief A copy constructor */
	GHistogram2D(const GHistogram2D&);

	/** @brief The destructor */
	~GHistogram2D();

	/** @brief The assignment operator */
	const GHistogram2D &operator=(const GHistogram2D&);

	/** @brief Retrieve specific header settings for this plot */
	virtual std::string headerData() const;
	/** @brief Retrieves the actual data sets */
	virtual std::string bodyData() const;
	/** @brief Retrieves specific draw commands for this plot */
	virtual std::string footerData() const;

	/** @brief Retrieve the number of bins in x-direction */
	std::size_t getNBinsX() const;
	/** @brief Retrieve the number of bins in y-direction */
	std::size_t getNBinsY() const;

	/** @brief Retrieve the lower boundary of the plot in x-direction */
	double getMinX() const;
	/** @brief Retrieve the upper boundary of the plot in x-direction */
	double getMaxX() const;
	/** @brief Retrieve the lower boundary of the plot in y-direction */
	double getMinY() const;
	/** @brief Retrieve the upper boundary of the plot in y-direction */
	double getMaxY() const;

	/** @brief Allows to specify 2d-drawing options */
	void set2DOpt(tddropt);
	/** @brief Allows to retrieve 2d-drawing options */
	tddropt get2DOpt() const;


private:
	GHistogram2D(); ///< The default constructor -- intentionally private and undefined

	std::size_t nBinsX_; ///< The number of bins in the x-direction of the histogram
	std::size_t nBinsY_; ///< The number of bins in the y-direction of the histogram

	double minX_; ///< The lower boundary of the histogram in x-direction
	double maxX_; ///< The upper boundary of the histogram in x-direction
	double minY_; ///< The lower boundary of the histogram in y-direction
	double maxY_; ///< The upper boundary of the histogram in y-direction

	tddropt dropt_; ///< The drawing options for 2-d histograms
};

/*********************************************************************************/
/**
 * A wrapper for the ROOT TF1 1d-function plotter
 */
class GFunctionPlotter1D
	: public GBasePlotter
{
public:
	/** @brief The standard constructor */
	GFunctionPlotter1D(
			const std::string&
			, const boost::tuple<double,double>&
	);

	/** @brief A copy constructor */
	GFunctionPlotter1D(const GFunctionPlotter1D&);

	/** @brief The destructor */
	~GFunctionPlotter1D();

	/** @brief The assignment operator */
	const GFunctionPlotter1D &operator=(const GFunctionPlotter1D&);

	/** @brief Allows to set the number of sampling points in x-direction */
	void setNSamplesX(std::size_t);

	/** @brief Retrieve specific header settings for this plot */
	virtual std::string headerData() const;
	/** @brief Retrieves the actual data sets */
	virtual std::string bodyData() const;
	/** @brief Retrieves specific draw commands for this plot */
	virtual std::string footerData() const;

private:
	GFunctionPlotter1D(); ///< The default constructor -- intentionally private and undefined

	std::string functionDescription_;

	boost::tuple<double,double> xExtremes_; ///< Minimum and maximum values for the x-axis
	std::size_t nSamplesX_; ///< The number of sampling points of the function
};

/*********************************************************************************/
/**
 * A wrapper for the ROOT TF2 2d-function plotter
 */
class GFunctionPlotter2D
	: public GBasePlotter
{
public:
	/** @brief The standard constructor */
	GFunctionPlotter2D(
			const std::string&
			, const boost::tuple<double,double>&
			, const boost::tuple<double,double>&
	);

	/** @brief A copy constructor */
	GFunctionPlotter2D(const GFunctionPlotter2D&);

	/** @brief The destructor */
	~GFunctionPlotter2D();

	/** @brief The assignment operator */
	const GFunctionPlotter2D &operator=(const GFunctionPlotter2D&);

	/** @brief Allows to set the number of sampling points in x-direction */
	void setNSamplesX(std::size_t);
	/** @brief Allows to set the number of sampling points in y-direction */
	void setNSamplesY(std::size_t);

	/** @brief Retrieve specific header settings for this plot */
	virtual std::string headerData() const;
	/** @brief Retrieves the actual data sets */
	virtual std::string bodyData() const;
	/** @brief Retrieves specific draw commands for this plot */
	virtual std::string footerData() const;

private:
	GFunctionPlotter2D(); ///< The default constructor -- intentionally private and undefined

	std::string functionDescription_;

	boost::tuple<double,double> xExtremes_; ///< Minimum and maximum values for the x-axis
	boost::tuple<double,double> yExtremes_; ///< Minimum and maximum values for the y-axis

	std::size_t nSamplesX_; ///< The number of sampling points of the function
	std::size_t nSamplesY_; ///< The number of sampling points of the function
};


/*********************************************************************************/
/**
 * This class allows to add free-form root-data to the master plot
 */
class GFreeFormPlotter : public GBasePlotter
{
public:
	/** @brief The default constructor */
	GFreeFormPlotter();
	/** @brief A copy constructor */
	GFreeFormPlotter(const GFreeFormPlotter&);
	/** @brief The destructor */
	virtual ~GFreeFormPlotter();

	/** @brief The assignment operator */
	const GFreeFormPlotter& operator=(const GFreeFormPlotter&);

	/** @brief Retrieve specific header settings for this plot */
	virtual std::string headerData() const;
	/** @brief Retrieves the actual data sets */
	virtual std::string bodyData() const;
	/** @brief Retrieves specific draw commands for this plot */
	virtual std::string footerData() const;

	void setHeaderData(const std::string&);
	void setBodyData(const std::string&);
	void setFooterData(const std::string&);

private:
	std::string headerData_; ///< The data to be written into the master plot's header
	std::string bodyData_; ///< The data to be written into the master plot's body
	std::string footerData_; ///< The data to be written into the master plot's footer
};

/*********************************************************************************/
/**
 * A class that outputs a ROOT input file (compare http://root.cern.ch), based
 * on the data providers stored in it.
 */
class GPlotDesigner
	: boost::noncopyable
{
public:
	/** @brief The standard constructor */
	GPlotDesigner(
			const std::string&
			, const std::size_t&
			, const std::size_t&
	);

	/* @brief Emits the overall plot */
	std::string plot() const;
	/** @brief Writes the plot to a file */
	void writeToFile(const std::string&);

	/** @brief Allows to add a new plotter object */
	void registerPlotter(boost::shared_ptr<GBasePlotter>);

	/** @brief Set the dimensions of the output canvas */
	void setCanvasDimensions(const boost::uint32_t&, const boost::uint32_t&);
	/** @brief Allows to retrieve the canvas dimensions */
	boost::tuple<boost::uint32_t, boost::uint32_t> getCanvasDimensions() const;

private:
	/** @brief The default constructor -- intentionally private and undefined */
	GPlotDesigner();

	/** @brief A header for static data in a ROOT file */
	std::string staticHeader() const;

	std::vector<boost::shared_ptr<GBasePlotter> > plotters_; ///< A list of plots to be added to the diagram

	std::size_t c_x_div_, c_y_div_; ///< The number of divisions in x- and y-direction
	boost::uint32_t c_x_dim_, c_y_dim_; ///< Holds the number of pixels of the canvas

	std::string canvasLabel_; ///< A label to be assigned to the entire canvas
};

/*********************************************************************************/

} /* namespace Common */
} /* namespace Gem */

#endif /* GPLOTDESIGNER_HPP_ */
