/**
 * @file GPlotDesigner.hpp
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

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard header files go here
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <sstream>
#include <functional>

// Boost headers go here
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/utility.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/tuple/tuple_io.hpp>
#include <boost/tuple/tuple_comparison.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/cstdint.hpp>
#include <boost/cast.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/variant.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/tracking.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/export.hpp>


#ifndef GPLOTDESIGNER_HPP_
#define GPLOTDESIGNER_HPP_

// Geneva headers go here
#include "common/GExceptions.hpp"
#include "common/GLogger.hpp"
#include "common/GHelperFunctionsT.hpp"
#include "common/GMathHelperFunctionsT.hpp"
#include "common/GCommonEnums.hpp"
#include "common/GCommonInterfaceT.hpp"

namespace Gem {
namespace Common {

/******************************************************************************/
/**
 * Determines whether a scatter plot or a curve should be recorded
 */
enum G_API_COMMON graphPlotMode {
	SCATTER = 0, CURVE = 1
};

/** @brief Puts a Gem::Common::graphPlotMode into a stream; Needed also for boost::lexical_cast<> */
G_API_COMMON std::ostream &operator<<(std::ostream &, const Gem::Common::graphPlotMode &);
/** @brief Reads a Gem::Common::graphPlotMode from a stream; Needed also for boost::lexical_cast<> */
G_API_COMMON std::istream &operator>>(std::istream &, Gem::Common::graphPlotMode &);

/******************************************************************************/

//Some default values

const boost::uint32_t DEFCXDIM = 1024;
const boost::uint32_t DEFCYDIM = 768;

const boost::uint32_t DEFCXDIV = 1;
const boost::uint32_t DEFCYDIV = 1;

const std::size_t DEFNSAMPLES = 100;

const graphPlotMode DEFPLOTMODE = CURVE;

// Easier access to the header-, body- and footer-data
typedef boost::tuple<std::string, std::string, std::string> plotData;

// Forward declaration in order to allow a friend statement in GBasePlotter
class GPlotDesigner;

/******************************************************************************/
/**
 * An abstract base class that defines functions for plots. Concrete plotters
 * derive from this class. They can be added to a master canvas, which takes care
 * to plot them into sub-pads.
 */
class GBasePlotter
	: public Gem::Common::GCommonInterfaceT<GBasePlotter>
{
	friend class GPlotDesigner;

	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int){
		using boost::serialization::make_nvp;

		ar
		& BOOST_SERIALIZATION_NVP(drawingArguments_)
		& BOOST_SERIALIZATION_NVP(x_axis_label_)
		& BOOST_SERIALIZATION_NVP(y_axis_label_)
		& BOOST_SERIALIZATION_NVP(z_axis_label_)
		& BOOST_SERIALIZATION_NVP(plot_label_)
		& BOOST_SERIALIZATION_NVP(dsMarker_)
		& BOOST_SERIALIZATION_NVP(secondaryPlotter_)
  		& BOOST_SERIALIZATION_NVP(id_);
	}
	///////////////////////////////////////////////////////////////////////

public:
	/** @brief The default constructor */
	G_API_COMMON GBasePlotter();
	/** @brief A copy constructor */
	G_API_COMMON GBasePlotter(const GBasePlotter &);

	/** @brief The destructor */
	virtual G_API_COMMON ~GBasePlotter();

	/** @brief The assignment operator */
	G_API_COMMON void operator=(const GBasePlotter &);

	/** @brief Allows to set the drawing arguments for this plot */
	G_API_COMMON void setDrawingArguments(std::string);

	/** @brief Sets the label for the x-axis */
	G_API_COMMON void setXAxisLabel(std::string);
	/** @brief Retrieve the x-axis label */
	G_API_COMMON std::string xAxisLabel() const;
	/** @brief Sets the label for the y-axis */
	G_API_COMMON void setYAxisLabel(std::string);
	/** @brief Retrieve the y-axis label */
	G_API_COMMON std::string yAxisLabel() const;
	/** @brief Sets the label for the z-axis */
	G_API_COMMON void setZAxisLabel(std::string);
	/** @brief Retrieve the z-axis label */
	G_API_COMMON std::string zAxisLabel() const;

	/** @brief Allows to assign a label to the entire plot */
	G_API_COMMON void setPlotLabel(std::string);
	/** @brief Allows to retrieve the plot label */
	G_API_COMMON std::string plotLabel() const;

	/** @brief Allows to assign a marker to data structures */
	G_API_COMMON void setDataStructureMarker(std::string);
	/** @brief Allows to retrieve the data structure marker */
	G_API_COMMON std::string dsMarker() const;

	/** @brief Allows to add secondary plots to be added to the same sub-canvas */
	G_API_COMMON void registerSecondaryPlotter(std::shared_ptr <GBasePlotter>);

	/** @brief Allows to retrieve the id of this object */
	G_API_COMMON std::size_t id() const;
	/** @brief Sets the id of the object */
	G_API_COMMON void setId(const std::size_t &);

	/** @brief Retrieves a unique name for this plotter */
	virtual G_API_COMMON std::string getPlotterName() const = 0;
	/** @brief Returns the name of this class */
	virtual G_API_COMMON std::string name() const override;

	/** @brief Searches for compliance with expectations with respect to another object of the same type */
	virtual void compare(
		const GBasePlotter& // the other object
		, const Gem::Common::expectation& // the expectation for this object, e.g. equality
		, const double& // the limit for allowed deviations of floating point types
	) const override;

	/***************************************************************************/
	/**
	 * The function creates a clone of the object, converts it to a pointer to a derived class
	 * and emits it as a std::shared_ptr<> . Note that this template will only be accessible to the
	 * compiler if GBasePlotter is a base type of clone_type.
	 *
	 * @return A converted clone of this object, wrapped into a std::shared_ptr
	 */
	template <typename clone_type>
	std::shared_ptr<clone_type> clone(
		typename std::enable_if<std::is_base_of<Gem::Common::GBasePlotter, clone_type>::value>::type* dummy = 0
	) const {
		return Gem::Common::convertSmartPointer<GBasePlotter, clone_type>(std::shared_ptr<GBasePlotter>(this->clone_()));
	}

	/***************************************************************************/
	/**
	 * Loads the data of another GBasePlotter(-derivative), wrapped in a shared pointer. Note that this
	 * function is only accessible to the compiler if load_type is a derivative of GBasePlotter.
	 *
	 * @param cp A copy of another GBasePlotter-derivative, wrapped into a std::shared_ptr<>
	 */
	template <typename load_type>
	inline void load(
		const std::shared_ptr<load_type>& cp
		, typename std::enable_if<std::is_base_of<Gem::Common::GBasePlotter, load_type>::value>::type* dummy = 0
	) {
		load_(cp.get());
	}

	/***************************************************************************/
	/**
	 * Loads the data of another GBasePlotter(-derivative), presented as a constant reference. Note that this
	 * function is only accessible to the compiler if load_type is a derivative of GBasePlotter.
	 *
	 * @param cp A copy of another GBasePlotter-derivative, wrapped into a std::shared_ptr<>
	 */
	template <typename load_type>
	inline void load(
		const load_type& cp
		, typename std::enable_if<std::is_base_of<Gem::Common::GBasePlotter, load_type>::value>::type* dummy = 0
	) {
		load_(&cp);
	}

protected:
	/***************************************************************************/
	/** @brief Loads the data of another object */
	virtual G_API_COMMON void load_(const GBasePlotter*);
	/** @brief Creates a deep clone of this object */
	virtual G_API_COMMON GBasePlotter* clone_() const = 0;

	/***************************************************************************/
	// Functions to be specified in derived classes

	/** @brief Retrieve specific header settings for this plot */
	virtual G_API_COMMON std::string headerData(bool, std::size_t) const = 0;

	/** @brief Retrieves the actual data sets */
	virtual G_API_COMMON std::string bodyData(bool, std::size_t) const = 0;

	/** @brief retrieves specific draw commands for this plot */
	virtual G_API_COMMON std::string footerData(bool, std::size_t) const = 0;

	/** @brief Retrieve the current drawing arguments */
	virtual G_API_COMMON std::string drawingArguments(bool) const = 0;

	/***************************************************************************/
	/** @brief Check that a given plotter is compatible with us */
	virtual G_API_COMMON bool isCompatible(std::shared_ptr <GBasePlotter>) const;

	/** @brief calculate a suffix from id and parent ids */
	G_API_COMMON std::string suffix(bool, std::size_t) const;

	/***************************************************************************/

	std::string drawingArguments_ = std::string(""); ///< Holds the drawing arguments for this plot

	std::string x_axis_label_ = std::string("x"); ///< A label for the x-axis
	std::string y_axis_label_ = std::string("y"); ///< A label for the y-axis
	std::string z_axis_label_ = std::string("z"); ///< A label for the z-axis (if available)

	std::string plot_label_ = std::string("");   ///< A label to be assigned to the entire plot
	std::string dsMarker_ = std::string("");     ///< A marker to make the origin of data structures clear in the output file

private:
	/***************************************************************************/

	/** @brief Retrieve specific header settings for this plot */
	std::string headerData_() const;

	/** @brief Retrieves the actual data sets */
	std::string bodyData_() const;

	/** @brief Retrieves specific draw commands for this plot */
	std::string footerData_() const;

	/***************************************************************************/

	/** @brief A list of plotters that should emit their data into the same canvas */
	std::vector<std::shared_ptr<GBasePlotter>> secondaryPlotter_ = std::vector<std::shared_ptr<GBasePlotter>>();

	std::size_t id_ = 0; ///< The id of this object
};

/******************************************************************************/
/**
 * A data collector for 1-d data of user-defined type. This will usually be
 * data of a histogram type.
 */
template<typename x_type>
class GDataCollector1T : public GBasePlotter {

	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int){
		using boost::serialization::make_nvp;

		ar
		& BOOST_SERIALIZATION_BASE_OBJECT_NVP(GBasePlotter)
		& BOOST_SERIALIZATION_NVP(data_);
	}
	///////////////////////////////////////////////////////////////////////

public:
	/***************************************************************************/
	/**
	 * The default constructor
	 */
	GDataCollector1T()
		: GBasePlotter(), data_() { /* nothing */ }

	/***************************************************************************/
	/**
	 * A copy constructor
	 *
	 * @param cp A copy of another GDataCollector1T<x_type> object
	 */
	GDataCollector1T(const GDataCollector1T<x_type> &cp)
		: GBasePlotter(cp), data_(cp.data_) { /* nothing */ }

	/***************************************************************************/
	/**
	 * The destructor
	 */
	virtual ~GDataCollector1T() {
		data_.clear();
	}

	/***************************************************************************/
	/**
	 * The assignment operator
	 */
	void operator=(const GDataCollector1T<x_type> &cp) {
		// Assign our parent class'es data
		GBasePlotter::operator=(cp);

		// and then our own
		data_ = cp.data_;
	}

	/***************************************************************************/
	/**
	 * This very simple functions allows derived classes
	 * to add data easily to their data sets, when called through a
	 * pointer. I.e., this makes "object_ptr->add(data)" instead of
	 * "*object_ptr & data" possible.
	 */
	template<typename data_type>
	void add(const data_type &item) {
		*this & item;
	}

	/***************************************************************************/
	/**
	 * Allows to add data of arbitrary type, provided it can be converted
	 * safely to the target type.
	 *
	 * @param x_undet The data item to be added to the collection
	 */
	template<typename x_type_undet>
	void operator&(const x_type_undet &x_undet) {
		using boost::numeric::bad_numeric_cast;

		x_type x = x_type(0);

		// Make sure the data can be converted to doubles
		try {
			x = boost::numeric_cast<x_type>(x_undet);
		}
		catch (bad_numeric_cast &e) {
			glogger
			<< "In GDataCollector1T::operator&(const T&): Error!" << std::endl
			<< "Encountered invalid cast with boost::numeric_cast," << std::endl
			<< "with the message " << std::endl
			<< e.what() << std::endl
			<< GEXCEPTION;
		}

		// Add the converted data to our collection
		data_.push_back(x);
	}

	/***************************************************************************/
	/**
	 * Allows to add data of type "x_type
	 *
	 * @param x The data item to be added to the collection
	 */
	void operator&(const x_type &x) {
		// Add the data item to our collection
		data_.push_back(x);
	}

	/***************************************************************************/
	/**
	 * Allows to add a collection of data items of undetermined type in one go,
	 * provided the type can be converted safely into the target type
	 *
	 * @param x_vec_undet A collection of data items of undetermined type, to be added to the collection
	 */
	template<typename x_type_undet>
	void operator&(const std::vector<x_type_undet> &x_vec_undet) {
		using boost::numeric::bad_numeric_cast;

		x_type x = x_type(0);

		typename std::vector<x_type_undet>::const_iterator cit;
		for (cit = x_vec_undet.begin(); cit != x_vec_undet.end(); ++cit) {
			// Make sure the data can be converted to doubles
			try {
				x = boost::numeric_cast<x_type>(*cit);
			}
			catch (bad_numeric_cast &e) {
				glogger
				<< "In GDataCollector1T::operator&(const std::vector<T>&): Error!" << std::endl
				<< "Encountered invalid cast with boost::numeric_cast," << std::endl
				<< "with the message " << std::endl
				<< e.what() << std::endl
				<< GEXCEPTION;
			}

			// Add the converted data to our collection
			data_.push_back(x);
		}
	}

	/***************************************************************************/
	/**
	 * Allows to add a collection of data items of type x_type to our data_ vector.
	 *
	 * @param x_vec A vector of data items to be added to the data_ vector
	 */
	void operator&(const std::vector<x_type> &x_vec) {
		typename std::vector<x_type>::const_iterator cit;
		for (cit = x_vec.begin(); cit != x_vec.end(); ++cit) {
			// Add the data item to our collection
			data_.push_back(*cit);
		}
	}

protected:
	/***************************************************************************/
	/**
	 * Loads the data of another object
	 */
	virtual void load_(const GBasePlotter* cp) {
		// Check that we are dealing with a GDataCollector1T<x_type> reference independent of this object and convert the pointer
		const GDataCollector1T<x_type> *p_load = Gem::Common::g_convert_and_compare<GBasePlotter, GDataCollector1T<x_type>>(cp, this);

		// Load our parent class'es data ...
		GBasePlotter::load_(cp);

		// ... and then our own
		data_ = p_load->data_; // This assumes that x_type is POD
	}

	/***************************************************************************/
	/** @brief Creates a deep clone of this object */
	virtual G_API_COMMON GBasePlotter* clone_() const = 0;

	/***************************************************************************/

	std::vector<x_type> data_; ///< Holds the actual data
};

/******************************************************************************/
/**
 * A wrapper for ROOT's TH1D class (1-d double data)
 */
class GHistogram1D : public GDataCollector1T<double> {

	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int){
		using boost::serialization::make_nvp;

		ar
		& make_nvp("GDataCollector1T_double", boost::serialization::base_object<GDataCollector1T<double>>(*this))
		& BOOST_SERIALIZATION_NVP(nBinsX_)
		& BOOST_SERIALIZATION_NVP(minX_)
		& BOOST_SERIALIZATION_NVP(maxX_);
	}
	///////////////////////////////////////////////////////////////////////

public:
	/** @brief The standard constructor */
	G_API_COMMON GHistogram1D(
		const std::size_t &, const double &, const double &
	);
	/** @brief Initialization with a range in the form of a tuple */
	G_API_COMMON GHistogram1D(
		const std::size_t &, const boost::tuple<double, double> &
	);
	/** @brief A copy constructor */
	G_API_COMMON GHistogram1D(const GHistogram1D &);

	/** @brief The destructor */
	virtual G_API_COMMON ~GHistogram1D();

	/** @brief The assignment operator */
	G_API_COMMON const GHistogram1D &operator=(const GHistogram1D &);

	/** @brief Retrieve the number of bins in x-direction */
	G_API_COMMON std::size_t getNBinsX() const;

	/** @brief Retrieve the lower boundary of the plot */
	G_API_COMMON double getMinX() const;
	/** @brief Retrieve the upper boundary of the plot */
	G_API_COMMON double getMaxX() const;

	/** @brief Retrieves a unique name for this plotter */
	virtual G_API_COMMON std::string getPlotterName() const;

protected:
	/** @brief Retrieve specific header settings for this plot */
	virtual std::string headerData(bool, std::size_t) const;

	/** @brief Retrieves the actual data sets */
	virtual std::string bodyData(bool, std::size_t) const;

	/** @brief Retrieves specific draw commands for this plot */
	virtual std::string footerData(bool, std::size_t) const;

	/** @brief Retrieve the current drawing arguments */
	virtual std::string drawingArguments(bool) const;

	/** @brief Loads the data of another object */
	virtual G_API_COMMON void load_(const GBasePlotter*);
	/** @brief Creates a deep clone of this object */
	virtual G_API_COMMON GBasePlotter* clone_() const;

private:
	GHistogram1D() = delete; ///< The default constructor -- intentionally private and undefined

	std::size_t nBinsX_; ///< The number of bins in the histogram

	double minX_; ///< The lower boundary of the histogram
	double maxX_; ///< The upper boundary of the histogram
};

/******************************************************************************/
/**
 * A wrapper for ROOT's TH1I class (1-d integer data)
 */
class GHistogram1I : public GDataCollector1T<boost::int32_t> {

	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int){
		using boost::serialization::make_nvp;

		ar
		& make_nvp("GDataCollector1T_int32_t", boost::serialization::base_object<GDataCollector1T<boost::int32_t>>(*this))
		& BOOST_SERIALIZATION_NVP(nBinsX_)
		& BOOST_SERIALIZATION_NVP(minX_)
		& BOOST_SERIALIZATION_NVP(maxX_);
	}
	///////////////////////////////////////////////////////////////////////

public:
	/** @brief The standard constructor */
	G_API_COMMON GHistogram1I(
		const std::size_t &, const double &, const double &
	);
	/** @brief Initialization with a range in the form of a tuple */
	G_API_COMMON GHistogram1I(
		const std::size_t &, const boost::tuple<double, double> &
	);
	/** @brief A copy constructor */
	G_API_COMMON GHistogram1I(const GHistogram1I &);

	/** @brief The destructor */
	G_API_COMMON ~GHistogram1I();

	/** @brief The assignment operator */
	G_API_COMMON const GHistogram1I &operator=(const GHistogram1I &);

	/** @brief Retrieve the number of bins in x-direction */
	G_API_COMMON std::size_t getNBinsX() const;

	/** @brief Retrieve the lower boundary of the plot */
	G_API_COMMON double getMinX() const;
	/** @brief Retrieve the upper boundary of the plot */
	G_API_COMMON double getMaxX() const;

	/** @brief Retrieves a unique name for this plotter */
	virtual G_API_COMMON std::string getPlotterName() const;

protected:
	/** @brief Retrieve specific header settings for this plot */
	virtual std::string headerData(bool, std::size_t) const;

	/** @brief Retrieves the actual data sets */
	virtual std::string bodyData(bool, std::size_t) const;

	/** @brief Retrieves specific draw commands for this plot */
	virtual std::string footerData(bool, std::size_t) const;

	/** @brief Retrieve the current drawing arguments */
	virtual std::string drawingArguments(bool) const;

	/** @brief Loads the data of another object */
	virtual G_API_COMMON void load_(const GBasePlotter*);
	/** @brief Creates a deep clone of this object */
	virtual G_API_COMMON GBasePlotter* clone_() const;

private:
	GHistogram1I() = delete; ///< The default constructor -- intentionally private and undefined

	std::size_t nBinsX_; ///< The number of bins in the histogram

	double minX_; ///< The lower boundary of the histogram // TODO: Really "double" ?
	double maxX_; ///< The upper boundary of the histogram
};

/******************************************************************************/
/**
 * A data collector for 2-d data of user-defined type, such as a TGraph
 */
template<typename x_type, typename y_type>
class GDataCollector2T : public GBasePlotter {

	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int){
		using boost::serialization::make_nvp;

		ar
		& BOOST_SERIALIZATION_BASE_OBJECT_NVP(GBasePlotter)
		& BOOST_SERIALIZATION_NVP(data_);
	}
	///////////////////////////////////////////////////////////////////////

public:
	/***************************************************************************/
	/**
	 * The default constructor
	 */
	GDataCollector2T()
		: GBasePlotter(), data_() { /* nothing */ }

	/***************************************************************************/
	/**
	 * A copy constructor
	 *
	 * @param cp A copy of another GDataCollector1T<x_type> object
	 */
	GDataCollector2T(const GDataCollector2T<x_type, y_type> &cp)
		: GBasePlotter(cp), data_(cp.data_) { /* nothing */ }

	/***************************************************************************/
	/**
	 * The destructor
	 */
	virtual ~GDataCollector2T() {
		data_.clear();
	}

	/***************************************************************************/
	/**
	 * The assignment operator
	 */
	void operator=(const GDataCollector2T<x_type, y_type> &cp) {
		// Assign our parent class'es data
		GBasePlotter::operator=(cp);

		// and then our own
		data_ = cp.data_;
	}

	/***************************************************************************/
	/**
	 * Allows to project the graph into a histogram (x-direction). This function is a
	 * trap to catch calls with un-implemented types. Use the corresponding specializations,
	 * if available.
	 */
	std::shared_ptr <GDataCollector1T<x_type>> projectX(
		std::size_t, boost::tuple<x_type, x_type>
	) const {
		glogger
		<< "In GDataCollector2T<>::projectX(range, nBins): Error!" << std::endl
		<< "Function was called for class with un-implemented types" << std::endl
		<< GEXCEPTION;

		// Make the compiler happy
		return std::shared_ptr<GDataCollector1T<x_type>>();
	}

	/***************************************************************************/
	/**
	 * Allows to project the graph into a histogram (y-direction). This function is a
	 * trap to catch calls with un-implemented types. Use the corresponding specializations,
	 * if available.
	 */
	std::shared_ptr <GDataCollector1T<y_type>> projectY(
		std::size_t, boost::tuple<y_type, y_type>
	) const {
		glogger
		<< "In GDataCollector2T<>::projectY(range, nBins): Error!" << std::endl
		<< "Function was called for class with un-implemented types" << std::endl
		<< GEXCEPTION;

		// Make the compiler happy
		return std::shared_ptr<GDataCollector1T<y_type>>();
	}

	/***************************************************************************/
	/**
	 * This very simple functions allows derived classes
	 * to add data easily to their data sets, when called through a
	 * pointer. I.e., this makes object_ptr->add(data) instead of
	 * *object_ptr & data possible.
	 */
	template<typename data_type>
	void add(const data_type &item) {
		*this & item;
	}

	/***************************************************************************/
	/**
	 * Allows to add data of undetermined type to the collection in an intuitive way,
	 * provided that it can be converted safely to the target type.
	 *
	 * @param point_undet The data item to be added to the collection
	 */
	template<typename x_type_undet, typename y_type_undet>
	void operator&(const boost::tuple<x_type_undet, y_type_undet> &point_undet) {
		using boost::numeric::bad_numeric_cast;

		x_type x = x_type(0);
		y_type y = y_type(0);

		// Make sure the data can be converted to doubles
		try {
			x = boost::numeric_cast<x_type>(boost::get<0>(point_undet));
			y = boost::numeric_cast<y_type>(boost::get<1>(point_undet));
		}
		catch (bad_numeric_cast &e) {
			glogger
			<< "In GDataCollector2T::operator&(const boost::tuple<S,T>&): Error!" << std::endl
			<< "Encountered invalid cast with boost::numeric_cast," << std::endl
			<< "with the message " << std::endl
			<< e.what() << std::endl
			<< GEXCEPTION;
		}

		data_.push_back(boost::tuple<x_type, y_type>(x, y));
	}

	/***************************************************************************/
	/**
	 * Allows to add data of type boost::tuple<x_type, y_type> to the collection in
	 * an intuitive way.
	 *
	 * @param point The data item to be added to the collection
	 */
	void operator&(const boost::tuple<x_type, y_type> &point) {
		// Add the data item to the collection
		data_.push_back(point);
	}

	/***************************************************************************/
	/**
	 * Allows to add a collection of data items of undetermined type to the
	 * collection in an intuitive way, provided they can be converted safely
	 * to the target type.
	 *
	 * @param point_vec_undet The collection of data items to be added to the collection
	 */
	template<typename x_type_undet, typename y_type_undet>
	void operator&(const std::vector<boost::tuple<x_type_undet, y_type_undet>> &point_vec_undet) {
		using boost::numeric::bad_numeric_cast;

		x_type x = x_type(0);
		y_type y = y_type(0);

		typename std::vector<boost::tuple<x_type_undet, y_type_undet>>::const_iterator cit;
		for (cit = point_vec_undet.begin(); cit != point_vec_undet.end(); ++cit) {
			// Make sure the data can be converted to doubles
			try {
				x = boost::numeric_cast<x_type>(boost::get<0>(*cit));
				y = boost::numeric_cast<y_type>(boost::get<1>(*cit));
			}
			catch (bad_numeric_cast &e) {
				glogger
				<< "In GDataCollector2T::operator&(const std::vector<boost::tuple<S,T>>&): Error!" << std::endl
				<< "Encountered invalid cast with boost::numeric_cast," << std::endl
				<< "with the message " << std::endl
				<< e.what() << std::endl
				<< GEXCEPTION;
			}

			data_.push_back(boost::tuple<x_type, y_type>(x, y));
		}
	}

	/***************************************************************************/
	/**
	 * Allows to add a collection of data items of type boost::tuple<x_type, y_type>
	 * to the collection in an intuitive way, provided they can be converted safely
	 * to the target type.
	 *
	 * @param point_vec The collection of data items to be added to the collection
	 */
	void operator&(const std::vector<boost::tuple<x_type, y_type>> &point_vec) {
		typename std::vector<boost::tuple<x_type, y_type>>::const_iterator cit;
		for (cit = point_vec.begin(); cit != point_vec.end(); ++cit) {
			// Add the data item to the collection
			data_.push_back(*cit);
		}
	}

	/***************************************************************************/
	/**
	 * Sorts the data according to its x-component
	 */
	void sortX() {
		std::sort(
			data_.begin(), data_.end(),
			[](const boost::tuple<x_type, y_type> &x, const boost::tuple<x_type, y_type> &y) -> bool {
				return boost::get<0>(x) < boost::get<0>(y);
			}
		);
	}

protected:
	/***************************************************************************/
	/**
	 * Loads the data of another object
	 */
	virtual void load_(const GBasePlotter* cp) {
		// Check that we are dealing with a GDataCollector2T<x_type, y_type> reference independent of this object and convert the pointer
		const GDataCollector2T<x_type, y_type> *p_load = Gem::Common::g_convert_and_compare<GBasePlotter, GDataCollector2T<x_type, y_type>>(cp, this);

		// Load our parent class'es data ...
		GBasePlotter::load_(cp);

		// ... and then our own
		data_ = p_load->data_; // This assumes that x_type is POD
	}

	/***************************************************************************/

	/** @brief Creates a deep clone of this object */
	virtual G_API_COMMON GBasePlotter* clone_() const = 0;

	/***************************************************************************/

	std::vector<boost::tuple<x_type, y_type>> data_; ///< Holds the actual data
};

/******************************************************************************/
/**
 * Specialization of projectX for <x_type, y_type> = <double, double>, that will return a
 * GHistogram1D object, wrapped into a std::shared_ptr<GHistogram1D>. In case of a
 * default-constructed range, the function will attempt to determine suitable parameters
 * for the range settings.
 *
 * @param nBins The number of bins of the histogram
 * @param range The minimum and maximum boundaries of the histogram
 */
template<>
inline
std::shared_ptr <GDataCollector1T<double>>
GDataCollector2T<double, double>::projectX(std::size_t nBinsX, boost::tuple<double, double> rangeX) const {
	boost::tuple<double, double> myRangeX;
	if (rangeX == boost::tuple<double, double>()) {
		// Find out about the minimum and maximum values in the data_ array
		boost::tuple<double, double, double, double> extremes = Gem::Common::getMinMax(this->data_);
		myRangeX = boost::tuple<double, double>(boost::get<0>(extremes), boost::get<1>(extremes));
	} else {
		myRangeX = rangeX;
	}

	// Construct the result object
	std::shared_ptr <GHistogram1D> result(new GHistogram1D(nBinsX, myRangeX));
	result->setXAxisLabel(this->xAxisLabel());
	result->setYAxisLabel("Number of entries");
	result->setPlotLabel(this->plotLabel() + " / x-projection");

	// Add data to the object
	for (std::size_t i = 0; i < data_.size(); i++) {
		(*result) & boost::get<0>(data_.at(i));
	}

	// Return the data
	return result;
}


/******************************************************************************/
/**
 * Specialization of projectY for <x_type, y_type> = <double, double>, that will return a
 * GHistogram1D object, wrapped into a std::shared_ptr<GHistogram1D>. In case of a
 * default-constructed range, the function will attempt to determine suitable parameters
 * for the range settings.
 *
 * @param nBins The number of bins of the histogram
 * @param range The minimum and maximum boundaries of the histogram
 */
template<>
inline
std::shared_ptr <GDataCollector1T<double>>
GDataCollector2T<double, double>::projectY(std::size_t nBinsY, boost::tuple<double, double> rangeY) const {
	boost::tuple<double, double> myRangeY;
	if (rangeY == boost::tuple<double, double>()) {
		// Find out about the minimum and maximum values in the data_ array
		boost::tuple<double, double, double, double> extremes = Gem::Common::getMinMax(data_);
		myRangeY = boost::tuple<double, double>(boost::get<2>(extremes), boost::get<3>(extremes));
	} else {
		myRangeY = rangeY;
	}

	// Construct the result object
	std::shared_ptr <GHistogram1D> result(new GHistogram1D(nBinsY, myRangeY));
	result->setXAxisLabel(this->yAxisLabel());
	result->setYAxisLabel("Number of entries");
	result->setPlotLabel(this->plotLabel() + " / y-projection");

	// Add data to the object
	for (std::size_t i = 0; i < data_.size(); i++) {
		(*result) & boost::get<1>(data_.at(i));
	}

	// Return the data
	return result;
}

/******************************************************************************/
/**
 * A data collector for 2-d data of user-defined type, with the ability to
 * additionally specify an error component for both dimensions.
 */
template<typename x_type, typename y_type>
class GDataCollector2ET : public GBasePlotter {

	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int){
		using boost::serialization::make_nvp;

		ar
		& BOOST_SERIALIZATION_BASE_OBJECT_NVP(GBasePlotter)
		& BOOST_SERIALIZATION_NVP(data_);
	}
	///////////////////////////////////////////////////////////////////////

public:
	/***************************************************************************/
	/**
	 * The default constructor
	 */
	GDataCollector2ET()
		: GBasePlotter(), data_() { /* nothing */ }

	/***************************************************************************/
	/**
	 * A copy constructor
	 *
	 * @param cp A copy of another GDataCollector1T<x_type> object
	 */
	GDataCollector2ET(const GDataCollector2ET<x_type, y_type> &cp)
		: GBasePlotter(cp), data_(cp.data_) { /* nothing */ }

	/***************************************************************************/
	/**
	 * The destructor
	 */
	virtual ~GDataCollector2ET() {
		data_.clear();
	}

	/***************************************************************************/
	/**
	 * The assignment operator
	 */
	void operator=(const GDataCollector2ET<x_type, y_type> &cp) {
		// Assign our parent class'es data
		GBasePlotter::operator=(cp);

		// and then our own
		data_ = cp.data_;
	}

	/***************************************************************************/
	/**
	 * Allows to add data of undetermined type to the collection in an intuitive way,
	 * provided that it can be converted safely to the target type.
	 *
	 * @param point_undet The data item to be added to the collection
	 */
	template<typename x_type_undet, typename y_type_undet>
	void operator&(const boost::tuple<x_type_undet, x_type_undet, y_type_undet, y_type_undet> &point_undet) {
		using boost::numeric::bad_numeric_cast;

		x_type x = x_type(0);
		x_type ex = x_type(0);
		y_type y = y_type(0);
		y_type ey = y_type(0);

		// Make sure the data can be converted to doubles
		try {
			x = boost::numeric_cast<x_type>(boost::get<0>(point_undet));
			ex = boost::numeric_cast<x_type>(boost::get<1>(point_undet));
			y = boost::numeric_cast<y_type>(boost::get<2>(point_undet));
			ey = boost::numeric_cast<y_type>(boost::get<3>(point_undet));
		}
		catch (bad_numeric_cast &e) {
			glogger
			<< "In GDataCollector2ET::operator&(const boost::tuple<S,S,T,T>&): Error!" << std::endl
			<< "Encountered invalid cast with boost::numeric_cast," << std::endl
			<< "with the message " << std::endl
			<< e.what() << std::endl
			<< GEXCEPTION;
		}

		data_.push_back(boost::tuple<x_type, x_type, y_type, y_type>(x, ex, y, ey));
	}

	/***************************************************************************/
	/**
	 * Allows to add data of type boost::tuple<x_type, y_type> to the collection in
	 * an intuitive way.
	 *
	 * @param point The data item to be added to the collection
	 */
	void operator&(const boost::tuple<x_type, x_type, y_type, y_type> &point) {
		// Add the data item to the collection
		data_.push_back(point);
	}

	/***************************************************************************/
	/**
	 * Allows to add a collection of data items of undetermined type to the
	 * collection in an intuitive way, provided they can be converted safely
	 * to the target type.
	 *
	 * @param point_vec_undet The collection of data items to be added to the collection
	 */
	template<typename x_type_undet, typename y_type_undet>
	void operator&(
		const std::vector<boost::tuple<x_type_undet, x_type_undet, y_type_undet, y_type_undet>> &point_vec_undet) {
		using boost::numeric::bad_numeric_cast;

		x_type x = x_type(0);
		x_type ex = x_type(0);
		y_type y = y_type(0);
		y_type ey = y_type(0);

		typename std::vector<boost::tuple<x_type_undet, x_type_undet, y_type_undet, y_type_undet>>::const_iterator cit;
		for (cit = point_vec_undet.begin(); cit != point_vec_undet.end(); ++cit) {
			// Make sure the data can be converted to doubles
			try {
				x = boost::numeric_cast<x_type>(boost::get<0>(*cit));
				ex = boost::numeric_cast<x_type>(boost::get<1>(*cit));
				y = boost::numeric_cast<y_type>(boost::get<2>(*cit));
				ey = boost::numeric_cast<y_type>(boost::get<3>(*cit));
			}
			catch (bad_numeric_cast &e) {
				glogger
				<< "In GDataCollector2ET::operator&(const std::vector<boost::tuple<S,S,T,T>>&): Error!" << std::endl
				<< "Encountered invalid cast with boost::numeric_cast," << std::endl
				<< "with the message " << std::endl
				<< e.what() << std::endl
				<< GEXCEPTION;
			}

			data_.push_back(boost::tuple<x_type, x_type, y_type, y_type>(x, ex, y, ey));
		}
	}

	/***************************************************************************/
	/**
	 * Allows to add a collection of data items of type boost::tuple<x_type, x_type, y_type, y_type>
	 * to the collection in an intuitive way, provided they can be converted safely
	 * to the target type.
	 *
	 * @param point_vec The collection of data items to be added to the collection
	 */
	void operator&(const std::vector<boost::tuple<x_type, x_type, y_type, y_type>> &point_vec) {
		typename std::vector<boost::tuple<x_type, x_type, y_type, y_type>>::const_iterator cit;
		for (cit = point_vec.begin(); cit != point_vec.end(); ++cit) {
			// Add the data item to the collection
			data_.push_back(*cit);
		}
	}

	/***************************************************************************/
	/**
	 * This very simple functions allows derived classes
	 * to add data easily to their data sets, when called through a
	 * pointer. I.e., this makes "object_ptr->add(data)" instead of
	 * "*object_ptr & data" possible.
	 */
	template<typename data_type>
	void add(const data_type &item) {
		*this & item;
	}

	/***************************************************************************/
	/**
	 * Sorts the data according to its x-component
	 */
	void sortX() {
		std::sort(
			data_.begin(), data_.end(), [](
				const boost::tuple<x_type, x_type, y_type, y_type> &x, const boost::tuple<x_type, x_type, y_type, y_type> &y
			) -> bool {
				return boost::get<0>(x) < boost::get<0>(y);
			}
		);
	}

protected:
	/***************************************************************************/
	/**
	 * Loads the data of another object
	 */
	virtual void load_(const GBasePlotter* cp) {
		// Check that we are dealing with a GDataCollector2ET<x_type, y_type> reference independent of this object and convert the pointer
		const GDataCollector2ET<x_type, y_type> *p_load = Gem::Common::g_convert_and_compare<GBasePlotter, GDataCollector2ET<x_type, y_type>>(cp, this);

		// Load our parent class'es data ...
		GBasePlotter::load_(cp);

		// ... and then our own
		data_ = p_load->data_; // This assumes that x_type is POD
	}

	/***************************************************************************/
	/** @brief Creates a deep clone of this object */
	virtual G_API_COMMON GBasePlotter* clone_() const = 0;

	/***************************************************************************/

	std::vector<boost::tuple<x_type, x_type, y_type, y_type>> data_; ///< Holds the actual data
};

/******************************************************************************/
/**
 * An enum for 2D-drawing options
 */
enum G_API_COMMON tddropt {
	TDEMPTY = 0,
	SURFONE = 1,
	SURFTWOZ = 2,
	SURFTHREE = 3,
	SURFFOUR = 4,
	CONTZ = 5,
	CONTONE = 6,
	CONTTWO = 7,
	CONTTHREE = 8,
	TEXT = 9,
	SCAT = 10,
	BOX = 11,
	ARR = 12,
	COLZ = 13,
	LEGO = 14,
	LEGOONE = 15,
	SURFONEPOL = 16,
	SURFONECYL = 17
};

/** @brief Puts a Gem::Common::tddropt into a stream; Needed also for boost::lexical_cast<> */
G_API_COMMON std::ostream &operator<<(std::ostream &, const Gem::Common::tddropt &);
/** @brief Reads a Gem::Common::tddropt from a stream; Needed also for boost::lexical_cast<> */
G_API_COMMON std::istream &operator>>(std::istream &, Gem::Common::tddropt &);

/******************************************************************************/
/**
 * A wrapper for ROOT's TH2D class (2-d double data)
 */
class GHistogram2D
	: public GDataCollector2T<double, double> {

	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int){
		using boost::serialization::make_nvp;

		ar
		& make_nvp("GDataCollector2T_double_double", boost::serialization::base_object<GDataCollector2T<double, double>>(*this))
		& BOOST_SERIALIZATION_NVP(nBinsX_)
		& BOOST_SERIALIZATION_NVP(nBinsY_)
		& BOOST_SERIALIZATION_NVP(minX_)
		& BOOST_SERIALIZATION_NVP(maxX_)
		& BOOST_SERIALIZATION_NVP(minY_)
		& BOOST_SERIALIZATION_NVP(maxY_)
		& BOOST_SERIALIZATION_NVP(dropt_);
	}
	///////////////////////////////////////////////////////////////////////

public:
	/** @brief The standard constructor */
	G_API_COMMON GHistogram2D(
		const std::size_t &, const std::size_t &, const double &, const double &, const double &, const double &
	);
	/** @brief Initialization with ranges */
	G_API_COMMON GHistogram2D(
		const std::size_t &, const std::size_t &, const boost::tuple<double, double> &,
		const boost::tuple<double, double> &
	);
	/** @brief A copy constructor */
	G_API_COMMON GHistogram2D(const GHistogram2D &);

	/** @brief The destructor */
	virtual G_API_COMMON ~GHistogram2D();

	/** @brief The assignment operator */
	G_API_COMMON const GHistogram2D &operator=(const GHistogram2D &);

	/** @brief Retrieve the number of bins in x-direction */
	G_API_COMMON std::size_t getNBinsX() const;
	/** @brief Retrieve the number of bins in y-direction */
	G_API_COMMON std::size_t getNBinsY() const;

	/** @brief Retrieve the lower boundary of the plot in x-direction */
	G_API_COMMON double getMinX() const;
	/** @brief Retrieve the upper boundary of the plot in x-direction */
	G_API_COMMON double getMaxX() const;
	/** @brief Retrieve the lower boundary of the plot in y-direction */
	G_API_COMMON double getMinY() const;
	/** @brief Retrieve the upper boundary of the plot in y-direction */
	G_API_COMMON double getMaxY() const;

	/** @brief Retrieves a unique name for this plotter */
	G_API_COMMON virtual std::string getPlotterName() const;

	/** @brief Allows to specify 2d-drawing options */
	G_API_COMMON void set2DOpt(tddropt);
	/** @brief Allows to retrieve 2d-drawing options */
	G_API_COMMON tddropt get2DOpt() const;

protected:
	/** @brief Retrieve specific header settings for this plot */
	virtual std::string headerData(bool, std::size_t) const;

	/** @brief Retrieves the actual data sets */
	virtual std::string bodyData(bool, std::size_t) const;

	/** @brief Retrieves specific draw commands for this plot */
	virtual std::string footerData(bool, std::size_t) const;

	/** @brief Retrieve the current drawing arguments */
	virtual std::string drawingArguments(bool) const;

	/** @brief Loads the data of another object */
	virtual G_API_COMMON void load_(const GBasePlotter*);
	/** @brief Creates a deep clone of this object */
	virtual G_API_COMMON GBasePlotter* clone_() const;

private:
	GHistogram2D() = delete; ///< The default constructor -- intentionally private and undefined

	std::size_t nBinsX_; ///< The number of bins in the x-direction of the histogram
	std::size_t nBinsY_; ///< The number of bins in the y-direction of the histogram

	double minX_; ///< The lower boundary of the histogram in x-direction
	double maxX_; ///< The upper boundary of the histogram in x-direction
	double minY_; ///< The lower boundary of the histogram in y-direction
	double maxY_; ///< The upper boundary of the histogram in y-direction

	tddropt dropt_; ///< The drawing options for 2-d histograms
};

/******************************************************************************/
/**
 * A wrapper for the ROOT TGraph class (2d data and curve-like structures). It
 * also adds the option to draw arrows between consecutive points.
 */
class GGraph2D
	: public GDataCollector2T<double, double> {

	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int){
		using boost::serialization::make_nvp;

		ar
		& make_nvp("GDataCollector2T_double_double", boost::serialization::base_object<GDataCollector2T<double, double>>(*this))
		& BOOST_SERIALIZATION_NVP(pM_)
		& BOOST_SERIALIZATION_NVP(drawArrows_);
	}
	///////////////////////////////////////////////////////////////////////

public:
	/** @brief The default constructor */
	G_API_COMMON GGraph2D();

	/** @brief A copy constructor */
	G_API_COMMON GGraph2D(const GGraph2D &);

	/** @brief The destructor */
	virtual G_API_COMMON ~GGraph2D();

	/** @brief The assignment operator */
	G_API_COMMON const GGraph2D &operator=(const GGraph2D &);

	/** @brief Adds arrows to the plots between consecutive points */
	G_API_COMMON void setDrawArrows(bool= true);
	/** @brief Retrieves the value of the drawArrows_ variable */
	G_API_COMMON bool getDrawArrows() const;

	/** @brief Determines whether a scatter plot or a curve is created */
	G_API_COMMON void setPlotMode(graphPlotMode);
	/** @brief Allows to retrieve the current plotting mode */
	G_API_COMMON graphPlotMode getPlotMode() const;

	/** @brief Retrieves a unique name for this plotter */
	virtual G_API_COMMON std::string getPlotterName() const;

protected:
	/** @brief Retrieve specific header settings for this plot */
	virtual std::string headerData(bool, std::size_t) const;

	/** @brief Retrieves the actual data sets */
	virtual std::string bodyData(bool, std::size_t) const;

	/** @brief Retrieves specific draw commands for this plot */
	virtual std::string footerData(bool, std::size_t) const;

	/** @brief Retrieve the current drawing arguments */
	virtual std::string drawingArguments(bool) const;

	/** @brief Loads the data of another object */
	virtual G_API_COMMON void load_(const GBasePlotter*);
	/** @brief Creates a deep clone of this object */
	virtual G_API_COMMON GBasePlotter* clone_() const;

private:
	graphPlotMode pM_; ///< Whether to create scatter plots or a curve, connected by lines
	bool drawArrows_; ///< When set to true, arrows will be drawn between consecutive points
};

/******************************************************************************/
/**
 * A wrapper for the ROOT TGraphErrors class (2d data and curve-like structures)
 */
class GGraph2ED
	: public GDataCollector2ET<double, double> {

	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int){
		using boost::serialization::make_nvp;

		ar
		& make_nvp("GDataCollector2ET_double_double", boost::serialization::base_object<GDataCollector2ET<double, double>>(*this))
		& BOOST_SERIALIZATION_NVP(pM_);
	}
	///////////////////////////////////////////////////////////////////////

public:
	/** @brief The default constructor */
	G_API_COMMON GGraph2ED();

	/** @brief A copy constructor */
	G_API_COMMON GGraph2ED(const GGraph2ED &);

	/** @brief The destructor */
	virtual G_API_COMMON ~GGraph2ED();

	/** @brief The assignment operator */
	G_API_COMMON const GGraph2ED &operator=(const GGraph2ED &);

	/** @brief Determines whether a scatter plot or a curve is created */
	G_API_COMMON void setPlotMode(graphPlotMode);
	/** @brief Allows to retrieve the current plotting mode */
	G_API_COMMON graphPlotMode getPlotMode() const;

	/** @brief Retrieves a unique name for this plotter */
	G_API_COMMON virtual std::string getPlotterName() const;

protected:
	/** @brief Retrieve specific header settings for this plot */
	virtual std::string headerData(bool, std::size_t) const;

	/** @brief Retrieves the actual data sets */
	virtual std::string bodyData(bool, std::size_t) const;

	/** @brief Retrieves specific draw commands for this plot */
	virtual std::string footerData(bool, std::size_t) const;

	/** @brief Retrieve the current drawing arguments */
	virtual std::string drawingArguments(bool) const;

	/** @brief Loads the data of another object */
	virtual G_API_COMMON void load_(const GBasePlotter*);
	/** @brief Creates a deep clone of this object */
	virtual G_API_COMMON GBasePlotter* clone_() const;

private:
	graphPlotMode pM_; ///< Whether to create scatter plots or a curve, connected by lines
};

/******************************************************************************/
/**
 * A data collector for 3-d data of user-defined type
 */
template<typename x_type, typename y_type, typename z_type>
class GDataCollector3T
	: public GBasePlotter {

	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int){
		using boost::serialization::make_nvp;

		ar
		& BOOST_SERIALIZATION_BASE_OBJECT_NVP(GBasePlotter)
		& BOOST_SERIALIZATION_NVP(data_);
	}
	///////////////////////////////////////////////////////////////////////

public:
	/***************************************************************************/
	/**
	 * The default constructor
	 */
	GDataCollector3T()
		: GBasePlotter(), data_() { /* nothing */ }

	/***************************************************************************/
	/**
	 * A copy constructor
	 *
	 * @param cp A copy of another GDataCollector3T object
	 */
	GDataCollector3T(const GDataCollector3T<x_type, y_type, z_type> &cp)
		: GBasePlotter(cp), data_(cp.data_) { /* nothing */ }

	/***************************************************************************/
	/**
	 * The destructor
	 */
	virtual ~GDataCollector3T() {
		data_.clear();
	}

	/***************************************************************************/
	/**
	 * The assignment operator
	 */
	void operator=(const GDataCollector3T<x_type, y_type, z_type> &cp) {
		// Assign our parent class'es data
		GBasePlotter::operator=(cp);

		// and then our own
		data_ = cp.data_;
	}

	/***************************************************************************/
	/**
	 * Allows to project the graph into a histogram (x-direction). This function is a
	 * trap to catch calls with un-implemented types. Use the corresponding specializations,
	 * if available.
	 */
	std::shared_ptr <GDataCollector1T<x_type>> projectX(
		std::size_t, boost::tuple<x_type, x_type>
	) const {
		glogger
		<< "In GDataCollector3T<>::projectX(range, nBins): Error!" << std::endl
		<< "Function was called for class with un-implemented types" << std::endl
		<< GEXCEPTION;

		// Make the compiler happy
		return std::shared_ptr<GDataCollector1T<x_type>>();
	}

	/***************************************************************************/
	/**
	 * Allows to project the graph into a histogram (y-direction). This function is a
	 * trap to catch calls with un-implemented types. Use the corresponding specializations,
	 * if available.
	 */
	std::shared_ptr <GDataCollector1T<y_type>> projectY(
		std::size_t, boost::tuple<y_type, y_type>
	) const {
		glogger
		<< "In GDataCollector3T<>::projectY(range, nBins): Error!" << std::endl
		<< "Function was called for class with un-implemented types" << std::endl
		<< GEXCEPTION;

		// Make the compiler happy
		return std::shared_ptr<GDataCollector1T<y_type>>();
	}

	/***************************************************************************/
	/**
	 * Allows to project the graph into a histogram (z-direction). This function is a
	 * trap to catch calls with un-implemented types. Use the corresponding specializations,
	 * if available.
	 */
	std::shared_ptr <GDataCollector1T<z_type>> projectZ(
		std::size_t, boost::tuple<z_type, z_type>
	) const {
		glogger
		<< "In GDataCollector3T<>::projectZ(range, nBins): Error!" << std::endl
		<< "Function was called for class with un-implemented types" << std::endl
		<< GEXCEPTION;

		// Make the compiler happy
		return std::shared_ptr<GDataCollector1T<z_type>>();
	}

	/***************************************************************************/
	/**
	 * This very simple functions allows derived classes
	 * to add data easily to their data sets, when called through a
	 * pointer. I.e., this makes object_ptr->add(data) instead of
	 * *object_ptr & data possible.
	 */
	template<typename data_type>
	void add(const data_type &item) {
		*this & item;
	}

	/***************************************************************************/
	/**
	 * Allows to add data of undetermined type to the collection in an intuitive way,
	 * provided that it can be converted safely to the target type.
	 *
	 * @param point_undet The data item to be added to the collection
	 */
	template<typename x_type_undet, typename y_type_undet, typename z_type_undet>
	void operator&(const boost::tuple<x_type_undet, y_type_undet, z_type_undet> &point_undet) {
		using boost::numeric::bad_numeric_cast;

		x_type x = x_type(0);
		y_type y = y_type(0);
		z_type z = z_type(0);

		// Make sure the data can be converted to doubles
		try {
			x = boost::numeric_cast<x_type>(boost::get<0>(point_undet));
			y = boost::numeric_cast<y_type>(boost::get<1>(point_undet));
			z = boost::numeric_cast<z_type>(boost::get<2>(point_undet));
		}
		catch (bad_numeric_cast &e) {
			glogger
			<< "In GDataCollector3T::operator&(const boost::tuple<S,T,U>&): Error!" << std::endl
			<< "Encountered invalid cast with boost::numeric_cast," << std::endl
			<< "with the message " << std::endl
			<< e.what() << std::endl
			<< GEXCEPTION;
		}

		data_.push_back(boost::tuple<x_type, y_type, z_type>(x, y, z));
	}

	/***************************************************************************/
	/**
	 * Allows to add data of type boost::tuple<x_type, y_type, z_type> to the collection
	 * in an intuitive way.
	 *
	 * @param point The data item to be added to the collection
	 */
	void operator&(const boost::tuple<x_type, y_type, z_type> &point) {
		// Add the data item to the collection
		data_.push_back(point);
	}

	/***************************************************************************/
	/**
	 * Allows to add a collection of data items of undetermined type to the
	 * collection in an intuitive way, provided they can be converted safely
	 * to the target type.
	 *
	 * @param point_vec_undet The collection of data items to be added to the collection
	 */
	template<typename x_type_undet, typename y_type_undet, typename z_type_undet>
	void operator&(const std::vector<boost::tuple<x_type_undet, y_type_undet, z_type_undet>> &point_vec_undet) {
		using boost::numeric::bad_numeric_cast;

		x_type x = x_type(0);
		y_type y = y_type(0);
		z_type z = z_type(0);

		typename std::vector<boost::tuple<x_type_undet, y_type_undet, z_type_undet>>::const_iterator cit;
		for (cit = point_vec_undet.begin(); cit != point_vec_undet.end(); ++cit) {
			// Make sure the data can be converted to doubles
			try {
				x = boost::numeric_cast<x_type>(boost::get<0>(*cit));
				y = boost::numeric_cast<y_type>(boost::get<1>(*cit));
				z = boost::numeric_cast<z_type>(boost::get<2>(*cit));
			}
			catch (bad_numeric_cast &e) {
				glogger
				<< "In GDataCollector3T::operator&(const std::vector<boost::tuple<S,T,U>>&): Error!" << std::endl
				<< "Encountered invalid cast with boost::numeric_cast," << std::endl
				<< "with the message " << std::endl
				<< e.what() << std::endl
				<< GEXCEPTION;
			}

			data_.push_back(boost::tuple<x_type, y_type, z_type>(x, y, z));
		}
	}

	/***************************************************************************/
	/**
	 * Allows to add a collection of data items of type boost::tuple<x_type, y_type, z_type>
	 * to the collection in an intuitive way, provided they can be converted safely
	 * to the target type.
	 *
	 * @param point_vec The collection of data items to be added to the collection
	 */
	void operator&(const std::vector<boost::tuple<x_type, y_type, z_type>> &point_vec) {
		typename std::vector<boost::tuple<x_type, y_type, z_type>>::const_iterator cit;
		for (cit = point_vec.begin(); cit != point_vec.end(); ++cit) {
			// Add the data item to the collection
			data_.push_back(*cit);
		}
	}

protected:
	/***************************************************************************/
	/**
	 * Loads the data of another object
	 */
	virtual void load_(const GBasePlotter* cp) {
		// Check that we are dealing with a GDataCollector3T<x_type, y_type, z_type> reference independent of this object and convert the pointer
		const GDataCollector3T<x_type, y_type, z_type> *p_load = Gem::Common::g_convert_and_compare<GBasePlotter, GDataCollector3T<x_type, y_type, z_type>>(cp, this);

		// Load our parent class'es data ...
		GBasePlotter::load_(cp);

		// ... and then our own
		data_ = p_load->data_; // This assumes that x_type is POD
	}

	/***************************************************************************/
	/** @brief Creates a deep clone of this object */
	virtual G_API_COMMON GBasePlotter* clone_() const = 0;

	/***************************************************************************/

	std::vector<boost::tuple<x_type, y_type, z_type>> data_; ///< Holds the actual data
};


/******************************************************************************/
/**
 * Specialization of projectX for <x_type, y_type, z_type> = <double, double, double>, that will return a
 * GHistogram1D object, wrapped into a std::shared_ptr<GHistogram1D>. In case of a
 * default-constructed range, the function will attempt to determine suitable parameters
 * for the range settings.
 *
 * @param nBins The number of bins of the histogram
 * @param range The minimum and maximum boundaries of the histogram
 */
template<>
inline
std::shared_ptr <GDataCollector1T<double>>
GDataCollector3T<double, double, double>::projectX(std::size_t nBinsX, boost::tuple<double, double> rangeX) const {
	boost::tuple<double, double> myRangeX;
	if (rangeX == boost::tuple<double, double>()) {
		// Find out about the minimum and maximum values in the data_ array
		boost::tuple<double, double, double, double, double, double> extremes = Gem::Common::getMinMax(this->data_);
		myRangeX = boost::tuple<double, double>(boost::get<0>(extremes), boost::get<1>(extremes));
	} else {
		myRangeX = rangeX;
	}

	// Construct the result object
	std::shared_ptr <GHistogram1D> result(new GHistogram1D(nBinsX, myRangeX));
	result->setXAxisLabel(this->xAxisLabel());
	result->setYAxisLabel("Number of entries");
	result->setPlotLabel(this->plotLabel() + " / x-projection");

	// Add data to the object
	for (std::size_t i = 0; i < data_.size(); i++) {
		(*result) & boost::get<0>(data_.at(i));
	}

	// Return the data
	return result;
}

/******************************************************************************/
/**
 * Specialization of projectY for <x_type, y_type, z_type> = <double, double, double>, that will return a
 * GHistogram1D object, wrapped into a std::shared_ptr<GHistogram1D>. In case of a
 * default-constructed range, the function will attempt to determine suitable parameters
 * for the range settings.
 *
 * @param nBins The number of bins of the histogram
 * @param range The minimum and maximum boundaries of the histogram
 */
template<>
inline
std::shared_ptr <GDataCollector1T<double>>
GDataCollector3T<double, double, double>::projectY(std::size_t nBinsY, boost::tuple<double, double> rangeY) const {
	boost::tuple<double, double> myRangeY;
	if (rangeY == boost::tuple<double, double>()) {
		// Find out about the minimum and maximum values in the data_ array
		boost::tuple<double, double, double, double, double, double> extremes = Gem::Common::getMinMax(data_);
		myRangeY = boost::tuple<double, double>(boost::get<2>(extremes), boost::get<3>(extremes));
	} else {
		myRangeY = rangeY;
	}

	// Construct the result object
	std::shared_ptr <GHistogram1D> result(new GHistogram1D(nBinsY, myRangeY));
	result->setXAxisLabel(this->yAxisLabel());
	result->setYAxisLabel("Number of entries");
	result->setPlotLabel(this->plotLabel() + " / y-projection");

	// Add data to the object
	for (std::size_t i = 0; i < data_.size(); i++) {
		(*result) & boost::get<1>(data_.at(i));
	}

	// Return the data
	return result;
}

/******************************************************************************/
/**
 * Specialization of projectZ for <x_type, y_type, z_type> = <double, double, double>, that will return a
 * GHistogram1D object, wrapped into a std::shared_ptr<GHistogram1D>. In case of a
 * default-constructed range, the function will attempt to determine suitable parameters
 * for the range settings.
 *
 * @param nBins The number of bins of the histogram
 * @param range The minimum and maximum boundaries of the histogram
 */
template<>
inline
std::shared_ptr <GDataCollector1T<double>>
GDataCollector3T<double, double, double>::projectZ(std::size_t nBinsZ, boost::tuple<double, double> rangeZ) const {
	boost::tuple<double, double> myRangeZ;
	if (rangeZ == boost::tuple<double, double>()) {
		// Find out about the minimum and maximum values in the data_ array
		boost::tuple<double, double, double, double, double, double> extremes = Gem::Common::getMinMax(data_);
		myRangeZ = boost::tuple<double, double>(boost::get<4>(extremes), boost::get<5>(extremes));
	} else {
		myRangeZ = rangeZ;
	}

	// Construct the result object
	std::shared_ptr <GHistogram1D> result(new GHistogram1D(nBinsZ, myRangeZ));
	result->setXAxisLabel(this->zAxisLabel());
	result->setYAxisLabel("Number of entries");
	result->setPlotLabel(this->plotLabel() + " / z-projection");

	// Add data to the object
	for (std::size_t i = 0; i < data_.size(); i++) {
		(*result) & boost::get<2>(data_.at(i));
	}

	// Return the data
	return result;
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A wrapper for the ROOT TGraph2D class (3d data). It
 * also adds the option to draw lines between consecutive points. This class
 * only allows a single plot mode.
 */
class GGraph3D : public GDataCollector3T<double, double, double> {

	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int){
		using boost::serialization::make_nvp;

		ar
		& make_nvp("GDataCollector3T_3double", boost::serialization::base_object<GDataCollector3T<double, double, double>>(*this))
		& BOOST_SERIALIZATION_NVP(drawLines_);
	}
	///////////////////////////////////////////////////////////////////////

public:
	/** @brief The default constructor */
	G_API_COMMON GGraph3D();

	/** @brief A copy constructor */
	G_API_COMMON GGraph3D(const GGraph3D &);

	/** @brief The destructor */
	virtual G_API_COMMON ~GGraph3D();

	/** @brief The assignment operator */
	G_API_COMMON const GGraph3D &operator=(const GGraph3D &);

	/** @brief Adds lines to the plots between consecutive points */
	G_API_COMMON void setDrawLines(bool= true);
	/** @brief Retrieves the value of the drawLines_ variable */
	G_API_COMMON bool getDrawLines() const;

	/** @brief Retrieves a unique name for this plotter */
	virtual G_API_COMMON std::string getPlotterName() const;

protected:
	/** @brief Retrieve specific header settings for this plot */
	virtual std::string headerData(bool, std::size_t) const;

	/** @brief Retrieves the actual data sets */
	virtual std::string bodyData(bool, std::size_t) const;

	/** @brief Retrieves specific draw commands for this plot */
	virtual std::string footerData(bool, std::size_t) const;

	/** @brief Retrieve the current drawing arguments */
	virtual std::string drawingArguments(bool) const;

	/** @brief Loads the data of another object */
	virtual G_API_COMMON void load_(const GBasePlotter*);
	/** @brief Creates a deep clone of this object */
	virtual G_API_COMMON GBasePlotter* clone_() const;

private:
	bool drawLines_; ///< When set to true, lines will be drawn between consecutive points
};

/******************************************************************************/
/**
 * A data collector for 4-d data of user-defined type
 */
template<
	typename x_type, typename y_type, typename z_type, typename w_type
>
class GDataCollector4T
	: public GBasePlotter {

	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int){
		using boost::serialization::make_nvp;

		ar
		& BOOST_SERIALIZATION_BASE_OBJECT_NVP(GBasePlotter)
		& BOOST_SERIALIZATION_NVP(data_);
	}
	///////////////////////////////////////////////////////////////////////

public:
	/***************************************************************************/
	/**
	 * The default constructor
	 */
	GDataCollector4T()
		: GBasePlotter(), data_() { /* nothing */ }

	/***************************************************************************/
	/**
	 * A copy constructor
	 *
	 * @param cp A copy of another GDataCollector4T object
	 */
	GDataCollector4T(const GDataCollector4T<x_type, y_type, z_type, w_type> &cp)
		: GBasePlotter(cp), data_(cp.data_) { /* nothing */ }

	/***************************************************************************/
	/**
	 * The destructor
	 */
	virtual ~GDataCollector4T() {
		data_.clear();
	}

	/***************************************************************************/
	/**
	 * The assignment operator
	 */
	void operator=(const GDataCollector4T<x_type, y_type, z_type, w_type> &cp) {
		// Assign our parent class'es data
		GBasePlotter::operator=(cp);

		// and then our own
		data_ = cp.data_;
	}

	/***************************************************************************/
	/**
	 * Allows to project the graph into a histogram (x-direction). This function is a
	 * trap to catch calls with un-implemented types. Use the corresponding specializations,
	 * if available.
	 */
	std::shared_ptr <GDataCollector1T<x_type>> projectX(
		std::size_t, boost::tuple<x_type, x_type>
	) const {
		glogger
		<< "In GDataCollector4T<>::projectX(range, nBins): Error!" << std::endl
		<< "Function was called for class with un-implemented types" << std::endl
		<< GEXCEPTION;

		// Make the compiler happy
		return std::shared_ptr<GDataCollector1T<x_type>>();
	}

	/***************************************************************************/
	/**
	 * Allows to project the graph into a histogram (y-direction). This function is a
	 * trap to catch calls with un-implemented types. Use the corresponding specializations,
	 * if available.
	 */
	std::shared_ptr <GDataCollector1T<y_type>> projectY(
		std::size_t, boost::tuple<y_type, y_type>
	) const {
		glogger
		<< "In GDataCollector4T<>::projectY(range, nBins): Error!" << std::endl
		<< "Function was called for class with un-implemented types" << std::endl
		<< GEXCEPTION;

		// Make the compiler happy
		return std::shared_ptr<GDataCollector1T<y_type>>();
	}

	/***************************************************************************/
	/**
	 * Allows to project the graph into a histogram (z-direction). This function is a
	 * trap to catch calls with un-implemented types. Use the corresponding specializations,
	 * if available.
	 */
	std::shared_ptr <GDataCollector1T<z_type>> projectZ(
		std::size_t, boost::tuple<z_type, z_type>
	) const {
		glogger
		<< "In GDataCollector4T<>::projectZ(range, nBins): Error!" << std::endl
		<< "Function was called for class with un-implemented types" << std::endl
		<< GEXCEPTION;

		// Make the compiler happy
		return std::shared_ptr<GDataCollector1T<z_type>>();
	}

	/***************************************************************************/
	/**
	 * Allows to project the graph into a histogram (w-direction). This function is a
	 * trap to catch calls with un-implemented types. Use the corresponding specializations,
	 * if available.
	 */
	std::shared_ptr <GDataCollector1T<w_type>> projectW(
		std::size_t, boost::tuple<w_type, w_type>
	) const {
		glogger
		<< "In GDataCollector4T<>::projectZ(range, nBins): Error!" << std::endl
		<< "Function was called for class with un-implemented types" << std::endl
		<< GEXCEPTION;

		// Make the compiler happy
		return std::shared_ptr<GDataCollector1T<w_type>>();
	}

	/***************************************************************************/
	/**
	 * This very simple functions allows derived classes
	 * to add data easily to their data sets, when called through a
	 * pointer. I.e., this makes object_ptr->add(data) instead of
	 * *object_ptr & data possible.
	 */
	template<typename data_type>
	void add(const data_type &item) {
		*this & item;
	}

	/***************************************************************************/
	/**
	 * Allows to add data of undetermined type to the collection in an intuitive way,
	 * provided that it can be converted safely to the target type.
	 *
	 * @param point_undet The data item to be added to the collection
	 */
	template<
		typename x_type_undet, typename y_type_undet, typename z_type_undet, typename w_type_undet
	>
	void operator&(const boost::tuple<x_type_undet, y_type_undet, z_type_undet, w_type_undet> &point_undet) {
		using boost::numeric::bad_numeric_cast;

		x_type x = x_type(0);
		y_type y = y_type(0);
		z_type z = z_type(0);
		w_type w = w_type(0);

		// Make sure the data can be converted to doubles
		try {
			x = boost::numeric_cast<x_type>(boost::get<0>(point_undet));
			y = boost::numeric_cast<y_type>(boost::get<1>(point_undet));
			z = boost::numeric_cast<z_type>(boost::get<2>(point_undet));
			w = boost::numeric_cast<w_type>(boost::get<3>(point_undet));
		}
		catch (bad_numeric_cast &e) {
			glogger
			<< "In GDataCollector4T::operator&(const boost::tuple<S,T,U,W>&): Error!" << std::endl
			<< "Encountered invalid cast with boost::numeric_cast," << std::endl
			<< "with the message " << std::endl
			<< e.what() << std::endl
			<< GEXCEPTION;
		}

		data_.push_back(boost::tuple<x_type, y_type, z_type, w_type>(x, y, z, w));
	}

	/***************************************************************************/
	/**
	 * Allows to add data of type boost::tuple<x_type, y_type, z_type> to the collection
	 * in an intuitive way.
	 *
	 * @param point The data item to be added to the collection
	 */
	void operator&(const boost::tuple<x_type, y_type, z_type, w_type> &point) {
		// Add the data item to the collection
		data_.push_back(point);
	}

	/***************************************************************************/
	/**
	 * Allows to add a collection of data items of undetermined type to the
	 * collection in an intuitive way, provided they can be converted safely
	 * to the target type.
	 *
	 * @param point_vec_undet The collection of data items to be added to the collection
	 */
	template<
		typename x_type_undet, typename y_type_undet, typename z_type_undet, typename w_type_undet
	>
	void operator&(
		const std::vector<boost::tuple<x_type_undet, y_type_undet, z_type_undet, w_type_undet>> &point_vec_undet) {
		using boost::numeric::bad_numeric_cast;

		x_type x = x_type(0);
		y_type y = y_type(0);
		z_type z = z_type(0);
		w_type w = w_type(0);

		typename std::vector<boost::tuple<x_type_undet, y_type_undet, z_type_undet>>::const_iterator cit;
		for (cit = point_vec_undet.begin(); cit != point_vec_undet.end(); ++cit) {
			// Make sure the data can be converted to doubles
			try {
				x = boost::numeric_cast<x_type>(boost::get<0>(*cit));
				y = boost::numeric_cast<y_type>(boost::get<1>(*cit));
				z = boost::numeric_cast<z_type>(boost::get<2>(*cit));
				w = boost::numeric_cast<w_type>(boost::get<3>(*cit));
			}
			catch (bad_numeric_cast &e) {
				glogger
				<< "In GDataCollector4T::operator&(const std::vector<boost::tuple<S,T,U,W>>&): Error!" << std::endl
				<< "Encountered invalid cast with boost::numeric_cast," << std::endl
				<< "with the message " << std::endl
				<< e.what() << std::endl
				<< GEXCEPTION;
			}

			data_.push_back(boost::tuple<x_type, y_type, z_type, w_type>(x, y, z, w));
		}
	}

	/***************************************************************************/
	/**
	 * Allows to add a collection of data items of type boost::tuple<x_type, y_type, z_type, w_type>
	 * to the collection in an intuitive way, provided they can be converted safely
	 * to the target type.
	 *
	 * @param point_vec The collection of data items to be added to the collection
	 */
	void operator&(const std::vector<boost::tuple<x_type, y_type, z_type, w_type>> &point_vec) {
		typename std::vector<boost::tuple<x_type, y_type, z_type, w_type>>::const_iterator cit;
		for (cit = point_vec.begin(); cit != point_vec.end(); ++cit) {
			// Add the data item to the collection
			data_.push_back(*cit);
		}
	}

protected:
	/***************************************************************************/
	/**
	 * Loads the data of another object
	 */
	virtual void load_(const GBasePlotter* cp) {
		// Check that we are dealing with a GDataCollector4T<x_type, y_type, z_type, w_type> reference independent of this object and convert the pointer
		const GDataCollector4T<x_type, y_type, z_type, w_type> *p_load = Gem::Common::g_convert_and_compare<GBasePlotter, GDataCollector4T<x_type, y_type, z_type, w_type>>(cp, this);

		// Load our parent class'es data ...
		GBasePlotter::load_(cp);

		// ... and then our own
		data_ = p_load->data_; // This assumes that x_type is POD
	}

	/***************************************************************************/
	/** @brief Creates a deep clone of this object */
	virtual G_API_COMMON GBasePlotter* clone_() const = 0;

	/***************************************************************************/

	std::vector<boost::tuple<x_type, y_type, z_type, w_type>> data_; ///< Holds the actual data
};

/******************************************************************************/
/**
 * Specialization of projectX for <x_type, y_type, z_type, w_type> = <double, double, double, double>,
 * that will return a GHistogram1D object, wrapped into a std::shared_ptr<GHistogram1D>. In case of a
 * default-constructed range, the function will attempt to determine suitable parameters
 * for the range settings.
 *
 * @param nBins The number of bins of the histogram
 * @param range The minimum and maximum boundaries of the histogram
 */
template<>
inline
std::shared_ptr <GDataCollector1T<double>>
GDataCollector4T<double, double, double, double>::projectX(std::size_t nBinsX,
																			  boost::tuple<double, double> rangeX) const {
	boost::tuple<double, double> myRangeX;
	if (rangeX == boost::tuple<double, double>()) {
		// Find out about the minimum and maximum values in the data_ array
		boost::tuple<double, double, double, double, double, double, double, double> extremes = Gem::Common::getMinMax(
			this->data_);
		myRangeX = boost::tuple<double, double>(boost::get<0>(extremes), boost::get<1>(extremes));
	} else {
		myRangeX = rangeX;
	}

	// Construct the result object
	std::shared_ptr <GHistogram1D> result(new GHistogram1D(nBinsX, myRangeX));
	result->setXAxisLabel(this->xAxisLabel());
	result->setYAxisLabel("Number of entries");
	result->setPlotLabel(this->plotLabel() + " / x-projection");

	// Add data to the object
	for (std::size_t i = 0; i < data_.size(); i++) {
		(*result) & boost::get<0>(data_.at(i));
	}

	// Return the data
	return result;
}

/******************************************************************************/
/**
 * Specialization of projectY for <x_type, y_type, z_type, w_type> = <double, double, double, double>,
 * that will return a GHistogram1D object, wrapped into a std::shared_ptr<GHistogram1D>. In case of a
 * default-constructed range, the function will attempt to determine suitable parameters
 * for the range settings.
 *
 * @param nBins The number of bins of the histogram
 * @param range The minimum and maximum boundaries of the histogram
 */
template<>
inline
std::shared_ptr <GDataCollector1T<double>>
GDataCollector4T<double, double, double, double>::projectY(std::size_t nBinsY,
																			  boost::tuple<double, double> rangeY) const {
	boost::tuple<double, double> myRangeY;
	if (rangeY == boost::tuple<double, double>()) {
		// Find out about the minimum and maximum values in the data_ array
		boost::tuple<double, double, double, double, double, double, double, double> extremes = Gem::Common::getMinMax(
			this->data_);
		myRangeY = boost::tuple<double, double>(boost::get<2>(extremes), boost::get<3>(extremes));
	} else {
		myRangeY = rangeY;
	}

	// Construct the result object
	std::shared_ptr <GHistogram1D> result(new GHistogram1D(nBinsY, myRangeY));
	result->setXAxisLabel(this->yAxisLabel());
	result->setYAxisLabel("Number of entries");
	result->setPlotLabel(this->plotLabel() + " / y-projection");

	// Add data to the object
	for (std::size_t i = 0; i < data_.size(); i++) {
		(*result) & boost::get<1>(data_.at(i));
	}

	// Return the data
	return result;
}

/******************************************************************************/
/**
 * Specialization of projectZ for <x_type, y_type, z_type, w_type> = <double, double, double, double>,
 * that will return a GHistogram1D object, wrapped into a std::shared_ptr<GHistogram1D>. In case of a
 * default-constructed range, the function will attempt to determine suitable parameters
 * for the range settings.
 *
 * @param nBins The number of bins of the histogram
 * @param range The minimum and maximum boundaries of the histogram
 */
template<>
inline
std::shared_ptr <GDataCollector1T<double>>
GDataCollector4T<double, double, double, double>::projectZ(std::size_t nBinsZ,
																			  boost::tuple<double, double> rangeZ) const {
	boost::tuple<double, double> myRangeZ;
	if (rangeZ == boost::tuple<double, double>()) {
		// Find out about the minimum and maximum values in the data_ array
		boost::tuple<double, double, double, double, double, double, double, double> extremes = Gem::Common::getMinMax(
			this->data_);
		myRangeZ = boost::tuple<double, double>(boost::get<4>(extremes), boost::get<5>(extremes));
	} else {
		myRangeZ = rangeZ;
	}


	// Construct the result object
	std::shared_ptr <GHistogram1D> result(new GHistogram1D(nBinsZ, myRangeZ));
	result->setXAxisLabel(this->zAxisLabel());
	result->setYAxisLabel("Number of entries");
	result->setPlotLabel(this->plotLabel() + " / z-projection");

	// Add data to the object
	for (std::size_t i = 0; i < data_.size(); i++) {
		(*result) & boost::get<2>(data_.at(i));
	}

	// Return the data
	return result;
}

/******************************************************************************/
/**
 * Specialization of projectZ for <x_type, y_type, z_type, w_type> = <double, double, double, double>,
 * that will return a GHistogram1D object, wrapped into a std::shared_ptr<GHistogram1D>. In case of a
 * default-constructed range, the function will attempt to determine suitable parameters
 * for the range settings.
 *
 * @param nBins The number of bins of the histogram
 * @param range The minimum and maximum boundaries of the histogram
 */
template<>
inline
std::shared_ptr <GDataCollector1T<double>>
GDataCollector4T<double, double, double, double>::projectW(std::size_t nBinsW,
																			  boost::tuple<double, double> rangeW) const {
	boost::tuple<double, double> myRangeW;
	if (rangeW == boost::tuple<double, double>()) {
		// Find out about the minimum and maximum values in the data_ array
		boost::tuple<double, double, double, double, double, double, double, double> extremes = Gem::Common::getMinMax(
			this->data_);
		myRangeW = boost::tuple<double, double>(boost::get<6>(extremes), boost::get<7>(extremes));
	} else {
		myRangeW = rangeW;
	}


	// Construct the result object
	std::shared_ptr <GHistogram1D> result(new GHistogram1D(nBinsW, myRangeW));
	result->setXAxisLabel("w");
	result->setYAxisLabel("Number of entries");
	result->setPlotLabel(this->plotLabel() + " / w-projection");

	// Add data to the object
	for (std::size_t i = 0; i < data_.size(); i++) {
		(*result) & boost::get<3>(data_.at(i));
	}

	// Return the data
	return result;
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A wrapper for the ROOT TPolyMarker3D class, intended for 4D data. The fourth
 * data component is represented as the size of the markers. The class will by
 * default only draw a selection of items.
 */
class GGraph4D
	: public GDataCollector4T<double, double, double, double> {

	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int){
		using boost::serialization::make_nvp;

		ar
		& make_nvp("GDataCollector4T_4double", boost::serialization::base_object<GDataCollector4T<double, double, double, double>>(*this))
		& BOOST_SERIALIZATION_NVP(minMarkerSize_)
		& BOOST_SERIALIZATION_NVP(maxMarkerSize_)
		& BOOST_SERIALIZATION_NVP(smallWLargeMarker_)
		& BOOST_SERIALIZATION_NVP(nBest_);
	}
	///////////////////////////////////////////////////////////////////////

public:
	/** @brief The default constructor */
	G_API_COMMON GGraph4D();

	/** @brief A copy constructor */
	G_API_COMMON GGraph4D(const GGraph4D &);

	/** @brief The destructor */
	virtual G_API_COMMON ~GGraph4D();

	/** @brief The assignment operator */
	G_API_COMMON const GGraph4D &operator=(const GGraph4D &);

	/** @brief Allows to set the minimum marker size */
	G_API_COMMON void setMinMarkerSize(const double &);
	/** @brief Allows to set the maximum marker size */
	G_API_COMMON void setMaxMarkerSize(const double &);

	/** @brief Allows to retrieve the minimum marker size */
	G_API_COMMON double getMinMarkerSize() const;
	/** @brief Allows to retrieve the maximum marker size */
	G_API_COMMON double getMaxMarkerSize() const;

	/** @brief Allows to specify whether small w yield large markers */
	G_API_COMMON void setSmallWLargeMarker(const bool &);
	/** @brief Allows to check whether small w yield large markers */
	G_API_COMMON bool getSmallWLargeMarker() const;

	/** @brief Retrieves a unique name for this plotter */
	G_API_COMMON virtual std::string getPlotterName() const;

	/** @brief Allows to set the number of solutions the class should show */
	G_API_COMMON void setNBest(const std::size_t &);
	/** @brief Allows to retrieve the number of solutions the class should show */
	G_API_COMMON std::size_t getNBest() const;

protected:
	/** @brief Retrieve specific header settings for this plot */
	virtual std::string headerData(bool, std::size_t) const;

	/** @brief Retrieves the actual data sets */
	virtual std::string bodyData(bool, std::size_t) const;

	/** @brief Retrieves specific draw commands for this plot */
	virtual std::string footerData(bool, std::size_t) const;

	/** @brief Retrieve the current drawing arguments */
	virtual std::string drawingArguments(bool) const;

	/** @brief Loads the data of another object */
	virtual G_API_COMMON void load_(const GBasePlotter*);
	/** @brief Creates a deep clone of this object */
	virtual G_API_COMMON GBasePlotter* clone_() const;

private:
	double minMarkerSize_; ///< The minimum allowed size of the marker
	double maxMarkerSize_; ///< The maximum allowed size of the marker

	bool smallWLargeMarker_; ///< Indicated whether a small w value yields a large marker

	std::size_t nBest_; ///< Determines the number of items the class should show
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A wrapper for the ROOT TF1 1d-function plotter
 */
class GFunctionPlotter1D
	: public GBasePlotter {

	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int){
		using boost::serialization::make_nvp;

		ar
		& BOOST_SERIALIZATION_BASE_OBJECT_NVP(GBasePlotter)
		& BOOST_SERIALIZATION_NVP(functionDescription_)
		& BOOST_SERIALIZATION_NVP(xExtremes_)
		& BOOST_SERIALIZATION_NVP(nSamplesX_);
	}
	///////////////////////////////////////////////////////////////////////

public:
	/** @brief The standard constructor */
	G_API_COMMON GFunctionPlotter1D(
		const std::string &, const boost::tuple<double, double> &
	);

	/** @brief A copy constructor */
	G_API_COMMON GFunctionPlotter1D(const GFunctionPlotter1D &);

	/** @brief The destructor */
	virtual G_API_COMMON ~GFunctionPlotter1D();

	/** @brief The assignment operator */
	G_API_COMMON const GFunctionPlotter1D &operator=(const GFunctionPlotter1D &);

	/** @brief Allows to set the number of sampling points in x-direction */
	G_API_COMMON void setNSamplesX(std::size_t);

	/** @brief Retrieves a unique name for this plotter */
	virtual G_API_COMMON std::string getPlotterName() const;

protected:
	/** @brief Retrieve specific header settings for this plot */
	virtual std::string headerData(bool, std::size_t) const;

	/** @brief Retrieves the actual data sets */
	virtual std::string bodyData(bool, std::size_t) const;

	/** @brief Retrieves specific draw commands for this plot */
	virtual std::string footerData(bool, std::size_t) const;

	/** @brief Retrieve the current drawing arguments */
	virtual std::string drawingArguments(bool) const;

	/** @brief Loads the data of another object */
	virtual G_API_COMMON void load_(const GBasePlotter*);
	/** @brief Creates a deep clone of this object */
	virtual G_API_COMMON GBasePlotter* clone_() const;

private:
	GFunctionPlotter1D() = delete; ///< The default constructor

	std::string functionDescription_;

	boost::tuple<double, double> xExtremes_; ///< Minimum and maximum values for the x-axis
	std::size_t nSamplesX_; ///< The number of sampling points of the function
};

/******************************************************************************/
/**
 * A wrapper for the ROOT TF2 2d-function plotter
 */
class GFunctionPlotter2D
	: public GBasePlotter {

	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int){
		using boost::serialization::make_nvp;

		ar
		& BOOST_SERIALIZATION_BASE_OBJECT_NVP(GBasePlotter)
		& BOOST_SERIALIZATION_NVP(functionDescription_)
		& BOOST_SERIALIZATION_NVP(xExtremes_)
		& BOOST_SERIALIZATION_NVP(yExtremes_)
		& BOOST_SERIALIZATION_NVP(nSamplesX_)
		& BOOST_SERIALIZATION_NVP(nSamplesY_);
	}
	///////////////////////////////////////////////////////////////////////

public:
	/** @brief The standard constructor */
	G_API_COMMON GFunctionPlotter2D(
		const std::string &, const boost::tuple<double, double> &, const boost::tuple<double, double> &
	);

	/** @brief A copy constructor */
	G_API_COMMON GFunctionPlotter2D(const GFunctionPlotter2D &);

	/** @brief The destructor */
	virtual G_API_COMMON ~GFunctionPlotter2D();

	/** @brief The assignment operator */
	G_API_COMMON const GFunctionPlotter2D &operator=(const GFunctionPlotter2D &);

	/** @brief Allows to set the number of sampling points in x-direction */
	G_API_COMMON void setNSamplesX(std::size_t);
	/** @brief Allows to set the number of sampling points in y-direction */
	G_API_COMMON void setNSamplesY(std::size_t);

	/** @brief Retrieves a unique name for this plotter */
	G_API_COMMON virtual std::string getPlotterName() const;

protected:
	/** @brief Retrieve specific header settings for this plot */
	virtual std::string headerData(bool, std::size_t) const;

	/** @brief Retrieves the actual data sets */
	virtual std::string bodyData(bool, std::size_t) const;

	/** @brief Retrieves specific draw commands for this plot */
	virtual std::string footerData(bool, std::size_t) const;

	/** @brief Retrieve the current drawing arguments */
	virtual std::string drawingArguments(bool) const;

	/** @brief Loads the data of another object */
	virtual G_API_COMMON void load_(const GBasePlotter*);
	/** @brief Creates a deep clone of this object */
	virtual G_API_COMMON GBasePlotter* clone_() const;

private:
	GFunctionPlotter2D() = delete; ///< The default constructor -- intentionally private and undefined

	std::string functionDescription_;

	boost::tuple<double, double> xExtremes_; ///< Minimum and maximum values for the x-axis
	boost::tuple<double, double> yExtremes_; ///< Minimum and maximum values for the y-axis

	std::size_t nSamplesX_; ///< The number of sampling points of the function
	std::size_t nSamplesY_; ///< The number of sampling points of the function
};

/******************************************************************************/
/**
 * A class that outputs a ROOT input file (compare http://root.cern.ch), based
 * on the data providers stored in it.
 */
class GPlotDesigner
	: boost::noncopyable {
public:
	/** @brief The standard constructor */
	G_API_COMMON GPlotDesigner(
		const std::string &, const std::size_t &, const std::size_t &
	);

	/* @brief Emits the overall plot */
	G_API_COMMON std::string plot(const boost::filesystem::path & = boost::filesystem::path("empty")) const;
	/** @brief Writes the plot to a file */
	G_API_COMMON void writeToFile(const boost::filesystem::path &);

	/** @brief Allows to add a new plotter object */
	G_API_COMMON void registerPlotter(std::shared_ptr <GBasePlotter>);

	/** @brief Set the dimensions of the output canvas */
	G_API_COMMON void setCanvasDimensions(const boost::uint32_t &, const boost::uint32_t &);
	/** @brief Set the dimensions of the output canvas */
	G_API_COMMON void setCanvasDimensions(const boost::tuple<boost::uint32_t, boost::uint32_t> &);
	/** @brief Allows to retrieve the canvas dimensions */
	G_API_COMMON boost::tuple<boost::uint32_t, boost::uint32_t> getCanvasDimensions() const;

	/** @brief Allows to set the canvas label */
	G_API_COMMON void setCanvasLabel(const std::string &);
	/** @brief Allows to retrieve the canvas label */
	G_API_COMMON std::string getCanvasLabel() const;

	/** @brief Allows to add a "Print" command to the end of the script so that picture files are created */
	G_API_COMMON void setAddPrintCommand(bool);
	/** @brief Allows to retrieve the current value of the addPrintCommand_ variable */
	G_API_COMMON bool getAddPrintCommand() const;

	/** @brief Resets the plotters */
	G_API_COMMON void resetPlotters();

private:
	/** @brief The default constructor -- intentionally private and undefined */
	GPlotDesigner() = delete;

	/** @brief A header for static data in a ROOT file */
	std::string staticHeader() const;

	std::vector<std::shared_ptr < GBasePlotter>>
	plotters_; ///< A list of plots to be added to the diagram

	std::size_t c_x_div_, c_y_div_; ///< The number of divisions in x- and y-direction
	boost::uint32_t c_x_dim_, c_y_dim_; ///< Holds the number of pixels of the canvas

	std::string canvasLabel_; ///< A label to be assigned to the entire canvas

	bool addPrintCommand_; ///< Indicates whether a print command for the creation of a png file should be added
};

/******************************************************************************/

} /* namespace Common */
} /* namespace Gem */

#endif /* GPLOTDESIGNER_HPP_ */
