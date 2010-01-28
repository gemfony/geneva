/**
 * @file GBoundedDouble.hpp
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

// Standard headers go here
#include <vector>
#include <sstream>
#include <cmath>

// Includes check for correct Boost version(s)
#include "GGlobalDefines.hpp"

// Boost headers go here

#include <boost/cstdint.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>
#include <boost/cast.hpp> // For boost::numeric_cast<>
#include <boost/optional.hpp>

#ifndef GBOUNDEDDOUBLE_HPP_
#define GBOUNDEDDOUBLE_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif


// GenEvA headers go here
#include "GBoundedNumT.hpp"

namespace Gem
{
namespace GenEvA
{

  /******************************************************************************/
  /**
   * The GBoundedDouble class allows to limit the value range of a double value,
   * while applying mutations to a continuous range. This is done by means of a
   * mapping from an internal representation to an externally visible value.
   */
class GBoundedDouble
	:public GBoundedNumT<double>
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int){
	  using boost::serialization::make_nvp;

	  ar & make_nvp("GBoundedNumT_double", boost::serialization::base_object<GBoundedNumT<double> >(*this));
	}
	///////////////////////////////////////////////////////////////////////

public:
	/** @brief The default constructor */
	GBoundedDouble();
	/** @brief Initialization with boundaries only */
	GBoundedDouble(const double&, const double&);
	/** @brief Initialization with value and boundaries */
	GBoundedDouble(const double&, const double&, const double&);
	/** @brief The copy constructor */
	GBoundedDouble(const GBoundedDouble&);
	/** @brief Initialization by contained value */
	explicit GBoundedDouble(const double&);
	/** @brief The destructor */
	virtual ~GBoundedDouble();

	/** @brief An assignment operator for the contained value type */
	virtual const double& operator=(const double&);

	/** @brief A standard assignment operator */
	const GBoundedDouble& operator=(const GBoundedDouble&);

	/** @brief Checks for equality with another GBoundedDouble object */
	bool operator==(const GBoundedDouble&) const;
	/** @brief Checks for inequality with another GBoundedDouble object */
	bool operator!=(const GBoundedDouble&) const;

	/** @brief Checks whether this object fulfills a given expectation in relation to another object */
	virtual boost::optional<std::string> checkRelationshipWith(const GObject&, const Gem::Util::expectation&, const double&, const std::string&, const std::string&, const bool&) const;

	/** @brief Loads the data of another GObject */
	virtual void load(const GObject* cp);

protected:
	/** @brief Creates a deep clone of this object. */
	virtual GObject* clone_() const;
};

  /******************************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */

#endif /* GBOUNDEDDOUBLE_HPP_ */
