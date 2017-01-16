/**
 * @file GAdHocSubmissionClientT.hpp
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
#include <sstream>
#include <vector>
#include <chrono>
#include <type_traits>
#include <functional>

// Boost headers go here
#include <boost/utility.hpp>
#include <boost/asio.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/lexical_cast.hpp>

#ifndef GSERIALSUBMISSIONCLIENTT_HPP
#define GSERIALSUBMISSIONCLIENTT_HPP

// Geneva headers go here
#include "common/GSerializationHelperFunctionsT.hpp"
#include "common/GLogger.hpp"
#include "courtier/GBaseClientT.hpp"
#include "courtier/GCourtierEnums.hpp"
#include "courtier/GProcessingContainerT.hpp"

namespace Gem {
namespace Courtier {

/******************************************************************************/
/**
 * This class forms the basis of a hierarchy of classes designed for client-side
 * network communication. Their task is to retrieve serialized objects from the server
 * over a given protocol (implemented in derived classes), to instantiate the
 * corresponding object, to process it and to deliver the results to the server.
 * This class assumes that the template parameter implements the "process()" call
 * and that processing may take an arbitrary amount of time, e.g. because the
 * connection is closed during the processing. A single work item is retrieved
 * at the time, amounting to a serial execution of workloads.
 */
template<typename processable_type>
class GSerialSubmissionClientT
	: public GBaseClientT<processable_type>
{
public:
	/***************************************************************************/
	/**
	 * The default constructor.
	 */
	GSerialSubmissionClientT()
	{ /* nothing */ }

	/***************************************************************************/
	/**
	 * A constructor that accepts a model of the item to be processed. This can be
	 * used to avoid having to transfer or reload data that doesn't change. Note that
	 * the model must understand the clone() command.
	 *
	 * @param additionalDataTemplate The model of the item to be processed
	 */
	GSerialSubmissionClientT(std::shared_ptr<processable_type> additionalDataTemplate)
		: GBaseClientT<processable_type>(additionalDataTemplate)
	{ /* nothing */ }

	/***************************************************************************/
	/**
	 * A standard destructor. We have no local, dynamically allocated data, hence it is empty.
	 */
	virtual ~GSerialSubmissionClientT()
	{ /* nothing */ }

	/***************************************************************************/
	/**
	 * This is the main loop of the client. It will continue to call the process()
	 * function (defined by derived classes), until it returns false or a halt-condition
	 * was reached.
	 */
	virtual void run_() override {
		while (!this->halt() && CLIENT_CONTINUE == this->process()) { /* nothing */ }
	}

protected:
	/***************************************************************************/
	/**
	 * In order to allow derived classes to concentrate on network issues, all
	 * unpacking, the calculation, and packing is done in the GAdHocSubmissionClientT class
	 */
	bool process() {
		// Store the current serialization mode
		Gem::Common::serializationMode serMode;

		// Get an item from the server
		//
		// TODO: Check if the clients drop out here
		//
		std::string istr, serModeStr;
		if(!this->retrieve(istr, serModeStr)) {
			glogger
			<< "In GSerialSubmissionClientT<T>::process() : Warning!" << std::endl
			<< "Could not retrieve item from server. Leaving ..." << std::endl
			<< GWARNING;

			return false;
		}

		// There is a possibility that we have received an unknown command
		// or a timeout command. In this case we want to try again until retrieve()
		// returns "false". If we return true here, the next "process" command will
		// be executed.
		if(istr == "empty") return true;

		// Check the serialization mode we need to use
		//
		// TODO: Check if the clients drop out here
		//
		if(serModeStr == "") {
			glogger
			<< "In GSerialSubmissionClientT<T>::process() : Warning!" << std::endl
			<< "Found empty serModeStr. Leaving ..." << std::endl
			<< GWARNING;

			return false;
		}

		serMode = boost::lexical_cast<Gem::Common::serializationMode>(serModeStr);

		// unpack the data and create a new object. Note that de-serialization must
		// generally happen through the same type that was used for serialization.
		std::shared_ptr<processable_type> target = Gem::Common::sharedPtrFromString<processable_type>(istr, serMode);

		// Check if we have received a valid target. Leave the function if this is not the case
		if(!target) {
			glogger
			<< "In GSerialSubmissionClientT<T>::process() : Warning!" << std::endl
			<< "Received empty target." << std::endl
			<< GWARNING;

			// This means that process() will be called again
			return true;
		}

		// If we have a model for the item to be parallelized, load its data into the target
	   this->loadDataTemplate(target);

		// This one line is all it takes to do the processing required for this object.
		// The object has all required functions on board. GSerialSubmissionClientT<T> does not need to understand
		// what is being done during the processing. If processing did not lead to a useful result,
		// information will be returned back to the server only if m_returnRegardless
		// is set to true.
		if(!target->process() && !this->getReturnRegardless()) return true;

		// transform target back into a string and submit to the server. The actual
		// actions done by submit are defined by derived classes.
		//
		// TODO: Check if the clients drop out here
		//
		if(!this->submit(Gem::Common::sharedPtrToString(target, serMode))) {
			glogger
			<< "In GSerialSubmissionClientT<T>::process() : Warning!" << std::endl
			<< "Could not return item to server. Leaving ..." << std::endl
			<< GWARNING;

			return false;
		}

		// Everything worked. Indicate that we want to continue
		return true;
	} // std::shared_ptr<processable_type> target will cease to exist at this point

	/***************************************************************************/
	/** @brief Retrieve work items from the server. To be defined by derived classes. */
	virtual bool retrieve(std::string &, std::string &) = 0;

	/***************************************************************************/
	/** @brief Submit processed items to the server. To be defined by derived classes. */
	virtual bool submit(const std::string &) = 0;
};

/******************************************************************************/


} /* namespace Courtier */
} /* namespace Gem */

#endif /* GSERIALSUBMISSIONCLIENTT_HPP */
