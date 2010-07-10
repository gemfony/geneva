/**
 * @file GBufferPortT.hpp
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

// Standard header files go here
#include <sstream>

// Includes check for correct Boost version(s)
#include "GGlobalDefines.hpp"

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

// GenEvA header files go here

#include "GBoundedBufferWithIdT.hpp"

namespace Gem {
namespace Util {

/*****************************************************************************/
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
	/*****************************************************************************/
	/**
	 * The default constructor. Note that, when using this constructor, the GBoundedBufferWithIdT
	 * objects will assume the default sizes.
	 */
	GBufferPortT()
		: original_(new Gem::Util::GBoundedBufferWithIdT<T>())
		, processed_(new Gem::Util::GBoundedBufferWithIdT<T>())
	{ /* nothing */	}

	/*****************************************************************************/
	/**
	 * Here we initialize the two GBoundedBufferWithIdT objects with a given size.
	 *
	 * @param size The desired capacity of the GBoundedBufferWithIdT objects
	 */
	explicit GBufferPortT(const std::size_t& size)
	   : original_(new Gem::Util::GBoundedBufferWithIdT<T>(size))
	   , processed_(new Gem::Util::GBoundedBufferWithIdT<T>(size))
	{ /* nothing */ }

	/*****************************************************************************/
	/**
	 * Retrieves a shared_ptr to the "original" queue, for consumption by the broker.
	 *
	 * @return A shared_ptr with the "original" queue
	 */
	boost::shared_ptr<Gem::Util::GBoundedBufferWithIdT<T> > getOriginal() {
		return original_;
	}

	/*****************************************************************************/
	/**
	 * Retrieves a shared_ptr to the "processed" queue, for consumption by the broker.
	 *
	 * @return A shared_ptr with the "processed" queue
	 */
	boost::shared_ptr<Gem::Util::GBoundedBufferWithIdT<T> > getProcessed() {
		return processed_;
	}

	/*****************************************************************************/
	/**
	 * Puts an item into the original queue. This is the queue for "raw" objects.
	 *
	 * @param item A raw object that needs to be processed
	 */
	inline void push_front_orig(const T& item) {
		original_->push_front(item);
	}

	/*****************************************************************************/
	/**
	 * Timed version of GBufferPortT::push_front_orig() . If the item could not be added
	 * after sec seconds and msec milliseconds, the function returns. Note that a time_out
	 * exception will be thrown in this case.
	 *
	 * @param item An item to be added to the buffer
	 * @param timeout duration until a timeout occurs
	 */
	inline void push_front_orig(const T& item, const boost::posix_time::time_duration& timeout) {
		original_->push_front(item, timeout);
	}

	/*****************************************************************************/
	/**
	 * Retrieves an item from the back of the "original_" queue. Blocks until
	 * an item could be retrieved.
	 */
	inline void pop_back_orig(T* item) {
		original_->pop_back(item);
	}

	/*****************************************************************************/
	/**
	 * A version of GBufferPortT::push_back_orig() with the ability to time-out. Note
	 * that a time_out exception will be thrown by original_ if the time-out was
	 * reached. It needs to be caught by the calling function.
	 *
	 * @param item The item that was retrieved from the queue
	 * @param timeout duration until a timeout occurs
	 */
	inline void pop_back_orig(T *item, const boost::posix_time::time_duration& timeout) {
		original_->pop_back(item, timeout);
	}

	/*****************************************************************************/
	/**
	 * Puts an item into the "processed" queue.
	 *
	 * @param item A raw object that needs to be processed
	 */
	inline void push_front_processed(const T& item) {
		processed_->push_front(item);
	}

	/*****************************************************************************/
	/**
	 * Timed version of GBufferPortT::putProc() . If the item could not be added
	 * after sec seconds and msec milliseconds, a timed_out exception will be thrown
	 * by processed_.
	 *
	 * @param item An item to be added to the buffer
	 * @param timeout duration until a timeout occurs
	 */
	inline void push_front_processed(const T& item, const boost::posix_time::time_duration& timeout) {
		processed_->push_front(item, timeout);
	}

	/*****************************************************************************/
	/**
	 * Retrieves an item from the "processed" queue. This function will usually be
	 * called directly or indirectly by GTransferPopulation.
	 *
	 * @param The item that was retrieved from the queue
	 */
	inline void pop_back_processed(T* item) {
		processed_->pop_back(item);
	}

	/*****************************************************************************/
	/**
	 * A version of GBufferPortT::getProc() with the ability to time-out. If the
	 * time-out was reached, processed_ will throw a time_out exception.
	 *
	 * @param item The item that was retrieved from the queue
	 * @param timeout duration until a timeout occurs
	 */
	inline void pop_back_processed(T* item, const boost::posix_time::time_duration& timeout) {
		processed_->pop_back(item, timeout);
	}

	/*****************************************************************************/

private:
	boost::shared_ptr<Gem::Util::GBoundedBufferWithIdT<T> > original_; ///< The queue for raw objects
	boost::shared_ptr<Gem::Util::GBoundedBufferWithIdT<T> > processed_; ///< The queue for processed objects
};

/*****************************************************************************/

} /* namespace Util */
} /* namespace Gem */

#endif /* GBUFFERPORTT_HPP_ */
