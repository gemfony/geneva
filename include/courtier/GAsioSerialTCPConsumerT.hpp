/**
 * @file GAsioSerialTCPConsumerT.hpp
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt)
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) eu
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
 * http://www.gemfony.eu .
 */

#ifndef GASIOSERIALTCPCONSUMERT_HPP
#define GASIOSERIALTCPCONSUMERT_HPP

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard headers go here
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <array>

// Boost headers go here
#include <boost/asio.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/utility.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_serialize.hpp>

// Geneva headers go here
#include "common/GStdThreadGroup.hpp"
#include "common/GThreadPool.hpp"
#include "common/GSerializationHelperFunctionsT.hpp"
#include "common/GCommonHelperFunctions.hpp"
#include "common/GCommonHelperFunctionsT.hpp"
#include "courtier/GCourtierHelperFunctions.hpp"
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
 * Forward declaration
 */
template<typename processable_type>
class GAsioSerialTCPConsumerT;

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class is responsible for the client side of network communication
 * with Boost::Asio. Note that this class is noncopyable, as it is derived
 * from a non-copyable base class. It assumes that a single data transfer
 * is done each time and that the connection is interrupted inbetween transfers.
 * This may be useful for long calculations, but may cause high load on the server
 * similar to a highly-frequented web server for short work loads or when all
 * workloads are returned at the same time.
 */
template<typename processable_type>
class GAsioSerialTCPClientT
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
	 GAsioSerialTCPClientT(
		 const std::string &server
		 , const std::string &port
	 )
		 : GBaseClientT<processable_type>()
			, m_query(server, port)
			, m_endpoint_iterator0(m_resolver.resolve(m_query))
	 {
		 m_tmpBuffer = new char[Gem::Courtier::COMMANDLENGTH];
	 }

	 /***************************************************************************/
	 /**
	  * Initialization by server name/ip, port and a model for the item to
	  * be processed.
	  *
	  * @param server Identifies the server
	  * @param port Identifies the port on the server
	  */
	 GAsioSerialTCPClientT(
		 const std::string &server
		 , const std::string &port
		 , std::shared_ptr<processable_type> additionalDataTemplate
	 )
		 : GBaseClientT<processable_type>(additionalDataTemplate)
			, m_query(server, port)
			, m_endpoint_iterator0(m_resolver.resolve(m_query))
	 {
		 m_tmpBuffer = new char[Gem::Courtier::COMMANDLENGTH];
	 }

	 /***************************************************************************/
	 /**
	  * The standard destructor.
	  */
	 virtual ~GAsioSerialTCPClientT() {
		 Gem::Common::g_array_delete(m_tmpBuffer);

		 glogger
			 << "In GAsioSerialTCPClientT<>::GAsioSerialTCPClientTlientT():" << std::endl
			 << "Recorded " << this->getTotalConnectionAttempts() << " failed connection" << std::endl
			 << "attempts during the runtime of this client" << std::endl
			 << GLOGGING;
	 }

	 /***************************************************************************/
	 /**
	  * Sets the maximum number of stalled connection attempts
	  *
	  * @param maxStalls The maximum number of stalled connection attempts
	  */
	 void setMaxStalls(const std::uint32_t &maxStalls) {
		 m_maxStalls = maxStalls;
	 }

	 /***************************************************************************/
	 /**
	  * Retrieves the maximum allowed number of stalled attempts
	  *
	  * @return The value of the m_maxStalls variable
	  */
	 std::uint32_t getMaxStalls() const {
		 return m_maxStalls;
	 }

	 /***************************************************************************/
	 /**
	  * Sets the maximum number of failed connection attempts before termination
	  *
	  * @param maxConnectionAttempts The maximum number of allowed failed connection attempts
	  */
	 void setMaxConnectionAttempts(const std::uint32_t &maxConnectionAttempts) {
		 m_maxConnectionAttempts = maxConnectionAttempts;
	 }

	 /***************************************************************************/
	 /**
	  * Retrieves the maximum allowed number of failed connection attempts
	  *
	  * @return The value of the m_maxConnectionAttempts variable
	  */
	 std::uint32_t getMaxConnectionAttempts() const {
		 return m_maxConnectionAttempts;
	 }

	 /***************************************************************************/
	 /**
	  * Retrieves the total number of failed connection attempts during program execution
	  * up the point of the call;
	  */
	 std::uint32_t getTotalConnectionAttempts() const {
		 return m_totalConnectionAttempts;
	 }

protected:
	 /***************************************************************************/
	 /**
	  * This is the main loop of the client. It will continue to call the process()
	  * function (defined by derived classes), until it returns false or a halt-condition
	  * was reached.
	  */
	 void run_() override {
		 while (!this->halt() && CLIENT_CONTINUE == this->process()) { /* nothing */ }
	 }

	 /***************************************************************************/
	 /**
	  * This function models a single processing step
	  */
	 bool process() {
		 // Store the current serialization mode
		 Gem::Common::serializationMode serMode;

		 // Get an item from the server
		 //
		 // TODO: Check if the clients drop out here
		 //
		 std::string istr, serModeStr;
		 if (!this->retrieve(istr, serModeStr)) {
			 glogger
				 << "In GAsioSerialTCPClientT<T>::process() : Warning!" << std::endl
				 << "Could not retrieve item from server. Leaving ..." << std::endl
				 << GWARNING;

			 return false;
		 }

		 // There is a possibility that we have received an unknown command
		 // or a timeout command. In this case we want to try again until retrieve()
		 // returns "false". If we return true here, the next "process" command will
		 // be executed.
		 if (istr == "empty") return true;

		 // Check the serialization mode we need to use
		 //
		 // TODO: Check if the clients drop out here
		 //
		 if (serModeStr == "") {
			 glogger
				 << "In GAsioSerialTCPClientT<T>::process() : Warning!" << std::endl
				 << "Found empty serModeStr. Leaving ..." << std::endl
				 << GWARNING;

			 return false;
		 }

		 serMode = boost::lexical_cast<Gem::Common::serializationMode>(serModeStr);

		 // unpack the data and create a new object. Note that de-serialization must
		 // generally happen through the same type that was used for serialization.
		 std::shared_ptr<processable_type> target = Gem::Common::sharedPtrFromString<processable_type>(istr, serMode);

		 // Check if we have received a valid target. Leave the function if this is not the case
		 if (!target) {
			 glogger
				 << "In GAsioSerialTCPClientT<T>::process() : Warning!" << std::endl
				 << "Received empty target." << std::endl
				 << GWARNING;

			 // This means that process() will be called again
			 return true;
		 }

		 // If we have a model for the item to be parallelized, load its data into the target
		 this->loadDataTemplate(target);

		 // This one line is all it takes to do the processing required for this object.
		 // The object has all required functions on board. GAsioSerialTCPClientT<T> does not need to understand
		 // what is being done during the processing.
		 target->process();
		 this->incrementProcessingCounter();

		 // transform target back into a string and submit to the server. The actual
		 // actions done by submit are defined by derived classes.
		 //
		 // TODO: Check if the clients drop out here
		 //
		 if (!this->submit(Gem::Common::sharedPtrToString(target, serMode))) {
			 glogger
				 << "In GAsioSerialTCPClientT<T>::process() : Warning!" << std::endl
				 << "Could not return item to server. Leaving ..." << std::endl
				 << GWARNING;

			 return false;
		 }

		 // Everything worked. Indicate that we want to continue
		 return true;
	 } // std::shared_ptr<processable_type> target will cease to exist at this point

	 /***************************************************************************/
	 /**
	  * Retrieve work items from the server.
	  *
	  * @param item Holds the string representation of the work item, if successful
	  * @return true if operation should be continued, otherwise false
	  */
	 bool retrieve(
		 std::string &item
		 , std::string &serMode
	 ) {
		 item = "empty"; // Indicates that no item could be retrieved
		 std::uint32_t idleTime = 0; // Holds information on the idle time in milliseconds, if "idle" command is received

		 try {
			 // Try to make a connection
			 if (!tryConnect()) {
				 glogger
					 << "In GAsioSerialTCPClientT<processable_type>::retrieve(): Warning" << std::endl
					 << "Could not connect to server. Shutting down now." << std::endl
					 << "NOTE: This might be simply caused by the server shutting down" << std::endl
					 << "at the end of an optimization run, so that usually this is no" << std::endl
					 << "cause for concern." << std::endl
					 << GLOGGING;

				 // Make sure we don't leave any open sockets lying around.
				 disconnect(m_socket);
				 return CLIENT_TERMINATE;
			 }

			 // Let the server know we want work
			 boost::asio::write(m_socket, boost::asio::buffer(assembleQueryString("ready", Gem::Courtier::COMMANDLENGTH)));

			 // Read answer. First we care for the command sent by the server
			 boost::asio::read(m_socket, boost::asio::buffer(m_tmpBuffer, Gem::Courtier::COMMANDLENGTH));

			 // Remove all leading or trailing white spaces from the command
			 std::string inboundCommandString = boost::algorithm::trim_copy(
				 std::string(m_tmpBuffer, Gem::Courtier::COMMANDLENGTH)
			 );

			 // Act on the command
			 if ("compute" == inboundCommandString) {
				 // We have likely received data. Let's find out how big it is
				 boost::asio::read(m_socket, boost::asio::buffer(m_tmpBuffer, Gem::Courtier::COMMANDLENGTH));
				 std::string inboundHeader = boost::algorithm::trim_copy(std::string(m_tmpBuffer, Gem::Courtier::COMMANDLENGTH));
				 std::size_t dataSize = boost::lexical_cast<std::size_t>(inboundHeader);

				 // Now retrieve the serialization mode that was used
				 boost::asio::read(m_socket, boost::asio::buffer(m_tmpBuffer, Gem::Courtier::COMMANDLENGTH));
				 serMode = boost::algorithm::trim_copy(std::string(m_tmpBuffer, Gem::Courtier::COMMANDLENGTH));

				 // Create appropriately sized buffer
				 char *inboundData = new char[dataSize];

				 // Read the real data section from the stream
				 boost::asio::read(m_socket, boost::asio::buffer(inboundData, dataSize));

				 // And make the data known to the outside world
				 item = std::string(inboundData, dataSize);

				 Gem::Common::g_array_delete(inboundData);

				 // We have successfully retrieved an item, so we need
				 // to reset the stall-counter
				 m_stalls = 0;

				 // Make sure we don't leave any open sockets lying around.
				 disconnect(m_socket);
				 // Indicate that we want to continue
				 return CLIENT_CONTINUE;
			 } else if (this->parseIdleCommand(idleTime, inboundCommandString)) { // Received no work. We have been instructed to wait for a certain time
				 // We might want to allow a given number of timeouts / stalls
				 if ((m_maxStalls != 0) && (m_stalls++ > m_maxStalls)) {
					 glogger
						 << "In GAsioSerialTCPClientT<processable_type>::retrieve(): Warning!" << std::endl
						 << "Maximum number of consecutive idle commands (" << m_maxStalls << ")" << std::endl
						 << "has been reached. Leaving now." << std::endl
						 << GWARNING;

					 // Make sure we don't leave any open sockets lying around.
					 disconnect(m_socket);
					 // Indicate that we don't want to continue
					 return CLIENT_TERMINATE;
				 }

				 // Make sure we don't leave any open sockets lying around.
				 disconnect(m_socket);

				 // We can continue. But let's wait for a time specified by the server in its idle-command
				 std::this_thread::sleep_for(std::chrono::milliseconds(idleTime));

				 // Indicate that we want to continue
				 return CLIENT_CONTINUE;
			 } else { // Received unknown command
				 glogger
					 << "In GAsioSerialTCPClientT<processable_type>::retrieve(): Warning!" << std::endl
					 << "Received unknown command " << inboundCommandString << std::endl
					 << "Leaving now." << std::endl
					 << GWARNING;

				 // Make sure we don't leave any open sockets lying around.
				 disconnect(m_socket);
				 // Indicate that we don't want to continue
				 return CLIENT_TERMINATE;
			 }
		 }
			 // Any system error (except for those where a connection attempt failed) is considered
			 // fatal and leads to the termination, by returning false.
		 catch (boost::system::system_error &e) {
			 {	// Make sure we do not hide the next error declaration (avoid a warning message)
				 glogger
					 << "In GAsioSerialTCPClientT<processable_type>::retrieve():" << std::endl
					 << "Caught boost::system::system_error exception" << std::endl
					 << "with message" << std::endl
					 << e.what() << std::endl
					 << "This is likely normal and due to a server shutdown." << std::endl
					 << "Leaving now." << std::endl
					 << GWARNING;
			 }

			 // Make sure we don't leave any open sockets lying around.
			 disconnect(m_socket);

			 // Indicate that we don't want to continue
			 return CLIENT_TERMINATE;
		 } catch (std::exception& e) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG,  time_and_place)
					 << "In GAsioSerialTCPClientT<processable_type>::retrieve():" << std::endl
					 << "Caught std::exception with message" << std::endl
					 << e.what() << std::endl
			 );
		 } catch (...) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG,  time_and_place)
					 << "In sharedPtrFromString(): Error!" << std::endl
					 << "Caught unknown exception" << std::endl
			 );
		 }

		 // This part of the function should never be reached. Let the audience know, then terminate.
		 throw gemfony_exception(
			 g_error_streamer(DO_LOG,  time_and_place)
				 << "In GAsioSerialTCPClientT<processable_type>::retrieve(): Error!" << std::endl
				 << "We are in a part of the function that should never have been reached!" << std::endl
		 );

		 return CLIENT_TERMINATE; // Make the compiler happy
	 }

	 /***************************************************************************/
	 /**
	  * Submit processed items to the server.
	  *
	  * @param item String to be submitted to the server
	  * @return true if operation was successful, otherwise false
	  */
	 bool submit(const std::string &item) {
		 // Let's assemble an appropriate buffer
		 std::vector<boost::asio::const_buffer> buffers;
		 std::string result = assembleQueryString("result", Gem::Courtier::COMMANDLENGTH); // The command
		 buffers.push_back(boost::asio::buffer(result));

		 // Assemble the size header
		 std::string sizeHeader = assembleQueryString(
			 Gem::Common::to_string(item.size())
			 , Gem::Courtier::COMMANDLENGTH
		 );
		 buffers.push_back(boost::asio::buffer(sizeHeader));

		 // Finally take care of the data section.
		 buffers.push_back(boost::asio::buffer(item));

		 try {
			 // Try to make a connection
			 if (!tryConnect()) {
				 glogger
					 << "In GAsioSerialTCPClientT<processable_type>::submit(): Warning" << std::endl
					 << "Could not connect to server. Shutting down now." << std::endl
					 << GWARNING;

				 // Make sure we don't leave any open sockets lying around.
				 disconnect(m_socket);

				 // Indicate that we don't want to continue
				 return CLIENT_TERMINATE;
			 }

			 // And write the serialized data to the socket. We use
			 // "gather-write" to send different buffers in a single
			 // write operation.
			 boost::asio::write(m_socket, buffers);

			 // Make sure we don't leave any open sockets lying around.
			 disconnect(m_socket);

			 // Indicate that we want to continue
			 return CLIENT_CONTINUE;
		 }
			 // Any system error (except for those where a connection attempt failed) is considered
			 // fatal and leads to the termination, by returning false.
		 catch (boost::system::system_error &) {
			 {
				 glogger
					 << "In GAsioSerialTCPClientT<processable_type>::submit():" << std::endl
					 << "Caught boost::system::system_error exception." << std::endl
					 << "This is likely normal and due to a server shutdown." << std::endl
					 << "Leaving now." << std::endl
					 << GLOGGING;
			 }

			 // Make sure we don't leave any open sockets lying around.
			 disconnect(m_socket);

			 // Indicate that we don't want to continue
			 return CLIENT_TERMINATE;
		 } catch (...) {
			 glogger
				 << "In GAsioSerialTCPClientT<processable_type>::submit():" << std::endl
				 << "Caught unknown error. Leaving now." << std::endl
				 << GLOGGING;

			 // Make sure we don't leave any open sockets lying around.
			 disconnect(m_socket);

			 // Indicate that we don't want to continue
			 return CLIENT_TERMINATE;
		 }

		 // This part of the function should never be reached. Let the audience know, then terminate.
		 glogger
			 << "In GAsioSerialTCPClientT<processable_type>::submit() :" << std::endl
			 << "In a part of the function that should never have been reached!" << std::endl
			 << GLOGGING;

		 return CLIENT_TERMINATE; // Make the compiler happy
	 }

private:
	 /***************************************************************************/
	 /**
	  * Tries to make a connection to the remote site. If a maximum number of
	  * connection attempts has been set, the function will increase the waiting
	  * time by a factor of 2 each time a connection could not be established, starting with
	  * 10 milliseconds. Thus, with a maximum of 10 connection attempts, the maximum
	  * sleep time would be about 10 seconds. This is the recommended mode of operation.
	  * If no maximum amount of connection attempts has been set, the function will
	  * sleep for 10 milliseconds every time a connection could not be established.
	  *
	  * @return true if the connection could be established, false otherwise.
	  */
	 bool tryConnect() {
		 // Try to make a connection, at max m_max_connection_attempts times
		 long milliSecondsWait = 10;

		 std::uint32_t connectionAttempt = 0;

		 boost::system::error_code error;
		 boost::asio::ip::tcp::resolver::iterator endpoint_iterator;

		 while (m_maxConnectionAttempts ? (connectionAttempt++ < m_maxConnectionAttempts) : true) {
			 // Restore the start of the iteration
			 endpoint_iterator = m_endpoint_iterator0;
			 error = boost::asio::error::host_not_found;

			 while (error && endpoint_iterator != m_end) {
				 // Make sure we do not try to re-open an already open socket
				 disconnect(m_socket);
				 // Make the connection attempt
				 m_socket.connect(*endpoint_iterator++, error);

				 if (error) {
					 m_totalConnectionAttempts++;
				 }
			 }

			 // We were successful
			 if (!error) break;

			 // Unsuccessful. Sleep for 0.01 seconds, then try again
			 std::this_thread::sleep_for(std::chrono::milliseconds(milliSecondsWait));

			 if (m_maxConnectionAttempts > 0) {
				 milliSecondsWait *= 2;
			 }
		 }

		 // Still error ? Return, terminate
		 if (error) return false;

		 return true;
	 }

	 /***************************************************************************/
	 // Data

	 std::uint32_t m_maxStalls = GASIOTCPCONSUMERMAXSTALLS; ///< The maximum allowed number of stalled connection attempts
	 std::uint32_t m_maxConnectionAttempts = GASIOTCPCONSUMERMAXCONNECTIONATTEMPTS; ///< The maximum allowed number of failed connection attempts
	 std::uint32_t m_totalConnectionAttempts = 0; ///< The total number of failed connection attempts during program execution

	 std::uint32_t m_stalls = 0; ///< counter for stalled connection attempts

	 char *m_tmpBuffer;

	 boost::asio::io_service m_io_service; ///< Holds the Boost::ASIO::io_service object
	 boost::asio::ip::tcp::socket m_socket{m_io_service}; ///< The underlying socket
	 boost::asio::ip::tcp::resolver m_resolver{m_io_service}; ///< Responsible for name resolution
	 boost::asio::ip::tcp::resolver::query m_query; ///< A query

	 boost::asio::ip::tcp::resolver::iterator m_endpoint_iterator0; ///< start of iteration
	 boost::asio::ip::tcp::resolver::iterator m_end; ///< end for end point iterator


	 /***************************************************************************/

	 // Prevent default construction
	 GAsioSerialTCPClientT() = delete;

	 // Prevent copy construction and assignment
	 GAsioSerialTCPClientT(const GAsioSerialTCPClientT<processable_type>&) = delete;
	 GAsioSerialTCPClientT(const GAsioSerialTCPClientT<processable_type>&&) = delete;
	 GAsioSerialTCPClientT<processable_type>& operator=(const GAsioSerialTCPClientT<processable_type>&) = delete;
	 GAsioSerialTCPClientT<processable_type>& operator=(const GAsioSerialTCPClientT<processable_type>&&) = delete;
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * An instance of this class is created for each new connection initiated
 * by the client. All the details of the data exchange between server
 * and client are implemented here. The class is declared in the same
 * file as the GAsioTCPConsumer in order to avoid cross referencing of
 * header files. It assumes that a single data transfer is done each time and
 * that the connection is interrupted inbetween transfers. This may be useful
 * for long calculations, but may cause high load on the server similar to
 * a highly-frequented web server for short work loads or when all workloads
 * are returned at the same time.
 */
template<typename processable_type>
class GAsioSerialServerSessionT
	: public std::enable_shared_from_this<GAsioSerialServerSessionT<processable_type>>
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
	 GAsioSerialServerSessionT(
		 boost::asio::io_service &io_service
		 , const Gem::Common::serializationMode &serMod
		 , GAsioSerialTCPConsumerT<processable_type> *master
	 )
		 : m_strand(io_service)
			, m_socket(io_service)
			, m_bytesTransferredDataBody(0)
			, m_dataBody_ptr(new std::string()) // An empty string
			, m_serializationMode(serMod)
			, m_master(master)
			, m_broker_ptr(master->m_broker_ptr)
	 { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * A standard destructor. Shuts down and closes the socket. Note: Non-virtual.
	  */
	 ~GAsioSerialServerSessionT()
	 { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * This function processes an individual request from a client.
	  */
	 void async_processRequest() {
		 try {
			 // Initiate the first read session. Every transmission starts with a command
			 boost::asio::async_read(
				 m_socket
				 , boost::asio::buffer(m_commandBuffer)
				 , m_strand.wrap(
					 std::bind(
						 &GAsioSerialServerSessionT<processable_type>::async_handle_read_command
						 , GAsioSerialServerSessionT<processable_type>::shared_from_this()
						 , std::placeholders::_1 // Replaces boost::asio::placeholders::error
					 )
				 )
			 );
		 } catch (const boost::system::system_error &e) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG,  time_and_place)
					 << "In GAsioSerialServerSessionT::async_processRequest():" << std::endl
					 << "Caught boost::system::system_error exception with messages:" << std::endl
					 << e.what() << std::endl
			 );
		 } catch (const boost::exception &e) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG,  time_and_place)
					 << "In GAsioSerialServerSessionT::async_processRequest():" << std::endl
					 << "Caught boost::exception exception with messages:" << std::endl
					 << boost::diagnostic_information(e) << std::endl
			 );
		 } catch (...) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG,  time_and_place)
					 << "In GAsioSerialServerSessionT::async_processRequest():" << std::endl
					 << "Caught unknown exception" << std::endl
			 );
		 }
	 }

	 /***************************************************************************/
	 /**
	  * Returns the socket used by this object
	  *
	  * @return The socket used by this object
	  */
	 boost::asio::ip::tcp::socket &getSocket() {
		 return m_socket;
	 }

protected:
	 /***************************************************************************/
	 /**
	  * Initiates all required action upon receiving a command
	  */
	 void async_handle_read_command(const boost::system::error_code &error) {
		 if (error) {
			 glogger
				 << "In GAsioSerialServerSessionT<processable_type>::async_handle_read_command(): Error!" << std::endl
				 << "Received boost::system::error_code " << error << std::endl
				 << "with message \"" << error.message() << "\"" << std::endl
				 << "Terminating this session."
				 << GWARNING;
			 return;
		 }

		 //------------------------------------------------------------------------
		 // Deal with the data itself

		 // Remove white spaces
		 std::string command
			 = boost::algorithm::trim_copy(
				 std::string(m_commandBuffer.data(), Gem::Courtier::COMMANDLENGTH)
			 );

		 //------------------------------------------------------------------------
		 // Initiate the next session, depending on the command

		 if ("ready" == command) {
			 this->async_submitToRemote();   // Submit our data to the client
		 } else if ("result" == command) {
			 this->async_retrieveFromRemote(); // Initiate the retrieval sequence
		 } else {
			 glogger
				 << "In GAsioSerialServerSessionT<processable_type>::async_handle_read_command(): Warning!" << std::endl
				 << "Received unknown command \"" << command << "\"" << std::endl
				 << GWARNING;

			 this->async_sendSingleCommand("unknown");
		 }

		 //------------------------------------------------------------------------
	 }

	 /***************************************************************************/
	 /**
	  * Retrieves an item from the client through the socket. This function
	  * simply initiates a chain of asynchronous commands, dealing in sequence
	  * with the data size header and the actual data body
	  */
	 void async_retrieveFromRemote() {
		 try {
			 // Initiate the next read session. Note that the requested data is returned
			 // in its entirety, not in chunks.
			 boost::asio::async_read(
				 m_socket
				 , boost::asio::buffer(m_commandBuffer)
				 , m_strand.wrap(std::bind(
					 &GAsioSerialServerSessionT<processable_type>::async_handle_read_datasize,
					 GAsioSerialServerSessionT<processable_type>::shared_from_this(),
					 std::placeholders::_1 // Replaces boost::asio::placeholders::error
				 ))
			 );
		 } catch (const boost::system::system_error &e) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG,  time_and_place)
					 << "In GAsioSerialServerSessionT::async_retrieveFromRemote():" << std::endl
					 << "Caught boost::system::system_error exception with messages:" << std::endl
					 << e.what() << std::endl
			 );
		 } catch (const boost::exception &e) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG,  time_and_place)
					 << "In GAsioSerialServerSessionT::async_retrieveFromRemote():" << std::endl
					 << "Caught boost::exception exception with messages:" << std::endl
					 << boost::diagnostic_information(e) << std::endl
			 );
		 } catch (...) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG,  time_and_place)
					 << "In GAsioSerialServerSessionT::async_retrieveFromRemote():" << std::endl
					 << "Caught unknown exception" << std::endl
			 );
		 }
	 }

	 /***************************************************************************/
	 /**
	  * A routine to be called when all data of the data size header has been read
	  */
	 void async_handle_read_datasize(const boost::system::error_code &error) {
		 if (error) {
			 glogger
				 << "In GAsioSerialServerSessionT<processable_type>::async_handle_read_datasize(): Warning!" << std::endl
				 << "Warning: Received error " << error << std::endl
				 << GWARNING;
			 return;
		 }

		 //------------------------------------------------------------------------
		 // Deal with the data itself

		 // Remove white spaces
		 std::string dataSize_str
			 = boost::algorithm::trim_copy(
				 std::string(m_commandBuffer.data(), Gem::Courtier::COMMANDLENGTH)
			 );

		 m_dataSize = boost::lexical_cast<std::size_t>(dataSize_str);

		 //------------------------------------------------------------------------
		 // Initiate the next read session, this time dealing with the data body
		 try {
			 m_socket.async_read_some(
				 boost::asio::buffer(m_dataBuffer)
				 , m_strand.wrap(std::bind(
					 &GAsioSerialServerSessionT<processable_type>::async_handle_read_body
					 , GAsioSerialServerSessionT<processable_type>::shared_from_this()
					 , std::placeholders::_1 // Replaces boost::asio::placeholders::error
					 , std::placeholders::_2 // Replaces boost::asio::placeholders::bytes_transferred
				 ))
			 );
		 } catch (const boost::system::system_error &e) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG,  time_and_place)
					 << "In GAsioSerialServerSessionT::async_handle_read_datasize():" << std::endl
					 << "Caught boost::system::system_error exception with messages:" << std::endl
					 << e.what() << std::endl
			 );
		 } catch (const boost::exception &e) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG,  time_and_place)
					 << "In GAsioSerialServerSessionT::async_handle_read_datasize():" << std::endl
					 << "Caught boost::exception exception with messages:" << std::endl
					 << boost::diagnostic_information(e) << std::endl
			 );
		 } catch (...) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG,  time_and_place)
					 << "In GAsioSerialServerSessionT::async_handle_read_datasize():" << std::endl
					 << "Caught unknown exception" << std::endl
			 );
		 }

		 //------------------------------------------------------------------------
	 }

	 /***************************************************************************/
	 /**
	  * A routine to be called whenever data snippets from the body section have
	  * been read
	  */
	 void async_handle_read_body(
		 const boost::system::error_code &error, std::size_t bytes_transferred
	 ) {
		 if (error) {
			 glogger
				 << "In GAsioSerialServerSessionT<processable_type>::async_handle_read_body(): Warning!" << std::endl
				 << "Warning: Received error " << error << std::endl
				 << GWARNING;
			 return;
		 }

		 //------------------------------------------------------------------------
		 // Extract the data and update counters

		 // Add the data to the result string. Note that m_dataBody_ptr is a shared_ptr,
		 // so we can easily hand it to the consumer, when complete.
		 *m_dataBody_ptr += std::string(m_dataBuffer.data(), bytes_transferred);
		 // Update our counter
		 m_bytesTransferredDataBody += bytes_transferred;

		 //------------------------------------------------------------------------
		 // Initiate the next read session, if still required

		 if (m_bytesTransferredDataBody < m_dataSize) {
			 try {
				 m_socket.async_read_some(
					 boost::asio::buffer(m_dataBuffer)
					 , m_strand.wrap(std::bind(
						 &GAsioSerialServerSessionT<processable_type>::async_handle_read_body
						 , GAsioSerialServerSessionT<processable_type>::shared_from_this()
						 , std::placeholders::_1 // Replaces boost::asio::placeholders::error
						 , std::placeholders::_2 // Replaces boost::asio::placeholders::bytes_transferred
										  )
					 )
				 );
			 } catch (const boost::system::system_error &e) {
				 throw gemfony_exception(
					 g_error_streamer(DO_LOG,  time_and_place)
						 << "In GAsioSerialServerSessionT::async_handle_read_body():" << std::endl
						 << "Caught boost::system::system_error exception with messages:" << std::endl
						 << e.what() << std::endl
				 );
			 } catch (const boost::exception &e) {
				 throw gemfony_exception(
					 g_error_streamer(DO_LOG,  time_and_place)
						 << "In GAsioSerialServerSessionT::async_handle_read_body():" << std::endl
						 << "Caught boost::exception exception with messages:" << std::endl
						 << boost::diagnostic_information(e) << std::endl
				 );
			 } catch (...) {
				 throw gemfony_exception(
					 g_error_streamer(DO_LOG,  time_and_place)
						 << "In GAsioSerialServerSessionT::async_handle_read_body():" << std::endl
						 << "Caught unknown exception" << std::endl
				 );
			 }
		 } else { // The transfer is complete, work with the results
			 // Disconnect the socket
			 disconnect(m_socket);

			 // ... and schedule the work item for de-serialization
			 m_master->async_scheduleDeSerialization(m_dataBody_ptr);
		 }

		 //------------------------------------------------------------------------
	 }

	 /***************************************************************************/
	 /**
	  * Submit an item to the client (i.e. the socket).
	  *
	  * @param item An item to be written to the socket
	  */
	 void async_submitToRemote() {
		 // Retrieve an item from the broker and submit it to the client.
		 std::shared_ptr <processable_type> p;

		 // Do not do anything if the server was stopped
		 if (m_master->stopped()) {
			 // Disconnect the socket
			 disconnect(m_socket);
			 return;
		 }

		 // Retrieve an item. Terminate the submission if
		 // no item could be retrieved
		 std::size_t nRetries = 0;

		 while (!m_broker_ptr->get(p, m_timeout)) {
			 if (++nRetries > m_brokerRetrieveMaxRetries) {
				 std::string idleCommand
					 = std::string("idle(") + Gem::Common::to_string(m_noDataClientSleepMilliSeconds) +
						std::string(")");
				 this->async_sendSingleCommand(idleCommand);
				 return;
			 }
		 }

		 // Retrieve a string representation of the data item
		 // TODO: processable_type should already hold the serialization code
		 std::string item = Gem::Common::sharedPtrToString(p, m_serializationMode);

		 // Format the command
		 std::string outbound_command_header = assembleQueryString(
			 "compute", Gem::Courtier::COMMANDLENGTH
		 );

		 // Format the size header
		 std::string outbound_size_header
			 = assembleQueryString(Gem::Common::to_string(item.size()), Gem::Courtier::COMMANDLENGTH);

		 // Format a header for the serialization mode
		 std::string serialization_header = assembleQueryString(
			 Gem::Common::to_string(m_serializationMode), Gem::Courtier::COMMANDLENGTH
		 );

		 // Assemble the data buffers
		 std::vector<boost::asio::const_buffer> buffers;

		 buffers.push_back(boost::asio::buffer(outbound_command_header));
		 buffers.push_back(boost::asio::buffer(outbound_size_header));
		 buffers.push_back(boost::asio::buffer(serialization_header));
		 buffers.push_back(boost::asio::buffer(item));

		 // Write the serialized data to the socket. We use "gather-write" to send
		 // command, header and data in a single write operation.
		 try {
			 boost::asio::async_write(
				 m_socket
				 , buffers
				 , m_strand.wrap(std::bind(
					 &GAsioSerialServerSessionT<processable_type>::handle_write,
					 GAsioSerialServerSessionT<processable_type>::shared_from_this(),
					 std::placeholders::_1 // Replaces boost::asio::placeholders::error
				 ))
			 );
		 } catch (const boost::system::system_error &e) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG,  time_and_place)
					 << "In GAsioSerialServerSessionT::async_submitToRemote():" << std::endl
					 << "Caught boost::system::system_error exception with messages:" << std::endl
					 << e.what() << std::endl
			 );
		 } catch (const boost::exception &e) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG,  time_and_place)
					 << "In GAsioSerialServerSessionT::async_submitToRemote():" << std::endl
					 << "Caught boost::exception exception with messages:" << std::endl
					 << boost::diagnostic_information(e) << std::endl
			 );
		 } catch (...) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG,  time_and_place)
					 << "In GAsioSerialServerSessionT::async_submitToRemote():" << std::endl
					 << "Caught unknown exception" << std::endl
			 );
		 }
	 }

	 /***************************************************************************/
	 /**
	  * Write a single command to a socket.
	  *
	  * @param command A command to be written to a socket
	  */
	 void async_sendSingleCommand(const std::string &command) {
		 // Format the command ...
		 std::string outbound_command = assembleQueryString(command, Gem::Courtier::COMMANDLENGTH);
		 // ... and tell the client
		 try {
			 boost::asio::async_write(
				 m_socket, boost::asio::buffer(outbound_command), m_strand.wrap(std::bind(
					 &GAsioSerialServerSessionT<processable_type>::handle_write,
					 GAsioSerialServerSessionT<processable_type>::shared_from_this(),
					 std::placeholders::_1 // Replaces boost::asio::placeholders::error
				 ))
			 );
		 } catch (const boost::system::system_error &e) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG,  time_and_place)
					 << "In GAsioSerialServerSessionT::async_sendSingleCommand():" << std::endl
					 << "Caught boost::system::system_error exception with messages:" << std::endl
					 << e.what() << std::endl
			 );
		 } catch (const boost::exception &e) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG,  time_and_place)
					 << "In GAsioSerialServerSessionT::async_sendSingleCommand():" << std::endl
					 << "Caught boost::exception exception with messages:" << std::endl
					 << boost::diagnostic_information(e) << std::endl
			 );
		 } catch (...) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG,  time_and_place)
					 << "In GAsioSerialServerSessionT::async_sendSingleCommand():" << std::endl
					 << "Caught unknown exception" << std::endl
			 );
		 }
	 }

	 /***************************************************************************/
	 /**
	  * A routine to be called when all data has been written
	  */
	 void handle_write(const boost::system::error_code &error) {
		 if (error) {
			 glogger
				 << "In GAsioSerialServerSessionT<processable_type>::handle_write(): Warning!" << std::endl
				 << "Warning: Received error " << error << std::endl
				 << GWARNING;
			 return;
		 }

		 // Disconnect the socket
		 disconnect(m_socket);
	 }

private:
	 /***************************************************************************/
	 // Prevent copying, moving and default-construction
	 GAsioSerialServerSessionT() = delete; ///< Intentionally left undefined
	 GAsioSerialServerSessionT(const GAsioSerialServerSessionT<processable_type> &) = delete; ///< Intentionally left undefined
	 GAsioSerialServerSessionT<processable_type> &operator=(const GAsioSerialServerSessionT<processable_type> &) = delete; ///< Intentionally left undefined
	 GAsioSerialServerSessionT(const GAsioSerialServerSessionT<processable_type> &&) = delete; ///< Intentionally left undefined
	 GAsioSerialServerSessionT<processable_type> &operator=(const GAsioSerialServerSessionT<processable_type> &&) = delete; ///< Intentionally left undefined

	 /***************************************************************************/

	 boost::asio::io_service::strand m_strand; ///< Ensure the connection's handlers are not called concurrently.
	 boost::asio::ip::tcp::socket m_socket; ///< The underlying socket

	 std::array<char, COMMANDLENGTH> m_commandBuffer; ///< A buffer to be used for command transfers
	 std::array<char, 16384> m_dataBuffer;    ///< A buffer holding body data

	 std::size_t m_bytesTransferredDataBody; ///< The number of bytes of the data body transferred so far
	 std::shared_ptr <std::string> m_dataBody_ptr; ///< The actual body data. Implemented as a shared_ptr so we can easily hand the data around

	 std::size_t m_dataSize = 0; ///< Holds the size of the body of data

	 Gem::Common::serializationMode m_serializationMode = Gem::Common::serializationMode::SERIALIZATIONMODE_BINARY; ///< Specifies the serialization mode
	 GAsioSerialTCPConsumerT<processable_type> *m_master;
	 std::shared_ptr<GBrokerT<processable_type>> m_broker_ptr;

	 std::chrono::duration<double> m_timeout = std::chrono::milliseconds(50); ///< A timeout for put- and get-operations

	 std::size_t m_brokerRetrieveMaxRetries = 1; ///< The maximum amount
	 std::uint32_t m_noDataClientSleepMilliSeconds = 100; ///< The amount of milliseconds the client should sleep when no data could be retrieved from the broker
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * It is the main responsibility of this class to start new server session
 * for each client request. It assumes that a single data transfer is done each time and
 * that the connection is interrupted inbetween transfers. This may be useful
 * for long calculations, but may cause high load on the server similar to
 * a highly-frequented web server for short work loads or when all workloads
 * are returned at the same time
 */
template<typename processable_type>
class GAsioSerialTCPConsumerT
	: public Gem::Courtier::GBaseConsumerT<processable_type> // note: GBaseConsumerT<> is non-copyable
{
	 friend class GAsioSerialServerSessionT<processable_type>;

public:
	 /***************************************************************************/
	 /**
	  * The default constructor
	  */
	 GAsioSerialTCPConsumerT()
	 { /* noting */ }

	 /***************************************************************************/
	 /**
	  * A constructor that accepts a number of vital parameters
	  *
	  * @param port The port where the server should wait for new connections
	  * @param listenerThreads The number of threads used to wait for incoming connections
	  * @param sm The desired serialization mode
	  */
	 GAsioSerialTCPConsumerT(
		 const unsigned short &port
		 , const std::size_t &listenerThreads = 0
		 , const Gem::Common::serializationMode &sm = Gem::Common::serializationMode::SERIALIZATIONMODE_BINARY
	 )
		 : m_listenerThreads(listenerThreads > 0 ? listenerThreads : Gem::Common::getNHardwareThreads(GASIOTCPCONSUMERTHREADS))
			, m_serializationMode(sm)
			, m_port(port)
	 { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * A standard destructor
	  */
	 virtual ~GAsioSerialTCPConsumerT()
	 { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * Allows to set the server name or ip
	  */
	 void setServer(std::string server) {
		 m_server = server;
	 }

	 /***************************************************************************/
	 /**
	  * Allows to retrieve the server name or ip
	  */
	 std::string getServer() const {
		 return m_server;
	 }

	 /***************************************************************************/
	 /**
	  * Allows to set the port the server listens on
	  */
	 void setPort(unsigned short port) {
		 m_port = port;
	 }

	 /***************************************************************************/
	 /**
	  * Allows to retrieve the port the server listens on
	  */
	 unsigned short getPort() const {
		 return m_port;
	 }

	 /***************************************************************************/
	 /**
	  * Allows to set the number of listener threads
	  */
	 void setNListenerThreads(std::size_t listenerThreads) {
		 m_listenerThreads = listenerThreads;
	 }

	 /***************************************************************************/
	 /**
	  * Allows to retrieve the number of listener threads
	  */
	 std::size_t getNListenerThreads() const {
		 return m_listenerThreads;
	 }

	 /***************************************************************************/
	 /**
	  * Allows to set the serialization mode
	  */
	 void setSerializationMode(Gem::Common::serializationMode sm) {
		 m_serializationMode = sm;
	 }

	 /***************************************************************************/
	 /**
	  * Retrieves the serialization mode.
	  *
	  * @return The current serialization mode
	  */
	 Gem::Common::serializationMode getSerializationMode() const {
		 return m_serializationMode;
	 }

	 /***************************************************************************/
	 /**
	  * Sets the maximum number of stalled connection attempts
	  *
	  * @param maxStalls The maximum number of stalled connection attempts
	  */
	 void setMaxStalls(const std::uint32_t &maxStalls) {
		 m_maxStalls = maxStalls;
	 }

	 /***************************************************************************/
	 /**
	  * Retrieves the maximum allowed number of stalled attempts
	  *
	  * @return The value of the m_maxStalls variable
	  */
	 std::uint32_t getMaxStalls() const {
		 return m_maxStalls;
	 }

	 /***************************************************************************/
	 /**
	  * Sets the maximum number of failed connection attempts before termination
	  *
	  * @param maxConnectionAttempts The maximum number of allowed failed connection attempts
	  */
	 void setMaxConnectionAttempts(const std::uint32_t &maxConnectionAttempts) {
		 m_maxConnectionAttempts = maxConnectionAttempts;
	 }

	 /***************************************************************************/
	 /**
	  * Retrieves the maximum allowed number of failed connection attempts
	  *
	  * @return The value of the m_maxConnectionAttempts variable
	  */
	 std::uint32_t getMaxConnectionAttempts() const {
		 return m_maxConnectionAttempts;
	 }

	 /***************************************************************************/
	 /**
	  * Allows to check whether this consumer needs a client to operate.
	  *
	  * @return A boolean indicating whether this consumer needs a client to operate
	  */
	 bool needsClient() const override {
		 return true;
	 }

	 /***************************************************************************/
	 /**
	  * Emits a client suitable for processing the data emitted by this consumer
	  */
	 std::shared_ptr<GBaseClientT<processable_type>> getClient() const override {
		 std::shared_ptr <GAsioSerialTCPClientT<processable_type>> p(
			 new GAsioSerialTCPClientT<processable_type>(m_server, Gem::Common::to_string(m_port))
		 );

		 p->setMaxStalls(m_maxStalls); // Set to 0 to allow an infinite number of stalls
		 p->setMaxConnectionAttempts(m_maxConnectionAttempts); // Set to 0 to allow an infinite number of failed connection attempts

		 return p;
	 }

	 /***************************************************************************/
	 /**
	  * Starts the actual processing loops
	  */
	 void async_startProcessing() override {
		 // Open the acceptor
		 boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), m_port);
		 m_acceptor.open(endpoint.protocol());
		 m_acceptor.bind(endpoint);
		 // Set the SOL_SOCKET/SO_REUSEADDR options
		 // compare http://www.boost.org/doc/libs/1_57_0/doc/html/boost_asio/reference/basic_socket_acceptor/reuse_address.html
		 boost::asio::socket_base::reuse_address option(true);
		 m_acceptor.set_option(option);
		 m_acceptor.listen();

		 // Retrieve a pointer to the global broker for later perusal
		 m_broker_ptr = GBROKER(processable_type);

		 if (!m_broker_ptr) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG,  time_and_place)
					 << "In GAsioSerialTCPConsumerT<processable_type>::async_startProcessing(): Error!" << std::endl
					 << "Got empty broker pointer" << std::endl
			 );
		 }

		 // Set the number of threads in the pool
		 if (m_listenerThreads) {
			 m_gtp.setNThreads(boost::numeric_cast<unsigned int>(m_listenerThreads));
			 std::cout << "GAsioSerialTCPConsumerT: Started acceptor with " << boost::numeric_cast<unsigned int>(m_listenerThreads) << " threads" << std::endl;
		 }

		 try {
			 // Prevent a race condition
			 m_work.reset(new boost::asio::io_service::work(m_io_service));

			 // Start the first session
			 this->async_newAccept();

			 // Create a number of threads responsible for the m_io_service objects
			 // This absolutely needs to happen after the first session has started,
			 // so the io_service doesn't run out of work
			 m_gtg.create_threads(
				 [&]() { this->m_io_service.run(); } // this-> deals with a problem of g++ 4.7.2
				 , m_listenerThreads
			 );
		 } catch (const boost::system::system_error &e) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG,  time_and_place)
					 << "In GAsioSerialTCPConsumerT::async_startProcessing():" << std::endl
					 << "Caught boost::system::system_error exception with messages:" << std::endl
					 << boost::diagnostic_information(e) << std::endl
			 );
		 } catch (const boost::exception &e) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG,  time_and_place)
					 << "In GAsioSerialTCPConsumerT::async_startProcessing():" << std::endl
					 << "Caught boost::exception with messages:" << std::endl
					 << boost::diagnostic_information(e) << std::endl
			 );
		 } catch (const std::exception &e) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG,  time_and_place)
					 << "In GAsioSerialTCPConsumerT::async_startProcessing():" << std::endl
					 << "Caught std::exception with messages:" << std::endl
					 << boost::diagnostic_information(e) << std::endl
			 );
		 } catch (...) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG,  time_and_place)
					 << "In GAsioSerialTCPConsumerT::async_startProcessing():" << std::endl
					 << "Caught unknown exception" << std::endl
			 );
		 }
	 }

	 /***************************************************************************/
	 /**
	  * Make sure the consumer and the server sessions shut down gracefully
	  */
	 void shutdown() override {
		 // Set the stop criterion
		 GBaseConsumerT<processable_type>::shutdown();

		 // Terminate the worker and clear the thread group
		 m_work.reset(); // This will initiate termination of all threads

		 // Terminate the io service
		 m_io_service.stop();
		 // Wait for the threads in the group to exit
		 m_gtg.join_all();
	 }

	 /***************************************************************************/
	 /**
	  * A unique identifier for a given consumer
	  *
	  * @return A unique identifier for a given consumer
	  */
	 std::string getConsumerName() const override {
		 return std::string("GAsioSerialTCPConsumerT");
	 }

	 /***************************************************************************/
	 /**
	  * Returns a short identifier for this consumer
	  */
	 std::string getMnemonic() const override {
		 return std::string("stcpc");
	 }

	 /***************************************************************************/
	 /**
	  * Returns an indication whether full return can be expected from this
	  * consumer. Since evaluation is performed remotely, we assume that this
	  * is not the case.
	  */
	 bool capableOfFullReturn() const override {
		 return false;
	 }

	 /***************************************************************************/
	 /**
	  * Adds local command line options to a boost::program_options::options_description object.
	  *
	  * @param visible Command line options that should always be visible
	  * @param hidden Command line options that should only be visible upon request
	  */
	 virtual void addCLOptions(
		 boost::program_options::options_description &visible, boost::program_options::options_description &hidden
	 ) override {
		 namespace po = boost::program_options;

		 visible.add_options()
			 ("stcpc_ip", po::value<std::string>(&m_server)->default_value(GASIOTCPCONSUMERDEFAULTSERVER),
				 "\t[stcpc] The name or ip of the server")
			 ("stcpc_port", po::value<unsigned short>(&m_port)->default_value(GASIOTCPCONSUMERDEFAULTPORT),
				 "\t[stcpc] The port of the server");

		 hidden.add_options()
			 ("stcpc_serializationMode", po::value<Gem::Common::serializationMode>(&m_serializationMode)->default_value(
				 GASIOTCPCONSUMERSERIALIZATIONMODE),
				 "\t[stcpc] Specifies whether serialization shall be done in TEXTMODE (0), XMLMODE (1) or BINARYMODE (2)")
			 ("stcpc_maxStalls", po::value<std::uint32_t>(&m_maxStalls)->default_value(GASIOTCPCONSUMERMAXSTALLS),
				 "\t[stcpc] The maximum allowed number of stalled connection attempts of a client. 0 means \"forever\".")
			 ("stcpc_maxConnectionAttempts",
				 po::value<std::uint32_t>(&m_maxConnectionAttempts)->default_value(GASIOTCPCONSUMERMAXCONNECTIONATTEMPTS),
				 "\t[stcpc] The maximum allowed number of failed connection attempts of a client")
			 ("stcpc_nListenerThreads", po::value<std::size_t>(&m_listenerThreads)->default_value(m_listenerThreads),
				 "\t[stcpc] The number of threads used to listen for incoming connections");
	 }

	 /***************************************************************************/
	 /**
	  * Takes a boost::program_options::variables_map object and checks for supplied options.
	  */
	 void actOnCLOptions(const boost::program_options::variables_map &vm) override
	 { /* nothing */ }

private:
	 /***************************************************************************/
	 /**
	  * Schedules the de-serialization of a completed work item, so the server
	  * session does not need to perform this work
	  */
	 void async_scheduleDeSerialization(
		 std::shared_ptr <std::string> dataBody_ptr
	 ) {
		 m_gtp.async_schedule(
			 std::function<void()>(
				 std::bind(
					 &GAsioSerialTCPConsumerT<processable_type>::handle_workItemComplete // Does its own error checks
					 , this
					 , dataBody_ptr
					 , m_serializationMode
					 , m_timeout
				 )
			 )
		 );
	 }

	 /***************************************************************************/
	 /**
	  * Handles the de-serialization of a completed work item. This function
	  * will usually be called in its own thread.
	  */
	 void handle_workItemComplete(
		 std::shared_ptr <std::string> dataBody_ptr
		 , Gem::Common::serializationMode sM
		 , std::chrono::duration<double> timeout
	 ) {
		 // De-Serialize the data
		 std::shared_ptr <processable_type> p
			 = Gem::Common::sharedPtrFromString<processable_type>(*dataBody_ptr, sM);

		 // Complain if this is an empty item
		 if (!p) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG,  time_and_place)
					 << "In GAsioSerialTCPConsumerT<>::handle_workItemComplete(): Error!" << std::endl
					 << "Received empty item when filled item was expected!" << std::endl
			 );
		 }

		 // Return the item to the broker. The item will be discarded
		 // if the requested target queue cannot be found.
		 try {
			 while (!m_broker_ptr->put(p, timeout)) {
				 if (this->stopped()) { // This may lead to a loss of items
					 glogger
						 << "GAsioSerialTCPConsumerT<>::In handle_workItemComplete(): Warning!" << std::endl
						 << "Discarding item as the consumer object stopped operation" << std::endl
						 << GWARNING;

					 return;
				 }

				 continue;
			 }
		 } catch (const Gem::Courtier::buffer_not_present &) { // discard the item
			 glogger
				 << "GAsioSerialTCPConsumerT<>::In handle_workItemComplete(): Warning!" << std::endl
				 << "Discarding item as buffer port is not present" << std::endl
				 << GWARNING;

			 return;
		 } catch (...) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG,  time_and_place)
					 << "GAsioSerialTCPConsumerT<>::In handle_workItemComplete():" << std::endl
					 << "Caught unknown exception" << std::endl
			 );
		 }
	 }

	 /***************************************************************************/
	 /**
	  * Starts a new session
	  */
	 void async_newAccept() {
		 // First we make sure a new session is started asynchronously so the next request can be served
		 std::shared_ptr <GAsioSerialServerSessionT<processable_type>> newSession(
			 new GAsioSerialServerSessionT<processable_type>(
				 m_io_service
				 , m_serializationMode
				 , this
			 )
		 );

		 try {
			 m_acceptor.async_accept(
				 newSession->getSocket()
				 , std::bind(
					 &GAsioSerialTCPConsumerT<processable_type>::async_handleAccept
					 , this
					 , newSession
					 , std::placeholders::_1 // Replaces boost::asio::placeholders::error
				 )
			 );
		 } catch (const boost::system::system_error &e) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG,  time_and_place)
					 << "In GAsioSerialTCPConsumerT::async_newAccept():" << std::endl
					 << "Caught boost::system::system_error exception with messages:" << std::endl
					 << e.what() << std::endl
			 );
		 } catch (const boost::exception &e) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG,  time_and_place)
					 << "In GAsioSerialTCPConsumerT::async_newAccept():" << std::endl
					 << "Caught boost::exception exception with messages:" << std::endl
					 << boost::diagnostic_information(e) << std::endl
			 );
		 } catch (...) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG,  time_and_place)
					 << "In GAsioSerialTCPConsumerT::async_newAccept():" << std::endl
					 << "Caught unknown exception" << std::endl
			 );
		 }
	 }


	 /***************************************************************************/
	 /**
	  * Handles a new connection request from a client.
	  *
	  * @param currentSession A pointer to the current session
	  * @param error Possible error conditions
	  */
	 void async_handleAccept(
		 std::shared_ptr <GAsioSerialServerSessionT<processable_type>> currentSession
		 , const boost::system::error_code &error
	 ) {
		 if (error) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG,  time_and_place)
					 << "In GAsioSerialTCPConsumerT<>::async_handleAccept():"
					 << "Terminating on error " << error << std::endl
			 );

			 return;
		 }

		 // Initiate waiting for new connections
		 try {
			 this->async_newAccept();

			 // Initiate the processing sequence
			 currentSession->async_processRequest();
		 } catch (const boost::system::system_error &e) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG,  time_and_place)
					 << "In GAsioSerialTCPConsumerT::async_handleAccept():" << std::endl
					 << "Caught boost::system::system_error exception with messages:" << std::endl
					 << e.what() << std::endl
			 );
		 } catch (const boost::exception &e) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG,  time_and_place)
					 << "In GAsioSerialTCPConsumerT::async_handleAccept():" << std::endl
					 << "Caught boost::exception exception with messages:" << std::endl
					 << boost::diagnostic_information(e) << std::endl
			 );
		 } catch (...) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG,  time_and_place)
					 << "In GAsioSerialTCPConsumerT::async_handleAccept():" << std::endl
					 << "Caught unknown exception" << std::endl
			 );
		 }
	 }

	 /***************************************************************************/
	 // Data

	 boost::asio::io_service m_io_service;   ///< ASIO's io service, responsible for event processing, absolutely needs to be _before_ acceptor so it gets initialized first.
	 std::shared_ptr<boost::asio::io_service::work> m_work; ///< A place holder ensuring that the io_service doesn't stop prematurely
	 std::size_t m_listenerThreads = Gem::Common::getNHardwareThreads(GASIOTCPCONSUMERTHREADS);  ///< The number of threads used to listen for incoming connections through io_servce::run()
	 boost::asio::ip::tcp::acceptor m_acceptor{m_io_service}; ///< takes care of external connection requests
	 Gem::Common::serializationMode m_serializationMode = Gem::Common::serializationMode::SERIALIZATIONMODE_BINARY; ///< Specifies the serialization mode
	 std::uint32_t m_maxStalls = GASIOTCPCONSUMERMAXSTALLS; ///< The maximum allowed number of stalled connection attempts of a client
	 std::uint32_t m_maxConnectionAttempts = GASIOTCPCONSUMERMAXCONNECTIONATTEMPTS; ///< The maximum allowed number of failed connection attempts of a client
	 unsigned short m_port = GASIOTCPCONSUMERDEFAULTPORT; ///< The port on which the server is supposed to listen
	 std::string m_server = GASIOTCPCONSUMERDEFAULTSERVER;  ///< The name or ip if the server
	 std::chrono::duration<double> m_timeout = std::chrono::milliseconds(10); ///< A timeout for put- and get-operations
	 Gem::Common::GStdThreadGroup m_gtg; ///< Holds listener threads
	 Gem::Common::GThreadPool m_gtp; ///< Holds workers sorting processed items back into the broker
	 std::shared_ptr<Gem::Courtier::GBrokerT<processable_type>> m_broker_ptr; ///< A pointer to the global broker
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Courtier */
} /* namespace Gem */

#endif /* GASIOSERIALTCPCONSUMERT_HPP */
