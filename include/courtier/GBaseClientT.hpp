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

#ifndef GBASECLIENTT_HPP_
#define GBASECLIENTT_HPP_

// Geneva headers go here
#include "common/GSerializationHelperFunctionsT.hpp"
#include "common/GLogger.hpp"
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
 * This class assumes that the template parameter implements the "process()" call.
 */
template<typename processable_type>
class GBaseClientT :
	private boost::noncopyable
{
	 // Make sure processable_type adheres to the GProcessingContainerT interface
	 static_assert(
		 std::is_base_of<Gem::Courtier::GProcessingContainerT<processable_type>, processable_type>::value
		 , "GBaseClientT: processable_type does not adhere to the GProcessingContainerT interface"
	 );

public:
	 /***************************************************************************/
	 /**
	  * The default constructor.
	  */
	 GBaseClientT()
	 { /* nothing*/ }

	 /***************************************************************************/
	 /**
	  * A constructor that accepts a model of the item to be processed. This can be
	  * used to avoid having to transfer or reload data that doesn't change. Note that
	  * the model must understand the clone() command.
	  *
	  * @param additionalDataTemplate The model of the item to be processed
	  */
	 GBaseClientT(std::shared_ptr <processable_type> additionalDataTemplate) :
		 m_additionalDataTemplate(additionalDataTemplate)
	 { /* nothing*/ }

	 /***************************************************************************/
	 /**
	  * A standard destructor. We have no local, dynamically allocated data, hence it is empty.
	  */
	 virtual ~GBaseClientT()
	 { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * Allows to set a maximum number of processing steps. If set to 0 or left unset,
	  * processing will be done until process() returns false.
	  *
	  * @param processMax Desired value for the m_processMax variable
	  */
	 void setProcessMax(const std::uint32_t &processMax)
	 {
		 m_processMax = processMax;
	 }

	 /***************************************************************************/
	 /**
	  * Retrieves the value of the m_processMax variable.
	  *
	  * @return The value of the m_processMax variable
	  */
	 std::uint32_t getProcessMax() const
	 {
		 return m_processMax;
	 }

	 /***************************************************************************/
	 /**
	  * Retrieves the number of items processed so far
	  */
	 std::uint32_t getNProcessed() const
	 {
		 return m_processed;
	 }

	 /***************************************************************************/
	 /**
	  * Sets the maximum allowed processing time
	  *
	  * @param maxDuration The maximum allowed processing time
	  */
	 void setMaxTime(const std::chrono::duration<double> &maxDuration)
	 {
		 m_maxDuration = maxDuration;
	 }

	 /***************************************************************************/
	 /**
	  * Retrieves the value of the m_maxDuration parameter.
	  *
	  * @return The maximum allowed processing time
	  */
	 std::chrono::duration<double> getMaxTime()
	 {
		 return m_maxDuration;
	 }

	 /***************************************************************************/
	 /**
	  * Specifies whether results should be returned regardless of the success achieved
	  * in the processing step.
	  *
	  * @param returnRegardless Specifies whether results should be returned to the server regardless of their success
	  */
	 void setReturnRegardless(const bool &returnRegardless)
	 {
		 m_returnRegardless = returnRegardless;
	 }

	 /***************************************************************************/
	 /**
	  * Checks whether results should be returned regardless of the success achieved
	  * in the processing step.
	  *
	  * @return Whether results should be returned to the server regardless of their success
	  */
	 bool getReturnRegardless() const
	 {
		 return m_returnRegardless;
	 }

	 /***************************************************************************/
	 /** @brief This is the main loop of the client */
	 virtual void run() BASE = 0;

protected:
	 /***************************************************************************/
	 /**
	  * Custom halt condition for processing
	  */
	 virtual bool customHalt()
	 { return false; }

	 /***************************************************************************/
	 /**
	  * Loads the additional data template into the processable_type target.
	  * This function needs to be called for each new item by derived classes.
	  */
	 void loadDataTemplate(std::shared_ptr<processable_type> target) {
		 // If we have a model for the item to be parallelized, load its data into the target
		 if(m_additionalDataTemplate) {
			 target->loadConstantData(m_additionalDataTemplate);
		 }
	 }

	 /***************************************************************************/
	 /**
	  * Checks whether a halt condition was reached. Either the maximum number of processing
	  * steps was reached or the maximum allowed time was reached.
	  *
	  * @return A boolean indicating whether a halt condition was reached
	  */
	 bool halt()
	 {
		 // Maximum number of processing steps reached ?
		 if (m_processMax > 0 && (m_processed++ >= m_processMax)) {
			 return true;
		 }

		 // Maximum duration reached ?
		 if (m_maxDuration.count() > 0. && ((std::chrono::system_clock::now() - m_startTime) >= m_maxDuration)) {
			 return true;
		 }

		 // Custom halt condition reached ?
		 if (customHalt()) {
			 return true;
		 }

		 return false;
	 }

private:
	 /***************************************************************************/

	 std::chrono::system_clock::time_point m_startTime = std::chrono::system_clock::now(); ///< Used to store the start time of the optimization
	 std::chrono::duration<double> m_maxDuration = std::chrono::microseconds(0); ///< Maximum time frame for the optimization

	 std::uint32_t m_processed = 0; ///< The number of processed items so far
	 std::uint32_t m_processMax = 0; ///< The maximum number of items to process

	 bool m_returnRegardless = true; ///< Specifies whether unsuccessful processing attempts should be returned to the server

	 std::shared_ptr <processable_type> m_additionalDataTemplate; ///< Optionally holds a template of the object to be processed
};

/******************************************************************************/


} /* namespace Courtier */
} /* namespace Gem */

#endif /* GBASECLIENTT_HPP_ */
