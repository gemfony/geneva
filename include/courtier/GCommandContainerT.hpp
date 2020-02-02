/**
 * @file
 */

/********************************************************************************
 *
 * This file is part of the Geneva library collection. The following license
 * applies to this file:
 *
 * ------------------------------------------------------------------------------
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * ------------------------------------------------------------------------------
 *
 * Note that other files in the Geneva library collection may use a different
 * license. Please see the licensing information in each file.
 *
 ********************************************************************************
 *
 * Geneva was started by Dr. Rüdiger Berlich and was later maintained together
 * with Dr. Ariel Garcia under the auspices of Gemfony scientific. For further
 * information on Gemfony scientific, see http://www.gemfomy.eu .
 *
 * The majority of files in Geneva was released under the Apache license v2.0
 * in February 2020.
 *
 * See the NOTICE file in the top-level directory of the Geneva library
 * collection for a list of contributors and copyright information.
 *
 ********************************************************************************/

#pragma once

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
	 // Defaulted constructors, destructor and move assigment operator

	 GCommandContainerT() = default;
	 GCommandContainerT(GCommandContainerT<processable_type, command_type>&& cp) noexcept = default;
	 ~GCommandContainerT() = default;

	 GCommandContainerT<processable_type, command_type>& operator=(GCommandContainerT<processable_type, command_type>&& cp) noexcept = default;

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

	// Make the compiler happy
	return std::string();
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

