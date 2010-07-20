/**
 * @file GParameterValuePair.hpp
 */

/* Copyright (C) Dr. Ruediger Berlich and Karlsruhe Institute of Technology
 * (University of the State of Baden-Wuerttemberg and National Laboratory
 * of the Helmholtz Association)
 *
 * Contact: info [at] gemfony (dot) com
 *
 * This file is part of Geneva, Gemfony scientific's optimization library.
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
#include <cmath>
#include <exception>

// Includes check for correct Boost version(s)
#include "GGlobalDefines.hpp"

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

#ifndef GPARAMETERVALUEPAIR_HPP_
#define GPARAMETERVALUEPAIR_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif

// Geneva headers go here
#include "GNumericParameterT.hpp"
#include "GBoolParameter.hpp"
#include "GDoubleParameter.hpp"
#include "GLongParameter.hpp"
#include "GHelperFunctionsT.hpp"
#include "GExceptions.hpp"

namespace Gem
{
namespace Dataexchange
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
	void serialize(Archive & ar, const unsigned int){
		using boost::serialization::make_nvp;

		ar & BOOST_SERIALIZATION_NVP(dArray_)
		   & BOOST_SERIALIZATION_NVP(lArray_)
		   & BOOST_SERIALIZATION_NVP(bArray_)
		   & BOOST_SERIALIZATION_NVP(value_)
		   & BOOST_SERIALIZATION_NVP(hasValue_);
	}
	///////////////////////////////////////////////////////////////////////

	/** @brief  The standard constructor */
	GParameterValuePair();
	/** @brief A standard copy constructor */
	GParameterValuePair(const GParameterValuePair&);
	/** @brief The destructor Made virtual due to issues described here http://thread.gmane.org/gmane.comp.lib.boost.user/52384/focus=52390 */
	virtual ~GParameterValuePair();

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
	/** @brief Checks whether any data is available locally */
	bool hasData();

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

	double value_; ///< The value of this particular data set, if it has already been assigned
	bool hasValue_; ///< Indicates whether a value has been assigned to the data set
};

/********************************************************************************************/

} /* namespace Dataexchange */
} /* namespace Gem */

#endif /* GPARAMETERVALUEPAIR_HPP_  */
