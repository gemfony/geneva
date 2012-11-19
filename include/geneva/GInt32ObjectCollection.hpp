/**
 * @file GInt32ObjectCollection.hpp
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

#ifndef GINT32OBJECTCOLLECTION_HPP_
#define GINT32OBJECTCOLLECTION_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif

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

	  ar & make_nvp("GParameterTCollectionT_ioc",
			  boost::serialization::base_object<GParameterTCollectionT<GInt32Object> >(*this));
	}
	///////////////////////////////////////////////////////////////////////

public:
	/** @brief The default constructor */
	GInt32ObjectCollection();
	/** @brief Initialization with a number of GInt32Object objects */
	GInt32ObjectCollection(const std::size_t&, boost::shared_ptr<GInt32Object>);
	/** @brief The copy constructor */
	GInt32ObjectCollection(const GInt32ObjectCollection&);
	/** @brief The destructor */
	virtual ~GInt32ObjectCollection();

	/** @brief A standard assignment operator */
	const GInt32ObjectCollection& operator=(const GInt32ObjectCollection&);

	/** @brief Checks for equality with another GInt32ObjectCollection object */
	bool operator==(const GInt32ObjectCollection&) const;
	/** @brief Checks for inequality with another GInt32ObjectCollection object */
	bool operator!=(const GInt32ObjectCollection&) const;

	/** @brief Checks whether this object fulfills a given expectation in relation to another object */
	virtual boost::optional<std::string> checkRelationshipWith(
	      const GObject&
	      , const Gem::Common::expectation&
	      , const double&
	      , const std::string&
	      , const std::string&
	      , const bool&
	) const;

protected:
	/** @brief Loads the data of another GObject */
	virtual void load_(const GObject*);
	/** @brief Creates a deep clone of this object. */
	virtual GObject* clone_() const;

public:
	/** @brief Applies modifications to this object. This is needed for testing purposes */
	virtual bool modify_GUnitTests();
	/** @brief Fills the collection with GInt32Object objects */
	void fillWithObjects(const std::size_t&);
	/** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
	virtual void specificTestsNoFailureExpected_GUnitTests();
	/** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
	virtual void specificTestsFailuresExpected_GUnitTests();
};

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GInt32ObjectCollection)

#endif /* GINT32OBJECTCOLLECTION_HPP_ */
