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

#ifndef GENEVA_LIBRARY_COLLECTION_GWEBSOCKETCONSUMERT_HPP_
#define GENEVA_LIBRARY_COLLECTION_GWEBSOCKETCONSUMERT_HPP_

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
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/rfc6455.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/lexical_cast.hpp>

// Geneva headers go here
#include "common/GStdThreadGroup.hpp"
#include "common/GThreadPool.hpp"
#include "common/GSerializationHelperFunctionsT.hpp"
#include "common/GCommonHelperFunctions.hpp"
#include "common/GCommonHelperFunctionsT.hpp"
#include "common/GCommonEnums.hpp"
#include "courtier/GCourtierHelperFunctions.hpp"
#include "courtier/GBrokerT.hpp"
#include "courtier/GCourtierEnums.hpp"
#include "courtier/GBaseConsumerT.hpp"
#include "courtier/GBaseClientT.hpp"
#include "courtier/GCourtierEnums.hpp"
#include "courtier/GBaseConsumerT.hpp"

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
	 GCommandContainerT(GCommandContainerT<processable_type, command_type>&& cp) noexcept {
		 m_command = cp.m_command; cp.m_command = command_type(0);
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
	  * @param cp Another GCommandContainerT<processable_type, command_type> object
	  * @return A reference to this object
	  */
	 GCommandContainerT<processable_type, command_type>& operator=(GCommandContainerT<processable_type, command_type>&& cp) noexcept {
		 m_command = cp.m_command; cp.m_command = command_type(0);
		 m_payload_ptr.swap(cp.m_payload_ptr); cp.m_payload_ptr.reset();

		 return *this;
	 }

	 //-------------------------------------------------------------------------
	 // Deleted copy-constructors and assignment operator -- the class is non-copyable

	 GCommandContainerT<processable_type, command_type>(const GCommandContainerT<processable_type, command_type>&) = delete;
	 GCommandContainerT<processable_type, command_type>& operator=(const GCommandContainerT<processable_type, command_type>&) = delete;

	 //-------------------------------------------------------------------------
	 /**
	  * Reset to a new command and payload or clear the object
	  *
	  * @return A reference to this object, so we can serialize it in one go
	  */
	 const GCommandContainerT<processable_type, command_type>& reset(
		 command_type command = command_type(0)
		 , std::shared_ptr<processable_type> payload_ptr = std::shared_ptr<processable_type>()
	 ) {
		 m_command = command;
		 m_payload_ptr = payload_ptr;
		 return *this;
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
	  * Retrieves the payload
	  */
	 std::shared_ptr<processable_type> get_payload() const {
		 return m_payload_ptr;
	 }

	 //-------------------------------------------------------------------------
	 /**
	  * Processing of the payload (if any)
	  *
	  * // TODO: Check for errors during processing
	  */
	 void process() {
		 if(m_payload_ptr) {
			 m_payload_ptr->process();
		 } else {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG,  time_and_place)
					 << "In GCommandContainerT<processable_type, command_type>::process():" << std::endl
					 << "Tried to process a work item while m_payload_ptr is empty" << std::endl
			 );
		 }
	 }

	 //-------------------------------------------------------------------------
	 /**
	  * Converts this object to a std::string
	  *
	  * TODO: Check for std::exception
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
				 } break; // archive and stream closed at end of scope

				 case Gem::Common::serializationMode::XML: {
					 std::stringstream(std::ios::out).swap(m_stringstream);
					 boost::archive::xml_oarchive oa(m_stringstream);
					 oa << boost::serialization::make_nvp(
						 "command_container"
						 , *this
					 );
				 } break;

				 case Gem::Common::serializationMode::BINARY: {
					 std::stringstream(std::ios::out | std::ios::binary).swap(m_stringstream);
					 boost::archive::binary_oarchive oa(m_stringstream);
					 oa << boost::serialization::make_nvp(
						 "command_container"
						 , *this
					 );
				 } break;
			 }
		 } catch (const boost::system::system_error &e) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG,  time_and_place)
					 << "In GCommandContainerT<processable_type, command_type>::to_string():" << std::endl
					 << "Caught boost::system::system_error exception with messages:" << std::endl
					 << e.what() << std::endl
			 );
		 } catch (const boost::exception &e) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG,  time_and_place)
					 << "In GCommandContainerT<processable_type, command_type>::to_string():" << std::endl
					 << "Caught boost::exception exception with messages:" << std::endl
					 << boost::diagnostic_information(e) << std::endl
			 );
		 } catch (...) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG,  time_and_place)
					 << "In GCommandContainerT<processable_type, command_type>::to_string():" << std::endl
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
			 GCommandContainerT<processable_type, command_type> local_command_container{command_type(0)};

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
					 << "In GCommandContainerT<processable_type, command_type>::from_string():" << std::endl
					 << "Caught boost::system::system_error exception with messages:" << std::endl
					 << e.what() << std::endl
			 );
		 } catch (const boost::exception &e) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG,  time_and_place)
					 << "In GCommandContainerT<processable_type, command_type>::from_string():" << std::endl
					 << "Caught boost::exception exception with messages:" << std::endl
					 << boost::diagnostic_information(e) << std::endl
			 );
		 } catch (...) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG,  time_and_place)
					 << "In GCommandContainerT<processable_type, command_type>::from_string():" << std::endl
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
	: public Gem::Courtier::GBaseClientT<processable_type>
	, public std::enable_shared_from_this<GWebsocketClientT<processable_type>>
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
	 	, Gem::Common::serializationMode serialization_mode
		, bool verbose_control_frames
	 )
	   : m_address(address)
	   , m_port(port)
	 	, m_serialization_mode(serialization_mode)
	 	, m_verbose_control_frames(verbose_control_frames)
	 {
		 // Set the auto_fragment option, so control frames are delivered timely
		 m_ws.auto_fragment(true);
		 m_ws.write_buffer_size(16384);

		 // Set the transfer mode
		 switch(m_serialization_mode) {
			 case Gem::Common::serializationMode::BINARY:
				 m_ws.binary(true);
				 break;
			 case Gem::Common::serializationMode::XML:
			 case Gem::Common::serializationMode::TEXT:
				 m_ws.binary(false);
				 break;
		 }

		 // Set a control-frame callback
		 f_when_control_frame_arrived
			 = [this](frame_type frame_t, string_view s) {
			 // Let the audience know what type of control frame we have received
			 // if the user has requested it.
			 if(this->m_verbose_control_frames) {
				 if (boost::beast::websocket::frame_type::close == frame_t) {
					 glogger
						 << "GWebsocketClientT<> client has received a close frame" << std::endl
						 << GLOGGING;
				 } else if (boost::beast::websocket::frame_type::ping == frame_t) {
					 glogger
						 << "GWebsocketClientT<> client has received a ping frame" << std::endl
						 << GLOGGING;
				 } else if (boost::beast::websocket::frame_type::pong == frame_t) {
					 glogger
						 << "GWebsocketClientT<> client has received a pong frame" << std::endl
						 << GLOGGING;
				 }
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
		 auto self = this->shared_from_this();
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

		 // We need a thread for the processing of incoming work items
		 std::thread processing_thread (
			 [this]() {
				 this->m_io_context.run();
			 }
		 );

		 // This call will block until no more work remains in the ASIO work queue
		 m_io_context.run();

		 // Wait for the processing thread to terminate
		 processing_thread.join();

		 // Finally close all outstanding connections
		 do_close(m_close_code);

		 // Let the audience know that we have finished the shutdown
		 glogger
			 << "GWebsocketClientT<processable_type>::run_(): Client session has terminated" << std::endl
			 << GLOGGING;
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
		 auto self = this->shared_from_this();
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

		 auto self = this->shared_from_this();
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

		 // Make the ASIO connection on the endpoint we get from a lookup
		 auto self = this->shared_from_this();
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
		 auto self = this->shared_from_this();
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
		 async_start_write(
			 m_command_container.reset(
				 beast_payload_command::GETDATA
			 ).to_string()
		 );

		 // Start the read cycle -- it will keep itself alife
		 async_start_read();
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
			 // Start asynchronous processing of the work item.
			 // As we need to keep the read-cycle alife in parallel,
			 // this requires that the io_context::run()-function is
			 // started in at least two threads.
			 auto self = this->shared_from_this();
			 m_io_context.post(
				 [self]() {
					 self->process_request();
				 }
			 );

			 async_start_read();
		 } catch(...) {
			 // Give the audience a hint why we are terminating
			 glogger
			 	<< "In GWebsocketClientT<processable_type>::when_read():" << std::endl
				<< "Caught exception" << std::endl
				<< GWARNING;
			 m_close_code = boost::beast::websocket::close_code::internal_error;
		 }
	 }

	 //-------------------------------------------------------------------------
	 /**
	  * Processing of incoming messages and creation of responses takes place here
	  */
	 void process_request(){
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
				 // TODO: Make this configurable via the server
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
		 async_start_write(m_command_container.to_string());
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
	 bool m_verbose_control_frames = false; ///< Whether a diagnostic message should be emitted when a control frame arrives

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
	  * @param get_payload_item Callback for the retrieval of payload items
	  * @param put_payload_item Callback for the submission of payload items
	  * @param check_server_stopped Callback used to check whether a halt was requested by the server
	  * @param server_sign_on Callback to inform the server that a new session is active or has retired
	  * @param serialization_mode Informs the session which Boost.Serialization mode should be used
	  * @param ping_interval The interval between two consecutive pings
	  * @param verbose_control_frames Whether the session should emit diagnostic messages upon receipt of a control frame
	  */
	 GWebsocketServerSessionT(
		 boost::asio::ip::tcp::socket socket
		 , std::function<std::shared_ptr<processable_type>()> get_payload_item
		 , std::function<void(std::shared_ptr<processable_type>)> put_payload_item
		 , std::function<bool()> check_server_stopped
		 , std::function<void(bool)> server_sign_on
		 , Gem::Common::serializationMode serialization_mode
		 , std::size_t ping_interval
		 , bool verbose_control_frames
	 )
		 : m_ws(std::move(socket))
		 , m_strand(m_ws.get_executor())
		 , m_timer(m_ws.get_executor().context(), (std::chrono::steady_clock::time_point::max)())
		 , m_get_payload_item(std::move(get_payload_item))
	 	 , m_put_payload_item(std::move(put_payload_item))
		 , m_check_server_stopped(std::move(check_server_stopped))
		 , m_server_sign_on(std::move(server_sign_on))
	 	 , m_serialization_mode(serialization_mode)
		 , m_ping_interval(std::chrono::seconds(ping_interval))
		 , m_verbose_control_frames(verbose_control_frames)
	 {
		 // Set the auto_fragment option, so control frames are delivered timely
		 m_ws.auto_fragment(true);
		 m_ws.write_buffer_size(16384);

		 // Set the transfer mode according to the defines in CMakeLists.txt
		 // Set the transfer mode
		 switch(m_serialization_mode) {
			 case Gem::Common::serializationMode::BINARY:
				 m_ws.binary(true);
				 break;
			 case Gem::Common::serializationMode::XML:
			 case Gem::Common::serializationMode::TEXT:
				 m_ws.binary(false);
				 break;
		 }
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
		 // Prepare ping cycle. It must start after the handshake, upon whose
		 // completion the when_connection_accepted() function is called.
		 // async_start_ping() is executed from there.

		 // Set a control-frame callback
		 f_when_control_frame_arrived
			 = [this](frame_type frame_t, string_view s) {
			 if(
				 // We might have received a pong as an answer to our own ping,
				 // or someone might be sending us pings. In either case the conection is alive.
				 boost::beast::websocket::frame_type::pong==frame_t
				 || boost::beast::websocket::frame_type::ping==frame_t
			 ) {
				 // Note that the connection is alive
				 this->m_ping_state = beast_ping_state::CONNECTION_IS_ALIVE;
			 }

			 // Let the audience know what type of control frame we have received
			 // if the user has requested it.
			 if(this->m_verbose_control_frames) {
				 if (boost::beast::websocket::frame_type::close == frame_t) {
					 glogger
					   << "GWebsocketServerSessionT<> session has received a close frame" << std::endl
						<< GLOGGING;
				 } else if (boost::beast::websocket::frame_type::ping == frame_t) {
					 glogger
						 << "GWebsocketServerSessionT<> session has received a ping frame" << std::endl
						 << GLOGGING;
				 } else if (boost::beast::websocket::frame_type::pong == frame_t) {
					 glogger
						 << "GWebsocketServerSessionT<> session has received a pong frame" << std::endl
						 << GLOGGING;
				 }
			 }
		 };

		 // Set the callback to be executed on every incoming control frame.
		 m_ws.control_callback(f_when_control_frame_arrived);

		 // ---------------------------------------------------
		 // Connections and communication

		 // Wait for a new websocket connection. Note that the
		 // ASIO connection should already be active at this place.
		 async_start_accept();

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
		 auto self = this->shared_from_this();
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
		 auto self = this->shared_from_this();
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
		 auto self = this->shared_from_this();
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
		 m_ping_state = beast_ping_state::SENDING_PING;

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
		 auto self = this->shared_from_this();
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

			 m_ping_state = beast_ping_state::CONNECTION_IS_STALE;
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

			 m_ping_state = beast_ping_state::CONNECTION_IS_STALE;
			 return;
		 }

		 if(m_ping_state == beast_ping_state::CONNECTION_IS_ALIVE) {
			 // Start the next ping session, if this is a healthy connection
			 async_start_ping();
			 return;
		 } else {
			 m_ping_state = beast_ping_state::CONNECTION_IS_STALE;

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
		 async_start_write(process_request());
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
		 try {
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
				 case beast_payload_command::ERROR: {
					 return getAndSerializeWorkItem();
				 } break;

				 case beast_payload_command::RESULT: {
					 // Retrieve the payload from the command container
					 auto payload_ptr = m_command_container.get_payload();

					 // Submit the payload to the server (which will send it to the broker)
					 if(payload_ptr) {
						 this->m_put_payload_item(payload_ptr);
					 } else {
						 glogger
							 << "GWebsocketServerSessionT<processable_type>::process_request():" << std::endl
							 << "payload is empty even though a result was expected" << std::endl
							 << GWARNING;
					 }

					 // Retrieve the next work item and send it to the client for processing
					 return getAndSerializeWorkItem();
				 } break;

				 default: {
					 glogger
						 << "GWebsocketServerSessionT<processable_type>::process_request():" << std::endl
						 << "Got unknown or invalid command " << boost::lexical_cast<std::string>(inboundCommand) << std::endl
						 << GWARNING;
				 } break;
			 }
		 } catch(...) {
			 glogger
				 << "GWebsocketServerSessionT<processable_type>::process_request(): Caught exception" << std::endl
				 << GLOGGING;

			 do_close(boost::beast::websocket::close_code::internal_error);
		 }

		 return std::string();
	 }

	 //-------------------------------------------------------------------------
	 /**
	  * Retrieval of a work item from the server and serialization
	  *
	  * @return A serialized representation of the work item
	  */
	 std::string getAndSerializeWorkItem() {
		 // Obtain a container_payload object from the queue, serialize it and send it off
		 auto payload_ptr = this->m_get_payload_item();

		 if(payload_ptr) { // Did we get a valid item ?
			 m_command_container.reset(beast_payload_command::COMPUTE, payload_ptr);
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

	 std::function<std::shared_ptr<processable_type>()> m_get_payload_item;
	 std::function<void(std::shared_ptr<processable_type>)> m_put_payload_item;

	 const std::chrono::seconds m_ping_interval{GBEASTCONSUMERPINGINTERVAL}; // Time between two pings in seconds
	 boost::asio::steady_timer m_timer;
	 std::atomic<beast_ping_state> m_ping_state{beast_ping_state::CONNECTION_IS_ALIVE};
	 const boost::beast::websocket::ping_data m_ping_data{};

	 GCommandContainerT<processable_type, beast_payload_command> m_command_container{beast_payload_command::NONE, nullptr}; ///< Holds the current command and payload (if any)

	 std::function<bool()> m_check_server_stopped;
	 boost::beast::websocket::close_code m_close_code
		 = boost::beast::websocket::close_code::normal; ///< Holds the close code when terminating the connection

	 std::function<void(bool)> m_server_sign_on;

	 Gem::Common::serializationMode m_serialization_mode = Gem::Common::serializationMode::BINARY;
	 bool m_verbose_control_frames = false;

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
class GWebsocketConsumerT final
	: public Gem::Courtier::GBaseConsumerT<processable_type> // note: GBaseConsumerT<> is non-copyable
	, public std::enable_shared_from_this<GWebsocketConsumerT<processable_type>>
{
	 //-------------------------------------------------------------------------
	 // Simplify usage of namespaces
	 using error_code = boost::system::error_code;

public:
	 //-------------------------------------------------------------------------
	 /** @brief The default constructor */
	 GWebsocketConsumerT() = default;

	 //-------------------------------------------------------------------------
	 // Deleted copy-/move-constructors and assignment operators.
	 GWebsocketConsumerT(const GWebsocketConsumerT<processable_type>&) = delete;
	 GWebsocketConsumerT(GWebsocketConsumerT<processable_type>&&) = delete;
	 GWebsocketConsumerT& operator=(const GWebsocketConsumerT<processable_type>&) = delete;
	 GWebsocketConsumerT& operator=(GWebsocketConsumerT<processable_type>&&) = delete;

protected:
	 //-------------------------------------------------------------------------
	 /**
	  * Stop execution
	  */
	 void shutdown_() override {
		 //------------------------------------------------------
		 // Set the class-wide shutdown-flag
		 GBaseConsumerT<processable_type>::shutdown_();

		 //------------------------------------------------------
		 // Wait for context threads to finish
		 for (auto &t: m_context_thread_vec) { t.join(); }
		 m_context_thread_vec.clear();

		 //------------------------------------------------------
	 }

private:
	 //-------------------------------------------------------------------------
	 /**
	  * Adds local command line options to a boost::program_options::options_description object.
	  *
	  * @param visible Command line options that should always be visible
	  * @param hidden Command line options that should only be visible upon request
	  */
	 void addCLOptions_(
		 boost::program_options::options_description &visible
		 , boost::program_options::options_description &hidden
	 ) override {
		 namespace po = boost::program_options;

		 visible.add_options()
			 ("beast_ip", po::value<std::string>(&m_server)->default_value(GCONSUMERDEFAULTSERVER),
				 "\t[beast] The name or ip of the server")
			 ("beast_port", po::value<unsigned short>(&m_port)->default_value(GCONSUMERDEFAULTPORT),
				 "\t[beast] The port of the server");

		 hidden.add_options()
			 ("beast_serializationMode", po::value<Gem::Common::serializationMode>(&m_serializationMode)->default_value(
				 GASIOTCPCONSUMERSERIALIZATIONMODE),
				 "\t[beast] Specifies whether serialization shall be done in TEXTMODE (0), XMLMODE (1) or BINARYMODE (2)")
			 ("beast_nListenerThreads", po::value<std::size_t>(&m_n_listener_threads)->default_value(m_n_listener_threads),
				 "\t[beast] The number of threads used to listen for incoming connections")
			 ("beast_pingInterval", po::value<std::size_t>(&m_ping_interval)->default_value(GBEASTCONSUMERPINGINTERVAL),
			    "\t[beast] The number of seconds between two consecutive pings")
			 ("beast_verboseControlFrames", po::value<bool>(&m_verbose_control_frames)->default_value(false)->implicit_value(true),
			 	 "\t[beast] Whether sending and arrival of ping/pong and receipt of a close frame should be announced by client and server");
	 }

	 //-------------------------------------------------------------------------
	 /**
	  * Takes a boost::program_options::variables_map object and acts on
	  * the received command line options.
	  */
	 void actOnCLOptions_(const boost::program_options::variables_map &vm) override
	 { /* nothing */ }

	 //-------------------------------------------------------------------------
	 /**
	  * A unique identifier for a given consumer
	  *
	  * @return A unique identifier for a given consumer
	  */
	 std::string getConsumerName_() const override {
		 return std::string("GWebsocketConsumerT");
	 }

	 //-------------------------------------------------------------------------
	 /**
	  * Returns a short identifier for this consumer
	  */
	 std::string getMnemonic_() const override {
		 return std::string("beast");
	 }

	 //-------------------------------------------------------------------------
	 /**
	  * Starts the consumer responder loops
	  */
	 void async_startProcessing_() override {
		 boost::system::error_code ec;

		 // Set up the endpoint according to the endpoint information we have received from the command line
		 m_endpoint = std::move(boost::asio::ip::tcp::endpoint{boost::asio::ip::make_address(m_server), m_port});

		 // Open the acceptor
		 m_acceptor.open(m_endpoint.protocol(), ec);
		 if(ec || !m_acceptor.is_open()) {
			 if(ec) {
				 glogger
					 << "GWebsocketConsumerT<>::async_startProcessing_() / m_acceptor.open: Got error message \"" << ec.message() << "\"" << std::endl
					 << "No connections will be accepted. The server is not running" << std::endl
					 << GWARNING;
			 } else {
				 glogger
					 << "GWebsocketConsumerT<>::async_startProcessing_() / m_acceptor.open did not succeed." << std::endl
					 << "No connections will be accepted. The server is not running" << std::endl
					 << GWARNING;
			 }

			 return;
		 }

		 // Bind to the server address
		 m_acceptor.bind(m_endpoint, ec);
		 if(ec) {
			 glogger
			    << "GWebsocketConsumerT<>::async_startProcessing_() / m_acceptor.bind: Got error message \"" << ec.message() << "\"" << std::endl
				 << "No connections will be accepted. The server is not running" << std::endl
			    << GWARNING;
			 return;
		 }

		 // Start listening for connections
		 m_acceptor.listen(boost::asio::socket_base::max_listen_connections, ec);
		 if(ec) {
			 glogger
				 << "GWebsocketConsumerT<>::async_startProcessing_() / m_acceptor.listen: Got error message \"" << ec.message() << "\"" << std::endl
				 << "No connections will be accepted. The server is not running" << std::endl
				 << GWARNING;
			 return;
		 }

		 // Start accepting connections
		 async_start_accept();

		 // Allow to serve requests from multiple threads
		 m_context_thread_vec.reserve(m_n_listener_threads);
		 for(std::size_t t_cnt=0; t_cnt<m_n_listener_threads; t_cnt++) {
			 m_context_thread_vec.emplace_back(
				 std::thread(
					 [this](){
						 this->m_io_context.run();
					 }
				 )
			 );
		 }

		 // Done -- the function will return immediately
	 }

	 //-------------------------------------------------------------------------
	 /**
	  * Asynchronously accepts new sessions requests (on the ASIO- and not the
	  * Websocket-level).
	  */
	 void async_start_accept(){
		 auto self = this->shared_from_this();
		 m_acceptor.async_accept(
			 m_socket
			 , [self](boost::system::error_code ec) {
				 self->when_accepted(ec);
			 }
		 );
	 }

	 //-------------------------------------------------------------------------
	 /**
	  * This callback will be executed when a new session has been accepted
	  *
	  * @param ec The code of a possible error
	  */
	 void when_accepted(error_code ec) {
		 if(this->stopped()) return;

		 if(ec) {
			 glogger
			 	<< "In GWebsocketConsumerT<>::when_accepted(): Got error code \"" << ec.message() << "\"" << std::endl
				<< "We will nevertheless try to accept more connections" << std::endl
				<< GWARNING;
		 } else {
			 // Create the GWebsocketConsumerT<processable_type>_session and async_start_run it. This call will return immediately.
			 std::make_shared<GWebsocketServerSessionT<processable_type>>(
				 std::move(m_socket) // m_socket will stay in a valid state
				 , [this]() -> std::shared_ptr<processable_type> { return this->getPayloadItem(); }
				 , [this](std::shared_ptr<processable_type> p) { this->putPayloadItem(p); }
				 , [this]() -> bool { return this->stopped(); }
				 , [this](bool sign_on) {
					 if(true==sign_on) {
						 this->m_n_active_sessions++;
					 } else {
						 if(0 == this->m_n_active_sessions) {
							 throw gemfony_exception(
								 g_error_streamer(DO_LOG,  time_and_place)
									 << "In GWebsocketConsumerT<>::when_accepted():" << std::endl
							       << "Tried to decrement #sessions which is already 0" << std::endl
							 );
						 } else {
							 // This won't help, though, if m_n_active_sessions becomes 0 after the if-check
							 this->m_n_active_sessions--;
						 }
					 }

					 glogger
						 << "GWebsocketConsumerT<>: " << this->m_n_active_sessions << " active sessions" << std::endl
						 << GLOGGING;
				 }
				 , m_ping_interval
				 , m_verbose_control_frames
			 )->async_start_run();
		 }

		 // Accept another connection
		 if(!this->stopped()) async_start_accept();
	 }

	 //-------------------------------------------------------------------------
	 /**
	  * Tries to retrieve a work item from the server, observing a timeout
	  *
	  * @return A work item (possibly empty)
	  */
	 std::shared_ptr<processable_type> getPayloadItem() {
		 std::shared_ptr<processable_type> p;

		 // Try to retrieve a work item from the broker
		 m_broker_ptr->get(p, m_timeout);

		 // May be empty, if we ran into a timeout
		 return p;
	 }

	 //-------------------------------------------------------------------------
	 /**
	  * Submits a work item to the server, observing a timeout
	  */
	 void putPayloadItem(std::shared_ptr<processable_type> p) {
		 if(!p) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG,  time_and_place)
					 << "GWebsocketConsumerT<>::putPayloadItem():" << std::endl
					 << "Function called with empty work item" << std::endl
			 );
		 }

		 if(!m_broker_ptr->put(p, m_timeout)) {
			 glogger
			 	<< "In GWebsocketConsumerT<>::putPayloadItem():" << std::endl
				<< "Work item could not be submitted to the broker" << std::endl
				<< "The item will be discarded" << std::endl
				<< GWARNING;
		 }
	 }


	 //-------------------------------------------------------------------------
	 /**
	  * This function returns a client associated with this consumer. By default
	  * it returns an empty smart pointer, so that consumers without the need for
	  * clients do not need to re-implement this function.
	  */
	 std::shared_ptr<GBaseClientT<processable_type>> getClient_() const override {
		 return std::shared_ptr<GBaseClientT<processable_type>>(
			 new GWebsocketClientT<processable_type>(
				 m_server
				 , m_port
				 , m_serializationMode
				 , m_verbose_control_frames
			 )
		 );
	 }

	 //-------------------------------------------------------------------------
	 /**
	  * Allows to check whether this consumer needs a client to operate.
	  *
	  * @return A boolean indicating whether this consumer needs a client to operate
	  */
	 bool needsClient_() const noexcept override {
		 return true;
	 }

	 //-------------------------------------------------------------------------
	 /**
	  * Returns the (possibly estimated) number of concurrent processing units.
	  * Note that this function does not make any assumptions whether processing
	  * units are dedicated solely to a given task.
	  */
	 std::size_t getNProcessingUnitsEstimate_(bool& exact) const override {
		 exact=false; // mark the answer as approximate
		 return m_n_active_sessions.load();
	 }

	 //-------------------------------------------------------------------------
	 /**
	  * Returns an indication whether full return can be expected from this
	  * consumer. Since evaluation is performed remotely, we assume that this
	  * is not the case.
	  */
	 bool capableOfFullReturn_() const override {
		 return false;
	 }

	 //-------------------------------------------------------------------------
	 // Data

	 std::string m_server = GCONSUMERDEFAULTSERVER;  ///< The name or ip if the server
	 unsigned short m_port = GCONSUMERDEFAULTPORT; ///< The port on which the server is supposed to listen
	 boost::asio::ip::tcp::endpoint m_endpoint{boost::asio::ip::make_address(m_server), m_port};
	 std::size_t m_n_listener_threads = Gem::Common::getNHardwareThreads(GCONSUMERLISTENERTHREADS);  ///< The number of threads used to listen for incoming connections through io_context::run()
	 boost::asio::io_context m_io_context{boost::numeric_cast<int>(m_n_listener_threads)};
	 boost::asio::ip::tcp::acceptor m_acceptor{m_io_context};
	 boost::asio::ip::tcp::socket m_socket{m_io_context};
	 Gem::Common::serializationMode m_serializationMode = Gem::Common::serializationMode::BINARY; ///< Specifies the serialization mode
	 std::vector<std::thread> m_context_thread_vec;
	 std::atomic<std::size_t> m_n_active_sessions{0};
	 std::size_t m_ping_interval = GBEASTCONSUMERPINGINTERVAL;
	 bool m_verbose_control_frames = false; ///< Whether the control_callback should emit information when a control frame is received

	 std::shared_ptr<GBrokerT<processable_type>> m_broker_ptr = GBROKER(processable_type); ///< Simplified access to the broker
	 const std::chrono::duration<double> m_timeout = std::chrono::milliseconds(GBEASTMSTIMEOUT); ///< A timeout for put- and get-operations via the broker

	 //-------------------------------------------------------------------------
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Courtier */
} /* namespace Gem */

#endif /* GENEVA_LIBRARY_COLLECTION_GWEBSOCKETCONSUMERT_HPP_ */
