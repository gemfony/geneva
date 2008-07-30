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

#include <boost/version.hpp>

#if BOOST_VERSION < 103600
#error "Error: Boost should at least have version 1.36 !"
#endif /* BOOST_VERSION */

#include <boost/cstdint.hpp>
#include <boost/bind.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/version.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/thread.hpp>
#include <boost/threadpool.hpp>

#ifndef GBOOSTTHREADCONSUMER_H_
#define GBOOSTTHREADCONSUMER_H_

using namespace std;
using namespace boost;
using namespace boost::threadpool;

#include "GConsumer.hpp"
#include "GBoostThreadGeneral.hpp"

namespace GenEvA
{
  const uint16_t DEFAULTGBTCMAXTHREADS = 5;

  /***************************************************************/
  /**
   * A derivative of GConsumer, that processes items in seperate
   * threads. This class is mainly intended for debuffing purposes.
   * Use GBoostThreadPopulation instead, if you want to have a
   * multi-threaded population.
   */
  class GBoostThreadConsumer
    :public GConsumer
  {
  public:
    /** \brief Standard constructor */
    GBoostThreadConsumer(void);
    /** \brief Standard destructor */
    virtual ~GBoostThreadConsumer();

    /** \brief Sets the maximum number of threads */
    void setMaxThreads(uint16_t);
    /** \brief Retrieves the maximum number of threads */
    uint16_t getMaxThreads(void) const;

  protected:
    /** \brief To be called before operator() */
    virtual void init(void);
    /** \brief The actual business logic */
    virtual void customProcess(void);
    /** \brief To be called after operator() */
    virtual void finally(void);

  private:
    GBoostThreadConsumer(const GBoostThreadConsumer&); ///< Intentionally left undefined
    const GBoostThreadConsumer& operator=(const GBoostThreadConsumer&); ///< Intentionally left undefined

    uint16_t maxThreads_; ///< The maxumum number of allowed threads in the pool
    pool tp_; ///< A thread pool
  };

  /***************************************************************/

}

#endif /*GBOOSTTHREADCONSUMER_H_*/
