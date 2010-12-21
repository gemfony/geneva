/**
 * @file GBooleanObject.hpp
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

// Boost headers go here

#ifndef GBOOLEANOBJECT_HPP_
#define GBOOLEANOBJECT_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif


// Geneva headers go here
#include "geneva/GParameterT.hpp"
#include "geneva/GBooleanAdaptor.hpp"

namespace Gem {
namespace Geneva {

/************************************************************************/
/**
 * This class encapsulates a single bit, represented as a bool. This might appear heavy weight,
 * and indeed for most applications this is not the recommended solution -
 * use the GBooleanCollection instead.
 */
class GBooleanObject
	:public GParameterT<bool>
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int){
	  using boost::serialization::make_nvp;

	  ar & make_nvp("GParameterT_bool", boost::serialization::base_object<GParameterT<bool> >(*this));
	}
	///////////////////////////////////////////////////////////////////////

public:
	/** @brief The default constructor */
	GBooleanObject();
	/** @brief The copy constructor */
	GBooleanObject(const GBooleanObject&);
	/** @brief Initialization by contained value */
	explicit GBooleanObject(const bool&);
	/** @brief The destructor */
	virtual ~GBooleanObject();

	/** @brief An assignment operator */
	virtual bool operator=(const bool&);

	/** @brief A standard assignment operator */
	const GBooleanObject& operator=(const GBooleanObject&);

	/** @brief Checks for equality with another GBooleanObject object */
	bool operator==(const GBooleanObject&) const;
	/** @brief Checks for inequality with another GBooleanObject object */
	bool operator!=(const GBooleanObject&) const;

	/** @brief Triggers random initialization of the parameter object */
	virtual void randomInit();
	/** @brief Triggers random initialization of the parameter object, with a given likelihood structure */
	virtual void randomInit(const double&);

	/** @brief Checks whether this object fulfills a given expectation in relation to another object */
	virtual boost::optional<std::string> checkRelationshipWith(const GObject&, const Gem::Common::expectation&, const double&, const std::string&, const std::string&, const bool&) const;

	/** @brief Attach our local value to the vector. */
	virtual void booleanStreamline(std::vector<bool>&) const;
	/** @brief Tell the audience that we own a boost::int32_t value */
	virtual std::size_t countBoolParameters() const;
	/** @brief Assigns part of a value vector to the parameter */
	virtual void assignBooleanValueVector(const std::vector<bool>&, std::size_t&);

protected:
	/** @brief Loads the data of another GObject */
	virtual void load_(const GObject*);
	/** @brief Creates a deep clone of this object. */
	virtual GObject* clone_() const;

	/** @brief Triggers random initialization of the parameter object */
	virtual void randomInit_();
	/** @brief Triggers random initialization of the parameter object, with a given likelihood structure */
	void randomInit_(const double&);

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

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GBooleanObject)

#endif /* GBOOLEANOBJECT_HPP_ */
