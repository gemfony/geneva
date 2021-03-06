/********************************************************************************
 *
 * This file is part of the Geneva library collection. The following license
 * applies to this file:
 *
 * ------------------------------------------------------------------------------
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * ------------------------------------------------------------------------------
 *
 * Note that other files in the Geneva library collection may use a different
 * license. Please see the licensing information in each file.
 *
 ********************************************************************************
 *
 * Geneva was started by Dr. Rüdiger Berlich and was later maintained together
 * with Dr. Ariel Garcia under the auspices of Gemfony scientific. For further
 * information on Gemfony scientific, see http://www.gemfomy.eu .
 *
 * The majority of files in Geneva was released under the Apache license v2.0
 * in February 2020.
 *
 * See the NOTICE file in the top-level directory of the Geneva library
 * collection for a list of contributors and copyright information.
 *
 ********************************************************************************/

#pragma once

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard headers go here
#include <deque>
#include <algorithm>
#include <functional>

// Boost headers go here
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/deque.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/tracking.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/export.hpp>

// Geneva headers go here
#include "common/GCommonHelperFunctionsT.hpp"
#include "common/GExceptions.hpp"
#include <common/GExpectationChecksT.hpp>
#include "common/GLogger.hpp"
#include "common/GErrorStreamer.hpp"
#include "common/GCommonMathHelperFunctionsT.hpp"
#include "common/GCommonInterfaceT.hpp"

namespace Gem {
namespace Common {

/******************************************************************************/
/**
 * This class implements a fixed-size priority queue. Note that data items
 * are held inside of std::shared_ptr objects and must be copy-constructible.
 * It is also required that T can be compared using operator== and operator!= .
 * A maxSize_ of 0 stands for an unlimited size of the data vector.
 *
 * IMPORTANT: This class assumes that T has a member function clone<T>()
 * which returns a std::shared_ptr<T> as a copy of the T object.
 */
template<typename T>
class GFixedSizePriorityQueueT
	: public GCommonInterfaceT<GFixedSizePriorityQueueT<T>>
{
	 ///////////////////////////////////////////////////////////////////////
	 friend class boost::serialization::access;

	 template<typename Archive>
	 void serialize(Archive &ar, const unsigned int) {
		 using boost::serialization::make_nvp;

		 ar
		 & BOOST_SERIALIZATION_NVP(m_data_deq)
		 & BOOST_SERIALIZATION_NVP(m_maxSize)
		 & BOOST_SERIALIZATION_NVP(m_sortOrder);
	 }
	 ///////////////////////////////////////////////////////////////////////

public:
	 /***************************************************************************/
	 /**
	  * Initialization with the maximum number of entries
	  *
	  * @param maxSize The maximum size of the queue
	  */
	 explicit GFixedSizePriorityQueueT(size_t maxSize)
		 : m_maxSize(maxSize)
	 { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * Initialization with the maximum number of entries and the information,
	  * whether higher or lower evaluations are better.
	  *
	  * @param maxSize The maximum size of the queue
	  */
	 GFixedSizePriorityQueueT(
         size_t maxSize
         , sortOrder sortOrder
     )
		 : m_maxSize(maxSize)
		 , m_sortOrder(sortOrder)
	 { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * The copy constructor
	  */
	 GFixedSizePriorityQueueT(GFixedSizePriorityQueueT<T> const &cp)
		 : m_maxSize(cp.m_maxSize)
		 , m_sortOrder(cp.m_sortOrder)
	 {
         Gem::Common::copyCloneableSmartPointerContainer(cp.m_data_deq, m_data_deq);
	 }

	 /***************************************************************************/
	 /**
	  * The move constructor
	  */
	 GFixedSizePriorityQueueT(GFixedSizePriorityQueueT<T> && cp) noexcept {
	 	m_data_deq = std::move(cp.m_data_deq);
	 	cp.m_data_deq.clear();

	 	m_maxSize = cp.m_maxSize;
	 	cp.m_maxSize = 10; // default value

	 	m_sortOrder = cp.m_sortOrder;
	 	cp.m_sortOrder = Gem::Common::sortOrder::LOWERISBETTER;
	 }

	 /***************************************************************************/
	 // Defaulted functions

	 GFixedSizePriorityQueueT() = default;
	 virtual ~GFixedSizePriorityQueueT() = default;

	 /***************************************************************************/
	 /**
	  * Assignment operator
	  */
	 GFixedSizePriorityQueueT<T> & operator=(GFixedSizePriorityQueueT<T> const & cp) {
		 m_maxSize = cp.m_maxSize;
		 m_sortOrder = cp.m_sortOrder;

         Gem::Common::copyCloneableSmartPointerContainer(cp.m_data_deq, m_data_deq);

		 return *this;
	 }

	 /***************************************************************************/
	 /**
      * Assignment operator
      */
	 GFixedSizePriorityQueueT<T> & operator=(GFixedSizePriorityQueueT<T> && cp) noexcept {
		m_maxSize = cp.m_maxSize;
		cp.m_maxSize = 10;

		m_sortOrder = cp.m_sortOrder;
		cp.m_sortOrder = Gem::Common::sortOrder::LOWERISBETTER;

		m_data_deq = std::move(cp.m_data_deq);
		cp.m_data_deq.clear();

		return *this;
	 }

	 /***************************************************************************/
	 /**
	  * Gives access to the best item without copying it
	  */
	 std::shared_ptr<T> best() const {
		 if (m_data_deq.empty()) {
			 // Throw an exception
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG, time_and_place)
					 << "In GFixedSizePriorityQueueT<T>::best(): Error!" << std::endl
					 << "Priority queue is empty." << std::endl
			 );
		 } else {
			 return m_data_deq.front();
		 }
	 }

	 /***************************************************************************/
	 /**
	  * Gives access to the worst item without copying it
	  */
	 std::shared_ptr<T> worst() const {
		 if (m_data_deq.empty()) {
			 // Throw an exception
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG, time_and_place)
					 << "In GFixedSizePriorityQueueT<T>::best(): Error!" << std::endl
					 << "Priority queue is empty." << std::endl
			 );
		 } else {
			 return m_data_deq.back();
		 }
	 }

	 /***************************************************************************/
	 /**
	  * Allows to set the priority mode. A value of "HIGHERISBETTER" means that higher
	  * values are considered better, "false" means that lower values are
	  * considered to be better.
	  */
	 void setSortOrder(Gem::Common::sortOrder sortOrder) {
		 m_sortOrder = sortOrder;
	 }

	 /***************************************************************************/
	 /**
	  * Allows to retrieve the current value of m_sortOrder
	  */
	 Gem::Common::sortOrder getSortOrder() const {
		 return m_sortOrder;
	 }

	 /***************************************************************************/
	 /**
	  * Add an item to the queue. Note that the comparator used in this function
	  * should sort the data in descending order (assuming that higher
	  * values are better) or ascending order (if lower values are better),
	  * so that the worst items are always at the end of the queue.
	  *
	  * @param item The item to be added to the queue
	  * @param do_clone If set to true, work items will be cloned. Otherwise only the smart pointer will be added
	  */
	 virtual void add(
		 std::shared_ptr<T> const & item
		 , bool do_clone
	 ) BASE {
		 // Add the work item to the queue
		 // - If the queue is unlimited
		 // - If the queue isn't full yet
		 // - If the item is better than the worst one contained in the queue
		 if (0 == m_maxSize || m_data_deq.size() < m_maxSize ||
			  isBetter(this->evaluation(item), this->evaluation(this->worst()))) {
			 if (do_clone) {
				 m_data_deq.push_back(item->template clone<T>());
			 } else {
				 m_data_deq.push_back(item);
			 }
		 }

		 // Sort the data according to their ids, so we may remove duplicates
		 std::sort(
			 m_data_deq.begin()
			 , m_data_deq.end()
			 , [this](std::shared_ptr<T> const & x_ptr, std::shared_ptr<T> const & y_ptr) -> bool {
				 return (this->id(x_ptr) < this->id(y_ptr));
			 }
		 );

		 // Remove duplicate items
		 m_data_deq.erase(
			 std::unique(
				 m_data_deq.begin()
				 , m_data_deq.end()
				 , [this](std::shared_ptr<T> const & x_ptr, std::shared_ptr <T> const & y_ptr) -> bool {
					 return (this->id(x_ptr) == this->id(y_ptr));
				 }
			 ), m_data_deq.end()
		 );

		 // Sort the data according to the evaluation
		 std::sort(
			 m_data_deq.begin()
			 , m_data_deq.end()
			 , [this](std::shared_ptr<T> const & x_ptr, std::shared_ptr<T> const & y_ptr) -> bool {
				 if (this->getSortOrder() == Gem::Common::sortOrder::LOWERISBETTER) { // higher is better
					 return this->evaluation(x_ptr) < this->evaluation(y_ptr);
				 } else { // HIGHERISBETTER
					 return this->evaluation(x_ptr) > this->evaluation(y_ptr);
				 }
			 }
		 );

		 // Remove surplus work items, if the queue has reached the corresponding size
		 // As the worst items are not at the end of the queue, they will be removed, if
		 // they are beyond the allowed size. This will only have an effect if m_maxSize is != 0 .
		 if (m_maxSize && m_data_deq.size() > m_maxSize) {
			 m_data_deq.resize(m_maxSize);
		 }
	 }

	 /***************************************************************************/
	 /**
	  * Add a set of items to the queue. Note that the comparator used in this
	  * function should sort the data in descending order (assuming that higher
	  * values are better) or ascending order (if lower values are better),
	  * so that the worst items are always at the end of the queue.
	  *
	  * @param items The items to be added to the queue
	  * @param do_clone If set to true, work items will be cloned. Otherwise only the smart pointer will be added
	  * @param replace If set to true, the queue will be emptied before adding new work items
	  */
	 virtual void add(
		 std::vector<std::shared_ptr<T>> const & items
		 , bool do_clone
		 , bool replace
	 ) BASE {
		 double worstKnownEvaluation = Gem::Common::getWorstCase<double>(m_sortOrder);
		 if (replace || m_data_deq.empty()) {
			 m_data_deq.clear();
		 } else {
			 // Data already exists, we know better than the worst known valid
			 worstKnownEvaluation = this->evaluation(this->worst());
		 }

		 // At this point, worstKnownEvaluation will be
		 // - the worst case, if the queue is empty or all entries in the queue will be replaced
		 // - the evaluation of the worst entry in the queue if we only add items (regardless of whether they will be cloned or not)
		 for(auto const & item_ptr: items) {
			 // Add the work item to the queue
			 // - If the queue is unlimited
			 // - If the queue isn't full yet
			 // - If the item is better than the worst one already contained in the queue
			 if (0 == m_maxSize || m_data_deq.size() < m_maxSize || isBetter(this->evaluation(item_ptr), worstKnownEvaluation)) {
				 if (do_clone) {
					 m_data_deq.push_back(item_ptr->template clone<T>());
				 } else {
					 m_data_deq.push_back(item_ptr);
				 }
			 }
		 }

		 // Sort the data according to their ids, so we may remove duplicates
		 std::sort(
			 m_data_deq.begin()
			 , m_data_deq.end()
			 , [this](std::shared_ptr<T> const & x_ptr, std::shared_ptr<T> const & y_ptr) -> bool {
				 return (this->id(x_ptr) < this->id(y_ptr));
			 }
		 );

		 // Remove duplicate items
		 m_data_deq.erase(
			 std::unique(
				 m_data_deq.begin()
				 , m_data_deq.end()
				 , [this](std::shared_ptr<T> const & x_ptr, std::shared_ptr<T> const & y_ptr) -> bool {
					 return (this->id(x_ptr) == this->id(y_ptr));
				 }
			 ), m_data_deq.end()
		 );

		 std::sort(
			 m_data_deq.begin()
			 , m_data_deq.end()
			 , [this](std::shared_ptr<T> const & x_ptr, std::shared_ptr<T> const & y_ptr) -> bool {
				 if (this->getSortOrder() == Gem::Common::sortOrder::LOWERISBETTER) {
					 return this->evaluation(x_ptr) < this->evaluation(y_ptr);
				 } else {
					 return this->evaluation(x_ptr) > this->evaluation(y_ptr);
				 }
			 }
		 );

		 // Remove surplus work items, if the queue has reached the corresponding size
		 // This will only have an effect if m_maxSize is != 0
		 if (m_maxSize && m_data_deq.size() > m_maxSize) {
			 m_data_deq.resize(m_maxSize);
		 }
	 }

	 /***************************************************************************/
	 /**
	  * Removes the best item from the queue and returns it
	  */
	 std::shared_ptr<T> pop() {
		 if (m_data_deq.empty()) {
			 // Throw an exception
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG, time_and_place)
					 << "In GFixedSizePriorityQueueT<T>::pop(): Error!" << std::endl
					 << "Priority queue is empty." << std::endl
			 );
		 } else {
			 auto item_ptr = m_data_deq.front();
			 m_data_deq.pop_front();
			 return item_ptr;
		 }
	 }

	 /***************************************************************************/
	 /**
	  * Converts the local deque to a std::vector and returns it
	  */
	 std::vector<std::shared_ptr<T>> toVector() const {
		 std::vector<std::shared_ptr<T>> result;

		 for(auto const & item_ptr: m_data_deq) {
			 result.push_back(item_ptr);
		 }

		 return result;
	 }

	 /***************************************************************************/
	 /**
	  * Returns the current size of the queue
	  */
	 std::size_t size() const {
		 return m_data_deq.size();
	 }

	 /***************************************************************************/
	 /**
	  * Checks whether the data is empty
	  */
	 bool empty() const {
		 return m_data_deq.empty();
	 }

	 /***************************************************************************/
	 /**
	  * Allows to clear the queue
	  */
	 void clear() {
		 m_data_deq.clear();
	 }

	 /***************************************************************************/
	 /**
	  * Sets the maximum size of the priority queue
	  */
	 void setMaxSize(std::size_t maxSize) {
		 // Make sure the current size of m_data complies with maxSize
		 if (m_data_deq.size() > maxSize) {
			 m_data_deq.resize(maxSize);
		 }

		 m_maxSize = maxSize;
	 }

	 /***************************************************************************/
	 /**
	  * Retrieves the maximum size of the priority queue
	  */
	 std::size_t getMaxSize() const {
		 return m_maxSize;
	 }

protected:
	 /***************************************************************************/
	 /**
	  * Loads the data of another GFixedSizePriorityQueue<T> object
	  */
	 void load_(const GFixedSizePriorityQueueT<T> *cp) override {
		 // Check that we are dealing with a GFixedSizePriorityQueueT<T> reference independent of this object and convert the pointer
		 const GFixedSizePriorityQueueT<T> *p_load
		 	= Gem::Common::g_convert_and_compare<GFixedSizePriorityQueueT<T>, GFixedSizePriorityQueueT<T>>(cp, this);

		 // Load local data
		 Gem::Common::copyCloneableSmartPointerContainer(p_load->m_data_deq, m_data_deq);
		 m_maxSize = p_load->m_maxSize;
		 m_sortOrder = p_load->m_sortOrder;
	 }

	/***************************************************************************/
	/** @brief Allow access to this classes compare_ function */
	friend void Gem::Common::compare_base_t<GFixedSizePriorityQueueT<T>>(
		GFixedSizePriorityQueueT<T> const &
		, GFixedSizePriorityQueueT<T> const &
		, Gem::Common::GToken &
	);

	/***************************************************************************/
	/**
     * Checks for compliance with expectations with respect to another object
     * of the same type
     *
     * @param cp A constant reference to another GFixedSizePriorityQueueT<T> object
     * @param e The expected outcome of the comparison
     * @param limit The maximum deviation for floating point values (important for similarity checks)
     */
	void compare_(
		const GFixedSizePriorityQueueT<T> &cp
		, const Gem::Common::expectation &e
		, const double &limit
	) const override {
		using namespace Gem::Common;

		// Check that we are dealing with a GFixedSizePriorityQueueT<T> reference independent of this object and convert the pointer
		const GFixedSizePriorityQueueT<T> *p_load
			= Gem::Common::g_convert_and_compare<GFixedSizePriorityQueueT<T>, GFixedSizePriorityQueueT<T>>(cp, this);

		GToken token("GFixedSizePriorityQueueT<T>", e);

		// Compare our parent data ...
		Gem::Common::compare_base_t<GCommonInterfaceT<GFixedSizePriorityQueueT<T>>>(*this, *p_load, token);

		// ... and then our local data
		compare_t(IDENTITY(m_data_deq, p_load->m_data_deq), token);
		compare_t(IDENTITY(m_maxSize, p_load->m_maxSize), token);
		compare_t(IDENTITY(m_sortOrder, p_load->m_sortOrder), token);

		// React on deviations from the expectation
		token.evaluate();
	}

	/***************************************************************************/
	 /**
	  * Checks whether value new_item is better than value old_item
	  */
	 bool isBetter(
	 	std::shared_ptr<T> const& new_item_ptr
	 	, std::shared_ptr<T> const& old_item_ptr
	 ) const {
	     return this->isBetter(this->evaluation(new_item_ptr), this->evaluation(old_item_ptr));
	 }

	 /***************************************************************************/
	 /**
	  * Checks whether value new_item is better than value old_item
	  */
	 bool isBetter(
	 	std::shared_ptr<T> const& new_item_ptr
	 	, double old_item_val
	 ) const {
         return this->isBetter(this->evaluation(new_item_ptr), old_item_val);
	 }

	 /***************************************************************************/
	 /**
	  * Checks whether value new_item is better than value old_item
	  */
	 bool isBetter(
         double new_item_val
         , std::shared_ptr<T> const& old_item_ptr
     ) const {
         return this->isBetter(new_item_val, this->evaluation(old_item_ptr));
	 }

	 /***************************************************************************/
	 /**
	  * Checks whether value new_item is better than value old_item
	  */
	 bool isBetter(
         double new_item_val
	 	, double old_item_val
	) const {
	     return
	        (m_sortOrder == Gem::Common::sortOrder::LOWERISBETTER)
	        ? (new_item_val < old_item_val)
	        : (new_item_val > old_item_val);
	 }

	 /***************************************************************************/
	 /** @brief Evaluates a single work item, so that it can be sorted */
	 virtual G_API_COMMON double evaluation(const std::shared_ptr<T>&) const BASE = 0;

	 /** @brief Returns a unique id for a work item */
	 virtual G_API_COMMON std::string id(const std::shared_ptr<T> &) const BASE = 0;

	 /***************************************************************************/
	 /**
	  * Applies modifications to this object. This is needed for testing purposes
	  */
	 bool modify_GUnitTests_() override {
	     return false;
	 }

	 /***************************************************************************/
	 /**
	  * Performs self tests that are expected to succeed. This is needed for testing purposes
	  */
	 void specificTestsNoFailureExpected_GUnitTests_() override
	 { /* nothing */ }

	/***************************************************************************/
	 /**
	  * Performs self tests that are expected to fail. This is needed for testing purposes
	  */
	 void specificTestsFailuresExpected_GUnitTests_() override
	 { /* nothing */ }

	/***************************************************************************/

	 std::deque<std::shared_ptr<T>> m_data_deq; ///< Holds the actual data

	 std::size_t m_maxSize = 10; ///< The maximum number of work-items
	 Gem::Common::sortOrder m_sortOrder = Gem::Common::sortOrder::LOWERISBETTER; ///< Indicates whether higher evaluations of items indicate a higher priority

private:
	 /***************************************************************************/
	 /**
	  * Returns the name of this class
	  */
	 std::string name_() const override {
		 return std::string("GFixedSizePriorityQueueT<T>");
	 }

	 /***************************************************************************/
	 /** @brief Creates a deep clone of this object */
	 G_API_COMMON GFixedSizePriorityQueueT<T> * clone_() const override = 0;
};

/******************************************************************************/

} /* namespace Common */
} /* namespace Gem */
