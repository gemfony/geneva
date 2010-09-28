/**
 * @file GDoubleObject.hpp
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

// Includes check for correct Boost version(s)
#include "common/GGlobalDefines.hpp"

// Boost headers go here

#include <boost/cstdint.hpp>

#ifndef GDOUBLEOBJECT_HPP_
#define GDOUBLEOBJECT_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif

// Geneva headers go here

#include "GNumFPT.hpp"
#include "GDoubleGaussAdaptor.hpp"

namespace Gem {
namespace Geneva {

/************************************************************************/
/**
 * This class encapsulates a double type. This might appear heavy weight,
 * and indeed for most applications this is not the recommended solution -
 * use the GDoubleCollection class or individual GConstrainedDoubleObject objects
 * instead.
 */
class GDoubleObject
	:public GNumFPT<double>
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int){
	  using boost::serialization::make_nvp;

	  ar & make_nvp("GNumFPT_double", boost::serialization::base_object<GNumFPT<double> >(*this));
	}
	///////////////////////////////////////////////////////////////////////

public:
	/** @brief The default constructor */
	GDoubleObject();
	/** @brief The copy constructor */
	GDoubleObject(const GDoubleObject&);
	/** @brief Initialization by contained value */
	explicit GDoubleObject(const double&);
	/** @brief Random initialization in a given range */
	GDoubleObject(const double&, const double&);
	/** @brief The destructor */
	virtual ~GDoubleObject();

	/** @brief An assignment operator for the contained value type */
	virtual double operator=(const double&);

	/** @brief A standard assignment operator */
	const GDoubleObject& operator=(const GDoubleObject&);

	/** @brief Checks for equality with another GDoubleObject object */
	bool operator==(const GDoubleObject&) const;
	/** @brief Checks for inequality with another GDoubleObject object */
	bool operator!=(const GDoubleObject&) const;

	/** @brief Checks whether this object fulfills a given expectation in relation to another object */
	virtual boost::optional<std::string> checkRelationshipWith(const GObject&, const Gem::Common::expectation&, const double&, const std::string&, const std::string&, const bool&) const;

	/** @brief Attach our local value to the vector. */
	virtual void doubleStreamline(std::vector<double>&) const;
	/** @brief Tell the audience that we own a double value */
	virtual std::size_t countDoubleParameters() const;
	/** @brief Assigns part of a value vector to the parameter */
	virtual void assignDoubleValueVector(const std::vector<double>&, std::size_t&);

protected:
	/** @brief Loads the data of another GObject */
	virtual void load_(const GObject*);
	/** @brief Creates a deep clone of this object. */
	virtual GObject* clone_() const;

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


/************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

#endif /* GDOUBLEOBJECT_HPP_ */
