/**
 * @file GBufferPortT.hpp
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt)
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) com
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
 * http://www.gemfony.com .
 */

// Standard header files go here
#include <sstream>

// Includes check for correct Boost version(s)
#include "common/GGlobalDefines.hpp"

// Boost header files go here

#include <boost/cstdint.hpp>
#include <boost/utility.hpp>
#include <boost/date_time.hpp>

#ifndef GBUFFERPORTT_HPP_
#define GBUFFERPORTT_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif

// Geneva header files go here

#include "common/GBoundedBufferWithIdT.hpp"

namespace Gem {
namespace Courtier {

/******************************************************************************/
/**
 * A GBufferPortT<T> consists of two GBoundedBufferWithIdT<T> objects, one intended for "raw"
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
class GBufferPortT
	:private boost::noncopyable
{
public:
	/***************************************************************************/
	/**
	 * The default constructor. Note that, when using this constructor, the
	 * GBoundedBufferWithIdT objects will assume the default sizes.
	 */
	GBufferPortT()
		: original_(new Gem::Common::GBoundedBufferWithIdT<T>())
		, processed_(new Gem::Common::GBoundedBufferWithIdT<T>())
	{
#ifdef GEM_COMMON_BENCHMARK_BOUNDED_BUFFER
		original_->setName("raw buffer");
		processed_->setName("processed buffer");
#endif /* GEM_COMMON_BENCHMARK_BOUNDED_BUFFER */
	}

	/***************************************************************************/
	/**
	 * Here we initialize the two GBoundedBufferWithIdT objects with a given size.
	 *
	 * @param size The desired capacity of the GBoundedBufferWithIdT objects
	 */
	explicit GBufferPortT(const std::size_t& size)
	   : original_(new Gem::Common::GBoundedBufferWithIdT<T>(size))
	   , processed_(new Gem::Common::GBoundedBufferWithIdT<T>(size))
	{
#ifdef GEM_COMMON_BENCHMARK_BOUNDED_BUFFER
		original_->setName("raw buffer");
		processed_->setName("processed buffer");
#endif /* GEM_COMMON_BENCHMARK_BOUNDED_BUFFER */
	}

	/***************************************************************************/
	/**
	 * Retrieves a shared_ptr to the "original" queue, for consumption by the broker.
	 *
	 * @return A shared_ptr with the "original" queue
	 */
	boost::shared_ptr<Gem::Common::GBoundedBufferWithIdT<T> > getOriginalQueue() {
		return original_;
	}

	/***************************************************************************/
	/**
	 * Retrieves a shared_ptr to the "processed" queue, for consumption by the broker.
	 *
	 * @return A shared_ptr with the "processed" queue
	 */
	boost::shared_ptr<Gem::Common::GBoundedBufferWithIdT<T> > getProcessedQueue() {
		return processed_;
	}

	/***************************************************************************/
	/**
	 * Puts an item into the original queue. This is the queue for "raw" objects.
	 *
	 * @param item A raw object that needs to be processed
	 */
	void push_front_orig(T item) {
		original_->push_front(item);
	}

	/***************************************************************************/
	/**
	 * Timed version of GBufferPortT::push_front_orig() . If the item could not be added
	 * after a given amount of time, the function returns. Note that a time_out
	 * exception will be thrown in this case.
	 *
	 * @param item An item to be added to the buffer
	 * @param timeout duration until a timeout occurs
	 */
	void push_front_orig(T item, const boost::posix_time::time_duration& timeout) {
		original_->push_front(item, timeout);
	}

	/***************************************************************************/
	/**
	 * Timed version of GBufferPortT::push_front_orig() that return a boolean
	 * indicating whether an item could be submitted
	 *
	 * @param item An item to be added to the buffer
	 * @param timeout duration until a timeout occurs
	 * @return A boolean indicating whether an item could be submitted
	 */
	bool push_front_orig_bool(T item, const boost::posix_time::time_duration& timeout) {
		return original_->push_front_bool(item, timeout);
	}

	/***************************************************************************/
	/**
	 * Retrieves an item from the back of the "original_" queue. Blocks until
	 * an item could be retrieved.
	 *
	 * @param item A reference to the item to be retrieved
	 */
	void pop_back_orig(T& item) {
		original_->pop_back(item);
	}

	/***************************************************************************/
	/**
	 * A version of GBufferPortT::push_back_orig() with the ability to time-out. Note
	 * that an exception will be thrown by original_ if the time-out was reached. It
	 * needs to be caught by the calling function.
	 *
	 * @param item The item that was retrieved from the queue
	 * @param timeout duration until a timeout occurs
	 */
	void pop_back_orig(T& item, const boost::posix_time::time_duration& timeout) {
		original_->pop_back(item, timeout);
	}

	/***************************************************************************/
	/**
	 * A version of GBufferPortT::push_back_orig() with the ability to time-out. Instead
	 * of throwing an exception it will return a boolean indicating whether an item
	 * could be retrieved.
	 *
	 * @param item The item that was retrieved from the queue
	 * @param timeout duration until a timeout occurs
	 * @return A boolean indicating whether an item could be retrieved
	 */
	bool pop_back_orig_bool(T& item, const boost::posix_time::time_duration& timeout) {
		return original_->pop_back_bool(item, timeout);
	}

	/***************************************************************************/
	/**
	 * Puts an item into the "processed" queue.
	 *
	 * @param item A raw object that needs to be processed
	 */
	void push_front_processed(T item) {
		processed_->push_front(item);
	}

	/***************************************************************************/
	/**
	 * Timed version of GBufferPortT::push_front_processed() . If the item could not
	 * be added after a given amount of time, a timed_out exception will be thrown by
	 * processed_.
	 *
	 * @param item An item to be added to the buffer
	 * @param timeout duration until a timeout occurs
	 */
	void push_front_processed(T item, const boost::posix_time::time_duration& timeout) {
		processed_->push_front(item, timeout);
	}

	/***************************************************************************/
	/**
	 * Timed version of GBufferPortT::push_front_processed() that returns a boolean
	 * indicating whether an item could be submitted.
	 *
	 * @param item An item to be added to the buffer
	 * @param timeout duration until a timeout occurs
	 * @return A boolean indicating whether an item could be submitted
	 */
	bool push_front_processed_bool(T item, const boost::posix_time::time_duration& timeout) {
		return processed_->push_front_bool(item, timeout);
	}

	/***************************************************************************/
	/**
	 * Retrieves an item from the "processed" queue. This function will usually be
	 * called directly or indirectly by GTransferPopulation.
	 *
	 * @param The item that was retrieved from the queue
	 */
	void pop_back_processed(T& item) {
		processed_->pop_back(item);
	}

	/***************************************************************************/
	/**
	 * A version of GBufferPortT::pop_back_processed() with the ability to time-out. If the
	 * time-out was reached, processed_ will throw a time_out exception.
	 *
	 * @param item The item that was retrieved from the queue
	 * @param timeout duration until a timeout occurs
	 */
	void pop_back_processed(T& item, const boost::posix_time::time_duration& timeout) {
		processed_->pop_back(item, timeout);
	}

	/***************************************************************************/
	/**
	 * A version of GBufferPortT::pop_back_processed() with the ability to time-out.
	 * Instead of throwing an exception, it will return a boolean indicating whether
	 * an item could be retrieved.
	 *
	 * @param item The item that was retrieved from the queue
	 * @param timeout duration until a timeout occurs
	 * @return A boolean indicating whether an item could be retrieved
	 */
	bool pop_back_processed_bool(T& item, const boost::posix_time::time_duration& timeout) {
		return processed_->pop_back_bool(item, timeout);
	}

	/***************************************************************************/
private:
	boost::shared_ptr<Gem::Common::GBoundedBufferWithIdT<T> > original_; ///< The queue for raw objects
	boost::shared_ptr<Gem::Common::GBoundedBufferWithIdT<T> > processed_; ///< The queue for processed objects
};

/******************************************************************************/

} /* namespace Courtier */
} /* namespace Gem */

#endif /* GBUFFERPORTT_HPP_ */
