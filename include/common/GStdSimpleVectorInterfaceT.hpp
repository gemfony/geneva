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
 *
 * TODO: Enable operator== etc.
 */
template<typename T>
class GStdSimpleVectorInterfaceT
{
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template<typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        ar & BOOST_SERIALIZATION_NVP(data);
    }
    ///////////////////////////////////////////////////////////////////////

    friend class GEqualityPrinter;

public:
    /***************************************************************************/
    /**
     * The default constructor
     */
    GStdSimpleVectorInterfaceT() { /* nothing */ }

    /***************************************************************************/
    /**
     * Initialization with a number of items of defined value
     *
     * @param nval The number of items to be added to the collection
     * @param val  The value to be assigned to each position
     */
    GStdSimpleVectorInterfaceT(const std::size_t &nval, const T &val)
        :
        data(
            nval
            , val
        ) { /* nothing */ }

    /***************************************************************************/
    /**
     * Copy construction
     *
     * @param cp A constant reference to another GStdSimpleVectorInterfaceT object
     */
    GStdSimpleVectorInterfaceT(const GStdSimpleVectorInterfaceT<T> &cp)
        :
        data(cp.data) { /* nothing */ }

    /***************************************************************************/
    /**
     * The destructor.
     */
    virtual ~GStdSimpleVectorInterfaceT() { data.clear(); }

    /***************************************************************************/
    /**
     * Assginment operator
     */
    const GStdSimpleVectorInterfaceT &operator=(const GStdSimpleVectorInterfaceT<T> &cp) {
        this->operator=(cp.data);
        return cp;
    }

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
        const GStdSimpleVectorInterfaceT<T> &cp, const Gem::Common::expectation &e, const double &limit
    ) const BASE {
        Gem::Common::GToken token(
            "GBaseEA::GEAOptimizationMonitor"
            , e
        );
        Gem::Common::compare_t(
            IDENTITY(this->data
                     , cp.data)
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
    size_type size() const { return data.size(); } // Used/tested in GDoubleCollection::fillWithData()
    bool empty() const { return data.empty(); } // Used/tested in GDoubleCollection::fillWithData()
    size_type max_size() const { return data.max_size(); } // Used/tested in GDoubleCollection::specificTestsNoFailureExpected_GUnitTests()

    size_type capacity() const { return data.capacity(); } // Used/tested in GDoubleCollection::specificTestsNoFailureExpected_GUnitTests()
    void reserve(size_type amount) {
        data.reserve(amount);
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
                data.begin()
                , data.end()
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
            data.begin()
            , data.end()
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
            data
            , cont
        );
    } // untested (likely irrelevant)

    // Access to elements (unchecked / checked)
    reference operator[](
        std::size_t pos
    ) { return data[pos]; } // Tested in GDoubleCollection::specificTestsNoFailureExpected_GUnitTests()
    const_reference operator[](std::size_t pos) const { return data[pos]; }

    reference at(std::size_t pos) {
        return data.at(pos);
    } // Tested in GDoubleCollection::specificTestsNoFailureExpected_GUnitTests()
    const_reference at(std::size_t pos) const { return data.at(pos); }

    reference front() { return data.front(); } // Tested in GDoubleCollection::specificTestsNoFailureExpected_GUnitTests()
    const_reference front() const { return data.front(); }

    reference back() { return data.back(); } // Tested in GDoubleCollection::specificTestsNoFailureExpected_GUnitTests()
    const_reference back() const { return data.back(); }

    // Iterators
    iterator begin() { return data.begin(); }

    const_iterator begin() const { return data.begin(); } // Tested in GDoubleCollection::specificTestsNoFailureExpected_GUnitTests()

    iterator end() { return data.end(); } // Tested in GDoubleCollection::specificTestsNoFailureExpected_GUnitTests()
    const_iterator end() const { return data.end(); }

    reverse_iterator rbegin() { return data.rbegin(); } // untested (likely irrelevant)
    const_reverse_iterator rbegin() const { return data.rbegin(); }

    reverse_iterator rend() { return data.rend(); } // untested (likely irrelevant)
    const_reverse_iterator rend() const { return data.rend(); }

    /***************************************************************************/
    // Insertion and removal

    /**
     * Inserts a given item at position pos. Checks whether the item actually points
     * somewhere.
     */
    iterator insert(iterator pos, const T &item) {
        return data.insert(
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
        data.insert(
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
    void push_back(const T &item) { data.push_back(item); } // Used/tested in GDoubleCollection::fillWithData()

    /***************************************************************************/

    // Removal at a given position or in a range
    iterator erase(iterator pos) {
        return data.erase(pos);
    } // Tested in GDoubleCollection::specificTestsNoFailureExpected_GUnitTests()
    iterator erase(iterator from, iterator to) {
        return data.erase(
            from
            , to
        );
    } // Tested in GDoubleCollection::specificTestsNoFailureExpected_GUnitTests()

    // Removing an element from the end of the vector
    void pop_back() { data.pop_back(); } // Tested in GDoubleCollection::specificTestsNoFailureExpected_GUnitTests()

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
        data.resize(
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
    void resize(size_type amount) { data.resize(amount); }

    /***************************************************************************/
    /** @brief Clearing the data vector */
    void clear() { data.clear(); } // Used/tested in GDoubleCollection::fillWithData()

    /***************************************************************************/
    /**
     * Assignment of a std::vector<T>
     *
     * @param cp A constant reference to another std::vector<T>
     * @return The argument of this function (a std::vector<T>)
     */
    const std::vector<T> &operator=(const std::vector<T> &cp) {
        data = cp;
        return cp;
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
    ) const { cp = data; }  // Tested in GDoubleCollection::specificTestsNoFailureExpected_GUnitTests()

    /***************************************************************************/
    /**
     * Performs a cross-over operation at a given position. Note: We do NOT require
     * the two vectors to be of the same size
     *
     * @param cp A copy of another GStdSimpleVectorInterfaceT<T> object
     * @param pos The position as of which the cross-over should be performed
     */
    void crossOver(GStdSimpleVectorInterfaceT<T> &cp, const std::size_t &pos) {
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
                    << "In GStdSimpleVectorInterfaceT::crossOver(cp,pos): Error!" << std::endl
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
    std::vector<T> data;

    /** @brief Intentionally make this object purely virtual, for performance reasons */
    virtual void dummyFunction() = 0;

private:
    /** @brief Checks for equality with another GStdSimpleVectorInterfaceT<T> object. Intentionally left undefined */
    bool operator==(const GStdSimpleVectorInterfaceT<T> &cp) const = delete;

    /** @brief Checks inequality with another GStdSimpleVectorInterfaceT<T> object. Intentionally left undefined */
    bool operator!=(const GStdSimpleVectorInterfaceT<T> &cp) const = delete;

    /** @brief Checks for equality with a std::vector<T> object. Intentionally left undefined */
    bool operator==(const std::vector<T> &cp_data) const = delete;

    /** @brief Checks for inequality with a std::vector<T> object. Intentionally left undefined */
    bool operator!=(const std::vector<T> &cp_data) const = delete;

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
struct is_abstract<Gem::Common::GStdSimpleVectorInterfaceT<T>> :
    public boost::true_type
{
};
template<typename T>
struct is_abstract<const Gem::Common::GStdSimpleVectorInterfaceT<T>> :
    public boost::true_type
{
};
} /* namespace serialization */
} /* namespace boost */

/******************************************************************************/
