/*
 * This file is part of the Geneva library collection.
 *
 * Note: this class was adapted from an earlier Boost 1.36 version of the
 * thread_group class. The original code contained the following text:
 *
 * ***
 * Distributed under the Boost Software License, Version 1.0. (See
 * accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 * (C) Copyright 2007-8 Anthony Williams
 * ***
 *
 * See the NOTICE file in the top-level directory of the Geneva library
 * collection for a list of contributors and copyright information.
 *
 * The following license applies to the code IN THIS FILE:
 *
 * ***************************************************************************
 *
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
 *
 * ***************************************************************************
 *
 * NOTE THAT THE BOOST-LICENSE DOES NOT APPLY TO ANY OTHER FILES OF THE
 * GENEVA LIBRARY, UNLESS THIS IS EXPLICITLY STATED IN THE CORRESPONDING FILE!
 */

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard headers go here
#include <vector>
#include <mutex>


// Boost headers go here
#include <boost/utility.hpp>
#include <boost/lexical_cast.hpp>

#ifndef GTHREADGROUP_HPP_
#define GTHREADGROUP_HPP_

// Geneva header files go here
#include "GThread.hpp"

namespace Gem {
namespace Common {

// Forward declaration
class GThreadPool;

/******************************************************************************/
/**
 * A simple thread group that extends the standard Boost thread group by the
 * ability to address the last threads in order to remove threads without
 * the need for a pointer to them. This class was adapted from a version by
 * Anthony Williams, as offered as part of the Boost 1.36 release
 */
class GThreadGroup
	: private boost::noncopyable
{
	friend class GThreadPool;

	using thread_ptr = std::shared_ptr <Gem::Common::thread>;
	using thread_vector = std::vector<thread_ptr>;

public:
	/** @brief The destructor */
	G_API_COMMON ~GThreadGroup();

	/** @brief Adds an already created thread to the group */
	G_API_COMMON void add_thread(thread_ptr);

	/** @brief Requests all threads to join */
	G_API_COMMON void join_all();

	/** @brief Sends all threads the interrupt signal */
	G_API_COMMON void interrupt_all();

	/** @brief Interrupts, joins and finally removes the last thread in the group. */
	G_API_COMMON void remove_last();

	/** @brief Interrupts, joins and finally removes the last nThreads threads in the group */
	G_API_COMMON void remove_last(const std::size_t &);

	/** @brief Returns the size of the current thread group */
	G_API_COMMON std::size_t size() const;

	/***************************************************************************/
	/**
	 * Creates a new thread and adds it to the group
	 *
	 * TODO: Add perfect forwarding, so we may pass arguments directly
	 *
	 * @param f The function to be run by the thread
	 * @return A pointer to the newly created thread
	 */
	template<typename F>
	std::shared_ptr<Gem::Common::thread> create_thread(F f) {
		std::unique_lock<std::mutex> guard(m_mutex);
		thread_ptr new_thread(new Gem::Common::thread(f));
		m_threads.push_back(new_thread);
		return new_thread;
	}

	/***************************************************************************/
	/**
	 * Creates nThreads new threads with the same function
	 * and adds them to the group
	 *
	 * @param f The function to be run by the thread
	 * @param nThreads The number of threads to add to the group
	 * @return A pointer to the newly created thread
	 */
	template<typename F>
	void create_threads(F f, const std::size_t &nThreads) {
		for (std::size_t i = 0; i < nThreads; i++) {
			create_thread(f);
		}
	}

	/***************************************************************************/

private:
	/** @brief Clears the thread vector */
	void clearThreads();

	thread_vector m_threads; ///< Holds the actual threads
	mutable std::mutex m_mutex; ///< Needed to synchronize access to the vector
};

/******************************************************************************/

} /* namespace Common*/
} /* namespace Gem */

#endif /* GTHREADGROUP_HPP_ */
