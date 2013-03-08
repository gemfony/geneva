/**
 * @file GConstrainedFloatObjectCollection.hpp
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt)
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) com
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
 * http://www.gemfony.com .
 */



// Standard header files go here

// Boost header files go here

#ifndef GCONSTRAINEDFLOATOBJECTCOLLECTION_HPP_
#define GCONSTRAINEDFLOATOBJECTCOLLECTION_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif


// Geneva header files go here
#include "geneva/GConstrainedFloatObject.hpp"
#include "geneva/GParameterTCollectionT.hpp"
#include "geneva/GFloatGaussAdaptor.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * A collection of GConstrainedFloatObject objects, ready for use in a
 * GParameterSet derivative.
 */
class GConstrainedFloatObjectCollection
	:public GParameterTCollectionT<GConstrainedFloatObject>
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int){
	  using boost::serialization::make_nvp;

	  ar & make_nvp("GParameterTCollectionT_gbd",
			  boost::serialization::base_object<GParameterTCollectionT<GConstrainedFloatObject> >(*this));
	}
	///////////////////////////////////////////////////////////////////////

public:
	/** @brief The default constructor */
	GConstrainedFloatObjectCollection();
	/** @brief Initialization with a number of GConstrainedFloatObject objects */
	GConstrainedFloatObjectCollection(
	      const std::size_t&
	      , boost::shared_ptr<GConstrainedFloatObject>
	);
	/** @brief The copy constructor */
	GConstrainedFloatObjectCollection(const GConstrainedFloatObjectCollection&);
	/** @brief The destructor */
	virtual ~GConstrainedFloatObjectCollection();

	/** @brief A standard assignment operator */
	const GConstrainedFloatObjectCollection& operator=(const GConstrainedFloatObjectCollection&);

	/** @brief Checks for equality with another GConstrainedFloatObjectCollection object */
	bool operator==(const GConstrainedFloatObjectCollection&) const;
	/** @brief Checks for inequality with another GConstrainedFloatObjectCollection object */
	bool operator!=(const GConstrainedFloatObjectCollection&) const;

	/** @brief Checks whether this object fulfills a given expectation in relation to another object */
	virtual boost::optional<std::string> checkRelationshipWith(
	      const GObject&
	      , const Gem::Common::expectation&
	      , const double&
	      , const std::string&
	      , const std::string&
	      , const bool&
	) const;

   /** @brief Emits a name for this class / object */
   virtual std::string name() const;

protected:
	/** @brief Loads the data of another GObject */
	virtual void load_(const GObject*);
	/** @brief Creates a deep clone of this object. */
	virtual GObject* clone_() const;

public:
	/** @brief Applies modifications to this object. This is needed for testing purposes */
	virtual bool modify_GUnitTests();
	/** @brief Fills the collection with GConstrainedFloatObject objects */
	void fillWithObjects(const std::size_t&);
	/** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
	virtual void specificTestsNoFailureExpected_GUnitTests();
	/** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
	virtual void specificTestsFailuresExpected_GUnitTests();
};

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GConstrainedFloatObjectCollection)

#endif /* GCONSTRAINEDFLOATOBJECTCOLLECTION_HPP_ */
