/**
 * @file GBufferPortT.hpp
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

#ifndef GBUFFERPORTT_HPP_
#define GBUFFERPORTT_HPP_

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
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_serialize.hpp>

// Geneva header files go here
#include "courtier/GCourtierEnums.hpp"
#include "courtier/GProcessingContainerT.hpp"
#include "common/GCommonHelperFunctionsT.hpp"
#include "common/GBoundedBufferT.hpp"

namespace Gem {
namespace Courtier {

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
class GBufferPortT : private boost::noncopyable
{
	 // Make sure processable_type adheres to the GProcessingContainerT interface
	 static_assert(
		 std::is_base_of<Gem::Courtier::GProcessingContainerT<processable_type, typename processable_type::result_type>, processable_type>::value
		 , "GBufferPortT: processable_type does not adhere to the GProcessingContainerT<> interface"
	 );

	 using RAW_BUFFER_TYPE = typename Gem::Common::GBoundedBufferT<std::shared_ptr<processable_type>, Gem::Common::DEFAULTBUFFERSIZE>;
	 using PROCESSED_BUFFER_TYPE = typename Gem::Common::GBoundedBufferT<std::shared_ptr<processable_type>, 0>;

public:
	 /***************************************************************************/
	 /**
	  * The default constructor
	  */
	 GBufferPortT()
		 : m_raw_ptr(new RAW_BUFFER_TYPE())
		 , m_processed_ptr(new PROCESSED_BUFFER_TYPE())
	 { /* nothing */ }

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
			 m_raw_ptr->push_and_block(item_ptr);
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
			 success = m_raw_ptr->push_and_wait(item_ptr, timeout);

#ifdef DEBUG
			 // Items may be lost here. This should be a very rare occasion. Emit
			 // a warning in DEBUG mode, as this might hint at some general problem
			 if(!success) {
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
		 m_raw_ptr->pop_and_block(item_ptr);

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
		 bool success = m_raw_ptr->pop_and_wait(
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
			 m_processed_ptr->push_and_block(item_ptr);
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
			 success = m_processed_ptr->push_and_wait(item_ptr, timeout);

#ifdef DEBUG
			 // Items may be lost here. This should be a very rare occasion. Emit
			 // a warning in DEBUG mode, as this might hint at some general problem
			 if(!success) {
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
		 m_processed_ptr->pop_and_block(item_ptr);

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
		 bool success = m_processed_ptr->pop_and_wait(
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
	  * @return The value of the m_id variable
	  */
	 boost::uuids::uuid getUniqueTag() const {
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
		 m_retrievalTimeCondition.wait(lock, [this]() -> bool {return !this->m_no_retrieval;});

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
	 // Retrieval of a random uuid
	 static boost::uuids::uuid getRandomUUID() {
		 boost::uuids::random_generator generator;
		 return generator();
	 }

	 /***************************************************************************/
	 // Data

	 std::atomic<bool> m_no_retrieval{true}; ///< Indicates whether an item was already retrieved from the raw queue
	 mutable std::mutex m_first_retrieval_mutex; ///< Blocks access to m_first_retrieval_time; mutable so we can lock it in a const function
	 std::chrono::high_resolution_clock::time_point m_retrieval_start_time = std::chrono::high_resolution_clock::now(); ///< Holds the time when the first work item was retrieved from the queue
	 mutable std::condition_variable m_retrievalTimeCondition; ///< Regulates retrieval of the data in m_retrieval_start_time

	 std::shared_ptr<RAW_BUFFER_TYPE> m_raw_ptr; ///< The queue for raw objects
	 std::shared_ptr<PROCESSED_BUFFER_TYPE> m_processed_ptr; ///< The queue for processed objects

	 std::atomic<bool> m_connected_to_producer{true}; ///< Indicates whether this object is currently connected to a producer. We assume that this happens upon creation of this object

	 // TODO: This could just be a consecutive number obtained from the broker
	 const boost::uuids::uuid m_tag = GBufferPortT<processable_type>::getRandomUUID(); ///< A unique id assigned to objects of this class
};

/******************************************************************************/

} /* namespace Courtier */
} /* namespace Gem */

#endif /* GBUFFERPORTT_HPP_ */
