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

	/** @brief Save the data of this class to a file */
	virtual bool saveToFile(const std::string&);
	/** @brief Loads the data of this class from a file */
	virtual bool loadFromFile(const std::string& fileName);

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

	/* Specializations of at<T>() function for different types */
	template <> double& at<double>(std::size_t);
	template <> boost::int32_t& at<boost::int32_t>(std::size_t);
	template <> bool& at<bool>(std::size_t);
	template <> char& at<char>(std::size_t);

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

	/* Specializations of the size<T>() function for different types */
	template <> std::size_t size<double>();
	template <> std::size_t size<boost::int32_t>();
	template <> std::size_t size<bool>();
	template <> std::size_t size<char>();

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

	/* Specializations of the append<T>() function for different types */
	template <> void append<double>(const double&);
	template <> void append<boost::int32_t>(const boost::int32_t);
	template <> void append<bool>(const bool&);
	template <> void append<char>(const char&);

	/**************************************************************************/

private:
	/**************************************************************************/
	std::vector<boost::shared_ptr<GDoubleParameter> > dArray_; ///< Array holding double values
	std::vector<boost::shared_ptr<GLongParameter> > lArray_; ///< Array holding long values
	std::vector<boost::shared_ptr<GBoolParameter> > bArray_; ///< Array holding boolean values
	std::vector<boost::shared_ptr<GCharParameter> > cArray_; ///< Array holding character values
};

} /* namespace Util */
} /* namespace Gem */

#endif /* GDATAEXCHANGE_HPP_ */

