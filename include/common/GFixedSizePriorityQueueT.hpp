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

// Boost headers go here
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/deque.hpp>

// Geneva headers go here
#include "common/GCommonHelperFunctionsT.hpp"
#include "common/GExceptions.hpp"
#include <common/GExpectationChecksT.hpp>
#include "common/GLogger.hpp"
#include "common/GErrorStreamer.hpp"
#include "common/GCommonMathHelperFunctionsT.hpp"
#include "common/GCommonInterfaceT.hpp"

namespace Gem::Common
{
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
    class GFixedSizePriorityQueueT
        : public GCommonInterfaceT<GFixedSizePriorityQueueT<T>>
    {
        ///////////////////////////////////////////////////////////////////////
        friend class boost::serialization::access;

        template <typename Archive>
        void serialize(Archive& ar, const unsigned int)
        {
            using boost::serialization::make_nvp;

            ar
                & BOOST_SERIALIZATION_NVP(m_maxSize_)
                & BOOST_SERIALIZATION_NVP(m_sortOrder_)
                & BOOST_SERIALIZATION_NVP(m_data_deq_);
        }

        ///////////////////////////////////////////////////////////////////////

    public:
        /***************************************************************************/
        /**
         * Initialization with the maximum number of entries
         *
         * @param maxSize The maximum size of the queue
         */
        explicit GFixedSizePriorityQueueT(const std::size_t& maxSize)
            : m_maxSize_(maxSize)
        {
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
        GFixedSizePriorityQueueT(
            const std::size_t& maxSize,
            const sortOrder& sortOrder
        )
            : m_maxSize_(maxSize),
              m_sortOrder_(sortOrder)
        {
            /* nothing */
        }

        /***************************************************************************/
        /**
         * The copy constructor
         */
        GFixedSizePriorityQueueT(GFixedSizePriorityQueueT const& cp)
            : m_maxSize_(cp.m_maxSize_),
              m_sortOrder_(cp.m_sortOrder_)
        {
            Common::copyCloneableSmartPointerContainer(cp.m_data_deq_, m_data_deq_);
        }

        /***************************************************************************/
        /**
         * The move constructor
         */
        GFixedSizePriorityQueueT(GFixedSizePriorityQueueT&& cp) noexcept
        {
            // Move content, then reset cp to default values
            m_maxSize_ = cp.m_maxSize_;
            cp.m_maxSize_ = GFSPQ_DEF_MAX_SIZE;

            m_sortOrder_ = cp.m_sortOrder_;
            cp.m_sortOrder_ = GFSPQ_DEF_SORT_ORDER;

            m_data_deq_ = std::move(cp.m_data_deq_);
            cp.m_data_deq_.clear();
        }

        /***************************************************************************/
        // Defaulted functions

        GFixedSizePriorityQueueT() = default;
        virtual ~GFixedSizePriorityQueueT() = default;

        /***************************************************************************/
        /**
         * Assignment operator
         */
        GFixedSizePriorityQueueT& operator=(GFixedSizePriorityQueueT const& cp)
        {
            m_maxSize_ = cp.m_maxSize_;
            m_sortOrder_ = cp.m_sortOrder_;

            Common::copyCloneableSmartPointerContainer(cp.m_data_deq_, m_data_deq_);

            return *this;
        }

        /***************************************************************************/
        /**
         * Assignment operator
         */
        GFixedSizePriorityQueueT& operator=(GFixedSizePriorityQueueT&& cp) noexcept
        {
            // Move data over, then set remote object to default values
            m_maxSize_ = cp.m_maxSize;
            cp.m_maxSize_ = GFSPQ_DEF_MAX_SIZE;

            m_sortOrder_ = cp.m_sortOrder;
            cp.m_sortOrder_ = GFSPQ_DEF_SORT_ORDER;

            m_data_deq_ = std::move(cp.m_data_deq_);
            cp.m_data_deq_.clear();

            return *this;
        }

        /***************************************************************************/
        /**
         * Gives access to the best item without copying it
         */
        std::shared_ptr<T> best() const
        {
            if (m_data_deq_.empty())
            {
                // Throw an exception
                throw gemfony_exception(
                    g_error_streamer(DO_LOG, time_and_place)
                    << "In GFixedSizePriorityQueueT<T>::best(): Error!" << std::endl
                    << "Priority queue is empty." << std::endl
                );
            }
            else
            {
                return m_data_deq_.front();
            }
        }

        /***************************************************************************/
        /**
         * Gives access to the worst item without copying it
         */
        std::shared_ptr<T> worst() const
        {
            if (m_data_deq_.empty())
            {
                // Throw an exception
                throw gemfony_exception(
                    g_error_streamer(DO_LOG, time_and_place)
                    << "In GFixedSizePriorityQueueT<T>::best(): Error!" << std::endl
                    << "Priority queue is empty." << std::endl
                );
            }
            else
            {
                return m_data_deq_.back();
            }
        }

        /***************************************************************************/
        /**
         * Allows to set the priority mode. A value of "HIGHERISBETTER" means that higher
         * values are considered better, "false" means that lower values are
         * considered to be better.
         */
        void setSortOrder(const sortOrder& sortOrder)
        {
            m_sortOrder_ = sortOrder;
        }

        /***************************************************************************/
        /**
         * Allows to retrieve the current value of m_sortOrder
         */
        sortOrder getSortOrder() const
        {
            return m_sortOrder_;
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
        virtual void add(
            std::shared_ptr<T> const& item_ptr
            , const bool do_clone
        ) BASE {
            // Add the work item to the queue
            // - If the queue is unlimited
            // - If the queue isn't full yet
            // - If the item is better than the worst one contained in the queue
            if (0 == m_maxSize_ || m_data_deq_.size() < m_maxSize_ ||
                isBetter(this->evaluation(item_ptr), this->evaluation(this->worst())))
            {
                if (do_clone)
                {
                    m_data_deq_.push_back(item_ptr->template clone<T>());
                }
                else
                {
                    m_data_deq_.push_back(item_ptr);
                }
            }

            // Sort the data according to their ids, so we may remove duplicates
            std::sort(
                m_data_deq_.begin()
                , m_data_deq_.end()
                , [this](std::shared_ptr<T> const& x_ptr, std::shared_ptr<T> const& y_ptr) -> bool
                {
                    return (this->id(x_ptr) < this->id(y_ptr));
                }
            );

            // Remove duplicate items
            m_data_deq_.erase(
                std::unique(
                    m_data_deq_.begin()
                    , m_data_deq_.end()
                    , [this](std::shared_ptr<T> const& x_ptr, std::shared_ptr<T> const& y_ptr) -> bool
                    {
                        return (this->id(x_ptr) == this->id(y_ptr));
                    }
                ), m_data_deq_.end()
            );

            // Sort the data according to the evaluation
            std::sort(
                m_data_deq_.begin()
                , m_data_deq_.end()
                , [this](std::shared_ptr<T> const& x_ptr, std::shared_ptr<T> const& y_ptr) -> bool
                {
                    if (this->getSortOrder() == sortOrder::LOWERISBETTER)
                    {
                        // higher is better
                        return this->evaluation(x_ptr) < this->evaluation(y_ptr);
                    }
                    else
                    {
                        // HIGHERISBETTER
                        return this->evaluation(x_ptr) > this->evaluation(y_ptr);
                    }
                }
            );

            // Remove surplus work items, if the queue has reached the corresponding size
            // As the worst items are not at the end of the queue, they will be removed, if
            // they are beyond the allowed size. This will only have an effect if m_maxSize is != 0 .
            if (m_maxSize_ && m_data_deq_.size() > m_maxSize_)
            {
                m_data_deq_.resize(m_maxSize_);
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
        virtual void add(
            std::vector<std::shared_ptr<T>> const& item_ptr_vec
            , bool do_clone
            , bool replace
        ) BASE {
            std::cout <<"GFixedSizePriorityQueueT::add(vec) called" << std::endl;

            double worstKnownEvaluation = Gem::Common::getWorstCase<double>(m_sortOrder_);
            if (replace || m_data_deq_.empty())
            {
                m_data_deq_.clear();
            }
            else
            {
                // Data already exists, we know better than the worst known valid
                worstKnownEvaluation = this->evaluation(this->worst());
            }

            // At this point, worstKnownEvaluation will be
            // - the worst case, if the queue is empty or all entries in the queue will be replaced
            // - the evaluation of the worst entry in the queue if we only add items (regardless of whether they will be cloned or not)
            for (auto const& item_ptr : item_ptr_vec)
            {
                // Only act on "filled" item_ptr
                if (not item_ptr) continue;

                // Add the work item to the queue
                // - If the queue is unlimited
                // - If the queue isn't full yet
                // - If the item is better than the worst one already contained in the queue
                if (0 == m_maxSize_ || m_data_deq_.size() < m_maxSize_ || isBetter(
                    this->evaluation(item_ptr), worstKnownEvaluation))
                {
                    if (do_clone)
                    {
                        m_data_deq_.push_back(item_ptr->template clone<T>());
                    }
                    else
                    {
                        m_data_deq_.push_back(item_ptr);
                    }
                }
            }

            // Sort the data according to their ids, so we may remove duplicates
            std::sort(
                m_data_deq_.begin()
                , m_data_deq_.end()
                , [this](std::shared_ptr<T> const& x_ptr, std::shared_ptr<T> const& y_ptr) -> bool
                {
                    return (this->id(x_ptr) < this->id(y_ptr));
                }
            );

            // Remove duplicate items
            m_data_deq_.erase(
                std::unique(
                    m_data_deq_.begin()
                    , m_data_deq_.end()
                    , [this](std::shared_ptr<T> const& x_ptr, std::shared_ptr<T> const& y_ptr) -> bool
                    {
                        return (this->id(x_ptr) == this->id(y_ptr));
                    }
                ), m_data_deq_.end()
            );

            std::sort(
                m_data_deq_.begin()
                , m_data_deq_.end()
                , [this](std::shared_ptr<T> const& x_ptr, std::shared_ptr<T> const& y_ptr) -> bool
                {
                    if (this->getSortOrder() == sortOrder::LOWERISBETTER)
                    {
                        return this->evaluation(x_ptr) < this->evaluation(y_ptr);
                    }
                    else
                    {
                        return this->evaluation(x_ptr) > this->evaluation(y_ptr);
                    }
                }
            );

            // Remove surplus work items, if the queue has reached the corresponding size
            // This will only have an effect if m_maxSize is != 0
            if (m_maxSize_ && m_data_deq_.size() > m_maxSize_)
            {
                m_data_deq_.resize(m_maxSize_);
            }
        }

        /***************************************************************************/
        /**
             * Removes the best item from the queue and returns it
             */
        std::shared_ptr<T> pop()
        {
            if (m_data_deq_.empty())
            {
                // Throw an exception
                throw gemfony_exception(
                    g_error_streamer(DO_LOG, time_and_place)
                    << "In GFixedSizePriorityQueueT<T>::pop(): Error!" << std::endl
                    << "Priority queue is empty." << std::endl
                );
            }
            else
            {
                auto item_ptr = m_data_deq_.front();
                m_data_deq_.pop_front();
                return item_ptr;
            }
        }

        /***************************************************************************/
        /**
             * Converts the local deque to a std::vector and returns it
             */
        std::vector<std::shared_ptr<T>> toVector() const
        {
            std::vector<std::shared_ptr<T>> result;

            for (auto const& item_ptr : m_data_deq_)
            {
                result.push_back(item_ptr);
            }

            return result;
        }

        /***************************************************************************/
        /**
             * Returns the current size of the queue
             */
        std::size_t size() const
        {
            return m_data_deq_.size();
        }

        /***************************************************************************/
        /**
             * Checks whether the data is empty
             */
        bool empty() const
        {
            return m_data_deq_.empty();
        }

        /***************************************************************************/
        /**
             * Allows to clear the queue
             */
        void clear()
        {
            m_data_deq_.clear();
        }

        /***************************************************************************/
        /**
             * Sets the maximum size of the priority queue
             */
        void setMaxSize(std::size_t maxSize)
        {
            // Make sure the current size of m_data complies with maxSize
            if (m_data_deq_.size() > maxSize)
            {
                m_data_deq_.resize(maxSize);
            }

            m_maxSize_ = maxSize;
        }

        /***************************************************************************/
        /**
             * Retrieves the maximum size of the priority queue
             */
        std::size_t getMaxSize() const
        {
            return m_maxSize_;
        }

        /***************************************************************************/
        /**
             * Prints the evaluations. This is for debugging purposes.
             */
        void printEvaluations() const
        {
            std::cout << "==================== printEvaluations =====================" << std::endl;
            for (auto const& item_ptr : m_data_deq_)
            {
                std::cout << this->evaluation(item_ptr) << std::endl;
            }
        }

    protected:
        /***************************************************************************/
        /**
             * Loads the data of another GFixedSizePriorityQueue<T> object
             */
        void load_(const GFixedSizePriorityQueueT* cp) override
        {
            // Check that we are dealing with a GFixedSizePriorityQueueT<T> reference independent of this object and convert the pointer
            const GFixedSizePriorityQueueT* p_load
                = Common::g_convert_and_compare<GFixedSizePriorityQueueT, GFixedSizePriorityQueueT>(
                    cp, this);

            // Load local data
            m_maxSize_ = p_load->m_maxSize_;
            m_sortOrder_ = p_load->m_sortOrder_;
            Common::copyCloneableSmartPointerContainer(p_load->m_data_deq_, m_data_deq_);
        }

        /***************************************************************************/
        /** @brief Allow access to this classes compare_ function */
        friend void Common::compare_base_t<GFixedSizePriorityQueueT>(
            GFixedSizePriorityQueueT const&
            , GFixedSizePriorityQueueT const&
            , GToken&
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
            const GFixedSizePriorityQueueT& cp
            , const expectation& e
            , const double& limit
        ) const override
        {
            using namespace Gem::Common;

            // Check that we are dealing with a GFixedSizePriorityQueueT<T> reference independent of this object and convert the pointer
            const GFixedSizePriorityQueueT* p_load
                = Common::g_convert_and_compare<GFixedSizePriorityQueueT, GFixedSizePriorityQueueT>(
                    cp, this);

            GToken token("GFixedSizePriorityQueueT<T>", e);

            // Compare our parent data ...
            Common::compare_base_t<GCommonInterfaceT<GFixedSizePriorityQueueT>>(*this, *p_load, token);

            // ... and then our local data
            compare_t(IDENTITY(m_maxSize_, p_load->m_maxSize_), token);
            compare_t(IDENTITY(m_sortOrder_, p_load->m_sortOrder_), token);
            compare_t(IDENTITY(m_data_deq_, p_load->m_data_deq_), token);

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
        ) const
        {
            return this->isBetter(this->evaluation(new_item_ptr), this->evaluation(old_item_ptr));
        }

        /***************************************************************************/
        /**
             * Checks whether value new_item is better than value old_item
             */
        bool isBetter(
            std::shared_ptr<T> const& new_item_ptr
            , double old_item_val
        ) const
        {
            return this->isBetter(this->evaluation(new_item_ptr), old_item_val);
        }

        /***************************************************************************/
        /**
             * Checks whether value new_item is better than value old_item
             */
        bool isBetter(
            double new_item_val
            , std::shared_ptr<T> const& old_item_ptr
        ) const
        {
            return this->isBetter(new_item_val, this->evaluation(old_item_ptr));
        }

        /***************************************************************************/
        /**
         * Checks whether value new_item is better than value old_item
         */
        bool isBetter(
            double new_item_val
            , double old_item_val
        ) const
        {
            return
                (m_sortOrder_ == sortOrder::LOWERISBETTER)
                    ? (new_item_val <= old_item_val)
                    : (new_item_val > old_item_val);
        }

        /***************************************************************************/
        /** @brief Checks whether an Item is valid */
        virtual G_API_COMMON bool isValid(const std::shared_ptr<T>&) const BASE = 0;
        /** @brief Evaluates a single work item, so that it can be sorted */
        virtual G_API_COMMON double evaluation(const std::shared_ptr<T>&) const BASE = 0;
        /** @brief Returns a unique id for a work item */
        virtual G_API_COMMON std::string id(const std::shared_ptr<T>&) const BASE = 0;

        /***************************************************************************/
        /**
             * Applies modifications to this object. This is needed for testing purposes
             */
        bool modify_GUnitTests_() override
        {
            return false;
        }

        /***************************************************************************/
        /**
             * Performs self tests that are expected to succeed. This is needed for testing purposes
             */
        void specificTestsNoFailureExpected_GUnitTests_() override
        {
            /* nothing */
        }

        /***************************************************************************/
        /**
             * Performs self tests that are expected to fail. This is needed for testing purposes
             */
        void specificTestsFailuresExpected_GUnitTests_() override
        {
            /* nothing */
        }

        /***************************************************************************/

        std::size_t m_maxSize_{GFSPQ_DEF_MAX_SIZE}; ///< The maximum number of work-items
        sortOrder m_sortOrder_{sortOrder::LOWERISBETTER};
        ///< Indicates whether higher evaluations of items indicate a higher priority

        std::deque<std::shared_ptr<T>> m_data_deq_{}; ///< Holds the actual data. Empty at the beginning.

    private:
        /***************************************************************************/
        /**
         * Makes sure the class is sorted and has the correct maximum size.
         */
        void rectify()
        {
            if (this->empty()) return; // Nothing to do
            else
            {
#ifdef DEBUG
                // Rectifying should never happen for an empty queue
                if (0==this->size())
                {
                    throw gemfony_exception(
                        g_error_streamer(DO_LOG,  time_and_place)
                            << "In GFixedSizePriorityQueueT<T>::rectify():" << std::endl
                            << "Rectification should not happen on an empty queue." << std::endl
                    );
                }

                // Make sure we only have valid entries
                for (auto const& item_ptr : m_data_deq_)
                {
                    if (not isValid(item_ptr))
                    {
                        throw gemfony_exception(
                            g_error_streamer(DO_LOG,  time_and_place)
                                << "In GFixedSizePriorityQueueT<T>::rectify():" << std::endl
                                << "Got invalid work item" << std::endl
                        );
                    }
                }
#endif
                // We now assume that we only have valid work items and that the queue
                // has at least one entry. In the next step we sort the queue according
                // to our sorting policy. We will then make sure that it does not exceed
                // the maximum allowed size.

            }
        }

        /***************************************************************************/
        /**
             * Returns the name of this class
             */
        std::string name_() const override
        {
            return std::string("GFixedSizePriorityQueueT<T>");
        }

        /***************************************************************************/
        /** @brief Creates a deep clone of this object */
        G_API_COMMON GFixedSizePriorityQueueT* clone_() const override = 0;
    };

    /******************************************************************************/
}
