/**
 * @file GDataExchange.hpp
 */

/*
 * Copyright (C) Authors of the Geneva library collection and Karlsruhe
 * Institute of Technology (University of the State of Baden-Wuerttemberg
 * and National Laboratory of the Helmholtz Association).
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: info [at] gemfony (dot) com
 *
 * This file is part of the Geneva library collection
 *
 * This file is free software: you can redistribute it and/or modify
 * it under the terms of version 3 of the GNU Affero General Public License,
 * or, at your option, under the terms of version 2 of the GNU General Public
 * License, as published by the Free Software Foundation.
 *
 * NOTE THAT THIS FORM OF DUAL-LICENSING DOES NOT APPLY TO ANY OTHER FILES
 * OF THE GENEVA LIBRARY, UNLESS THIS IS EXPLICITLY STATED IN THE CORRESPONDING FILE.
 *
 * Geneva is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of version 2 of the GNU General Public License
 * and of version 3 of the GNU Affero General Public License along with the Geneva
 * library. If not, see <http://www.gnu.org/licenses/>.
 *
 * For further information on Gemfony scientific and Geneva, visit
 * http://www.gemfony.com .
 */


// Standard headers go here
#include <vector>
#include <algorithm>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <exception>

// Includes check for correct Boost version(s)
#include "common/GGlobalDefines.hpp"

// Boost headers go here

#include <boost/cstdint.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/tracking.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/export.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>

#ifndef GDATAEXCHANGE_HPP_
#define GDATAEXCHANGE_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif

// Geneva headers go here
#include "common/GExceptions.hpp"
#include "common/GHelperFunctionsT.hpp"
#include "GNumericParameterT.hpp"
#include "GBoolParameter.hpp"
#include "GDoubleParameter.hpp"
#include "GLongParameter.hpp"
#include "GParameterValuePair.hpp"

namespace Gem
{
namespace Dataexchange
{

/** @brief Set to the average number of decimal digits of a double number. This will likely be 15. */
const std::streamsize DEFAULTPRECISION=std::numeric_limits< double >::digits10;

/*******************************************************************************/
/**
 * This class serves as an exchange vehicle between external programs and
 * the Geneva library. It allows to store and load parameters particular to a given
 * individual. Particular storage formats can be re-defined in derived classes
 * in order to accommodate "foreign" exchange formats. The class itself only
 * implements a very simple format, where all data is stored in ASCII or binary
 * format consecutively in a file. For most purposes, however, the binary format
 * should suffice.
 */
class GDataExchange {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template<typename Archive>
	void save(Archive & ar, const unsigned int) const {
        using boost::serialization::make_nvp;
        ar & make_nvp("parameterValueSet_", parameterValueSet_);

        std::size_t currentPosition = current_ - parameterValueSet_.begin();
        ar & make_nvp("currentPosition_", currentPosition);
        ar & make_nvp("precision_", precision_);
    }

    template<typename Archive>
    void load(Archive & ar, const unsigned int){
    	using boost::serialization::make_nvp;
    	ar & make_nvp("parameterValueSet_", parameterValueSet_);

    	std::size_t currentPosition;
    	ar & make_nvp("currentPosition_", currentPosition);
    	current_ = parameterValueSet_.begin() + currentPosition;

        ar & make_nvp("precision_", precision_);
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()
    ///////////////////////////////////////////////////////////////////////

public:
	/** @brief The default constructor */
	GDataExchange();
	/** @brief A standard copy constructor */
	GDataExchange(const GDataExchange&);
	/** @brief The destructor */
	virtual ~GDataExchange();

	/** @brief A standard assignment operator */
	const GDataExchange& operator=(const GDataExchange&);

	/** @brief Checks equality with another GDataExchange object */
	bool operator==(const GDataExchange&) const;

	/** @brief Checks inequality with another GDataExchange object */
	bool operator!=(const GDataExchange&) const;

	/** @brief Checks whether this object is similar to another */
	bool isSimilarTo(const GDataExchange&, const double&);

	/** @brief Resets the current parameter set */
	void reset();
	/** @brief Resets all parameter sets in sequence */
	void resetAll();

	/** @brief Sorts the data sets according to their values */
	void sort(const bool& ascending=true);

	/** @brief Switches the iterator to the best data set */
	void switchToBestDataSet(const bool& ascending=true);

	/** @brief Set the precision of ASCII IO of FP numbers */
	void setPrecision(const std::streamsize&);
	/** @brief Retrieves the current precision value */
	std::streamsize getPrecision() const;

	/** @brief Assign a value to the current data set */
	void setValue(double);
	/** @brief Retrieve value of the current data set */
	double value();
	/** @brief Check whether the current data set has a value */
	bool hasValue();

	/** @brief Goes to to the start of the list */
	void gotoStart();
	/** @brief Switches to the next available data set */
	bool nextDataSet();
	/** @brief Adds a new, empty data set and adjusts the counter */
	void newDataSet();

	/** @brief Retrieves the number of data sets in the collection */
	std::size_t nDataSets();
	/** @brief Checks whether any data sets are present */
	bool dataIsAvailable();

	/**************************************************************************/
	/**
	 * Retrieves the number of parameters of particular type.
	 * This is a trap. The specializations should be used instead.
	 */
	template <typename T>
	std::size_t numberOfParameterSets() const {
		std::ostringstream error;
		error << "In GDataExchange::numberOfParameterSets<T>: Error!" << std::endl
			    << "The function seems to have been called with a type" << std::endl
			    << "it was not designed for. Leaving ..." << std::endl;

		throw(Gem::Common::gemfony_error_condition(error.str()));

		// Make the compiler happy
		return 0;
	}

	/**************************************************************************/
	/**
	 * Gives access to a full data set of a particular type, including its
	 * boundaries. This is a trap. The specializations should be used instead.
	 *
	 * @param pos The position of the data access is sought for
	 * @return The data access was sought for
	 */
	template <typename T>
	boost::shared_ptr<GNumericParameterT<T> > parameterSet_at(std::size_t pos) {
		std::ostringstream error;
		error << "In GDataExchange::parameterSet_at<T>(std::size_t): Error!" << std::endl
		         << "The function seems to have been called with a type" << std::endl
		         << "it was not designed for. Leaving ..." << std::endl;

		throw(Gem::Common::gemfony_error_condition(error.str()));

		// Make the compiler happy
		return boost::shared_ptr<GNumericParameterT<T> >(new GNumericParameterT<T>());
	}

	/**************************************************************************/
	/**
	 * Gives access to the data of a particular type.
	 * This is a trap. The specializations should be used instead.
	 *
	 * @param pos The position of the data access is sought for
	 * @return The data access was sought for
	 */
	template <typename T>
	T at(std::size_t pos) {
		std::ostringstream error;
		error << "In GDataExchange::at<T>(std::size_t): Error!" << std::endl
			    << "The function seems to have been called with a type" << std::endl
			    << "it was not designed for. Leaving ..." << std::endl;

		throw(Gem::Common::gemfony_error_condition(error.str()));

		// Make the compiler happy
		return 0.;
	}

	/**************************************************************************/
	/**
	 * Gives access to the size of a vector of a particular type.
	 * This is a trap. The specializations should be used instead.
	 *
	 * @return The size of the vector of a particular type
	 */
	template <typename T>
	std::size_t size() {
		std::ostringstream error;
		error << "In GDataExchange::size<T>(): Error!" << std::endl
			     << "The function seems to have been called with a type" << std::endl
			     << "it was not designed for. Leaving ..." << std::endl;

		throw(Gem::Common::gemfony_error_condition(error.str()));

		// Make the compiler happy
		return (std::size_t)NULL;
	}

	/**************************************************************************/
	/**
	 * Allows to append data of a given type to the corresponding vector.
	 * This is a trap. The specializations should be used instead.
	 */
	template <typename T>
	void append(const T&) {
		std::ostringstream error;
		error << "In GDataExchange::append<T>(): Error!" << std::endl
		        << "The function seems to have been called with a type" << std::endl
			    << "it was not designed for. Leaving ..." << std::endl;

		throw(Gem::Common::gemfony_error_condition(error.str()));
	}

	/**************************************************************************/
	/**
	 * Allows to append data of a given type to the corresponding vector.
	 * This is a trap. The specializations should be used instead.
	 */
	template <typename T>
	void append(const T&, const T&, const T&) {
		std::ostringstream error;
		error << "In GDataExchange::append<T>(): Error!" << std::endl
			    << "The function seems to have been called with a type" << std::endl
			    << "it was not designed for. Leaving ..." << std::endl;

		throw(Gem::Common::gemfony_error_condition(error.str()));
	}

	/**************************************************************************/
	/** @brief Adds a boost::shared_ptr<GDoubleParameter> object to the corresponding vector */
	void append(boost::shared_ptr<GDoubleParameter>);
	/** @brief Adds a boost::shared_ptr<GLongParameter> object to the corresponding vector */
	void append(boost::shared_ptr<GLongParameter>);
	/** @brief Adds a boost::shared_ptr<GBoolParameter> object to the corresponding vector */
	void append(boost::shared_ptr<GBoolParameter>);

	/**************************************************************************/

	/** @brief Writes the class'es data to a stream */
	void writeToStream(std::ostream&) const;
	/** @brief Reads the class'es data from a stream */
	void readFromStream(std::istream&);

	/** @brief Writes the class'es data to a stream in binary mode */
	void binaryWriteToStream(std::ostream&) const;
	/** @brief Reads the class'es data from a stream in binary mode */
	void binaryReadFromStream(std::istream&);

	/** @brief Writes the class'es data to a file in binary or text mode */
	void writeToFile(const std::string&, const bool& binary=true, const std::size_t& nDataSets=0, const bool& ascending=true);
	/** @brief Reads the class'es data to a file in binary or text mode */
	void readFromFile(const std::string&, bool binary=true);

private:
	/** @brief This vector holds the actual data */
	std::vector<boost::shared_ptr<GParameterValuePair> > parameterValueSet_;
	/** @brief An iterator indicating the current position in the vector */
	std::vector<boost::shared_ptr<GParameterValuePair> >::iterator current_;
	/** @brief The precision used for text-based floating point i/o */
	std::streamsize precision_;
};

/**************************************************************************/
// Specializations for the usual cases

/** @brief Gives access to the number of double parameters in the current parameter set */
template <> std::size_t GDataExchange::numberOfParameterSets<double>() const;
/** @brief Gives access to the number of long parameters in the current parameter set */
template <> std::size_t GDataExchange::numberOfParameterSets<boost::int32_t>() const;
/** @brief Gives access to the number of bool parameters in the current parameter set */
template <> std::size_t GDataExchange::numberOfParameterSets<bool>() const;

/** @brief Gives access to a full data set containing doubles */
template <> boost::shared_ptr<GDoubleParameter> GDataExchange::parameterSet_at<double>(std::size_t);
/** @brief Gives access to a full data set containing boost::int32_ts */
template <> boost::shared_ptr<GLongParameter> GDataExchange::parameterSet_at<boost::int32_t>(std::size_t);
/** @brief Gives access to a full data set containing bools */
template <> boost::shared_ptr<GBoolParameter> GDataExchange::parameterSet_at<bool>(std::size_t);

/** @brief Gives access to a parameter of type double */
template <> double GDataExchange::at<double>(std::size_t);
/** @brief Gives access to a parameter of type boost::int32_t */
template <> boost::int32_t GDataExchange::at<boost::int32_t>(std::size_t);
/** @brief Gives access to a parameter of type bool */
template <> bool GDataExchange::at<bool>(std::size_t);

/** @brief Gives access to the size of the dArray_ vector */
template <> std::size_t GDataExchange::size<double>();
/** @brief Gives access to the size of the lArray_ vector */
template <> std::size_t GDataExchange::size<boost::int32_t>();
/** @brief Gives access to the size of the bArray_ vector */
template <> std::size_t GDataExchange::size<bool>();

/** @brief Appends a boost::shared_ptr<GDoubleParameter> object without boundaries to the corresponding vector. */
template <> void GDataExchange::append<double>(const double&);
/** @brief Appends a boost::shared_ptr<GLongParameter> object without boundaries to the corresponding vector. */
template <> void GDataExchange::append<boost::int32_t>(const boost::int32_t&);
/** @brief Appends a boost::shared_ptr<GBoolParameter> object without boundaries to the corresponding vector. */
template <> void GDataExchange::append<bool>(const bool&);

/** @brief  Adds a boost::shared_ptr<GDoubleParameter> object with boundaries to the corresponding array */
template <> void GDataExchange::append<double>(const double&, const double&, const double&);
/** @brief  Adds a boost::shared_ptr<GLongParameter> object with boundaries to the corresponding array */
template <> void GDataExchange::append<boost::int32_t>(const boost::int32_t&, const boost::int32_t&, const boost::int32_t&);
/** @brief  Adds a boost::shared_ptr<GBooleanParameter> object to the corresponding array */
template <> void GDataExchange::append<bool>(const bool&, const bool&, const bool&);

/*************************************************************************/
// IO helper functions

/** @brief Helper function to aid IO  of this data set */
std::ostream& operator<<(std::ostream&, const GDataExchange&);
/** @brief Helper function to aid IO  of this data set */
std::istream& operator>>(std::istream&, GDataExchange&);

/*************************************************************************/

} /* namespace Dataexchange */
} /* namespace Gem */

#endif /* GDATAEXCHANGE_HPP_ */
