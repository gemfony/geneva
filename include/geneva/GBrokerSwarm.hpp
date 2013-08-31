/**
 * @file GBrokerSwarm.hpp
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


// Standard headers go here

// Boost headers go here

#ifndef GBROKERSWARM_HPP_
#define GBROKERSWARM_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif

// Geneva headers go here
#include "common/GExceptions.hpp"
#include "courtier/GBufferPortT.hpp"
#include "courtier/GBrokerConnector2T.hpp"
#include "geneva/GSwarmPersonalityTraits.hpp"
#include "geneva/GBaseSwarm.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * This class implements a swarm algorithm with the ability to delegate certain
 * tasks to remote clients, using Geneva's broker infrastructure.
 */
class GBrokerSwarm
  : public GBaseSwarm
  , public Gem::Courtier::GBrokerConnector2T<Gem::Geneva::GParameterSet>
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int){
		using boost::serialization::make_nvp;

		ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(GBaseSwarm)
		   & make_nvp("GBrokerConnector2T_GParameterSet",
		         boost::serialization::base_object<Gem::Courtier::GBrokerConnector2T<GParameterSet> >(*this));
	}
	///////////////////////////////////////////////////////////////////////

public:
	/** @brief The default constructor */
	GBrokerSwarm();
	/** @brief The standard constructor */
    GBrokerSwarm(const std::size_t&, const std::size_t&);
    /** @brief A standard copy constructor */
    GBrokerSwarm(const GBrokerSwarm&);
    /** @brief The standard destructor */
    virtual ~GBrokerSwarm();

    /** @brief A standard assignment operator */
    const GBrokerSwarm& operator=(const GBrokerSwarm&);

	/** @brief Checks for equality with another GBrokerSwarm object */
	bool operator==(const GBrokerSwarm&) const;
	/** @brief Checks for inequality with another GBrokerSwarm object */
	bool operator!=(const GBrokerSwarm&) const;

	/** @brief Checks whether this object fulfills a given expectation in relation to another object */
	virtual boost::optional<std::string> checkRelationshipWith(
			const GObject&
			, const Gem::Common::expectation&
			, const double&
			, const std::string&
			, const std::string&
			, const bool&
	) const OVERRIDE;

	/** @brief Checks whether a given algorithm type likes to communicate via the broker */
	virtual bool usesBroker() const OVERRIDE;

	/** @brief Adds local configuration options to a GParserBuilder object */
	virtual void addConfigurationOptions (
		Gem::Common::GParserBuilder& gpb
		, const bool& showOrigin
	) OVERRIDE;

	/** @brief Allows to assign a name to the role of this individual(-derivative) */
	virtual std::string getIndividualCharacteristic() const OVERRIDE;

   /** @brief Emits a name for this class / object */
   virtual std::string name() const OVERRIDE;

protected:
   /***************************************************************************/
	/** @brief Loads the data of another GTransfer Population */
   virtual void load_(const GObject *) OVERRIDE;
   /** @brief Creates a deep copy of this object */
   virtual GObject *clone_() const OVERRIDE;

	/** @brief Performs any necessary initialization work before the start of the optimization cycle */
	virtual void init() OVERRIDE;
	/** @brief Performs any necessary finalization work after the end of the optimization cycle */
	virtual void finalize() OVERRIDE;

   /** @brief The actual business logic to be performed during each iteration; Returns the best achieved fitness */
   virtual double cycleLogic() OVERRIDE;

	/** @brief Updates all individual's positions */
	virtual void updatePositions() OVERRIDE;
	/** @brief Triggers the fitness calculation of all individuals */
	virtual void updateFitness() OVERRIDE;

   /** @brief Fixes the population after a job submission */
   void adjustNeighborhoods();
   /** @brief Checks whether each neighborhood has the default size */
   bool neighborhoodsHaveNominalValues() const;

private:
	/***************************************************************************/
	bool storedServerMode_; ///< Indicates whether an individual runs in server mode
	std::vector<boost::shared_ptr<GParameterSet> > oldIndividuals_; ///< A temporary copy of the last iteration's individuals

	std::vector<boost::shared_ptr<GParameterSet> > oldWorkItems_; ///< Temporarily holds old returned work items

	/***************************************************************************/
	/**
	 * A simple comparison operator that helps to sort individuals according to their
	 * affiliation to a neighborhood. Smaller neighborhood numbers should be in front.
	 */
	class indNeighborhoodComp {
	public:
	   bool operator()(boost::shared_ptr<GParameterSet> x, boost::shared_ptr<GParameterSet> y) {
	      return x->getPersonalityTraits<GSwarmPersonalityTraits>()->getNeighborhood() < y->getPersonalityTraits<GSwarmPersonalityTraits>()->getNeighborhood();
	   }
	};

	/***************************************************************************/

public:
	/** @brief Applies modifications to this object. This is needed for testing purposes */
	virtual bool modify_GUnitTests() OVERRIDE;
	/** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
	virtual void specificTestsNoFailureExpected_GUnitTests() OVERRIDE;
	/** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
	virtual void specificTestsFailuresExpected_GUnitTests() OVERRIDE;
};

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GBrokerSwarm)

#endif /* GBROKERSWARM_HPP_ */
