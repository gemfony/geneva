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
 * See the NOTICE file in the top-level directory of the Geneva library
 * collection for a list of contributors and copyright information.
 *
 ********************************************************************************/

#pragma once

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard headers go here
#include <algorithm>
#include <deque>
#include <unordered_set>

// Boost headers go here
#include <boost/serialization/deque.hpp>
#include <boost/serialization/nvp.hpp>

// Geneva headers go here
#include "common/GCommonHelperFunctionsT.hpp"
#include "common/GCommonInterfaceT.hpp"
#include "common/GCommonMathHelperFunctionsT.hpp"
#include "common/GErrorStreamer.hpp"
#include "common/GExceptions.hpp"
#include "common/GLogger.hpp"
#include <common/GExpectationChecksT.hpp>

namespace Gem::Common {
// Some default values
constexpr std::size_t GFSPQ_DEF_MAX_SIZE = 10;
constexpr auto GFSPQ_DEF_SORT_ORDER = Gem::Common::sortOrder::LOWERISBETTER;

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
template <typename T>
class GFixedSizePriorityQueueT : public GCommonInterfaceT<GFixedSizePriorityQueueT<T>> {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        ar &BOOST_SERIALIZATION_NVP(maxSize_) & BOOST_SERIALIZATION_NVP(sortOrder_) &
            BOOST_SERIALIZATION_NVP(data_deq_);
    }

    ///////////////////////////////////////////////////////////////////////

public:
    /***************************************************************************/
    /**
         * Initialization with the maximum number of entries
         *
         * @param maxSize The maximum size of the queue
         */
    explicit GFixedSizePriorityQueueT(const std::size_t &maxSize)
      : maxSize_(maxSize) {
        /* nothing */
    }

    /***************************************************************************/
    /**
         * Initialization with the maximum number of entries and the information,
         * whether higher or lower evaluations are better.
         *
         * @param maxSize The maximum size of the queue
         * @param sortOrder Indicates whether the queue should minimize or maximize
         */
    GFixedSizePriorityQueueT(const std::size_t &maxSize, const sortOrder &sortOrder)
      : maxSize_(maxSize)
      , sortOrder_(sortOrder) {
        /* nothing */
    }

    /***************************************************************************/
    /**
         * The copy constructor
         */
    GFixedSizePriorityQueueT(GFixedSizePriorityQueueT const &cp)
      : maxSize_(cp.maxSize_)
      , sortOrder_(cp.sortOrder_) {
        Common::copyCloneableSmartPointerContainer(cp.data_deq_, data_deq_);
    }

    /***************************************************************************/
    /**
         * The move constructor
         */
    GFixedSizePriorityQueueT(GFixedSizePriorityQueueT &&cp) noexcept {
        // Move content, then reset cp to default values
        maxSize_ = cp.maxSize_;
        cp.maxSize_ = GFSPQ_DEF_MAX_SIZE;

        sortOrder_ = cp.sortOrder_;
        cp.sortOrder_ = GFSPQ_DEF_SORT_ORDER;

        data_deq_ = std::move(cp.data_deq_);
        cp.data_deq_.clear();
    }

    /***************************************************************************/
    // Defaulted functions

    GFixedSizePriorityQueueT() = default;
    virtual ~GFixedSizePriorityQueueT() = default;

    /***************************************************************************/
    /**
         * Assignment operator
         */
    GFixedSizePriorityQueueT &operator=(GFixedSizePriorityQueueT const &cp) {
        if(this == &cp)
            return *this;
        maxSize_ = cp.maxSize_;
        sortOrder_ = cp.sortOrder_;

        Common::copyCloneableSmartPointerContainer(cp.data_deq_, data_deq_);

        return *this;
    }

    /***************************************************************************/
    /**
         * Assignment operator
         */
    GFixedSizePriorityQueueT &operator=(GFixedSizePriorityQueueT &&cp) noexcept {
        // Move data over, then set remote object to default values
        maxSize_ = cp.maxSize_;
        cp.maxSize_ = GFSPQ_DEF_MAX_SIZE;

        sortOrder_ = cp.sortOrder_;
        cp.sortOrder_ = GFSPQ_DEF_SORT_ORDER;

        data_deq_ = std::move(cp.data_deq_);
        cp.data_deq_.clear();

        return *this;
    }

    /***************************************************************************/
    /**
         * Gives access to the best item without copying it
         */
    std::shared_ptr<T> best() const {
        if(data_deq_.empty()) {
            // Throw an exception
            throw geneva_exception(
                g_error_streamer(DO_LOG, time_and_place)
                << "In GFixedSizePriorityQueueT<T>::best(): Error!" << '\n'
                << "Priority queue is empty." << '\n'
            );
        }
        else {
            return data_deq_.front();
        }
    }

    /***************************************************************************/
    /**
         * Gives access to the worst item without copying it
         */
    std::shared_ptr<T> worst() const {
        if(data_deq_.empty()) {
            // Throw an exception
            throw geneva_exception(
                g_error_streamer(DO_LOG, time_and_place)
                << "In GFixedSizePriorityQueueT<T>::best(): Error!" << '\n'
                << "Priority queue is empty." << '\n'
            );
        }
        else {
            return data_deq_.back();
        }
    }

    /***************************************************************************/
    /**
         * Allows to set the priority mode. A value of "HIGHERISBETTER" means that higher
         * values are considered better, "false" means that lower values are
         * considered to be better.
         */
    void setSortOrder(const sortOrder &sortOrder) {
        sortOrder_ = sortOrder;
    }

    /***************************************************************************/
    /**
         * Allows to retrieve the current value of sortOrder_
         */
    sortOrder getSortOrder() const {
        return sortOrder_;
    }

    /***************************************************************************/
    /**
         * Add an item to the queue. Note that the comparator used in this function
         * should sort the data in descending order (assuming that higher
         * values are better) or ascending order (if lower values are better),
         * so that the worst items are always at the end of the queue.
         *
         * @param item_ptr The item to be added to the queue
         * @param do_clone If set to true, work items will be cloned. If not, then only the smart pointer will be added
         */
    virtual void add(std::shared_ptr<T> const &item_ptr, const bool do_clone) {
        // Add the work item to the queue
        // - If the queue is unlimited
        // - If the queue isn't full yet
        // - If the item is better than the worst one contained in the queue
        if(0 == maxSize_ || data_deq_.size() < maxSize_ ||
           isBetter(this->evaluation(item_ptr), this->evaluation(this->worst()))) {
            if(do_clone) {
                data_deq_.push_back(item_ptr->template clone<T>());
            }
            else {
                data_deq_.push_back(item_ptr);
            }
        }

        // Remove duplicate items
        removeDuplicates(data_deq_);

        // Sort the data according to the evaluation
        std::sort(
            data_deq_.begin(),
            data_deq_.end(),
            [this](std::shared_ptr<T> const &x_ptr, std::shared_ptr<T> const &y_ptr) -> bool {
                if(this->getSortOrder() == sortOrder::LOWERISBETTER) {
                    // higher is better
                    return this->evaluation(x_ptr) < this->evaluation(y_ptr);
                }
                else {
                    // HIGHERISBETTER
                    return this->evaluation(x_ptr) > this->evaluation(y_ptr);
                }
            }
        );

        // Remove surplus work items, if the queue has reached the corresponding size
        // As the worst items are not at the end of the queue, they will be removed, if
        // they are beyond the allowed size. This will only have an effect if maxSize_ is != 0 .
        if(maxSize_ && data_deq_.size() > maxSize_) {
            data_deq_.resize(maxSize_);
        }
    }

    /***************************************************************************/
    /**
         * Adds a range of items to the priority queue.
         */
    virtual void
    add(typename std::vector<std::shared_ptr<T>>::const_iterator begin,
        typename std::vector<std::shared_ptr<T>>::const_iterator end,
        bool do_clone,
        bool replace) {
        double worstKnownEvaluation = Gem::Common::getWorstCase<double>(sortOrder_);
        if(replace || data_deq_.empty()) {
            data_deq_.clear();
        }
        else {
            // Data already exists, we know better than the worst known valid
            worstKnownEvaluation = this->evaluation(this->worst());
        }

        // At this point, worstKnownEvaluation will be
        // - the worst case, if the queue is empty or all entries in the queue will be replaced
        // - the evaluation of the worst entry in the queue if we only add items (regardless of whether they will be cloned or not)
        for(auto it = begin; it != end; ++it) {
            // Dereference the iterator
            auto item_ptr = *it;

            // Only act on "filled" item_ptr
            if(not(item_ptr))
                continue;

            // Add the work item to the queue
            // - If the queue is unlimited
            // - If the queue isn't full yet
            // - If the item is better than the worst one already contained in the queue
            if(0 == maxSize_ || data_deq_.size() < maxSize_ ||
               isBetter(this->evaluation(item_ptr), worstKnownEvaluation)) {
                if(do_clone) {
                    data_deq_.push_back(item_ptr->template clone<T>());
                }
                else {
                    data_deq_.push_back(item_ptr);
                }
            }
        }

        // Remove duplicate items
        removeDuplicates(data_deq_);

        // Sort according to the evaluation in ascending or descending order
        std::sort(
            data_deq_.begin(),
            data_deq_.end(),
            [this](std::shared_ptr<T> const &x_ptr, std::shared_ptr<T> const &y_ptr) -> bool {
                if(this->getSortOrder() == sortOrder::LOWERISBETTER) {
                    return this->evaluation(x_ptr) < this->evaluation(y_ptr);
                }
                else {
                    return this->evaluation(x_ptr) > this->evaluation(y_ptr);
                }
            }
        );

        // Remove surplus work items, if the queue has reached the corresponding size
        // This will only have an effect if maxSize_ is != 0
        if(maxSize_ && data_deq_.size() > maxSize_) {
            data_deq_.resize(maxSize_);
        }
    }

    /***************************************************************************/
    /**
         * Add a set of items to the queue. Note that the comparator used in this
         * function should sort the data in descending order (assuming that higher
         * values are better) or ascending order (if lower values are better),
         * so that the worst items are always at the end of the queue.
         *
         * @param item_ptr_vec The items to be added to the queue
         * @param do_clone If set to true, work items will be cloned. Otherwise only the smart pointer will be added
         * @param replace If set to true, the queue will be emptied before adding new work items
         */
    virtual void
    add(std::vector<std::shared_ptr<T>> const &item_ptr_vec, bool do_clone, bool replace) {
        this->add(item_ptr_vec.begin(), item_ptr_vec.end(), do_clone, replace);
    }

    /***************************************************************************/
    /**
             * Removes the best item from the queue and returns it
             */
    std::shared_ptr<T> pop() {
        if(data_deq_.empty()) {
            // Throw an exception
            throw geneva_exception(
                g_error_streamer(DO_LOG, time_and_place)
                << "In GFixedSizePriorityQueueT<T>::pop(): Error!" << '\n'
                << "Priority queue is empty." << '\n'
            );
        }
        else {
            auto item_ptr = data_deq_.front();
            data_deq_.pop_front();
            return item_ptr;
        }
    }

    /***************************************************************************/
    /**
             * Converts the local deque to a std::vector and returns it
             */
    std::vector<std::shared_ptr<T>> toVector() const {
        std::vector<std::shared_ptr<T>> result;

        for(auto const &item_ptr : data_deq_) {
            result.push_back(item_ptr);
        }

        return result;
    }

    /***************************************************************************/
    /**
             * Returns the current size of the queue
             */
    std::size_t size() const {
        return data_deq_.size();
    }

    /***************************************************************************/
    /**
             * Checks whether the data is empty
             */
    bool empty() const {
        return data_deq_.empty();
    }

    /***************************************************************************/
    /**
             * Allows to clear the queue
             */
    void clear() {
        data_deq_.clear();
    }

    /***************************************************************************/
    /**
             * Sets the maximum size of the priority queue
             */
    void setMaxSize(std::size_t maxSize) {
        // Make sure the current size of data_ complies with maxSize
        if(data_deq_.size() > maxSize) {
            data_deq_.resize(maxSize);
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

    /***************************************************************************/
    /**
             * Prints the evaluations. This is for debugging purposes.
             */
    void printEvaluations() const {
        std::cout << "==================== printEvaluations =====================" << '\n';
        for(auto const &item_ptr : data_deq_) {
            std::cout << this->evaluation(item_ptr) << '\n';
        }
    }

protected:
    /***************************************************************************/
    /**
             * Loads the data of another GFixedSizePriorityQueue<T> object
             */
    void load_(const GFixedSizePriorityQueueT *cp) override {
        // Check that we are dealing with a GFixedSizePriorityQueueT<T> reference independent of this object and convert the pointer
        const GFixedSizePriorityQueueT *p_load =
            Common::g_convert_and_compare<GFixedSizePriorityQueueT, GFixedSizePriorityQueueT>(
                cp,
                this
            );

        // Load local data
        maxSize_ = p_load->maxSize_;
        sortOrder_ = p_load->sortOrder_;
        Common::copyCloneableSmartPointerContainer(p_load->data_deq_, data_deq_);
    }

    /***************************************************************************/
    /** @brief Allow access to this classes compare_ function */
    friend void Common::compare_base_t<GFixedSizePriorityQueueT>(
        GFixedSizePriorityQueueT const &,
        GFixedSizePriorityQueueT const &,
        GToken &
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
        const GFixedSizePriorityQueueT &cp,
        const expectation &e,
        const double & /*limit*/
    ) const override {
        using namespace Gem::Common;

        // Check that we are dealing with a GFixedSizePriorityQueueT<T> reference independent of this object and convert the pointer
        const GFixedSizePriorityQueueT *p_load =
            Common::g_convert_and_compare<GFixedSizePriorityQueueT, GFixedSizePriorityQueueT>(
                cp,
                this
            );

        GToken token("GFixedSizePriorityQueueT<T>", e);

        // Compare our parent data ...
        Common::compare_base_t<GCommonInterfaceT<GFixedSizePriorityQueueT>>(*this, *p_load, token);

        // ... and then our local data
        compare_t(IDENTITY(maxSize_, p_load->maxSize_), token);
        compare_t(IDENTITY(sortOrder_, p_load->sortOrder_), token);
        compare_t(IDENTITY(data_deq_, p_load->data_deq_), token);

        // React on deviations from the expectation
        token.evaluate();
    }

    /***************************************************************************/
    /**
             * Checks whether value new_item is better than value old_item
             */
    bool
    isBetter(std::shared_ptr<T> const &new_item_ptr, std::shared_ptr<T> const &old_item_ptr) const {
        return this->isBetter(this->evaluation(new_item_ptr), this->evaluation(old_item_ptr));
    }

    /***************************************************************************/
    /**
             * Checks whether value new_item is better than value old_item
             */
    bool isBetter(std::shared_ptr<T> const &new_item_ptr, double old_item_val) const {
        return this->isBetter(this->evaluation(new_item_ptr), old_item_val);
    }

    /***************************************************************************/
    /**
             * Checks whether value new_item is better than value old_item
             */
    bool isBetter(double new_item_val, std::shared_ptr<T> const &old_item_ptr) const {
        return this->isBetter(new_item_val, this->evaluation(old_item_ptr));
    }

    /***************************************************************************/
    /**
         * Checks whether value new_item is *strictly* better than value old_item.
         *
         * Both branches use a strict comparison (`<` / `>`) so the ordering is
         * symmetric: equal values are never reported as "better", regardless of
         * sort direction. The previous LOWERISBETTER branch used `<=`, which
         * treated equality as "better" only for one direction — making
         * incumbent-replacement behave differently for the two sort orders.
         */
    bool isBetter(double new_item_val, double old_item_val) const {
        return (sortOrder_ == sortOrder::LOWERISBETTER) ? (new_item_val < old_item_val)
                                                          : (new_item_val > old_item_val);
    }

    /***************************************************************************/
    /** @brief Checks whether an Item is valid */
    virtual bool isValid(const std::shared_ptr<T> &) const = 0;
    /** @brief Evaluates a single work item, so that it can be sorted */
    virtual double evaluation(const std::shared_ptr<T> &) const = 0;

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
    void specificTestsNoFailureExpected_GUnitTests_() override {
        /* nothing */
    }

    /***************************************************************************/
    /**
             * Performs self tests that are expected to fail. This is needed for testing purposes
             */
    void specificTestsFailuresExpected_GUnitTests_() override {
        /* nothing */
    }

    /***************************************************************************/

    std::size_t maxSize_{GFSPQ_DEF_MAX_SIZE}; ///< The maximum number of work-items
    sortOrder sortOrder_{sortOrder::LOWERISBETTER};
    ///< Indicates whether higher evaluations of items indicate a higher priority

    std::deque<std::shared_ptr<T>>
        data_deq_{}; ///< Holds the actual data. Empty at the beginning.

private:
    /***************************************************************************/
    /**
         * Uses the storage addresses of individuals to remove duplicates
         */
    void removeDuplicates(std::deque<std::shared_ptr<T>> &items) {
        // Stores addresses we have already encountered
        std::unordered_set<T *> knownAddresses;

        // Iterator for removing elements efficiently
        auto it = items.begin();
        while(it != items.end()) {
            // Check if storage address is already known
            if(T *raw_ptr = it->get(); knownAddresses.find(raw_ptr) != knownAddresses.end()) {
                // Found a duplicate -- remove it. "it" will then point to
                // the next element.
                it = items.erase(it);
            }
            else {
                // Add the storage address to our set and move to the next item
                knownAddresses.insert(raw_ptr);
                ++it;
            }
        }
    }

    /***************************************************************************/
    /**
             * Returns the name of this class
             */
    std::string name_() const override {
        return std::string("GFixedSizePriorityQueueT<T>");
    }

    /***************************************************************************/
    /** @brief Creates a deep clone of this object */
    GFixedSizePriorityQueueT *clone_() const override = 0;
};

/******************************************************************************/
} /* namespace Gem::Common */
