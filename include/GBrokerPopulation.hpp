/**
 * @file GBrokerPopulation.hpp
 */

/* Copyright (C) 2004-2009 Dr. Ruediger Berlich
 * Copyright (C) 2007-2009 Forschungszentrum Karlsruhe GmbH
 *
 * This file is part of Geneva, Gemfony scientific's optimization library.
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

#ifndef GBROKERPOPULATION_HPP_
#define GBROKERPOPULATION_HPP_

// GenEvA headers go here

#include "GenevaExceptions.hpp"
#include "GIndividual.hpp"
#include "GBasePopulation.hpp"
#include "GBufferPortT.hpp"
#include "GIndividualBroker.hpp"

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
  const std::string DEFAULTFIRSTTIMEOUT = EMPTYDURATION; // defined in GBasePopulation

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
   *
   * @todo Find a mechanism to prevent usage of this population inside another broker population
   */
  class GBrokerPopulation
    :public GenEvA::GBasePopulation
  {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template<typename Archive>
    void serialize(Archive & ar, const unsigned int){
      using boost::serialization::make_nvp;
      ar & make_nvp("GBasePopulation", boost::serialization::base_object<GBasePopulation>(*this));
      ar & make_nvp("firstTimeOut_", firstTimeOut_);
      ar & make_nvp("loopTime_",loopTime_);
      ar & make_nvp("waitFactor_", waitFactor_);
      ar & make_nvp("maxWaitFactor_", maxWaitFactor_);
    }
    ///////////////////////////////////////////////////////////////////////

  public:
	typedef boost::shared_ptr<Gem::Util::GBufferPortT<boost::shared_ptr<Gem::GenEvA::GIndividual> > > GBufferPortT_ptr;

	/** @brief The standard constructor */
    GBrokerPopulation();
    /** @brief A standard copy constructor */
    GBrokerPopulation(const GBrokerPopulation&);
    /** @brief The standard destructor */
    virtual ~GBrokerPopulation();

    /** @brief A standard assignment operator */
    const GBrokerPopulation& operator=(const GBrokerPopulation&);

    /** @brief Loads the data of another GTransfer Population */
    virtual void load(const GObject *);
    /** @brief Creates a deep copy of this object */
    virtual GObject *clone() const;

	/** @brief Checks for equality with another GBrokerPopulation object */
	bool operator==(const GBrokerPopulation&) const;
	/** @brief Checks for inequality with another GBrokerPopulation object */
	bool operator!=(const GBrokerPopulation&) const;
	/** @brief Checks for equality with another GBrokerPopulation object */
	virtual bool isEqualTo(const GObject&, const boost::logic::tribool& expected = boost::logic::indeterminate) const;
	/** @brief Checks for similarity with another GBrokerPopulation object */
	virtual bool isSimilarTo(const GObject&, const double&, const boost::logic::tribool& expected = boost::logic::indeterminate) const;

    /** @brief Starts the optimization cycle */
    virtual void optimize();

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

  protected:
    /** @brief Mutates all children in sequence */
    virtual void mutateChildren();
    /** @brief Selects new parents */
    virtual void select();

  private:
	boost::uint32_t waitFactor_; ///< Affects the timeout for returning individuals
	boost::uint32_t maxWaitFactor_; ///< Determines the maximum allowed wait factor during automatic adaption of the waitFactor_ variable

    boost::posix_time::time_duration firstTimeOut_; ///< Maximum time frame for first individual
    boost::posix_time::time_duration loopTime_;

    GBufferPortT_ptr CurrentBufferPort_; ///< Holds a GBufferPortT object during the optimization cycle
  };

  /**********************************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */

#endif /* GBROKERPOPULATION_HPP_ */
