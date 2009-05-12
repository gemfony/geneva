/**
 * @file GAsioTCPClient.hpp
 */

/* Copyright (C) 2004-2008 Dr. Ruediger Berlich
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

// Standard headers go here

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

// Boost headers go here

#include <boost/version.hpp>
#include "GGlobalDefines.hpp"
#if BOOST_VERSION < ALLOWED_BOOST_VERSION
#error "Error: Boost has incorrect version !"
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

#ifndef GASIOTCPCLIENT_HPP_
#define GASIOTCPCLIENT_HPP_

// GenEvA headers go here

#include "GAsioHelperFunctions.hpp"
#include "GSerializationHelperFunctions.hpp"
#include "GBaseClient.hpp"


namespace Gem
{
namespace GenEvA
{

/***********************************************************************/

/**
 * Global variables for failed transfers and connection attempts.
 */
const boost::uint32_t ASIOMAXSTALLS=10;
const boost::uint32_t ASIOMAXCONNECTIONATTEMPTS=10;

/**
 * This class is responsible for the client side of network communication
 * with Boost::Asio.
 */
class GAsioTCPClient
 	:public Gem::GenEvA::GBaseClient
{
public:
	/** @brief The main constructor */
	GAsioTCPClient(const std::string&, const std::string&);
	/** @brief A standard destructor */
	virtual ~GAsioTCPClient();

	/** @brief Sets the maximum allowed number of stalled attempts */
	void setMaxStalls(const boost::uint32_t&) ;
	/** @brief Retrieves the maximum allowed number of stalled attempts */
	boost::uint32_t getMaxStalls() const ;

	/** @brief Sets the maximum allowed number of failed connection attempts */
	void setMaxConnectionAttempts(const boost::uint32_t&) ;
	/** @brief Retrieves the maximum allowed number of failed connection attempts */
	boost::uint32_t getMaxConnectionAttempts() const ;

protected:
	/** @brief Retrieve work items from the server. */
	virtual bool retrieve(std::string&, std::string&);

	/** @brief Submit processed items to the server. */
	virtual bool submit(const std::string&, const std::string&, const std::string&, const std::string&);

private:
	GAsioTCPClient(); ///< Default constructor intentionally left blank
	GAsioTCPClient(const GAsioTCPClient&); ///< Intentionally left undefined
	const GAsioTCPClient& operator=(const GAsioTCPClient&);  ///< Intentionally left undefined

	/** @brief Close socket before leaving */
	bool shutdown(const bool&);

	/** @brief Make a connection attempt */
	bool tryConnect();

	boost::uint32_t maxStalls_; ///< The maximum allowed number of stalled connection attempts
	boost::uint32_t maxConnectionAttempts_; ///< The maximum allowed number of failed connection attempts

	boost::uint32_t stalls_; ///< counter for stalled connection attempts

	char tmpBuffer_[COMMANDLENGTH];

	boost::asio::io_service io_service_; ///< Holds the Boost::ASIO::io_service object

	boost::asio::ip::tcp::socket socket_; ///< The underlying socket

	boost::asio::ip::tcp::resolver resolver_; ///< Responsible for name resolution
	boost::asio::ip::tcp::resolver::query query_; ///< A query

	boost::asio::ip::tcp::resolver::iterator endpoint_iterator0_; ///< start of iteration
	boost::asio::ip::tcp::resolver::iterator end_; ///< end for endpoint iterator
};

/***********************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */

#endif /*GASIOTCPCLIENT_HPP_*/
