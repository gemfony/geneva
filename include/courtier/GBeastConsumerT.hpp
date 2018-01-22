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
		 switch(serMode) {
			 case Gem::Common::serializationMode::TEXT: {
				 std::stringstream(std::ios::out).swap(m_stringstream);
				 boost::archive::text_oarchive oa(m_stringstream);
				 oa << boost::serialization::make_nvp("command_container", *this);
			 } break; // archive and stream closed at end of scope

			 case Gem::Common::serializationMode::XML: {
				 std::stringstream(std::ios::out).swap(m_stringstream);
				 boost::archive::xml_oarchive oa(m_stringstream);
				 oa << boost::serialization::make_nvp("command_container", *this);
			 } break;

			 case Gem::Common::serializationMode::BINARY: {
				 std::stringstream(std::ios::out | std::ios::binary).swap(m_stringstream);
				 boost::archive::binary_oarchive oa(m_stringstream);
				 oa << boost::serialization::make_nvp("command_container", *this);
			 } break;
		 }

		 return m_stringstream.str();
	 }

	 //-------------------------------------------------------------------------
	 /**
	  * Loads a serialized version of an instantiation of this class into this object
	  */
	 void from_string(const std::string& descr, Gem::Common::serializationMode serMode) {
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

class async_websocket_client final
	: public std::enable_shared_from_this<async_websocket_client>
{
	 //--------------------------------------------------------------
	 // Make the code easier to read

	 using error_code = boost::system::error_code;
	 using resolver = boost::asio::ip::tcp::resolver;
	 using socket = boost::asio::ip::tcp::socket;
	 using close_code = boost::beast::websocket::close_code;
	 using frame_type = boost::beast::websocket::frame_type;
	 using string_view = boost::beast::string_view;

public:
	 //--------------------------------------------------------------
	 // External "API"

	 // Initialization with host/ip and port
	 async_websocket_client(const std::string&, unsigned short);
	 // Starts the main async_start_run-loop
	 void run();

	 //--------------------------------------------------------------

	 // Deleted default-constructor -- enforce usage of a particular constructor
	 async_websocket_client() = delete;
	 // Deleted copy-constructors and assignment operators -- the client is non-copyable
	 async_websocket_client(const async_websocket_client&) = delete;
	 async_websocket_client(async_websocket_client&&) = delete;
	 async_websocket_client& operator=(const async_websocket_client&) = delete;
	 async_websocket_client& operator=(async_websocket_client&&) = delete;

private:
	 //--------------------------------------------------------------
	 // Communication and processing

	 void async_start_write(const std::string&);
	 void async_start_read();

	 void when_resolved(error_code, const resolver::results_type&);
	 void when_connected(error_code);
	 void when_handshake_complete(error_code);
	 void when_written(error_code, std::size_t);
	 void when_read(error_code, std::size_t);

	 std::string process_request();

	 void do_close(boost::beast::websocket::close_code);

	 /** @brief Callback for control frames */
	 std::function<void(boost::beast::websocket::frame_type, boost::beast::string_view)> f_when_control_frame_arrived;

	 //--------------------------------------------------------------
	 // Data

	 boost::asio::io_context m_io_context;
	 resolver m_resolver{m_io_context};
	 boost::beast::websocket::stream<socket> m_ws{m_io_context};

	 std::string m_address;
	 unsigned int m_port;

	 boost::beast::multi_buffer m_incoming_buffer;
	 std::string m_outgoing_message;

	 std::random_device m_nondet_rng; ///< Source of non-deterministic random numbers
	 std::mt19937 m_rng_engine{m_nondet_rng()}; ///< The actual random number engine, seeded my m_nondet_rng

	 boost::beast::websocket::close_code m_close_code
		 = boost::beast::websocket::close_code::normal; ///< Holds the close code when terminating the connection

	 command_container m_command_container{payload_command::NONE, nullptr}; ///< Holds the current command and payload (if any)

	 //--------------------------------------------------------------
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
