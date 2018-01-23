/**
 * @file GBeastConsumerT.hpp
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

#ifndef GENEVA_LIBRARY_COLLECTION_GBEASTCONSUMERT_HPP
#define GENEVA_LIBRARY_COLLECTION_GBEASTCONSUMERT_HPP

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
#include <boost/beast.hpp>
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
#include "GCourtierEnums.hpp"
#include "../common/GCommonEnums.hpp"


namespace Gem {
namespace Courtier {

/******************************************************************************************/
////////////////////////////////////////////////////////////////////////////////////////////
/******************************************************************************************/

template<
	typename processable_type
	, typename command_type
>
class GCommandContainerT {
	 ///////////////////////////////////////////////////////////////
	 friend class boost::serialization::access;

	 template<class Archive>
	 void serialize(Archive & ar, const unsigned int version) {
		 ar
		 & BOOST_SERIALIZATION_NVP(m_command)
		 & BOOST_SERIALIZATION_NVP(m_payload_ptr);
	 }
	 ///////////////////////////////////////////////////////////////

	 // Make sure processable_type adheres to the GProcessingContainerT interface
	 static_assert(
		 std::is_base_of<Gem::Courtier::GProcessingContainerT<processable_type, typename processable_type::result_type>, processable_type>::value
		 , "processable_type does not adhere to the GProcessingContainerT interface"
	 );

public:
	 //-------------------------------------------------------------------------
	 /**
	  * Initialization with a command only, in cases where no payload
	  * needs to be transported
	  *
	  * @param command The command to be executed
	  */
	 explicit GCommandContainerT(command_type command)
		 : m_command(command)
	 { /* nothing */ }

	 //-------------------------------------------------------------------------
	 /**
	  * Initialization with command and payload (in cases where a payload needs
	  * to be transferred)
	  *
	  * @param command The command to be executed
	  * @param payload_ptr The payload transported by this object
	  */
	 GCommandContainerT(
		 command_type command
		 , std::shared_ptr<processable_type> payload_ptr
	 )
		 : m_command(command)
		 , m_payload_ptr(payload_ptr)
	 { /* nothing */ }

	 //-------------------------------------------------------------------------
	 /**
	  * The move constructor
	  * @param cp Another GCommandContainerT<processable_type> object
	  */
	 GCommandContainerT(GCommandContainerT<processable_type>&& cp) noexcept {
		 m_command = cp.m_command; cp.m_command = payload_command::NONE;
		 m_payload_ptr.swap(cp.m_payload_ptr); cp.m_payload_ptr.reset();
	 }

	 //-------------------------------------------------------------------------
	 /**
	  * The destructor
	  */
	 ~GCommandContainerT() {
		 m_payload_ptr.reset();
	 }

	 //-------------------------------------------------------------------------
	 /**
	  * Move assignment
	  * @param cp Another GCommandContainerT<processable_type> object
	  * @return A reference to this object
	  */
	 GCommandContainerT<processable_type>& operator=(GCommandContainerT<processable_type>&& cp) noexcept {
		 m_command = cp.m_command; cp.m_command = payload_command::NONE;
		 m_payload_ptr.swap(cp.m_payload_ptr); cp.m_payload_ptr.reset();

		 return *this;
	 }

	 //-------------------------------------------------------------------------
	 // Deleted copy-constructors and assignment operator -- the class is non-copyable

	 GCommandContainerT<processable_type>(const GCommandContainerT<processable_type>&) = delete;
	 GCommandContainerT<processable_type>& operator=(const GCommandContainerT<processable_type>&) = delete;

	 //-------------------------------------------------------------------------
	 /**
	  * Reset to a new command and payload or clear the object
	  */
	 void reset(
		 command_type command = command_type(0);
		 , std::shared_ptr<processable_type> payload_ptr = std::shared_ptr<processable_type>()
	 ) {
		 m_command = command;
		 m_payload_ptr = payload_ptr;
	 }

	 //-------------------------------------------------------------------------
	 /**
	  * Setting of the command to be executed on the payload (possibly on the remote side)
	  * @param command The command to be executed on the payload
	  */
	 void set_command(command_type command) {
		 m_command = command;
	 }

	 //-------------------------------------------------------------------------
	 /**
	  * Retrieval of the command to be executed on the payload
	  * @return The command to be executed on the payload
	  */
	 command_type get_command() const noexcept {
		 return m_command;
	 }

	 //-------------------------------------------------------------------------
	 /**
	  * Processing of the payload (if any)
	  */
	 void process() {
		 if(m_payload_ptr) {
			 m_payload_ptr->process();
		 } else {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG,  time_and_place)
					 << "In GCommandContainerT<processable_type>::process():" << std::endl
					 << "Tried to process a work item while m_payload_ptr is empty" << std::endl
			 );
		 }
	 }

	 //-------------------------------------------------------------------------
	 /**
	  * Converts this object to a std::string
	  *
	  * @param serMode The serialization mode (binary, xml or plain text)
	  * @return A serialized version of this object
	  */
	 std::string to_string(Gem::Common::serializationMode serMode) const {
		 try {
			 switch (serMode) {
				 case Gem::Common::serializationMode::TEXT: {
					 std::stringstream(std::ios::out).swap(m_stringstream);
					 boost::archive::text_oarchive oa(m_stringstream);
					 oa << boost::serialization::make_nvp(
						 "command_container"
						 , *this
					 );
				 }
					 break; // archive and stream closed at end of scope

				 case Gem::Common::serializationMode::XML: {
					 std::stringstream(std::ios::out).swap(m_stringstream);
					 boost::archive::xml_oarchive oa(m_stringstream);
					 oa << boost::serialization::make_nvp(
						 "command_container"
						 , *this
					 );
				 }
					 break;

				 case Gem::Common::serializationMode::BINARY: {
					 std::stringstream(std::ios::out | std::ios::binary).swap(m_stringstream);
					 boost::archive::binary_oarchive oa(m_stringstream);
					 oa << boost::serialization::make_nvp(
						 "command_container"
						 , *this
					 );
				 }
					 break;
			 }
		 } catch (const boost::system::system_error &e) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG,  time_and_place)
					 << "In GCommandContainerT<processable_type>::to_string():" << std::endl
					 << "Caught boost::system::system_error exception with messages:" << std::endl
					 << e.what() << std::endl
			 );
		 } catch (const boost::exception &e) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG,  time_and_place)
					 << "In GCommandContainerT<processable_type>::to_string():" << std::endl
					 << "Caught boost::exception exception with messages:" << std::endl
					 << boost::diagnostic_information(e) << std::endl
			 );
		 } catch (...) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG,  time_and_place)
					 << "In GCommandContainerT<processable_type>::to_string():" << std::endl
					 << "Caught unknown exception" << std::endl
			 );
		 }

		 return m_stringstream.str();
	 }

	 //-------------------------------------------------------------------------
	 /**
	  * Loads a serialized version of an instantiation of this class into this object
	  */
	 void from_string(
		 const std::string& descr
		 , Gem::Common::serializationMode serMode
	 ) {
		 try {
			 GCommandContainerT<processable_type> local_command_container{command_type(0)};

			 switch(serMode) {
				 case Gem::Common::serializationMode::TEXT: {
					 std::stringstream(descr, std::ios::in).swap(m_stringstream);
					 boost::archive::text_iarchive ia(m_stringstream);
					 ia >> boost::serialization::make_nvp("command_container", local_command_container);
				 } break; // archive and stream closed at end of scope

				 case Gem::Common::serializationMode::XML: {
					 std::stringstream(descr, std::ios::in).swap(m_stringstream);
					 boost::archive::xml_iarchive ia(m_stringstream);
					 ia >> boost::serialization::make_nvp("command_container", local_command_container);
				 } break;

				 case Gem::Common::serializationMode::BINARY: {
					 std::stringstream(descr, std::ios::in | std::ios::binary).swap(m_stringstream);
					 boost::archive::binary_iarchive ia(m_stringstream);
					 ia >> boost::serialization::make_nvp("command_container", local_command_container);
				 } break;
			 }

			 *this = std::move(local_command_container);
		 } catch (const boost::system::system_error &e) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG,  time_and_place)
					 << "In GCommandContainerT<processable_type>::from_string():" << std::endl
					 << "Caught boost::system::system_error exception with messages:" << std::endl
					 << e.what() << std::endl
			 );
		 } catch (const boost::exception &e) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG,  time_and_place)
					 << "In GCommandContainerT<processable_type>::from_string():" << std::endl
					 << "Caught boost::exception exception with messages:" << std::endl
					 << boost::diagnostic_information(e) << std::endl
			 );
		 } catch (...) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG,  time_and_place)
					 << "In GCommandContainerT<processable_type>::from_string():" << std::endl
					 << "Caught unknown exception" << std::endl
			 );
		 }
	 }

private:
	 //-------------------------------------------------------------------------

	 /** Default constructor -- only needed for de-serialization, hence private */
	 GCommandContainerT() = default;

	 //-------------------------------------------------------------------------
	 // Data

	 command_type m_command{command_type(0)}; ///< The command to be exeecuted
	 std::shared_ptr<processable_type>  m_payload_ptr; ///< The actual payload, if any

	 mutable std::stringstream m_stringstream; ///< Needed to facilitate (de-)serialization

	 //-------------------------------------------------------------------------
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class is responsible for the client side of network communication
 * with Boost::Beast. Connections are kept open permanently.
 */
template<typename processable_type>
class GBeastClientT final
	: public std::enable_shared_from_this<GBeastClientT>
{
	 //-------------------------------------------------------------------------
	 // Make the code easier to read

	 using error_code = boost::system::error_code;
	 using resolver = boost::asio::ip::tcp::resolver;
	 using socket = boost::asio::ip::tcp::socket;
	 using close_code = boost::beast::websocket::close_code;
	 using frame_type = boost::beast::websocket::frame_type;
	 using string_view = boost::beast::string_view;

public:
	 //-------------------------------------------------------------------------
	 /**
	  * Initialization with host/ip and port
	  */
	 GBeastClientT(
	 	const std::string& address
		, unsigned short port
	 	, Gem::Common::serializationMode serialization_mode = Gem::Common::serializationMode::BINARY
	 )
	   : m_address(address)
	   , m_port(port)
	 	, m_serialization_mode(serialization_mode)
	 {
		 // Set the auto_fragment option, so control frames are delivered timely
		 m_ws.auto_fragment(true);
		 m_ws.write_buffer_size(16384);

		 // Set the transfer mode according to the defines in CMakeLists.txt
		 set_transfer_mode(m_ws);

		 // Set a control-frame callback
		 f_when_control_frame_arrived
			 = [this](frame_type frame_t, string_view s) {
			 if(boost::beast::websocket::frame_type::close==frame_t) {
				 std::cout << "Client has received a close frame" << std::endl;
			 }
		 };

		 // Set the callback to be executed on every incoming control frame.
		 m_ws.control_callback(f_when_control_frame_arrived);
	 }

	 //-------------------------------------------------------------------------
	 /** @brief The destructor */
	 ~GBeastClientT() = default;

	 //-------------------------------------------------------------------------
	 // Deleted functions

	 // Deleted default-constructor -- enforce usage of a particular constructor
	 GBeastClientT() = delete;
	 // Deleted copy-constructors and assignment operators -- the client is non-copyable
	 GBeastClientT(const GBeastClientT<processable_type>&) = delete;
	 GBeastClientT(GBeastClientT<processable_type>&&) = delete;
	 GBeastClientT<processable_type>& operator=(const GBeastClientT<processable_type>&) = delete;
	 GBeastClientT<processable_type>& operator=(GBeastClientT<processable_type>&&) = delete;

private:
	 //-------------------------------------------------------------------------
	 /**
	  * Starts the main async_start_run-loop
	  */
	 void run_() override {
		 // Start looking up the domain name. This call will return immediately,
		 // when_resolved() will be called once the operation is complete.
		 auto self = shared_from_this();
		 m_resolver.async_resolve(
			 m_address
			 , std::to_string(m_port)
			 , [self](
				 boost::system::error_code ec
				 , const resolver::results_type &results
			 ) {
				 self->when_resolved(ec, results);
			 }
		 );

		 // This call will block until no more work remains in the ASIO work queue
		 m_io_context.run();

		 // Finally close all outstanding connections and let the audience know
		 glogger
		 	<< "GBeastClientT<processable_type>::async_start_run(): Closing down remaining connections" << std::endl
		   << GLOGGING;
		 do_close(m_close_code);
	 }

	 //-------------------------------------------------------------------------
	 /**
	  * Starts a new write session
	  *
	  * @param message The message to be transferred to the peer
	  */
	 void async_start_write(const std::string& message) {
		 // We need to persist the message for asynchronous operations.
		 // It is hence stored in a class variable.
		 m_outgoing_message = message;

		 // Send the message
		 auto self = shared_from_this();
		 m_ws.async_write(
			 boost::asio::buffer(m_outgoing_message)
			 , [self](
				 boost::system::error_code ec
				 , std::size_t nBytesTransferred
			 ) {
				 self->when_written(ec, nBytesTransferred);
			 }
		 );
	 }

	 //-------------------------------------------------------------------------
	 /**
	  * Starts a new read session
	  */
	 void async_start_read() {
		 auto self = shared_from_this();
		 m_ws.async_read(
			 m_incoming_buffer
			 , [self] (
				 boost::system::error_code ec
				 , std::size_t nBytesTransferred
			 ) {
				 self->when_read(ec, nBytesTransferred);
			 }
		 );
	 }

	 //-------------------------------------------------------------------------
	 /**
	  * Code to be executed when the async_resolve-operation has completed
	  *
	  * @param ec Indicates a possible error
	  * @param results The results of the resolve-operation
	  */
	 void when_resolved(
		 boost::system::error_code ec
		 , const resolver::results_type &results
	 ) {
		 if(ec) {
			 glogger
				 << "In GBeastClientT<processable_type>::when_resolved():" << std::endl
				 << "Got ec(\"" << ec.message() << "\"). async_connect() will not be executed." << std::endl
				 << "This will terminate the client." << std::endl
				 << GLOGGING;

			 // Give the audience a hint why we are terminating
			 m_close_code = boost::beast::websocket::close_code::going_away;

			 return;
		 }

		 // Make the connection on the endpoint we get from a lookup
		 auto self = shared_from_this();
		 boost::asio::async_connect(
			 m_ws.next_layer()
			 , results.begin()
			 , results.end()
			 , [self](boost::system::error_code ec, boost::asio::ip::tcp::resolver::iterator /* unused */) {
				 self->when_connected(ec);
			 }
		 );
	 }

	 //-------------------------------------------------------------------------
	 /**
	  * Code to be executed when the async_connect call has completed
	  *
	  * @param ec Indicates a possible error
	  */
	 void when_connected(boost::system::error_code ec) {
		 if(ec) {
			 glogger
				 << "In GBeastClientT<processable_type>::when_connected():" << std::endl
				 << "Got ec(\"" << ec.message() << "\"). async_handshake() will not be executed." << std::endl
				 << "This will terminate the client." << std::endl
				 << GLOGGING;

			 // Give the audience a hint why we are terminating
			 m_close_code = boost::beast::websocket::close_code::going_away;

			 return;
		 }

		 // Perform the handshake
		 auto self = shared_from_this();
		 m_ws.async_handshake(
			 m_address
			 , "/"
			 , [self](boost::system::error_code ec) {
				 self->when_handshake_complete(ec);
			 }
		 );
	 }

	 //-------------------------------------------------------------------------
	 /**
	  * Code to be executed when the async_handshake-operation has completed
	  *
	  * @param ec Indicates a possible error
	  */

	 void when_handshake_complete(boost::system::error_code ec) {
		 if(ec) {
			 glogger
				 << "In GBeastClientT<processable_type>::when_handshake_complete():" << std::endl
				 << "Got ec(\"" << ec.message() << "\"). async_start_write() will not be executed." << std::endl
				 << "This will terminate the client." << std::endl
				 << GLOGGING;

			 // Give the audience a hint why we are terminating
			 m_close_code = boost::beast::websocket::close_code::going_away;

			 // This will terminate the client
			 return;
		 }

		 // Send the first command to the server
		 m_command_container.reset(payload_command::GETDATA);
		 async_start_write(m_command_container.to_string());
	 }

	 //-------------------------------------------------------------------------
	 /**
	  * Code to be executed when the entire message was sent to the remote side
	  *
	  * @param ec Indicates a possible error
	  */
	 void when_written(
		 boost::system::error_code ec
		 , std::size_t /* nothing */
	 ) {
		 if(ec) {
			 glogger
				 << "In GBeastClientT<processable_type>::when_written():" << std::endl
				 << "Got ec(\"" << ec.message() << "\"). async_start_read() will not be executed." << std::endl
				 << "This will terminate the client." << std::endl
				 << GLOGGING;

			 // Give the audience a hint why we are terminating
			 m_close_code = boost::beast::websocket::close_code::going_away;

			 // This will terminate the client
			 return;
		 }

		 // Clear the outgoing message -- no longer needed
		 m_outgoing_message.clear();

		 // Read the next message
		 async_start_read();
	 }

	 //-------------------------------------------------------------------------
	 /**
	  * Code to be executed when an entire new message was read
	  *
	  * @param ec Indicates a possible error
	  */

	 void when_read(
		 boost::system::error_code ec
		 , std::size_t /* nothing */
	 ) {
		 if(ec) {
			 glogger
				 << "In GBeastClientT<processable_type>::when_read():" << std::endl
				 << "Got ec(\"" << ec.message() << "\"). async_start_write() will not be executed." << std::endl
				 << "This will terminate the client." << std::endl
				 << GLOGGING;

			 // Give the audience a hint why we are terminating
			 m_close_code = boost::beast::websocket::close_code::going_away;

			 // This will terminate the client
			 return;
		 }

		 // Deal with the message and send a response back. Processing
		 // of work items is done inside of process_request().
		 try {
			 async_start_write(process_request());
		 } catch(...) {
			 // Give the audience a hint why we are terminating
			 m_close_code = boost::beast::websocket::close_code::internal_error;
		 }
	 }

	 //-------------------------------------------------------------------------
	 /**
	  * Processing of incoming messages and creation of responses takes place here
	  *
	  * @return A string holding the response to be sent to the peer
	  */

	 std::string process_request(){
		 // Extract the string from the buffer
		 auto message = boost::beast::buffers_to_string(m_incoming_buffer.data());

		 // De-serialize the object
		 m_command_container.from_string(message, m_serialization_mode); // may throw

		 // Clear the buffer, so we may later fill it with data to be sent
		 m_incoming_buffer.consume(m_incoming_buffer.size());

		 // Extract the command
		 auto inboundCommand = m_command_container.get_command();

		 // Act on the command received
		 switch(inboundCommand) {
			 case payload_command::COMPUTE: {
				 // Process the work item
				 m_command_container.process();

				 // Set the command for the way back to the server
				 m_command_container.set_command(payload_command::RESULT);
			 } break;

			 case payload_command::NODATA: // This must be a command payload
			 case payload_command::ERROR: { // We simply ask for new work
				 // sleep for a short while (between 10 and 50 milliseconds, randomly),
				 // before we ask for new work.
				 std::uniform_int_distribution<> dist(10, 50);
				 std::this_thread::sleep_for(std::chrono::milliseconds(dist(m_rng_engine)));

				 // Tell the server again we need work
				 m_command_container.reset(payload_command::GETDATA);
			 } break;

			 default: {
				 throw gemfony_exception(
					 g_error_streamer(DO_LOG,  time_and_place)
						 << "GBeastClientT<processable_type>::process_request():" << std::endl
						 << "Got unknown or invalid command " << boost::lexical_cast<std::string>(inboundCommand) << std::endl
				 );
			 } break;
		 }

		 // Serialize the object again and return the result
		 return m_command_container.to_string();
	 }

	 //-------------------------------------------------------------------------
	 /**
	  * Closes the connection to the peer
	  *
	  * @param cc The close code to be sent to the peer
	  */
	 void do_close(close_code cc) {
		 if(m_ws.is_open()) {
			 m_ws.close(cc);
		 }

		 if(m_ws.next_layer().is_open()) {
			 boost::system::error_code ec;

			 m_ws.next_layer().shutdown(socket::shutdown_both, ec);
			 m_ws.next_layer().close(ec);

			 if(ec) {
				 glogger
					 << "In GBeastClientT<processable_type>::do_close():" << std::endl
					 << "Got ec(\"" << ec.message() << "\")." << std::endl
					 << "We will throw an exception, as there are no other options left" << std::endl
					 << GLOGGING;

				 // Not much more we can do
				 throw gemfony_exception(
					 g_error_streamer(DO_LOG,  time_and_place)
						 << "GBeastClientT<processable_type>::do_close():" << std::endl
						 << "Shutdown of next layer has failed" << std::endl
				 );
			 }
		 }
	 }

	 //-------------------------------------------------------------------------

	 /** @brief Callback for control frames */
	 std::function<void(boost::beast::websocket::frame_type, boost::beast::string_view)> f_when_control_frame_arrived;

	 //-------------------------------------------------------------------------
	 // Data

	 boost::asio::io_context m_io_context; ///< The io-service object handling the asynchronous processing
	 resolver m_resolver{m_io_context}; ///< Helps to resolve the peer
	 boost::beast::websocket::stream<socket> m_ws{m_io_context}; ///< All messages are sent and received through this socket

	 std::string m_address; ///< The ip address or name of the peer system
	 unsigned int m_port; ///< The peer port

	 boost::beast::multi_buffer m_incoming_buffer;
	 std::string m_outgoing_message; ///< Helps to persist outgoing messages

	 std::random_device m_nondet_rng; ///< Source of non-deterministic random numbers
	 std::mt19937 m_rng_engine{m_nondet_rng()}; ///< The actual random number engine, seeded my m_nondet_rng

	 boost::beast::websocket::close_code m_close_code
		 = boost::beast::websocket::close_code::normal; ///< Holds the close code when terminating the connection

	 Gem::Common::serializationMode m_serialization_mode = Gem::Common::serializationMode::BINARY;

	 GCommandContainerT<processable_type, beast_payload_command> m_command_container{beast_payload_command::NONE, nullptr}; ///< Holds the current command and payload (if any)

	 //-------------------------------------------------------------------------
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * It is the main responsibility of this class to start new server sessions
 * for each client connection, and to interact with the Broker
 */
template<typename processable_type>
class GBeastConsumerT
	: public Gem::Courtier::GBaseConsumerT<processable_type> // note: GBaseConsumerT<> is non-copyable
{
	 // --------------------------------------------------------------
	 // Simplify usage of namespaces
	 using error_code = boost::system::error_code;

public:
private:
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Courtier */
} /* namespace Gem */

#endif /* GENEVA_LIBRARY_COLLECTION_GBEASTCONSUMERT_HPP */
