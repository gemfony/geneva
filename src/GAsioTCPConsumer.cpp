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
 * This function processes an individual request from a client.
 */
void GAsioServerSession::processRequest() {
	boost::posix_time::time_duration timeout(boost::posix_time::milliseconds(10));

	// Every client transmission starts with a command
	std::string command = getSingleCommand();

	// Retrieve an item from the broker and
	// submit it to the client.
	if (command == "ready") {
		try{
			boost::shared_ptr<GIndividual> p(new GIndividual);
			Gem::Util::PORTIDTYPE id;

			// Retrieve an item
			id = GINDIVIDUALBROKER.get(p, timeout);


			// Store the id in the individual
			p->setAttribute("id", boost::lexical_cast<std::string>(id));

			if (!this->submit(p->toString(),"compute")) {
				std::ostringstream information;
				information << "In GAsioServerSession::processRequest():" << std::endl
							<< "Could not submit item to client!" << std::endl;

				LOGGER.log(information.str(), Gem::GLogFramework::INFORMATIONAL);
			}
		}
		catch(Gem::Util::gem_util_condition_time_out &) {
			this->sendSingleCommand("timeout");
		}
	}
	// Retrieve an item from the client and
	// submit it to the broker
	else if (command == "result") {
		std::string itemString, portid, fitness, dirtyflag;

		if (this->retrieve(itemString, portid, fitness, dirtyflag)) {
			// Give the object back to the broker, so it can be sorted back
			// into the GBufferPort objects.
			boost::shared_ptr<GIndividual> p(new GIndividual(itemString, TEXTSERIALIZATION));
			Gem::Util::PORTIDTYPE id = boost::lexical_cast<Gem::Util::PORTIDTYPE>(portid);
			try {
				GINDIVIDUALBROKER.put(id, p, timeout);
			}
			catch(Gem::Util::gem_util_condition_time_out &){ /* nothing we can do */ }
		}
		else {
			std::ostringstream information;
			information << "GAsioServerSession::processRequest():" << std::endl
						<< "Could not retrieve item from client." << std::endl;
			LOGGER.log(information.str(), Gem::GLogFramework::INFORMATIONAL);
		}
	}
	else { // Also covers the "empty" return value of getSingleCommand()
		std::ostringstream warning;
		warning << "In GAsioServerSession::processRequest: Warning!" << std::endl
				<< "Received command \"" << command << "\"" << std::endl;
		LOGGER.log(warning.str(), Gem::GLogFramework::WARNING);

		this->sendSingleCommand("unknown");
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
	char inbound_command[COMMANDLENGTH];

	try{
		// Read a command from the socket. This will remove it from the stream.
		boost::asio::read(socket_, boost::asio::buffer(inbound_command));
	}
	catch(boost::system::system_error&){
		std::ostringstream error;
		error << "In GAsioServerSession::getSingleCommand(): Warning" << std::endl
			  << "Caught boost::system::system_error exception. The function" << std::endl
			  << "will return the \"empty\" value." << std::endl;
		LOGGER.log(error.str(), Gem::GLogFramework::WARNING);

		return std::string("empty");
	}

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
	try{
		boost::asio::write(socket_, boost::asio::buffer(outbound_command));
	}
	catch(boost::system::system_error&){
		std::ostringstream error;
		error << "In GAsioServerSession::sendSingleCommand(): Warning" << std::endl
			  << "Caught boost::system::system_error exception" << std::endl;
		LOGGER.log(error.str(), Gem::GLogFramework::WARNING);
	}
}

/*********************************************************************/
/**
 * Retrieves an item from the client (i.e. the socket).
 *
 * @param itemString An item to be retrieved from the socket
 * @param portid String representation of the id of the port the item originated from
 * @param fitness String representation of the fitness of the item
 * @param dirtyFlag Indicates whether the dirtyFlag of the item is set
 * @return true if successful, otherwise false
 */
bool GAsioServerSession::retrieve(std::string& itemString, std::string& portid, std::string& fitness, std::string& dirtyFlag){
	itemString="";
	portid="";
	fitness="";
	dirtyFlag="";

	char inbound_header[COMMANDLENGTH];

	try{
		// Read the port id from the socket and translate it into a string
		boost::asio::read(socket_, boost::asio::buffer(inbound_header,COMMANDLENGTH));
		portid = boost::algorithm::trim_copy(std::string(inbound_header, COMMANDLENGTH)); // removes white spaces

		// Read the fitness from the socket
		boost::asio::read(socket_, boost::asio::buffer(inbound_header,COMMANDLENGTH));
		fitness = boost::algorithm::trim_copy(std::string(inbound_header, COMMANDLENGTH));

		// Read the dirty flag from the socket
		boost::asio::read(socket_, boost::asio::buffer(inbound_header,COMMANDLENGTH));
		dirtyFlag = boost::algorithm::trim_copy(std::string(inbound_header, COMMANDLENGTH));

		// Read the data size from the socket and translate into a number
		boost::asio::read(socket_, boost::asio::buffer(inbound_header,COMMANDLENGTH));
		std::size_t dataSize = boost::lexical_cast<std::size_t>(boost::algorithm::trim_copy(std::string(inbound_header, COMMANDLENGTH)));

		// Read item data and transfer into itemString
		char data_section[dataSize];
		boost::asio::read(socket_, boost::asio::buffer(data_section, dataSize));
		itemString = boost::algorithm::trim_copy(std::string(data_section, dataSize));
	}
	catch(boost::system::system_error&){ // Retrieval didn't work
		return false;
	}

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
	std::string outbound_command_header = assembleQueryString(command, COMMANDLENGTH);

	// Format the size header
	std::string outbound_size_header = assembleQueryString(boost::lexical_cast<std::string>(item.size()), COMMANDLENGTH);

	// Write the serialized data to the socket. We use "gather-write" to send
	// command, header and data in a single write operation.
	std::vector<boost::asio::const_buffer> buffers;

	buffers.push_back(boost::asio::buffer(outbound_command_header));
	buffers.push_back(boost::asio::buffer(outbound_size_header));
	buffers.push_back(boost::asio::buffer(item));

	try{
		boost::asio::write(socket_, buffers);
	}
	catch(boost::system::system_error&){ // Transfer didn't work
		return false;
	}

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
