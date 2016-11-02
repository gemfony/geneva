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
#include <boost/lockfree/spsc_queue.hpp>
#include <boost/lockfree/policies.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_serialize.hpp>

#ifndef GBUFFERPORTT_HPP_
#define GBUFFERPORTT_HPP_

// Geneva header files go here
#include "courtier/GCourtierEnums.hpp"
#include "common/GHelperFunctionsT.hpp"

namespace Gem {
namespace Courtier {

/******************************************************************************/
/**
 * A GBufferPortT<T> consists of two GBoundedBufferT<T> objects, one intended for "raw"
 * items, the other for returning, processed items. While this class could
 * be useful in many scenarios, the most common application is as a mediator
 * between GBrokerEA and GConsumer-derivatives. The GBrokerEA
 * is a source of raw items, which are processed by GConsumer-derivatives
 * (such as GBoostThreadConsumer and GAsioTCPConsumerT) and then returned to the
 * population. GBrokerT instantiations orchestrate this exchange.
 * All of this happens in a multi-threaded environment. It is not possible to
 * create copies of this class, as one GBufferPortT is intended to serve one
 * single population.
 */
template<typename T>
class GBufferPortT : private boost::noncopyable {
public:
	 typedef boost::lockfree::spsc_queue<T, boost::lockfree::capacity<DEFAULTRAWBUFFERSIZE>> RAW_BUFFER_TYPE; // The size is limited
	 typedef boost::lockfree::spsc_queue<T, boost::lockfree::capacity<DEFAULTPROCESSEDBUFFERSIZE>> PROCESSED_BUFFER_TYPE; // unlimited size (returning items are always welcome)

	/***************************************************************************/
	/**
	 * The default constructor
	 */
	GBufferPortT()
		: m_raw(new RAW_BUFFER_TYPE())
		, m_processed(new PROCESSED_BUFFER_TYPE())
   { /* nothing */ }

	/***************************************************************************/
	/**
	 * Retrieves a shared_ptr to the "raw" queue, for consumption by the broker.
	 *
	 * @return A shared_ptr with the "raw" queue
	 */
	std::shared_ptr<RAW_BUFFER_TYPE> getOriginalQueue() {
		return m_raw;
	}

	/***************************************************************************/
	/**
	 * Retrieves a shared_ptr to the "processed" queue, for consumption by the broker.
	 *
	 * @return A shared_ptr with the "processed" queue
	 */
	std::shared_ptr<PROCESSED_BUFFER_TYPE> getProcessedQueue() {
		return m_processed;
	}

	/***************************************************************************/
	/**
	 * Puts an item into the raw queue. This is the queue for "raw" objects.
	 *
	 * @param item A raw object that needs to be processed
	 * @return A boolean which indicates whether the submission was successful
	 */
	bool push_front_raw(T item) {
		Gem::Common::forcedSubmissionToBoostLockfree(
			*m_raw
			, item
		);
	}

	/***************************************************************************/
	/**
	 * Timed version of GBufferPortT::push_front_raw() . If the item could not be added
	 * after a given amount of time, the function returns. Note that a time_out
	 * exception will be thrown in this case.
	 *
	 * @param item An item to be added to the buffer
	 * @param timeout duration until a timeout occurs
	 * @return A boolean which indicates whether the submission was successful
	 */
	bool push_front_raw(
		T item
		, const std::chrono::duration<double> &timeout
	) {
		Gem::Common::timedSubmissionToBoostLockfree(
			*m_raw
			, item
			, timeout
		);
	}

	/***************************************************************************/
	/**
	 * Retrieves an item from the back of the "m_raw" queue. Blocks until
	 * an item could be retrieved.
	 *
	 * @param item A reference to the item to be retrieved
	 * @return A boolean which indicates whether the retrieval was successful
	 */
	 bool pop_back_raw(T &item) {
		 Gem::Common::forcedRetrievalFromBoostLockfree(
			 *m_raw
			 , item
		 );
	}

	/***************************************************************************/
	/**
	 * A version of GBufferPortT::pop_back_raw() with the ability to time-out. Note
	 * that an exception will be thrown by m_raw if the time-out was reached. It
	 * needs to be caught by the calling function.
	 *
	 * @param item The item that was retrieved from the queue
	 * @param timeout duration until a timeout occurs
	 * @return A boolean which indicates whether the retrieval was successful
	 */
	bool pop_back_raw(
		T &item
		, const std::chrono::duration<double> &timeout
	) {
		Gem::Common::timedRetrievalFromBoostLockfree(
			*m_raw
			, item
			, timeout
		);
	}

	/***************************************************************************/
	/**
	 * Puts an item into the "processed" queue.
	 *
	 * @param item A raw object that needs to be processed
	 * @return A boolean which indicates whether the submission was successful
	 */
	bool push_front_processed(T item) {
		Gem::Common::forcedSubmissionToBoostLockfree(
			*m_processed
			, item
		);
	}

	/***************************************************************************/
	/**
	 * Timed version of GBufferPortT::push_front_processed() . If the item could not
	 * be added after a given amount of time, a timed_out exception will be thrown by
	 * m_processed.
	 *
	 * @param item An item to be added to the buffer
	 * @param timeout duration until a timeout occurs
	 * @return A boolean which indicates whether the submission was successful
	 */
	bool push_front_processed(
		T item
		, const std::chrono::duration<double> &timeout
	) {
		Gem::Common::timedSubmissionToBoostLockfree(
			*m_processed
			, item
			, timeout
		);
	}

	/***************************************************************************/
	/**
	 * Retrieves an item from the "processed" queue. This function will usually be
	 * called directly or indirectly by GTransferPopulation.
	 *
	 * @param The item that was retrieved from the queue
	 * @return A boolean which indicates whether the retrieval was successful
	 */
	bool pop_back_processed(T &item) {
		Gem::Common::forcedRetrievalFromBoostLockfree(
			*m_processed
			, item
		);
	}

	/***************************************************************************/
	/**
	 * A version of GBufferPortT::pop_back_processed() with the ability to time-out. If the
	 * time-out was reached, m_processed will throw a time_out exception.
	 *
	 * @param item The item that was retrieved from the queue
	 * @param timeout duration until a timeout occurs
	 * @return A boolean which indicates whether the retrieval was successful
	 */
	bool pop_back_processed(
		T &item
		, const std::chrono::duration<double> &timeout
	) {
		Gem::Common::timedRetrievalFromBoostLockfree(
			*m_processed
			, item
			, timeout
		);
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
	std::shared_ptr <RAW_BUFFER_TYPE> m_raw; ///< The queue for raw objects
	std::shared_ptr <PROCESSED_BUFFER_TYPE> m_processed; ///< The queue for processed objects

   boost::uuids::uuid m_tag = boost::uuids::random_generator()(); ///< A unique id assigned to objects of this class
};

/******************************************************************************/

} /* namespace Courtier */
} /* namespace Gem */

#endif /* GBUFFERPORTT_HPP_ */
