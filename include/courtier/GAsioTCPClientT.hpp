/**
 * @file GAsioTCPClientT.hpp
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
#include <boost/utility.hpp>
#include <boost/cstdint.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread/mutex.hpp>

#ifndef GASIOTCPCLIENTT_HPP_
#define GASIOTCPCLIENTT_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif


// Geneva headers go here
#include "common/GSerializationHelperFunctionsT.hpp"
#include "hap/GRandomT.hpp"
#include "GAsioHelperFunctions.hpp"
#include "GBaseClientT.hpp"
#include "GCommunicationEnums.hpp"

namespace Gem
{
namespace Courtier
{

/***********************************************************************/

/**
 * Global variables for failed transfers and connection attempts.
 */
const boost::uint32_t ASIOMAXSTALLS=10;
const boost::uint32_t ASIOMAXCONNECTIONATTEMPTS=10;

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
	/***********************************************************************/
	/**
	 * The main constructor for this class. No default constructor is provided.
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

	/***********************************************************************/
	/**
	 * The standard destructor.
	 */
	virtual ~GAsioTCPClientT()	{
		delete [] tmpBuffer_;
	}

	/**********************************************************************/
	/**
	 * Sets the maximum number of stalled connection attempts
	 *
	 * @param maxStalls The maximum number of stalled connection attempts
	 */
	void setMaxStalls(const boost::uint32_t& maxStalls)  {
		maxStalls_ = maxStalls;
	}

	/**********************************************************************/
	/**
	 * Retrieves the maximum allowed number of stalled attempts
	 *
	 * @return The value of the maxStalls_ variable
	 */
	boost::uint32_t getMaxStalls() const  {
		return maxStalls_;
	}

	/**********************************************************************/
	/**
	 * Sets the maximum number of failed connection attempts before termination
	 *
	 * @param maxConnectionAttempts The maximum number of allowed failed connection attempts
	 */
	void setMaxConnectionAttempts(const boost::uint32_t& maxConnectionAttempts)  {
		maxConnectionAttempts_ = maxConnectionAttempts;
	}

	/**********************************************************************/
	/**
	 * Retrieves the maximum allowed number of failed connection attempts
	 *
	 * @return The value of the maxConnectionAttempts_ variable
	 */
	boost::uint32_t getMaxConnectionAttempts() const  {
		return maxConnectionAttempts_;
	}

protected:
	/**********************************************************************/
	/**
	 * Performs initialization work, such as the retrieval and setting of a
	 * seed for the GRandomFactory
	 *
	 * @return A boolean indicating whether initialization was successful
	 */
	bool init() {
		try {
			// Try to make a connection
			if(!tryConnect()) {
				std::ostringstream warning;
				warning << "In GAsioTCPClientT<processable_type>::init(): Warning" << std::endl
				        << "Could not connect to server. Shutting down now." << std::endl;

				std::cerr << warning.str();

				return shutdown(false);
			}

			// Let the server know we want an initial seed
			boost::asio::write(socket_, boost::asio::buffer(assembleQueryString("getSeed", Gem::Courtier::COMMANDLENGTH)));

			// Read answer. The seed should have been sent by the server
			boost::asio::read(socket_, boost::asio::buffer(tmpBuffer_, Gem::Courtier::COMMANDLENGTH));

			// Remove all leading or trailing white spaces from the command
			std::string inboundSeedString = boost::algorithm::trim_copy(std::string(tmpBuffer_, Gem::Courtier::COMMANDLENGTH));
			boost::uint32_t seed = boost::lexical_cast<boost::uint32_t>(inboundSeedString);

			std::cout << "Received seed " << seed << " from the server" << std::endl;

			// Set the seed of the global factory
			if(!(GRANDOMFACTORY->setStartSeed(seed))) {
				std::cerr << "In GAsioTCPClientT<processable_type>::init(): Warning" << std::endl
						  << "Seed (" << seed << ") has already been set." << std::endl;
			}

			return shutdown(true);
		}
		// Any system error (except for those where a connection attempt failed) is considered
		// fatal and leads to the termination, by returning false.
		catch (boost::system::system_error& bse) {
			{ // Make sure we do not hide the next error declaration (avoid a warning message)
				std::ostringstream error;
				error << "In GAsioTCPClientT<processable_type>::init():" << std::endl
					  << "Caught boost::system::system_error exception." << std::endl
					  << "Leaving now." << std::endl;

				std::cerr << error.str();
			}

			try {
				return shutdown(false);
			} catch (...) {
				std::ostringstream error;
				error << "In GAsioTCPClientT<processable_type>::init():" << std::endl
					  << "Cannot shutdown gracefully as shutdown command" << std::endl
					  << "threw inside of catch statement." << std::endl;

				std::cerr << error.str();

				std::terminate();
			}
		}

		// This part of the function should never be reached. Let the audience know, then terminate.
		std::ostringstream error;
		error << "In GAsioTCPClientT<processable_type>::init() :" << std::endl
			  << "In a part of the function that should never have been reached!" << std::endl;
		std::cerr << error.str();

		std::terminate();

		return false; // Make the compiler happy
	}

	/**********************************************************************/
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

				return shutdown(false);
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
				return shutdown(true);
			}
			else { // Received no work. Try again a number of times
				// We will usually only allow a given number of timeouts / stalls
				if (maxStalls_ && (stalls_++ > maxStalls_)) {
					std::ostringstream error;
					error << "In GAsioTCPClientT<processable_type>::retrieve():" << std::endl
						  << "Maximum number of consecutive stalls reached,"
						  << std::endl << "with last command = "
						  << inboundCommandString << std::endl
						  << "Cannot cope. Leaving now." << std::endl;

					return shutdown(false);
				}

				// We can continue. But let's wait approximately 0.5 seconds first.
				usleep(500000);

				// Indicate that we want to continue
				return shutdown(true);
			}
		}
		// Any system error (except for those where a connection attempt failed) is considered
		// fatal and leads to the termination, by returning false.
		catch (boost::system::system_error&) {
			{ // Make sure we do not hide the next error declaration (avoid a warning message)
				std::ostringstream error;
				error << "In GAsioTCPClientT<processable_type>::retrieve():" << std::endl
						 << "Caught boost::system::system_error exception."
						 << std::endl << "Leaving now." << std::endl;

				std::cerr << error.str();
			}

			try {
				return shutdown(false);
			} catch (...) {
				std::ostringstream error;
				error << "In GAsioTCPClientT<processable_type>::retrieve():" << std::endl
					  << "Cannot shutdown gracefully as shutdown command" << std::endl
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

	/**********************************************************************/
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

				return shutdown(false);
			}

			// And write the serialized data to the socket. We use
			// "gather-write" to send different buffers in a single
			// write operation.
			boost::asio::write(socket_, buffers);

			// Make sure we don't leave any open sockets lying around.
			return shutdown(true);
		}
		// Any system error (except for those where a connection attempt failed) is considered
		// fatal and leads to the termination, by returning false.
		catch (boost::system::system_error&) {
			{
				std::ostringstream error;
				error << "In GAsioTCPClientT<processable_type>::retrieve:" << std::endl
						  << "Caught boost::system::system_error exception."
						  << std::endl << "Leaving now." << std::endl;

				std::cerr << error.str();
			}

			try {
				return shutdown(false);
			} catch (...) {
				std::ostringstream error;
				error << "In GAsioTCPClientT<processable_type>::retrieve:" << std::endl
					  << "Cannot shutdown gracefully as shutdown command" << std::endl
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
	/**********************************************************************/
	/**
	 * A small helper function that shuts down the socket and emits a return code
	 *
	 * @param returnCode The return code to be emitted
	 * @return The return code
	 */
	bool shutdown(const bool& returnCode){
		// Make sure we don't leave any open sockets lying around.
		socket_.close();
		return returnCode;
	}

	/**********************************************************************/
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
			if (!error)	break;

			// Unsuccessful. Sleep for a second, then try again
			usleep(200000);
		}

		// Still error ? Return, terminate
		if (error) return false;

		return true;
	}

	/**********************************************************************/

	boost::uint32_t maxStalls_; ///< The maximum allowed number of stalled connection attempts
	boost::uint32_t maxConnectionAttempts_; ///< The maximum allowed number of failed connection attempts

	boost::uint32_t stalls_; ///< counter for stalled connection attempts

	char *tmpBuffer_;

	boost::asio::io_service io_service_; ///< Holds the Boost::ASIO::io_service object

	boost::asio::ip::tcp::socket socket_; ///< The underlying socket

	boost::asio::ip::tcp::resolver resolver_; ///< Responsible for name resolution
	boost::asio::ip::tcp::resolver::query query_; ///< A query

	boost::asio::ip::tcp::resolver::iterator endpoint_iterator0_; ///< start of iteration
	boost::asio::ip::tcp::resolver::iterator end_; ///< end for endpoint iterator
};

/***********************************************************************/

} /* namespace Courtier */
} /* namespace Gem */

#endif /* GASIOTCPCLIENTT_HPP_ */
