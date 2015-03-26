/**
 * @file GSerialGD.hpp
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


#ifndef GSERIALGD_HPP_
#define GSERIALGD_HPP_

// Geneva headers go here
#include "common/GExceptions.hpp"
#include "common/GHelperFunctions.hpp"
#include "geneva/GBaseGD.hpp"
#include "geneva/GOptimizableEntity.hpp"
#include "geneva/GObject.hpp"

#ifdef GEM_TESTING
#include "geneva/GTestIndividual1.hpp"
#endif /* GEM_TESTING */

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * A serial gradient descent
 */
class GSerialGD
	:public GBaseGD
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int) {
		using boost::serialization::make_nvp;

		ar
		& BOOST_SERIALIZATION_BASE_OBJECT_NVP(GBaseGD);
	}

	///////////////////////////////////////////////////////////////////////

public:
	/** @brief The default constructor */
	G_API_GENEVA GSerialGD();
	/** @brief Initialization with the number of starting points and the size of the finite step */
	G_API_GENEVA GSerialGD(const std::size_t&, const double&, const double&);
	/** @brief A standard copy constructor */
	G_API_GENEVA GSerialGD(const GSerialGD&);
	/** @brief The destructor */
	virtual G_API_GENEVA ~GSerialGD();

   /** @brief The standard assignment operator */
   G_API_GENEVA const GSerialGD& operator=(const GSerialGD&);

	/** @brief Checks for equality with another GSerialGD object */
	G_API_GENEVA bool operator==(const GSerialGD&) const;
	/** @brief Checks for inequality with another GSerialGD object */
	G_API_GENEVA bool operator!=(const GSerialGD&) const;

	/** @brief Checks whether this object fulfills a given expectation in relation to another object */
	virtual G_API_GENEVA boost::optional<std::string> checkRelationshipWith(
      const GObject&
      , const Gem::Common::expectation&
      , const double&
      , const std::string&
      , const std::string&
      , const bool&
	) const OVERRIDE;

	/** @brief Adds local configuration options to a GParserBuilder object */
	virtual G_API_GENEVA void addConfigurationOptions (
		Gem::Common::GParserBuilder& gpb
	) OVERRIDE;

	/** @brief Allows to assign a name to the role of this individual(-derivative) */
	virtual G_API_GENEVA std::string getIndividualCharacteristic() const OVERRIDE;

   /** @brief Emits a name for this class / object */
   virtual G_API_GENEVA std::string name() const OVERRIDE;

protected:
	/** @brief Loads the data of another population */
	virtual G_API_GENEVA void load_(const GObject *) OVERRIDE;
	/** @brief Creates a deep clone of this object */
	virtual G_API_GENEVA GObject *clone_() const OVERRIDE;

	virtual G_API_GENEVA void init() OVERRIDE;
	/** @brief Does any necessary finalization work */
	virtual G_API_GENEVA void finalize() OVERRIDE;

	/** @brief Triggers fitness calculation of a number of individuals */
	virtual G_API_GENEVA void runFitnessCalculation() OVERRIDE;

public:
	/***************************************************************************/
	/** @brief Applies modifications to this object. This is needed for testing purposes */
	virtual G_API_GENEVA bool modify_GUnitTests() OVERRIDE;
	/** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
	virtual G_API_GENEVA void specificTestsNoFailureExpected_GUnitTests() OVERRIDE;
	/** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
	virtual G_API_GENEVA void specificTestsFailuresExpected_GUnitTests() OVERRIDE;
};

} /* namespace Geneva */
} /* namespace Gem */

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
#ifdef GEM_TESTING
/**
 * A factory function that emits a GSerialGD object
 */
template <>
inline boost::shared_ptr<Gem::Geneva::GSerialGD> TFactory_GUnitTests<Gem::Geneva::GSerialGD>() {
   using namespace Gem::Tests;
   boost::shared_ptr<Gem::Geneva::GSerialGD> p;
   BOOST_CHECK_NO_THROW(p= boost::shared_ptr<Gem::Geneva::GSerialGD>(new Gem::Geneva::GSerialGD()));
   p->push_back(boost::shared_ptr<GTestIndividual1>(new GTestIndividual1()));
   return p;
}
#endif /* GEM_TESTING */

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GSerialGD)

#endif /* GSERIALGD_HPP_ */
