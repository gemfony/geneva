/**
 * @file GBrokerT.hpp
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

// Boost headers go here
#include <boost/utility.hpp>
#include <boost/function.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_serialize.hpp>


#ifndef GBROKERT_HPP_
#define GBROKERT_HPP_

// Geneva headers go here
#include "common/GExceptions.hpp"
#include "common/GLogger.hpp"
#include "common/GBoundedBufferT.hpp"
#include "common/GHelperFunctionsT.hpp"
#include "common/GSingletonT.hpp"
#include "courtier/GBaseConsumerT.hpp"
#include "courtier/GBufferPortT.hpp"
#include "courtier/GCourtierEnums.hpp"

namespace Gem {
namespace Courtier {

/******************************************************************************/
/** @brief Class to be thrown as a message in the case of a time-out in GBuffer */
class buffer_not_present : public std::exception
{
public:
	 buffer_not_present(void) throw() { /* nothing */ }
	 buffer_not_present(const buffer_not_present &) throw() { /* nothing */ }
	 ~buffer_not_present() throw() { /* nothing */ }
};

/******************************************************************************/
/**
 * This class acts as the main interface between producers and consumers.
 */
template<typename carrier_type>
class GBrokerT
	: private boost::noncopyable
{
	 using GBUFFERPORT = GBufferPortT<std::shared_ptr<carrier_type>>;
	 using GBUFFERPORT_PTR = typename std::shared_ptr<GBUFFERPORT>;
	 using RawBufferPtrMap = typename std::map<boost::uuids::uuid, GBUFFERPORT_PTR>;
	 using ProcessedBufferPtrMap = typename std::map<boost::uuids::uuid, GBUFFERPORT_PTR>;

public:
	 /***************************************************************************/
	 /**
	  * The default constructor.
	  */
	 GBrokerT()
		 : m_finalized(ATOMIC_FLAG_INIT) // false
		 , m_currentGetPosition(m_RawBuffers.begin())
		 , m_buffersPresent(ATOMIC_FLAG_INIT) // false
	 { /* nothing */ }

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
		 if (true == m_finalized.load()) return;

		 // Shut down all consumers
		 for(auto c: m_consumerCollection) {
			 c->shutdown();
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
			 m_consumerCollection.clear();
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
	  * to drop a GBufferPortT. This is simply done by letting the shared_ptr<GBufferPortT>
	  * go out of scope. As the producer holds the only copy, the GBufferPortT will then be
	  * deleted. A BufferPort contains two shared_ptr<GBoundedBufferT> objects. A shared_ptr
	  * to these objects is saved upon enrollment with the broker, so that letting the
	  * shared_ptr<GBufferPortT> go out of scope will not drop the shared_ptr<GBoundedBufferT>
	  * objects immediately. This is important, as there may still be active connections
	  * with items being collected from or dropped into them by the consumers. It is the
	  * task of this function to remove the orphaned shared_ptr<GBoundedBufferT> objects.
	  * It thus needs to block access to the entire object during its operation. Note that one of
	  * the effects of this function is that the buffer collections will never run empty,
	  * once the first buffer has been registered.
	  *
	  * @param gbp_ptr A shared pointer to a new GBufferPortT object
	  */
	 void enrol(std::shared_ptr<GBufferPortT<std::shared_ptr<carrier_type>>> gbp_ptr) {
		 {
			 //-----------------------------------------------------------------------
			 // Lock the access to our internal data simultaneously for all mutexs

			 std::unique_lock <std::mutex> switchGetPositionLock(
				 m_switchGetPositionMutex
				 , std::defer_lock
			 );
			 std::unique_lock <std::mutex> findProcessedBufferLock(
				 m_findProcesedBufferMutex
				 , std::defer_lock
			 );
			 std::lock(
				 switchGetPositionLock
				 , findProcessedBufferLock
			 );
			 //-----------------------------------------------------------------------

			 // Retrieve the uuid of the buffer port
			 boost::uuids::uuid gbp_tag = gbp_ptr->getUniqueTag();

			 // Find orphaned items in the two collections and remove them.
			 // Note that, unforunately, g++ < 5.0 does not support auto in lambda statements,
			 // otherwise the following statements could be simplified. We use references
			 // as lambda arguments, so the use count of the std::shared_ptr-objects isn't increased
			 Gem::Common::erase_if(
				 m_RawBuffers
				 , [](const std::pair<boost::uuids::uuid, GBUFFERPORT_PTR>& p) -> bool { return (p.second.use_count()==1); }
			 ); // m_RawBuffers is a std::map, so items are of type std::pair
			 Gem::Common::erase_if(
				 m_ProcessedBuffers
				 , [](const std::pair<boost::uuids::uuid, GBUFFERPORT_PTR>& p) -> bool { return (p.second.use_count()==1); }
			 ); // m_ProcessedBuffers is a std::map, so items are of type std::pair

			 // Attach the new items to the maps
			 m_RawBuffers[gbp_tag] = gbp_ptr;
			 m_ProcessedBuffers[gbp_tag] = gbp_ptr;

			 // Fix the current get-pointer. We simply attach it to the start of the list
			 m_currentGetPosition = m_RawBuffers.begin();

			 std::cout << "Buffer port with id " << gbp_tag << " successfully enrolled" << std::endl;
		 }

		 // If this was the first registered GBufferPortT object, we need to notify all
		 // available consumer objects. We use notify_all here, as indeed all need to be informed
		 // that there is work to do. We only check one variable, as both are set
		 // simultaneously.
		 if (false == m_buffersPresent.load()) {
			 m_buffersPresent.store(true);

			 m_readyToGoRaw.notify_all();
			 m_readyToGoProcessed.notify_all();
		 }
	 }

	 /***************************************************************************/
	 /**
	  * Adds a new consumer to this class and starts its thread.
	  *
	  * TODO: Check what happens if no consumer is registered anymore
	  *
	  * @param gc_ptr A pointer to a GBaseConsumerT<carrier_type> object
	  */
	 void enrol(std::shared_ptr<GBaseConsumerT<carrier_type>> gc_ptr) {
		 std::unique_lock<std::mutex> consumerEnrolmentLock(m_consumerEnrolmentMutex);

		 // Do nothing if a consumer of this type has already been registered
		 if (std::find(
			 m_consumerTypesPresent.begin()
			 , m_consumerTypesPresent.end()
			 , gc_ptr->getConsumerName()) != m_consumerTypesPresent.end()
		 ) {
			 return;
		 }

		 // Archive the consumer and its name, then start its thread
		 m_consumerCollection.push_back(gc_ptr);
		 m_consumerTypesPresent.push_back(gc_ptr->getConsumerName());

		 // Initiate processing in the consumer. This call will not block.
		 gc_ptr->async_startProcessing();
	 }

	 /***************************************************************************/
	 /**
	  * Retrieves a "raw" item from a GBufferPortT. This function will block
	  * if no item can be retrieved.
	  *
	  * TODO: Check what happens if no buffer port is registered anymore
	  *
	  * @param p Holds the retrieved "raw" item
	  */
	 void get(std::shared_ptr<carrier_type>& p) {
		 // Make sure we are dealing with an empty pointer
		 p.reset();

		 // Retrieve the current buffer port ...
		 GBUFFERPORT_PTR rawBuffer_ptr = getNextRawBufferPort();
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
	  * TODO: Check what happens if no buffer port is registered anymore
	  *
	  * @param p Holds the retrieved "raw" item
	  * @param timeout Time after which the function should time out
	  * @return A boolean which indicates whether the operation was successful
	  */
	 bool get(
		 std::shared_ptr<carrier_type> &p
		 , std::chrono::duration<double> timeout
	 ) {
		 // Make sure we are dealing with an empty pointer
		 p.reset();

		 // Retrieve the current buffer port ...
		 GBUFFERPORT_PTR rawBuffer_ptr = getNextRawBufferPort();
		 if(rawBuffer_ptr) {
			 // ... and get an item from it. This function is thread-safe.
		 	 rawBuffer_ptr->pop_raw(p, timeout); // Note that p might be empty
		 }

		 // If no raw buffer pointer was registered at the time
		 // of the getNextRawBufferPort()-call, p will be empty.

		 if(!p) {
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
		 std::shared_ptr<carrier_type> p
	 ) {
		 // Retrieve the correct processed buffer for a given uuid
		 boost::uuids::uuid portId = p->getBufferId();
		 GBUFFERPORT_PTR processedBuffer_ptr = getProcessedBufferPort(portId);

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
		 std::shared_ptr<carrier_type> p
		 , std::chrono::duration<double> timeout
	 ) {
		 // Retrieve the correct processed buffer for our uuid
		 boost::uuids::uuid portId = p->getBufferId();
		 GBUFFERPORT_PTR processedBuffer_ptr = getProcessedBufferPort(portId);

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

		 // Make the compiler happy
		 return false;
	 }

	 /***************************************************************************/
	 /**
	  * Checks whether any consumers have been enrolled at the time of calling. As
	  * consumers are maintained inside of a thread group and consumers may be added
	  * asynchronously, this function can only give a hint.
	  *
	  * @return A boolean indicating whether any consumers are registered
	  */
	 bool hasConsumers() const {
		 std::unique_lock<std::mutex> consumerEnrolmentLock(m_consumerEnrolmentMutex);
		 return m_consumerCollection.size() > 0 ? true : false;
	 }

	 /***************************************************************************/
	 /**
	  * This function checks all registered consumers to see whether all of them
	  * are capable of full return. If so, it returns true. If at least one is
	  * found that is not capable of full return, it returns false.
	  */
	 bool capableOfFullReturn() const {
		 std::unique_lock<std::mutex> consumerEnrolmentLock(m_consumerEnrolmentMutex);

		 if (!m_consumerCollection.empty()) {
			 glogger
				 << "In GBrokerT<carrier_type>::capableOfFullReturn(): Error!" << std::endl
				 << "No consumers registered" << std::endl
				 << GEXCEPTION;
		 }

		 bool result = true;
		 typename std::vector<std::shared_ptr<GBaseConsumerT<carrier_type>>>::const_iterator cit;
		 for (cit = m_consumerCollection.begin(); cit != m_consumerCollection.end(); ++cit) {
			 if (!(*cit)->capableOfFullReturn()) {
				 result = false;
				 break; // stop the loop
			 }
		 }

		 return result;
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

		 if (!m_RawBuffers.empty()) {
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
	  * Retrieves the processed buffer pointer for a given uuid. As we are dealing
	  * with a (not thread-safe) std::map, we need to coordinate the access.
	  */
	 GBUFFERPORT_PTR getProcessedBufferPort(boost::uuids::uuid uuid) {
		 // Protect access to the map
		 std::unique_lock<std::mutex> findProcessedBufferLock(m_findProcesedBufferMutex);

		 // Find the buffer port (if any)
		 auto it = m_ProcessedBuffers.find(uuid);
		 if (it != m_ProcessedBuffers.end()) {
			 return it->second;
		 }

		 // Return an empty pointer
		 return GBUFFERPORT_PTR();
	 }

	 /***************************************************************************/

	 GBrokerT(const GBrokerT<carrier_type> &) = delete; ///< Intentionally left undefined
	 const GBrokerT &operator=(const GBrokerT<carrier_type> &) = delete; ///< Intentionally left undefined

	 /***************************************************************************/
	 // Data

	 std::atomic<bool> m_finalized; ///< Indicates whether the finalization code has already been executed

	 mutable std::mutex m_consumerEnrolmentMutex; ///< Protects the enrolment of consumers
	 mutable std::mutex m_switchGetPositionMutex; ///< Protects switches to the next get position
	 mutable std::mutex m_findProcesedBufferMutex; ///< Protects finding a given processed buffer

	 mutable std::condition_variable m_readyToGoRaw; ///< The get function will block until this condition variable is set
	 mutable std::condition_variable m_readyToGoProcessed; ///< The put function will block until this condition variable is set

	 RawBufferPtrMap m_RawBuffers; ///< Holds a std::map of buffer pointers
	 ProcessedBufferPtrMap m_ProcessedBuffers; ///< Holds a std::map of buffer pointers

	 typename RawBufferPtrMap::iterator m_currentGetPosition; ///< The current get position in the m_RawBuffers collection
	 std::atomic<bool> m_buffersPresent; ///< Set to true once the first buffers have been enrolled

	 std::vector<std::shared_ptr<GBaseConsumerT<carrier_type>>> m_consumerCollection; ///< Holds the actual consumers
	 std::vector<std::string> m_consumerTypesPresent; ///< Holds identifying strings for each consumer
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

} /* namespace Courtier */
} /* namespace Gem */

#endif /* GBROKERT_HPP_ */
