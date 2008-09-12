/**
 * @file GBoundedBufferT.hpp
 */

/* This file is part of Geneva, Gemfony scientific's optimization library.
 *
 * Note: This class was adapted from a circular_buffer test case (
 * [Boost 1.36 trunk version 17.7.08]/libs/circular_buffer/test/bounded_buffer_comparison.cpp )
 * by Jan Gaspar. bounded_buffer_comparison.cpp is covered by the
 * Boost Software License, Version 1.0 (see
 * http://www.boost.org/LICENSE_1_0.txt) of the file
 * BOOST_LICENSE_1_0.
 *
 * Modifications were applied to the code.
 *
 * The code by Jan Gaspar contained the following text:
 ***************************************************************
 * Copyright (c) 2003-2007 Jan Gaspar
 * Use, modification, and distribution is subject to the Boost Software
 * License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 ***************************************************************
 *
 * All modifications to Jan Gaspar's original code, as shown in this
 * file, are also covered by the Boost Software License, Version 1.0,
 * and are Copyright (C) 2008 Ruediger Berlich and Copyright (C) 2008
 * Forschungszentrum Karlsruhe GmbH.
 *
 * In particular, please note that
 *
 ***************************************************************
 * RUEDIGER BERLICH AND FORSCHUNGSZENTRUM KARLSRUHE MAKE NO
 * REPRESENTATION ABOUT THE SUITABILITY OF THIS SOFTWARE FOR
 * ANY PURPOSE. IT IS PROVIDED "AS IS" WITHOUT EXPRESS OR IMPLIED
 * WARRANTY.
 ***************************************************************
 */

/*
 * Boost Software License - Version 1.0 - August 17th, 2003
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

// Standard headers go here

#include <string>
#include <iostream>
#include <deque>

// Boost headers go here

#include <boost/version.hpp>

#if BOOST_VERSION < 103600
#error "Error: Boost should at least have version 1.36 !"
#endif /* BOOST_VERSION */

#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/thread.hpp>
#include <boost/progress.hpp>
#include <boost/bind.hpp>
#include <boost/utility.hpp>
#include <boost/exception.hpp>
#include <boost/cstdint.hpp>
#include <boost/date_time.hpp>

#ifndef GBOUNDEDBUFFERT_HPP_
#define GBOUNDEDBUFFERT_HPP_

// GenEvA headers go here

namespace Gem {
namespace Util {

/***********************************************************************************/
/** @brief Class to be thrown as a message in the case of a time-out in GBuffer */
class gem_util_condition_time_out: public boost::exception {};

/***********************************************************************************/
/**
 * By default the buffer will have this size. As the buffer
 * dynamically grows and shrinks, we choose a very high value. This
 * is a safeguard against errors like endless loops that might keep
 * filling the buffer until memory is exhausted. In normal work
 * conditions, however, the buffer should never reach its upper
 * limit.
 */
const std::size_t DEFAULTBUFFERSIZE = 10000;

/********************************************************************/
/**
 * This class implements a bounded buffer. Items can be added to one
 * end by multiple threads and retrieved from the other, also by
 * multiple threads. When the buffer is full, attempts to add items
 * will block until there is again enough space. When the buffer is
 * empty, retrieval of items will block until new items have become
 * available. The class contains a "get" function that times out
 * when no item could be retrieved from the buffer. This allows
 * timeouts for data sinks. This can be important in situations
 * where sources might permanently or temporarily go away (e.g. due
 * to network failure). The underlying data structure is a
 * std::deque. Access to it is protected by a mutex. The class works
 * with condition variables.  Note that this class assumes that an
 * operator= is available for the items stored in the buffer.
 */
template<typename T>
class GBoundedBufferT
	: private boost::noncopyable
{
public:

	typedef typename std::deque<T> container_type;
	typedef typename container_type::value_type value_type;

	/***************************************************************/
	/**
	 * The default constructor. Sets up a buffer of size DEFAULTBUFFERSIZE.
	 */
	GBoundedBufferT(void) throw()
		:capacity_(DEFAULTBUFFERSIZE)
	{ /* nothing */}

	/***************************************************************/
	/**
	 * A constructor that creates a buffer with custom size "capacity".
	 * It enforces a minimum buffer size of 1.
	 *
	 * @param capacity The desired size of the buffer
	 */
	explicit GBoundedBufferT(std::size_t capacity) throw()
		:capacity_(capacity?capacity:1)
	{ /* nothing */}

	/***************************************************************/
	/**
	 * A standard destructor. Virtual, as classes such as a producer-type
	 * class might get derived from this one. We do not want the destructor
	 * to throw, hence we try to catch all errors locally. Any error here
	 * means termination of the program. No logging takes place, as we want
	 * this class to be independent of the GenEvA framework
	 */
	virtual ~GBoundedBufferT()
	{
		// Any error here is deadly ...
		try {
			boost::mutex::scoped_lock lock(mutex_);
			container_.clear();
		}
		// This is a standard error raised by the lock/mutex
		catch(boost::thread_resource_error&) {

			std::cerr << "Caught thread_resource_error in GBoundedBufferT::~GBoundedBufferT(). Terminating ..." << std::endl;
			std::terminate();
		}
		// We do not know whether any of the destructors of the items in the buffer throw anything
		catch(...) {
			std::cerr << "Caught unknown exception in GBoundedBufferT::~GBoundedBufferT(). Terminating ..." << std::endl;
			std::terminate();
		}
	}

	/***************************************************************/
	/**
	 * Adds a single item to the front of the buffer. The function
	 * will block if there is no space in the buffer and continue once
	 * space is available.
	 *
	 * @param item An item to be added to the front of the buffer
	 */
	void push_front(const value_type& item)
	{
		boost::mutex::scoped_lock lock(mutex_);
		// Note that this overload of wait() internally runs a loop on is_not_full to
		// deal with spurious wakeups
		not_full_.wait(lock, boost::bind(&GBoundedBufferT<value_type>::is_not_full, this));
		container_.push_front(item);
		lock.unlock();
		not_empty_.notify_one();
	}

	/***************************************************************/
	/**
	 * Adds a single item to the front of the buffer. The function
	 * will time out after a given amount of time. This function was
	 * added to Jan Gaspar's original implementation.
	 *
	 * @param item An item to be added to the front of the buffer
	 * @param timeout duration until a timeout occurs
	 */
	void push_front(const value_type& item, const boost::posix_time::time_duration& timeout)
	{
		boost::mutex::scoped_lock lock(mutex_);
		if(!not_full_.timed_wait(lock,timeout,boost::bind(&GBoundedBufferT<value_type>::is_not_full, this)))
			throw Gem::Util::gem_util_condition_time_out();
		container_.push_front(item);
		lock.unlock();
		not_empty_.notify_one();
	}

	/***************************************************************/
	/**
	 * Retrieves a single item from the end of the buffer. The
	 * function will block if no items are available and will continue
	 * once items become available again.
	 *
	 * @param pItem Pointer to a single item that was removed from the end of the buffer
	 */
	void pop_back(value_type* pItem)
	{
		boost::mutex::scoped_lock lock(mutex_);
		not_empty_.wait(lock, boost::bind(&GBoundedBufferT<value_type>::is_not_empty, this));
		*pItem = container_.back();
		container_.pop_back();
		lock.unlock();
		not_full_.notify_one();
	}

	/***************************************************************/
	/**
	 * Retrieves a single item from the end of the buffer. The function
	 * will time out after a given amount of time. This function was
	 * added to Jan Gaspar's original implementation.
	 *
	 * @param pItem Pointer to a single item that was removed from the end of the buffer
	 * @param timeout duration until a timeout occurs
	 */
	void pop_back(value_type* pItem, const boost::posix_time::time_duration& timeout)
	{
		boost::mutex::scoped_lock lock(mutex_);
		if(!not_empty_.timed_wait(lock,timeout,boost::bind(&GBoundedBufferT<value_type>::is_not_empty, this)))
			throw Gem::Util::gem_util_condition_time_out();
		*pItem = container_.back();
		container_.pop_back();
		lock.unlock();
		not_full_.notify_one();
	}

	/***************************************************************/
	/**
	 * Retrieves the maximum allowed size of the buffer. No need for
	 * synchronization, as reading the value should be an atomic
	 * operation.
	 *
	 * @return The maximum allowed capacity
	 */
	std::size_t getCapacity(void) const throw()
	{
		return capacity_;
	}

	/***************************************************************/
	/**
	 * Retrieves the remaining space in the buffer. Note that the capacity
	 * may change once this function has completed. The information taken
	 * from this function can thus only serve as an indication. GRandomFactory
	 * uses it to find out whether it should continue to produce items.
	 *
	 * @return The currently remaining space in the buffer
	 */
	std::size_t remainingSpace(void)
	{
		boost::mutex::scoped_lock lock(mutex_);
		return container_.size() - capacity_;
	}

protected:
	/***************************************************************
	 * We want to be able to add custom producer threads. Hence the
	 * following functions are all protected, not private.
	 */

	/**
	 * A helper function needed for the condition variables. It is only called
	 * in a safe context, where a mutex has been locked. Hence we do not need
	 * any local synchronization.
	 *
	 * @return A boolean value indication whether the buffer is not empty
	 */
	bool is_not_empty() const throw()
	{
		return (container_.size() > 0);
	}

	/**
	 * A helper function needed for the condition variables. It is only called
	 * in a safe context, where a mutex has been locked. Hence we do not need
	 * any local synchronization.
	 *
	 * @return A boolean value indicating whether the buffer is not full
	 */
	bool is_not_full() const throw()
	{
		return (container_.size() < capacity_);
	}

	const std::size_t capacity_; ///< The maximum allowed size of the container
	container_type container_; ///< The actual data store
	boost::mutex mutex_; ///< Used for synchronization of access to the container
	boost::condition_variable not_empty_; ///< Used for synchronization of access to the container
	boost::condition_variable not_full_; ///< Used for synchronization of access to the container

private:
	/***************************************************************/
	GBoundedBufferT(const GBoundedBufferT&); ///< Disabled copy constructor
	GBoundedBufferT& operator = (const GBoundedBufferT&); ///< Disabled assign operator
};

} /* namespace Util */
} /* namespace Gem */

#endif /* GBOUNDEDBUFFERT_HPP_ */
