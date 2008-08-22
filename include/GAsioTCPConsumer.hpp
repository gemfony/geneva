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
#include <boost/utility.hpp>
#include <boost/threadpool.hpp>


#ifndef GASIOTCPCONSUMER_H_
#define GASIOTCPCONSUMER_H_

using namespace std;
using namespace boost;
using boost::asio::ip::tcp;

#include "GConsumer.hpp"
#include "GMemberBroker.hpp"
#include "GHelperFunctions.hpp"
#include "GServerSession.hpp"


namespace GenEvA
{

const uint16_t GASIOTCPCONSUMERTHREADS = 4;

/*********************************************************************/
///////////////////////////////////////////////////////////////////////
/*********************************************************************/

/**
 * An instance of this class is created for each new connection request
 * by the client. All the details of the data exchange between server
 * and client are implemented here.
 */
class GAsioServerSession
	:public GServerSession
{
public:
	/** \brief The standard constructor */
	GAsioServerSession(boost::asio::io_service&);
	/** \brief The standard destructor */
	virtual ~GAsioServerSession();

	/** \brief Retrieves the socket */
	boost::asio::ip::tcp::socket& socket(void);

protected:
	/** \brief Retrieve a single command from the stream */
	virtual string getSingleCommand(void);
	/** \brief Write a single command to the stream */
	virtual void sendSingleCommand(const string&);
	/** \brief Retrieve items from the client. */
	virtual bool retrieve(string&);
	/** \brief Submit items to the client. */
	virtual bool submit(const string&);

private:
	tcp::socket socket_; ///< The underlying socket.
};

/*********************************************************************/
///////////////////////////////////////////////////////////////////////
/*********************************************************************/
/**
 * It is the main responsibility of this class to start new server session
 * for each client request.
 */
class GAsioTCPConsumer
	:public GenEvA::GConsumer // note: GConsumer is noncopyable
{
public:
	/** \brief The standard constructor */
	GAsioTCPConsumer(uint8_t);
	/** \brief A standard destructor */
	virtual ~GAsioTCPConsumer();

	/** \brief Initialization code, called from GMemberBroker */
	virtual void init(void);
	/** \brief The actual business logic, called from GMemberBroker */
	virtual void customProcess(void);
	/** \brief Finalization code, called from GMemberBroker */
	virtual void finally(void);

private:
	/** \brief Called for each new client request */
	void handleAccept(shared_ptr<GAsioServerSession>, const boost::system::error_code&);

	/**
	 * ASIO's ioservice, responsible for event processing, absolutely
	 * needs to be _before_ acceptor so it gets initialized first.
	 */
	boost::asio::io_service _io_service;
	boost::asio::ip::tcp::acceptor _acceptor; ///< takes care of external connection requests

	boost::threadpool::pool tp;
};

/*********************************************************************/

} /* namespace GenEvA */

#endif /*GASIOTCPCONSUMER_H_*/
