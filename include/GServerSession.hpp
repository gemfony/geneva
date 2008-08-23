/**
 * @file GServerSession.hpp
 */

/* Copyright (C) 2004-2008 Dr. Ruediger Berlich
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

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

// Boost headers go here

#include <boost/version.hpp>

#if BOOST_VERSION < 103600
#error "Error: Boost should at least have version 1.36 !"
#endif /* BOOST_VERSION */

#include <boost/cstdint.hpp>
#include <boost/utility.hpp>
#include <boost/date_time.hpp>
#include <boost/lexical_cast.hpp>

#ifndef GSERVERSESSION_HPP_
#define GSERVERSESSION_HPP_

// Geneva headers go here

#include "GenevaExceptions.hpp"
#include "GIndividualBroker.hpp"
#include "GIndividual.hpp"
#include "GBufferPort.hpp"
#include "GLogger.hpp"

namespace Gem
{
namespace GenEvA
{
  /*********************************************************************/
  /**
   * This class serves as the base class for a session hierarchy. Classes
   * of this hierarchy form the counterpart to remote clients, particular
   * the GBaseClient class.
   */
  class GServerSession: boost::noncopyable
  {
  public:
    /** @brief The default constructor */
    GServerSession();
    /** @brief The standard destructor */
    virtual ~GServerSession();

    /** @brief Processes an individual request from a client */
    void processRequest();

  protected:
    /** @brief Retrieve a single command from the stream */
    virtual std::string getSingleCommand() = 0;
    /** @brief Write a single command to the stream */
    virtual void sendSingleCommand(const std::string&) = 0;
    /** @brief Retrieve items from the client. */
    virtual bool retrieve(std::string&, std::string&, std::string&, std::string&) = 0;
    /** @brief Submit items to the client. */
    virtual bool submit(const std::string&, const std::string&) = 0;
  };

  /*********************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */

#endif /*GSERVERSESSION_HPP_*/
