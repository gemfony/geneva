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
#include <sstream>
#include <vector>
#include <list>
#include <map>
#include <string>
#include <utility>
#include <limits>
#include <stdexcept>
#include <algorithm>
#include <functional>
#include <limits>
#include <chrono>
#include <mutex>
#include <thread>
#include <memory>
#include <condition_variable>
#include <stdexcept>

// Boost headers go here
#include <boost/utility.hpp>
#include <boost/lexical_cast.hpp>

// Geneva headers go here
#include "common/GExceptions.hpp"
#include "common/GLogger.hpp"
#include "common/GErrorStreamer.hpp"
#include "common/GBoundedBufferT.hpp"
#include "common/GCommonHelperFunctionsT.hpp"
#include "common/GSingletonT.hpp"
#include "courtier/GBaseConsumerT.hpp"
#include "courtier/GBufferPortT.hpp"
#include "courtier/GCourtierEnums.hpp"
#include "courtier/GProcessingContainerT.hpp"

namespace Gem::Courtier {

/******************************************************************************/
/** @brief Exception to be thrown as a message in the case of a time-out in GBrokerT */
class buffer_not_present : public std::exception
{
public:
	 using std::exception::exception;
};

/******************************************************************************/
/**
 * This class acts as the main mediator between producers and consumers.
 */
template<typename processable_type>
class GBrokerT {
	 // Make sure processable_type adheres to the GProcessingContainerT interface
	 static_assert(
		 std::is_base_of<Gem::Courtier::GProcessingContainerT<processable_type, typename processable_type::result_type>, processable_type>::value
		 , "GBaseExecutorT: processable_type does not adhere to the GProcessingContainerT<> interface"
	 );

	 // Syntactic sugar
	 using GBUFFERPORT = GBufferPortT<processable_type>;
	 using GBUFFERPORT_PTR = typename std::shared_ptr<GBUFFERPORT>;
	 using RawBufferPtrMap = typename std::map<BUFFERPORT_ID_TYPE, GBUFFERPORT_PTR>;
	 using ProcessedBufferPtrMap = typename std::map<BUFFERPORT_ID_TYPE, GBUFFERPORT_PTR>;

public:
	 /***************************************************************************/
	 /**
	  * The standard destructor. Notifies all consumers that they should stop, then waits
	  * for their threads to terminate.
	  */
	 ~GBrokerT() {
		 // Make sure the finalization code is executed
		 // (if this hasn't happened already). Calling
		 // finalize() multiple times is safe.
		 finalize();
	 }

	 /***************************************************************************/
	 // Defaulted or deleted constructors and assignment operators.
	 // This class should be noncopyable.

	 GBrokerT() = default;

	 GBrokerT(const GBrokerT<processable_type>&) = delete;
	 GBrokerT(GBrokerT<processable_type>&&) = delete;

	 GBrokerT<processable_type>& operator=(const GBrokerT<processable_type>&) = delete;
	 GBrokerT<processable_type>& operator=(GBrokerT<processable_type>&&) = delete;

	 /***************************************************************************/
	 /**
	  * Initializes the broker. This function does nothing. Its only purpose is to control
	  * initialization of the factory in the singleton.
	  */
	 void init() { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * Shuts the broker down, together with all consumers.
	  */
	 void finalize() {
		 // Only allow one finalization action to be carried out
		 if (m_finalized) return;

		 // Shut down all consumers
		 for(auto const& c_ptr: m_consumer_collection_cnt) {
			 c_ptr->shutdown();
		 }

		 {
			 //-----------------------------------------------------------------------
			 // Lock the access to our internal data simultaneously for all mutexs
			 std::unique_lock<std::mutex> switchGetPositionLock(
				 m_switchGetPositionMutex
				 , std::defer_lock
			 );
			 std::unique_lock<std::mutex> findProcessedBufferLock(
				 m_findProcesedBufferMutex
				 , std::defer_lock
			 );
			 std::unique_lock<std::mutex> consumerEnrolmentLock(
				 m_consumerEnrolmentMutex
				 , std::defer_lock
			 );

			 std::lock(
				 switchGetPositionLock
				 , findProcessedBufferLock
				 , consumerEnrolmentLock
			 );
			 //-----------------------------------------------------------------------

			 // Clear raw and processed buffers and the consumer lists
			 m_RawBuffers.clear();
			 m_ProcessedBuffers.clear();
			 m_consumer_collection_cnt.clear();
			 m_buffersPresent.store(false);

			 // Make sure this function does not execute code a second time
			 m_finalized.store(true);
		 }
	 }

	 /***************************************************************************/
	 /**
	  * This function is used by producers to register a new GBufferPortT object
	  * with the broker. A GBufferPortT object contains bounded buffers for raw (i.e.
	  * unprocessed) items and for processed items. A producer may at any time decide
	  * to drop a GBufferPortT, but needs to indicate this fact to the buffer port. It is the
	  * task of this function to remove the orphaned shared_ptr<GBufferPortT> pointers.
	  * It thus needs to block access to the entire object during its operation. Note that one of
	  * the effects of this function is that the buffer collections will never run empty,
	  * once the first buffer has been registered.
	  *
	  * @param gbp_ptr A shared pointer to a new GBufferPortT object
	  * @return A boolean which indicates, whether all consumers are capable of full return
	  */
	 bool enrol_buffer_port(std::shared_ptr<GBufferPortT<processable_type>> gbp_ptr) {
		 //-----------------------------------------------------------------------
		 // Lock the access to our internal data simultaneously for all mutexes

		 std::unique_lock <std::mutex> switchGetPositionLock(
			 m_switchGetPositionMutex
			 , std::defer_lock
		 );
		 std::unique_lock <std::mutex> findProcessedBufferLock(
			 m_findProcesedBufferMutex
			 , std::defer_lock
		 );
		 std::lock( // Lock both locks simultaneously
			 switchGetPositionLock
			 , findProcessedBufferLock
		 );

		 //-----------------------------------------------------------------------
		 // Find orphaned items in the two collections and remove them.
		 // Note that, unforunately, g++ < 5.0 does not support auto in lambda statements,
		 // otherwise the following statements could be simplified.
		 std::size_t nErasedRaw = Gem::Common::erase_if(
			 m_RawBuffers
			 , [](const std::pair<BUFFERPORT_ID_TYPE, GBUFFERPORT_PTR>& p) -> bool { return (not p.second->is_connected_to_producer()); }
		 ); // m_RawBuffers is a std::map, so items are of type std::pair

#ifdef DEBUG
		 if(nErasedRaw > 0 ) {
			 glogger
				 << "In GBrokerT<>::enrol_buffer_port(buffer-port-ptr): Removed " << nErasedRaw << " raw buffers" << std::endl
				 << GLOGGING;
		 }
#endif

		 std::size_t nErasedProc = Gem::Common::erase_if(
			 m_ProcessedBuffers
			 , [](const std::pair<BUFFERPORT_ID_TYPE, GBUFFERPORT_PTR>& p) -> bool { return (not p.second->is_connected_to_producer()); }
		 ); // m_ProcessedBuffers is a std::map, so items are of type std::pair

#ifdef DEBUG
	    if(nErasedProc != nErasedRaw) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG, time_and_place)
					 << "In GBrokerT<>::enrol_buffer_port(buffer-port-ptr):" << std::endl
				    << "nErasedProc (" << nErasedProc << ") != nErasedRaw (" << nErasedRaw << ")" << std::endl
			 );
	    }

		 if(nErasedProc > 0 ) {
			 glogger
				 << "In GBrokerT<>::enrol_buffer_port(buffer-port-ptr): Removed " << nErasedProc << " processed buffers" << std::endl
				 << GLOGGING;
		 }
#endif

		 // Update the number of registered buffer ports
#ifdef DEBUG
		 if(BUFFERPORT_ID_TYPE(nErasedRaw) > m_n_registered_buffer_ports) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG, time_and_place)
					 << "In GBrokerT<>::enrol_buffer_port(buffer-port-ptr):" << std::endl
					 << "nErasedRaw (" << nErasedRaw << ") > m_n_registered_buffer_ports (" << m_n_registered_buffer_ports << ")" << std::endl
			 );
		 }
#endif
		 m_n_registered_buffer_ports -= BUFFERPORT_ID_TYPE(nErasedRaw);

		 // Retrieve a new id for the buffer port.
		 auto gbp_tag = getNextBufferPortId();

		 // Register the id with the buffer port
		 gbp_ptr->set_port_tag(gbp_tag);

		 // Attach the new items to the maps
		 m_RawBuffers[gbp_tag] = gbp_ptr;
		 m_ProcessedBuffers[gbp_tag] = gbp_ptr;

	    // Increment the number of registered buffer ports and check if we have exceeded the allowed amound
		 if(++m_n_registered_buffer_ports > MAXREGISTEREDBUFFERPORTS) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG, time_and_place)
					 << "In GBrokerT<>::enrol_buffer_port(buffer-port-ptr):" << std::endl
					 << "Maximum number " << MAXREGISTEREDBUFFERPORTS << " of registered buffer ports exceeded" << std::endl
			 );
		 }

		 // Fix the current get-pointer. We simply attach it to the start of the list
		 m_currentGetPosition = m_RawBuffers.begin();

		 std::cout << "Buffer port with id " << gbp_tag << " successfully enrolled" << std::endl;

		 // Let the audience know
		 m_buffersPresent.store(true);

		 // Let the audience know whether all consumers are capable of full return
		 return m_capable_of_full_return;
	 }

	 /***************************************************************************/
	 /**
	  * Adds a new consumer to this class and starts its thread.
	  *
	  * @param gc_ptr A pointer to a GBaseConsumerT<processable_type> object
	  */
	 void enrol_consumer(std::shared_ptr<GBaseConsumerT<processable_type>> gc_ptr) {
		 //-----------------------------------------------------------------------
		 // Check whether consumers have already been enrolled. As this may happen
		 // only once, we emit a warning and return
		 if(m_consumersPresent) {
			 glogger
				 << "In GBrokerT<>::enrol_buffer_port(consumer_ptr): One or more consumers have already been enrolled." << std::endl
				 << "We will ignore the new enrolment request." << std::endl
				 << GWARNING;

			 return;
		 }

		 //-----------------------------------------------------------------------
		 std::unique_lock<std::mutex> consumerEnrolmentLock(m_consumerEnrolmentMutex);

		 // Do nothing if a consumer of this type has already been registered
		 if (std::find(
			 m_consumerTypesPresent.begin()
			 , m_consumerTypesPresent.end()
			 , gc_ptr->getConsumerName()) != m_consumerTypesPresent.end()
		 ) {
			 glogger
			 << "In GBrokerT<>::enrol_buffer_port(consumer):" << std::endl
			 << "Consumer with name " << gc_ptr->getConsumerName() << " aleady exists." << std::endl
			 << "We will ignore the new enrolment request." << std::endl
		    << GWARNING;

			 return;
		 }

		 // Archive the consumer and its name, then start its thread
		 m_consumer_collection_cnt.push_back(gc_ptr);
		 m_consumerTypesPresent.push_back(gc_ptr->getConsumerName());

		 // Initiate processing in the consumer. This call will not block.
		 gc_ptr->async_startProcessing();

		 //-----------------------------------------------------------------------
		 // Make it known to subsequent calls that a consumer is already present

		 m_consumersPresent.store(true);

		 //-----------------------------------------------------------------------

		 // Check whether all registered consumers are capable of full return
		 m_capable_of_full_return.store(this->checkConsumersCapableOfFullReturn());

		 // Notify all interested parties
		 m_consumersEnrolledCondition.notify_all();

		 //-----------------------------------------------------------------------
	 }

	 /***************************************************************************/
	 /**
	  * Adds multiple consumers to this class and starts their threads.
	  *
	  * @param gc_ptr_cnt A vector of pointers to GBaseConsumerT<processable_type> objects
	  */
	 void enrol_consumer_vec(std::vector<std::shared_ptr<GBaseConsumerT<processable_type>>> gc_ptr_cnt) {
		 //-----------------------------------------------------------------------
		 // Check whether consumers have already been enrolled. As this may happen
		 // only once, we emit a warning and return
		 if(m_consumersPresent) {
			 glogger
				 << "In GBrokerT<>::enrol_buffer_port(consumer_ptr_vec): One or more consumers have already been enrolled." << std::endl
				 << "We will ignore the new enrolment request." << std::endl
				 << GWARNING;

			 return;
		 }

		 //-----------------------------------------------------------------------
		 std::unique_lock<std::mutex> consumerEnrolmentLock(m_consumerEnrolmentMutex);

		 for(auto const& consumer_ptr: gc_ptr_cnt) {
			 // Do nothing if a consumer of this type has already been registered
			 if (std::find(
				 m_consumerTypesPresent.begin()
				 , m_consumerTypesPresent.end()
				 , consumer_ptr->getConsumerName()) != m_consumerTypesPresent.end()
			 ) {
				 glogger
					 << "In GBrokerT<>::enrol_buffer_port(consumer_ptr_vec): A consumer with name " << consumer_ptr->getConsumerName() << std::endl
				    << "has already been enrolled. We will ignore the new enrolment request." << std::endl
					 << GWARNING;

				 continue;
			 }

			 // Archive the consumer and its name, then start its thread
			 m_consumer_collection_cnt.push_back(consumer_ptr);
			 m_consumerTypesPresent.push_back(consumer_ptr->getConsumerName());

			 // Initiate processing in the consumer. This call will not block.
			 consumer_ptr->async_startProcessing();
		 }

		 //-----------------------------------------------------------------------
		 // Make it known to subsequent calls that a consumer is already present

		 m_consumersPresent.store(true);

		 //-----------------------------------------------------------------------

		 // Check whether all registered consumers are capable of full return
		 m_capable_of_full_return.store(this->checkConsumersCapableOfFullReturn());

		 // Notify all interested parties
		 m_consumersEnrolledCondition.notify_all();

		 //-----------------------------------------------------------------------
	 }

	 /***************************************************************************/
	 /**
	  * Retrieves a "raw" item from a GBufferPortT. This function will block
	  * if no item can be retrieved.
	  *
	  * @param p Holds the retrieved "raw" item
	  */
	 void get(std::shared_ptr<processable_type>& p) {
		 // Make sure we are dealing with an empty pointer
		 p.reset();

		 // Retrieve the current buffer port ...
		 auto rawBuffer_ptr = getNextRawBufferPort();
		 if(rawBuffer_ptr) {
			 // ... and get an item from it. This function is thread-safe.
			 rawBuffer_ptr->pop_raw(p);
		 }

		 // If no raw buffer pointer was registered at the time
		 // of the getNextRawBufferPort()-call, p will be empty.
	 }

	 /***************************************************************************/
	 /**
	  * Retrieves a "raw" item from a GBufferPortT, observing a timeout. Note that upon
	  * time-out an exception is thrown.
	  *
	  * @param p Holds the retrieved "raw" item
	  * @param timeout Time after which the function should time out
	  * @return A boolean which indicates whether the operation was successful
	  */
	 bool get(
		 std::shared_ptr<processable_type> &p
		 , std::chrono::duration<double> timeout
	 ) {
		 // Make sure we are dealing with an empty pointer
		 p.reset();

		 // Retrieve the current buffer port ...
		 auto rawBuffer_ptr = getNextRawBufferPort();
		 if(rawBuffer_ptr) {
			 // ... and get an item from it. This function is thread-safe.
		 	 rawBuffer_ptr->pop_raw(p, timeout); // Note that p might be empty
		 }

		 // If no raw buffer pointer was registered at the time
		 // of the getNextRawBufferPort()-call, p will be empty.

		 if(not p) {
			 return false;
		 }

		 return true;
	 }

	 /***************************************************************************/
	 /**
	  * Puts a processed item into the processed queue. Note that the item will simply
	  * be discarded if no target queue with the required id exists. The function will
	  * block otherwise, until it is again possible to submit the item.
	  *
	  * @param p Holds the "raw" item to be submitted to the processed queue
	  */
	 void put(
		 std::shared_ptr<processable_type> p
	 ) {
		 // Retrieve the correct processed buffer for a given id
		 auto portId = p->getBufferId();
		 auto processedBuffer_ptr = getProcessedBufferPort(portId);

		 // Submit the item
		 if(processedBuffer_ptr) {
			 // This function is thread-safe.
			 processedBuffer_ptr->push_processed(p);
		 } else {
			 glogger
				 << "In GBokerT<>::put(1): Warning!" << std::endl
				 << "Did not find buffer with id " << portId << "." << std::endl
				 << "Item will be discarded" << std::endl
				 << GWARNING;

			 throw Gem::Courtier::buffer_not_present();
		 }
	 }

	 /***************************************************************************/
	 /**
	  * Puts a processed item into the processed queue, observing a timeout. The function
	  * will throw a Gem::Courtier::buffer_not_present exception if the requested buffer
	  * isn't present. The function will return false if no item could be added to the buffer
	  * inside if the allowed time limits.
	  *
	  * @param id A key that uniquely identifies the origin of p
	  * @param p Holds the item to be submitted to the processed queue
	  * @param timeout Time after which the function should time out
	  * @param A boolean indicating whether the item could be added to the queue in time
	  */
	 bool put(
		 std::shared_ptr<processable_type> p
		 , std::chrono::duration<double> timeout
	 ) {
		 // Retrieve the correct processed buffer for our id
		 auto portId = p->getBufferId();
		 auto processedBuffer_ptr = getProcessedBufferPort(portId);

		 // Submit the item
		 if(processedBuffer_ptr) {
			 // This function is thread-safe.
			 return processedBuffer_ptr->push_processed(p, timeout);
		 } else {
			 glogger
				 << "In GBokerT<>::put(1): Warning!" << std::endl
				 << "Did not find buffer with id " << portId << "." << std::endl
				 << "Item will be discarded" << std::endl
				 << GWARNING;

			 throw Gem::Courtier::buffer_not_present();
		 }
	 }

	 /***************************************************************************/
	 /**
	  * Checks whether any consumers have been enrolled at the time of calling.
	  *
	  * @return A boolean indicating whether any consumers are registered
	  */
	 bool hasConsumers() const {
		 return m_consumersPresent;
	 }

	 /***************************************************************************/
	 /**
	  * This function relies on a prior check during the enrolment process whether
	  * all registered consumers are capable of full return. It will block until
	  * a consumer has been registered. The lock will be either released by the
	  * condition variable or when the function is left, so enrolling of consumers
	  * is not prevented.
	  */
	 bool capableOfFullReturn() const {
		 std::unique_lock<std::mutex> consumerEnrolmentLock(m_consumerEnrolmentMutex);
		 if(not m_consumersPresent) {
			 m_consumersEnrolledCondition.wait(
				 consumerEnrolmentLock
				 , [this]() -> bool { return this->hasConsumers(); }
			 );
		 }

		 return m_capable_of_full_return;
	 }

private:
	 /***************************************************************************/
	 /**
	  * Retrieves the next raw buffer port pointer. As we are dealing with a
	  * (not thread-safe) std::map, we need to coordinate the access.
	  */
	 GBUFFERPORT_PTR getNextRawBufferPort() {
		 // Protect access to the iterator
		 std::unique_lock<std::mutex> switchGetPositionLock(m_switchGetPositionMutex);

		 if (not m_RawBuffers.empty()) {
			 // Save the current get position
			 auto currentGetPosition = m_currentGetPosition;

			 // Switch to the next position, if any
			 if ((m_RawBuffers.size() > 1) && (++m_currentGetPosition == m_RawBuffers.end())) {
				 m_currentGetPosition = m_RawBuffers.begin();
			 }

			 // Return the shared_ptr. This will also keep the buffer port alive
			 return currentGetPosition->second;
		 } else {
			 return GBUFFERPORT_PTR();
		 }
	 }

	 /***************************************************************************/
	 /**
	  * Retrieves the processed buffer pointer for a given id. As we are dealing
	  * with a (possibly thread-unsafe) std::map, we need to synchronize the access.
	  */
	 GBUFFERPORT_PTR getProcessedBufferPort(BUFFERPORT_ID_TYPE id) {
		 // Protect access to the map
		 std::unique_lock<std::mutex> findProcessedBufferLock(m_findProcesedBufferMutex);

		 // Find the buffer port (if any)
		 try {
			 return m_ProcessedBuffers.at(id);
		 } catch(const std::out_of_range&) {
			 // Return an empty pointer
			 return GBUFFERPORT_PTR();
		 }
	 }

	 /***************************************************************************/
	 /**
	  * Checks if all registered consumers are capable of full return. This
	  * function is not thread-safe and must be called in a controlled environment.
	  *
	  * @return A boolean indicating whether all registered consumers are capable of full return
	  */
	 bool checkConsumersCapableOfFullReturn() {
		 if (m_consumer_collection_cnt.empty()) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG, time_and_place)
					 << "In GBrokerT<processable_type>::checkConsumersCapableOfFullReturn(): Error!" << std::endl
					 << "No consumers registered" << std::endl
			 );
		 }

		 bool capable_of_full_return = true;
		 for(auto const& item_ptr: m_consumer_collection_cnt) {
			 if (not item_ptr->capableOfFullReturn()) {
				 capable_of_full_return = false;
				 break; // stop the loop
			 }
		 }

		 return capable_of_full_return;
	 }


	 /***************************************************************************/
	 /**
	  * Retrieval of an id to be assigned to the next registered buffer port.
	  * This function is only used within a locked environment, so we can safely
	  * increment and compare the id. As the id may roll over and
	  * buffer ports may have different lifetimes, we need to make sure the chosen
	  * buffer port id hasn't been used yet. We do this by searching through the
	  * list of registered buffer ports
	  */
	 BUFFERPORT_ID_TYPE getNextBufferPortId() {
		 bool id_in_use = false;
		 BUFFERPORT_ID_TYPE next_id = m_current_bufferport_id;

		 do {
			 id_in_use = false;
			 next_id = m_current_bufferport_id;

			 // Reset the id, if we have reached the maximum size
			 if(++m_current_bufferport_id == (std::numeric_limits<BUFFERPORT_ID_TYPE>::max)()) {
				 m_current_bufferport_id = 0;
			 }

			 // Check if the id is already being used
			 for (const auto &port: m_RawBuffers) {
				 if (port.first == next_id) {
					 id_in_use = true;
					 break;
				 }
			 }
		 } while(id_in_use);

		 return next_id;
	 }

	 /***************************************************************************/
	 // Data

	 std::atomic<bool> m_finalized{false}; ///< Indicates whether the finalization code has already been executed

	 mutable std::mutex m_consumerEnrolmentMutex; ///< Protects the enrolment of consumers
	 mutable std::mutex m_switchGetPositionMutex; ///< Protects switches to the next get position
	 mutable std::mutex m_findProcesedBufferMutex; ///< Protects finding a given processed buffer

	 mutable std::condition_variable m_consumersEnrolledCondition; ///< Allows to notify interested parties once consumers have been enrolled

	 RawBufferPtrMap m_RawBuffers; ///< Holds a std::map of buffer pointers
	 ProcessedBufferPtrMap m_ProcessedBuffers; ///< Holds a std::map of buffer pointers

	 typename RawBufferPtrMap::iterator m_currentGetPosition{m_RawBuffers.begin()}; ///< The current get position in the m_RawBuffers collection
	 std::atomic<bool> m_buffersPresent{false}; ///< Set to true once the first buffers have been enrolled

	 std::atomic<bool> m_consumersPresent{false}; ///< Set to true once one or more consumers have been enrolled
	 std::atomic<bool> m_capable_of_full_return{false}; ///< Set to true if all registered consumers are capable of full return, otherwise false

	 std::vector<std::shared_ptr<GBaseConsumerT<processable_type>>> m_consumer_collection_cnt; ///< Holds the actual consumers
	 std::vector<std::string> m_consumerTypesPresent; ///< Holds identifying strings for each consumer

	 std::atomic<BUFFERPORT_ID_TYPE> m_current_bufferport_id{BUFFERPORT_ID_TYPE(0)}; ///< The id assigned to the last registered buffer port
	 std::atomic<BUFFERPORT_ID_TYPE> m_n_registered_buffer_ports{BUFFERPORT_ID_TYPE(0)}; ///< The current number of registered buffer ports
};

/******************************************************************************/
/**
 * We require GBrokerT<T> to be a singleton. This ensures that, for a given T, one
 * and only one Broker object exists that is constructed before main begins. All
 * external communication should refer to GBROKER(T).
 */
#define GBROKER(T)      Gem::Common::GSingletonT<Gem::Courtier::GBrokerT<T>>::Instance(0)
#define RESETGBROKER(T) Gem::Common::GSingletonT<Gem::Courtier::GBrokerT<T>>::Instance(1)

/******************************************************************************/

} // namespace Gem::Courtier

