/**
 * @file
 */

/* GBroker.hpp
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
#include <vector>
#include <map>
#include <string>

// Boost headers go here

#include <boost/version.hpp>

#if BOOST_VERSION < 103600
#error "Error: Boost should at least have version 1.36 !"
#endif /* BOOST_VERSION */

#include <boost/cstdint.hpp>
#include <boost/pool/detail/singleton.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread/thread.hpp>
#include <boost/utility.hpp>
#include <boost/function.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/thread/mutex.hpp>

#ifndef GBROKER_HPP_
#define GBROKER_HPP_

// GenEvA headers go here

#include "GException.hpp"
#include "GBiBuffer.hpp"
#include "GMemberCarrier.hpp"
#include "GConsumer.hpp"

namespace Gem
{
namespace Util
{
  /**************************************************************************************/

  const boost::uint32_t DEFAULTWAITINGTIMEMSEC = 2; // 0.002 seconds
  const boost::uint32_t DEFAULTWAITINGTIMESEC  = 0;

  /**************************************************************************************/
  /**
   * The GBroker class has a collection of GBiBuffer objects. It manages communication
   * between communicators and populations. Asynchronous processing requests by populations
   * are effectively serialized, so consumer objects can handle communication with the outside
   * world.
   *
   * One other important duty of this class is to start the GConsumer object's processing threads
   * once the first population is registered with this class. Note that no GConsumer threads
   * will be started once this has happened.
   *
   * Note that this class internally uses a vector of pointers. This is usually
   * not good style. However, we do not own these pointers and are not responsible
   * for their destruction. If this GBroker object goes away, the GConsumer
   * objects pointed to will still be there. Also, the gcv_ vector is private
   * and noone except for this class has access to it.
   *
   * This class acts behind the scenes, as it is created as a global singleton that
   * usually only the GTransferPopulation class and the GConsumer derivatives will
   * have knowledge of. There is no need to manually create a GBroker object.
   */
  template <class carrier_type>
  class GBroker
	  :boost::noncopyable
  {
  public:
    typedef shared_ptr<carryer_type> carryer_ptr;
    typedef std::Map<std::string, shared_ptr<GBiBuffer<carryer_ptr> >, less<std::string> > carryer_buffer_map;

    /** * @brief The default constructor */
    GBroker(void);
    /** @brief Standard destructor */
    ~GBroker();

    /** @brief Retrieves a shared_ptr to a GBiBuffer, using a key string */
    GBiBufferPtr at(const std::string&);
    /** @brief Creates a new GBiBuffer object for a given (new) id */
    void enrol(std::string);
    /** @brief Makes a consumer known to this class */
    void enrol(GConsumer<carryer_type> *);
    /** @brief Removes a GBiBuffer object with a given id from the list */
    void signoff(std::string);

    /** @brief Retrieves a "raw" item from a GBiBuffer, observing a timeout */
    bool get(carryer_ptr&, bool &);
    /** @brief Retrieves an item from the broker in text format */
    bool get(std::string&, bool&);

    /** @brief Puts a processed item into the processed queue */
    bool put(const carryer_ptr&);
    /** @brief Submits an item to the broker in text format */
    bool put(const std::string&);

    /** @brief Sets the waiting time used for the GBroker::get function */
    void setWaitingTime(boost::uint32_t, boost::uint32_t);
    /** @brief Retrieves the second part of the timeout value of the get function */
    boost::uint32_t getWaitingTimeSec(void) const;
    /** @brief Retrieves the millisecond part of the timeout value of the get function */
    boost::uint32_t getWaitingTimeMSec(void) const;

    /** @brief Check for the halt condition */
    bool stop(void) const;

    /** @brief Shuts down the broker */
    void shutdown(void);

  private:
    GBroker(const GBroker&); ///< Intentionally left undefined
    const GBroker& operator=(const GBroker&); ///< Intentionally left undefined

    vector<GConsumer *> gcv_; ///< Holds pointers to consumers
    boost::thread_group consumerThreads_; ///< A thread group used to hold consumer threads
    bool noPopulationEnrolled_; ///< No further consumer threads will be started after this variable is set to true

    carryer_buffer_map gbpMap_; ///< Holds GBiBuffer objects and keys

    boost::mutex gbp_mutex_; ///< Access synchronisation for gbpMap_
    boost::mutex gcv_mutex_; ///< Access synchronisation for gcv_
    boost::mutex variable_mutex_; ///< Access to internal variables
    boost::uint32_t waitingTimeSec_; ///< Waiting time of get functions in seconds
    boost::uint32_t waitingTimeMSec_; ///< Waiting time of get functions in milli seconds
    bool halt_; ///< If set to true, a general halt condition was reached
    bool stopped_; ///< Set if stopBrokering() has been called

    bool processingInProgress_; ///< Set by GConsumer::startProcessing()
  };

  /**************************************************************************************/
  /**
   * We require the global GBroker object to be a singleton. This
   * ensures that one and only one Broker objet exists that is constructed
   * before main begins. All external communication should refer to BROKER.
   */
  typedef boost::details::pool::singleton_default<GBroker> gbroker;
#define BROKER gbroker::instance()

  /**************************************************************************************/

} /* namespace Util */
} /* namespace Gem */

#endif /* GBROKER_HPP_ */
