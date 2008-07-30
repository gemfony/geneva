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
#include <sstream>
#include <string>
#include <vector>

#include <boost/version.hpp>

#if BOOST_VERSION < 103600
#error "Error: Boost should at least have version 1.36 !"
#endif /* BOOST_VERSION */

#include <boost/cstdint.hpp>
#include <boost/utility.hpp>

#ifndef GSESSION_H_
#define GSESSION_H_

using namespace std;
using namespace boost;

#include "GMemberBroker.hpp"

namespace GenEvA
{
  /*********************************************************************/
  ///////////////////////////////////////////////////////////////////////
  /*********************************************************************/
  /**
   * This class serves as the base class for a session hierarchy. Classes
   * of this hierarchy form the counterpart to remote clients, particular
   * the GBaseClient class.
   */
  class GServerSession
    :boost::noncopyable
  {
  public:
    /** \brief The default constructor */
    GServerSession();
    /** \brief The standard destructor */
    virtual ~GServerSession();
	
    /** \brief Processes an individual request from a client */
    void processRequest(void);
	
  protected:
    /** \brief Retrieve a single command from the stream */
    virtual string getSingleCommand(void) = 0; 	
    /** \brief Write a single command to the stream */
    virtual void sendSingleCommand(const string&) = 0;
    /** \brief Retrieve items from the client. To be defined in derived classes. */
    virtual bool retrieve(string&) = 0;
    /** \brief Submit items to the client. To be defined in derived classes. */
    virtual bool submit(const string&) = 0;
  };

  /*********************************************************************/

} /* namespace GenEvA */

#endif /*GSESSION_H_*/
