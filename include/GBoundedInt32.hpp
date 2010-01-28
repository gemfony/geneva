/**
 * @file GBoundedInt32.hpp
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

#ifndef GBOUNDEDINT32_HPP_
#define GBOUNDEDINT32_HPP_

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
 * The GBoundedInt32 class allows to limit the value range of a boost::int32_t value,
 * while applying mutations to a continuous range. This is done by means of a
 * mapping from an internal representation to an externally visible value.
 */
class GBoundedInt32
  : public GBoundedNumT<boost::int32_t>
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int){
		using boost::serialization::make_nvp;

		ar & make_nvp("GBoundedNumT_int32", boost::serialization::base_object<GBoundedNumT<boost::int32_t> >(*this));
	}
	///////////////////////////////////////////////////////////////////////

public:
	/** @brief The default constructor */
	GBoundedInt32();
	/** @brief Initialization with boundaries only */
	GBoundedInt32(const boost::int32_t&, const boost::int32_t&);
	/** @brief Initialization with value and boundaries */
	GBoundedInt32(const boost::int32_t&, const boost::int32_t&, const boost::int32_t&);
	/** @brief The copy constructor */
	GBoundedInt32(const GBoundedInt32&);
	/** @brief Initialization by contained value */
	explicit GBoundedInt32(const boost::int32_t&);
	/** @brief The destructor */
	virtual ~GBoundedInt32();

	/** @brief An assignment operator for the contained value type */
	virtual const boost::int32_t& operator=(const boost::int32_t&);

	/** @brief A standard assignment operator */
	const GBoundedInt32& operator=(const GBoundedInt32&);

	/** @brief Checks for equality with another GBoundedInt32 object */
	bool operator==(const GBoundedInt32&) const;
	/** @brief Checks for inequality with another GBoundedInt32 object */
	bool operator!=(const GBoundedInt32&) const;

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

#endif /* GBOUNDEDINT32_HPP_ */
