/**
 * @file GInt32ObjectCollection.hpp
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

// Standard header files go here

// Boost header files go here

#ifndef GINT32OBJECTCOLLECTION_HPP_
#define GINT32OBJECTCOLLECTION_HPP_

// Geneva header files go here
#include "geneva/GInt32Object.hpp"
#include "geneva/GParameterTCollectionT.hpp"
#include "geneva/GInt32GaussAdaptor.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * A collection of GInt32Object objects, ready for use in a
 * GParameterSet derivative.
 */
class GInt32ObjectCollection
	:public GParameterTCollectionT<GInt32Object>
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int){
	  using boost::serialization::make_nvp;

	  ar
	  & make_nvp("GParameterTCollectionT_ioc",
			  boost::serialization::base_object<GParameterTCollectionT<GInt32Object> >(*this));
	}
	///////////////////////////////////////////////////////////////////////

public:
	/** @brief The default constructor */
	G_API_GENEVA GInt32ObjectCollection();
	/** @brief Initialization with a number of GInt32Object objects */
	G_API_GENEVA GInt32ObjectCollection(const std::size_t&, boost::shared_ptr<GInt32Object>);
	/** @brief The copy constructor */
	G_API_GENEVA GInt32ObjectCollection(const GInt32ObjectCollection&);
	/** @brief The destructor */
	virtual G_API_GENEVA ~GInt32ObjectCollection();

   /** @brief The standard assignment operator */
   G_API_GENEVA const GInt32ObjectCollection& operator=(const GInt32ObjectCollection&);

	/** @brief Checks for equality with another GInt32ObjectCollection object */
	G_API_GENEVA bool operator==(const GInt32ObjectCollection&) const;
	/** @brief Checks for inequality with another GInt32ObjectCollection object */
	G_API_GENEVA bool operator!=(const GInt32ObjectCollection&) const;


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

public:
	/** @brief Applies modifications to this object. This is needed for testing purposes */
	virtual G_API_GENEVA bool modify_GUnitTests() OVERRIDE;
	/** @brief Fills the collection with GInt32Object objects */
	G_API_GENEVA void fillWithObjects(const std::size_t&);
	/** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
	virtual G_API_GENEVA void specificTestsNoFailureExpected_GUnitTests() OVERRIDE;
	/** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
	virtual G_API_GENEVA void specificTestsFailuresExpected_GUnitTests() OVERRIDE;
};

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GInt32ObjectCollection)

#endif /* GINT32OBJECTCOLLECTION_HPP_ */
