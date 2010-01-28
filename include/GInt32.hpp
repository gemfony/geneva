/**
 * @file GInt32.hpp
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

#ifndef GINT32_HPP_
#define GINT32_HPP_

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
 * This class encapsulates a single integer value. This might appear heavy weight,
 * and indeed for most applications this is not the recommended solution -
 * use the GInt32Collection instead.
 *
 * Integers are mutated by the GInt32FlipAdaptor or the GInt32GaussAdaptor in GenEvA.
 * The reason for this class is that there might be applications where one might want different
 * adaptor characteristics for different values. This cannot be done with a GInt32Collection.
 * Plus, having a separate integer class adds some consistency to GenEvA, as other values
 * (most notably doubles) have their own class as well (GBoundedDouble, GDouble).
 */
class GInt32
	:public GParameterT<boost::int32_t>
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int){
	  using boost::serialization::make_nvp;

	  ar & make_nvp("GParameterT_int32", boost::serialization::base_object<GParameterT<boost::int32_t> >(*this));
	}
	///////////////////////////////////////////////////////////////////////

public:
	/** @brief The default constructor */
	GInt32();
	/** @brief The copy constructor */
	GInt32(const GInt32&);
	/** @brief Initialization by contained value */
	explicit GInt32(const boost::int32_t&);
	/** @brief The destructor */
	virtual ~GInt32();

	/** @brief An assignment operator for the contained value type */
	virtual const boost::int32_t& operator=(const boost::int32_t&);

	/** @brief A standard assignment operator */
	const GInt32& operator=(const GInt32&);

	/** @brief Checks for equality with another GInt32 object */
	bool operator==(const GInt32&) const;
	/** @brief Checks for inequality with another GInt32 object */
	bool operator!=(const GInt32&) const;

	/** @brief Checks whether this object fulfills a given expectation in relation to another object */
	virtual boost::optional<std::string> checkRelationshipWith(const GObject&, const Gem::Util::expectation&, const double&, const std::string&, const std::string&, const bool&) const;

	/** @brief Loads the data of another GObject */
	virtual void load(const GObject* cp);

protected:
	/** @brief Creates a deep clone of this object. */
	virtual GObject* clone_() const;
};

/************************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */

#endif /* GINT32_HPP_ */
