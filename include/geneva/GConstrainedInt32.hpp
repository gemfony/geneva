/**
 * @file GConstrainedInt32.hpp
 */
/*
 * Copyright (C) Authors of the Geneva library collection and Karlsruhe
 * Institute of Technology (University of the State of Baden-Wuerttemberg
 * and National Laboratory of the Helmholtz Association).
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: info [at] gemfony (dot) com
 *
 * This file is part of the Geneva library collection
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
#include "common/GGlobalDefines.hpp"

// Boost headers go here

#include <boost/cstdint.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>
#include <boost/cast.hpp> // For boost::numeric_cast<>
#include <boost/cstdint.hpp>

#ifndef GCONSTRAINEDINT32_HPP_
#define GCONSTRAINEDINT32_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif


// Geneva headers go here
#include "GConstrainedIntegerT.hpp"

namespace Gem
{
namespace Geneva
{

/******************************************************************************/
/**
 * The GConstrainedInt32 class allows to limit the value range of a boost::int32_t value,
 * while applying adaptions to a continuous range. This is done by means of a
 * mapping from an internal representation to an externally visible value.
 */
class GConstrainedInt32
  : public GConstrainedIntegerT<boost::int32_t>
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int){
		using boost::serialization::make_nvp;

		ar & make_nvp("GConstrainedIntegerT_int32", boost::serialization::base_object<GConstrainedIntegerT<boost::int32_t> >(*this));
	}
	///////////////////////////////////////////////////////////////////////

public:
	/** @brief The default constructor */
	GConstrainedInt32();
	/** @brief Initialization with boundaries only */
	GConstrainedInt32(const boost::int32_t&, const boost::int32_t&);
	/** @brief Initialization with value and boundaries */
	GConstrainedInt32(const boost::int32_t&, const boost::int32_t&, const boost::int32_t&);
	/** @brief The copy constructor */
	GConstrainedInt32(const GConstrainedInt32&);
	/** @brief Initialization by contained value */
	explicit GConstrainedInt32(const boost::int32_t&);
	/** @brief The destructor */
	virtual ~GConstrainedInt32();

	/** @brief An assignment operator for the contained value type */
	virtual boost::int32_t operator=(const boost::int32_t&);

	/** @brief A standard assignment operator */
	const GConstrainedInt32& operator=(const GConstrainedInt32&);

	/** @brief Checks for equality with another GConstrainedInt32 object */
	bool operator==(const GConstrainedInt32&) const;
	/** @brief Checks for inequality with another GConstrainedInt32 object */
	bool operator!=(const GConstrainedInt32&) const;

	/** @brief Checks whether this object fulfills a given expectation in relation to another object */
	virtual boost::optional<std::string> checkRelationshipWith(const GObject&, const Gem::Common::expectation&, const double&, const std::string&, const std::string&, const bool&) const;

protected:
	/** @brief Loads the data of another GObject */
	virtual void load_(const GObject*);
	/** @brief Creates a deep clone of this object. */
	virtual GObject* clone_() const;

	/** @brief Triggers random initialization of the parameter object */
	virtual void randomInit_();

#ifdef GENEVATESTING
public:
	/** @brief Applies modifications to this object. This is needed for testing purposes */
	virtual bool modify_GUnitTests();
	/** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
	virtual void specificTestsNoFailureExpected_GUnitTests();
	/** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
	virtual void specificTestsFailuresExpected_GUnitTests();
#endif /* GENEVATESTING */
};

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

#endif /* GCONSTRAINEDINT32_HPP_ */
