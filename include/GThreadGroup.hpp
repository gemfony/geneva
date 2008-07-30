/**
 * @file
 */

/* GThreadGroup.hpp
 *
 * This file is part of Geneva, Gemfony scientific's optimization library.
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

using namespace boost;

namespace Gem {
namespace GenEvA {
  /******************************************************************************/
  /**
   * A simple thread group that extends the standard Boost thread group by the
   * ability to address the last threads in order to remove threads without
   * the need for a pointer to them. This class was adapted from a version by
   * Anthony Williams, offered as part of the Boost 1.36 release
   */
  class GThreadGroup : private boost::noncopyable
  {
    typedef boost::shared_ptr<thread> thread_ptr;
    typedef std::vector<thread_ptr> thread_vector;

  public:
    /********************************************************************/
    /**
     * The standard destructor. Assumes that all threads have already
     * been terminated (?), so that the thread objects can be safely deleted.
     * As we are using a vector of boost::shared_ptr<thread> objects, we do not have
     * to manually erase anything.
     */
    ~GThreadGroup()
    { /* */ }

    /********************************************************************/
    /**
     * Creates a new thread and adds it to the group
     *
     * @param threadfunc The function to be run by the thread
     * @return A pointer to the newly created thread
     */
    template<typename F>
      boost::shared_ptr<thread>
      create_thread(F threadfunc)
      {
        boost::lock_guard<mutex> guard(m_);
        thread_ptr new_thread(new thread(threadfunc));
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
      void
      create_threads(F threadfunc, std::size_t nThreads)
      {
        for(std::size_t i=0; i<nThreads; i++) create_thread(threadfunc);
      }

    /********************************************************************/
    /**
     * Adds an already created thread to the group
     *
     * @param thrd A pointer to a thread that should be added to the group
     */
    void
    add_thread(thread* thrd)
    {
      if (thrd)
        {
          boost::lock_guard<mutex> guard(m_);
          thread_ptr p_thrd(thrd);
          threads_.push_back(p_thrd);
        }
    }

    /********************************************************************/
    /**
     * Remove a thread from the group. Does nothing if the thread is empty.
     *
     * @param thrd A pointer to the thread that shall be removed from the group
     */
    void
    remove_thread(thread* thrd)
    {
      if(!thrd) return;

      boost::lock_guard<mutex> guard(m_);
      thread_vector::iterator const it = std::find(threads_.begin(),
          threads_.end(), thread_ptr(thrd));
      if (it != threads_.end())
        {
          threads_.erase(it);
        }
    }

    /********************************************************************/
    /**
     * Requests all threads to join
     */
    void
    join_all(void)
    {
      boost::lock_guard<mutex> guard(m_);

      for (thread_vector::iterator it = threads_.begin(), end =
          threads_.end(); it != end; ++it)
        {
          (*it)->join();
        }
    }

    /********************************************************************/
    /**
     * Sends all threads the interrupt signal
     */
    void
    interrupt_all(void)
    {
      boost::lock_guard<mutex> guard(m_);

      for (thread_vector::iterator it = threads_.begin(), end =
          threads_.end(); it != end; ++it)
        {
          (*it)->interrupt();
        }
    }

    /********************************************************************/
    /**
     * Interrupts, joins and finally removes the last thread in the group.
     * Does nothing if the group is already empty.
     */
    void
    remove_last(void)
    {
      if(this->size() == 0) return;

      boost::lock_guard<mutex> guard(m_);

      (threads_.back())->interrupt();
      (threads_.back())->join();
      // boost::shared_ptr takes care of the deletion of the thread object
      threads_.pop_back();
    }

    /********************************************************************/
    /**
     * Interrupts, joins and finally removes the last nThreads threads
     * in the group. Stops if the number of threads would drop below 0.
     *
     * @param nThreads The number of threads at the end of the group that shall be removed
     */
    void
    remove_last(std::size_t nThreads){
      for(std::size_t i=0; i<nThreads; i++) remove_last();
    }

    /********************************************************************/
    /**
     * Returns the size of the current thread group.
     * @return The size of the current group
     */
    size_t
    size() const
    {
      boost::lock_guard<mutex> guard(m_);
      return threads_.size();
    }

  private:
    /********************************************************************/
    thread_vector threads_; ///< Holds the actual threads
    mutable mutex m_; ///< Needed to synchronize access to the vector
  };

}} /* namespace GenEvA */

#endif /* GTHREADGROUP_HPP_ */
