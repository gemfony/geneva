/**
 * @file GBrokerEA.hpp
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

#ifndef GBROKEREA_HPP_
#define GBROKEREA_HPP_

// Geneva headers go here
#include "courtier/GBufferPortT.hpp"
#include "courtier/GBrokerExecutorT.hpp"
#include "geneva/GEAPersonalityTraits.hpp"
#include "geneva/GBaseEA.hpp"
#include "geneva/GParameterSet.hpp"
#include "common/GExceptions.hpp"
#include "common/GThreadPool.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * This population handles evolutionary algorithm-based optimization in environments
 * where communication between client and server is handled through Geneva's
 * broker infrastructure (libcourtier).
 * Note that serialization of this class makes sense only for backup-purposes,
 * in order to allow later, manual recovery. A broker object needs to be registered,
 * and serialization does not help here.
 * Serialization in a network context only happens below the level of this population,
 * it is itself usually not shipped over a network connection.
 */
class GBrokerEA
	: public GBaseEA
	, public Gem::Courtier::GBrokerExecutorT<Gem::Geneva::GParameterSet>
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int){
		using boost::serialization::make_nvp;

		ar
		& BOOST_SERIALIZATION_BASE_OBJECT_NVP(GBaseEA)
		& make_nvp("GBrokerExecutorT_GParameterSet", boost::serialization::base_object<Gem::Courtier::GBrokerExecutorT<GParameterSet>>(*this))
		& BOOST_SERIALIZATION_NVP(nThreads_);
	}
	///////////////////////////////////////////////////////////////////////

public:
	/** @brief The standard constructor */
	G_API_GENEVA GBrokerEA();
	/** @brief A standard copy constructor */
	G_API_GENEVA GBrokerEA(const GBrokerEA&);
	/** @brief The standard destructor */
	virtual G_API_GENEVA ~GBrokerEA();

	/** @brief Checks for equality with another GBrokerEA object */
	G_API_GENEVA bool operator==(const GBrokerEA&) const;
	/** @brief Checks for inequality with another GBrokerEA object */
	G_API_GENEVA bool operator!=(const GBrokerEA&) const;

	/** @brief The standard assignment operator */
	G_API_GENEVA const GBrokerEA& operator=(const GBrokerEA&);

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

	/** @brief Sets the maximum number of threads */
	G_API_GENEVA void setNThreads(std::uint16_t);
	/** @brief Retrieves the maximum number of threads */
	G_API_GENEVA std::uint16_t getNThreads() const ;

	/** @brief Allows to assign a name to the role of this individual(-derivative) */
	virtual G_API_GENEVA std::string getIndividualCharacteristic() const override;

	/** @brief Emits a name for this class / object */
	virtual G_API_GENEVA std::string name() const override;

protected:
	/** @brief Loads the data of another GTransfer Population */
	virtual G_API_GENEVA void load_(const GObject *) override;
	/** @brief Creates a deep copy of this object */
	virtual G_API_GENEVA GObject *clone_() const override;

	/** @brief Adapt children in a serial manner */
	virtual G_API_GENEVA void adaptChildren() override;
	/** @brief Calculates the fitness of all required individuals */
	virtual G_API_GENEVA void runFitnessCalculation() override;
	/** @brief Selects new parents */
	virtual G_API_GENEVA void selectBest() override;

	/** @brief Performs any necessary initialization work before the start of the optimization cycle */
	virtual G_API_GENEVA void init() override;
	/** @brief Performs any necessary finalization work after the end of the optimization cycle */
	virtual G_API_GENEVA void finalize() override;

private:
	/***************************************************************************/

	std::uint16_t nThreads_ = DEFAULTNBOOSTTHREADS; ///< The number of threads
	std::shared_ptr<Gem::Common::GThreadPool> tp_ptr_; ///< Temporarily holds a thread pool

	std::vector<std::shared_ptr<GParameterSet>> oldWorkItems_; ///< Temporarily holds old returned work items

	/***************************************************************************/

	/** @brief Fixes the population after a job submission */
	void fixAfterJobSubmission();

	/***************************************************************************/

public:
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

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GBrokerEA)

#endif /* GBROKEREA_HPP_ */
