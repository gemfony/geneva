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
	void serialize(Archive & ar, const unsigned int){
		using boost::serialization::make_nvp;

		ar
			& make_nvp("GParameterTCollectionT_gbo",
						  boost::serialization::base_object<GParameterTCollectionT<GBooleanObject>>(*this));
	}
	///////////////////////////////////////////////////////////////////////

public:
	/** @brief The default constructor */
	G_API_GENEVA GBooleanObjectCollection();
	/** @brief Initialization with a number of GBooleanObject objects */
	G_API_GENEVA GBooleanObjectCollection(const std::size_t&, std::shared_ptr<GBooleanObject>);
	/** @brief Initialization with a number of GBoolean objects with a given probability for the value "true" */
	G_API_GENEVA GBooleanObjectCollection(const std::size_t&, const double&);
	/** @brief The copy constructor */
	G_API_GENEVA GBooleanObjectCollection(const GBooleanObjectCollection&);
	/** @brief The destructor */
	virtual G_API_GENEVA ~GBooleanObjectCollection();

	/** @brief The standard assignment operator */
	G_API_GENEVA const GBooleanObjectCollection& operator=(const GBooleanObjectCollection&);

	/** @brief Checks for equality with another GBooleanObjectCollection object */
	G_API_GENEVA bool operator==(const GBooleanObjectCollection&) const;
	/** @brief Checks for inequality with another GBooleanObjectCollection object */
	G_API_GENEVA bool operator!=(const GBooleanObjectCollection&) const;

	/** @brief Searches for compliance with expectations with respect to another object of the same type */
	virtual G_API_GENEVA void compare(
		const GObject& // the other object
		, const Gem::Common::expectation& // the expectation for this object, e.g. equality
		, const double& // the limit for allowed deviations of floating point types
	) const override;

	/** @brief Emits a name for this class / object */
	virtual G_API_GENEVA std::string name() const override;

protected:
	/** @brief Loads the data of another GObject */
	virtual G_API_GENEVA void load_(const GObject*) override;

private:
	/** @brief Creates a deep clone of this object. */
	virtual G_API_GENEVA GObject* clone_() const override;

public:
	/** @brief Applies modifications to this object. This is needed for testing purposes */
	virtual G_API_GENEVA bool modify_GUnitTests() override;
	/** @brief Fills the collection with GBooleanObject objects */
	G_API_GENEVA void fillWithObjects(const std::size_t&);
	/** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
	virtual G_API_GENEVA void specificTestsNoFailureExpected_GUnitTests() override;
	/** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
	virtual G_API_GENEVA void specificTestsFailuresExpected_GUnitTests() override;
};

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GBooleanObjectCollection)

#endif /* GBOOLEANOBJECTCOLLECTION_HPP_ */
