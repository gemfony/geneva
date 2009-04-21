/**
 * @file GAsioTCPConsumer.hpp
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
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/thread.hpp>
#include <boost/utility.hpp>
#include <boost/threadpool.hpp>
#include <boost/date_time.hpp>
#include <boost/lexical_cast.hpp>


#ifndef GASIOTCPCONSUMER_HPP_
#define GASIOTCPCONSUMER_HPP_

// Geneva headers go here

#include "GEnums.hpp"
#include "GConsumer.hpp"
#include "GIndividualBroker.hpp"
#include "GAsioHelperFunctions.hpp"
#include "GSerializationHelperFunctions.hpp"
#include "GLogger.hpp"

namespace Gem
{
namespace GenEvA
{

const boost::uint16_t GASIOTCPCONSUMERTHREADS = 4;

/*********************************************************************/
///////////////////////////////////////////////////////////////////////
/*********************************************************************/

/**
 * An instance of this class is created for each new connection request
 * by the client. All the details of the data exchange between server
 * and client are implemented here. The class is declared in the same
 * file as the GAsioTCPConsumer in order to avoid cross referencing of
 * header files.
 */
class GAsioServerSession
	:private boost::noncopyable
{
public:
	/** @brief The standard constructor */
	GAsioServerSession(boost::asio::io_service&, const serializationMode&);
	/** @brief The standard destructor. Note: Non-virtual */
	~GAsioServerSession();

    /** @brief Processes an individual request from a client */
    void processRequest();

	/** @brief Retrieves the socket */
	boost::asio::ip::tcp::socket& socket();

protected:
	/** @brief Retrieve a single command from the stream */
	std::string getSingleCommand();
	/** @brief Write a single command to the stream */
	void sendSingleCommand(const std::string&);
	/** @brief Retrieve items from the client. */
	bool retrieve(std::string&, std::string&, std::string&, std::string&);
	/** @brief Submit items to the client. */
	bool submit(const std::string&, const std::string&, const std::string&);

private:
	GAsioServerSession(); ///< Intentionally left undefined
	GAsioServerSession(const GAsioServerSession&); ///< Intentionally left undefined
	const GAsioServerSession& operator=(const GAsioServerSession&); ///< Intentionally left undefined

	boost::asio::ip::tcp::socket socket_; ///< The underlying socket
	serializationMode serializationMode_; ///< Specifies the serialization mode
};

/*********************************************************************/
///////////////////////////////////////////////////////////////////////
/*********************************************************************/
/**
 * It is the main responsibility of this class to start new server session
 * for each client request.
 */
class GAsioTCPConsumer
	:public Gem::Util::GConsumer // note: GConsumer is non-copyable
{
public:
	/** @brief The standard constructor */
	GAsioTCPConsumer(const unsigned short&);
	/** @brief A standard destructor */
	virtual ~GAsioTCPConsumer();

	/** @brief The actual business logic, called from GMemberBroker */
	virtual void process();
	/** @brief Finalization code, called from GMemberBroker */
	virtual void shutdown();

	/** @brief Retrieves the current serialization mode */
	serializationMode getSerializationMode() const throw();
	/** @brief Sets the serialization mode */
	void setSerializationMode(const serializationMode&) throw();

private:
	/** @brief Called for each new client request */
	void handleAccept(boost::shared_ptr<GAsioServerSession>, const boost::system::error_code&);

	/**
	 * ASIO's io service, responsible for event processing, absolutely
	 * needs to be _before_ acceptor so it gets initialized first.
	 */
	boost::asio::io_service io_service_;
	boost::shared_ptr<boost::asio::io_service::work> work_;

	boost::asio::ip::tcp::acceptor acceptor_; ///< takes care of external connection requests

	boost::threadpool::pool tp_; ///< A simple threadpool, see http://threadpoo.sf.net

	serializationMode serializationMode_; ///< Specifies the serialization mode
};

/*********************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */

#endif /*GASIOTCPCONSUMER_HPP_*/
