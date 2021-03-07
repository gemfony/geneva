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

// Standard header files go here
#include <sstream>
#include <thread>
#include <chrono>
#include <atomic>
#include <memory>
#include <mutex>

// Boost header files go here
#include <boost/utility.hpp>

// Geneva header files go here
#include "courtier/GCourtierEnums.hpp"
#include "courtier/GProcessingContainerT.hpp"
#include "common/GCommonHelperFunctionsT.hpp"
#include "common/GBoundedBufferT.hpp"

namespace Gem::Courtier {

// Forward declaration of GBrokerT
template <typename processable_type> class GBrokerT;

/******************************************************************************/
/**
 * A GBufferPortT<processable_type> consists of two GBoundedBufferT<std::shared_ptr<processable_type>>
 * objects, one intended for "raw" items, the other for returning, processed items. While this class could
 * be useful in many scenarios, the most common application is as a mediator
 * between optimization algorithms and GConsumer-derivatives. The optimization algorithm
 * is a source of raw items, which are processed by GConsumer-derivatives
 * (such as GBoostThreadConsumer and GAsioTCPConsumerT) and then returned to the
 * population. GBrokerT instantiations orchestrate this exchange.
 * All of this happens in a multi-threaded environment. It is not possible to
 * create copies of this class, as one GBufferPortT is intended to serve one
 * single population.
 */
template<typename processable_type>
class GBufferPortT
{
	 // Make sure processable_type adheres to the GProcessingContainerT interface
	 static_assert(
		 std::is_base_of<Gem::Courtier::GProcessingContainerT<processable_type, typename processable_type::result_type>, processable_type>::value
		 , "GBufferPortT: processable_type does not adhere to the GProcessingContainerT<> interface"
	 );

	 // We want GBrokerT to be the only class to be able to set our ID, so we declare it as friend
	 friend class GBrokerT<processable_type>;

	 using RAW_BUFFER_TYPE = typename Gem::Common::GBoundedBufferT<std::shared_ptr<processable_type>, Gem::Common::DEFAULTBUFFERSIZE>;
	 using PROCESSED_BUFFER_TYPE = typename Gem::Common::GBoundedBufferT<std::shared_ptr<processable_type>, 0>;

public:
	 /***************************************************************************/
	 // Defaulted or deleted constructors, destructor and assignment operators

	 GBufferPortT() = default;

	 ~GBufferPortT() = default;

	 GBufferPortT(GBufferPortT<processable_type> const&) = delete;
	 GBufferPortT(GBufferPortT<processable_type> &&) = delete;

	 GBufferPortT<processable_type>& operator=(GBufferPortT<processable_type> const&) = delete;
	 GBufferPortT<processable_type>& operator=(GBufferPortT<processable_type> &&) = delete;

	 /***************************************************************************/
	 /**
	  * Puts an item into the raw queue. This is the queue for "raw" objects.
	  * This function will block until the item was submitted.
	  *
	  * @param item_ptr A raw object that needs to be processed, wrapped into a std::shared_ptr
	  * @return A boolean which indicates whether the submission was successful
	  */
	 void push_raw(std::shared_ptr<processable_type> item_ptr) {
		 if(item_ptr) {
			 // Make it known to the work item when it has left its origin
			 // This timing may be wrong if the submission has blocked.
			 item_ptr->markRawSubmissionTime();
			 // The actual submission
			 m_raw_ptr->push_and_block_copy(item_ptr);
		 }
	 }

	 /***************************************************************************/
	 /**
	  * Timed version of GBufferPortT::push_raw() . If the item could not be added
	  * after a given amount of time, the function returns false.
	  *
	  * @param item_ptr An item to be added to the buffer
	  * @param timeout duration until a timeout occurs
	  * @return A boolean which indicates whether the submission was successful
	  */
	 bool push_raw(
		 std::shared_ptr<processable_type> item_ptr
		 , const std::chrono::duration<double> &timeout
	 ) {
		 bool success = false;
		 if(item_ptr) {
			 // Make it known to the work item when it has left its origin
			 item_ptr->markRawSubmissionTime();
			 // The actual submission
			 success = m_raw_ptr->push_and_wait_copy(item_ptr, timeout);

#ifdef DEBUG
			 // Items may be lost here. This should be a very rare occasion. Emit
			 // a warning in DEBUG mode, as this might hint at some general problem
			 if(not success) {
				 glogger
					 << "In GBufferPortT<processable_type>::push_raw(item_ptr, timeout):" << std::endl
					 << "Submission was not successful. The work item might be discarded." << std::endl
					 << "Timeout was " << timeout.count() << " seconds" << std::endl
					 << GWARNING;
			 }
#endif
		 }

		 return success;
	 }

	 /***************************************************************************/
	 /**
	  * Retrieves an item from the back of the "m_raw_ptr" queue. Blocks until
	  * an item could be retrieved. This function will block until the item was submitted.
	  *
	  * @param item_ptr A reference to the item to be retrieved
	  */
	 void pop_raw(std::shared_ptr<processable_type> &item_ptr) {
		 // Do the actual retrieval
		 m_raw_ptr->pop_and_block_copy(item_ptr);

		 if(item_ptr) {
			 // Make it known to the work item when it was taken from the raw queue for processing
			 item_ptr->markRawRetrievalTime();
		 }

		 // If this is the first retrieval, mark the time for later usage
		 if(m_no_retrieval && item_ptr) {
			 std::unique_lock<std::mutex> lock(m_first_retrieval_mutex);
			 if(m_no_retrieval) {
				 m_retrieval_start_time = std::chrono::high_resolution_clock::now();
				 m_no_retrieval = false;
				 m_retrievalTimeCondition.notify_all();
			 }
		 }
	 }

	 /***************************************************************************/
	 /**
	  * A version of GBufferPortT::pop_raw() with the ability to time-out. False
	  * will be returned if no item could be popped.
	  *
	  * @param item_ptr The item that was retrieved from the queue
	  * @param timeout duration until a timeout occurs
	  * @return A boolean which indicates whether the retrieval was successful
	  */
	 bool pop_raw(
		 std::shared_ptr<processable_type> &item_ptr
		 , const std::chrono::duration<double> &timeout
	 ) {
		 // Do the actual retrieval
		 bool success = m_raw_ptr->pop_and_wait_copy(
			 item_ptr
			 , timeout
		 );
		 if(success && item_ptr) {
			 // Make it known to the work item when it has returned to its origin
			 item_ptr->markRawRetrievalTime();
		 }

		 // If this is the first retrieval, mark the time for later usage
		 if(m_no_retrieval && item_ptr) {
			 std::unique_lock<std::mutex> lock(m_first_retrieval_mutex);
			 if(m_no_retrieval) {
				 m_retrieval_start_time = std::chrono::high_resolution_clock::now();
				 m_no_retrieval = false;
				 m_retrievalTimeCondition.notify_all();
			 }
		 }

		 return success;
	 }

	 /***************************************************************************/
	 /**
	  * Puts an item into the "processed" queue. This function will block until the item was submitted.
	  *
	  * @param item A raw object that needs to be processed
	  * @return A boolean which indicates whether the submission was successful
	  */
	 void push_processed(std::shared_ptr<processable_type> item_ptr) {
		 if(item_ptr) {
			 // Make it known to the work item when it has entered the processed queue.
			 // This timing may be wrong if the submission has blocked.
			 item_ptr->markProcSubmissionTime();
			 // The actual submission
			 m_processed_ptr->push_and_block_copy(item_ptr);
		 }
	 }

	 /***************************************************************************/
	 /**
	  * Timed version of GBufferPortT::push_processed() . If the item could not
	  * be added after a given amount of time, false will be returned
	  *
	  * @param item_ptr An item to be added to the buffer
	  * @param timeout duration until a timeout occurs
	  * @return A boolean which indicates whether the submission was successful
	  */
	 bool push_processed(
		 std::shared_ptr<processable_type> item_ptr
		 , const std::chrono::duration<double> &timeout
	 ) {
		 bool success = false;
		 if(item_ptr) {
			 // Make it known to the work item when it has entered the processed queue
			 item_ptr->markProcSubmissionTime();
			 // The actual submission
			 success = m_processed_ptr->push_and_wait_copy(item_ptr, timeout);

#ifdef DEBUG
			 // Items may be lost here. This should be a very rare occasion. Emit
			 // a warning in DEBUG mode, as this might hint at some general problem
			 if(not success) {
				 glogger
					 << "In GBufferPortT<processable_type>::push_processed(item_ptr, timeout):" << std::endl
					 << "Submission was not successful. The work item might be discarded." << std::endl
					 << "Timeout was " << timeout.count() << " seconds" << std::endl
					 << GWARNING;
			 }
#endif
		 }

		 return success;
	 }

	 /***************************************************************************/
	 /**
	  * Retrieves an item from the "processed" queue. This function will usually be
	  * called directly or indirectly by an optimization algorithm.
	  * This function will block until the item was submitted.
	  *
	  * @param item_ptr The item that was retrieved from the queue
	  * @return A boolean which indicates whether the retrieval was successful
	  */
	 void pop_processed(std::shared_ptr<processable_type> &item_ptr) {
		 // The actual retrieval
		 m_processed_ptr->pop_and_block_copy(item_ptr);

		 if(item_ptr) {
			 // Make it known to the work item when it has returned to its origin
			 item_ptr->markProcRetrievalTime();
		 }
	 }

	 /***************************************************************************/
	 /**
	  * A version of GBufferPortT::pop_processed() with the ability to time-out.
	  * False will be returned of the timeout was reached.
	  *
	  * @param item_ptr The item that was retrieved from the queue
	  * @param timeout duration until a timeout occurs
	  * @return A boolean which indicates whether the retrieval was successful
	  */
	 bool pop_processed(
		 std::shared_ptr<processable_type> &item_ptr
		 , const std::chrono::duration<double> &timeout
	 ) {
		 // The actual retrieval
		 bool success = m_processed_ptr->pop_and_wait_copy(
			 item_ptr
			 , timeout
		 );
		 if(success && item_ptr) {
			 // Make it known to the work item when it has returned to its origin
			 item_ptr->markProcRetrievalTime();
		 }

		 return success;
	 }

	 /***************************************************************************/
	 /*
	  * Retrieves the unique tag that was assigned to this object
	  *
	  * @return The unique tag assigned to this object
	  */
	 BUFFERPORT_ID_TYPE getUniqueTag() const {
		 return m_tag;
	 }

	 /***************************************************************************/
	 /**
	  * Retrieves the time of the first retrieval
	  *
	  * @return The timepoint of the first retrieval
	  */
	 std::chrono::high_resolution_clock::time_point getFirstRetrievalTime() const {
		 std::unique_lock<std::mutex> lock(m_first_retrieval_mutex);

		 // Wait until a first work item was retrieved
		 m_retrievalTimeCondition.wait(lock, [this]() -> bool {return not this->m_no_retrieval;});

		 // Let the audience know when the first retrieval has occurred
		 return m_retrieval_start_time;
	 }

	 /***************************************************************************/
	 /**
	  * Allows a producer to indicate that it has lost interest
	  */
	 void producer_disconnect() {
		 m_connected_to_producer = false;
	 }

	 /***************************************************************************/
	 /**
	  * Allows to check whether this object is still connected to a producer
	  */
	 bool is_connected_to_producer() const {
		 return m_connected_to_producer;
	 }

private:
	 /***************************************************************************/
	 /**
	  * Setting of a unique id for this buffer port
	  */
  	 void set_port_tag(BUFFERPORT_ID_TYPE tag) {
		 m_tag = tag;
  	 }

	 /***************************************************************************/
	 // Data

	 std::atomic<bool> m_no_retrieval{true}; ///< Indicates whether an item was already retrieved from the raw queue
	 mutable std::mutex m_first_retrieval_mutex; ///< Blocks access to m_first_retrieval_time; mutable so we can lock it in a const function
	 std::chrono::high_resolution_clock::time_point m_retrieval_start_time = std::chrono::high_resolution_clock::now(); ///< Holds the time when the first work item was retrieved from the queue
	 mutable std::condition_variable m_retrievalTimeCondition; ///< Regulates retrieval of the data in m_retrieval_start_time

	 std::shared_ptr<RAW_BUFFER_TYPE> m_raw_ptr{new RAW_BUFFER_TYPE()}; ///< The queue for raw objects
	 std::shared_ptr<PROCESSED_BUFFER_TYPE> m_processed_ptr{new PROCESSED_BUFFER_TYPE()}; ///< The queue for processed objects

	 std::atomic<bool> m_connected_to_producer{true}; ///< Indicates whether this object is currently connected to a producer. We assume that this happens upon creation of this object

	 BUFFERPORT_ID_TYPE m_tag = 0; ///< A unique id assigned to objects of this class
};

/******************************************************************************/

} // namespace Gem::Courtier

