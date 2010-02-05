/**
 * @file GDouble.hpp
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

// Includes check for correct Boost version(s)
#include "GGlobalDefines.hpp"

// Boost headers go here

#include <boost/cstdint.hpp>

#ifndef GDOUBLE_HPP_
#define GDOUBLE_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif

// GenEvA headers go here

#include "GParameterT.hpp"

namespace Gem {
namespace GenEvA {

/************************************************************************/
/**
 * This class encapsulates a double type. This might appear heavy weight,
 * and indeed for most applications this is not the recommended solution -
 * use the GDoubleCollection class or individual GBoundedDouble objects
 * instead.
 */
class GDouble
	:public GParameterT<double>
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int){
	  using boost::serialization::make_nvp;

	  ar & make_nvp("GParameterT_double", boost::serialization::base_object<GParameterT<double> >(*this));
	}
	///////////////////////////////////////////////////////////////////////

public:
	/** @brief The default constructor */
	GDouble();
	/** @brief The copy constructor */
	GDouble(const GDouble&);
	/** @brief Initialization by contained value */
	explicit GDouble(const double&);
	/** @brief The destructor */
	virtual ~GDouble();

	/** @brief An assignment operator for the contained value type */
	virtual const double& operator=(const double&);

	/** @brief A standard assignment operator */
	const GDouble& operator=(const GDouble&);

	/** @brief Checks for equality with another GDouble object */
	bool operator==(const GDouble&) const;
	/** @brief Checks for inequality with another GDouble object */
	bool operator!=(const GDouble&) const;

	/** @brief Checks whether this object fulfills a given expectation in relation to another object */
	virtual boost::optional<std::string> checkRelationshipWith(const GObject&, const Gem::Util::expectation&, const double&, const std::string&, const std::string&, const bool&) const;

protected:
	/** @brief Loads the data of another GObject */
	virtual void load_(const GObject*);
	/** @brief Creates a deep clone of this object. */
	virtual GObject* clone_() const;
};


/************************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */

#endif /* GDOUBLE_HPP_ */
