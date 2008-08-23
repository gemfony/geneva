/**
 * @file GAsioTCPConsumer.cpp
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

#include "GAsioTCPConsumer.hpp"

namespace Gem
{
namespace GenEvA
{

/*********************************************************************/
/**
 * Standard constructor. Its main purpose it to initialize the underlying
 * socket. We do not initialize the various character arrays and strings,
 * as they are overwritten for each call to this class.
 *
 * @param io_service A reference to the server's io_service
 */
GAsioServerSession::GAsioServerSession(boost::asio::io_service& io_service)
 	:socket_(io_service)
{ /* nothing */ }

/*********************************************************************/
/**
 * A standard destructor. Shuts down and closes the socket.
 */
GAsioServerSession::~GAsioServerSession(){
	try{
		// Make sure we don't leave any open sockets lying around.
		socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
		socket_.close();
	}
	// It is a severe error if an exception is thrown here.
	// Log and leave.
	catch(boost::system::system_error&){
		std::ostringstream error;
		error << "In GAsioServerSession::~GAsioServerSession():" << std::endl
			  << "Caught boost::system::system_error exception" << std::endl;
		LOGGER.log(error.str(), Gem::GLogFramework::CRITICAL);

		std::terminate();
	}
}

/*********************************************************************/
/**
 * Returns the socket used by this object
 *
 * @return The socket used by this object
 */
boost::asio::ip::tcp::socket& GAsioServerSession::socket(){
    return socket_;
}

/*********************************************************************/
/**
 * Retrieve a single command from the stream. It will afterwards have
 * been removed from the stream.
 *
 * @return A single command that has been retrieved from a network socket
 */
std::string GAsioServerSession::getSingleCommand(){
	std::string result = "empty";

	char inbound_command[COMMANDLENGTH];

    // Read a command from the socket. This will remove it from the stream.
    boost::asio::read(socket_, boost::asio::buffer(inbound_command));

    // Remove all leading or trailing white spaces from the command.
    return boost::algorithm::trim_copy(std::string(inbound_command, COMMANDLENGTH));
}

/*********************************************************************/
/**
 * Write a single command to a socket.
 *
 * @param command A command to be written to a socket
 */
void GAsioServerSession::sendSingleCommand(const std::string& command){
	// Format the command ...
	std::string outbound_command = assembleQueryString(command, COMMANDLENGTH);
	// ... and tell the client
	boost::asio::write(socket_, boost::asio::buffer(outbound_command));
}

/*********************************************************************/
/**
 * Retrieves an item from the client (i.e. the socket).
 *
 * @param item An item to be retrieved from the socket
 * @return true if successful, otherwise false
 */
bool GAsioServerSession::retrieve(std::string& itemString, std::string& portid, std::string& fitness, std::string& dirtyflag){
	char inbound_header[COMMANDLENGTH];

	// First read the header from socket_
	boost::asio::read(socket_, boost::asio::buffer(inbound_header));

	// And transform to an integer
	boost::uint16_t dataSize;
	try{
		dataSize=lexical_cast<boost::uint16_t>(boost::algorithm::trim_copy(std::string(inbound_header, COMMANDLENGTH)));
	}
	catch(boost::bad_lexical_cast& e){
		// tried to put an unknown key back - needs to be logged ...
		GLogStreamer gls;
		gls << "GAsioServerSession::retrieve(string&):" << endl
			<< "Conversion of dataSize failed with message" << endl
			<< e.what() << endl
			<< logLevel(UNCRITICAL);

		return false;
	}

	// Read inbound data
	std::vector<char> inboundData(dataSize);
	boost::asio::read(socket_, boost::asio::buffer(inboundData));

	// Transfer data into a string
	std::ostringstream oss;
	for(vector<char>::iterator it=inboundData.begin(); it != inboundData.end(); ++it) oss << *it;
	item = oss.str();

	return true;
}

/*********************************************************************/
/**
 * Submit an item to the client (i.e. the socket).
 *
 * @param item An item to be written to the socket
 * @return true if successful, otherwise false
 */
bool GAsioServerSession::submit(const std::string& item, const std::string& command){
	// Format the command
	std::string outbound_command = assembleQueryString("compute", COMMANDLENGTH);

	// Format the size header
	std::string outbound_header = assembleQueryString(lexical_cast<string>(item.size()), COMMANDLENGTH);

	// Write the serialized data to the socket. We use "gather-write" to send
	// command, header and data in a single write operation.
	std::vector<boost::asio::const_buffer> buffers;

	buffers.push_back(boost::asio::buffer(outbound_command));
	buffers.push_back(boost::asio::buffer(outbound_header));
	buffers.push_back(boost::asio::buffer(item));

	boost::asio::write(socket_, buffers);

	return true;
}

/*********************************************************************/
///////////////////////////////////////////////////////////////////////
/*********************************************************************/
/**
 * Standard constructor. Initializes the acceptor with ioservice and port.
 * It would be good to use boost::uint8_t here, however endpoint uses
 * unsigned short, so we stick with this convention.
 *
 * @param port The port through which clients can access the server
 */
GAsioTCPConsumer::GAsioTCPConsumer(unsigned short port)
	:GConsumer(),
	 acceptor_(io_service_, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)),
	 tp_(GASIOTCPCONSUMERTHREADS),
	 stop_(false)
{ /* nothing */ }

/*********************************************************************/
/**
 * A standard destructor.
 */
GAsioTCPConsumer::~GAsioTCPConsumer()
{
	// Make sure we have no remaining tasks
	tp_.clear(); // Remove pending tasks
	tp_.wait(); // Wait for all running tasks to terminate
}

/*********************************************************************/
/**
 * Handles a new connection request from a client.
 *
 * @param currentSession A pointer to the current session
 * @param error Possible error conditions
 */
void GAsioTCPConsumer::handleAccept(boost::shared_ptr<GAsioServerSession> currentSession, const boost::system::error_code& error)
{
	// First check whether the stop condition was reached or an error occurred in the
	// previous call. If so, we return immediately and thus interrupt the restart of the
	// listeners. io_service_ then runs out of work, and the main loop terminates
	{
		boost::mutex::scoped_lock stopLock(stopMutex_);
		if(error || stop_) return; ///< io_service_ will run out of work and terminate
	}

	// First we make sure a new session is started asynchronously so the next request can be served
	boost::shared_ptr<GAsioServerSession> newSession(new GAsioServerSession(io_service_));
	acceptor_.async_accept(newSession->socket(), boost::bind(&GAsioTCPConsumer::handleAccept, this, newSession,	boost::asio::placeholders::error));

	// Now we can dispatch the actual session code to our thread pool
	tp_.schedule(boost::bind(&GAsioServerSession::processRequest, currentSession));
}

/*********************************************************************/
/**
 * This function is processed in a separate thread, which is started by the broker.
 */
void GAsioTCPConsumer::process(){
	// Start the actual processing. All real work is done in the GAsioServerSession class .
	boost::shared_ptr<GAsioServerSession> newSession(new GAsioServerSession(io_service_));
	acceptor_.async_accept(newSession->socket(), boost::bind(&GAsioTCPConsumer::handleAccept, this, newSession, boost::asio::placeholders::error));

	// The io_service's event loop
	io_service_.run();
}

/*********************************************************************/
/**
 * Finalization code. Sets the stop_ variable to true. handleAccept
 * checks this and terminates instead of starting a new handleAccept
 * session.
 */
void GAsioTCPConsumer::shutdown()
{
	boost::mutex::scoped_lock stopLock(stopMutex_);
	stop_=true;
}

/*********************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */
