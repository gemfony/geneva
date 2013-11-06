/**
 * @file GBooleanObject.hpp
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt)
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) eu
 *
 * This file is part of the Geneva library collection.
 *
 * Geneva was developed with kind support from Karlsruhe Institute of
 * Technology (KIT) and Steinbuch Centre for Computing (SCC). Further
 * information about KIT and SCC can be found at http://www.kit.edu/english
 * and http://scc.kit.edu .
 *
 * Geneva is free software: you can redistribute and/or modify it under
 * the terms of version 3 of the GNU Affero General Public License
 * as published by the Free Software Foundation.
 *
 * Geneva is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with the Geneva library. If not, see <http://www.gnu.org/licenses/>.
 *
 * For further information on Gemfony scientific and Geneva, visit
 * http://www.gemfony.eu .
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

/******************************************************************************/
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
	/** @brief Initialization with a given probability for "true" */
	explicit GBooleanObject(const double&);
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
	virtual void randomInit() OVERRIDE;
	/** @brief Triggers random initialization of the parameter object, with a given likelihood structure */
	void randomInit(const double&);

	/** @brief Checks whether this object fulfills a given expectation in relation to another object */
	virtual boost::optional<std::string> checkRelationshipWith(
	      const GObject&
	      , const Gem::Common::expectation&
	      , const double&
	      , const std::string&
	      , const std::string&
	      , const bool&
	) const OVERRIDE;

	/** @brief Attach our local value to the vector. */
	virtual void booleanStreamline(std::vector<bool>&) const OVERRIDE;
	/** @brief Attach boundaries of type bool to the vectors */
	virtual void booleanBoundaries(std::vector<bool>&, std::vector<bool>&) const OVERRIDE;
	/** @brief Tell the audience that we own a boost::int32_t value */
	virtual std::size_t countBoolParameters() const OVERRIDE;
	/** @brief Assigns part of a value vector to the parameter */
	virtual void assignBooleanValueVector(const std::vector<bool>&, std::size_t&) OVERRIDE;

   /** @brief Attach our local value to the map. */
   virtual void booleanStreamline(std::map<std::string, std::vector<bool> >&) const OVERRIDE;

   /** @brief Emits a name for this class / object */
   virtual std::string name() const OVERRIDE;

protected:
	/** @brief Loads the data of another GObject */
	virtual void load_(const GObject*) OVERRIDE;
	/** @brief Creates a deep clone of this object. */
	virtual GObject* clone_() const OVERRIDE;

	/** @brief Triggers random initialization of the parameter object */
	virtual void randomInit_() OVERRIDE;
	/** @brief Triggers random initialization of the parameter object, with a given likelihood structure */
	void randomInit_(const double&);

public:
	/** @brief Applies modifications to this object. This is needed for testing purposes */
	virtual bool modify_GUnitTests() OVERRIDE;
	/** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
	virtual void specificTestsNoFailureExpected_GUnitTests() OVERRIDE;
	/** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
	virtual void specificTestsFailuresExpected_GUnitTests() OVERRIDE;
};

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GBooleanObject)

#endif /* GBOOLEANOBJECT_HPP_ */
