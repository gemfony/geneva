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

#include <iostream>
#include <exception>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <unistd.h>
#include <cstdlib>

#include <boost/version.hpp>

#if BOOST_VERSION < 103600
#error "Error: Boost should at least have version 1.36 !"
#endif /* BOOST_VERSION */

#include <boost/cstdint.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>
#include <boost/function.hpp>
#include <boost/pool/detail/singleton.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>
#include <boost/thread/xtime.hpp>

#ifndef GLOGGER_H_
#define GLOGGER_H_

using namespace std;
using namespace boost;

#include "GLogTargets.hpp"


namespace GenEvA
{
	
  /***********************************************************************************/
  /**
   * Every class in GenEvA should be able to log events, to targets defined by the
   * user. Different log targets, such as console or disk can thus be added to this
   * class. Logging will be done to all registered targets simultaneously. The class
   * allows concurrent access from different sources. Log events are performed in one 
   * go, so that different log events do not interfer. By design it is impossible for
   * the GLogger to be copied.
   */
  class GLogger
    :boost::noncopyable
  {
  public:
    /** \brief The default constructor - needed for the singleton*/
    GLogger(void);
    /** \brief A standard destructor */
    virtual ~GLogger();
    
    /** \brief Adds a log target, such as console or file */
    void addTarget(GBaseLogTarget *gblt);
    /** \brief Does the actual logging */
    void log(const string& msg, uint16_t level);
    /** \brief Sets a log level that should be observed */
    void addLogLevel(uint16_t val);
    /** \brief Adds log levels <= a given threshold */
    void addLogLevelsUpTo(uint16_t val);
    
    /** \brief Checks whether any log targets are present */
    bool hasLogTargets(void) const;

    /** \brief Registers an exception handler to be called when an exception is received */
    void registerExceptionHandler(boost::function<void(const string&)>);
    
  private:
    vector<shared_ptr<GBaseLogTarget> > logVector_; ///< Contains the log targets
    vector<uint16_t> logLevel_; ///< Contains the log levels to be observed
    
    boost::mutex mutex_logger_; ///< Needed for concurrent access
	
    boost::function<void(const string&)> exceptionHandler_;
  };

  /**
   * We currently require the global GLogger object to be a singleton
   */
  typedef boost::details::pool::singleton_default<GLogger> logger;
#define LOGGER logger::instance()

  /***********************************************************************************/

} /* namespace GenEvA */

#endif /*GLOGGER_H_*/

