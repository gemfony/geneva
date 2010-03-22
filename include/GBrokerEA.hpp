/**
 * @file GBrokerEA.hpp
 */

/* Copyright (C) Dr. Ruediger Berlich and Karlsruhe Institute of Technology
 * (University of the State of Baden-Wuerttemberg and National Laboratory
 * of the Helmholtz Association)
 *
 * Contact: info [at] gemfony (dot) com
 *
 * This file is part of the Geneva library, Gemfony scientific's optimization
 * library.
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
#include "GGlobalDefines.hpp"

// Boost headers go here

#include <boost/cstdint.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/date_time.hpp>
#include <boost/date_time/gregorian/greg_serialize.hpp>
#include <boost/date_time/posix_time/time_serialize.hpp>

#ifndef GBROKEREA_HPP_
#define GBROKEREA_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif

// GenEvA headers go here
#include "GenevaExceptions.hpp"
#include "GIndividual.hpp"
#include "GEvolutionaryAlgorithm.hpp"
#include "GBufferPortT.hpp"
#include "GIndividualBroker.hpp"
#include "GEAPersonalityTraits.hpp"

namespace Gem
{
namespace GenEvA
{

  /**
   * The default factor applied to the turn-around time
   * of the first individual in the first generation. Used to
   * find a suitable timeout-value for following individuals.
   */
  const boost::uint32_t DEFAULTWAITFACTOR = 20;

  /**
   * The default maximum value of the wait factor used during automatic
   * adaption of the waitFactor_ variable. If set to 0, no automatic
   * adaption will take place.
   */
  const boost::uint32_t DEFAULTMAXWAITFACTOR = 0;

  /**
   * The default allowed time in seconds for the first individual
   * in generation 0 to return. Set it to 0 to disable this timeout.
   */
  const std::string DEFAULTFIRSTTIMEOUT = EMPTYDURATION; // defined in GEvolutionaryAlgorithm

  /**
   * The default number of milliseconds before the broker times out
   */
  const boost::uint32_t DEFAULTLOOPMSEC = 20;

  /**********************************************************************************/
  /**
   * This population handles optimization in environments where communication between
   * client and communication is handled through a single point of contact. The most
   * likely scenario is a network interface. However, for testing purposes, also a thread
   * consumer interface is available.
   *
   * Note that serialization of this population makes sense only for backup-purposes,
   * in order to allow later, manual recovery. A broker object needs to be registered,
   * and serialization does not help here.
   *
   * Serialization in a network context only happens below the level of this population,
   * it is itself usually not shipped over a network connection.
   */
  class GBrokerEA
    :public GenEvA::GEvolutionaryAlgorithm
  {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template<typename Archive>
    void serialize(Archive & ar, const unsigned int){
      using boost::serialization::make_nvp;
      ar & make_nvp("GEvolutionaryAlgorithm", boost::serialization::base_object<GEvolutionaryAlgorithm>(*this));
      ar & make_nvp("firstTimeOut_", firstTimeOut_);
      ar & make_nvp("loopTime_",loopTime_);
      ar & make_nvp("waitFactor_", waitFactor_);
      ar & make_nvp("maxWaitFactor_", maxWaitFactor_);
    }
    ///////////////////////////////////////////////////////////////////////

  public:
	typedef boost::shared_ptr<Gem::Util::GBufferPortT<boost::shared_ptr<Gem::GenEvA::GIndividual> > > GBufferPortT_ptr;

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
	virtual boost::optional<std::string> checkRelationshipWith(const GObject&, const Gem::Util::expectation&, const double&, const std::string&, const std::string&, const bool&) const;

	/** @brief Performs any necessary initialization work before the start of the optimization cycle */
	virtual void init();
	/** @brief Performs any necessary finalization work after the end of the optimization cycle */
	virtual void finalize();

    /** @brief Sets the wait factor */
    void setWaitFactor(const boost::uint32_t&);
    /** @brief Sets the wait factor, including automatic adaption of the factor */
    void setWaitFactor(const boost::uint32_t&, const boost::uint32_t&);
    /** @brief Retrieves the wait factor */
    boost::uint32_t getWaitFactor() const;
    /** @brief Retrieves the maximum wait factor used in automatic adaption of the wait factor */
    boost::uint32_t getMaxWaitFactor() const;

    /** @brief Sets the first timeout factor */
    void setFirstTimeOut(const boost::posix_time::time_duration&);
    /** @brief Retrieves the first timeout factor */
    boost::posix_time::time_duration getFirstTimeOut() const;

    /** @brief Sets the loop time */
    void setLoopTime(const boost::posix_time::time_duration&);
    /** @brief Retrieves the second part of the loop time */
    boost::posix_time::time_duration getLoopTime() const;

	/** @brief Applies modifications to this object. This is needed for testing purposes */
	virtual bool modify_GUnitTests();
	/** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
	virtual void specificTestsNoFailureExpected_GUnitTests();
	/** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
	virtual void specificTestsFailuresExpected_GUnitTests();

	/** @brief Allows to specify whether logging of arrival times of individuals should be done */
	void doLogging(const bool& dl=true);
	/** @brief Allows to determine whether logging of arrival times has been activated */
	bool loggingActivated() const;
	/** @brief Allows to retrieve the logging results */
	std::vector<std::vector<boost::uint32_t> > getLoggingResults() const;

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

	boost::uint32_t waitFactor_; ///< Affects the timeout for returning individuals
	boost::uint32_t maxWaitFactor_; ///< Determines the maximum allowed wait factor during automatic adaption of the waitFactor_ variable

    boost::posix_time::time_duration firstTimeOut_; ///< Maximum time frame for first individual
    boost::posix_time::time_duration loopTime_;

    GBufferPortT_ptr CurrentBufferPort_; ///< Holds a GBufferPortT object during the optimization cycle

    bool doLogging_; ///< Specifies whether arrival times of individuals should be logged
    std::vector<std::vector<boost::uint32_t> >  arrivalTimes_; ///< Holds the actual arrival times. Note: Neither serialized nor copied
  };

  /**********************************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */

#endif /* GBROKEREA_HPP_ */
