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

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard headers go here

// Boost headers go here

#ifndef GBOOLEANOBJECT_HPP_
#define GBOOLEANOBJECT_HPP_


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

	  ar
	  & make_nvp("GParameterT_bool", boost::serialization::base_object<GParameterT<bool> >(*this));
	}
	///////////////////////////////////////////////////////////////////////

public:
	/** @brief The default constructor */
	G_API_GENEVA GBooleanObject();
	/** @brief The copy constructor */
	G_API_GENEVA GBooleanObject(const GBooleanObject&);
	/** @brief Initialization by contained value */
	explicit G_API_GENEVA GBooleanObject(const bool&);
	/** @brief Initialization with a given probability for "true" */
	explicit G_API_GENEVA GBooleanObject(const double&);
	/** @brief The destructor */
	virtual G_API_GENEVA ~GBooleanObject();

	/** @brief An assignment operator */
	virtual G_API_GENEVA bool operator=(const bool&);
   /** @brief The standard assignment operator */
   G_API_GENEVA const GBooleanObject& operator=(const GBooleanObject&);

	/** @brief Checks for equality with another GBooleanObject object */
	G_API_GENEVA bool operator==(const GBooleanObject&) const;
	/** @brief Checks for inequality with another GBooleanObject object */
	G_API_GENEVA bool operator!=(const GBooleanObject&) const;

	/** @brief Triggers random initialization of the parameter object */
	virtual G_API_GENEVA void randomInit(const activityMode&) OVERRIDE;
	/** @brief Triggers random initialization of the parameter object, with a given likelihood structure */
	G_API_GENEVA void randomInit(const double&, const activityMode&);

   /** @brief Searches for compliance with expectations with respect to another object of the same type */
   virtual G_API_GENEVA void compare(
      const GObject& // the other object
      , const Gem::Common::expectation& // the expectation for this object, e.g. equality
      , const double& // the limit for allowed deviations of floating point types
   ) const OVERRIDE;

   /** @brief Emits a name for this class / object */
   virtual G_API_GENEVA std::string name() const OVERRIDE;

protected:
	/** @brief Loads the data of another GObject */
	virtual G_API_GENEVA void load_(const GObject*) OVERRIDE;
	/** @brief Creates a deep clone of this object. */
	virtual G_API_GENEVA GObject* clone_() const OVERRIDE;

	/** @brief Triggers random initialization of the parameter object */
	virtual G_API_GENEVA void randomInit_(const activityMode&) OVERRIDE;
	/** @brief Triggers random initialization of the parameter object, with a given likelihood structure */
	G_API_GENEVA void randomInit_(const double&, const activityMode&);

	/** @brief Returns a "comparative range" for this type */
	virtual G_API_GENEVA bool range() const OVERRIDE;

   /** @brief Attach our local value to the vector. */
   virtual G_API_GENEVA void booleanStreamline(std::vector<bool>&, const activityMode& am) const OVERRIDE;
   /** @brief Attach boundaries of type bool to the vectors */
   virtual G_API_GENEVA void booleanBoundaries(std::vector<bool>&, std::vector<bool>&, const activityMode& am) const OVERRIDE;
   /** @brief Tell the audience that we own a boost::int32_t value */
   virtual std::size_t countBoolParameters(const activityMode& am) const OVERRIDE;
   /** @brief Assigns part of a value vector to the parameter */
   virtual G_API_GENEVA void assignBooleanValueVector(const std::vector<bool>&, std::size_t&, const activityMode& am) OVERRIDE;
   /** @brief Attach our local value to the map. */
   virtual G_API_GENEVA void booleanStreamline(std::map<std::string, std::vector<bool> >&, const activityMode& am) const OVERRIDE;
   /** @brief Assigns part of a value map to the parameter */
   virtual G_API_GENEVA void assignBooleanValueVectors(const std::map<std::string, std::vector<bool> >&, const activityMode& am) OVERRIDE;

public:
	/** @brief Applies modifications to this object. This is needed for testing purposes */
	virtual G_API_GENEVA bool modify_GUnitTests() OVERRIDE;
	/** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
	virtual G_API_GENEVA void specificTestsNoFailureExpected_GUnitTests() OVERRIDE;
	/** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
	virtual G_API_GENEVA void specificTestsFailuresExpected_GUnitTests() OVERRIDE;
};

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GBooleanObject)

#endif /* GBOOLEANOBJECT_HPP_ */
