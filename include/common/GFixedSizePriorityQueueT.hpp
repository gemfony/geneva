/**
 * @file GFixedSizePriorityQueue.hpp
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

// Standard headers go here
#include <deque>
#include <algorithm>

// Includes check for correct Boost version(s)
#include "common/GGlobalDefines.hpp"

// Boost headers go here
#include <boost/shared_ptr.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/variant.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/deque.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/tracking.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/export.hpp>

#ifndef GFIXEDSIZEPRIORITYQUEUET_HPP_
#define GFIXEDSIZEPRIORITYQUEUET_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif

// Geneva headers go here
#include "common/GHelperFunctionsT.hpp"
#include "common/GExceptions.hpp"
#include "common/GLogger.hpp"
#include "common/GPODExpectationChecksT.hpp"

namespace Gem {
namespace Common {

/******************************************************************************/
/**
 * This class implements a fixed-size priority queue. Note that data items
 * are held inside of boost::shared_ptr objects and must be copy-constructible.
 * It is also required that T can be compared using operator== and operator!= .
 */
template <typename T>
class GFixedSizePriorityQueueT
{
   ///////////////////////////////////////////////////////////////////////
   friend class boost::serialization::access;

   template<typename Archive>
   void serialize(Archive & ar, const unsigned int){
     using boost::serialization::make_nvp;

     ar
     & BOOST_SERIALIZATION_NVP(data_)
     & BOOST_SERIALIZATION_NVP(maxSize_);
   }
   ///////////////////////////////////////////////////////////////////////

public:
   /***************************************************************************/
   /**
    * The default constructor
    */
   GFixedSizePriorityQueueT()
      : maxSize_(10)
   { /* nothing */ }

   /***************************************************************************/
   /**
    * The standard constructor
    *
    * @param maxSize The maximum size of the queue
    */
   GFixedSizePriorityQueueT(const std::size_t& maxSize)
      : maxSize_(maxSize)
   { /* nothing */ }

   /***************************************************************************/
   /**
    * The copy constructor
    */
   GFixedSizePriorityQueueT(const GFixedSizePriorityQueueT<T>& cp)
      : maxSize_(cp.maxSize_)
   {
      typename std::deque<boost::shared_ptr<T> >::const_iterator cit;
      for(cit=cp.data_.begin(); cit!=cp.data_.end(); ++cit) {
         data_.push_back(Gem::Common::clone_ptr(*cit));
      }
   }

   /***************************************************************************/
   /**
    * The destructor
    */
   virtual ~GFixedSizePriorityQueueT() {
      data_.clear();
   }

   /***************************************************************************/
   /**
    * Creates a deep clone of this object
    */
   boost::shared_ptr<GFixedSizePriorityQueueT> clone() {
      return boost::shared_ptr<GFixedSizePriorityQueueT>(new GFixedSizePriorityQueueT(*this));
   }

   /***************************************************************************/
   /**
    * Loads the data of another GFixedSizePriorityQueue<T> object
    */
   void load(const GFixedSizePriorityQueueT<T>& cp) {
#ifdef DEBUG
      if(cp.data_.size() != cp.maxSize_) {
         glogger
         << "In GFixedSizePriorityQueue<T>::load(): Error!" << std::endl
         << "maximum size " << cp.maxSize_ << " of cp does not match data size " << cp.data_.size() << std::endl
         << GEXCEPTION;
      }
#endif

      // Make sure data_ is empty
      data_.clear();

      // Copy all data over
      typename std::deque<boost::shared_ptr<T> >::const_iterator cit;
      for(cit=cp.data_.begin(); cit!=cp.data_.end(); ++cit) {
         data_.push_back(Gem::Common::clone_ptr(*cit));
      }

      // Copy the maximum data size
      maxSize_ = cp.maxSize_;
   }

   /***************************************************************************/
   /**
    * Copy the data of another GFixedSizePriorityQueueT<T> over
    */
   const GFixedSizePriorityQueueT<T>& operator=(const GFixedSizePriorityQueueT<T>& cp) {
      this->load(cp);
      return *this;
   }

   /***************************************************************************/
   /**
    * Checks whether a given expectation for the relationship between this object and another object
    * is fulfilled.
    *
    * @param cp A constant reference to another object of type GFixedSizePriorityQueueT<T>
    * @param e The expected outcome of the comparison
    * @param limit The maximum deviation for floating point values (important for similarity checks)
    * @param caller An identifier for the calling entity
    * @param y_name An identifier for the object that should be compared to this one
    * @param withMessages Whether or not information should be emitted in case of deviations from the expected outcome
    * @return A boost::optional<std::string> object that holds a descriptive string if expectations were not met
    */
   virtual boost::optional<std::string> checkRelationshipWith (
      const GFixedSizePriorityQueueT<T>& cp
      , const Gem::Common::expectation& e
      , const double& limit
      , const std::string& caller
      , const std::string& y_name
      , const bool& withMessages
   ) const BASE {
       using namespace Gem::Common;

      // Will hold possible deviations from the expectation, including explanations
       std::vector<boost::optional<std::string> > deviations;

       // Assemble a suitable caller string
       std::string className = std::string("GFixedSizePriorityQueueT<") + typeid(T).name() + ">";

      // No parent class to check ...

      // Check local data
      deviations.push_back(checkExpectation(withMessages, className , this->data_, cp.data_, "data_", "cp.data_", e , limit));
      deviations.push_back(checkExpectation(withMessages, className , this->maxSize_, cp.maxSize_, "maxSize_", "cp.maxSize_", e , limit));

      return evaluateDiscrepancies(className, caller, deviations, e);
   }

   /***************************************************************************/
   /**
    * Gives access to the best item without copying it
    */
   const T& best() const {
      if(data_.empty()) {
         // Throw an exception
         glogger
         << "In GFixedSizePriorityQueueT<T>::best(): Error!" << std::endl
         << "Priority queue is empty." << std::endl
         << GEXCEPTION;

         // Make the compiler happy
         return (T)0;
      } else {
         return data_.front();
      }
   }

   /***************************************************************************/
   /**
    * Gives access to the worst item without copying it
    */
   const T& worst() const {
      if(data_.empty()) {
         // Throw an exception
         glogger
         << "In GFixedSizePriorityQueueT<T>::best(): Error!" << std::endl
         << "Priority queue is empty." << std::endl
         << GEXCEPTION;

         // Make the compiler happy
         return (T)0;
      } else {
         return data_.back();
      }
   }

   /***************************************************************************/
   /**
    * Add an item to the queue. We also provide an evaluation function for this
    * object, so that we do not need to serialize that function. Note that the
    * comparator should sort the data in descending order (assuming that higher
    * values are better), so that the worst items are at the end of the queue.
    */
   void add(
      T item
      , boost::function<bool(const T&, const T&)> comp
   ) {
      data_.push_back(item);

      // Sort the data, so that worst items are at the end of the queue
      std::sort(data_.begin(), data_.end(), comp);

      // Remove surplus work items, if the queue has reached the corresponding size
      if(data_.size() > maxSize_) {
         data_.resize(maxSize_);
      }
   }

   /***************************************************************************/
   /**
    * Removes the best item from the queue and returns it
    */
   T pop() {
      if(data_.empty()) {
         // Throw an exception
         glogger
         << "In GFixedSizePriorityQueueT<T>::pop(): Error!" << std::endl
         << "Priority queue is empty." << std::endl
         << GEXCEPTION;

         // Make the compiler happy
         return (T)0;
      } else {
         T item = data_.front();
         data_.pop_back();
         return item;
      }
   }

   /***************************************************************************/
   /**
    * Returns the size of the queue
    */
   std::size_t size() const {
      return data_.size();
   }

   /***************************************************************************/
   /**
    * Checks whether the data is empty
    */
   bool empty() const {
      return data_.empty();
   }

   /***************************************************************************/
   /**
    * Sets the maximum size of the priority queue
    */
   void setMaxSize(std::size_t maxSize) {
      // Make sure the current size of data_ complies with maxSize
      if(data_.size() > maxSize) {
         data_.resize(maxSize);
      }

      maxSize_ = maxSize;
   }

   /***************************************************************************/
   /**
    * Retrieves the maximum size of the priority queue
    */
   std::size_t getMaxSize() const {
      return maxSize_;
   }

private:
   /***************************************************************************/

   std::deque<boost::shared_ptr<T> > data_; ///< Holds the actual data
   std::size_t maxSize_; ///< The maximum number of work-items
};

/******************************************************************************/

} /* namespace Common */
} /* namespace Gem */

#endif /* GFIXEDSIZEPRIORITYQUEUET_HPP_ */
