/**
 * @file GDoubleObjectCollection.hpp
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

#ifndef GDOUBLEOBJECTCOLLECTION_HPP_
#define GDOUBLEOBJECTCOLLECTION_HPP_

// Geneva header files go here
#include "geneva/GDoubleObject.hpp"
#include "geneva/GDoubleGaussAdaptor.hpp"
#include "geneva/GParameterTCollectionT.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * A collection of GDoubleObject objects, ready for use in a
 * GParameterSet derivative.
 */
class GDoubleObjectCollection
	:public GParameterTCollectionT<GDoubleObject>
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int){
	  using boost::serialization::make_nvp;

	  ar & make_nvp("GParameterTCollectionT_gbd",
			  boost::serialization::base_object<GParameterTCollectionT<GDoubleObject> >(*this));
	}
	///////////////////////////////////////////////////////////////////////

public:
	/** @brief The default constructor */
	GDoubleObjectCollection();
	/** @brief Initialization with a number of GDoubleObject objects */
	GDoubleObjectCollection(const std::size_t&, boost::shared_ptr<GDoubleObject>);
	/** @brief The copy constructor */
	GDoubleObjectCollection(const GDoubleObjectCollection&);
	/** @brief The destructor */
	virtual ~GDoubleObjectCollection();

	/** @brief A standard assignment operator */
	const GDoubleObjectCollection& operator=(const GDoubleObjectCollection&);

	/** @brief Checks for equality with another GDoubleObjectCollection object */
	bool operator==(const GDoubleObjectCollection&) const;
	/** @brief Checks for inequality with another GDoubleObjectCollection object */
	bool operator!=(const GDoubleObjectCollection&) const;

	/** @brief Checks whether this object fulfills a given expectation in relation to another object */
	virtual boost::optional<std::string> checkRelationshipWith(
			const GObject&
			, const Gem::Common::expectation&
			, const double&
			, const std::string&
			, const std::string&
			, const bool&
	) const OVERRIDE;

   /** @brief Emits a name for this class / object */
   virtual std::string name() const OVERRIDE;

protected:
	/** @brief Loads the data of another GObject */
	virtual void load_(const GObject*) OVERRIDE;
	/** @brief Creates a deep clone of this object. */
	virtual GObject* clone_() const OVERRIDE;

public:
	/** @brief Applies modifications to this object. This is needed for testing purposes */
	virtual bool modify_GUnitTests() OVERRIDE;
	/** @brief Fills the collection with GDoubleObject objects */
	void fillWithObjects(const std::size_t&);
	/** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
	virtual void specificTestsNoFailureExpected_GUnitTests() OVERRIDE;
	/** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
	virtual void specificTestsFailuresExpected_GUnitTests() OVERRIDE;
};

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GDoubleObjectCollection)

#endif /* GDOUBLEOBJECTCOLLECTION_HPP_ */
