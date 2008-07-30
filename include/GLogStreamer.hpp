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

#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <unistd.h>
#include <cstdlib>

#include <boost/version.hpp>

#if BOOST_VERSION < 103600
#error "Error: Boost should at least have version 1.36 !"
#endif /* BOOST_VERSION */

#include <boost/cstdint.hpp>
#include <boost/integer_traits.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>
#include <boost/pool/detail/singleton.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>
#include <boost/thread/xtime.hpp>

#ifndef GLOGSTREAMER_H_
#define GLOGSTREAMER_H_

using namespace std;

#include "GLogger.hpp"
#include "GLogTargets.hpp"
#include "GManipulator.hpp"

namespace GenEvA
{

  /***********************************************************************************/
  /** Definition of log levels */
  const uint16_t EXCEPTIONNOEXIT = boost::integer_traits<uint16_t>::const_max; ///< The application will be terminated through an exception that is thrown
  const uint16_t EXCEPTION = 0; ///< The application will be terminated by the GLogger
  const uint16_t CRITICAL = 1; ///< Warning: it is possible that the application might produce bad results
  const uint16_t UNCRITICAL = 2; ///< The flow of the operation will not be affeced 
  const uint16_t INFORMATIONAL = 3; ///< Useful information
  const uint16_t PROGRESS = 4; ///< Information useful for progress check 
  const uint16_t TRACK = 9; ///< Progress tracking with ROOT
  const uint16_t REPORT = 10; ///< For the GObject::report function and overloaded functions  

  /***********************************************************************************/
  /**
   * The GLogStreamer derives from the std::ostringstream class and thus accepts the
   * same datasources as this class. In particular, data can be streamed to the GLogStreamer,
   * just like one is used to from the iostream library. In addition the class accepts a
   * number of manipulators (in reality functions that emit a GManipulator object, which
   * triggers a dedicated operator<<). This allows to perform logging of the information
   * that was streamed to the GLogStreamer object. This allows for very intuitive logging.
   * It is also possible to collect the date over an entire function and only do the
   * logging at the end. Streaming can be interrupted, where necessary. This class is
   * also the basis for GenEvA's exception mechanism, the philosophy being that every
   * exception should also be logged. Hence exceptions (see the GException class) can be
   * very intuitively filled with information using streams.
   */
  class GLogStreamer
    :public std::ostringstream
  {
  public:
    /** \brief A standard constructor */
    GLogStreamer(void) throw();
    /** \brief Initialisation with a logger object */
    explicit GLogStreamer(GLogger *g) throw();
    /** \brief A standard copy constructor */
    GLogStreamer(const GLogStreamer& cp) throw();
    /** \brief The standard destructor */
    virtual ~GLogStreamer() throw();
    /** \brief Retrieves the content of the ostringstream object */
    string content(void) const;
    /** \brief Stores an empty string in the ostringstream object */
    void reset(void);
    /** \brief Stores a GLogger object in the GLogStreamer object */
    void setLogger(GLogger *g) throw();
    /** \brief Retrieves a pointer to the GLogger object */
    GLogger *logger(void);
    /** \brief Checks whether a GLogger has been registered */
    bool hasLogger(void) const;

    /** \brief Hands over all streaming events to the ostringstream class */
    template <class T>
    GLogStreamer& operator<<(const T& val){
      *((std::ostringstream*)this) << val; // ugly downcast
      return *this;	
    }

    /** \brief Needed for ostringstream */
    GLogStreamer& operator<< (ostream& ( *val )(ostream&));
    /** \brief Needed for ostringstream */
    GLogStreamer& operator<< (ios& ( *val )(ios&));
    /** \brief Needed for ostringstream */
    GLogStreamer& operator<< (ios_base& ( *val )(ios_base&));
    /** \brief A GManipulator object triggers the logging */
    void operator<<(GManipulator gm);
    
  private:
    GLogger *gl; ///< A pointer to the GLogger object, which does the actual logging
  };

  /***********************************************************************************/
  /** \brief This is where the actual logger is called */
  void log_level(GLogStreamer& gss, uint16_t severity);

  /***********************************************************************************/
  /** \brief This is the user-visible front-end to the logging mechanism */
  GManipulator logLevel(uint16_t severity);

  /***********************************************************************************/
	
} /* namespace GenEvA */

#endif /*GLOGSTREAMER_H_*/
