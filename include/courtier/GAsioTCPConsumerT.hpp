/**
 * @file GAsioTCPConsumerT.hpp
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt)
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) com
 *
 * This file is part of the Geneva library collection.
 *
 * Geneva was developed with kind support from Karlsruhe Institute of
 * Technology (KIT) and Steinbuch Centre for Computing (SCC). Further
 * information about KIT and SCC can be found at http://www.kit.edu/english
 * and http://scc.kit.edu .
 *
 * Geneva is free software: you can redistribute and/or modify it under
 * the terms of version 3 of the GNU Affero General Public License
 * as published by the Free Software Foundation.
 *
 * Geneva is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with the Geneva library. If not, see <http://www.gnu.org/licenses/>.
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

#ifndef GASIOTCPCONSUMERT_HPP_
#define GASIOTCPCONSUMERT_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif


// Geneva headers go here
#include "common/GThreadGroup.hpp"
#include "common/GSerializationHelperFunctionsT.hpp"
#include "common/GHelperFunctions.hpp"
#include "courtier/GAsioHelperFunctions.hpp"
#include "courtier/GBrokerT.hpp"
#include "courtier/GCourtierEnums.hpp"
#include "courtier/GBaseConsumerT.hpp"
#include "courtier/GBaseClientT.hpp"

namespace Gem {
namespace Courtier {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * Global variables for failed transfers and connection attempts.
 */
const boost::uint32_t ASIOMAXSTALLS=10;
const boost::uint32_t ASIOMAXCONNECTIONATTEMPTS=10;
const boost::uint16_t GASIOTCPCONSUMERTHREADS = 4;

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * Forward declaration
 */
template <typename processable_type> class GAsioTCPConsumerT;

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class is responsible for the client side of network communication
 * with Boost::Asio. Note that this class is noncopyable, as it is derived
 * from a non-copyable base class.
 */
template <typename processable_type>
class GAsioTCPClientT
   : public Gem::Courtier::GBaseClientT<processable_type>
{
public:
   /***************************************************************************/
   /**
    * Initialization by server name/ip and port
    *
    * @param server Identifies the server
    * @param port Identifies the port on the server
    */
   GAsioTCPClientT(const std::string& server, const std::string& port)
      : GBaseClientT<processable_type>()
      , maxStalls_(ASIOMAXSTALLS)
      , maxConnectionAttempts_(ASIOMAXCONNECTIONATTEMPTS)
      , stalls_(0)
      , io_service_()
      , socket_(io_service_)
      , resolver_(io_service_)
      , query_(server, port)
      , endpoint_iterator0_(resolver_.resolve(query_))
      , end_()
   {
      tmpBuffer_ = new char[Gem::Courtier::COMMANDLENGTH];
   }

   /***************************************************************************/
   /**
    * Initialization by server name/ip, port and a model for the item to
    * be processed.
    *
    * @param server Identifies the server
    * @param port Identifies the port on the server
    */
   GAsioTCPClientT(
         const std::string& server
         , const std::string& port
         , boost::shared_ptr<processable_type> additionalDataTemplate
   )
      : GBaseClientT<processable_type>(additionalDataTemplate)
      , maxStalls_(ASIOMAXSTALLS)
      , maxConnectionAttempts_(ASIOMAXCONNECTIONATTEMPTS)
      , stalls_(0)
      , io_service_()
      , socket_(io_service_)
      , resolver_(io_service_)
      , query_(server, port)
      , endpoint_iterator0_(resolver_.resolve(query_))
      , end_()
   {
      tmpBuffer_ = new char[Gem::Courtier::COMMANDLENGTH];
   }

   /***************************************************************************/
   /**
    * The standard destructor.
    */
   virtual ~GAsioTCPClientT() {
      delete [] tmpBuffer_;
   }

   /***************************************************************************/
   /**
    * Sets the maximum number of stalled connection attempts
    *
    * @param maxStalls The maximum number of stalled connection attempts
    */
   void setMaxStalls(const boost::uint32_t& maxStalls)  {
      maxStalls_ = maxStalls;
   }

   /***************************************************************************/
   /**
    * Retrieves the maximum allowed number of stalled attempts
    *
    * @return The value of the maxStalls_ variable
    */
   boost::uint32_t getMaxStalls() const  {
      return maxStalls_;
   }

   /***************************************************************************/
   /**
    * Sets the maximum number of failed connection attempts before termination
    *
    * @param maxConnectionAttempts The maximum number of allowed failed connection attempts
    */
   void setMaxConnectionAttempts(const boost::uint32_t& maxConnectionAttempts)  {
      maxConnectionAttempts_ = maxConnectionAttempts;
   }

   /***************************************************************************/
   /**
    * Retrieves the maximum allowed number of failed connection attempts
    *
    * @return The value of the maxConnectionAttempts_ variable
    */
   boost::uint32_t getMaxConnectionAttempts() const  {
      return maxConnectionAttempts_;
   }

protected:
   /***************************************************************************/
   /**
    * Retrieve work items from the server.
    *
    * @param item Holds the string representation of the work item, if successful
    * @return true if operation should be continued, otherwise false
    */
   bool retrieve(std::string& item, std::string& serMode, std::string& portId) {
      item = "empty"; // Indicates that no item could be retrieved

      try {
         // Try to make a connection
         if(!tryConnect()) {
            std::ostringstream warning;
            warning << "In GAsioTCPClientT<processable_type>::retrieve(): Warning" << std::endl
                    << "Could not connect to server. Shutting down now." << std::endl;

            std::cerr << warning.str();

            return continueCalculation(false);
         }

         // Let the server know we want work
         boost::asio::write(socket_, boost::asio::buffer(assembleQueryString("ready", Gem::Courtier::COMMANDLENGTH)));

         // Read answer. First we care for the command sent by the server
         boost::asio::read(socket_, boost::asio::buffer(tmpBuffer_, Gem::Courtier::COMMANDLENGTH));

         // Remove all leading or trailing white spaces from the command
         std::string inboundCommandString = boost::algorithm::trim_copy(std::string(tmpBuffer_, Gem::Courtier::COMMANDLENGTH));

         // Act on the command
         if (inboundCommandString == "compute") {
            // We have likely received data. Let's find out how big it is
            boost::asio::read(socket_, boost::asio::buffer(tmpBuffer_, Gem::Courtier::COMMANDLENGTH));
            std::string inboundHeader = boost::algorithm::trim_copy(std::string(tmpBuffer_, Gem::Courtier::COMMANDLENGTH));
            std::size_t dataSize = boost::lexical_cast<std::size_t>(inboundHeader);

            // Now retrieve the serialization mode that was used
            boost::asio::read(socket_, boost::asio::buffer(tmpBuffer_, Gem::Courtier::COMMANDLENGTH));
            serMode = boost::algorithm::trim_copy(std::string(tmpBuffer_, Gem::Courtier::COMMANDLENGTH));

            // Retrieve the port id
            boost::asio::read(socket_, boost::asio::buffer(tmpBuffer_, Gem::Courtier::COMMANDLENGTH));
            portId = boost::algorithm::trim_copy(std::string(tmpBuffer_, Gem::Courtier::COMMANDLENGTH));

            // Create appropriately sized buffer
            char *inboundData = new char[dataSize];

            // Read the real data section from the stream
            boost::asio::read(socket_, boost::asio::buffer(inboundData, dataSize));

            // And make the data known to the outside world
            item = std::string(inboundData,dataSize);

            delete [] inboundData;

            // We have successfully retrieved an item, so we need
            // to reset the stall-counter
            stalls_ = 0;

            // Close sockets, return true
            return continueCalculation(true);
         }
         else { // Received no work. Try again a number of times
            // We will usually only allow a given number of timeouts / stalls
            if (maxStalls_ && (stalls_++ > maxStalls_)) {
               std::ostringstream error;
               error << "In GAsioTCPClientT<processable_type>::retrieve():" << std::endl
                    << "Maximum number of consecutive stalls reached,"
                    << std::endl << "with last command = "
                    << inboundCommandString << std::endl
                    << "Leaving now." << std::endl;

               return continueCalculation(false);
            }

            // We can continue. But let's wait a short time (0.05 seconds) first.
            usleep(50000);

            // Indicate that we want to continue
            return continueCalculation(true);
         }
      }
      // Any system error (except for those where a connection attempt failed) is considered
      // fatal and leads to the termination, by returning false.
      catch (boost::system::system_error&) {
         { // Make sure we do not hide the next error declaration (avoid a warning message)
#ifdef DEBUG
            std::ostringstream error;
            error << "In GAsioTCPClientT<processable_type>::retrieve():" << std::endl
                   << "Caught boost::system::system_error exception." << std::endl
                   << "This is likely normal and due to a server shutdown." << std::endl
                   << "Leaving now." << std::endl;
            std::cerr << error.str();
#endif /* DEBUG */
         }

         try {
            return continueCalculation(false);
         } catch (...) {
            std::ostringstream error;
            error << "In GAsioTCPClientT<processable_type>::retrieve():" << std::endl
                 << "Cannot shutdown gracefully as continueCalculation command" << std::endl
                 << "threw inside of catch statement." << std::endl;

            std::cerr << error.str();

            std::terminate();
         }
      }

      // This part of the function should never be reached. Let the audience know, then terminate.
      std::ostringstream error;
      error << "In GAsioTCPClientT<processable_type>::retrieve() :" << std::endl
           << "In a part of the function that should never have been reached!" << std::endl;
      std::cerr << error.str();

      std::terminate();

      return false; // Make the compiler happy
   }

   /***************************************************************************/
   /**
    * Submit processed items to the server.
    *
    * @param item String to be submitted to the server
    * @param portid The port id of the individual to be submitted
    * @return true if operation was successful, otherwise false
    */
   bool submit(const std::string& item, const std::string& portid) {
      // Let's assemble an appropriate buffer
      std::vector<boost::asio::const_buffer> buffers;
      std::string result = assembleQueryString("result", Gem::Courtier::COMMANDLENGTH); // The command
      buffers.push_back(boost::asio::buffer(result));

      // Assemble a buffer for the port id
      std::string portidString = assembleQueryString(portid,Gem::Courtier::COMMANDLENGTH);
      buffers.push_back(boost::asio::buffer(portidString));

      // Assemble the size header
      std::string sizeHeader = assembleQueryString(boost::lexical_cast<std::string> (item.size()), Gem::Courtier::COMMANDLENGTH);
      buffers.push_back(boost::asio::buffer(sizeHeader));

      // Finally take care of the data section.
      buffers.push_back(boost::asio::buffer(item));

      try{
         // Try to make a connection
         if(!tryConnect()){
            std::ostringstream warning;
            warning << "In GAsioTCPClientT<processable_type>::submit(): Warning" << std::endl
                  << "Could not connect to server. Shutting down now." << std::endl;

            std::cerr << warning.str();

            return continueCalculation(false);
         }

         // And write the serialized data to the socket. We use
         // "gather-write" to send different buffers in a single
         // write operation.
         boost::asio::write(socket_, buffers);

         // Make sure we don't leave any open sockets lying around.
         return continueCalculation(true);
      }
      // Any system error (except for those where a connection attempt failed) is considered
      // fatal and leads to the termination, by returning false.
      catch (boost::system::system_error&) {
         {
#ifdef DEBUG
            std::ostringstream error;
            error << "In GAsioTCPClientT<processable_type>::submit():" << std::endl
                    << "Caught boost::system::system_error exception." << std::endl
                    << "This is likely normal and due to a server shutdown." << std::endl
                    << "Leaving now." << std::endl;
            std::cerr << error.str();
#endif /* DEBUG */
         }

         try {
            return continueCalculation(false);
         } catch (...) {
            std::ostringstream error;
            error << "In GAsioTCPClientT<processable_type>::retrieve:" << std::endl
                 << "Cannot shutdown gracefully as continueCalculation command" << std::endl
                 << "threw inside of catch statement." << std::endl;

            std::cerr << error.str();

            std::terminate();
         }
      }

      // This part of the function should never be reached. Let the audience know, then terminate.
      std::ostringstream error;
      error << "In GAsioTCPClientT<processable_type>::submit() :" << std::endl
           << "In a part of the function that should never have been reached!" << std::endl;
      std::cerr << error.str();

      std::terminate();

      return false; // Make the compiler happy
   }

private:
   /***************************************************************************/
   /**
    * A small helper function that shuts down the socket and emits a return code
    *
    * @param returnCode The return code to be emitted
    * @return The return code
    */
   bool continueCalculation(const bool& returnCode){
      // Make sure we don't leave any open sockets lying around.
      socket_.close();
      return returnCode;
   }

   /***************************************************************************/
   /**
    * Tries to make a connection to the remote site.
    *
    * @return true if the connection could be established, false otherwise.
    */
   bool tryConnect(){
      // Try to make a connection, at max maxConnectionAttempts_ times
      boost::uint32_t connectionAttempt = 0;

      boost::system::error_code error;
      boost::asio::ip::tcp::resolver::iterator endpoint_iterator;

      while (maxConnectionAttempts_ ? (connectionAttempt++ < maxConnectionAttempts_) : true) {
          // Restore the start of the iteration
         endpoint_iterator=endpoint_iterator0_;
         error = boost::asio::error::host_not_found;

         while (error && endpoint_iterator != end_) {
            // Make sure we try not to re-open an already open socket
            socket_.close();
            // Make the connection attempt
            socket_.connect(*endpoint_iterator++, error);
         }

         // We were successful
         if (!error) break;

         // Unsuccessful. Sleep for 0.1 seconds, then try again
         usleep(100000);
      }

      // Still error ? Return, terminate
      if (error) return false;

      return true;
   }

   /***************************************************************************/

   boost::uint32_t maxStalls_; ///< The maximum allowed number of stalled connection attempts
   boost::uint32_t maxConnectionAttempts_; ///< The maximum allowed number of failed connection attempts

   boost::uint32_t stalls_; ///< counter for stalled connection attempts

   char *tmpBuffer_;

   boost::asio::io_service io_service_; ///< Holds the Boost::ASIO::io_service object

   boost::asio::ip::tcp::socket socket_; ///< The underlying socket

   boost::asio::ip::tcp::resolver resolver_; ///< Responsible for name resolution
   boost::asio::ip::tcp::resolver::query query_; ///< A query

   boost::asio::ip::tcp::resolver::iterator endpoint_iterator0_; ///< start of iteration
   boost::asio::ip::tcp::resolver::iterator end_; ///< end for end point iterator
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * An instance of this class is created for each new connection request
 * by the client. All the details of the data exchange between server
 * and client are implemented here. The class is declared in the same
 * file as the GAsioTCPConsumer in order to avoid cross referencing of
 * header files.
 */
template <typename processable_type>
class GAsioServerSessionT
:private boost::noncopyable
 {
 public:
   /***************************************************************************/
   /**
    * Standard constructor. Its main purpose it to initialize the underlying
    * socket. We do not initialize the various character arrays and strings,
    * as they are overwritten for each call to this class.
    *
    * @param io_service A reference to the server's io_service
    */
   GAsioServerSessionT(
         boost::asio::io_service& io_service
         , const Gem::Common::serializationMode& serMod
         , GAsioTCPConsumerT<processable_type> *master
   )
   : socket_(io_service)
   , serializationMode_(serMod)
   , master_(master)
   { /* nothing */ }

   /***************************************************************************/
   /**
    * A standard destructor. Shuts down and closes the socket. Note: Non-virtual.
    */
   ~GAsioServerSessionT(){
      try{
         // Make sure we don't leave any open sockets lying around.
         socket_.close();
      }
      // It is a severe error if an exception is thrown here.
      // Log and leave.
      catch(boost::system::system_error&){
         std::ostringstream error;
         error << "In GAsioServerSessionT<processable_type>::~GAsioServerSessionT():" << std::endl
               << "Caught boost::system::system_error exception" << std::endl;
         std::cerr << error.str();
         std::terminate();
      }
   }

   /***************************************************************************/
   /**
    * This function processes an individual request from a client.
    */
   void processRequest() {
      boost::posix_time::time_duration timeout(boost::posix_time::milliseconds(10));

      // Every client transmission starts with a command
      std::string command = getSingleCommand();

      if (command == "ready") {
         // Retrieve an item from the broker and submit it to the client.
         boost::shared_ptr<processable_type> p;
         Gem::Common::PORTIDTYPE id;

         // Retrieve an item
         while(!(GBROKER(processable_type)->get(id, p, timeout))) {
            if(master_->GBaseConsumerT<processable_type>::stopped()) break;

            continue;
         }

         // This will submit an empty item in case the stop criterion has been reached
         if (!this->submit(Gem::Common::sharedPtrToString(p, serializationMode_),
               "compute",
               boost::lexical_cast<std::string>(serializationMode_),
               boost::lexical_cast<std::string>(id)))
         {
            std::ostringstream information;
            information << "In GAsioServerSessionT<processable_type>::processRequest():" << std::endl
                  << "Could not submit item to client!" << std::endl;

            std::cout << information.str();
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

            // Complain if this is an empty item
            if(!p) {
               glogger
               << "In GAsioServerSessionT<>::processRequest(): Error!" << std::endl
               << "Received empty item when filled item was expected!" << std::endl
               << GEXCEPTION;
            }

            Gem::Common::PORTIDTYPE id = boost::lexical_cast<Gem::Common::PORTIDTYPE>(portid);

            // Return the item to the broker. The item will be discarded
            // if the requested target queue cannot be found.
            while(true) {
               if(master_->GBaseConsumerT<processable_type>::stopped()) break;

               try {
                  if((GBROKER(processable_type)->put(id, p, timeout))) { // Can this be a source of data loss ?
                     // we have done our job -- break the loop
                     break;
                  } else {
                     // try again
                     continue;
                  }
               } catch(Gem::Courtier::buffer_not_present&) { // discard the item
                  break;
               }
            }
         }
         else {
            std::ostringstream information;
            information << "GAsioServerSessionT<processable_type>::processRequest():" << std::endl
                  << "Could not retrieve item from client." << std::endl;
            std::cout << information.str();
         }
      }
      else { // Also covers the "empty" return value of getSingleCommand()
         std::ostringstream warning;
         warning << "In GAsioServerSessionT<processable_type>::processRequest: Warning!" << std::endl
               << "Received command \"" << command << "\"" << std::endl;
         std::cout << warning.str();

         this->sendSingleCommand("unknown");
      }
   }

   /***************************************************************************/
   /**
    * Returns the socket used by this object
    *
    * @return The socket used by this object
    */
   boost::asio::ip::tcp::socket& getSocket(){
      return socket_;
   }


 protected:
   /***************************************************************************/
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
         error << "In GAsioServerSessionT<processable_type>::getSingleCommand(): Warning" << std::endl
               << "Caught boost::system::system_error exception. The function" << std::endl
               << "will return the \"empty\" value." << std::endl;
         std::cerr << error.str();

         return std::string("empty");
      }

      // Remove all leading or trailing white spaces from the command.
      return boost::algorithm::trim_copy(std::string(inbound_command, Gem::Courtier::COMMANDLENGTH));
   }

   /***************************************************************************/
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
         error << "In GAsioServerSessionT<processable_type>::sendSingleCommand(): Warning" << std::endl
               << "Caught boost::system::system_error exception" << std::endl;
         std::cerr << error.str();
      }
   }

   /***************************************************************************/
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

   /***************************************************************************/
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
   GAsioServerSessionT(); ///< Intentionally left undefined
   GAsioServerSessionT(const GAsioServerSessionT<processable_type>&); ///< Intentionally left undefined
   const GAsioServerSessionT<processable_type>& operator=(const GAsioServerSessionT<processable_type>&); ///< Intentionally left undefined

   boost::asio::ip::tcp::socket socket_; ///< The underlying socket
   Gem::Common::serializationMode serializationMode_; ///< Specifies the serialization mode
   GAsioTCPConsumerT<processable_type> *master_;
 };

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * It is the main responsibility of this class to start new server session
 * for each client request.
 */
template <typename processable_type>
class GAsioTCPConsumerT
   :public Gem::Courtier::GBaseConsumerT<processable_type> // note: GBaseConsumerT<> is non-copyable
 {
 public:
   /***************************************************************************/
   /**
    * The standard constructor. Note that we have a private, undefined
    * default constructor, as we want to enforce that a port is provided
    * to this class. We do not need to specify our own address, as we
    * are listening only on a port of the local machine.
    *
    * @param port The port where the server should wait for new connections
    * @param listenerThreads The number of threads used to wait for incoming connections
    * @param sm The desired serialization mode
    */
   GAsioTCPConsumerT(
         const unsigned short& port
         , const std::size_t& listenerThreads = 0
         , const Gem::Common::serializationMode& sm = Gem::Common::SERIALIZATIONMODE_BINARY
   )
   : listenerThreads_(listenerThreads>0?listenerThreads:Gem::Common::getNHardwareThreads(GASIOTCPCONSUMERTHREADS))
   , acceptor_(io_service_)
   , serializationMode_(sm)
   {
      // Open the acceptor with the option to reuse the address (i.e. SO_REUSEADDR).
      boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), port);
      acceptor_.open(endpoint.protocol());
      acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
      acceptor_.bind(endpoint);
      acceptor_.listen();

      boost::shared_ptr<GAsioServerSessionT<processable_type> > currentSession(
            new GAsioServerSessionT<processable_type>(
                  io_service_
                  , serializationMode_
                  , this
            )
      );

      acceptor_.async_accept(
            currentSession->getSocket()
            , boost::bind(
                  &GAsioTCPConsumerT<processable_type>::handleAccept
                  , this
                  , currentSession
                  , boost::asio::placeholders::error
            )
      );
   }

   /***************************************************************************/
   /**
    * A standard destructor
    */
   virtual ~GAsioTCPConsumerT()
   { /* nothing */ }

   /***************************************************************************/
   /**
    * Starts the actual processing loops
    */
   void async_startProcessing() {
      // Create a number of threads responsible for the io_service_ objects
      gtg_.create_threads(
            boost::bind(&boost::asio::io_service::run, &io_service_)
      , listenerThreads_
      );
   }

   /***************************************************************************/
   /**
    * Make sure the consumer and the server sessions shut down gracefully
    */
   void shutdown() {
      // Set the stop criterion
      GBaseConsumerT<processable_type>::shutdown();

      // Terminate the io service
      io_service_.stop();
      // Wait for the threads in the group to exit
      gtg_.join_all();
   }

   /***************************************************************************/
   /**
    * Retrieves the serialization mode. Note that the only way to set the
    * mode is via the constructor.
    *
    * @return The current serialization mode
    */
   Gem::Common::serializationMode getSerializationMode() const  {
      return serializationMode_;
   }

   /***************************************************************************/
   /**
    * A unique identifier for a given consumer
    *
    * @return A unique identifier for a given consumer
    */
   virtual std::string getConsumerName() const {
      return std::string("GAsioTCPConsumerT");
   }

   /***************************************************************************/
   /**
    * Returns a short identifier for this consumer
    */
   virtual std::string getMnemomic() const {
      return std::string("tcpc");
   }

   /***************************************************************************/
   /**
    * Returns an indication whether full return can be expected from this
    * consumer. Since evaluation is performed remotely, we assume that this
    * is not the case.
    */
   virtual bool capableOfFullReturn() const {
      return false;
   }

 private:
   /***************************************************************************/
   /**
    * Handles a new connection request from a client.
    *
    * @param currentSession A pointer to the current session
    * @param error Possible error conditions
    */
   void handleAccept(
         boost::shared_ptr<GAsioServerSessionT<processable_type> > currentSession
         , const boost::system::error_code& error)
   {
      // Check whether an error occurred. This will likely indicate that we've been asked to stop.
      if(error) return;

      // First we make sure a new session is started asynchronously so the next request can be served
      boost::shared_ptr<GAsioServerSessionT<processable_type> > newSession(new GAsioServerSessionT<processable_type>(io_service_, serializationMode_, this));
      acceptor_.async_accept(
            newSession->getSocket()
            , boost::bind(
                  &GAsioTCPConsumerT<processable_type>::handleAccept
                  , this, newSession
                  , boost::asio::placeholders::error
            )
      );

      // Now we can run the actual session code
      currentSession->processRequest();
   }

   /***************************************************************************/
   boost::asio::io_service io_service_; 	///< ASIO's io service, responsible for event processing, absolutely needs to be _before_ acceptor so it gets initialized first.
   std::size_t listenerThreads_;  ///< The number of threads used to listen for incoming connections through io_servce::run()
   boost::asio::ip::tcp::acceptor acceptor_; ///< takes care of external connection requests
   Gem::Common::serializationMode serializationMode_; ///< Specifies the serialization mode
   Gem::Common::GThreadGroup gtg_;

   GAsioTCPConsumerT(); ///< Default constructor intentionally private and undefined
   const GAsioTCPConsumerT<processable_type>& operator=(const GAsioTCPConsumerT<processable_type>&); ///< Intentionally left undefined
 };

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Courtier */
} /* namespace Gem */

#endif /* GASIOTCPCONSUMERT_HPP_ */
