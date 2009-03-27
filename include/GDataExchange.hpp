/**
 * @file GDataExchange.hpp
 */

/* Copyright (C) 2009 Dr. Ruediger Berlich
 *
 * This file is part of Geneva, Gemfony scientific's optimization library.
 *
 * This file is free software: you can redistribute it and/or modify
 * it under the terms of version 3 of the GNU Affero General Public License,
 * or, at your option, under the terms of version 2 of the GNU General Public
 * License, as published by the Free Software Foundation.
 *
 * NOTE THAT THIS FORM OF DUAL-LICENSING DOES NOT APPLY TO ANY OTHER FILES
 * OF THE GENEVA LIBRARY, UNLESS THIS IS EXPLICITLY STATED IN THE CORRSPONDING FILE.
 *
 * Geneva is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of version 2 of the GNU General Public License
 * and of version 3 of the GNU Affero General Public License along with the Geneva
 * library. If not, see <http://www.gnu.org/licenses/>.
 */

// Standard headers go here
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

// Boost headers go here
#include <boost/version.hpp>

#if BOOST_VERSION < 103600
#error "Error: Boost should at least have version 1.36 !"
#endif /* BOOST_VERSION */

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

#ifndef GDATAEXCHANGE_HPP_
#define GDATAEXCHANGE_HPP_

#include "GNumericParameterT.hpp"

#include "GBoolParameter.hpp"
#include "GCharParameter.hpp"
#include "GDoubleParameter.hpp"
#include "GLongParameter.hpp"

namespace Gem
{
namespace Util
{

/*******************************************************************************/
/**
 * This class serves as an exchange vehicle between external programs and
 * the Geneva library. It allows to store and load parameters particular to a given
 * individual. Particular storage formats can be re-defined in derived classes
 * in order to accommodate "foreign" exchange formats. The class itself only
 * implements a very simple format, where all data is stored as ASCII data
 * consecutively in a file.
 */
class GDataExchange {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template<typename Archive>
    void serialize(Archive & ar, const unsigned int version){
      using boost::serialization::make_nvp;

      ar & make_nvp("dArray_", dArray_);
      ar & make_nvp("lArray_", lArray_);
      ar & make_nvp("bArray_", bArray_);
      ar & make_nvp("cArray_", cArray_);
      ar & make_nvp("value_", value_);
      ar & make_nvp("hasValue_", hasValue_);
    }
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

	/** @brief Resets all data structures of the object */
	void reset();

	/** @brief Assign a value to the current data set */
	void setValue(double);
	/** @brief Retrieve the current value */
	double value();
	/** @brief Check whether the current data set has a value */
	bool hasValue();

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
		std::cout << "In GDataExchange::parameterSet_at<T>(std::size_t): Error!" << std::endl
			      << "The function seems to have been called with a type" << std::endl
			      << "it was not designed for. Leaving ..." << std::endl;
		exit(1);

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
		std::cout << "In GDataExchange::at<T>(std::size_t): Error!" << std::endl
			      << "The function seems to have been called with a type" << std::endl
			      << "it was not designed for. Leaving ..." << std::endl;
		exit(1);

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
		std::cout << "In GDataExchange::size<T>(): Error!" << std::endl
			      << "The function seems to have been called with a type" << std::endl
			      << "it was not designed for. Leaving ..." << std::endl;
		exit(1);

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
		std::cout << "In GDataExchange::append<T>(): Error!" << std::endl
			      << "The function seems to have been called with a type" << std::endl
			      << "it was not designed for. Leaving ..." << std::endl;
		exit(1);
	}

	/**************************************************************************/
	/**
	 * Allows to append data of a given type to the corresponding vector.
	 * This is a trap. The specializations should be used instead.
	 */
	template <typename T>
	void append(const T&, const T&, const T&) {
		std::cout << "In GDataExchange::append<T>(): Error!" << std::endl
			      << "The function seems to have been called with a type" << std::endl
			      << "it was not designed for. Leaving ..." << std::endl;
		exit(1);
	}

	/**************************************************************************/
	/** @brief Adds a boost::shared_ptr<GDoubleParameter> object to the corresponding vector */
	void append(const boost::shared_ptr<GDoubleParameter>&);
	/** @brief Adds a boost::shared_ptr<GLongParameter> object to the corresponding vector */
	void append(const boost::shared_ptr<GLongParameter>&);
	/** @brief Adds a boost::shared_ptr<GBoolParameter> object to the corresponding vector */
	void append(const boost::shared_ptr<GBoolParameter>&);
	/** @brief Adds a boost::shared_ptr<GCharParameter> object to the corresponding vector */
	void append(const boost::shared_ptr<GCharParameter>&);

	/**************************************************************************/

	/** @brief Writes the class'es data to a stream */
	void writeToStream(std::ostream&) const;
	/** @brief Reads the class'es data from a stream */
	void readFromStream(std::istream&);

private:
	/**************************************************************************/
	std::vector<boost::shared_ptr<GDoubleParameter> > dArray_; ///< vector holding double parameter sets
	std::vector<boost::shared_ptr<GLongParameter> > lArray_; ///< vector holding long parameter sets
	std::vector<boost::shared_ptr<GBoolParameter> > bArray_; ///< vector holding boolean parameter sets
	std::vector<boost::shared_ptr<GCharParameter> > cArray_; ///< vector holding character parameter sets

	double value_; ///< The value of this particular data set
	bool hasValue_; ///< Indicates whether a value has been assigned to the data set
};

/**************************************************************************/
// Specializations for the usual cases

/** @brief Gives access to a full data set containing doubles */
template <> boost::shared_ptr<GDoubleParameter> GDataExchange::parameterSet_at<double>(std::size_t);
/** @brief Gives access to a full data set containing boost::int32_ts */
template <> boost::shared_ptr<GLongParameter> GDataExchange::parameterSet_at<boost::int32_t>(std::size_t);
/** @brief Gives access to a full data set containing bools */
template <> boost::shared_ptr<GBoolParameter> GDataExchange::parameterSet_at<bool>(std::size_t);
/** @brief Gives access to a full data set containing chars */
template <> boost::shared_ptr<GCharParameter> GDataExchange::parameterSet_at<char>(std::size_t);

/** @brief Gives access to a parameter of type double */
template <> double GDataExchange::at<double>(std::size_t);
/** @brief Gives access to a parameter of type boost::int32_t */
template <> boost::int32_t GDataExchange::at<boost::int32_t>(std::size_t);
/** @brief Gives access to a parameter of type bool */
template <> bool GDataExchange::at<bool>(std::size_t);
/** @brief Gives access to a parameter of type char */
template <> char GDataExchange::at<char>(std::size_t);

/** @brief Gives access to the size of the dArray_ vector */
template <> std::size_t GDataExchange::size<double>();
/** @brief Gives access to the size of the lArray_ vector */
template <> std::size_t GDataExchange::size<boost::int32_t>();
/** @brief Gives access to the size of the bArray_ vector */
template <> std::size_t GDataExchange::size<bool>();
/** @brief Gives access to the size of the cArray_ vector */
template <> std::size_t GDataExchange::size<char>();

/** @brief Appends a boost::shared_ptr<GDoubleParameter> object without boundaries to the corresponding vector. */
template <> void GDataExchange::append<double>(const double&);
/** @brief Appends a boost::shared_ptr<GLongParameter> object without boundaries to the corresponding vector. */
template <> void GDataExchange::append<boost::int32_t>(const boost::int32_t&);
/** @brief Appends a boost::shared_ptr<GBoolParameter> object without boundaries to the corresponding vector. */
template <> void GDataExchange::append<bool>(const bool&);
/** @brief Appends a boost::shared_ptr<GCharParameter> object without boundaries to the corresponding vector. */
template <> void GDataExchange::append<char>(const char&);

/** @brief  Adds a boost::shared_ptr<GDoubleParameter> object with boundaries to the corresponding array */
template <> void GDataExchange::append<double>(const double&, const double&, const double&);
/** @brief  Adds a boost::shared_ptr<GDoubleParameter> object with boundaries to the corresponding array */
template <> void GDataExchange::append<boost::int32_t>(const boost::int32_t&, const boost::int32_t&, const boost::int32_t&);
/** @brief  Adds a boost::shared_ptr<GDoubleParameter> object with boundaries to the corresponding array */
template <> void GDataExchange::append<char>(const char&, const char&, const char&);

/*************************************************************************/
// IO helper functions

/** @brief Helper function to aid IO  of this data set */
std::ostream& operator<<(std::ostream&, const GDataExchange&);
/** @brief Helper function to aid IO  of this data set */
std::istream& operator>>(std::istream&, GDataExchange&);

/*************************************************************************/

} /* namespace Util */
} /* namespace Gem */

#endif /* GDATAEXCHANGE_HPP_ */

