/**
 * @file
 */

/**
 * Copyright (C) 2004-2008 Dr. Ruediger Berlich
 * Copyright (C) 2007-2008 Forschungszentrum Karlsruhe GmbH
 *
 * This file is part of Geneva, Gemfony scientific's optimization library.
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
 */

// Standard header files go here
#include <sstream>

// Boost header files go here

#include <boost/version.hpp>

#if BOOST_VERSION < 103600
#error "Error: Boost should at least have version 1.36 !"
#endif /* BOOST_VERSION */

#include <boost/cstdint.hpp>
#include <boost/utility.hpp>


#ifndef GBIBUFFER_H_
#define GBIBUFFER_H_

// GenEvA header files go here

#include "GBoundedBuffer.hpp"

namespace Gem
{
namespace Util
{

  /*****************************************************************************/
  /**
   * A GBiBuffer<T> consists of two GBoundedBuffer<T> objects, one intended for "raw"
   * items, the other for returning, processed items. While this class could
   * be useful in many scenarios, the most common application is as a mediator
   * between GTransferPopulation and GConsumer-derivatives. The GTransferPopulation
   * is a source of raw items, which are processed by GConsumer-derivatives
   * (such as GBoostThreadConsumer and GAsioTCPConsumer) and then returned to the
   * population. GBroker-derivatives (such as the GMemberBroker) orchestrate this exchange.
   * All of this happens in a multi-threaded environment. It is not possible to
   * create copies of this class, as one GBiBuffer is intended to serve one
   * single population.
   */
  template <class T>
  class GBiBuffer
    :boost::noncopyable
  {
  public:
    /*****************************************************************************/
    /**
     * The default constructor. Note that, when using this constructor, the GBoundedBuffer
     * objects will assume the default sizes.
     */
    GBiBuffer(void)
    { /* nothing */ }

    /*****************************************************************************/
    /**
     * Here we initialize the two GBoundedBuffer objects with a given size.
     *
     * @param size The desired capacity of the GBoundedBuffer objects
     */
    explicit GBiBuffer(std::size_t size)
      :original_(size),
       processed_(size)
    { /* nothing */ }

    /*****************************************************************************/
    /**
     * Puts an item into the original queue. This is the queue for "raw" objects.
     *
     * @param item A raw object that needs to be processed
     */
    inline void push_front_orig(const T& item){
      original_.push_front(item);
    }

    /*****************************************************************************/
    /**
     * Timed version of GBiBuffer::push_front_orig() . If the item could not be added
     * after sec seconds and msec milliseconds, the function returns. Note that a time_out
     * exception will be thrown in this case.
     *
     * @param item An item to be added to the buffer
     * @param sec The second-part of the maximum waiting time
     * @param msec The millisecond-part of the maximum waiting time
     * @return A boolean indicating whether the operation was successful
     */
    inline void push_front_orig(const T& item, boost::uint32_t sec, boost::uint32_t msec){
    	original_.push_front(item, sec, msec);
    }

    /*****************************************************************************/
    /**
     * Retrieves an item from the back of the "original_" queue. Blocks until
     * an item could be retrieved.
     */
    inline void pop_back_orig(T* item){
    	original_.pop_back(item);
    }

    /*****************************************************************************/
    /**
     * A version of GBiBuffer::push_back_orig() with the ability to time-out. Note
     * that a time_out exception will be thrown by original_ if the time-out was
     * reached. It needs to be caught by the calling function.
     *
     * @param item The item that was retrieved from the queue
     * @param sec The second-part of the maximum waiting time
     * @param msec The millisecond-part of the maximum waiting time
     */
    inline void pop_back_orig(T *item, boost::uint32_t sec, boost::uint32_t msec){
    	original_.pop_back(item, sec, msec);
    }

    /*****************************************************************************/
    /**
     * Puts an item into the "processed" queue.
     *
     * @param item A raw object that needs to be processed
     */
    inline void push_front_processed(const T& item){
      processed_.push_front(item);
    }

    /*****************************************************************************/
    /**
     * Timed version of GBiBuffer::putProc() . If the item could not be added
     * after sec seconds and msec milliseconds, a timed_out exception will be thrown
     * by processed_.
     *
     * @param item An item to be added to the buffer
     * @param sec The second-part of the maximum waiting time
     * @param msec The millisecond-part of the maximum waiting time
     */
    inline void push_front_processed(const T& item, boost::uint32_t sec, boost::uint32_t msec){
    	processed_.push_front(item, sec, msec);
    }

    /*****************************************************************************/
    /**
     * Retrieves an item from the "processed" queue. This function will usually be
     * called directly or indirectly by GTransferPopulation.
     *
     * @param The item that was retrieved from the queue
     */
    inline void pop_back_processed(T* item){
    	processed_.pop_back(item);
    }

    /*****************************************************************************/
    /**
     * A version of GBiBuffer::getProc() with the ability to time-out. If the
     * time-out was reached, processed_ will throw a time_out exception.
     *
     * @param item The item that was retrieved from the queue
     * @param sec The second-part of the maximum waiting time
     * @param msec The millisecond-part of the maximum waiting time
     */
    inline void pop_back_processed(T* item, boost::uint32_t sec, boost::uint32_t msec){
    	processed_.pop_back(item, sec, msec);
    }

    /*****************************************************************************/

  private:
    Gem::Util::GBoundedBuffer<T> original_; ///< The queue for raw objects
    Gem::Util::GBoundedBuffer<T> processed_; ///< The queue for processed objects
  };

  /*****************************************************************************/

} /* namespace Util */
} /* namespace Gem */

#endif /* GBIBUFFER_H_ */
