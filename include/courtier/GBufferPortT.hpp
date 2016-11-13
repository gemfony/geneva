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

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard header files go here
#include <sstream>
#include <thread>
#include <chrono>
#include <atomic>

// Boost header files go here
#include <boost/utility.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_serialize.hpp>

#ifndef GBUFFERPORTT_HPP_
#define GBUFFERPORTT_HPP_

// Geneva header files go here
#include "courtier/GCourtierEnums.hpp"
#include "common/GHelperFunctionsT.hpp"
#include "common/GBoundedBufferT.hpp"

namespace Gem {
namespace Courtier {

/******************************************************************************/
/**
 * A GBufferPortT<T> consists of two GBoundedBufferT<T> objects, one intended for "raw"
 * items, the other for returning, processed items. While this class could
 * be useful in many scenarios, the most common application is as a mediator
 * between optimization algorithms and GConsumer-derivatives. The optimization algorithm
 * is a source of raw items, which are processed by GConsumer-derivatives
 * (such as GBoostThreadConsumer and GAsioTCPConsumerT) and then returned to the
 * population. GBrokerT instantiations orchestrate this exchange.
 * All of this happens in a multi-threaded environment. It is not possible to
 * create copies of this class, as one GBufferPortT is intended to serve one
 * single population.
 */
template<typename T>
class GBufferPortT
	: private boost::noncopyable
{
public:
	 using RAW_BUFFER_TYPE = typename Gem::Common::GBoundedBufferT<T, Gem::Common::DEFAULTBUFFERSIZE>;
	 using PROCESSED_BUFFER_TYPE = typename Gem::Common::GBoundedBufferT<T, 0>;

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
	  * @param item A raw object that needs to be processed
	  * @return A boolean which indicates whether the submission was successful
	  */
	 void push_raw(T item) {
		 m_raw_ptr->push_front(item);
	 }

	 /***************************************************************************/
	 /**
	  * Timed version of GBufferPortT::push_raw() . If the item could not be added
	  * after a given amount of time, the function returns. Note that a time_out
	  * exception will be thrown in this case.
	  *
	  * @param item An item to be added to the buffer
	  * @param timeout duration until a timeout occurs
	  * @return A boolean which indicates whether the submission was successful
	  */
	 bool push_raw(
		 T item
		 , const std::chrono::duration<double> &timeout
	 ) {
		 return m_raw_ptr->push_front_bool(item, timeout);
	 }

	 /***************************************************************************/
	 /**
	  * Retrieves an item from the back of the "m_raw_ptr" queue. Blocks until
	  * an item could be retrieved. This function will block until the item was submitted.
	  *
	  * @param item A reference to the item to be retrieved
	  * @return A boolean which indicates whether the retrieval was successful
	  */
	 void pop_raw(T &item) {
		 m_raw_ptr->pop_back(item);
	 }

	 /***************************************************************************/
	 /**
	  * A version of GBufferPortT::pop_raw() with the ability to time-out. Note
	  * that an exception will be thrown by m_raw_ptr if the time-out was reached. It
	  * needs to be caught by the calling function.
	  *
	  * @param item The item that was retrieved from the queue
	  * @param timeout duration until a timeout occurs
	  * @return A boolean which indicates whether the retrieval was successful
	  */
	 bool pop_raw(
		 T &item
		 , const std::chrono::duration<double> &timeout
	 ) {
		 return m_raw_ptr->pop_back_bool(item, timeout);
	 }

	 /***************************************************************************/
	 /**
	  * Puts an item into the "processed" queue. This function will block until the item was submitted.
	  *
	  * @param item A raw object that needs to be processed
	  * @return A boolean which indicates whether the submission was successful
	  */
	 void push_processed(T item) {
		 m_processed_ptr->push_front(item);
	 }

	 /***************************************************************************/
	 /**
	  * Timed version of GBufferPortT::push_processed() . If the item could not
	  * be added after a given amount of time, a timed_out exception will be thrown by
	  * m_processed_ptr.
	  *
	  * @param item An item to be added to the buffer
	  * @param timeout duration until a timeout occurs
	  * @return A boolean which indicates whether the submission was successful
	  */
	 bool push_processed(
		 T item
		 , const std::chrono::duration<double> &timeout
	 ) {
		 return m_processed_ptr->push_front_bool(item, timeout);
	 }

	 /***************************************************************************/
	 /**
	  * Retrieves an item from the "processed" queue. This function will usually be
	  * called directly or indirectly by an optimization algorithm.
	  * This function will block until the item was submitted.
	  *
	  * @param The item that was retrieved from the queue
	  * @return A boolean which indicates whether the retrieval was successful
	  */
	 void pop_processed(T &item) {
		 m_processed_ptr->pop_back(item);
	 }

	 /***************************************************************************/
	 /**
	  * A version of GBufferPortT::pop_processed() with the ability to time-out. If the
	  * time-out was reached, m_processed_ptr will throw a time_out exception.
	  *
	  * @param item The item that was retrieved from the queue
	  * @param timeout duration until a timeout occurs
	  * @return A boolean which indicates whether the retrieval was successful
	  */
	 bool pop_processed(
		 T &item
		 , const std::chrono::duration<double> &timeout
	 ) {
		 return m_processed_ptr->pop_back_bool(item, timeout);
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
private:
	 std::shared_ptr<RAW_BUFFER_TYPE> m_raw_ptr; ///< The queue for raw objects
	 std::shared_ptr<PROCESSED_BUFFER_TYPE> m_processed_ptr; ///< The queue for processed objects

	 const boost::uuids::uuid m_tag = boost::uuids::random_generator()(); ///< A unique id assigned to objects of this class
};

/******************************************************************************/

} /* namespace Courtier */
} /* namespace Gem */

#endif /* GBUFFERPORTT_HPP_ */
