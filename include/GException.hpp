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
#include <boost/pool/detail/singleton.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>
#include <boost/thread/xtime.hpp>

#ifndef GEXCEPTION_H_
#define GEXCEPTION_H_

using namespace std;
using namespace boost;

#include "GLogStreamer.hpp"
#include "GLogTargets.hpp"
#include "GLogger.hpp"


namespace GenEvA
{



  /***********************************************************************************/
  /**
   * \brief This function is interpreted by
   * GException as a sign to throw itself
   */
  void raiseException(void);

  /***********************************************************************************/
  /**
   * In the GenEvA context, exceptions are coupled to the logging mechanism. The
   * philosophy is that ever exception should also be logged. By deriving from the
   * GLogStreamer object, we can achieve a very intuitive way of "filling" an
   * exception with useful information (similar to the iostream mechanism), and logging
   * of exceptions comes almost for free, without disturbing the flow of the program,
   * if no exception is thrown.
   */
  class GException
    :public std::exception, 
     public GLogStreamer
  {
  public:
    /** \brief The standard constructor */
    GException(void) throw();
    /** \brief A standard copy constructor */
    GException(const GException& cp) throw();
    /** \brief The standard destructor */
    virtual ~GException() throw();
    /** \brief Sets a specific error message */
    void setError(string error);
    /** \brief Retrieves the error message */
    string getError(void) const throw();
    /** \brief Interface to the C++ exception mechanism */
    virtual const char *what(void) const throw();

    /** \brief Forces GException to throw itself as an exception */
    void forceException(bool fe=true);
    /** \brief Retrieve the forceException_ parameter */
    bool getForceException(void) const throw();
    
    /** Hands over streaming events to the unerlying GLogStreamer */
    template <class T>
    inline GException& operator<<(const T& val){
      *((GLogStreamer*)this)<<val;
      return *this;
    }

    /** \brief Needed for the streaming mechanism */
    GException& operator<< (ostream& ( *val )(ostream&));
    /** \brief Needed for the streaming mechanism */
    GException& operator<< (ios& ( *val )(ios&));
    /** \brief Needed for the streaming mechanism */
    GException& operator<< (ios_base& ( *val )(ios_base&));
    /** \brief Triggers the throwing of this object */
    void operator<< (void (*val) (void));

  private:
    string error_; ///< The error message
    bool forceException_;
  };

  /***********************************************************************************/

} /* namespace GenEvA */

#endif /*GEXCEPTION_H_*/
