/**
 * @file GThreadGroup.hpp
 */

/* This file is part of Geneva, Gemfony scientific's optimization library.
 *
 * Note: This class was adapted from an early Boost 1.36 version of the
 * thread_group class. The original code containted the following text:
 *
 * Distributed under the Boost Software License, Version 1.0. (See
 * accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 * (C) Copyright 2007-8 Anthony Williams
 *
 * Modifications were applied to the code.
 *
 * All modifications to Anthony William's original code, as shown in this
 * file, are also covered by the Boost Software License, Version 1.0,
 * and are Copyright (C) 2008 Ruediger Berlich and Copyright (C) 2008
 * Forschungszentrum Karlsruhe GmbH .
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

/* Boost Software License - Version 1.0 - August 17th, 2003
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

#include <vector>

#include <boost/cstdint.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>
#include <boost/utility.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/exception.hpp>
#include <boost/cstdint.hpp>

#ifndef GTHREADGROUP_HPP_
#define GTHREADGROUP_HPP_

namespace Gem {
namespace Util {

/******************************************************************************/
/**
 * A simple thread group that extends the standard Boost thread group by the
 * ability to address the last threads in order to remove threads without
 * the need for a pointer to them. This class was adapted from a version by
 * Anthony Williams, as offered as part of the Boost 1.36 release
 */
class GThreadGroup: private boost::noncopyable {
	typedef boost::shared_ptr<boost::thread> thread_ptr;
	typedef std::vector<thread_ptr> thread_vector;

public:
	/** @brief The standard destructor */
	~GThreadGroup();

	/** @brief Adds an already created thread to the group */
	void add_thread(boost::thread*);

	/** @brief Remove a thread from the group. Does nothing if the thread is empty. */
	void remove_thread(boost::thread*);

	/** @brief Requests all threads to join */
	void join_all();

	/** @brief Sends all threads the interrupt signal */
	void interrupt_all();

	/** @brief Interrupts, joins and finally removes the last thread in the group. */
	void remove_last();

	/** @brief Interrupts, joins and finally removes the last nThreads threads in the group */
	void remove_last(const std::size_t&);

	/** @brief Returns the size of the current thread group */
	std::size_t size() const;

	/********************************************************************/
	/**
	 * Creates a new thread and adds it to the group
	 *
	 * @param threadfunc The function to be run by the thread
	 * @return A pointer to the newly created thread
	 */
	template<typename F>
	boost::shared_ptr<boost::thread> create_thread(F threadfunc) {
		boost::lock_guard<boost::mutex> guard(m_);
		thread_ptr new_thread(new boost::thread(threadfunc));
		threads_.push_back(new_thread);
		return new_thread;
	}

	/********************************************************************/
	/**
	 * Creates nThreads new threads with the same function
	 * and adds them to the group
	 *
	 * @param threadfunc The function to be run by the thread
	 * @param nThreads The number of threads to add to the group
	 * @return A pointer to the newly created thread
	 */
	template<typename F>
	void create_threads(F threadfunc, const std::size_t& nThreads)	{
		for(std::size_t i=0; i<nThreads; i++) create_thread(threadfunc);
	}

	/********************************************************************/

private:
	thread_vector threads_; ///< Holds the actual threads
	mutable boost::mutex m_; ///< Needed to synchronize access to the vector
};

} /* namespace Util*/
} /* namespace Gem */

#endif /* GTHREADGROUP_HPP_ */
