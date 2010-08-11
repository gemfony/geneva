 /**
 * @file GAsioTCPConsumerT.hpp
 */

/* Copyright (C) Dr. Ruediger Berlich and Karlsruhe Institute of Technology
 * (University of the State of Baden-Wuerttemberg and National Laboratory
 * of the Helmholtz Association)
 *
 * Contact: info [at] gemfony (dot) com
 *
 * This file is part of the Geneva library, Gemfony scientific's optimization
 * library.
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
 *
 * For further information on Gemfony scientific and Geneva, visit
 * http://www.gemfony.com .
 */

// Standard headers go here

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

// Includes check for correct Boost version(s)
#include "common/GGlobalDefines.hpp"

// Boost headers go here
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
#include <boost/date_time.hpp>
#include <boost/lexical_cast.hpp>
#include <common/thirdparty/boost/threadpool.hpp>


#ifndef GASIOTCPCONSUMERT_HPP_
#define GASIOTCPCONSUMERT_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif


// Geneva headers go here
#include "common/GSerializationHelperFunctionsT.hpp"
#include "hap/GRandomT.hpp"
#include "GAsioHelperFunctions.hpp"
#include "GBrokerT.hpp"
#include "GCommunicationEnums.hpp"
#include "GConsumer.hpp"

namespace Gem
{
namespace Courtier
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
template <class processable_type>
class GAsioServerSession
	:private boost::noncopyable
{
public:
	/*********************************************************************/
	/**
	 * Standard constructor. Its main purpose it to initialize the underlying
	 * socket. We do not initialize the various character arrays and strings,
	 * as they are overwritten for each call to this class.
	 *
	 * @param io_service A reference to the server's io_service
	 */
	GAsioServerSession(boost::asio::io_service& io_service, const Gem::Common::serializationMode& serMod)
	    : socket_(io_service)
	    , serializationMode_(serMod)
	{ /* nothing */ }

	/*********************************************************************/
	/**
	 * A standard destructor. Shuts down and closes the socket. Note: Non-virtual.
	 */
	~GAsioServerSession(){
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
	void processRequest() {
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
	            boost::shared_ptr<processable_type> p;
	            Gem::Common::PORTIDTYPE id;

	            // Retrieve an item
	            id = GBROKER( boost::shared_ptr<processable_type> )->get(p, timeout);

	            if (!this->submit(Gem::Common::sharedPtrToString(p, serializationMode_),
	                              "compute",
	                              boost::lexical_cast<std::string>(serializationMode_),
	                              boost::lexical_cast<std::string>(id)))
	            {
	                std::ostringstream information;
	                information << "In GAsioServerSession::processRequest():" << std::endl
	                            << "Could not submit item to client!" << std::endl;

	                std::cout << information.str();
	            }
	        }
	        catch(Gem::Common::condition_time_out &gucto) {
	            this->sendSingleCommand("timeout");
	        }

	        // p has automatically been destroyed at this point
	        // and should thus exist no longer on the system
	    }
	    // Retrieve an item from the client and
	    // submit it to the broker
	    else if (command == "result") {
	        std::string itemString, portid;

	        if (this->retrieve(itemString, portid)) {
	            // Give the object back to the broker, so it can be sorted back
	            // into the GBufferPortT objects.
	            boost::shared_ptr<processable_type> p = Gem::Common::sharedPtrFromString<processable_type>(itemString, serializationMode_);

	            Gem::Common::PORTIDTYPE id = boost::lexical_cast<Gem::Common::PORTIDTYPE>(portid);
	            try {
	                GBROKER( boost::shared_ptr<processable_type> )->put(id, p, timeout);
	            }
	            catch(Gem::Common::condition_time_out &gucto){ /* nothing we can do */ }
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
	boost::asio::ip::tcp::socket& socket(){
	    return socket_;
	}


protected:

	/*********************************************************************/
	/**
	 * Retrieve a single command from the stream. It will afterwards have
	 * been removed from the stream.
	 *
	 * @return A single command that has been retrieved from a network socket
	 */
	std::string getSingleCommand(){
	    char inbound_command[Gem::Courtier::COMMANDLENGTH];

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
	    return boost::algorithm::trim_copy(std::string(inbound_command, Gem::Courtier::COMMANDLENGTH));
	}

	/*********************************************************************/
	/**
	 * Write a single command to a socket.
	 *
	 * @param command A command to be written to a socket
	 */
	void sendSingleCommand(const std::string& command){
	    // Format the command ...
	    std::string outbound_command = assembleQueryString(command, Gem::Courtier::COMMANDLENGTH);
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
	 * @return true if successful, otherwise false
	 */
	bool retrieve(std::string& itemString, std::string& portid){
	    itemString="";
	    portid="";

	    char inbound_header[Gem::Courtier::COMMANDLENGTH];

	    try{
	        // Read the port id from the socket and translate it into a string
	        boost::asio::read(socket_, boost::asio::buffer(inbound_header,Gem::Courtier::COMMANDLENGTH));
	        portid = boost::algorithm::trim_copy(std::string(inbound_header, Gem::Courtier::COMMANDLENGTH)); // removes white spaces

	        // Read the data size from the socket and translate into a number
	        boost::asio::read(socket_, boost::asio::buffer(inbound_header,Gem::Courtier::COMMANDLENGTH));
	        std::size_t dataSize = boost::lexical_cast<std::size_t>(boost::algorithm::trim_copy(std::string(inbound_header, Gem::Courtier::COMMANDLENGTH)));

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
	bool submit(const std::string& item,
                const std::string& command,
	            const std::string& serMode,
	            const std::string& portId){
	    // Format the command
	    std::string outbound_command_header = assembleQueryString(command, Gem::Courtier::COMMANDLENGTH);

	    // Format the size header
	    std::string outbound_size_header = assembleQueryString(boost::lexical_cast<std::string>(item.size()), Gem::Courtier::COMMANDLENGTH);

	    // Format a header for the serialization mode
	    std::string serialization_header = assembleQueryString(serMode, Gem::Courtier::COMMANDLENGTH);

	    // Format the portId header
	    std::string portid_header = assembleQueryString(portId, Gem::Courtier::COMMANDLENGTH);

	    // Write the serialized data to the socket. We use "gather-write" to send
	    // command, header and data in a single write operation.
	    std::vector<boost::asio::const_buffer> buffers;

	    buffers.push_back(boost::asio::buffer(outbound_command_header));
	    buffers.push_back(boost::asio::buffer(outbound_size_header));
	    buffers.push_back(boost::asio::buffer(serialization_header));
	    buffers.push_back(boost::asio::buffer(portid_header));
	    buffers.push_back(boost::asio::buffer(item));

	    try{
	        boost::asio::write(socket_, buffers);
	    }
	    catch(boost::system::system_error&){ // Transfer didn't work
	        return false;
	    }

	    return true;
	}


private:
	GAsioServerSession(); ///< Intentionally left undefined
	GAsioServerSession(const GAsioServerSession&); ///< Intentionally left undefined
	const GAsioServerSession& operator=(const GAsioServerSession&); ///< Intentionally left undefined

	boost::asio::ip::tcp::socket socket_; ///< The underlying socket
	Gem::Common::serializationMode serializationMode_; ///< Specifies the serialization mode
};

/*********************************************************************/
///////////////////////////////////////////////////////////////////////
/*********************************************************************/
/**
 * It is the main responsibility of this class to start new server session
 * for each client request.
 */
template <class processable_type>
class GAsioTCPConsumerT
	:public Gem::Courtier::GConsumer // note: GConsumer is non-copyable
{
public:

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
    GAsioTCPConsumerT(const unsigned short& port, const std::size_t& listenerThreads = 0)
        :GConsumer(),
         work_(new boost::asio::io_service::work(io_service_)),
         acceptor_(work_->get_io_service(), boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)),
         listenerThreads_(listenerThreads>0?listenerThreads:boost::thread::hardware_concurrency()),
         tp_(listenerThreads_),
         serializationMode_(Gem::Common::SERIALIZATIONMODE_TEXT)
    {
        if(listenerThreads_ == 0) { // boost::thread::hardware_concurrency() has returned 0. Needs to be corrected.
            listenerThreads_ = GASIOTCPCONSUMERTHREADS;
            tp_.size_controller().resize(listenerThreads_);
        }
        // Start the actual processing. All real work is done in the GAsioServerSession class .
        boost::shared_ptr<GAsioServerSession<processable_type> > newSession(new GAsioServerSession<processable_type>(work_->get_io_service() ,serializationMode_));
        acceptor_.async_accept(newSession->socket(), boost::bind(&GAsioTCPConsumerT<processable_type>::handleAccept, this, newSession, _1));
    }

    /*********************************************************************/
    /**
     * A standard destructor.
     */
    ~GAsioTCPConsumerT()
    {
        // Make sure we have no remaining tasks
        tp_.clear(); // Remove pending tasks
        tp_.wait(); // Wait for all running tasks to terminate
    }

    /*********************************************************************/
    /**
     * The actual business logic. This function is processed in a separate thread,
     * which is started by the broker.
     */
    void process(){
        // The io_service's event loop
        (work_->get_io_service()).run();
    }

    /*********************************************************************/
    /**
     * Finalization code. Sets the stop_ variable to true. handleAccept
     * checks this and terminates instead of starting a new handleAccept
     * session.
     */
    void shutdown()
    {
        work_.reset();
    }

    /*********************************************************************/
    /**
     * Retrieves the current serialization mode
     *
     * @return The current serialization mode
     */
    Gem::Common::serializationMode getSerializationMode() const  {
        return serializationMode_;
    }

    /*********************************************************************/
    /**
     * Sets the serialization mode. The only allowed values of the enum serializationMode are
     * Gem::Common::SERIALIZATIONMODE_BINARY, Gem::Common::SERIALIZATIONMODE_TEXT and Gem::Common::SERIALIZATIONMODE_XML.
     * The compiler does the error-checking for us.
     *
     * @param ser The new serialization mode
     */
    void setSerializationMode(const Gem::Common::serializationMode& ser)  {
        serializationMode_ = ser;
    }

private:

    /*********************************************************************/
    /**
     * Handles a new connection request from a client.
     *
     * @param currentSession A pointer to the current session
     * @param error Possible error conditions
     */
    void handleAccept(boost::shared_ptr<GAsioServerSession<processable_type> > currentSession, const boost::system::error_code& error)
    {
        // Check whether an error occurred. This will likely indicate that we've been asked to stop.
        if(error) return;

        // First we make sure a new session is started asynchronously so the next request can be served
        boost::shared_ptr<GAsioServerSession<processable_type> > newSession(new GAsioServerSession<processable_type>(work_->get_io_service(), serializationMode_));
        acceptor_.async_accept(newSession->socket(), boost::bind(&GAsioTCPConsumerT<processable_type>::handleAccept, this, newSession, _1));

        // Now we can dispatch the actual session code to our thread pool
        tp_.schedule(boost::bind(&GAsioServerSession<processable_type>::processRequest, currentSession));
    }

	/**
	 * ASIO's io service, responsible for event processing, absolutely
	 * needs to be _before_ acceptor so it gets initialized first.
	 */
	boost::asio::io_service io_service_;
	boost::shared_ptr<boost::asio::io_service::work> work_;

	boost::asio::ip::tcp::acceptor acceptor_; ///< takes care of external connection requests

	std::size_t listenerThreads_; /// The number of threads used to listen for incoming connections

	boost::threadpool::pool tp_; ///< A simple threadpool, see http://threadpoo.sf.net

	Gem::Common::serializationMode serializationMode_; ///< Specifies the serialization mode
};

/*********************************************************************/

} /* namespace Courtier */
} /* namespace Gem */

#endif /*GASIOTCPCONSUMERT_HPP_*/
