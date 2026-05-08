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

// Standard header files go here
#include <algorithm>
#include <cmath>
#include <sstream>
#include <type_traits>
#include <vector>

// Boost header files go here

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/logic/tribool.hpp>
#include <boost/serialization/assume_abstract.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/tracking.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/vector.hpp>

// Geneva headers go here

#include "common/GCommonHelperFunctionsT.hpp"
#include "common/GErrorStreamer.hpp"
#include "common/GExceptions.hpp"
#include "common/GExpectationChecksT.hpp"
#include "common/GLogger.hpp"

// Forward declaration
class GEqualityPrinter;

namespace Gem::Common {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class implements most important functions of the std::vector
 * class. It is intended to hold basic types or types that can treated
 * like simple types.
 */
template <typename T>
class GPODVectorT {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        ar &BOOST_SERIALIZATION_NVP(data_cnt_);
    }
    ///////////////////////////////////////////////////////////////////////

    friend class GEqualityPrinter;

    static_assert(std::is_trivial<T>::value && std::is_standard_layout<T>::value, "T is no POD");

public:
    /***************************************************************************/
    /**
     * Initialization with a number of items of defined value
     *
     * @param nval The number of items to be added to the collection
     * @param val  The value to be assigned to each position
     */
    GPODVectorT(const std::size_t &nval, const T &val)
      : data_cnt_(nval, val) { /* nothing */
    }

    /***************************************************************************/
    /**
     * The destructor -- purely virtual to make this an abstract base class
     */
    virtual ~GPODVectorT() = 0;

    /***************************************************************************/
    /**
     * The default constructor
     */
    GPODVectorT() = default;
    GPODVectorT(GPODVectorT<T> const &) = default;
    GPODVectorT(GPODVectorT<T> &&) noexcept = default;

    GPODVectorT<T> &operator=(GPODVectorT<T> const &) = default;
    GPODVectorT<T> &operator=(GPODVectorT<T> &&) noexcept = default;

    /***************************************************************************/
    // Deleted comparison operators

    bool operator==(const GPODVectorT<T> &cp) const = delete;
    bool operator!=(const GPODVectorT<T> &cp) const = delete;
    bool operator==(const std::vector<T> &cp_data) const = delete;
    bool operator!=(const std::vector<T> &cp_data) const = delete;

    /***************************************************************************/
    /**
     * Searches for compliance with expectations with respect to another object
     * of the same type
     *
     * @param cp A constant reference to another GStdSimpleVectorInterfaceT object
     * @param e The expected outcome of the comparison
     * @param limit The maximum deviation for floating point values (important for similarity checks)
     */
    virtual void compare_base(
        const GPODVectorT<T> &cp,
        const Gem::Common::expectation &e,
        const double & /*limit*/
    ) const {
        Gem::Common::GToken token("GBaseEA::GEAOptimizationMonitor", e);
        Gem::Common::compare_t(IDENTITY(this->data_cnt_, cp.data_cnt_), token);
        token.evaluate();
    }

    /***************************************************************************/
    // Typedefs
    using value_type = typename std::vector<T>::value_type;
    using reference = typename std::vector<T>::reference;
    using const_reference = typename std::vector<T>::const_reference;

    using iterator = typename std::vector<T>::iterator;
    using const_iterator = typename std::vector<T>::const_iterator;
    using reverse_iterator = typename std::vector<T>::reverse_iterator;
    using const_reverse_iterator = typename std::vector<T>::const_reverse_iterator;

    using size_type = typename std::vector<T>::size_type;
    using difference_type = typename std::vector<T>::difference_type;

    /***************************************************************************/
    // Non modifying access
    size_type size() const {
        return data_cnt_.size();
    } // Used/tested in GDoubleCollection::fillWithData()
    bool empty() const {
        return data_cnt_.empty();
    } // Used/tested in GDoubleCollection::fillWithData()
    size_type max_size() const {
        return data_cnt_.max_size();
    } // Used/tested in GDoubleCollection::specificTestsNoFailureExpected_GUnitTests()

    size_type capacity() const {
        return data_cnt_.capacity();
    } // Used/tested in GDoubleCollection::specificTestsNoFailureExpected_GUnitTests()
    void reserve(size_type amount) {
        data_cnt_.reserve(amount);
    } // Used/tested in GDoubleCollection::specificTestsNoFailureExpected_GUnitTests()

    /***************************************************************************/
    /**
     * Counts the elements whose content is equal to item.
    *
     * @param item The item to be counted in the collection
     * @return The number of items found
     */
    size_type count(const T &item) const {
        return boost::numeric_cast<size_type>(
            std::count(data_cnt_.begin(), data_cnt_.end(), item)
        );
    }

    /* ----------------------------------------------------------------------------
     * Tested in GDoubleCollection::specificTestsNoFailureExpected_GUnitTests()
     * ----------------------------------------------------------------------------
     */

    /***************************************************************************/
    /**
     * Searches for item in the entire range of the vector.
     */
    const_iterator /* decltype(auto) */ find(const T &item) const {
        return std::find(data_cnt_.begin(), data_cnt_.end(), item);
    }

    /* ----------------------------------------------------------------------------
     * Tested in GDoubleCollection::specificTestsNoFailureExpected_GUnitTests()
     * ----------------------------------------------------------------------------
     */

    /***************************************************************************/

    // Modifying functions
    void swap(std::vector<T> &cont) {
        std::swap(data_cnt_, cont);
    } // untested (likely irrelevant)

    // Access to elements (unchecked / checked)
    reference operator[](std::size_t pos) {
        return data_cnt_[pos];
    } // Tested in GDoubleCollection::specificTestsNoFailureExpected_GUnitTests()
    const_reference operator[](std::size_t pos) const {
        return data_cnt_[pos];
    }

    reference at(std::size_t pos) {
        return data_cnt_.at(pos);
    } // Tested in GDoubleCollection::specificTestsNoFailureExpected_GUnitTests()
    const_reference at(std::size_t pos) const {
        return data_cnt_.at(pos);
    }

    reference front() {
        return data_cnt_.front();
    } // Tested in GDoubleCollection::specificTestsNoFailureExpected_GUnitTests()
    const_reference front() const {
        return data_cnt_.front();
    }

    reference back() {
        return data_cnt_.back();
    } // Tested in GDoubleCollection::specificTestsNoFailureExpected_GUnitTests()
    const_reference back() const {
        return data_cnt_.back();
    }

    // Iterators
    iterator begin() {
        return data_cnt_.begin();
    }

    const_iterator begin() const {
        return data_cnt_.begin();
    } // Tested in GDoubleCollection::specificTestsNoFailureExpected_GUnitTests()

    iterator end() {
        return data_cnt_.end();
    } // Tested in GDoubleCollection::specificTestsNoFailureExpected_GUnitTests()
    const_iterator end() const {
        return data_cnt_.end();
    }

    reverse_iterator rbegin() {
        return data_cnt_.rbegin();
    } // untested (likely irrelevant)
    const_reverse_iterator rbegin() const {
        return data_cnt_.rbegin();
    }

    reverse_iterator rend() {
        return data_cnt_.rend();
    } // untested (likely irrelevant)
    const_reverse_iterator rend() const {
        return data_cnt_.rend();
    }

    /***************************************************************************/
    // Insertion and removal

    /**
     * Inserts a given item at position pos. Checks whether the item actually points
     * somewhere.
     */
    iterator insert(iterator pos, const T &item) {
        return data_cnt_.insert(pos, item);
    }

    /* ----------------------------------------------------------------------------
     * Tested in GDoubleCollection::specificTestsNoFailureExpected_GUnitTests()
     * ----------------------------------------------------------------------------
     */

    /***************************************************************************/
    /**
     * Inserts a given amount of items after position pos.
     */
    void insert(iterator pos, size_type amount, const T &item) {
        data_cnt_.insert(pos, amount, item);
    }

    /* ----------------------------------------------------------------------------
     * Tested in GDoubleCollection::specificTestsNoFailureExpected_GUnitTests()
     * ----------------------------------------------------------------------------
     */

    /***************************************************************************/
    // Adding simple items to the  back of the vector
    void push_back(const T &item) {
        data_cnt_.push_back(item);
    } // Used/tested in GDoubleCollection::fillWithData()

    /***************************************************************************/

    // Removal at a given position or in a range
    iterator erase(iterator pos) {
        return data_cnt_.erase(pos);
    } // Tested in GDoubleCollection::specificTestsNoFailureExpected_GUnitTests()
    iterator erase(iterator from, iterator to) {
        return data_cnt_.erase(from, to);
    } // Tested in GDoubleCollection::specificTestsNoFailureExpected_GUnitTests()

    // Removing an element from the end of the vector
    void pop_back() {
        data_cnt_.pop_back();
    } // Tested in GDoubleCollection::specificTestsNoFailureExpected_GUnitTests()

    /***************************************************************************/
    /**
     * Resizing the vector, initialization with item. This function does nothing
     * if amount is the same as data.size(). We assume in this function that
     * T is copy-constructible.
     *
     * @param amount The new desired size of the vector
     * @param item An item that should be used for initialization of new items, if any
     */
    void resize(size_type amount, const T &item) {
        data_cnt_.resize(amount, item);
    } // Tested in GDoubleCollection::specificTestsNoFailureExpected_GUnitTests()

    /***************************************************************************/
    /**
     * Resize the vector without "template" items. This essentially means that
     * T will be default-constructed. For numeric values this will usually mean
     * T(0).
     */
    void resize(size_type amount) {
        data_cnt_.resize(amount);
    }

    /***************************************************************************/
    /** @brief Clearing the data vector */
    void clear() {
        data_cnt_.clear();
    } // Used/tested in GDoubleCollection::fillWithData()

    /***************************************************************************/
    /**
     * Assignment of a std::vector<T>
     *
     * @param cp A constant reference to another std::vector<T>
     * @return The argument of this function (a std::vector<T>)
     */
    GPODVectorT &operator=(const std::vector<T> &cp) {
        data_cnt_ = cp;
        return *this;
    }

    /* ----------------------------------------------------------------------------
     * Tested in GDoubleCollection::specificTestsNoFailureExpected_GUnitTests()
     * ----------------------------------------------------------------------------
     */

    /***************************************************************************/
    /**
     * Creates a copy of the data vector. It is assumed that cp is empty or that
     * all data in it can be deleted.
     *
     * @param cp A reference to a vector that will hold a copy of our local data vector
     */
    void getDataCopy(std::vector<T> &cp) const {
        cp = data_cnt_;
    } // Tested in GDoubleCollection::specificTestsNoFailureExpected_GUnitTests()

    /***************************************************************************/
    /**
     * Performs a cross-over operation at a given position. Note: We do NOT require
     * the two vectors to be of the same size
     *
     * @param cp A copy of another GStdSimpleVectorInterfaceT<T> object
     * @param pos The position as of which the cross-over should be performed
     */
    void crossOver(GPODVectorT<T> &cp, const std::size_t &pos) {
        // Find out the minimum size of both vectors
        std::size_t minSize = (std::min)(this->size(), cp.size());

#ifdef DEBUG
        // Do some error checking
        if(pos >= minSize) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, time_and_place)
                << "In GPODVectorT::crossOver(cp,pos): Error!" << std::endl
                << "Invalid position " << pos << " / " << this->size() << " / " << cp.size()
                << std::endl
            );
        }
#endif /* DEBUG */

        // Swap the elements
        for(std::size_t i = pos; i < minSize; i++) {
            std::swap(this->at(i), cp.at(i));
        }

        // Move the elements of the longer vector over to the other
        // and remove the elements from the other vector
        if(this->size() > cp.size()) {
            // Attach elements to the other vector
            for(std::size_t i = cp.size(); i < this->size(); i++) {
                cp.push_back(this->at(i));
            }

            // Remove the surplus elements from this vector
            this->erase(this->begin() + minSize, this->end());
        }
        else if(cp.size() > this->size()) {
            // Attach elements to the other vector
            for(std::size_t i = this->size(); i < cp.size(); i++) {
                this->push_back(cp.at(i));
            }

            // Remove the surplus elements from this vector
            cp.erase(cp.begin() + minSize, cp.end());
        }

        // Nothing to do if both vectors have the same size
    }

protected:
    /** @brief Applies modifications to this object. This is needed for testing purposes */
    virtual bool modify_GUnitTests_() { /* nothing here yet */
        return false;
    }
    /** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
    virtual void specificTestsNoFailureExpected_GUnitTests_() { /* nothing here yet */
    }
    /** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
    virtual void specificTestsFailuresExpected_GUnitTests_() { /* nothing here yet */
    }

    std::vector<T> data_cnt_;
};

/******************************************************************************/
/**
 * The destructor -- purely virtual to make this an abstract base class
 */
template <typename T>
inline GPODVectorT<T>::~GPODVectorT() {
    data_cnt_.clear();
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Gem::Common */

/******************************************************************************/
/**
 * @brief The content of the BOOST_SERIALIZATION_ASSUME_ABSTRACT(T) macro. Needed for Boost.Serialization
 */
namespace boost::serialization {

template <typename T>
struct is_abstract<Gem::Common::GPODVectorT<T>> : public boost::true_type { /* nothing */
};
template <typename T>
struct is_abstract<const Gem::Common::GPODVectorT<T>> : public boost::true_type { /* nothing */
};

} /* namespace boost::serialization */

/******************************************************************************/
