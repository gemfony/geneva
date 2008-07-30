/**
 * \file
 */

/**
 * Copyright (C) 2004-2008 Dr. Ruediger Berlich
 * Copyright (C) 2008 Forschungszentrum Karlsruhe GmbH
 * 
 * This file is part of the Geneva library.
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
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread/mutex.hpp>

#ifndef GASIOTCPCLIENT_H_
#define GASIOTCPCLIENT_H_

using namespace std;
using namespace boost;

#include "GBaseClient.hpp"
#include "GConsumer.hpp"
#include "GHelperFunctions.hpp"


namespace GenEvA
{

/***********************************************************************/

/**
 * Global variables for failed transfers and connection attempts.
 */
const uint16_t ASIOMAXSTALLS=10;
const uint16_t ASIOMAXCONNECTIONATTEMPTS=10;

/**
 * This class is responsible for the client side of network communication
 * with Boost::ASIO.
 */
class GAsioTCPClient 
 	:public GenEvA::GBaseClient
{
public:
	/** \brief The main constructor */
	GAsioTCPClient(string, string);
	/** \brief A standard destructor */
	virtual ~GAsioTCPClient();
	
	/** \brief Sets the maximum allowed number of stalled attempts */
	void setMaxStalls(uint16_t);
	/** \brief Sets the maximum allowed number of failed connection attempts */
	void setMaxConnectionAttempts(uint16_t);
	
protected:
	/** \brief Retrieve work items from the server. */
	virtual bool retrieve(string&);

	/** \brief Submit processed items to the server. */
	virtual bool submit(const string&);

private:
	/** \brief Default constructor intentionally left blank */
	GAsioTCPClient(); 

	/** \brief Tries to connect to the server */
	bool tryConnect(uint16_t);

	uint16_t maxStalls_; ///< The maximum allowed number of stalled connection attempts
	uint16_t maxConnectionAttempts_; ///< The maximum allowed number of failed connection attempts
	
	uint16_t stalls; ///< counter for stalled connection attempts 
	
	enum { command_length = 32 }; ///< The size of the command header.		    
	enum { header_length = 8 }; ///< The size of a fixed length header.
		
	boost::asio::io_service io_service_; ///< Holds the Boost::ASIO::io_service object
	
	boost::asio::ip::tcp::socket socket_; ///< The underlying socket
	
	boost::asio::ip::tcp::resolver resolver_; ///< Responsible for name resolution
	boost::asio::ip::tcp::resolver::query query_; ///< A query

	boost::asio::ip::tcp::resolver::iterator endpoint_iterator0; ///< start of iteration
	boost::asio::ip::tcp::resolver::iterator end; ///< end for endpoint iterator
};

/***********************************************************************/

} /* namespace GenEvA */

#endif /*GASIOTCPCLIENT_H_*/
