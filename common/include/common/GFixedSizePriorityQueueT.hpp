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
#include <ranges>
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
         * @brief A fixed-size priority queue holding the best items seen so far.
         *
         * This class implements a fixed-size priority queue. Note that data items
         * are held inside of std::shared_ptr objects and must be copy-constructible.
         * It is also required that T can be compared using operator== and operator!= .
         * A maxSize_ of 0 stands for an unlimited size of the data vector.
         *
         * IMPORTANT: This class assumes that T has a member function clone<T>()
         * which returns a std::shared_ptr<T> as a copy of the T object.
         *
         * @tparam T The type of the work items stored in the queue
         */
template <typename T>
class GFixedSizePriorityQueueT : public GCommonInterfaceT<GFixedSizePriorityQueueT<T>> {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    /***************************************************************************/
    /**
     * @brief Single declaration of this class'es local data members
     * @return A tuple of named member references driving serialize(), load_() and compare_()
     */
    template <typename Self>
    auto localMembers_(this Self &self) {
        return std::make_tuple(
            Gem::Common::make_member("max_size_", self.max_size_),
            Gem::Common::make_member("sort_order_", self.sort_order_),
            Gem::Common::make_cloneable_container_member("data_deq_", self.data_deq_)
        );
    }

    template <typename Archive>
    void serialize(Archive &ar, [[maybe_unused]] const unsigned int version) {
        using boost::serialization::make_nvp;
        // The member list is derived from the single localMembers() declaration
        // so serialize()/load_()/compare_() stay in sync (no silently-dropped member).
        Gem::Common::serialize_members(ar, this->localMembers_());
    }

    ///////////////////////////////////////////////////////////////////////

public:
    /***************************************************************************/
    /**
         * @brief Initialization with the maximum number of entries
         *
         * @param maxSize The maximum size of the queue (0 means unlimited)
         */
    explicit GFixedSizePriorityQueueT(const std::size_t &maxSize)
      : max_size_(maxSize) {
        /* nothing */
    }

    /***************************************************************************/
    /**
         * @brief Initialization with the maximum number of entries and the information,
         * whether higher or lower evaluations are better.
         *
         * @param maxSize The maximum size of the queue (0 means unlimited)
         * @param sortOrder Indicates whether the queue should minimize or maximize
         */
    GFixedSizePriorityQueueT(const std::size_t &maxSize, const sortOrder &sortOrder)
      : max_size_(maxSize)
      , sort_order_(sortOrder) {
        /* nothing */
    }

    /***************************************************************************/
    /**
         * @brief The copy constructor
         *
         * @param cp A constant reference to another object of the same type that is copied
         */
    GFixedSizePriorityQueueT(GFixedSizePriorityQueueT const &cp)
      : max_size_(cp.max_size_)
      , sort_order_(cp.sort_order_) {
        Common::copyCloneableSmartPointerContainer(cp.data_deq_, data_deq_);
    }

    /***************************************************************************/
    /**
         * @brief The move constructor
         *
         * @param cp An rvalue reference to another object of the same type whose content is moved in
         *           (and which is reset to default values afterwards)
         */
    GFixedSizePriorityQueueT(GFixedSizePriorityQueueT &&cp) noexcept
        : max_size_(cp.max_size_), sort_order_(cp.sort_order_), data_deq_(std::move(cp.data_deq_)) {
        // Reset cp to default values
        cp.max_size_ = GFSPQ_DEF_MAX_SIZE;
        cp.sort_order_ = GFSPQ_DEF_SORT_ORDER;
        cp.data_deq_.clear();
    }

    /***************************************************************************/
    // Defaulted functions

    GFixedSizePriorityQueueT() = default;
    ~GFixedSizePriorityQueueT() override = default;

    /***************************************************************************/
    /**
         * @brief Copy assignment operator
         *
         * @param cp A constant reference to another object of the same type that is copied
         * @return A reference to this object
         */
    GFixedSizePriorityQueueT &operator=(GFixedSizePriorityQueueT const &cp) {
        if(this == &cp) {
            return *this;
        }
        max_size_ = cp.max_size_;
        sort_order_ = cp.sort_order_;

        Common::copyCloneableSmartPointerContainer(cp.data_deq_, data_deq_);

        return *this;
    }

    /***************************************************************************/
    /**
         * @brief Move assignment operator
         *
         * @param cp An rvalue reference to another object of the same type whose content is moved in
         *           (and which is reset to default values afterwards)
         * @return A reference to this object
         */
    GFixedSizePriorityQueueT &operator=(GFixedSizePriorityQueueT &&cp) noexcept {
        // Move data over, then set remote object to default values
        max_size_ = cp.max_size_;
        cp.max_size_ = GFSPQ_DEF_MAX_SIZE;

        sort_order_ = cp.sort_order_;
        cp.sort_order_ = GFSPQ_DEF_SORT_ORDER;

        data_deq_ = std::move(cp.data_deq_);
        cp.data_deq_.clear();

        return *this;
    }

    /***************************************************************************/
    /**
         * @brief Gives access to the best item without copying it
         *
         * @return A std::shared_ptr to the best item (the front of the queue)
         * @throw geneva_exception if the queue is empty
         */
    std::shared_ptr<T> best() const {
        if(data_deq_.empty()) {
            // Throw an exception
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GFixedSizePriorityQueueT<T>::best(): Error!" << '\n'
                << "Priority queue is empty." << '\n'
            );
        }
                    return data_deq_.front();
       
    }

    /***************************************************************************/
    /**
         * @brief Gives access to the worst item without copying it
         *
         * @return A std::shared_ptr to the worst item (the back of the queue)
         * @throw geneva_exception if the queue is empty
         */
    std::shared_ptr<T> worst() const {
        if(data_deq_.empty()) {
            // Throw an exception
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GFixedSizePriorityQueueT<T>::best(): Error!" << '\n'
                << "Priority queue is empty." << '\n'
            );
        }
                    return data_deq_.back();
       
    }

    /***************************************************************************/
    /**
         * @brief Allows to set the priority mode. A value of "HIGHERISBETTER" means that higher
         * values are considered better, "LOWERISBETTER" means that lower values are
         * considered to be better.
         *
         * @param sortOrder The new sort order to be used for prioritizing items
         */
    void setSortOrder(const sortOrder &sortOrder) {
        sort_order_ = sortOrder;
    }

    /***************************************************************************/
    /**
         * @brief Allows to retrieve the current value of sort_order_
         *
         * @return The currently configured sort order
         */
    sortOrder getSortOrder() const {
        return sort_order_;
    }

    /***************************************************************************/
    /**
         * @brief Add an item to the queue.
         *
         * Note that the comparator used in this function
         * sorts the data in descending order (assuming that higher
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
        if(0 == max_size_ || data_deq_.size() < max_size_ ||
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
                    // lower is better
                    return this->evaluation(x_ptr) < this->evaluation(y_ptr);
                }
                                    // HIGHERISBETTER
                    return this->evaluation(x_ptr) > this->evaluation(y_ptr);
               
            }
        );

        // Remove surplus work items, if the queue has reached the corresponding size
        // As the worst items are not at the end of the queue, they will be removed, if
        // they are beyond the allowed size. This will only have an effect if maxSize_ is != 0 .
        if(max_size_ && data_deq_.size() > max_size_) {
            data_deq_.resize(max_size_);
        }
    }

    /***************************************************************************/
    /**
         * @brief Adds a range of items to the priority queue.
         *
         * @param begin Iterator pointing to the first item of the range to be added
         * @param end Iterator pointing one past the last item of the range to be added
         * @param do_clone If set to true, work items will be cloned. Otherwise only the smart pointer will be added
         * @param replace If set to true, the queue will be emptied before adding the new work items
         */
    virtual void
    add(typename std::vector<std::shared_ptr<T>>::const_iterator begin,
        typename std::vector<std::shared_ptr<T>>::const_iterator end,
        bool do_clone,
        bool replace) {
        auto worstKnownEvaluation = Gem::Common::getWorstCase<double>(sort_order_);
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
            const auto& item_ptr = *it;

            // Only act on "filled" item_ptr
            if(not(item_ptr)) {
                continue;
            }

            // Add the work item to the queue
            // - If the queue is unlimited
            // - If the queue isn't full yet
            // - If the item is better than the worst one already contained in the queue
            if(0 == max_size_ || data_deq_.size() < max_size_ ||
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
                                    return this->evaluation(x_ptr) > this->evaluation(y_ptr);
               
            }
        );

        // Remove surplus work items, if the queue has reached the corresponding size
        // This will only have an effect if maxSize_ is != 0
        if(max_size_ && data_deq_.size() > max_size_) {
            data_deq_.resize(max_size_);
        }
    }

    /***************************************************************************/
    /**
         * @brief Add a set of items to the queue.
         *
         * Note that the comparator used in this
         * function sorts the data in descending order (assuming that higher
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
             * @brief Removes the best item from the queue and returns it
             *
             * @return A std::shared_ptr to the (now removed) best item
             * @throw geneva_exception if the queue is empty
             */
    std::shared_ptr<T> pop() {
        if(data_deq_.empty()) {
            // Throw an exception
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GFixedSizePriorityQueueT<T>::pop(): Error!" << '\n'
                << "Priority queue is empty." << '\n'
            );
        }
                    auto item_ptr = data_deq_.front();
            data_deq_.pop_front();
            return item_ptr;
       
    }

    /***************************************************************************/
    /**
             * @brief Converts the local deque to a std::vector and returns it
             *
             * @return A std::vector holding the stored items in priority order (best first)
             */
    std::vector<std::shared_ptr<T>> toVector() const {
        return std::ranges::to<std::vector<std::shared_ptr<T>>>(data_deq_);
    }

    /***************************************************************************/
    /**
             * @brief Returns the current size of the queue
             *
             * @return The number of items currently held in the queue
             */
    std::size_t size() const {
        return data_deq_.size();
    }

    /***************************************************************************/
    /**
             * @brief Checks whether the data is empty
             *
             * @return A boolean indicating whether the queue holds no items
             */
    bool empty() const {
        return data_deq_.empty();
    }

    /***************************************************************************/
    /**
             * @brief Allows to clear the queue, removing all stored items
             */
    void clear() {
        data_deq_.clear();
    }

    /***************************************************************************/
    /**
             * @brief Sets the maximum size of the priority queue
             *
             * If the queue currently holds more items than maxSize, surplus
             * (worst) items are dropped.
             *
             * @param maxSize The new maximum number of items the queue may hold (0 means unlimited)
             */
    void setMaxSize(std::size_t maxSize) {
        // Make sure the current size of data_ complies with maxSize
        if(data_deq_.size() > maxSize) {
            data_deq_.resize(maxSize);
        }

        max_size_ = maxSize;
    }

    /***************************************************************************/
    /**
             * @brief Retrieves the maximum size of the priority queue
             *
             * @return The maximum number of items the queue may hold (0 means unlimited)
             */
    std::size_t getMaxSize() const {
        return max_size_;
    }

    /***************************************************************************/
    /**
             * @brief Prints the evaluations of all stored items to std::cout. This is for debugging purposes.
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
             * @brief Loads the data of another GFixedSizePriorityQueueT<T> object
             *
             * @param cp A pointer to another GFixedSizePriorityQueueT<T> object whose data is loaded into this object
             */
    void load_(const GFixedSizePriorityQueueT *cp) override {
        // Check that we are dealing with a GFixedSizePriorityQueueT<T> reference independent of this object and convert the pointer
        const auto *p_load =
            Common::g_convert_and_compare<GFixedSizePriorityQueueT, GFixedSizePriorityQueueT>(
                cp,
                this
            );

        // Load local data
        Gem::Common::g_load_members(this->localMembers_(), p_load->localMembers_());
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
             * @brief Checks for compliance with expectations with respect to another object
             * of the same type
             *
             * @param cp A constant reference to another GFixedSizePriorityQueueT<T> object to compare against
             * @param e The expected outcome of the comparison
             * @param limit The maximum deviation tolerated for floating point comparisons (unused here)
             */
    void compare_(
        const GFixedSizePriorityQueueT &cp,
        const expectation &e,
        [[maybe_unused]] const double & limit
    ) const override {
        using namespace Gem::Common;

        // Check that we are dealing with a GFixedSizePriorityQueueT<T> reference independent of this object and convert the pointer
        const auto *p_load =
            Common::g_convert_and_compare<GFixedSizePriorityQueueT, GFixedSizePriorityQueueT>(
                cp,
                this
            );

        GToken token("GFixedSizePriorityQueueT<T>", e);

        // Compare our parent data ...
        Common::compare_base_t<GCommonInterfaceT<GFixedSizePriorityQueueT>>(*this, *p_load, token);

        // ... and then our local data
        Gem::Common::g_compare_members(this->localMembers_(), p_load->localMembers_(), token);

        // React on deviations from the expectation
        token.evaluate();
    }

    /***************************************************************************/
    /**
             * @brief Checks whether the evaluation of new_item_ptr is better than that of old_item_ptr
             *
             * @param new_item_ptr The candidate item whose evaluation is being judged
             * @param old_item_ptr The reference item to compare against
             * @return A boolean indicating whether new_item_ptr is strictly better than old_item_ptr
             */
    bool
    isBetter(std::shared_ptr<T> const &new_item_ptr, std::shared_ptr<T> const &old_item_ptr) const {
        return this->isBetter(this->evaluation(new_item_ptr), this->evaluation(old_item_ptr));
    }

    /***************************************************************************/
    /**
             * @brief Checks whether the evaluation of new_item_ptr is better than the value old_item_val
             *
             * @param new_item_ptr The candidate item whose evaluation is being judged
             * @param old_item_val The reference evaluation value to compare against
             * @return A boolean indicating whether new_item_ptr is strictly better than old_item_val
             */
    bool isBetter(std::shared_ptr<T> const &new_item_ptr, double old_item_val) const {
        return this->isBetter(this->evaluation(new_item_ptr), old_item_val);
    }

    /***************************************************************************/
    /**
             * @brief Checks whether the value new_item_val is better than the evaluation of old_item_ptr
             *
             * @param new_item_val The candidate evaluation value being judged
             * @param old_item_ptr The reference item to compare against
             * @return A boolean indicating whether new_item_val is strictly better than old_item_ptr
             */
    bool isBetter(double new_item_val, std::shared_ptr<T> const &old_item_ptr) const {
        return this->isBetter(new_item_val, this->evaluation(old_item_ptr));
    }

    /***************************************************************************/
    /**
         * @brief Checks whether value new_item_val is *strictly* better than value old_item_val.
         *
         * Both branches use a strict comparison (`<` / `>`) so the ordering is
         * symmetric: equal values are never reported as "better", regardless of
         * sort direction. The previous LOWERISBETTER branch used `<=`, which
         * treated equality as "better" only for one direction — making
         * incumbent-replacement behave differently for the two sort orders.
         *
         * @param new_item_val The candidate evaluation value being judged
         * @param old_item_val The reference evaluation value to compare against
         * @return A boolean that is true if new_item_val is strictly better than old_item_val
         *         under the current sort order
         */
    bool isBetter(double new_item_val, double old_item_val) const {
        return (sort_order_ == sortOrder::LOWERISBETTER) ? (new_item_val < old_item_val)
                                                          : (new_item_val > old_item_val);
    }

    /***************************************************************************/
    /**
     * @brief Checks whether an item is valid
     *
     * @param item_ptr A std::shared_ptr to the work item to be checked
     * @return A boolean indicating whether the item is valid
     */
    virtual bool isValid(const std::shared_ptr<T> &item_ptr) const = 0;
    /**
     * @brief Evaluates a single work item, so that it can be sorted
     *
     * @param item_ptr A std::shared_ptr to the work item to be evaluated
     * @return The evaluation (priority) value associated with the item
     */
    virtual double evaluation(const std::shared_ptr<T> &item_ptr) const = 0;

    /***************************************************************************/
    /**
             * @brief Applies modifications to this object. This is needed for testing purposes
             *
             * @return A boolean indicating whether the object was modified (always false here)
             */
    bool modify_GUnitTests_() override {
        return false;
    }

    /***************************************************************************/
    /**
             * @brief Performs self tests that are expected to succeed. This is needed for testing purposes
             */
    void specificTestsNoFailureExpected_GUnitTests_() override {
        /* nothing */
    }

    /***************************************************************************/
    /**
             * @brief Performs self tests that are expected to fail. This is needed for testing purposes
             */
    void specificTestsFailuresExpected_GUnitTests_() override {
        /* nothing */
    }

    /***************************************************************************/

    std::size_t max_size_{GFSPQ_DEF_MAX_SIZE}; ///< The maximum number of work-items
    sortOrder sort_order_{sortOrder::LOWERISBETTER};
    ///< Indicates whether higher evaluations of items indicate a higher priority

    std::deque<std::shared_ptr<T>>
        data_deq_{}; ///< Holds the actual data. Empty at the beginning.

private:
    /***************************************************************************/
    /**
         * @brief Uses the storage addresses of individuals to remove duplicates
         *
         * @param items The deque from which duplicate items (sharing the same raw pointer) are removed in place
         */
    void removeDuplicates(std::deque<std::shared_ptr<T>> &items) {
        // Stores addresses we have already encountered
        std::unordered_set<T *> knownAddresses;

        // Iterator for removing elements efficiently
        auto it = items.begin();
        while(it != items.end()) {
            // Check if storage address is already known
            if(T *raw_ptr = it->get(); knownAddresses.contains(raw_ptr)) {
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
             * @brief Returns the name of this class
             *
             * @return The string "GFixedSizePriorityQueueT<T>"
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
