/**
 * @file GParameterValuePair.hpp
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
#include <algorithm>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cmath>

// Boost headers go here
#include <boost/version.hpp>
#include "GGlobalDefines.hpp"
#if BOOST_VERSION < ALLOWED_BOOST_VERSION
#error "Error: Boost has incorrect version !"
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
#include <boost/bind.hpp>

#ifndef GPARAMETERVALUEPAIR_HPP_
#define GPARAMETERVALUEPAIR_HPP_

#include "GNumericParameterT.hpp"
#include "GBoolParameter.hpp"
#include "GCharParameter.hpp"
#include "GDoubleParameter.hpp"
#include "GLongParameter.hpp"
#include "GHelperFunctionsT.hpp"

namespace Gem
{
namespace Util
{

/********************************************************************************************/
/**
 * An internal struct used to store a single parameter-value pair
 */
struct GParameterValuePair {
public:
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

	/** @brief  The standard constructor */
	GParameterValuePair();
	/** @brief A standard copy constructor */
	GParameterValuePair(const GParameterValuePair&);
	/** @brief The destructor */
	~GParameterValuePair();

	/** @brief  A standard assignment operator */
	const GParameterValuePair& operator=(const GParameterValuePair&);
	/** @brief Checks equality of this object with another object of the same type.*/

	bool operator==(const GParameterValuePair&) const;
	/** @brief Checks inequality of this object with another object of the same type.*/
	bool operator!=(const GParameterValuePair&) const;

	/** @brief Checks whether this object is similar to another */
	bool isSimilarTo(const GParameterValuePair& cp, const double& limit = 0.) const;

	/** @brief Resets the structure to its initial state */
	void reset();

	/** @brief Gives access to the object's value */
	double value();
	/** @brief Determines whether a value has been set */
	bool hasValue();

	/** @brief Sets the precision of FP IO in ASCII mode */
	void setPrecision(const std::streamsize&);

	/** @brief Writes the class'es data to a stream */
	void writeToStream(std::ostream&) const;
	/** @brief Reads the class'es data from a stream */
	void readFromStream(std::istream&);
	/** @brief Writes the object's data to a stream in binary mode */
	void binaryWriteToStream(std::ostream&) const;
	/** @brief Reads the class'es data from a stream in binary mode.*/
	void binaryReadFromStream(std::istream&);

	/******************************************************************************/

	std::vector<boost::shared_ptr<GDoubleParameter> > dArray_; ///< vector holding double parameter sets
	std::vector<boost::shared_ptr<GLongParameter> > lArray_; ///< vector holding long parameter sets
	std::vector<boost::shared_ptr<GBoolParameter> > bArray_; ///< vector holding boolean parameter sets
	std::vector<boost::shared_ptr<GCharParameter> > cArray_; ///< vector holding character parameter sets

	double value_; ///< The value of this particular data set, if it has already been assigned
	bool hasValue_; ///< Indicates whether a value has been assigned to the data set
};

/********************************************************************************************/

} /* namespace Util */
} /* namespace Gem */

#endif /* GPARAMETERVALUEPAIR_HPP_  */
