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
		 m_command = cp.m_command; cp.m_command = beast_payload_command::NONE;
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
		 m_command = cp.m_command; cp.m_command = beast_payload_command::NONE;
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
class GWebsocketClientT final
	: public std::enable_shared_from_this<GWebsocketClientT>
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
	 GWebsocketClientT(
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
	 ~GWebsocketClientT() = default;

	 //-------------------------------------------------------------------------
	 // Deleted functions

	 // Deleted default-constructor -- enforce usage of a particular constructor
	 GWebsocketClientT() = delete;
	 // Deleted copy-constructors and assignment operators -- the client is non-copyable
	 GWebsocketClientT(const GWebsocketClientT<processable_type>&) = delete;
	 GWebsocketClientT(GWebsocketClientT<processable_type>&&) = delete;
	 GWebsocketClientT<processable_type>& operator=(const GWebsocketClientT<processable_type>&) = delete;
	 GWebsocketClientT<processable_type>& operator=(GWebsocketClientT<processable_type>&&) = delete;

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
		 	<< "GWebsocketClientT<processable_type>::async_start_run(): Closing down remaining connections" << std::endl
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
		 // Do nothing if we have been asked to stop
		 if(this->halt()) return;

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
		 // Do nothing if we have been asked to stop
		 if(this->halt()) return;

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
				 << "In GWebsocketClientT<processable_type>::when_resolved():" << std::endl
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
				 << "In GWebsocketClientT<processable_type>::when_connected():" << std::endl
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
				 << "In GWebsocketClientT<processable_type>::when_handshake_complete():" << std::endl
				 << "Got ec(\"" << ec.message() << "\"). async_start_write() will not be executed." << std::endl
				 << "This will terminate the client." << std::endl
				 << GLOGGING;

			 // Give the audience a hint why we are terminating
			 m_close_code = boost::beast::websocket::close_code::going_away;

			 // This will terminate the client
			 return;
		 }

		 // Send the first command to the server
		 m_command_container.reset(beast_payload_command::GETDATA);
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
				 << "In GWebsocketClientT<processable_type>::when_written():" << std::endl
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
				 << "In GWebsocketClientT<processable_type>::when_read():" << std::endl
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
			 case beast_payload_command::COMPUTE: {
				 // Process the work item
				 m_command_container.process();

				 // Set the command for the way back to the server
				 m_command_container.set_command(beast_payload_command::RESULT);
			 } break;

			 case beast_payload_command::NODATA: // This must be a command payload
			 case beast_payload_command::ERROR: { // We simply ask for new work
				 // sleep for a short while (between 10 and 50 milliseconds, randomly),
				 // before we ask for new work.
				 std::uniform_int_distribution<> dist(10, 50);
				 std::this_thread::sleep_for(std::chrono::milliseconds(dist(m_rng_engine)));

				 // Tell the server again we need work
				 m_command_container.reset(beast_payload_command::GETDATA);
			 } break;

			 default: {
				 throw gemfony_exception(
					 g_error_streamer(DO_LOG,  time_and_place)
						 << "GWebsocketClientT<processable_type>::process_request():" << std::endl
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
					 << "In GWebsocketClientT<processable_type>::do_close():" << std::endl
					 << "Got ec(\"" << ec.message() << "\")." << std::endl
					 << "We will throw an exception, as there are no other options left" << std::endl
					 << GLOGGING;

				 // Not much more we can do
				 throw gemfony_exception(
					 g_error_streamer(DO_LOG,  time_and_place)
						 << "GWebsocketClientT<processable_type>::do_close():" << std::endl
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
/**
 * Server-side handling of client-connection. A new session is started for each
 * new connection.
 */
template<typename processable_type>
class GWebsocketServerSessionT final
	: public std::enable_shared_from_this<GWebsocketServerSessionT<processable_type>>
{
	 //-------------------------------------------------------------------------
	 // Make the code easier to read
	 using error_code = boost::system::error_code;
	 using frame_type = boost::beast::websocket::frame_type;
	 using string_view = boost::beast::string_view;

public:
	 //-------------------------------------------------------------------------
	 /**
	  * The only allowed constructor for this class
	  *
	  * @param socket All communication goes through this socket
	  * @param get_next_payload_item Callback for the retrieval of payload items
	  * @param check_server_stopped Callback used to check whether a halt was requested by the server
	  * @param server_sign_on Callback to inform the server that a new session is active or has retired
	  */
	 GWebsocketServerSessionT(
		 boost::asio::ip::tcp::socket socket
		 , std::function<bool(payload_base*& plb_ptr)> get_next_payload_item
		 , std::function<bool()> check_server_stopped
		 , std::function<void(bool)> server_sign_on
		 , Gem::Common::serializationMode serialization_mode
	 )
		 : m_ws(std::move(socket))
		 , m_strand(m_ws.get_executor())
		 , m_timer(m_ws.get_executor().context(), (std::chrono::steady_clock::time_point::max)())
		 , m_get_next_payload_item(std::move(get_next_payload_item))
		 , m_check_server_stopped(std::move(check_server_stopped))
		 , m_server_sign_on(std::move(server_sign_on))
	 	 , m_serialization_mode(serialization_mode
	 {
		 // Set the auto_fragment option, so control frames are delivered timely
		 m_ws.auto_fragment(true);
		 m_ws.write_buffer_size(16384);

		 // Set the transfer mode according to the defines in CMakeLists.txt
		 set_transfer_mode(m_ws);
	 }

	 //-------------------------------------------------------------------------
	 /** @brief The destructor */
	 ~GWebsocketServerSessionT() = default;

	 //-------------------------------------------------------------------------
	 /**
	  * Initiates all communication and processing
	  */
	 void async_start_run() {
		 // ---------------------------------------------------
		 // Connections and communication

		 async_start_accept();

		 // ---------------------------------------------------
		 // Prepare ping cycle. It must start after the handshake, upon whose
		 // completion the when_connection_accepted() function is called.
		 // async_start_ping() is executed from there.

		 // Set a control-frame callback
		 f_when_control_frame_arrived
			 = [this](frame_type frame_t, string_view s) {
			 if(
				 // We might have received a pong as an answer to a our own ping,
				 // or someone might be sending us pings. In either case the line is active.
				 boost::beast::websocket::frame_type::pong==frame_t
				 || boost::beast::websocket::frame_type::ping==frame_t
				 ) {
				 // Note that the connection is alive
				 this->m_ping_state = ping_state::CONNECTION_IS_ALIVE;
			 }
		 };

		 // Set the callback to be executed on every incoming control frame.
		 m_ws.control_callback(f_when_control_frame_arrived);

		 // ---------------------------------------------------
		 // This function will terminate shortly after it was called,
		 // as all operations are performed asynchronously.
	 }

	 //-------------------------------------------------------------------------
	 // Deleted functions
	 GWebsocketServerSessionT() = delete;
	 GWebsocketServerSessionT(const GWebsocketServerSessionT<processable_type>&) = delete;
	 GWebsocketServerSessionT(GWebsocketServerSessionT<processable_type>&&) = delete;
	 GWebsocketServerSessionT<processable_type>& operator=(const GWebsocketServerSessionT<processable_type>&) = delete;
	 GWebsocketServerSessionT<processable_type>& operator=(GWebsocketServerSessionT<processable_type>&&) = delete;

private:
	 //-------------------------------------------------------------------------
	 /**
	  * Initiates a new asynchroneous read session
	  */
	 void async_start_read() {
		 // Read a message into our buffer
		 auto self = shared_from_this();
		 m_ws.async_read(
			 m_incoming_buffer
			 , boost::asio::bind_executor(
				 m_strand
				 , [self](
					 boost::system::error_code ec
					 , std::size_t nBytesTransferred
				 ) {
					 self->when_read(ec, nBytesTransferred);
				 }
			 )
		 );
	 }

	 //-------------------------------------------------------------------------
	 /**
	  * Initiates a new asynchroneous write session
	  *
	  * @param message The message to be sent to the peer
	  */
	 void async_start_write(const std::string& message) {
		 // We need to persist the message for asynchronous operations
		 m_outgoing_message = message;

		 // Echo the message
		 auto self = shared_from_this();
		 m_ws.async_write(
			 boost::asio::buffer(m_outgoing_message)
			 , boost::asio::bind_executor(
				 m_strand
				 , [self](
					 boost::system::error_code ec
					 , std::size_t nBytesTransferred
				 ) {
					 self->when_written(ec, nBytesTransferred);
				 }
			 )
		 );
	 }

	 //-------------------------------------------------------------------------
	 /**
	  * Initiates pinging of the peer, so the connection may be kept alive
	  */
	 void async_start_ping() {
		 // Set the timer
		 m_timer.expires_after(m_ping_interval);

		 // Start to wait asynchronously. This call will return immediately.
		 // when_timer_fired() will be called once the timer has expired.
		 auto self = shared_from_this();
		 m_timer.async_wait(
			 boost::asio::bind_executor(
				 m_strand
				 , [self](boost::system::error_code ec) {
					 self->when_timer_fired(ec);
				 }
			 )
		 );

		 // Setting the ping state must be done before the ping is sent, or
		 // else the pong might arrive before the SENDING_PING state is set
		 // and we might overwrite the CONNECTION_IS_ALIVE state set by the
		 // control-frame callback
		 m_ping_state = ping_state::SENDING_PING;

		 // Start the ping session
		 m_ws.async_ping(
			 m_ping_data
			 , boost::asio::bind_executor(
				 m_strand
				 , [self](boost::system::error_code ec) {
					 self->when_ping_sent(ec);
				 }
			 )
		 );
	 }

	 //-------------------------------------------------------------------------
	 /**
	  * Starts waiting for websocket handshakes over an already established
	  * ASIO connection.
	  */
	 void async_start_accept() {
		 // This function initiates an asynchronous chain of callbacks, where each callback is
		 // executed when the previous call (here: async_accept) is completed. Error handling is
		 // done in the callback, using an error code provided by Boost.Beast and/or Boost.ASIO.
		 auto self = shared_from_this();
		 m_ws.async_accept(
			 boost::asio::bind_executor(
				 m_strand
				 , [self](boost::system::error_code ec) {
					 self->when_connection_accepted(ec);
				 }
			 )
		 );
	 }

	 //-------------------------------------------------------------------------
	 /**
	  * Reacts on errors when sending a ping
	  *
	  * @param ec The error code of a potential error
	  */
	 void when_ping_sent(boost::system::error_code ec) {
		 if(ec) {
			 if(ec != boost::asio::error::operation_aborted) {
				 glogger
				 	<< "GWebsocketServerSessionT<processable_type>::when_ping_sent(): " << ec.message() << std::endl
				   << GLOGGING;
			 }

			 m_ping_state = ping_state::CONNECTION_IS_STALE;
		 }
	 }

	 //-------------------------------------------------------------------------
	 /**
	  * A timer is started in regular intervals. This code is executed when
	  * the timer expires
	  *
	  * @param ec The error code of a potential error
	  */
	 void when_timer_fired(boost::system::error_code ec) {
		 if(ec) {
			 if(ec != boost::asio::error::operation_aborted) {
				 glogger
					 << "GWebsocketServerSessionT<processable_type>::when_timer_fired(): " << ec.message() << std::endl
					 << GLOGGING;
			 }

			 m_ping_state = ping_state::CONNECTION_IS_STALE;
			 return;
		 }

		 if(m_ping_state == ping_state::CONNECTION_IS_ALIVE) {
			 // Start the next ping session, if this is a healthy connection
			 async_start_ping();
			 return;
		 } else {
			 m_ping_state = ping_state::CONNECTION_IS_STALE;

			 if(!this->m_check_server_stopped()) {
				 // Either this is a stale connection or the SENDING_PING flag is still set
				 glogger
				 	<< "GWebsocketServerSessionT<processable_type>::when_timer_fired():" << std::endl
				 	<< "Connection seems to be dead: " << m_ping_state << std::endl
					<< GLOGGING;
			 }
			 return;
		 }
	 }

	 //-------------------------------------------------------------------------
	 /**
	  * Code to be executed when a handshake was accepted
	  *
	  * @param ec The error code of a potential error
	  */
	 void when_connection_accepted(boost::system::error_code ec) {
		 if(ec) {
			 glogger
				 << "GWebsocketServerSessionT<processable_type>::when_connection_accepted(): "  << ec.message() << std::endl
				 << GLOGGING;

			 do_close(boost::beast::websocket::close_code::going_away);

			 return;
		 }

		 // Make it known to the server that a new session is alive
		 m_server_sign_on(true);

		 // Start reading an incoming message. This
		 // call will return immediately.
		 async_start_read();

		 // Start the ping cycle
		 async_start_ping();
	 }

	 //-------------------------------------------------------------------------
	 /**
	  * Code to be executed when a new message was read
	  *
	  * @param ec The error code of a potential error
	  */
	 void when_read(
		 boost::system::error_code ec
		 , std::size_t /* nothing */
	 ) {
		 if(ec) {
			 if(ec != boost::beast::websocket::error::closed) {
				 glogger
					 << "GWebsocketServerSessionT<processable_type>::when_read(): " << ec.message() << std::endl
		          << GLOGGING;
			 }

			 do_close(boost::beast::websocket::close_code::going_away);
			 return;
		 }

		 // Deal with the message and send a response back
		 try {
			 async_start_write(process_request());
		 } catch(...) {
			 do_close(boost::beast::websocket::close_code::internal_error);
			 return;
		 }
	 }

	 //-------------------------------------------------------------------------
	 /**
	  * Code to be executed when a message was written
	  *
	  * @param ec The error code of a potential error
	  */
	 void when_written(
		 boost::system::error_code ec
		 , std::size_t /* nothing */
	 ) {
		 if(ec) {
			 if(ec != boost::beast::websocket::error::closed) {
				 glogger
					 << "GWebsocketServerSessionT<processable_type>::when_written(): " << ec.message() << std::endl
					 << GLOGGING;
			 }

			 do_close(boost::beast::websocket::close_code::going_away);
			 return;
		 }

		 // Clear the outgoing message -- no longer needed
		 m_outgoing_message.clear();

		 if(this->m_check_server_stopped()) {
			 glogger
				 << "GWebsocketServerSessionT<processable_type>::when_written(): Server seems to be stopped" << std::endl
				 << GLOGGING;

			 // Do not continue if a stop criterion was reached
			 do_close(boost::beast::websocket::close_code::normal);
		 } else {
			 // Start another read cycle
			 async_start_read();
		 }
	 }

	 //-------------------------------------------------------------------------
	 /** @brief Callback for control frames */
	 std::function<void(boost::beast::websocket::frame_type, boost::beast::string_view)> f_when_control_frame_arrived;

	 //-------------------------------------------------------------------------
	 /**
	  * Shuts down the (websocket and ASIO) connection to the peer
	  *
	  * @param cc A close-code to be transmitted to the peer
	  */
	 void do_close(boost::beast::websocket::close_code cc) {
		 glogger
			 << "GWebsocketServerSessionT<processable_type>::do_close(): Closing down connection" << std::endl
			 << GLOGGING;

		 // Store the close code for later reference
		 m_close_code = cc;

		 // Make sure no more pings are sent and the timer expires
		 m_timer.cancel();

		 if(m_ws.is_open()) {
			 // Close the connection
			 m_ws.close(cc);
		 }

		 if(m_ws.next_layer().is_open()) {
			 boost::system::error_code ec;

			 // Closing the socket cancels all outstanding operations. They
			 // will complete with boost::asio::error::operation_aborted
			 m_ws.next_layer().shutdown(
				 boost::asio::ip::tcp::socket::shutdown_both
				 , ec
			 );
			 m_ws.next_layer().close(ec);

			 if (ec) {
				 // Not much else we can do here
				 throw gemfony_exception(
					 g_error_streamer(DO_LOG,  time_and_place)
						 << "GWebsocketServerSessionT<processable_type>::do_close():" << std::endl
						 << "Shutdown of next layer has failed" << std::endl
						 << "Got error code " << ec.message() << std::endl
				 );
			 }
		 }

		 // Make it known to the server that a session is leaving
		 m_server_sign_on(false);
	 }

	 //-------------------------------------------------------------------------
	 /**
	  * Processing of incoming messages and creation of a response-string
	  *
	  * @return The response to be sent to the server in the case of an incoming message
	  */
	 std::string process_request(){
		 // Extract the string from the buffer
		 auto message = boost::beast::buffers_to_string(m_incoming_buffer.data());

		 // De-serialize the object
		 m_command_container.from_string(message, m_serialization_mode);

		 // Clear the buffer, so we may later fill it with data to be sent
		 m_incoming_buffer.consume(m_incoming_buffer.size());

		 // Extract the command
		 auto inboundCommand = m_command_container.get_command();

		 // Act on the command received
		 switch(inboundCommand) {
			 case beast_payload_command::GETDATA:
			 case beast_payload_command::ERROR: { // TODO: Act on error condition
				 return getAndSerializeWorkItem();
			 } break;

			 case beast_payload_command::RESULT: {
				 // TODO: Add a server callback to hand back processed work items

				 // Check that work was indeed done
				 // TODO: This check is not necessary here. Work items may have errors.
				 if(!m_command_container.is_processed()) {
					 throw std::runtime_error("GWebsocketServerSessionT<processable_type>::process_request(): Returned payload is unprocessed");
				 }

				 // Retrieve the next work item and send it to the client for processing
				 return getAndSerializeWorkItem();
			 } break;

			 default: {
				 throw std::runtime_error(
					 "GWebsocketServerSessionT<processable_type>::process_request(): Got unknown or invalid command "
					 + boost::lexical_cast<std::string>(inboundCommand)
				 );
			 } break;
		 }
	 }

	 //-------------------------------------------------------------------------
	 /**
	  * Retrieval of a work item from the server and serialization
	  *
	  * @return A serialized representation of the work item
	  */
	 std::string getAndSerializeWorkItem() {
		 // Obtain a container_payload object from the queue, serialize it and send it off
		 payload_base *plb_ptr = nullptr;
		 if (this->m_get_next_payload_item(plb_ptr) && plb_ptr != nullptr) {
			 m_command_container.reset(beast_payload_command::COMPUTE, plb_ptr);
		 } else {
			 // Let the remote side know whe don't have work
			 m_command_container.reset(beast_payload_command::NODATA);
		 }

		 return m_command_container.to_string(m_serialization_mode);
	 }

	 //-------------------------------------------------------------------------
	 // Data

	 boost::beast::websocket::stream<boost::asio::ip::tcp::socket> m_ws;
	 boost::asio::strand<boost::asio::io_context::executor_type> m_strand;

	 boost::beast::multi_buffer m_incoming_buffer;
	 std::string m_outgoing_message;

	 std::function<bool(payload_base*& plb_ptr)> m_get_next_payload_item;

	 const std::chrono::seconds m_ping_interval{DEFAULTPINGINTERVAL}; // Time between two pings in seconds
	 boost::asio::steady_timer m_timer;
	 std::atomic<ping_state> m_ping_state{ping_state::CONNECTION_IS_ALIVE};
	 const boost::beast::websocket::ping_data m_ping_data{};

	 command_container m_command_container{beast_payload_command::NONE, nullptr}; ///< Holds the current command and payload (if any)

	 std::function<bool()> m_check_server_stopped;
	 boost::beast::websocket::close_code m_close_code
		 = boost::beast::websocket::close_code::normal; ///< Holds the close code when terminating the connection

	 std::function<void(bool)> m_server_sign_on;

	 Gem::Common::serializationMode m_serialization_mode = Gem::Common::serializationMode::BINARY;

	 //-------------------------------------------------------------------------
};


/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * It is the main responsibility of this class to start new server sessions
 * for each client connection, and to interact with the Broker
 */
template<typename processable_type>
class GWebsocketServerT final
	: public Gem::Courtier::GBaseConsumerT<processable_type> // note: GBaseConsumerT<> is non-copyable
{
	 //-------------------------------------------------------------------------
	 // Simplify usage of namespaces
	 using error_code = boost::system::error_code;

public:
	 //-------------------------------------------------------------------------

	 GWebsocketServerT(
		 const std::string& address
		 , unsigned short port
		 , std::size_t n_context_threads
		 , std::size_t n_producer_threads
		 , std::size_t n_max_packages_served
		 , payload_type payload_type
		 , std::size_t container_size
		 , double sleep_time
		 , std::size_t full_queue_sleep_ms
		 , std::size_t max_queue_size
		 , std::size_t ping_interval
		 , bool verbose_control_frames
	 )
		 : m_endpoint(boost::asio::ip::make_address(address), port)
			, m_n_listener_threads(n_context_threads>0?n_context_threads:std::thread::hardware_concurrency())
			, m_n_max_packages_served(n_max_packages_served)
			, m_payload_type(payload_type)
			, m_n_producer_threads(n_producer_threads>0?n_producer_threads:std::thread::hardware_concurrency())
			, m_container_size(container_size)
			, m_sleep_time(sleep_time)
			, m_full_queue_sleep_ms(full_queue_sleep_ms)
			, m_max_queue_size(max_queue_size)
			, m_ping_interval(ping_interval)
			, m_verbose_control_frames(verbose_control_frames)
			, m_payload_queue{m_max_queue_size}
	 { /* nothing */ }

	 //-------------------------------------------------------------------------

	 void run() {
		 boost::system::error_code ec;

		 // Reset the package counter
		 m_n_packages_served = 0;

		 // Indicate that the server may async_start_run
		 m_server_stopped = false;

		 // Open the acceptor
		 m_acceptor.open(m_endpoint.protocol(), ec);
		 if(ec || !m_acceptor.is_open()) {
			 if(ec) { std::cerr << "async_start_run/m_acceptor.open: " << ec.message() << std::endl; }
			 return;
		 }

		 // Bind to the server address
		 m_acceptor.bind(m_endpoint, ec);
		 if(ec) {
			 std::cerr << "async_start_run/m_acceptor.bind: " << ec.message() << std::endl;
			 return;
		 }

		 // Start listening for connections
		 m_acceptor.listen(boost::asio::socket_base::max_listen_connections, ec);
		 if(ec) {
			 std::cerr << "async_start_run/m_acceptor.listen: " << ec.message() << std::endl;
			 return;
		 }

		 // Start producers
		 m_producer_threads_vec.reserve(m_n_producer_threads);
		 switch(m_payload_type) {
			 //------------------------------------------------
			 case payload_type::container: {
				 for (std::size_t i = 0; i < m_n_producer_threads; i++) {
					 m_producer_threads_vec.emplace_back(
						 std::thread(
							 [this](std::size_t container_size, std::size_t full_queue_sleep_ms){
								 this->container_payload_producer(container_size, full_queue_sleep_ms);
							 }
							 , m_container_size
							 , m_full_queue_sleep_ms
						 )
					 );
				 }
			 } break;

				 //------------------------------------------------
			 case payload_type::sleep: {
				 for (std::size_t i = 0; i < m_n_producer_threads; i++) {
					 m_producer_threads_vec.emplace_back(
						 std::thread(
							 [this](double sleep_time, std::size_t full_queue_sleep_ms) {
								 this->sleep_payload_producer(sleep_time, full_queue_sleep_ms);
							 }
							 , m_sleep_time
							 , m_full_queue_sleep_ms
						 )
					 );
				 }
			 } break;

				 //------------------------------------------------
			 case payload_type::command: { // This is a severe error
				 throw std::runtime_error(R"(async_websocket_server::run(): Got invalid payload_type "command")");
			 } break;

				 //------------------------------------------------
		 }


		 //------------------------------------------------------
		 // And ... action!

		 // Will return immediately
		 async_start_accept();

		 // Allow to serve requests from multiple threads
		 m_context_thread_vec.reserve(m_n_listener_threads-1);
		 for(std::size_t t_cnt=0; t_cnt<(m_n_listener_threads-1); t_cnt++) {
			 m_context_thread_vec.emplace_back(
				 std::thread(
					 [this](){
						 this->m_io_context.run();
					 }
				 )
			 );
		 }

		 // Block until all work is done
		 m_io_context.run();

		 //------------------------------------------------------
		 // Wait for the server to shut down

		 // Make sure the stop flag has really been set
		 assert(true==this->m_server_stopped);

		 // Wait for context threads to finish
		 for (auto &t: m_context_thread_vec) { t.join(); }
		 m_context_thread_vec.clear();

		 // Wait for producer threads to finish
		 for (auto &t: m_producer_threads_vec) { t.join(); }
		 m_producer_threads_vec.clear();
	 }

	 //-------------------------------------------------------------------------
	 // Deleted default constructor, copy-/move-constructors and assignment operators.
	 // We want to enforce the usage of a single, specialized constructor.

	 GWebsocketServerT() = delete;
	 GWebsocketServerT(const GWebsocketServerT<processable_type>&) = delete;
	 GWebsocketServerT(GWebsocketServerT<processable_type>&&) = delete;
	 GWebsocketServerT& operator=(const GWebsocketServerT<processable_type>&) = delete;
	 GWebsocketServerT& operator=(GWebsocketServerT<processable_type>&&) = delete;

private:
	 //-------------------------------------------------------------------------

	 void async_start_accept(){
		 auto self = shared_from_this();
		 m_acceptor.async_accept(
			 m_socket
			 , [self](boost::system::error_code ec) {
				 self->when_accepted(ec);
			 }
		 );
	 }

	 //-------------------------------------------------------------------------

	 void when_accepted(error_code) {
		 if(m_server_stopped) return;

		 if(ec) {
			 std::cerr << "when_connection_accepted: " << ec.message() << std::endl;
		 } else {
			 // Create the async_websocket_server_session and async_start_run it. This call will return immediately.
			 std::make_shared<async_websocket_server_session>(
				 std::move(m_socket)
				 , [this](payload_base *&plb_ptr) -> bool { return this->getNextPayloadItem(plb_ptr); }
				 , [this]() -> bool { return this->server_stopped(); }
				 , [this](bool sign_on) {
					 if(sign_on) {
						 this->m_n_active_sessions++;
					 } else {
						 if(0 == this->m_n_active_sessions) {
							 throw std::runtime_error("In async_websocket_server::when_accepted(): Tried to decrement #sessions which is already 0");
						 } else {
							 // This won't help, though, if m_n_active_sessions becomes 0 after the if-check
							 this->m_n_active_sessions--;
						 }
					 }

					 std::cout << this->m_n_active_sessions << " active sessions" << std::endl;
				 }
				 , m_ping_interval
				 , m_verbose_control_frames
			 )->async_start_run();
		 }

		 // Accept another connection
		 if(!this->m_server_stopped) async_start_accept();
	 }

	 //-------------------------------------------------------------------------

	 bool getNextPayloadItem(payload_base*& plb_ptr) {
		 // Try to retrieve a work item
		 bool success = m_payload_queue.pop(plb_ptr);

		 // Update counters and the stop flag, if successful
		 if(success) {
			 if(m_n_packages_served++ < m_n_max_packages_served){
				 if(m_n_packages_served%10 == 0) {
					 std::cout << "async_websocket_server served " << m_n_packages_served << " packages" << std::endl;
				 }
			 } else { // Leave
				 // Indicate to all parties that we want to stop
				 m_server_stopped = true;
				 // Stop accepting new connections
				 m_acceptor.close();
				 // Finally close the socket
				 m_socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
				 m_socket.close();
			 }
		 }

		 // Let the audience know
		 return success;
	 }

	 //-------------------------------------------------------------------------

	 void container_payload_producer(
		 std::size_t containerSize
		 , std::size_t full_queue_sleep_ms
	 ) {
		 std::random_device nondet_rng;
		 std::mt19937 mersenne(nondet_rng());
		 std::normal_distribution<double> normalDist(0.,1.);

		 bool produce_new_container = true;
		 container_payload *sc_ptr = nullptr;
		 while (true) {
			 using namespace std::literals;

			 // Only create a new container if the old one was
			 // successfully added to the queue
			 if (produce_new_container) {
				 sc_ptr = new container_payload(containerSize, normalDist, mersenne);
			 }

			 if (!m_payload_queue.push(sc_ptr)) { // Container could not be added to the queue
				 if(this->m_server_stopped) break;
				 produce_new_container = false;
				 std::this_thread::sleep_for(std::chrono::milliseconds(full_queue_sleep_ms));
			 } else {
				 produce_new_container = true;
			 }
		 }
	 }

	 //-------------------------------------------------------------------------

	 void sleep_payload_producer(
		 double sleep_time
		 , std::size_t full_queue_sleep_ms
	 ) {
		 bool produce_new_container = true;
		 sleep_payload *sp_ptr = nullptr;
		 while (!this->m_server_stopped) {
			 using namespace std::literals;

			 // Only create a new container if the old one was
			 // successfully added to the queue
			 if (produce_new_container) {
				 sp_ptr = new sleep_payload(sleep_time);
			 }

			 if (!m_payload_queue.push(sp_ptr)) { // Container could not be added to the queue
				 if(this->m_server_stopped) break;
				 produce_new_container = false;
				 std::this_thread::sleep_for(std::chrono::milliseconds(full_queue_sleep_ms));
			 } else {
				 produce_new_container = true;
			 }
		 }
	 }

	 //-------------------------------------------------------------------------

	 bool server_stopped() const {
		 return this->m_server_stopped.load();
	 }

	 //-------------------------------------------------------------------------
	 // Data and Queues

	 boost::asio::ip::tcp::endpoint m_endpoint;
	 std::size_t m_n_listener_threads;
	 boost::asio::io_context m_io_context{boost::numeric_cast<int>(m_n_listener_threads)};
	 boost::asio::ip::tcp::acceptor m_acceptor{m_io_context};
	 boost::asio::ip::tcp::socket m_socket{m_io_context};
	 std::vector<std::thread> m_context_thread_vec;
	 std::atomic<std::size_t> m_n_active_sessions{0};
	 std::atomic<std::size_t> m_n_packages_served{0};
	 const std::size_t m_n_max_packages_served;
	 const std::size_t m_full_queue_sleep_ms = 5;
	 const std::size_t m_max_queue_size = 5000;
	 const std::size_t m_ping_interval = 15;
	 bool  m_verbose_control_frames = false; // Whether the control_callback should emit information when a control frame is received

	 std::atomic<bool> m_server_stopped{false};

	 payload_type m_payload_type = payload_type::container; ///< Indicates which sort of payload should be produced

	 std::size_t m_n_producer_threads = 4;
	 std::vector<std::thread> m_producer_threads_vec; ///< Holds threads used to produce payload packages

	 std::size_t m_container_size = 1000; ///< The size of container_payload objects
	 double m_sleep_time = 1.; ///< The sleep time of sleep_payload objects

	 // Holds payloads to be passed to the sessions
	 boost::lockfree::queue<payload_base *, boost::lockfree::fixed_sized<true>> m_payload_queue;

	 //-------------------------------------------------------------------------
};
/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Courtier */
} /* namespace Gem */

#endif /* GENEVA_LIBRARY_COLLECTION_GBEASTCONSUMERT_HPP */
