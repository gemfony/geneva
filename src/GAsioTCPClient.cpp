/**
 * \file
 */

/**
 * Copyright (C) 2004-2008 Dr. Ruediger Berlich
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

namespace GenEvA
{

  /***********************************************************************/
  /**
   * The standard constructor.
   *
   * \param server Identifies the server
   * \param port Identifies the port on the server
   */
  GAsioTCPClient::GAsioTCPClient(string server, string port)
    :GBaseClient(),
     socket_(io_service_),
     resolver_(io_service_),
     query_(server,port)
  {
	maxStalls_ = ASIOMAXSTALLS;
    maxConnectionAttempts_ = ASIOMAXCONNECTIONATTEMPTS;
    stalls=0;

    endpoint_iterator0=resolver_.resolve(query_);
  }

  /***********************************************************************/
  /**
   * A standard destructor. As we have no local, dynamically allocated
   * data, it is empty.
   */
  GAsioTCPClient::~GAsioTCPClient()
  { /* nothing */ }

  /**********************************************************************/
  /**
   * Recursively tries to connect to the server, until a maximum number
   * of connection requests was made.
   *
   * \param attempt The current connection attempt
   */
  bool GAsioTCPClient::tryConnect(uint16_t attempt=0){
    // Restore the start of the iteration
    boost::asio::ip::tcp::resolver::iterator endpoint_iterator=endpoint_iterator0;
    boost::system::error_code error = boost::asio::error::host_not_found;
    while (error && endpoint_iterator != end)
      {
    	socket_.close();
    	socket_.connect(*endpoint_iterator++, error);
      }

    if(error){
      if(maxConnectionAttempts_ && (attempt >= maxConnectionAttempts_)){
	     GLogStreamer gls;
	     gls << "In GAsioTCPClient::tryConnect() : Unable to connect" << endl
	     	 << "in attempt " << attempt << endl
	     	 << logLevel(CRITICAL);

	     // Make sure we don't leave any open socket lying around
	     socket_.close();
	     return false;
      }
      else{
    	 usleep(999999); // ~1s
    	 GLogStreamer gls;
    	 gls << "In GAsioTCPClient::tryConnect()" << endl
    	     << "Connection attempt " << attempt << endl
    	     << logLevel(CRITICAL);
    	 return tryConnect(attempt+1);
      	}
    }

    return true;
  }

  /**********************************************************************/
  /**
   * Sets the maximum number of stalled connection attempts
   *
   * \param maxStalls The maximum number of stalled connection attempts
   */
  void GAsioTCPClient::setMaxStalls(uint16_t maxStalls){
    maxStalls_ = maxStalls;
  }

  /**********************************************************************/
  /**
   * Sets the maxumum number of failed connection attempts before termination
   *
   * \param maxConnectionAttempts The maximum number of allowed failed connection attempts
   */
  void GAsioTCPClient::setMaxConnectionAttempts(uint16_t maxConnectionAttempts){
    maxConnectionAttempts_ = maxConnectionAttempts;
  }

  /**********************************************************************/
  /**
   * Retrieve work items from the server.
   *
   * \param item String containing a description of a work item
   * \return true if operation was successful, otherwise false
   */
  bool GAsioTCPClient::retrieve(string& item){
	// Initialise payload
	item="empty";

    // Try to connect to the server. Leave, if this was not succesful.
    if(!tryConnect()) return false;

    // Let the server know we want work
    boost::asio::write(socket_, boost::asio::buffer(assembleQueryString("getdata", command_length)));

    // Read answer. First we care for the command sent by the server
    char inboundCommand[command_length];
    boost::asio::read(socket_, boost::asio::buffer(inboundCommand));

    // Remove all leading or trailing white spaces from the command
    string inboundCommandString=
    	boost::algorithm::trim_copy(std::string(inboundCommand, command_length));

    // Act on answer. The only answer we accept is "compute".
    if(inboundCommandString == "compute"){
        // We have likely received data. Let's find out how big it is
    	char inboundHeader_[command_length];
        boost::asio::read(socket_, boost::asio::buffer(inboundHeader_));
        string inboundHeader=boost::algorithm::trim_copy(std::string(inboundHeader_, command_length));
        uint16_t dataSize = lexical_cast<uint16_t>(inboundHeader);

        // Read inbound data into appropriately sized buffer
        vector<char> inboundData;
        inboundData.resize(dataSize);

        // Read the real data section from the stream
        boost::asio::read(socket_, boost::asio::buffer(inboundData));

        // And transfer the data into a string
        ostringstream oss;
        vector<char>::iterator it;
        for(it=inboundData.begin(); it!=inboundData.end(); ++it) oss << *it;

        // Make the data known to the outside world
        item=oss.str();

        // We have succesfully retrieved an item, so we need
        // to reset the stall-counter
        stalls = 0;
    }
    else{
    	// This is either an unknown command, a server-side timeout or the nosuccess command

    	// We only allow a given amout of timeouts / stalls
        if(maxStalls_ && (stalls++ > maxStalls_)){
        	GLogStreamer gls;
        	gls << "In GAsioTCPClient::process():" << endl
	  	  		<< "Maximum number of consecutive stalls, unknown or nosuccess commands reached: " << stalls << endl
    	   	    << "with last command = " << inboundCommand << endl
	    	  	<< "Cannot cope - leaving now." << endl
	    	  	<< logLevel(CRITICAL);

        	// Do not leave any open sockets lying around
            socket_.close();
    	    return false;
	    }

        // O.k., then let's wait some time and try again
        usleep(999999); // ~1s
    }

    // Make sure we don't leave any open sockets lying around.
    socket_.close();
    return true;
  }

  /**********************************************************************/
  /**
   * Submit processed items to the server.
   *
   * \param item String to be submitted to the server
   * \return true if operation was successful, otherwise false
   */
  bool GAsioTCPClient::submit(const string& item){
    // Let's assemble an appropriate buffer
    std::vector<boost::asio::const_buffer> buffers;
    string result = assembleQueryString("result",command_length);
    buffers.push_back(boost::asio::buffer(result));

    // Assemble the size header
    string sizeHeader = assembleQueryString(lexical_cast<string>(item.size()),command_length);
    buffers.push_back(boost::asio::buffer(sizeHeader));

    // Finally take care of the data section.
    buffers.push_back(boost::asio::buffer(item));

    // Try to connect to the server. Leave, if this was not succesful.
    if(!tryConnect()) return false;

    // And write the serialized data to the socket. We use
    // "gather-write" to send command, header and data in a single
    // write operation.
    boost::asio::write(socket_, buffers);

    // Make sure we don't leave any open sockets lying around.
    socket_.close();
    return true;
  }

} /* namespace GenEvA */
