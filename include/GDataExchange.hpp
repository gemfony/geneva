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

	/**************************************************************************/
	/**
	 * Gives access to a full data set of a particular type, including its
	 * boundaries. This is a trap. The specializations should be used instead.
	 *
	 * @param pos The position of the data access is sought for
	 * @return The data access was sought for
	 */
	template <typename T>
	boost::shared_ptr<GNumericParameterT<T> > parameterSet_at<T>(std::size_t pos) {
		std::cout << "In GDataExchange::parameterSet_at<T>(std::size_t): Error!" << std::endl
			      << "The function seems to have been called with a type" << std::endl
			      << "it was not designed for. Leaving ..." << std::endl;
		exit(1);
	}

	/**************************************************************************/
	/**
	 * Gives access to a parameter set of type double, including its boundaries.
	 *
	 * @param pos The position of the data access is sought for
	 * @return The data access was sought for
	 */
	template <> boost::shared_ptr<GDoubleParameter> parameterSet_at<double>(std::size_t pos) {
		return dArray_.at(pos);
	}

	/**************************************************************************/
	/**
	 * Gives access to a parameter set of type boost::int32_t, including its boundaries.
	 *
	 * @param pos The position of the data access is sought for
	 * @return The data access was sought for
	 */
	template <> boost::shared_ptr<GLongParameter> parameterSet_at<boost::int32_t>(std::size_t pos) {
		return lArray_.at(pos);
	}

	/**************************************************************************/
	/**
	 * Gives access to a parameter set of type bool, including its boundaries.
	 *
	 * @param pos The position of the data access is sought for
	 * @return The data access was sought for
	 */
	template <> boost::shared_ptr<GBoolParameter> parameterSet_at<bool>(std::size_t pos) {
		return bArray_.at(pos);
	}

	/**************************************************************************/
	/**
	 * Gives access to a parameter set of type char, including its boundaries.
	 *
	 * @param pos The position of the data access is sought for
	 * @return The data access was sought for
	 */
	template <> boost::shared_ptr<GCharParameter> parameterSet_at<char>(std::size_t pos) {
		return cArray_.at(pos);
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
	T& at<T>(std::size_t pos) {
		std::cout << "In GDataExchange::at<T>(std::size_t): Error!" << std::endl
			      << "The function seems to have been called with a type" << std::endl
			      << "it was not designed for. Leaving ..." << std::endl;
		exit(1);
	}

	/**************************************************************************/
	/**
	 * Gives access to a parameter of type double
	 *
	 * @param pos The position of the data access is sought for
	 * @return The data access was sought for
	 */
	template <> double& at<double>(std::size_t pos) {
		return dArray_.at(pos)->getParameter();
	}

	/**************************************************************************/
	/**
	 * Gives access to a parameter of type boost::int32_t
	 *
	 * @param pos The position of the data access is sought for
	 * @return The data access was sought for
	 */
	template <> boost::int32_t& at<boost::int32_t>(std::size_t pos) {
		return lArray_.at(pos)->getParameter();
	}

	/**************************************************************************/
	/**
	 * Gives access to a parameter of type bool
	 *
	 * @param pos The position of the data access is sought for
	 * @return The data access was sought for
	 */
	template <> bool& at<bool>(std::size_t pos) {
		return bArray_.at(pos)->getParameter();
	}

	/**************************************************************************/
	/**
	 * Gives access to a parameter of type char
	 *
	 * @param pos The position of the data access is sought for
	 * @return The data access was sought for
	 */
	template <> char& at<char>(std::size_t pos) {
		return cArray_.at(pos)->getParameter();
	}

	/**************************************************************************/
	/**
	 * Gives access to the size of a vector of a particular type.
	 * This is a trap. The specializations should be used instead.
	 *
	 * @return The size of the vector of a particular type
	 */
	template <typename T>
	std::size_t size<T>() {
		std::cout << "In GDataExchange::size<T>(): Error!" << std::endl
			      << "The function seems to have been called with a type" << std::endl
			      << "it was not designed for. Leaving ..." << std::endl;
		exit(1);
	}

	/**************************************************************************/
	/**
	 * Gives access to the size of the dArray_ vector
	 *
	 * @return The size of the dArray_ vector
	 */
	template <> std::size_t size<double>() {
		return dArray_.size();
	}

	/**************************************************************************/
	/**
	 * Gives access to the size of the lArray_ vector
	 *
	 * @return The size of the lArray_ vector
	 */
	template <> std::size_t size<boost::int32_t>() {
		return lArray_.size();
	}

	/**************************************************************************/
	/**
	 * Gives access to the size of the bArray_ vector
	 *
	 * @return The size of the bArray_ vector
	 */
	template <> std::size_t size<bool>() {
		return bArray_.size();
	}

	/**************************************************************************/
	/**
	 * Gives access to the size of the cArray_ vector
	 *
	 * @return The size of the cArray_ vector
	 */
	template <> std::size_t size<char>() {
		return cArray_.size();
	}

	/**************************************************************************/
	/**
	 * Allows to append data of a given type to the corresponding vector.
	 * This is a trap. The specializations should be used instead.
	 */
	template <typename T>
	void append<T>(const T&) {
		std::cout << "In GDataExchange::append<T>(): Error!" << std::endl
			      << "The function seems to have been called with a type" << std::endl
			      << "it was not designed for. Leaving ..." << std::endl;
		exit(1);
	}

	/**************************************************************************/
	/**
	 * Creates a boost::shared_ptr<GDoubleParameter> object without boundaries
	 * and appends it to the corresponding vector.
	 *
	 * @param x The initial parameter value
	 */
	template <> void append<double>(const double& x) {
		boost::shared_ptr<GDoubleParameter> p(new GDoubleParameter(x));
		dArray_.push_back(p);
	}

	/**************************************************************************/
	/**
	 * Creates a boost::shared_ptr<GLongParameter> object without boundaries
	 * and appends it to the corresponding vector.
	 *
	 * @param x The initial parameter value
	 */
	template <> void append<boost::int32_t>(const boost::int32_t x) {
		boost::shared_ptr<GLongParameter> p(new GLongParameter(x));
		lArray_.push_back(p);
	}

	/**************************************************************************/
	/**
	 * Creates a boost::shared_ptr<GBoolParameter> object without boundaries
	 * and appends it to the corresponding vector.
	 *
	 * @param x The initial parameter value
	 */
	template <> void append<bool>(const bool& x) {
		boost::shared_ptr<GBoolParameter> p(new GBoolParameter(x));
		bArray_.push_back(p);
	}

	/**************************************************************************/
	/**
	 * Creates a boost::shared_ptr<GCharParameter> object without boundaries
	 * and appends it to the corresponding vector.
	 *
	 * @param x The initial parameter value
	 */
	template <> void append<char>(const char& x) {
		boost::shared_ptr<GCharParameter> p(new GCharParameter(x));
		cArray_.push_back(p);
	}

	/**************************************************************************/
	/**
	 * Allows to append data of a given type to the corresponding vector.
	 * This is a trap. The specializations should be used instead.
	 */
	template <typename T>
	void append<T>(const T&, const T&, const T&) {
		std::cout << "In GDataExchange::append<T>(): Error!" << std::endl
			      << "The function seems to have been called with a type" << std::endl
			      << "it was not designed for. Leaving ..." << std::endl;
		exit(1);
	}

	/**************************************************************************/
	/**
	 * Adds a boost::shared_ptr<GDoubleParameter> object to the corresponding array
	 *
	 * @param x The initial value of the double parameter
	 * @param x_l The lower boundary of the double parameter
	 * @param x_u The upper boundary of the double parameter
	 */
	template <> void append<double>(const double& x, const double& x_l, const double& x_u) {
		boost::shared_ptr<GDoubleParameter> p(new GDoubleParameter(x,x_l,x_u));
		dArray_.push_back(p);
	}

	/**************************************************************************/
	/**
	 * Adds a boost::shared_ptr<GLongParameter> object to the corresponding array
	 *
	 * @param x The initial value of the boost::in32_t parameter
	 * @param x_l The lower boundary of the boost::in32_t parameter
	 * @param x_u The upper boundary of the boost::in32_t parameter
	 */
	template <> void append<boost::int32_t>(const boost::int32_t x, const boost::int32_t x_l, const boost::int32_t x_u) {
		boost::shared_ptr<GLongParameter> p(new GLongParameter(x,x_l,x_u));
		lArray_.push_back(p);
	}

	/**************************************************************************/
	/**
	 * Adds a boost::shared_ptr<GCharParameter> object to the corresponding array
	 *
	 * @param x The initial value of the char parameter
	 * @param x_l The lower boundary of the char parameter
	 * @param x_u The upper boundary of the char parameter
	 */
	template <> void append<char>(const char& x, const char& x_l, const char& x_u) {
		boost::shared_ptr<GCharParameter> p(new GCharParameter(x,x_l,x_u));
		cArray_.push_back(p);
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
};

/*************************************************************************/

/** @brief Helper function to aid IO  of this data set */
std::ostream& operator<<(std::ostream&, const GDataExchange&);
/** @brief Helper function to aid IO  of this data set */
std::istream& operator>>(std::istream&, GDataExchange&);

/*************************************************************************/

} /* namespace Util */
} /* namespace Gem */

#endif /* GDATAEXCHANGE_HPP_ */

