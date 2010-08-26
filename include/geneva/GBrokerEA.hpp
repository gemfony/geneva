/**
 * @file GBrokerEA.hpp
 */

/*
 * Copyright (C) Authors of the Geneva library collection and Karlsruhe
 * Institute of Technology (University of the State of Baden-Wuerttemberg
 * and National Laboratory of the Helmholtz Association).
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: info [at] gemfony (dot) com
 *
 * This file is part of the Geneva library collection
 *
 * Geneva is free software: you can redistribute it and/or modify
 * it under the terms of version 3 of the GNU Affero General Public License
 * as published by the Free Software Foundation.
 *
 * Geneva is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with the Geneva library.  If not, see <http://www.gnu.org/licenses/>.
 *
 * For further information on Gemfony scientific and Geneva, visit
 * http://www.gemfony.com .
 */


// Standard headers go here

#include <sstream>
#include <algorithm>
#include <vector>

// Includes check for correct Boost version(s)
#include "common/GGlobalDefines.hpp"

// Boost headers go here

#include <boost/cstdint.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/date_time.hpp>
#include <boost/date_time/gregorian/greg_serialize.hpp>
#include <boost/date_time/posix_time/time_serialize.hpp>

#ifndef GBROKEREA_HPP_
#define GBROKEREA_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif

// Geneva headers go here
#include "common/GExceptions.hpp"
#include "courtier/GBufferPortT.hpp"
#include "GEAPersonalityTraits.hpp"
#include "GEvolutionaryAlgorithm.hpp"
#include "GBrokerConnector.hpp"
#include "GIndividual.hpp"

namespace Gem
{
namespace Geneva
{
  /**********************************************************************************/
  /**
   * This population handles evolutionary algorithm-based optimization in environments
   * where communication between client and communication is handled through Geneva's
   * broker infrastructure (libcourtier).
   * Note that serialization of this class makes sense only for backup-purposes,
   * in order to allow later, manual recovery. A broker object needs to be registered,
   * and serialization does not help here.
   * Serialization in a network context only happens below the level of this population,
   * it is itself usually not shipped over a network connection.
   */
  class GBrokerEA
    : public GEvolutionaryAlgorithm
    , public GBrokerConnector
  {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template<typename Archive>
    void serialize(Archive & ar, const unsigned int){
      using boost::serialization::make_nvp;

      ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(GEvolutionaryAlgorithm)
		 & BOOST_SERIALIZATION_BASE_OBJECT_NVP(GBrokerConnector);
    }
    ///////////////////////////////////////////////////////////////////////

  public:
	/** @brief The standard constructor */
    GBrokerEA();
    /** @brief A standard copy constructor */
    GBrokerEA(const GBrokerEA&);
    /** @brief The standard destructor */
    virtual ~GBrokerEA();

    /** @brief A standard assignment operator */
    const GBrokerEA& operator=(const GBrokerEA&);

	/** @brief Checks for equality with another GBrokerEA object */
	bool operator==(const GBrokerEA&) const;
	/** @brief Checks for inequality with another GBrokerEA object */
	bool operator!=(const GBrokerEA&) const;

	/** @brief Checks whether this object fulfills a given expectation in relation to another object */
	virtual boost::optional<std::string> checkRelationshipWith(const GObject&, const Gem::Common::expectation&, const double&, const std::string&, const std::string&, const bool&) const;

	/** @brief The actual business logic to be performed during each iteration. Returns the best achieved fitness */
	virtual double cycleLogic();
	/** @brief Performs any necessary initialization work before the start of the optimization cycle */
	virtual void init();
	/** @brief Performs any necessary finalization work after the end of the optimization cycle */
	virtual void finalize();

  protected:
    /** @brief Loads the data of another GTransfer Population */
    virtual void load_(const GObject *);
    /** @brief Creates a deep copy of this object */
    virtual GObject *clone_() const;

    /** @brief Adapts all children in sequence */
    virtual void adaptChildren();
    /** @brief Selects new parents */
    virtual void select();

  private:
    /*********************************************************************************/
    /**
     * A simple comparison operator that helps to sort individuals according to their
     * status as parents or children
     */
    class indParentComp {
    public:
    	bool operator()(boost::shared_ptr<GIndividual> x, boost::shared_ptr<GIndividual> y) {
    		return x->getEAPersonalityTraits()->isParent() > y->getEAPersonalityTraits()->isParent();
    	}
    };

    /*********************************************************************************/

#ifdef GENEVATESTING
  public:
	/** @brief Applies modifications to this object. This is needed for testing purposes */
	virtual bool modify_GUnitTests();
	/** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
	virtual void specificTestsNoFailureExpected_GUnitTests();
	/** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
	virtual void specificTestsFailuresExpected_GUnitTests();
#endif /* GENEVATESTING */
  };

  /**********************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

#endif /* GBROKEREA_HPP_ */
