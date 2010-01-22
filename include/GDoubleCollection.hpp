/**
 * @file GDoubleCollection.hpp
 */

/* Copyright (C) Dr. Ruediger Berlich and Karlsruhe Institute of Technology
 * (University of the State of Baden-Wuerttemberg and National Laboratory
 * of the Helmholtz Association)
 *
 * Contact: info [at] gemfony (dot) com
 *
 * This file is part of the Geneva library, Gemfony scientific's optimization
 * library.
 *
 * Geneva is free software: you can redistribute it and/or modify
 * it under the terms of version 3 of the GNU Affero General Public License
 * as published by the Free Software Foundation.
 *
 * Geneva is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with the Geneva library.  If not, see <http://www.gnu.org/licenses/>.
 *
 * For further information on Gemfony scientific and Geneva, visit
 * http://www.gemfony.com .
 */

// Standard header files go here
#include <string>
#include <sstream>
#include <vector>

// Includes check for correct Boost version(s)
#include "GGlobalDefines.hpp"

// Boost header files go here

#include <boost/cstdint.hpp>

#ifndef GDOUBLECOLLECTION_HPP_
#define GDOUBLECOLLECTION_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif


// GenEvA header files go here
#include "GNumCollectionT.hpp"


namespace Gem {
namespace GenEvA {

/*****************************************************************************************/
/**
 * A collection of double objects without boundaries
 */
class GDoubleCollection
	:public GNumCollectionT<double>
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int){
	  using boost::serialization::make_nvp;

	  ar & make_nvp("GNumCollectionT_double", boost::serialization::base_object<GNumCollectionT<double> >(*this));
	}
	///////////////////////////////////////////////////////////////////////

public:
	/** @brief The default constructor */
	GDoubleCollection();
	/** @brief The copy constructor */
	GDoubleCollection(const GDoubleCollection&);
	/** @brief Initialization with a number of random values */
	explicit GDoubleCollection(const std::size_t& nval, const double& min, const double& max);
	/** @brief The destructor */
	virtual ~GDoubleCollection();

	/** @brief A standard assignment operator */
	const GDoubleCollection& operator=(const GDoubleCollection&);
	/** @brief Creates a deep clone of this object. */
	virtual GObject* clone() const;

	/** @brief Checks for equality with another GDoubleCollection object */
	bool operator==(const GDoubleCollection&) const;
	/** @brief Checks for inequality with another GDoubleCollection object */
	bool operator!=(const GDoubleCollection&) const;

	/** @brief Checks for equality with another GDoubleCollection object. */
	virtual bool isEqualTo(const GObject& cp, const boost::logic::tribool& expected = boost::logic::indeterminate) const;
	/** @brief Checks for similarity with another GDoubleCollection object. */
	virtual bool isSimilarTo(const GObject& cp, const double& limit, const boost::logic::tribool& expected = boost::logic::indeterminate) const;

	/** @brief Loads the data of another GObject */
	virtual void load(const GObject* cp);
};

/*****************************************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */

#endif /* GDOUBLECOLLECTION_HPP_ */
