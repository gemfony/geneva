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
GAsioServerSession::GAsioServerSession(boost::asio::io_service& io_service, const serializationMode& serMod)
 	:socket_(io_service),
 	 serializationMode_(serMod)
{ /* nothing */ }

/*********************************************************************/
/**
 * A standard destructor. Shuts down and closes the socket.
 */
GAsioServerSession::~GAsioServerSession(){
	try{
		// Make sure we don't leave any open sockets lying around.
		socket_.close();
	}
	// It is a severe error if an exception is thrown here.
	// Log and leave.
	catch(boost::system::system_error&){
		std::ostringstream error;
		error << "In GAsioServerSession::~GAsioServerSession():" << std::endl
			  << "Caught boost::system::system_error exception" << std::endl;
		std::cerr << error.str();
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

	if(command == "getSeed") {
		boost::uint32_t seed = GRANDOMFACTORY->getSeed();

#ifdef DEBUG
		std::cout << "Sending seed " << seed << std::endl;
#endif

		this->sendSingleCommand(boost::lexical_cast<std::string>(seed));
	}
	else if (command == "ready") {
		// Retrieve an item from the broker and submit it to the client.
		try{
			boost::shared_ptr<GIndividual> p;
			Gem::Util::PORTIDTYPE id;

			// Retrieve an item
			id = GINDIVIDUALBROKER->get(p, timeout);

			// Store the id in the individual
			p->setAttribute("id", boost::lexical_cast<std::string>(id));

			if (!this->submit(indptrToString(p, serializationMode_),"compute",boost::lexical_cast<std::string>(serializationMode_))) {
				std::ostringstream information;
				information << "In GAsioServerSession::processRequest():" << std::endl
							<< "Could not submit item to client!" << std::endl;

				std::cout << information.str();
			}
		}
		catch(Gem::Util::gem_util_condition_time_out &gucto) {
			this->sendSingleCommand("timeout");
		}

		// p have automatically been destroyed at this point
		// and should thus exist no longer on the system
	}
	// Retrieve an item from the client and
	// submit it to the broker
	else if (command == "result") {
		std::string itemString, portid, fitness, dirtyflag;

		if (this->retrieve(itemString, portid, fitness, dirtyflag)) {
			// Give the object back to the broker, so it can be sorted back
			// into the GBufferPortT objects.
			boost::shared_ptr<GIndividual> p = indptrFromString(itemString, serializationMode_);

			Gem::Util::PORTIDTYPE id = boost::lexical_cast<Gem::Util::PORTIDTYPE>(portid);
			try {
				GINDIVIDUALBROKER->put(id, p, timeout);
			}
			catch(Gem::Util::gem_util_condition_time_out &gucto){ /* nothing we can do */ }
		}
		else {
			std::ostringstream information;
			information << "GAsioServerSession::processRequest():" << std::endl
						<< "Could not retrieve item from client." << std::endl;
			std::cout << information.str();
		}
	}
	else { // Also covers the "empty" return value of getSingleCommand()
		std::ostringstream warning;
		warning << "In GAsioServerSession::processRequest: Warning!" << std::endl
				<< "Received command \"" << command << "\"" << std::endl;
		std::cout << warning.str();

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
		std::cerr << error.str();

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
		std::cerr << error.str();
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
		char *data_section = new char[dataSize];
		boost::asio::read(socket_, boost::asio::buffer(data_section, dataSize));
		itemString = boost::algorithm::trim_copy(std::string(data_section, dataSize));
		delete [] data_section;
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
bool GAsioServerSession::submit(const std::string& item, const std::string& command, const std::string& serMode){
	// Format the command
	std::string outbound_command_header = assembleQueryString(command, COMMANDLENGTH);

	// Format the size header
	std::string outbound_size_header = assembleQueryString(boost::lexical_cast<std::string>(item.size()), COMMANDLENGTH);

	// Format a header for the serialization mode
	std::string serialization_header = assembleQueryString(serMode, COMMANDLENGTH);

	// Write the serialized data to the socket. We use "gather-write" to send
	// command, header and data in a single write operation.
	std::vector<boost::asio::const_buffer> buffers;

	buffers.push_back(boost::asio::buffer(outbound_command_header));
	buffers.push_back(boost::asio::buffer(outbound_size_header));
	buffers.push_back(boost::asio::buffer(serialization_header));
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
 * unsigned short, so we stick with this convention. The number of threads will
 * be initialized with the number of processor cores, if listenerThreads is
 * set to 0.
 *
 * @param port The port through which clients can access the server
 * @param initialThreads The number of threads used to listen for incoming connections
 */
GAsioTCPConsumer::GAsioTCPConsumer(const unsigned short& port, const std::size_t& listenerThreads)
	:GConsumer(),
	 work_(new boost::asio::io_service::work(io_service_)),
	 acceptor_(work_->get_io_service(), boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)),
	 listenerThreads_(listenerThreads>0?listenerThreads:boost::thread::hardware_concurrency()),
	 tp_(listenerThreads_),
	 serializationMode_(TEXTSERIALIZATION)
{
	if(listenerThreads_ == 0) { // boost::thread::hardware_concurrency() has returned 0. Needs to be corrected.
		listenerThreads_ = GASIOTCPCONSUMERTHREADS;
		tp_.size_controller().resize(listenerThreads_);
	}

	// Start the actual processing. All real work is done in the GAsioServerSession class .
	boost::shared_ptr<GAsioServerSession> newSession(new GAsioServerSession(work_->get_io_service() ,serializationMode_));
	acceptor_.async_accept(newSession->socket(), boost::bind(&GAsioTCPConsumer::handleAccept, this, newSession, _1));
}

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
	// Check whether an error occurred. This will likely indicate that we've been asked to stop.
	if(error) return;

	// First we make sure a new session is started asynchronously so the next request can be served
	boost::shared_ptr<GAsioServerSession> newSession(new GAsioServerSession(work_->get_io_service(), serializationMode_));
	acceptor_.async_accept(newSession->socket(), boost::bind(&GAsioTCPConsumer::handleAccept, this, newSession,	_1));

	// Now we can dispatch the actual session code to our thread pool
	tp_.schedule(boost::bind(&GAsioServerSession::processRequest, currentSession));
}

/*********************************************************************/
/**
 * This function is processed in a separate thread, which is started by the broker.
 */
void GAsioTCPConsumer::process(){
	// The io_service's event loop
	(work_->get_io_service()).run();
}

/*********************************************************************/
/**
 * Finalization code. Sets the stop_ variable to true. handleAccept
 * checks this and terminates instead of starting a new handleAccept
 * session.
 */
void GAsioTCPConsumer::shutdown()
{
	work_.reset();
}

/*********************************************************************/
/**
 * Retrieves the current serialization mode
 *
 * @return The current serialization mode
 */
serializationMode GAsioTCPConsumer::getSerializationMode() const  {
	return serializationMode_;
}

/*********************************************************************/
/**
 * Sets the serialization mode. The only allowed values of the enum serializationMode are
 * BINARYSERIALIZATION, TEXTSERIALIZATION and XMLSERIALIZATION. The compiler does the
 * error-checking for us.
 *
 * @param ser The new serialization mode
 */
void GAsioTCPConsumer::setSerializationMode(const serializationMode& ser)  {
	serializationMode_ = ser;
}

/*********************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */
