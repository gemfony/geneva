/**
 * @file GCommandContainerT.hpp
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

#ifndef GCOMMANDCONTAINERT_HPP_
#define GCOMMANDCONTAINERT_HPP_

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
#include <boost/serialization/nvp.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/lexical_cast.hpp>

// Geneva headers go here
#include "courtier/GCourtierEnums.hpp"
#include "courtier/GProcessingContainerT.hpp"

namespace Gem {
namespace Courtier {

/******************************************************************************************/
////////////////////////////////////////////////////////////////////////////////////////////
/******************************************************************************************/
/**
 * This class encapsulate a processable item that may be transmitted to a remote site,
 * equiped with a command.
 *
 * @tparam processable_type The type of the processable type
 * @tparam command_type The command set to be executed on the processable type
 */
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
	 /** Default constructor */
	 GCommandContainerT() = default;

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

private:
	 //-------------------------------------------------------------------------
	 // Data

	 command_type m_command{command_type(0)}; ///< The command to be exeecuted
	 std::shared_ptr<processable_type>  m_payload_ptr; ///< The actual payload, if any

	 //-------------------------------------------------------------------------
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * Conversion of a GCommandContainerT to a string
 */
template<
	typename processable_type
	, typename command_type
>
std::string container_to_string(
	const GCommandContainerT<processable_type, command_type>& container
	, Gem::Common::serializationMode serMode
) {
	try {
		switch (serMode) {
			case Gem::Common::serializationMode::TEXT: {
				std::ostringstream oss;
				boost::archive::text_oarchive oa(oss);
				oa << boost::serialization::make_nvp(
					"command_container"
					, container
				);
				return oss.str();
			} break; // archive and stream closed at end of scope

			case Gem::Common::serializationMode::XML: {
				std::ostringstream oss;
				boost::archive::xml_oarchive oa(oss);
				oa << boost::serialization::make_nvp(
					"command_container"
					, container
				);
				return oss.str();
			} break;

			case Gem::Common::serializationMode::BINARY: {
				std::ostringstream oss(std::ios_base::binary);
				boost::archive::binary_oarchive oa(oss);
				oa << boost::serialization::make_nvp(
					"command_container"
					, container
				);
				return oss.str();
			} break;
		}
	} catch (const boost::system::system_error &e) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In container_to_string(GCommandContainerT<>):" << std::endl
				<< "Caught boost::system::system_error exception with messages:" << std::endl
				<< e.what() << std::endl
				<< "with serializationMode == " << Gem::Common::serModeToString(serMode) << std::endl
		);
	} catch (const boost::exception &e) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In container_to_string(GCommandContainerT<>):" << std::endl
				<< "Caught boost::exception exception with messages:" << std::endl
				<< boost::diagnostic_information(e) << std::endl
				<< "with serializationMode == " << Gem::Common::serModeToString(serMode) << std::endl
		);
	} catch (const std::exception& e) {
		throw gemfony_exception(
			g_error_streamer(
				DO_LOG
				, time_and_place
			)
				<< "In container_to_string(GCommandContainerT<>):" << std::endl
				<< "Caught std::exception exception with messages:" << std::endl
				<< e.what() << std::endl
				<< "with serializationMode == " << Gem::Common::serModeToString(serMode) << std::endl
		);
	} catch (...) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In container_to_string(GCommandContainerT<>):" << std::endl
				<< "Caught unknown exception" << std::endl
				<< "with serializationMode == " << Gem::Common::serModeToString(serMode) << std::endl
		);
	}
}

/******************************************************************************/
/**
 * Loading of a GCommandContainerT from a string
 */
template<
	typename processable_type
	, typename command_type
>
void container_from_string(
	const std::string& descr
	, GCommandContainerT<processable_type, command_type>& container
	, Gem::Common::serializationMode serMode
) {
	container.reset();

	try {
		switch(serMode) {
			case Gem::Common::serializationMode::TEXT: {
				std::istringstream iss(descr);
				boost::archive::text_iarchive ia(iss);
				ia >> boost::serialization::make_nvp("command_container", container);
			} break; // archive and stream closed at end of scope

			case Gem::Common::serializationMode::XML: {
				std::istringstream iss(descr);
				boost::archive::xml_iarchive ia(iss);
				ia >> boost::serialization::make_nvp("command_container", container);
			} break;

			case Gem::Common::serializationMode::BINARY: {
				std::istringstream iss(descr, std::ios_base::binary);
				boost::archive::binary_iarchive ia(iss);
				ia >> boost::serialization::make_nvp("command_container", container);
			} break;
		}
	} catch (const boost::system::system_error &e) {
		throw gemfony_exception(
			g_error_streamer(
				DO_LOG
				,  time_and_place
			)
				<< "In container_from_string(GCommandContainerT<>):" << std::endl
				<< "Caught boost::system::system_error exception with messages:" << std::endl
				<< e.what() << std::endl
				<< "with serializationMode == " << Gem::Common::serModeToString(serMode) << std::endl
		);
	} catch (const boost::exception &e) {
		throw gemfony_exception(
			g_error_streamer(
				DO_LOG
				, time_and_place
			)
				<< "In container_from_string(GCommandContainerT<>):" << std::endl
				<< "Caught boost::exception exception with messages:" << std::endl
				<< boost::diagnostic_information(e) << std::endl
				<< "with serializationMode == " << Gem::Common::serModeToString(serMode) << std::endl
		);
	} catch (const std::exception& e) {
		throw gemfony_exception(
			g_error_streamer(
				DO_LOG
				, time_and_place
			)
				<< "In container_from_string(GCommandContainerT<>):" << std::endl
				<< "Caught std::exception exception with messages:" << std::endl
				<< e.what() << std::endl
				<< "with serializationMode == " << Gem::Common::serModeToString(serMode) << std::endl
		);
	} catch (...) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In container_from_string(GCommandContainerT<>):" << std::endl
				<< "Caught unknown exception" << std::endl
				<< "with serializationMode == " << Gem::Common::serModeToString(serMode) << std::endl
		);
	}
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Courtier */
} /* namespace Gem */

/******************************************************************************/


#endif /* GCOMMANDCONTAINERT_HPP_ */
