/**
 * @file GBaseClientT.hpp
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
#include <sstream>
#include <vector>

// Includes check for correct Boost version(s)
#include "common/GGlobalDefines.hpp"

// Boost headers go here
#include <boost/cstdint.hpp>
#include <boost/utility.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/date_time.hpp>
#include <boost/lexical_cast.hpp>

#ifndef GBASECLIENTT_HPP_
#define GBASECLIENTT_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif

// Geneva headers go here
#include "common/GSerializationHelperFunctionsT.hpp"
#include "GCommunicationEnums.hpp"

namespace Gem
{
namespace Courtier
{

/*********************************************************************************/
/**
 * This class forms the basis of a hierarchy of classes designed for client-side
 * network communication. Their task is to retrieve serialized objects from the server
 * over a given protocol (implemented in derived classes), to instantiate the
 * corresponding object, to process it and to deliver the results to the server.
 * This class assumes that the template parameter implements the "process()" call.
 */
template <typename processable_type>
class GBaseClientT
	:private boost::noncopyable
{
public:
	/*********************************************************************************/
	/**
	 * The default constructor.
	 */
	GBaseClientT()
		: startTime_(boost::posix_time::microsec_clock::local_time())
		, maxDuration_(boost::posix_time::microsec(0))
		, processed_(0)
		, processMax_(0)
		, returnRegardless_(false)
	{ /* nothing*/ }

	/*********************************************************************************/
	/**
	 * A standard destructor. We have no local, dynamically allocated data, hence it is empty.
	 */
	virtual ~GBaseClientT()
	{ /* nothing */ }

	/*********************************************************************************/
	/**
	 * This is the main loop of the client. It will continue to call the process()
	 * function (defined by derived classes), until it returns false or the maximum
	 * number of processing steps has been reached. All network connectivity is done
	 * in process().
	 */
	void run(){
		try{
			if(this->init()) {
				while(!this->halt() && this->process()){;}
			}
			else {
				std::cerr << "In GBaseClientT<T>::run(): Initialization failed. Leaving ..." << std::endl;
				std::terminate();
			}

			if(!this->finally()) {
				std::cerr << "In GBaseClientT<T>::run(): Finalization failed." << std::endl;
			}
		}
		catch(std::exception& e){
			std::cerr << "In GBaseClientT<T>::run(): Caught std::exception with message" << std::endl
				      << e.what();
			std::terminate();
		}
		catch(boost::exception& e){
			std::cerr << "In GBaseClientT<T>::run(): Caught boost::exception" << std::endl;
			std::terminate();
		}
		catch(...){
			std::cerr << "In GBaseClientT<T>::run(): Caught unknown exception" << std::endl;
			std::terminate();
		}
	}

	/*********************************************************************************/
	/**
	 * Allows to set a maximum number of processing steps. If set to 0 or left unset,
	 * processing will be done until process() returns false.
	 *
	 * @param processMax Desired value for the processMax_ variable
	 */
	void setProcessMax(const boost::uint32_t& processMax){
		processMax_ = processMax;
	}

	/*********************************************************************************/
	/**
	 * Retrieves the value of the processMax_ variable.
	 *
	 * @return The value of the processMax_ variable
	 */
	boost::uint32_t getProcessMax() const{
		return processMax_;
	}

	/*********************************************************************************/
	/**
	 * Sets the maximum allowed processing time
	 *
	 * @param maxDuration The maximum allowed processing time
	 */
	void setMaxTime(const boost::posix_time::time_duration& maxDuration) {
		using namespace boost::posix_time;

		// Only allow "real" values
		if(maxDuration.is_special() || maxDuration.is_negative()) {
			std::ostringstream error;
			error << "In GBaseClientT<T>::setMaxTime() : Error!" << std::endl
				  << "Invalid maxDuration." << std::endl;

			throw Gem::Common::gemfony_error_condition(error.str());
		}

		maxDuration_ = maxDuration;
	}

	/*********************************************************************************/
	/**
	 * Retrieves the value of the maxDuration_ parameter.
	 *
	 * @return The maximum allowed processing time
	 */
	boost::posix_time::time_duration getMaxTime() {
		return maxDuration_;
	}

	/*********************************************************************************/
	/**
	 * Specifies whether results should be returned regardless of the success achieved
	 * in the processing step.
	 *
	 * @param returnRegardless Specifies whether results should be returned to the server regardless of their success
	 */
	void returnResultIfUnsuccessful(const bool& returnRegardless) {
		returnRegardless_ = returnRegardless;
	}

protected:
	/*********************************************************************************/
	/**
	 * In order to allow derived classes to concentrate on network issues, all
	 * unpacking, the calculation, and packing is done in the GBaseClientT class
	 */
	bool process(){
		 // Store the current serialization mode
		Gem::Common::serializationMode serMode;

		// Get an item from the server
		std::string istr, serModeStr, portId;
		if(!this->retrieve(istr, serModeStr, portId)) return false;

		// There is a possibility that we have received an unknown command
		// or a timeout command. In this case we want to try again until retrieve()
		// returns "false". If we return true here, the next "process" command will
		// be executed.
		if(istr == "empty") return true;

		// Check the serialization mode we need to use
		if(serModeStr == ""){
			std::ostringstream error;
			error << "In GBaseClientT<T>::process() : Error!" << std::endl
				  << "Found empty serModeStr" << std::endl;
			std::cerr << error.str();

			return false;
		}

		serMode = boost::lexical_cast<Gem::Common::serializationMode>(serModeStr);

		// unpack the data and create a new object. Note that de-serialization must
		// generally happen through the same type that was used for serialization.
		boost::shared_ptr<processable_type> target = Gem::Common::sharedPtrFromString<processable_type>(istr, serMode);

		// This one line is all it takes to do the processing required for this object.
		// The object has all required functions on board. GBaseClientT<T> does not need to understand
		// what is being done during the processing. If processing did not lead to a useful result,
		// information will be returned back to the server only if discardIfUnsuccessful_ is set to
		// false.
		if(!target->process() && !returnRegardless_) return true;

		// transform target back into a string and submit to the server. The actual
		// actions done by submit are defined by derived classes.
		if(!this->submit(Gem::Common::sharedPtrToString(target, serMode), portId)) return false;

		// Everything worked. Indicate that we want to continue
		return true;
	} // boost::shared_ptr<processable_type> target will cease to exist at this point

	/*********************************************************************************/
	/** @brief Performs initialization work */
	virtual bool init() { return true; }

	/*********************************************************************************/
	/** @brief Perform necessary finalization activities */
	virtual bool finally() { return true; }

	/*********************************************************************************/
	/** @brief Retrieve work items from the server. To be defined by derived classes. */
	virtual bool retrieve(std::string&, std::string&, std::string&) = 0;

	/*********************************************************************************/
	/** @brief Submit processed items to the server. To be defined by derived classes. */
	virtual bool submit(const std::string&, const std::string&) = 0;

	/*********************************************************************************/
	/** @brief Custom halt condition for processing */
	virtual bool customHalt(){ return false; }

private:
	/*********************************************************************************/
	/**
	 * Checks whether a halt condition was reached. Either the maximum number of processing
	 * steps was reached or the maximum allowed time was reached.
	 *
	 * @return A boolean indicating whether a halt condition was reached
	 */
	bool halt(){
		using namespace boost::posix_time;

		// Maximum number of processing steps reached ?
		if(processMax_ && (processed_++ >= processMax_)) return true;

		// Maximum duration reached ?
		if(maxDuration_.total_microseconds() &&
		  ((microsec_clock::local_time() - startTime_) >= maxDuration_)) return true;

		// Custom halt condition reached ?
		if(customHalt()) return true;

		return false;
	}

	/*********************************************************************************/
	boost::posix_time::ptime startTime_; ///< Used to store the start time of the optimization
	boost::posix_time::time_duration maxDuration_; ///< Maximum time frame for the optimization

	boost::uint32_t processed_; ///< The number of processed items so far
	boost::uint32_t processMax_; ///< The maximum number of items to process

	bool returnRegardless_; ///< Specifies whether unsuccessful processing attempts should be returned to the server
};

/*********************************************************************************/

} /* namespace Courtier */
} /* namespace Gem */

#endif /* GBASECLIENTT_HPP_ */
