/**
 * @file GBaseClientT.hpp
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
#include <boost/thread/mutex.hpp>
#include <boost/date_time.hpp>
#include <boost/lexical_cast.hpp>

#ifndef GBASECLIENTT_HPP_
#define GBASECLIENTT_HPP_

// Geneva headers go here
#include "common/GSerializationHelperFunctionsT.hpp"
#include "common/GLogger.hpp"
#include "courtier/GCourtierEnums.hpp"

namespace Gem {
namespace Courtier {

/******************************************************************************/
/**
 * This class forms the basis of a hierarchy of classes designed for client-side
 * network communication. Their task is to retrieve serialized objects from the server
 * over a given protocol (implemented in derived classes), to instantiate the
 * corresponding object, to process it and to deliver the results to the server.
 * This class assumes that the template parameter implements the "process()" call.
 */
template<typename processable_type>
class GBaseClientT
	: private boost::noncopyable {
public:
	/***************************************************************************/
	/**
	 * The default constructor.
	 */
	GBaseClientT()
		: startTime_(boost::posix_time::microsec_clock::universal_time()), maxDuration_(boost::posix_time::microsec(0)),
		  processed_(0), processMax_(0), returnRegardless_(true),
		  additionalDataTemplate_(std::shared_ptr<processable_type>()) { /* nothing*/ }

	/***************************************************************************/

	/**
	 * A constructor that accepts a model of the item to be processed. This can be
	 * used to avoid having to transfer or reload data that doesn't change. Note that
	 * the model must understand the clone() command.
	 *
	 * @param additionalDataTemplate The model of the item to be processed
	 */
	GBaseClientT(std::shared_ptr <processable_type> additionalDataTemplate)
		: startTime_(boost::posix_time::microsec_clock::universal_time()), maxDuration_(boost::posix_time::microsec(0)),
		  processed_(0), processMax_(0), returnRegardless_(true),
		  additionalDataTemplate_(additionalDataTemplate) { /* nothing*/ }


	/***************************************************************************/

	/**
	 * A standard destructor. We have no local, dynamically allocated data, hence it is empty.
	 */
	virtual ~GBaseClientT() { /* nothing */ }

	/***************************************************************************/
	/**
	 * This is the main loop of the client. It will continue to call the process()
	 * function (defined by derived classes), until it returns false or the maximum
	 * number of processing steps has been reached. All network connectivity is done
	 * in process().
	 */
	void run() {
		try {
			if (this->init()) {
				while (
					!this->halt() &&
					CLIENT_CONTINUE == this->process()
					) { /* nothing*/ }
			} else {
				glogger
				<< "In GBaseClientT<T>::run(): Initialization failed. Leaving ..." << std::endl
				<< GEXCEPTION;
			}

			if (!this->finally()) {
				glogger
				<< "In GBaseClientT<T>::run(): Finalization failed." << std::endl
				<< GEXCEPTION;
			}
		}
		catch (Gem::Common::gemfony_error_condition &e) {
			glogger
			<< "In GBaseClientT<T>::run():" << std::endl
			<< "Caught Gem::Common::gemfony_error_condition" << std::endl
			<< "with message" << std::endl
			<< e.what()
			<< GEXCEPTION;
		}
		catch (boost::exception &) {
			glogger
			<< "In GBaseClientT<T>::run(): Caught boost::exception" << std::endl
			<< GEXCEPTION;
		}
		catch (std::exception &e) {
			glogger
			<< "In GBaseClientT<T>::run(): Caught std::exception with message" << std::endl
			<< e.what()
			<< GEXCEPTION;
		}
		catch (...) {
			glogger
			<< "In GBaseClientT<T>::run(): Caught unknown exception" << std::endl
			<< GEXCEPTION;
		}
	}

	/***************************************************************************/

	/**
	 * Allows to set a maximum number of processing steps. If set to 0 or left unset,
	 * processing will be done until process() returns false.
	 *
	 * @param processMax Desired value for the processMax_ variable
	 */
	void setProcessMax(const std::uint32_t &processMax) {
		processMax_ = processMax;
	}

	/***************************************************************************/

	/**
	 * Retrieves the value of the processMax_ variable.
	 *
	 * @return The value of the processMax_ variable
	 */
	std::uint32_t getProcessMax() const {
		return processMax_;
	}

	/***************************************************************************/
	/**
	 * Sets the maximum allowed processing time
	 *
	 * @param maxDuration The maximum allowed processing time
	 */
	void setMaxTime(const boost::posix_time::time_duration &maxDuration) {
		using namespace boost::posix_time;

		// Only allow "real" values
		if (maxDuration.is_special() || maxDuration.is_negative()) {
			glogger
			<< "In GBaseClientT<T>::setMaxTime():" << std::endl
			<< "Invalid maxDuration." << std::endl
			<< GEXCEPTION;
		}

		maxDuration_ = maxDuration;
	}

	/***************************************************************************/
	/**
	 * Retrieves the value of the maxDuration_ parameter.
	 *
	 * @return The maximum allowed processing time
	 */
	boost::posix_time::time_duration getMaxTime() {
		return maxDuration_;
	}

	/***************************************************************************/
	/**
	 * Specifies whether results should be returned regardless of the success achieved
	 * in the processing step.
	 *
	 * @param returnRegardless Specifies whether results should be returned to the server regardless of their success
	 */
	void setReturnRegardless(const bool &returnRegardless) {
		returnRegardless_ = returnRegardless;
	}

	/***************************************************************************/
	/**
	 * Checks whether results should be returned regardless of the success achieved
    * in the processing step.
    *
    * @return Whether results should be returned to the server regardless of their success
	 */
	bool getReturnRegardless() const {
		return returnRegardless_;
	}

protected:
	/***************************************************************************/

	/**
	 * In order to allow derived classes to concentrate on network issues, all
	 * unpacking, the calculation, and packing is done in the GBaseClientT class
	 */
	bool process() {
		// Store the current serialization mode
		Gem::Common::serializationMode serMode;

		// Get an item from the server
		std::string istr, serModeStr, portId;
		if (!this->retrieve(istr, serModeStr, portId)) {
			return false;
		}

		// There is a possibility that we have received an unknown command
		// or a timeout command. In this case we want to try again until retrieve()
		// returns "false". If we return true here, the next "process" command will
		// be executed.
		if (istr == "empty") return true;

		// Check the serialization mode we need to use
		if (serModeStr == "") {
			glogger
			<< "In GBaseClientT<T>::process() : Warning!" << std::endl
			<< "Found empty serModeStr. Leaving." << std::endl
			<< GWARNING;

			return false;
		}

		serMode = boost::lexical_cast<Gem::Common::serializationMode>(serModeStr);

		// unpack the data and create a new object. Note that de-serialization must
		// generally happen through the same type that was used for serialization.
		std::shared_ptr <processable_type> target = Gem::Common::sharedPtrFromString<processable_type>(istr, serMode);

		// Check if we have received a valid target. Leave the function if this is not the case
		if (!target) {
			glogger
			<< "In GBaseClientT<T>::process() : Warning!" << std::endl
			<< "Received empty target." << std::endl
			<< GWARNING;

			// This means that process() will be called again
			return true;
		}

		// If we have a model for the item to be parallelized, load its data into the target
		if (additionalDataTemplate_) {
			target->loadConstantData(additionalDataTemplate_);
		}

		// This one line is all it takes to do the processing required for this object.
		// The object has all required functions on board. GBaseClientT<T> does not need to understand
		// what is being done during the processing. If processing did not lead to a useful result,
		// information will be returned back to the server only if returnRegardless_
		// is set to true.
		if (!target->process() && !returnRegardless_) return true;

		// transform target back into a string and submit to the server. The actual
		// actions done by submit are defined by derived classes.
		if (!this->submit(Gem::Common::sharedPtrToString(target, serMode), portId)) {
			return false;
		}

		// Everything worked. Indicate that we want to continue
		return true;
	} // std::shared_ptr<processable_type> target will cease to exist at this point

	/***************************************************************************/

	/** @brief Performs initialization work */
	virtual bool init() { return true; }

	/***************************************************************************/

	/** @brief Perform necessary finalization activities */
	virtual bool finally() { return true; }

	/***************************************************************************/

	/** @brief Retrieve work items from the server. To be defined by derived classes. */
	virtual bool retrieve(std::string &, std::string &, std::string &) = 0;

	/***************************************************************************/

	/** @brief Submit processed items to the server. To be defined by derived classes. */
	virtual bool submit(const std::string &, const std::string &) = 0;

	/***************************************************************************/

	/** @brief Custom halt condition for processing */
	virtual bool customHalt() { return false; }

private:
	/***************************************************************************/

	/**
	 * Checks whether a halt condition was reached. Either the maximum number of processing
	 * steps was reached or the maximum allowed time was reached.
	 *
	 * @return A boolean indicating whether a halt condition was reached
	 */
	bool halt() {
		using namespace boost::posix_time;

		// Maximum number of processing steps reached ?
		if (processMax_ && (processed_++ >= processMax_)) return true;

		// Maximum duration reached ?
		if (maxDuration_.total_microseconds() &&
			 ((microsec_clock::universal_time() - startTime_) >= maxDuration_))
			return true;

		// Custom halt condition reached ?
		if (customHalt()) return true;

		return false;
	}

	/***************************************************************************/

	boost::posix_time::ptime startTime_; ///< Used to store the start time of the optimization
	boost::posix_time::time_duration maxDuration_; ///< Maximum time frame for the optimization

	std::uint32_t processed_; ///< The number of processed items so far
	std::uint32_t processMax_; ///< The maximum number of items to process

	bool returnRegardless_; ///< Specifies whether unsuccessful processing attempts should be returned to the server

	std::shared_ptr <processable_type> additionalDataTemplate_; ///< Optionally holds a template of the object to be processed
};

/******************************************************************************/


} /* namespace Courtier */
} /* namespace Gem */

#endif /* GBASECLIENTT_HPP_ */
