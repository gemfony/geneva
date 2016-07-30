/**
 * @file GAsioTCPConsumerT.hpp
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
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/utility.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/fusion/include/tuple.hpp>
#include <boost/fusion/include/boost_tuple.hpp>
#include <boost/fusion/adapted/boost_tuple.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/io.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/qi_lit.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>

#ifndef GASIOTCPCONSUMERT_HPP_
#define GASIOTCPCONSUMERT_HPP_

// Geneva headers go here
#include "common/GThreadGroup.hpp"
#include "common/GThreadPool.hpp"
#include "common/GSerializationHelperFunctionsT.hpp"
#include "common/GHelperFunctions.hpp"
#include "common/GHelperFunctionsT.hpp"
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
const std::uint32_t GASIOTCPCONSUMERMAXSTALLS = 10;
const std::uint32_t GASIOTCPCONSUMERMAXCONNECTIONATTEMPTS = 10;
const unsigned short GASIOTCPCONSUMERDEFAULTPORT = 10000;
const std::string GASIOTCPCONSUMERDEFAULTSERVER = "localhost";
const std::uint16_t GASIOTCPCONSUMERTHREADS = 4;
const Gem::Common::serializationMode GASIOTCPCONSUMERSERIALIZATIONMODE = Gem::Common::serializationMode::SERIALIZATIONMODE_BINARY;
const bool GASIOTCPCONSUMERRETURNREGARDLESS = true;

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * Forward declaration
 */
template<typename processable_type>
class GAsioTCPConsumerT;

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class is responsible for the client side of network communication
 * with Boost::Asio. Note that this class is noncopyable, as it is derived
 * from a non-copyable base class.
 */
template<typename processable_type>
class GAsioTCPClientT
	: public Gem::Courtier::GBaseClientT<processable_type> {
public:
	/***************************************************************************/
	/**
	 * Initialization by server name/ip and port
	 *
	 * @param server Identifies the server
	 * @param port Identifies the port on the server
	 */
	GAsioTCPClientT(const std::string &server, const std::string &port)
		: GBaseClientT<processable_type>()
	   , m_maxStalls(GASIOTCPCONSUMERMAXSTALLS)
	   , m_maxConnectionAttempts(GASIOTCPCONSUMERMAXCONNECTIONATTEMPTS)
	  	, m_totalConnectionAttempts(0)
	   , m_stalls(0)
	   , m_io_service()
	   , m_socket(m_io_service)
	   , m_resolver(m_io_service)
	   , m_query(server, port)
	   , m_endpoint_iterator0(m_resolver.resolve(m_query))
	   , m_end()
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
	GAsioTCPClientT(
		const std::string &server
		, const std::string &port
		, std::shared_ptr<processable_type> additionalDataTemplate
	)
		: GBaseClientT<processable_type>(additionalDataTemplate)
	   , m_maxStalls(GASIOTCPCONSUMERMAXSTALLS)
	   , m_maxConnectionAttempts(GASIOTCPCONSUMERMAXCONNECTIONATTEMPTS)
	   , m_totalConnectionAttempts(0), m_stalls(0)
	   , m_io_service()
	   , m_socket(m_io_service)
	   , m_resolver(m_io_service)
	   , m_query(server, port)
	   , m_endpoint_iterator0(m_resolver.resolve(m_query))
	   , m_end()
	{
		m_tmpBuffer = new char[Gem::Courtier::COMMANDLENGTH];
	}

	/***************************************************************************/
	/**
	 * The standard destructor.
	 */
	virtual ~GAsioTCPClientT() {
		Gem::Common::g_array_delete(m_tmpBuffer);

		glogger
		<< "In GAsioTCPClinetT<>::~GAsioTCPClientT():" << std::endl
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
	 * Parses an in-bound "idle" command string, so we know how long the client
	 * should wait before reconnecting to the server. The idle command will be
	 * of the type "idle(5000)", where the number specifies the amount of
	 * milliseconds the client should wait before reconnecting.
	 */
	bool parseIdleCommand(std::uint32_t &idleTime, const std::string &idleCommand) {
		using boost::spirit::ascii::space;
		using boost::spirit::qi::phrase_parse;
		using boost::spirit::qi::uint_;
		using boost::spirit::qi::lit;
		using boost::spirit::ascii::string;
		using boost::spirit::lexeme;
		using boost::spirit::qi::attr;

		using boost::spirit::qi::raw;
		using boost::spirit::qi::alpha;
		using boost::spirit::qi::alnum;
		using boost::spirit::qi::hold;

		using boost::spirit::qi::_1;

		bool success = false;

		std::string::const_iterator from = idleCommand.begin();
		std::string::const_iterator to = idleCommand.end();

		success = phrase_parse(
			from, to, (lit("idle") >> '(' >> uint_ >> ')')[boost::phoenix::ref(idleTime) = _1], space
		);

		if (!success || from != to) {
			return false;
		} else {
			return true;
		}
	}

	/***************************************************************************/
	/**
	 * Retrieve work items from the server.
	 *
	 * @param item Holds the string representation of the work item, if successful
	 * @return true if operation should be continued, otherwise false
	 */
	bool retrieve(
		std::string &item, std::string &serMode, std::string &portId
	) {
		item = "empty"; // Indicates that no item could be retrieved
		std::uint32_t idleTime = 0; // Holds information on the idle time in milliseconds, if "idle" command is received

		try {
			// Try to make a connection
			if (!tryConnect()) {
				glogger
				<< "In GAsioTCPClientT<processable_type>::retrieve(): Warning" << std::endl
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


			// Not currently getting here

			// Remove all leading or trailing white spaces from the command
			std::string inboundCommandString = boost::algorithm::trim_copy(
				std::string(m_tmpBuffer, Gem::Courtier::COMMANDLENGTH));

			// Act on the command
			if ("compute" == inboundCommandString) {
				// We have likely received data. Let's find out how big it is
				boost::asio::read(m_socket, boost::asio::buffer(m_tmpBuffer, Gem::Courtier::COMMANDLENGTH));
				std::string inboundHeader = boost::algorithm::trim_copy(std::string(m_tmpBuffer, Gem::Courtier::COMMANDLENGTH));
				std::size_t dataSize = boost::lexical_cast<std::size_t>(inboundHeader);

				// Now retrieve the serialization mode that was used
				boost::asio::read(m_socket, boost::asio::buffer(m_tmpBuffer, Gem::Courtier::COMMANDLENGTH));
				serMode = boost::algorithm::trim_copy(std::string(m_tmpBuffer, Gem::Courtier::COMMANDLENGTH));

				// Retrieve the port id
				boost::asio::read(m_socket, boost::asio::buffer(m_tmpBuffer, Gem::Courtier::COMMANDLENGTH));
				portId = boost::algorithm::trim_copy(std::string(m_tmpBuffer, Gem::Courtier::COMMANDLENGTH));

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
				// We will usually only allow a given number of timeouts / stalls
				if (m_maxStalls && (m_stalls++ > m_maxStalls)) {
					glogger
					<< "In GAsioTCPClientT<processable_type>::retrieve(): Warning!" << std::endl
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
				std::chrono::milliseconds timeout(idleTime);

				// Indicate that we want to continue
				return CLIENT_CONTINUE;
			} else { // Received unknown command
				glogger
				<< "In GAsioTCPClientT<processable_type>::retrieve(): Warning!" << std::endl
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
		catch (boost::system::system_error &) {
			{ // Make sure we do not hide the next error declaration (avoid a warning message)
				glogger
				<< "In GAsioTCPClientT<processable_type>::retrieve():" << std::endl
				<< "Caught boost::system::system_error exception." << std::endl
				<< "This is likely normal and due to a server shutdown." << std::endl
				<< "Leaving now." << std::endl
				<< GWARNING;
			}

			// Make sure we don't leave any open sockets lying around.
			disconnect(m_socket);

			// Indicate that we don't want to continue
			return CLIENT_TERMINATE;
		}

		// This part of the function should never be reached. Let the audience know, then terminate.
		glogger
		<< "In GAsioTCPClientT<processable_type>::retrieve(): Error!" << std::endl
		<< "We are in a part of the function that should never have been reached!" << std::endl
		<< GEXCEPTION;

		return CLIENT_TERMINATE; // Make the compiler happy
	}

	/***************************************************************************/
	/**
	 * Submit processed items to the server.
	 *
	 * @param item String to be submitted to the server
	 * @param portid The port id of the individual to be submitted
	 * @return true if operation was successful, otherwise false
	 */
	bool submit(const std::string &item, const std::string &portid) {
		// Let's assemble an appropriate buffer
		std::vector<boost::asio::const_buffer> buffers;
		std::string result = assembleQueryString("result", Gem::Courtier::COMMANDLENGTH); // The command
		buffers.push_back(boost::asio::buffer(result));

		// Assemble a buffer for the port id
		std::string portidString = assembleQueryString(portid, Gem::Courtier::COMMANDLENGTH);
		buffers.push_back(boost::asio::buffer(portidString));

		// Assemble the size header
		std::string sizeHeader = assembleQueryString(boost::lexical_cast<std::string>(item.size()),
																	Gem::Courtier::COMMANDLENGTH);
		buffers.push_back(boost::asio::buffer(sizeHeader));

		// Finally take care of the data section.
		buffers.push_back(boost::asio::buffer(item));

		try {
			// Try to make a connection
			if (!tryConnect()) {
				glogger
				<< "In GAsioTCPClientT<processable_type>::submit(): Warning" << std::endl
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
				<< "In GAsioTCPClientT<processable_type>::submit():" << std::endl
				<< "Caught boost::system::system_error exception." << std::endl
				<< "This is likely normal and due to a server shutdown." << std::endl
				<< "Leaving now." << std::endl
				<< GLOGGING;
			}

			// Make sure we don't leave any open sockets lying around.
			disconnect(m_socket);

			// Indicate that we don't want to continue
			return CLIENT_TERMINATE;
		}

		// This part of the function should never be reached. Let the audience know, then terminate.
		glogger
		<< "In GAsioTCPClientT<processable_type>::submit() :" << std::endl
		<< "In a part of the function that should never have been reached!" << std::endl
		<< GEXCEPTION;

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
		// Try to make a connection, at max m_maxConnectionAttempts times
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

	std::uint32_t m_maxStalls; ///< The maximum allowed number of stalled connection attempts
	std::uint32_t m_maxConnectionAttempts; ///< The maximum allowed number of failed connection attempts
	std::uint32_t m_totalConnectionAttempts; ///< The total number of failed connection attempts during program execution

	std::uint32_t m_stalls; ///< counter for stalled connection attempts

	char *m_tmpBuffer;

	boost::asio::io_service m_io_service; ///< Holds the Boost::ASIO::io_service object
	boost::asio::ip::tcp::socket m_socket; ///< The underlying socket

	boost::asio::ip::tcp::resolver m_resolver; ///< Responsible for name resolution
	boost::asio::ip::tcp::resolver::query m_query; ///< A query

	boost::asio::ip::tcp::resolver::iterator m_endpoint_iterator0; ///< start of iteration
	boost::asio::ip::tcp::resolver::iterator m_end; ///< end for end point iterator
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * An instance of this class is created for each new connection initiated
 * by the client. All the details of the data exchange between server
 * and client are implemented here. The class is declared in the same
 * file as the GAsioTCPConsumer in order to avoid cross referencing of
 * header files.
 */
template<typename processable_type>
class GAsioServerSessionT
	: public std::enable_shared_from_this<GAsioServerSessionT<processable_type>>
	, private boost::noncopyable
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
		boost::asio::io_service &io_service
		, const Gem::Common::serializationMode &serMod
		, GAsioTCPConsumerT<processable_type> *master
	)
		: m_strand(io_service), m_socket(io_service), m_bytesTransferredDataBody(0)
	   , m_dataBody_ptr(new std::string()) // An empty string
		, m_portId(Gem::Common::PORTIDTYPE(0))
	   , m_dataSize(0)
	   , m_serializationMode(serMod)
	   , m_master(master)
	   , m_broker_ptr(master->m_broker_ptr)
	   , m_timeout(std::chrono::milliseconds(10))
	   , m_brokerRetrieveMaxRetries(10)
	   , m_noDataClientSleepMilliSeconds(50)
	{ /* nothing */ }

	/***************************************************************************/
	/**
	 * A standard destructor. Shuts down and closes the socket. Note: Non-virtual.
	 */
	~GAsioServerSessionT()
	{ /* nothing */ }

	/***************************************************************************/
	/**
	 * This function processes an individual request from a client.
	 */
	void async_processRequest() {
		try {
			// Initiate the first read session. Every transmission starts with a command
			auto p = GAsioServerSessionT<processable_type>::shared_from_this();
			boost::asio::async_read(
				m_socket
				, boost::asio::buffer(m_commandBuffer)
				, m_strand.wrap(
					std::bind(
						&GAsioServerSessionT<processable_type>::async_handle_read_command
						, GAsioServerSessionT<processable_type>::shared_from_this()
						, std::placeholders::_1 // Replaces boost::asio::placeholders::error
					)
				)
			);
		} catch (const boost::system::system_error &e) {
			glogger
			<< "In GAsioServerSessionT::async_processRequest():" << std::endl
			<< "Caught boost::system::system_error exception with messages:" << std::endl
			<< e.what() << std::endl
			<< GEXCEPTION;
		} catch (const boost::exception &e) {
			glogger
			<< "In GAsioServerSessionT::async_processRequest():" << std::endl
			<< "Caught boost::exception exception with messages:" << std::endl
			<< boost::diagnostic_information(e) << std::endl
			<< GEXCEPTION;
		} catch (...) {
			glogger
			<< "In GAsioServerSessionT::async_processRequest():" << std::endl
			<< "Caught unknown exception" << std::endl
			<< GEXCEPTION;
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
			<< "In GAsioServerSessionT<processable_type>::async_handle_read_command():!" << std::endl
			<< "Received boost::system::error_code " << error << std::endl
			<< "with message " << error.message() << std::endl
			<< GEXCEPTION;
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
			<< "In GAsioServerSessionT<processable_type>::async_handle_read_command(): Warning!" << std::endl
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
	 * with the port id, the data size header and the actual data body
	 */
	void async_retrieveFromRemote() {
		try {
			// Initiate the next read session. Note that the requested data is returned
			// in its entirety, not in chunks. async_async_handle_read_portid is only called
			// once the portId has been fully read.
			boost::asio::async_read(
				m_socket, boost::asio::buffer(m_commandBuffer)
				, m_strand.wrap(
					std::bind(
						&GAsioServerSessionT<processable_type>::async_handle_read_portid
						, GAsioServerSessionT<processable_type>::shared_from_this()
						, std::placeholders::_1 // Replaces boost::asio::placeholders::error
					)
				)
			);
		} catch (const boost::system::system_error &e) {
			glogger
			<< "In GAsioServerSessionT::async_retrieveFromRemote():" << std::endl
			<< "Caught boost::system::system_error exception with messages:" << std::endl
			<< e.what() << std::endl
			<< GEXCEPTION;
		} catch (const boost::exception &e) {
			glogger
			<< "In GAsioServerSessionT::async_retrieveFromRemote():" << std::endl
			<< "Caught boost::exception exception with messages:" << std::endl
			<< boost::diagnostic_information(e) << std::endl
			<< GEXCEPTION;
		} catch (...) {
			glogger
			<< "In GAsioServerSessionT::async_retrieveFromRemote():" << std::endl
			<< "Caught unknown exception" << std::endl
			<< GEXCEPTION;
		}
	}

	/***************************************************************************/
	/**
	 * A routine to be called when all data of the port id has been read
	 */
	void async_handle_read_portid(const boost::system::error_code &error) {
		if (error) {
			glogger
			<< "In GAsioServerSessionT<processable_type>::async_handle_read_portid():" << std::endl
			<< "Received error " << error << std::endl
			<< GEXCEPTION;
			return;
		}

		//------------------------------------------------------------------------
		// Deal with the data itself

		// Remove white spaces
		std::string portId_str
			= boost::algorithm::trim_copy(std::string(m_commandBuffer.data(), Gem::Courtier::COMMANDLENGTH));

		m_portId = boost::lexical_cast<Gem::Common::PORTIDTYPE>(portId_str);

		//------------------------------------------------------------------------
		// Initiate the next read session, this time dealing with the data size
		try {
			boost::asio::async_read(
				m_socket, boost::asio::buffer(m_commandBuffer), m_strand.wrap(std::bind(
					&GAsioServerSessionT<processable_type>::async_handle_read_datasize,
					GAsioServerSessionT<processable_type>::shared_from_this(),
					std::placeholders::_1 // Replaces boost::asio::placeholders::error
				))
			);
		} catch (const boost::system::system_error &e) {
			glogger
			<< "In GAsioServerSessionT::async_handle_read_portid():" << std::endl
			<< "Caught boost::system::system_error exception with messages:" << std::endl
			<< e.what() << std::endl
			<< GEXCEPTION;
		} catch (const boost::exception &e) {
			glogger
			<< "In GAsioServerSessionT::async_handle_read_portid():" << std::endl
			<< "Caught boost::exception exception with messages:" << std::endl
			<< boost::diagnostic_information(e) << std::endl
			<< GEXCEPTION;
		} catch (...) {
			glogger
			<< "In GAsioServerSessionT::async_handle_read_portid():" << std::endl
			<< "Caught unknown exception" << std::endl
			<< GEXCEPTION;
		}

		//------------------------------------------------------------------------
	}

	/***************************************************************************/
	/**
	 * A routine to be called when all data of the data size header has been read
	 */
	void async_handle_read_datasize(const boost::system::error_code &error) {
		if (error) {
			glogger
			<< "In GAsioServerSessionT<processable_type>::async_handle_read_datasize(): Warning!" << std::endl
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
				boost::asio::buffer(m_dataBuffer), m_strand.wrap(std::bind(
					&GAsioServerSessionT<processable_type>::async_handle_read_body,
					GAsioServerSessionT<processable_type>::shared_from_this(),
					std::placeholders::_1 // Replaces boost::asio::placeholders::error
					, std::placeholders::_2 // Replaces boost::asio::placeholders::bytes_transferred
				))
			);
		} catch (const boost::system::system_error &e) {
			glogger
			<< "In GAsioServerSessionT::async_handle_read_datasize():" << std::endl
			<< "Caught boost::system::system_error exception with messages:" << std::endl
			<< e.what() << std::endl
			<< GEXCEPTION;
		} catch (const boost::exception &e) {
			glogger
			<< "In GAsioServerSessionT::async_handle_read_datasize():" << std::endl
			<< "Caught boost::exception exception with messages:" << std::endl
			<< boost::diagnostic_information(e) << std::endl
			<< GEXCEPTION;
		} catch (...) {
			glogger
			<< "In GAsioServerSessionT::async_handle_read_datasize():" << std::endl
			<< "Caught unknown exception" << std::endl
			<< GEXCEPTION;
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
			<< "In GAsioServerSessionT<processable_type>::async_handle_read_body(): Warning!" << std::endl
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
					boost::asio::buffer(m_dataBuffer), m_strand.wrap(std::bind(
						&GAsioServerSessionT<processable_type>::async_handle_read_body,
						GAsioServerSessionT<processable_type>::shared_from_this(),
						std::placeholders::_1 // Replaces boost::asio::placeholders::error
						, std::placeholders::_2 // Replaces boost::asio::placeholders::bytes_transferred
					))
				);
			} catch (const boost::system::system_error &e) {
				glogger
				<< "In GAsioServerSessionT::async_handle_read_body():" << std::endl
				<< "Caught boost::system::system_error exception with messages:" << std::endl
				<< e.what() << std::endl
				<< GEXCEPTION;
			} catch (const boost::exception &e) {
				glogger
				<< "In GAsioServerSessionT::async_handle_read_body():" << std::endl
				<< "Caught boost::exception exception with messages:" << std::endl
				<< boost::diagnostic_information(e) << std::endl
				<< GEXCEPTION;
			} catch (...) {
				glogger
				<< "In GAsioServerSessionT::async_handle_read_body():" << std::endl
				<< "Caught unknown exception" << std::endl
				<< GEXCEPTION;
			}
		} else { // The transfer is complete, work with the results
			// Disconnect the socket
			disconnect(m_socket);

			// ... and schedule the work item for de-serialization
			m_master->async_scheduleDeSerialization(
				m_dataBody_ptr, m_portId
			);
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
		Gem::Common::PORTIDTYPE portId;

		// Do not do anything if the server was stopped
		if (m_master->stopped()) {
			// Disconnect the socket
			disconnect(m_socket);
			return;
		}

		// Retrieve an item. Terminate the submission if
		// no item could be retrieved
		std::size_t nRetries = 0;

		while (!m_broker_ptr->get(portId, p, m_timeout)) {
			if (++nRetries > m_brokerRetrieveMaxRetries) {
				std::string idleCommand
					= std::string("idle(") + boost::lexical_cast<std::string>(m_noDataClientSleepMilliSeconds) +
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
			= assembleQueryString(boost::lexical_cast<std::string>(item.size()), Gem::Courtier::COMMANDLENGTH);

		// Format a header for the serialization mode
		std::string serialization_header = assembleQueryString(
			boost::lexical_cast<std::string>(m_serializationMode), Gem::Courtier::COMMANDLENGTH
		);

		// Format the portId header
		std::string portid_header = assembleQueryString(
			boost::lexical_cast<std::string>(portId), Gem::Courtier::COMMANDLENGTH
		);

		// Assemble the data buffers
		std::vector<boost::asio::const_buffer> buffers;

		buffers.push_back(boost::asio::buffer(outbound_command_header));
		buffers.push_back(boost::asio::buffer(outbound_size_header));
		buffers.push_back(boost::asio::buffer(serialization_header));
		buffers.push_back(boost::asio::buffer(portid_header));
		buffers.push_back(boost::asio::buffer(item));

		// Write the serialized data to the socket. We use "gather-write" to send
		// command, header and data in a single write operation.
		try {
			boost::asio::async_write(
				m_socket, buffers, m_strand.wrap(std::bind(
					&GAsioServerSessionT<processable_type>::handle_write,
					GAsioServerSessionT<processable_type>::shared_from_this(),
					std::placeholders::_1 // Replaces boost::asio::placeholders::error
				))
			);
		} catch (const boost::system::system_error &e) {
			glogger
			<< "In GAsioServerSessionT::async_submitToRemote():" << std::endl
			<< "Caught boost::system::system_error exception with messages:" << std::endl
			<< e.what() << std::endl
			<< GEXCEPTION;
		} catch (const boost::exception &e) {
			glogger
			<< "In GAsioServerSessionT::async_submitToRemote():" << std::endl
			<< "Caught boost::exception exception with messages:" << std::endl
			<< boost::diagnostic_information(e) << std::endl
			<< GEXCEPTION;
		} catch (...) {
			glogger
			<< "In GAsioServerSessionT::async_submitToRemote():" << std::endl
			<< "Caught unknown exception" << std::endl
			<< GEXCEPTION;
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
					&GAsioServerSessionT<processable_type>::handle_write,
					GAsioServerSessionT<processable_type>::shared_from_this(),
					std::placeholders::_1 // Replaces boost::asio::placeholders::error
				))
			);
		} catch (const boost::system::system_error &e) {
			glogger
			<< "In GAsioServerSessionT::async_sendSingleCommand():" << std::endl
			<< "Caught boost::system::system_error exception with messages:" << std::endl
			<< e.what() << std::endl
			<< GEXCEPTION;
		} catch (const boost::exception &e) {
			glogger
			<< "In GAsioServerSessionT::async_sendSingleCommand():" << std::endl
			<< "Caught boost::exception exception with messages:" << std::endl
			<< boost::diagnostic_information(e) << std::endl
			<< GEXCEPTION;
		} catch (...) {
			glogger
			<< "In GAsioServerSessionT::async_sendSingleCommand():" << std::endl
			<< "Caught unknown exception" << std::endl
			<< GEXCEPTION;
		}
	}

	/***************************************************************************/
	/**
	 * A routine to be called when all data has been written
	 */
	void handle_write(const boost::system::error_code &error) {
		if (error) {
			glogger
			<< "In GAsioServerSessionT<processable_type>::handle_write(): Warning!" << std::endl
			<< "Warning: Received error " << error << std::endl
			<< GWARNING;
			return;
		}

		// Disconnect the socket
		disconnect(m_socket);
	}

private:
	/***************************************************************************/

	GAsioServerSessionT(); ///< Intentionally left undefined
	GAsioServerSessionT(const GAsioServerSessionT<processable_type> &); ///< Intentionally left undefined
	const GAsioServerSessionT<processable_type> &operator=(
		const GAsioServerSessionT<processable_type> &); ///< Intentionally left undefined

	/***************************************************************************/

	boost::asio::io_service::strand m_strand; ///< /// Ensure the connection's handlers are not called concurrently.
	boost::asio::ip::tcp::socket m_socket; ///< The underlying socket

	std::array<char, COMMANDLENGTH> m_commandBuffer; ///< A buffer to be used for command transfers
	std::array<char, 16384> m_dataBuffer;    ///< A buffer holding body data -- was 4096

	std::size_t m_bytesTransferredDataBody; ///< The number of bytes if the data body transferred so far
	std::shared_ptr <std::string> m_dataBody_ptr; ///< The actual body data. Implemented as a shared_ptr so we can easily hand the data around

	Gem::Common::PORTIDTYPE m_portId; ///< The id of a port
	std::size_t m_dataSize; ///< Holds the size of the body of data

	Gem::Common::serializationMode m_serializationMode; ///< Specifies the serialization mode
	GAsioTCPConsumerT<processable_type> *m_master;
	std::shared_ptr <Gem::Courtier::GBrokerT<processable_type>> m_broker_ptr;

	std::chrono::duration<double> m_timeout; ///< A timeout for put- and get-operations

	std::size_t m_brokerRetrieveMaxRetries; ///< The maximum amount
	std::uint32_t m_noDataClientSleepMilliSeconds; ///< The amount of milliseconds the client should sleep when no data could be retrieved from the broker
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * It is the main responsibility of this class to start new server session
 * for each client request.
 */
template<typename processable_type>
class GAsioTCPConsumerT
	: public Gem::Courtier::GBaseConsumerT<processable_type> // note: GBaseConsumerT<> is non-copyable
{
	friend class GAsioServerSessionT<processable_type>;

public:
	/***************************************************************************/
	/**
	 * The default constructor
	 */
	GAsioTCPConsumerT()
		: m_listenerThreads(Gem::Common::getNHardwareThreads(GASIOTCPCONSUMERTHREADS))
		, m_acceptor(m_io_service)
	   , m_serializationMode(Gem::Common::serializationMode::SERIALIZATIONMODE_BINARY)
		, m_maxStalls(GASIOTCPCONSUMERMAXSTALLS)
		, m_maxConnectionAttempts(GASIOTCPCONSUMERMAXCONNECTIONATTEMPTS)
	   , m_returnRegardless(GASIOTCPCONSUMERRETURNREGARDLESS)
  		, m_port(GASIOTCPCONSUMERDEFAULTPORT)
		, m_server(GASIOTCPCONSUMERDEFAULTSERVER)
		, m_timeout(std::chrono::milliseconds(10))
		, m_broker_ptr()
	{ /* noting */ }

	/***************************************************************************/
	/**
	 * A constructor that accepts a number of vital parameters
	 *
	 * @param port The port where the server should wait for new connections
	 * @param listenerThreads The number of threads used to wait for incoming connections
	 * @param sm The desired serialization mode
	 */
	GAsioTCPConsumerT(
		const unsigned short &port
		, const std::size_t &listenerThreads = 0
		, const Gem::Common::serializationMode &sm = Gem::Common::serializationMode::SERIALIZATIONMODE_BINARY
	)
		: m_listenerThreads(listenerThreads > 0 ? listenerThreads : Gem::Common::getNHardwareThreads(GASIOTCPCONSUMERTHREADS))
		, m_acceptor(m_io_service)
	   , m_serializationMode(sm)
		, m_maxStalls(GASIOTCPCONSUMERMAXSTALLS)
		, m_maxConnectionAttempts(GASIOTCPCONSUMERMAXCONNECTIONATTEMPTS)
		, m_returnRegardless(GASIOTCPCONSUMERRETURNREGARDLESS)
		, m_port(port)
		, m_server(GASIOTCPCONSUMERDEFAULTSERVER)
		, m_timeout(std::chrono::milliseconds(10))
		, m_broker_ptr()
	{ /* nothing */ }

	/***************************************************************************/
	/**
	 * A standard destructor
	 */
	virtual ~GAsioTCPConsumerT()
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
	 * Specifies whether results should be returned regardless of the success achieved
	 * in the processing step.
	 *
	 * @param returnRegardless Specifies whether results should be returned to the server regardless of their success
	 */
	void setReturnRegardless(const bool &returnRegardless) {
		m_returnRegardless = returnRegardless;
	}

	/***************************************************************************/
	/**
	 * Checks whether results should be returned regardless of the success achieved
	 * in the processing step.
	 *
	 * @return Whether results should be returned to the server regardless of their success
	 */
	bool getReturnRegardless() const {
		return m_returnRegardless;
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
	virtual bool needsClient() const {
		return true;
	}

	/***************************************************************************/
	/**
	 * Emits a client suitable for processing the data emitted by this consumer
	 */
	virtual std::shared_ptr <GBaseClientT<processable_type>> getClient() const {
		std::shared_ptr <GAsioTCPClientT<processable_type>> p(
			new GAsioTCPClientT<processable_type>(m_server, boost::lexical_cast<std::string>(m_port))
		);

		p->setMaxStalls(m_maxStalls); // Set to 0 to allow an infinite number of stalls
		p->setMaxConnectionAttempts(
			m_maxConnectionAttempts); // Set to 0 to allow an infinite number of failed connection attempts
		p->setReturnRegardless(m_returnRegardless);  // Prevent return of unsuccessful adaption attempts to the server

		return p;
	}

	/***************************************************************************/
	/**
	 * Starts the actual processing loops
	 */
	void async_startProcessing() {
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
			glogger
			<< "In GAsioTCPConsumerT<processable_type>::async_startProcessing(): Error!" << std::endl
			<< "Got empty broker pointer" << std::endl
			<< GEXCEPTION;
		}

		// Set the number of threads in the pool
		if (m_listenerThreads) {
			m_gtp.setNThreads(boost::numeric_cast<unsigned int>(m_listenerThreads));
			std::cout << "GAsioTCPConsumerT: Started acceptor with " << boost::numeric_cast<unsigned int>(m_listenerThreads) << " threads" << std::endl;
		}

		try {
			// Prevent a race condition
			m_work.reset(new boost::asio::io_service::work(m_io_service));

			// Start the first session
			this->async_newAccept();

			// TODO: Can the async call return quickly enough so that the io_service may nevertheless run out of work ?

			// Create a number of threads responsible for the m_io_service objects
			// This absolutely needs to happen after the first session has started,
			// so the io_service doesn't run out of work
			m_gtg.create_threads(
				[&]() { this->m_io_service.run(); } // this-> deals with a problem of g++ 4.7.2
				, m_listenerThreads
			);
		} catch (const boost::system::system_error &e) {
			glogger
			<< "In GAsioTCPConsumerT::async_startProcessing():" << std::endl
			<< "Caught boost::system::system_error exception with messages:" << std::endl
			<< boost::diagnostic_information(e) << std::endl
			<< GEXCEPTION;
		} catch (const boost::exception &e) {
			glogger
			<< "In GAsioTCPConsumerT::async_startProcessing():" << std::endl
			<< "Caught boost::exception with messages:" << std::endl
			<< boost::diagnostic_information(e) << std::endl
			<< GEXCEPTION;
		} catch (const std::exception &e) {
			glogger
			<< "In GAsioTCPConsumerT::async_startProcessing():" << std::endl
			<< "Caught std::exception with messages:" << std::endl
			<< boost::diagnostic_information(e) << std::endl
			<< GEXCEPTION;
		} catch (...) {
			glogger
			<< "In GAsioTCPConsumerT::async_startProcessing():" << std::endl
			<< "Caught unknown exception" << std::endl
			<< GEXCEPTION;
		}
	}

	/***************************************************************************/
	/**
	 * Make sure the consumer and the server sessions shut down gracefully
	 */
	void shutdown() {
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
	virtual std::string getConsumerName() const {
		return std::string("GAsioTCPConsumerT");
	}

	/***************************************************************************/
	/**
	 * Returns a short identifier for this consumer
	 */
	virtual std::string getMnemonic() const {
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

	/***************************************************************************/
	/**
	 * Adds local command line options to a boost::program_options::options_description object.
	 *
	 * @param visible Command line options that should always be visible
	 * @param hidden Command line options that should only be visible upon request
	 */
	virtual void addCLOptions(
		boost::program_options::options_description &visible, boost::program_options::options_description &hidden
	) {
		namespace po = boost::program_options;

		visible.add_options()
			("ip,i", po::value<std::string>(&m_server)->default_value(GASIOTCPCONSUMERDEFAULTSERVER),
			 "\t[tcpc] The name or ip of the server")
			("port,p", po::value<unsigned short>(&m_port)->default_value(GASIOTCPCONSUMERDEFAULTPORT),
			 "\t[tcpc] The port of the server");

		hidden.add_options()
			("serializationMode,s", po::value<Gem::Common::serializationMode>(&m_serializationMode)->default_value(
				GASIOTCPCONSUMERSERIALIZATIONMODE),
			 "\t[tcpc] Specifies whether serialization shall be done in TEXTMODE (0), XMLMODE (1) or BINARYMODE (2)")
			("maxStalls", po::value<std::uint32_t>(&m_maxStalls)->default_value(GASIOTCPCONSUMERMAXSTALLS),
			 "\t[tcpc] The maximum allowed number of stalled connection attempts of a client")
			("maxConnectionAttempts",
			 po::value<std::uint32_t>(&m_maxConnectionAttempts)->default_value(GASIOTCPCONSUMERMAXCONNECTIONATTEMPTS),
			 "\t[tcpc] The maximum allowed number of failed connection attempts of a client")
			("returnRegardless", po::value<bool>(&m_returnRegardless)->default_value(GASIOTCPCONSUMERRETURNREGARDLESS),
			 "\t[tcpc] Specifies whether unsuccessful client-side processing attempts should be returned to the server")
			("nListenerThreads", po::value<std::size_t>(&m_listenerThreads)->default_value(m_listenerThreads),
			 "\t[tcpc] The number of threads used to listen for incoming connections");
	}

	/***************************************************************************/
	/**
	 * Takes a boost::program_options::variables_map object and checks for supplied options.
	 */
	virtual void actOnCLOptions(const boost::program_options::variables_map &vm) { /* nothing */ }

private:
	/***************************************************************************/
	/**
	 * Schedules the de-serialization of a completed work item, so the server
	 * session does not need to perform this work
	 */
	void async_scheduleDeSerialization(
		std::shared_ptr <std::string> dataBody_ptr, Gem::Common::PORTIDTYPE portId
	) {
		m_gtp.async_schedule(
			std::function<void()>(
				std::bind(
					&GAsioTCPConsumerT<processable_type>::handle_workItemComplete // Does its own error checks
					, this, dataBody_ptr, m_serializationMode, portId, m_timeout
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
		std::shared_ptr <std::string> dataBody_ptr, Gem::Common::serializationMode sM, Gem::Common::PORTIDTYPE portId,
		std::chrono::duration<double> timeout
	) {
		// De-Serialize the data
		std::shared_ptr <processable_type> p
			= Gem::Common::sharedPtrFromString<processable_type>(*dataBody_ptr, sM);

		// Complain if this is an empty item
		if (!p) {
			glogger
			<< "In GAsioTCPConsumerT<>::handle_workItemComplete(): Error!" << std::endl
			<< "Received empty item when filled item was expected!" << std::endl
			<< GEXCEPTION;
		}

		// Return the item to the broker. The item will be discarded
		// if the requested target queue cannot be found.
		try {
			while (!m_broker_ptr->put(portId, p, timeout)) {
				if (this->stopped()) { // This may lead to a loss of items
					glogger
					<< "GAsioTCPConsumerT<>::In handle_workItemComplete(): Warning!" << std::endl
					<< "Discarding item as the consumer object stopped operation" << std::endl
					<< GWARNING;

					return;
				}

				continue;
			}
		} catch (const Gem::Courtier::buffer_not_present &) { // discard the item
			glogger
			<< "GAsioTCPConsumerT<>::In handle_workItemComplete(): Warning!" << std::endl
			<< "Discarding item as buffer port is not present" << std::endl
			<< GWARNING;

			return;
		} catch (...) {
			glogger
			<< "GAsioTCPConsumerT<>::In handle_workItemComplete():" << std::endl
			<< "Caught unknown exception" << std::endl
			<< GEXCEPTION;
		}
	}

	/***************************************************************************/
	/**
	 * Starts a new session
	 */
	void async_newAccept() {
		// First we make sure a new session is started asynchronously so the next request can be served
		std::shared_ptr <GAsioServerSessionT<processable_type>> newSession(
			new GAsioServerSessionT<processable_type>(
				m_io_service
				, m_serializationMode
				, this
			)
		);

		try {
			m_acceptor.async_accept(
				newSession->getSocket()
				, std::bind(
					&GAsioTCPConsumerT<processable_type>::async_handleAccept
					, this, newSession
					, std::placeholders::_1 // Replaces boost::asio::placeholders::error
				)
			);
		} catch (const boost::system::system_error &e) {
			glogger
			<< "In GAsioTCPConsumerT::async_newAccept():" << std::endl
			<< "Caught boost::system::system_error exception with messages:" << std::endl
			<< e.what() << std::endl
			<< GEXCEPTION;
		} catch (const boost::exception &e) {
			glogger
			<< "In GAsioTCPConsumerT::async_newAccept():" << std::endl
			<< "Caught boost::exception exception with messages:" << std::endl
			<< boost::diagnostic_information(e) << std::endl
			<< GEXCEPTION;
		} catch (...) {
			glogger
			<< "In GAsioTCPConsumerT::async_newAccept():" << std::endl
			<< "Caught unknown exception" << std::endl
			<< GEXCEPTION;
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
		std::shared_ptr <GAsioServerSessionT<processable_type>> currentSession
		, const boost::system::error_code &error
	) {
		if (error) {
			glogger
			<< "In GAsioTCPConsumerT<>::async_handleAccept():"
			<< "Terminating on error " << error << std::endl
			<< GEXCEPTION;
			return;
		}

		// Initiate waiting for new connections
		try {
			this->async_newAccept();

			// Initiate the processing sequence
			currentSession->async_processRequest();
		} catch (const boost::system::system_error &e) {
			glogger
			<< "In GAsioTCPConsumerT::async_handleAccept():" << std::endl
			<< "Caught boost::system::system_error exception with messages:" << std::endl
			<< e.what() << std::endl
			<< GEXCEPTION;
		} catch (const boost::exception &e) {
			glogger
			<< "In GAsioTCPConsumerT::async_handleAccept():" << std::endl
			<< "Caught boost::exception exception with messages:" << std::endl
			<< boost::diagnostic_information(e) << std::endl
			<< GEXCEPTION;
		} catch (...) {
			glogger
			<< "In GAsioTCPConsumerT::async_handleAccept():" << std::endl
			<< "Caught unknown exception" << std::endl
			<< GEXCEPTION;
		}
	}

	/***************************************************************************/
	boost::asio::io_service m_io_service;   ///< ASIO's io service, responsible for event processing, absolutely needs to be _before_ acceptor so it gets initialized first.
	std::shared_ptr <boost::asio::io_service::work> m_work; ///< A place holder ensuring that the io_service doesn't stop prematurely
	std::size_t m_listenerThreads;  ///< The number of threads used to listen for incoming connections through io_servce::run()
	boost::asio::ip::tcp::acceptor m_acceptor; ///< takes care of external connection requests
	Gem::Common::serializationMode m_serializationMode; ///< Specifies the serialization mode
	std::uint32_t m_maxStalls; ///< The maximum allowed number of stalled connection attempts of a client
	std::uint32_t m_maxConnectionAttempts; ///< The maximum allowed number of failed connection attempts of a client
	bool m_returnRegardless; ///< Specifies whether unsuccessful processing attempts should be returned to the server
	unsigned short m_port; ///< The port on which the server is supposed to listen
	std::string m_server;  ///< The name or ip if the server
	std::chrono::duration<double> m_timeout; ///< A timeout for put- and get-operations
	Gem::Common::GThreadGroup m_gtg; ///< Holds listener threads
	Gem::Common::GThreadPool m_gtp; ///< Holds workers sorting processed items back into the broker
	std::shared_ptr <Gem::Courtier::GBrokerT<processable_type>> m_broker_ptr; ///< A pointer to the global broker
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Courtier */
} /* namespace Gem */

#endif /* GASIOTCPCONSUMERT_HPP_ */
