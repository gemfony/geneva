/**
 * @file GBrokerGD.hpp
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

#ifndef GBROKERGD_HPP_
#define GBROKERGD_HPP_


// Geneva headers go here
#include "common/GExceptions.hpp"
#include "common/GHelperFunctions.hpp"
#include "courtier/GBrokerConnectorT.hpp"
#include "geneva/GObject.hpp"
#include "geneva/GBaseGD.hpp"
#include "geneva/GParameterSet.hpp"

#ifdef GEM_TESTING
#include "geneva/GTestIndividual1.hpp"
#endif /* GEM_TESTING */

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * A networked version of the GBaseGD class
 */
class GBrokerGD
	: public GBaseGD
	, public Gem::Courtier::GBrokerConnectorT<Gem::Geneva::GParameterSet>
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int) {
		using boost::serialization::make_nvp;

		ar
		& BOOST_SERIALIZATION_BASE_OBJECT_NVP(GBaseGD)
		& make_nvp("GBrokerConnectorT_GParameterSet",
					  boost::serialization::base_object<Gem::Courtier::GBrokerConnectorT<Gem::Geneva::GParameterSet>>(*this));
	}

	///////////////////////////////////////////////////////////////////////

public:
	/** @brief The default constructor */
	G_API_GENEVA GBrokerGD();
	/** @brief Initialization with the number of starting points and the size of the finite step */
	G_API_GENEVA GBrokerGD(const std::size_t&, const double&, const double&);
	/** @brief A standard copy constructor */
	G_API_GENEVA GBrokerGD(const GBrokerGD&);
	/** @brief The destructor */
	virtual G_API_GENEVA ~GBrokerGD();

	/** @brief The standard assignment operator */
	G_API_GENEVA const GBrokerGD& operator=(const GBrokerGD&);

	/** @brief Checks for equality with another GBrokerGD object */
	G_API_GENEVA bool operator==(const GBrokerGD&) const;
	/** @brief Checks for inequality with another GBrokerGD object */
	G_API_GENEVA bool operator!=(const GBrokerGD&) const;

	/** @brief Searches for compliance with expectations with respect to another object of the same type */
	virtual G_API_GENEVA void compare(
		const GObject& // the other object
		, const Gem::Common::expectation& // the expectation for this object, e.g. equality
		, const double& // the limit for allowed deviations of floating point types
	) const override;

	/** @brief Checks whether a given algorithm type likes to communicate via the broker */
	virtual G_API_GENEVA bool usesBroker() const override;

	/** @brief Adds local configuration options to a GParserBuilder object */
	virtual G_API_GENEVA void addConfigurationOptions (
		Gem::Common::GParserBuilder& gpb
	) override;

	/** @brief Allows to assign a name to the role of this individual(-derivative) */
	virtual G_API_GENEVA std::string getIndividualCharacteristic() const override;

	/** @brief Emits a name for this class / object */
	virtual G_API_GENEVA std::string name() const override;

protected:
	/** @brief Loads the data of another population */
	virtual G_API_GENEVA void load_(const GObject *) override;
	/** @brief Creates a deep clone of this object */
	virtual G_API_GENEVA GObject *clone_() const override;

	/** @brief Performs necessary initialization work */
	virtual G_API_GENEVA void init() override;
	/** @brief Does any necessary finalization work */
	virtual G_API_GENEVA void finalize() override;

	/** @brief Triggers fitness calculation of a number of individuals */
	virtual G_API_GENEVA void runFitnessCalculation() override;

private:
	/***************************************************************************/
	/**
	 * A simple comparison operator that helps to sort individuals according to their
	 * position in the population Smaller position numbers will end up in front.
	 */
	struct indPositionComp {
		bool operator()(std::shared_ptr<GParameterSet> x, std::shared_ptr<GParameterSet> y) {
			return x->getPersonalityTraits<GGDPersonalityTraits>()->getPopulationPosition()
					 < y->getPersonalityTraits<GGDPersonalityTraits>()->getPopulationPosition();
		}
	};
	/***************************************************************************/

	std::vector<std::shared_ptr<GParameterSet>> oldWorkItems_; ///< Temporarily holds old returned work items

public:
	/***************************************************************************/
	/** @brief Applies modifications to this object. This is needed for testing purposes */
	virtual G_API_GENEVA bool modify_GUnitTests() override;
	/** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
	virtual G_API_GENEVA void specificTestsNoFailureExpected_GUnitTests() override;
	/** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
	virtual G_API_GENEVA void specificTestsFailuresExpected_GUnitTests() override;
};

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

#ifdef GEM_TESTING

// Tests of this class (and parent classes)
/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * As Gem::Geneva::GBrokerGD has a protected default constructor, we need to provide a
 * specialization of the factory function that creates objects of this type.
 */
template <>
inline std::shared_ptr<Gem::Geneva::GBrokerGD> TFactory_GUnitTests<Gem::Geneva::GBrokerGD>() {
   std::shared_ptr<Gem::Geneva::GBrokerGD> p;
   BOOST_CHECK_NO_THROW(p= std::shared_ptr<Gem::Geneva::GBrokerGD>(new Gem::Geneva::GBrokerGD()));
   return p;
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

#endif /* GEM_TESTING */


BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GBrokerGD)

#endif /* GBROKERGD_HPP_ */
