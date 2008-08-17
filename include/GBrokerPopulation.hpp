/**
 * @file
 */

/* GBrokerPopulation.hpp
 *
 * Copyright (C) 2004-2008 Dr. Ruediger Berlich
 * Copyright (C) 2007-2008 Forschungszentrum Karlsruhe GmbH
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


// Boost headers go here

#include <boost/version.hpp>

#if BOOST_VERSION < 103600
#error "Error: Boost should at least have version 1.36 !"
#endif /* BOOST_VERSION */

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
#include "GLogger.hpp"
#include "GIndividual.hpp"
#include "GBasePopulation.hpp"
#include "GBufferPort.hpp"
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
  const boost::uint32_t DEFAULTWAITFACTOR = 5;

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
   */
  class GBrokerPopulation
    :public GenEvA::GBasePopulation
  {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version){
      using boost::serialization::make_nvp;
      ar & make_nvp("GBasePopulation", boost::serialization::base_object<GBasePopulation>(*this));
      ar & make_nvp("firstTimeOut_", firstTimeOut_);
      ar & make_nvp("loopTime_",loopTime_);
      ar & make_nvp("waitFactor_", waitFactor_);
    }
    ///////////////////////////////////////////////////////////////////////

  public:
    /** \brief The standard constructor */
    GBrokerPopulation();
    /** \brief A standard copy constructor */
    GBrokerPopulation(const GBrokerPopulation&);
    /** \brief The standard destructor */
    virtual ~GBrokerPopulation();

    /** \brief A standard assignment operator */
    const GBrokerPopulation& operator=(const GBrokerPopulation&);

    /** \brief Loads the data of another GTransfer Population */
    virtual void load(const GObject *);
    /** \brief Creates a deep copy of this object */
    virtual GObject *clone();

    /** \brief Starts the optimization cycle */
    virtual void optimize();

    /** \brief Sets the wait factor */
    void setWaitFactor(boost::uint32_t) throw();
    /** \brief Retrieves the wait factor */
    uint32_t getWaitFactor() const throw();

    /** \brief Sets the first timeout factor */
    void setFirstTimeOut(const boost::posix_time::time_duration&);
    /** \brief Retrieves the first timeout factor */
    boost::posix_time::time_duration getFirstTimeOut() const;

    /** \brief Sets the loop time */
    void setLoopTime(const boost::posix_time::time_duration&);
    /** \brief Retrieves the second part of the loop time */
    boost::posix_time::time_duration getLoopTime() const;

  protected:
    /** \brief Mutates all children in sequence */
    virtual void mutateChildren();
    /** \brief Selects new parents */
    virtual void select();

  private:
    boost::posix_time::time_duration firstTimeOut_; ///< Maximum time frame for first individual
    boost::posix_time::time_duration loopTime_;

    boost::uint32_t waitFactor_; ///< Affects the timeout for returning individuals

    shared_ptr<GBufferPort> CurrentBufferPort_; ///< Holds a GBufferPort object during the optimization cycle
  };

  /**********************************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */

#endif /* GBROKERPOPULATION_HPP_ */
