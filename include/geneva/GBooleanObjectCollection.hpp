/**
 * @file GBooleanObjectCollection.hpp
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

#ifndef GBOOLEANOBJECTCOLLECTION_HPP_
#define GBOOLEANOBJECTCOLLECTION_HPP_


// Geneva header files go here
#include "geneva/GBooleanObject.hpp"
#include "geneva/GParameterTCollectionT.hpp"
#include "geneva/GBooleanAdaptor.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * A collection of GBooleanObject objects, ready for use in a
 * GParameterSet derivative.
 */
class GBooleanObjectCollection
	:public GParameterTCollectionT<GBooleanObject>
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	G_API void serialize(Archive & ar, const unsigned int){
	  using boost::serialization::make_nvp;

	  ar
	  & make_nvp("GParameterTCollectionT_gbo",
			  boost::serialization::base_object<GParameterTCollectionT<GBooleanObject> >(*this));
	}
	///////////////////////////////////////////////////////////////////////

public:
	/** @brief The default constructor */
	G_API GBooleanObjectCollection();
	/** @brief Initialization with a number of GBooleanObject objects */
	G_API GBooleanObjectCollection(const std::size_t&, boost::shared_ptr<GBooleanObject>);
	/** @brief Initialization with a number of GBoolean objects with a given probability for the value "true" */
	G_API GBooleanObjectCollection(const std::size_t&, const double&);
	/** @brief The copy constructor */
	G_API GBooleanObjectCollection(const GBooleanObjectCollection&);
	/** @brief The destructor */
	virtual G_API ~GBooleanObjectCollection();

	/** @brief A standard assignment operator */
	G_API const GBooleanObjectCollection& operator=(const GBooleanObjectCollection&);

	/** @brief Checks for equality with another GBooleanObjectCollection object */
	G_API bool operator==(const GBooleanObjectCollection&) const;
	/** @brief Checks for inequality with another GBooleanObjectCollection object */
	G_API bool operator!=(const GBooleanObjectCollection&) const;

	/** @brief Checks whether this object fulfills a given expectation in relation to another object */
	virtual G_API boost::optional<std::string> checkRelationshipWith(
      const GObject&
      , const Gem::Common::expectation&
      , const double&
      , const std::string&
      , const std::string&
      , const bool&
	) const OVERRIDE;

   /** @brief Emits a name for this class / object */
   virtual G_API std::string name() const OVERRIDE;

protected:
	/** @brief Loads the data of another GObject */
	virtual G_API void load_(const GObject*) OVERRIDE;
	/** @brief Creates a deep clone of this object. */
	virtual G_API GObject* clone_() const OVERRIDE;

public:
	/** @brief Applies modifications to this object. This is needed for testing purposes */
	virtual G_API bool modify_GUnitTests() OVERRIDE;
	/** @brief Fills the collection with GBooleanObject objects */
	G_API void fillWithObjects(const std::size_t&);
	/** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
	virtual G_API void specificTestsNoFailureExpected_GUnitTests() OVERRIDE;
	/** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
	virtual G_API void specificTestsFailuresExpected_GUnitTests() OVERRIDE;
};

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GBooleanObjectCollection)

#endif /* GBOOLEANOBJECTCOLLECTION_HPP_ */
