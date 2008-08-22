/**
 * @file GAsioTCPClient.cpp
 */

/* Copyright (C) 2004-2008 Dr. Ruediger Berlich
 * Copyright (C) 2007-2008 Forschungszentrum Karlsruhe GmbH
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

#include "GAsioTCPClient.hpp"

namespace Gem
{
namespace GenEvA
{

  /***********************************************************************/
  /**
   * The standard constructor for this class.
   *
   * @param server Identifies the server
   * @param port Identifies the port on the server
   */
  GAsioTCPClient::GAsioTCPClient(std::string server, std::string port)
    :GBaseClient(),
     maxStalls_(ASIOMAXSTALLS),
     maxConnectionAttempts_(ASIOMAXCONNECTIONATTEMPTS),
     stalls_(0),
     command_length_(CLIENTCOMMANDLENGTH),
     io_service_(),
     socket_(io_service_),
     resolver_(io_service_),
     query_(server,port),
     endpoint_iterator0_(resolver_.resolve(query_)),
     end_()
  { /* nothing */ }

  /***********************************************************************/
  /**
   * A standard destructor. As we have no local, dynamically allocated
   * data, it is empty.
   */
  GAsioTCPClient::~GAsioTCPClient()
  { /* nothing */ }

  /**********************************************************************/
  /**
   * Sets the maximum number of stalled connection attempts
   *
   * @param maxStalls The maximum number of stalled connection attempts
   */
  void GAsioTCPClient::setMaxStalls(boost::uintre_t maxStalls) throw() {
    maxStalls_ = maxStalls;
  }

  /**********************************************************************/
  /**
   * Retrieves the maximum allowed number of stalled attempts
   *
   * @return The value of the maxStalls_ variable
   */
  boost::uint32_t GAsioTCPClient::getMaxStalls() const throw(){
	  return maxStalls_;
  }

  /**********************************************************************/
  /**
   * Sets the maximum number of failed connection attempts before termination
   *
   * @param maxConnectionAttempts The maximum number of allowed failed connection attempts
   */
  void GAsioTCPClient::setMaxConnectionAttempts(boost::uint32_t maxConnectionAttempts) throw() {
    maxConnectionAttempts_ = maxConnectionAttempts;
  }

  /**********************************************************************/
  /**
   * Retrieves the maximum allowed number of failed connection attempts
   *
   * @return The value of the maxConnectionAttempts_ variable
   */
	boost::uint32_t GAsioTCPClient::getMaxConnectionAttempts() const throw(){
		return maxConnectionAttempts_;
	}

  /**********************************************************************/
  /**
   * Retrieve work items from the server.
   *
   * @param item Holds the string representation of the work item, if successful
   * @param attempt The number of consecutive transfer attempts
   * @return true if operation was successful, otherwise false
   */
  bool GAsioTCPClient::retrieve(std::string& item, boost::uint32_t attempt=0){
	item="empty";

    try{
    	// Restore the start of the iteration. Note that we assume here that the
    	// list of end-points doesn't change during the execution of the client.
    	boost::asio::ip::tcp::resolver::iterator endpoint_iterator=endpoint_iterator0_;

    	// Iterate over all end points, until a suitable one was found
    	boost::system::error_code error = boost::asio::error::host_not_found;
    	while (error && endpoint_iterator != end_){
    		socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
    		socket_.close();
    		socket_.connect(*endpoint_iterator++);
    	}

    	// Still an error ?
    	if(error){

    	}

        // Let the server know we want work
        boost::asio::write(socket_, boost::asio::buffer(assembleQueryString("ready", command_length_)));

        // Read answer. First we care for the command sent by the server
        char inboundCommand[command_length_];
        boost::asio::read(socket_, boost::asio::buffer(inboundCommand));

        // Remove all leading or trailing white spaces from the command
        std::string inboundCommandString=
        	boost::algorithm::trim_copy(std::string(inboundCommand, command_length_));

        // Act on answer.
        if(inboundCommandString == "compute"){
            // We have likely received data. Let's find out how big it is
        	char inboundHeader_[command_length_];
            boost::asio::read(socket_, boost::asio::buffer(inboundHeader_));
            std::string inboundHeader=boost::algorithm::trim_copy(std::string(inboundHeader_, command_length_));
            std::size_t dataSize = boost::lexical_cast<std::size_t>(inboundHeader);

            // Create appropriately sized buffer
            std::vector<char> inboundData(dataSize);

            // Read the real data section from the stream
            boost::asio::read(socket_, boost::asio::buffer(inboundData));

            // And transfer the data into a string
            std::ostringstream oss;
            std::vector<char>::iterator it;
            for(it=inboundData.begin(); it!=inboundData.end(); ++it) oss << *it;

            // Make the data known to the outside world
            item=oss.str();

            // We have successfully retrieved an item, so we need
            // to reset the stall-counter
            stalls_ = 0;

            // Make sure we don't leave any open sockets lying around.
            socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
            socket_.close();

            // Tell GBaseClient::run() that we want to continue
            return true;
        }
        else {// This is either an unknown command, a server-side timeout or the nosuccess command
			// We only allow a given amount of timeouts / stalls
            if(maxStalls_ && (stalls_++ > maxStalls_)){
            	std::ostringstream error;
            	error << "In GAsioTCPClient::retrieve():" << std::endl
    	  	  		  << "Maximum number of consecutive stalls reached or" << std::endl
    	  	  		  << "maximum number of unknown or \"nosuccess\" commands reached: " << stalls_ << std::endl
        	   	      << "with last command = " << inboundCommand << std::endl
        	   	      << "Cannot cope. Leaving now." << std::endl;

            	// Do not leave any open sockets lying around
            	socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
                socket_.close();

        	    return false;
    	    }

            // We can continue. But let's wait approximately 1 second first.
            usleep(999999);
        }
    }
    catch(boost::system::system_error&){
    	if(maxConnectionAttempts_ && (attempt >= maxConnectionAttempts_)){
    		std::ostringstream error;
    		error << "In GAsioTCPClient::retrieve: In attempt " << attempt << ":" << std::endl
				  << "Caught boost::system::system_error exception." << std::endl
				  << "Unable to connect. Leaving now." << std::endl;

    		LOGGER.log(error.str(), Gem::GLogFramework::CRITICAL);

    		// Do not leave any open sockets lying around
    		socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
    		socket_.close();

    		return false;
    	}
    }

	usleep(999999); // Sleep approximately 1 second, then try again
    return retrieve(item, attempt+1);
  }

  /**********************************************************************/
  /**
   * Submit processed items to the server.
   *
   * @param item String to be submitted to the server
   * @param fitness The current fitness of the individual to be submitted
   * @param isDirty Specifies whether the dirty flag was set on the individual
   * @return true if operation was successful, otherwise false
   */
  bool GAsioTCPClient::submit(const std::string& item, const double& fitness, const bool& isDirty){
    // Let's assemble an appropriate buffer
    std::vector<boost::asio::const_buffer> buffers;
    std::string result = assembleQueryString("result",command_length_);
    buffers.push_back(boost::asio::buffer(result));

    // Assemble the size header
    std::string sizeHeader = assembleQueryString(lexical_cast<std::string>(item.size()),command_length_);
    buffers.push_back(boost::asio::buffer(sizeHeader));

    // Finally take care of the data section.
    buffers.push_back(boost::asio::buffer(item));

    // Try to connect to the server. Leave, if this was not successful.
    if(!tryConnect()) return false;

    // And write the serialized data to the socket. We use
    // "gather-write" to send command, header and data in a single
    // write operation.
    boost::asio::write(socket_, buffers);

    // Make sure we don't leave any open sockets lying around.
    socket_.close();
    return true;
  }

  /**********************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */
