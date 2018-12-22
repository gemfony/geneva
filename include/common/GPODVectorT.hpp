/**
 * @file
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

#pragma once

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard header files go here
#include <sstream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <type_traits>

// Boost header files go here

#include <boost/lexical_cast.hpp>
#include <boost/logic/tribool.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/assume_abstract.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/tracking.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/export.hpp>

// Geneva headers go here

#include "common/GExceptions.hpp"
#include "common/GCommonHelperFunctionsT.hpp"
#include "common/GLogger.hpp"
#include "common/GErrorStreamer.hpp"
#include "common/GExpectationChecksT.hpp"

// Forward declaration
class GEqualityPrinter;

namespace Gem {
namespace Common {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class implements most important functions of the std::vector
 * class. It is intended to hold basic types or types that can treated
 * like simple types.
 */
template<typename T>
class GPODVectorT
{
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template<typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

#if BOOST_VERSION <= 105800
        // Some preparation needed if this is a load operation.
		// This is needed to work around a problem in Boost 1.58
		if (Archive::is_loading::value) {
			data.clear();
		}
#endif

        ar & BOOST_SERIALIZATION_NVP(m_data_cnt);
    }
    ///////////////////////////////////////////////////////////////////////

    friend class GEqualityPrinter;

    static_assert(
        std::is_trivial<T>::value && std::is_standard_layout<T>::value
        , "T is no POD"
    );

public:
    /***************************************************************************/
    /**
     * Initialization with a number of items of defined value
     *
     * @param nval The number of items to be added to the collection
     * @param val  The value to be assigned to each position
     */
    GPODVectorT(const std::size_t &nval, const T &val)
        :m_data_cnt(nval, val)
    { /* nothing */ }

    /***************************************************************************/
    /**
     * The destructor -- purely virtual to make this an abstract base class
     */
    virtual ~GPODVectorT() BASE = 0;

    /***************************************************************************/
    /**
     * The default constructor
     */
    GPODVectorT() = default;
    GPODVectorT(GPODVectorT<T> const&) = default;
    GPODVectorT(GPODVectorT<T> &&) noexcept = default;

    GPODVectorT<T>& operator=(GPODVectorT<T> const&) = default;
    GPODVectorT<T>& operator=(GPODVectorT<T> &&) noexcept = default;

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
        const GPODVectorT<T> &cp, const Gem::Common::expectation &e, const double &limit
    ) const BASE {
        Gem::Common::GToken token(
            "GBaseEA::GEAOptimizationMonitor"
            , e
        );
        Gem::Common::compare_t(
            IDENTITY(this->m_data_cnt, cp.m_data_cnt)
            , token
        );
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
    size_type size() const { return m_data_cnt.size(); } // Used/tested in GDoubleCollection::fillWithData()
    bool empty() const { return m_data_cnt.empty(); } // Used/tested in GDoubleCollection::fillWithData()
    size_type max_size() const { return m_data_cnt.max_size(); } // Used/tested in GDoubleCollection::specificTestsNoFailureExpected_GUnitTests()

    size_type capacity() const { return m_data_cnt.capacity(); } // Used/tested in GDoubleCollection::specificTestsNoFailureExpected_GUnitTests()
    void reserve(size_type amount) {
        m_data_cnt.reserve(amount);
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
            std::count(
                m_data_cnt.begin()
                , m_data_cnt.end()
                , item
            ));
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
        return std::find(
            m_data_cnt.begin()
            , m_data_cnt.end()
            , item
        );
    }

    /* ----------------------------------------------------------------------------
     * Tested in GDoubleCollection::specificTestsNoFailureExpected_GUnitTests()
     * ----------------------------------------------------------------------------
     */

    /***************************************************************************/

    // Modifying functions
    void swap(std::vector<T> &cont) {
        std::swap(
            m_data_cnt
            , cont
        );
    } // untested (likely irrelevant)

    // Access to elements (unchecked / checked)
    reference operator[](
        std::size_t pos
    ) { return m_data_cnt[pos]; } // Tested in GDoubleCollection::specificTestsNoFailureExpected_GUnitTests()
    const_reference operator[](std::size_t pos) const { return m_data_cnt[pos]; }

    reference at(std::size_t pos) {
        return m_data_cnt.at(pos);
    } // Tested in GDoubleCollection::specificTestsNoFailureExpected_GUnitTests()
    const_reference at(std::size_t pos) const { return m_data_cnt.at(pos); }

    reference front() { return m_data_cnt.front(); } // Tested in GDoubleCollection::specificTestsNoFailureExpected_GUnitTests()
    const_reference front() const { return m_data_cnt.front(); }

    reference back() { return m_data_cnt.back(); } // Tested in GDoubleCollection::specificTestsNoFailureExpected_GUnitTests()
    const_reference back() const { return m_data_cnt.back(); }

    // Iterators
    iterator begin() { return m_data_cnt.begin(); }

    const_iterator begin() const { return m_data_cnt.begin(); } // Tested in GDoubleCollection::specificTestsNoFailureExpected_GUnitTests()

    iterator end() { return m_data_cnt.end(); } // Tested in GDoubleCollection::specificTestsNoFailureExpected_GUnitTests()
    const_iterator end() const { return m_data_cnt.end(); }

    reverse_iterator rbegin() { return m_data_cnt.rbegin(); } // untested (likely irrelevant)
    const_reverse_iterator rbegin() const { return m_data_cnt.rbegin(); }

    reverse_iterator rend() { return m_data_cnt.rend(); } // untested (likely irrelevant)
    const_reverse_iterator rend() const { return m_data_cnt.rend(); }

    /***************************************************************************/
    // Insertion and removal

    /**
     * Inserts a given item at position pos. Checks whether the item actually points
     * somewhere.
     */
    iterator insert(iterator pos, const T &item) {
        return m_data_cnt.insert(
            pos
            , item
        );
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
        m_data_cnt.insert(
            pos
            , amount
            , item
        );
    }

    /* ----------------------------------------------------------------------------
     * Tested in GDoubleCollection::specificTestsNoFailureExpected_GUnitTests()
     * ----------------------------------------------------------------------------
     */

    /***************************************************************************/
    // Adding simple items to the  back of the vector
    void push_back(const T &item) { m_data_cnt.push_back(item); } // Used/tested in GDoubleCollection::fillWithData()

    /***************************************************************************/

    // Removal at a given position or in a range
    iterator erase(iterator pos) {
        return m_data_cnt.erase(pos);
    } // Tested in GDoubleCollection::specificTestsNoFailureExpected_GUnitTests()
    iterator erase(iterator from, iterator to) {
        return m_data_cnt.erase(
            from
            , to
        );
    } // Tested in GDoubleCollection::specificTestsNoFailureExpected_GUnitTests()

    // Removing an element from the end of the vector
    void pop_back() { m_data_cnt.pop_back(); } // Tested in GDoubleCollection::specificTestsNoFailureExpected_GUnitTests()

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
        m_data_cnt.resize(
            amount
            , item
        );
    } // Tested in GDoubleCollection::specificTestsNoFailureExpected_GUnitTests()

    /***************************************************************************/
    /**
     * Resize the vector without "template" items. This essentially means that
     * T will be default-constructed. For numeric values this will usually mean
     * T(0).
     */
    void resize(size_type amount) { m_data_cnt.resize(amount); }

    /***************************************************************************/
    /** @brief Clearing the data vector */
    void clear() { m_data_cnt.clear(); } // Used/tested in GDoubleCollection::fillWithData()

    /***************************************************************************/
    /**
     * Assignment of a std::vector<T>
     *
     * @param cp A constant reference to another std::vector<T>
     * @return The argument of this function (a std::vector<T>)
     */
    GPODVectorT& operator=(const std::vector<T> &cp) {
        m_data_cnt = cp;
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
    void getDataCopy(
        std::vector<T> &cp
    ) const { cp = m_data_cnt; }  // Tested in GDoubleCollection::specificTestsNoFailureExpected_GUnitTests()

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
        std::size_t minSize = (std::min)(
            this->size()
            , cp.size());

#ifdef DEBUG
        // Do some error checking
        if (pos >= minSize) {
            throw gemfony_exception(
                g_error_streamer(
                    DO_LOG
                    , time_and_place
                )
                    << "In GPODVectorT::crossOver(cp,pos): Error!" << std::endl
                    << "Invalid position " << pos << " / " << this->size() << " / " << cp.size() << std::endl
            );
        }
#endif /* DEBUG */

        // Swap the elements
        for (std::size_t i = pos; i < minSize; i++) {
            std::swap(
                this->at(i)
                , cp.at(i));
        }

        // Move the elements of the longer vector over to the other
        // and remove the elements from the other vector
        if (this->size() > cp.size()) {
            // Attach elements to the other vector
            for (std::size_t i = cp.size(); i < this->size(); i++) {
                cp.push_back(this->at(i));
            }

            // Remove the surplus elements from this vector
            this->erase(
                this->begin() + minSize
                , this->end());
        } else if (cp.size() > this->size()) {
            // Attach elements to the other vector
            for (std::size_t i = this->size(); i < cp.size(); i++) {
                this->push_back(cp.at(i));
            }

            // Remove the surplus elements from this vector
            cp.erase(
                cp.begin() + minSize
                , cp.end());
        }


        // Nothing to do if both vectors have the same size
    }

protected:
    std::vector<T> m_data_cnt;

public:
    /***************************************************************************/
    /** @brief Applies modifications to this object. This is needed for testing purposes */
    virtual bool modify_GUnitTests() BASE { /* nothing here yet */ return false; }

    /** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
    virtual void specificTestsNoFailureExpected_GUnitTests() BASE { /* nothing here yet */ }

    /** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
    virtual void specificTestsFailuresExpected_GUnitTests() BASE { /* nothing here yet */  }
    /***************************************************************************/
};

/******************************************************************************/
/**
 * The destructor -- purely virtual to make this an abstract base class
 */
template<typename T>
inline GPODVectorT<T>::~GPODVectorT() {
    m_data_cnt.clear();
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Common */
} /* namespace Gem */

/******************************************************************************/
/**
 * @brief The content of the BOOST_SERIALIZATION_ASSUME_ABSTRACT(T) macro. Needed for Boost.Serialization
 */
namespace boost {
namespace serialization {

template<typename T>
struct is_abstract<Gem::Common::GPODVectorT<T>> :
    public boost::true_type
{ /* nothing */ };
template<typename T>
struct is_abstract<const Gem::Common::GPODVectorT<T>> :
    public boost::true_type
{ /* nothing */ };

} /* namespace serialization */
} /* namespace boost */

/******************************************************************************/
