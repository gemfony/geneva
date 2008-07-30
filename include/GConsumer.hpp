/**
 * \file
 */

/**
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

#include <sstream>

#include <boost/version.hpp>

#if BOOST_VERSION < 103600
#error "Error: Boost should at least have version 1.36 !"
#endif /* BOOST_VERSION */

#include <boost/cstdint.hpp>
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
#include <boost/exception.hpp>
#include <boost/thread/mutex.hpp>

#ifndef GCONSUMER_H_
#define GCONSUMER_H_

using namespace std;

#include "GMemberCarrier.hpp"
#include "GLogStreamer.hpp"
#include "GException.hpp"

namespace GenEvA
{

  /***************************************************************/
  /**
   * Forward declaration to break cyclic dependency.
   * #include can be found in GConsumer.cpp .
   */
  class GMemberBroker;

  /**
   * This class forms the basis of a hierarchy of classes that take
   * GMember objects from the GMemberBroker and process them, either
   * locally or remotely. Derived classes such as the GAsioTCPConsumer
   * form the single point of contact for remote clients. We do not want
   * this class and its derivatives to be copyable, hence we derive it
   * from the boost::noncopyable class.
   */
  class GConsumer
    :boost::noncopyable // private derivation
  {
  public:
    /**
     * GMemberBroker is declared friend so that it is the only class
     * that can access the GConsumer::setStopCondition() function
     */
    friend class GMemberBroker;

    /** Standard constructor */
    GConsumer(void);

    /** \brief Standard destructor */
    virtual ~GConsumer();

  protected:
    /** \brief To be called before GConsumer::process() from GMemberBroker */
    virtual void init(void) = 0;
    /** \brief The actual business logic */
    virtual void customProcess(void) = 0;
    /** \brief A wrapper around customProcess needed to catch exceptions */
    void process(void);
    /** \brief To be called after GConsumer::process() from GMemberBroker*/
    virtual void finally(void) = 0;

    /** \brief Sets the stop condition. Called by GMemberBroker */
    void setStopCondition(void);
    /** \brief Checks whether a stop condition was reached */
    bool stopConditionReached(void) const;

  private:
    GConsumer(const GConsumer&); ///< Intentionally left undefined
    const GConsumer& operator=(const GConsumer&); ///< Intentionally left undefined

    boost::mutex stop_mutex_;
    bool stopConditionReached_; ///< Indicates that a stop condition was reached
  };

  /***************************************************************/

}

#endif /*GCONSUMER_H_*/
