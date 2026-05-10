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
#include <functional>
#include <sstream>
#include <type_traits>
#include <typeinfo>
#include <vector>

// Boost header files go here

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
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
#include "common/GExpectationChecksT.hpp"
#include "common/GLogger.hpp"
#include "common/GTypeTraitsT.hpp"

namespace Gem::Common {

/******************************************************************************/
/**
 * This class implements the most important functions of the std::vector
 * class. It is intended to hold std::shared_ptr smart pointers. Hence
 * special implementations of some functions are required. Furthermore,
 * using this class prevents us from having to derive directly from a
 * std::vector, which has a non-virtual destructor. Note that we assume here
 * that T holds a complex type, such as a class.  T must implement
 * the interface "usual" for Gemfony optimization library objects, in particular
 * T must implement the clone() function.
 *
 * Some std::vector functions can not be fully implemented, as they require
 * the data in this class to be default-constructible. As this class can hold
 * smart pointers with purely virtual base pointers, this cannot be done. One
 * important example is the resize(std::size_t) function, which would need to
 * add default-constructed T() objects, if the requested size is larger than
 * the current one.
 */
template <
    typename T,
    typename B // B stands for "base type"
    >
class GPtrVectorT {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        ar &BOOST_SERIALIZATION_NVP(data_cnt_);
    }
    ///////////////////////////////////////////////////////////////////////

    static_assert(
        std::is_base_of<B, T>::value && Gem::Common::has_gemfony_common_interface<B>::value,
        "B is no base of T or B has no gemfony_common_interface"
    );

public:
    /***************************************************************************/
    // Defaulted constructors, destructors and assignment operators

    GPtrVectorT() = default;
    GPtrVectorT(GPtrVectorT<T, B> &&) noexcept = default;

    GPtrVectorT<T, B> &operator=(GPtrVectorT<T, B> &&) noexcept = default;

    /***************************************************************************/
    /**
     * The destructor -- purely virtual to make this an abstract base class
     */
    virtual ~GPtrVectorT() = 0;

    /***************************************************************************/
    /**
	 * Copy construction. The content of the smart pointers is cloned (if content is
	 * available).
	 *
	 * @param cp A constant reference to another GStdPtrVectorInterfaceT object
	 */
    GPtrVectorT(GPtrVectorT<T, B> const &cp) {
        Gem::Common::copyCloneableSmartPointerContainer(cp.data_cnt_, data_cnt_);
    }

    /***************************************************************************/
    /**
     * The assignment operator
     */
    GPtrVectorT<T, B> &operator=(GPtrVectorT<T, B> const &cp) {
        if(this == &cp)
            return *this;
        Gem::Common::copyCloneableSmartPointerContainer(cp.data_cnt_, data_cnt_);
        return *this;
    }

    /***************************************************************************/
    // Deleted comparison operators

    bool operator==(GPtrVectorT<T, B> const &) const = delete;
    bool operator!=(GPtrVectorT<T, B> const &) const = delete;
    bool operator==(const std::vector<std::shared_ptr<T>> &) const = delete;
    bool operator!=(const std::vector<std::shared_ptr<T>> &) const = delete;

    /***************************************************************************/
    /**
	 * "Deep" Assignment of a std::vector<std::shared_ptr<T>> . As the vector contains smart
	 * pointers, we cannot just copy the pointers themselves but need to copy their content.
	 *
	 * @param cp A constant reference to another std::vector<std::shared_ptr<T>>
	 * @return A reference to this object
	 */
    GPtrVectorT<T, B> &operator=(std::vector<std::shared_ptr<T>> const &cp) {
        typename std::vector<std::shared_ptr<T>>::const_iterator cp_it;
        typename std::vector<std::shared_ptr<T>>::iterator it;

        std::size_t localSize = data_cnt_.size(); // NOLINT(cppcoreguidelines-init-variables)
        std::size_t cpSize = cp.size();

        if(cpSize == localSize) { // The most likely case
            for(it = data_cnt_.begin(), cp_it = cp.begin(); it != data_cnt_.end();
                ++it, ++cp_it) {
                (*it)->B::load(*cp_it);
            }
        }
        else if(cpSize > localSize) {
            // First copy the initial elements
            for(it = data_cnt_.begin(), cp_it = cp.begin(); it != data_cnt_.end();
                ++it, ++cp_it) {
                (*it)->B::load(*cp_it);
            }

            // Then attach the remaining objects from cp
            for(cp_it = cp.begin() + localSize; cp_it != cp.end(); ++cp_it) {
                data_cnt_.push_back((*cp_it)->T::template clone<T>());
            }
        }
        else if(cpSize < localSize) {
            // First get rid of surplus items
            data_cnt_.resize(cpSize);

            // Then copy the elements
            for(it = data_cnt_.begin(), cp_it = cp.begin(); it != data_cnt_.end();
                ++it, ++cp_it) {
                (*it)->B::load(*cp_it);
            }
        }

        return *this;
    }

    /***************************************************************************/
    /**
	 * Searches for compliance with expectations with respect to another object
	 * of the same type
	 *
	 * @param cp A constant reference to another GStdPtrVectorInterfaceT object
	 * @param e The expected outcome of the comparison
	 * @param limit The maximum deviation for floating point values (important for similarity checks)
	 */
    virtual void compare_base(
        GPtrVectorT<T, B> const &cp,
        Gem::Common::expectation const &e,
        double const & /*limit*/
    ) const {
        Gem::Common::GToken token("GBaseEA::GEAOptimizationMonitor", e);
        Gem::Common::compare_t(IDENTITY(this->data_cnt_, cp.data_cnt_), token);
        token.evaluate();
    }

    /***************************************************************************/
    // Typedefs
    using value_type = typename std::vector<std::shared_ptr<T>>::value_type;
    using reference = typename std::vector<std::shared_ptr<T>>::reference;
    using const_reference = typename std::vector<std::shared_ptr<T>>::const_reference;

    using iterator = typename std::vector<std::shared_ptr<T>>::iterator;
    using const_iterator = typename std::vector<std::shared_ptr<T>>::const_iterator;
    using reverse_iterator = typename std::vector<std::shared_ptr<T>>::reverse_iterator;
    using const_reverse_iterator = typename std::vector<std::shared_ptr<T>>::const_reverse_iterator;

    using size_type = typename std::vector<std::shared_ptr<T>>::size_type;
    using difference_type = typename std::vector<std::shared_ptr<T>>::difference_type;

    /***************************************************************************/
    // Non modifying access
    size_type size() const {
        return data_cnt_.size();
    } // not tested -- trivial mapping
    bool empty() const {
        return data_cnt_.empty();
    } // not tested -- trivial mapping
    size_type max_size() const {
        return data_cnt_.max_size();
    } // not tested -- trivial mapping

    size_type capacity() const {
        return data_cnt_.capacity();
    } // not tested -- trivial mapping
    void reserve(size_type amount) {
        data_cnt_.reserve(amount);
    } // not tested -- trivial mapping

    /***************************************************************************/
    /**
     * Counts the elements whose content is equal to the content of item.
     * Needs to be re-implemented here, as we are dealing with a collection of smart pointers
     * and we do not want to compare the pointers themselves.
     *
     * @param item The item to be counted in the collection
     */
    template <typename item_type>
    size_type count(
        std::shared_ptr<item_type> const &item,
        typename std::enable_if<std::is_base_of<T, item_type>::value>::type *dummy = nullptr
    ) const {
        if(not item) { // Check that item actually contains something useful
            throw geneva_exception(
                g_error_streamer(DO_LOG, time_and_place)
                << "In GParameterTCollectionT<T>::count(item):"
                << "Tried to count an empty smart pointer." << std::endl
            );
        }

        return Gem::Common::narrow_cast<size_type>(std::count_if(
            data_cnt_.begin(),
            data_cnt_.end(),
            [&item](const std::shared_ptr<T> &cont_item) -> bool {
#ifdef DEBUG
                try {
                    return (*item == *(std::dynamic_pointer_cast<item_type>(cont_item)));
                }
                catch(...) {
                    throw geneva_exception(
                        g_error_streamer(DO_LOG, time_and_place)
                        << "Conversion error in GPtrVectorT::count()" << std::endl
                    );
                }
#else
                return (*item == *(std::static_pointer_cast<item_type>(cont_item)));
#endif
            }
        ));
    }

    /* -------------------------------------------------------------------------
	 * Tested in GTestIndividual1::specificTestsNoFailureExpected_GUnitTests()
	 * Throwing tested in GTestIndividual1::specificTestsFailuresExpected_GUnitTests()
	 * -------------------------------------------------------------------------
	 */

    /***************************************************************************/
    /**
	 * Searches for the content of item in the entire range of the vector. Needs to be
	 * re-implemented here, as we are dealing with a collection of smart pointers
	 * and we do not want to compare the pointers themselves.
	 */
    template <typename item_type>
    const_iterator find(
        std::shared_ptr<item_type> const &item,
        typename std::enable_if<std::is_base_of<T, item_type>::value>::type *dummy = nullptr
    ) const {
        if(not item) { // Check that item actually contains something useful
            throw geneva_exception(
                g_error_streamer(DO_LOG, time_and_place)
                << "In GParameterTCollectionT<T>::find(item):"
                << "Tried to find an empty smart pointer." << std::endl
            );
        }

        return std::find_if(
            data_cnt_.begin(),
            data_cnt_.end(),
            [&item](const std::shared_ptr<T> &cont_item) -> bool {
#ifdef DEBUG
                try {
                    return (*item == *(std::dynamic_pointer_cast<item_type>(cont_item)));
                }
                catch(...) {
                    throw geneva_exception(
                        g_error_streamer(DO_LOG, time_and_place)
                        << "Conversion error in GPtrVectorT::find()" << std::endl
                    );
                }
#else
                return (*item == *(std::static_pointer_cast<item_type>(cont_item)));
#endif
            }
        );
    }

    /* ------------------------------------------------------------------------------------------------
	 * Tested in GTestIndividual1::specificTestsNoFailureExpected_GUnitTests()
	 * Throwing tested in GTestIndividual1::specificTestsFailuresExpected_GUnitTests()
	 * ------------------------------------------------------------------------------------------------
	 */

    /***************************************************************************/
    /**
	 * Clones an object at a given position and convert it to a given target type
	 */
    template <typename target_type>
    std::shared_ptr<target_type> clone_at(std::size_t pos) const {
        return (data_cnt_.at(pos))->T::template clone<target_type>();
    }

    /***************************************************************************/
    // Modifying functions

    // Exchange of two data sets
    void swap(std::vector<std::shared_ptr<T>> &cont) {
        data_cnt_.swap(cont);
    } // not tested -- trivial mapping

    // Access to elements (unchecked / checked)
    reference operator[](std::size_t pos) {
        return data_cnt_[pos];
    } // not tested -- trivial mapping
    const_reference operator[](std::size_t pos) const {
        return data_cnt_[pos];
    } // not tested -- trivial mapping

    reference at(std::size_t pos) {
        return data_cnt_.at(pos);
    } // not tested -- trivial mapping
    const_reference at(std::size_t pos) const {
        return data_cnt_.at(pos);
    } // not tested -- trivial mapping

    reference front() {
        return data_cnt_.front();
    } // not tested -- trivial mapping
    const_reference front() const {
        return data_cnt_.front();
    } // not tested -- trivial mapping

    reference back() {
        return data_cnt_.back();
    } // not tested -- trivial mapping
    const_reference back() const {
        return data_cnt_.back();
    } // not tested -- trivial mapping

    // Iterators
    iterator begin() {
        return data_cnt_.begin();
    } // not tested -- trivial mapping
    const_iterator begin() const {
        return data_cnt_.begin();
    } // not tested -- trivial mapping

    iterator end() {
        return data_cnt_.end();
    } // not tested -- trivial mapping
    const_iterator end() const {
        return data_cnt_.end();
    } // not tested -- trivial mapping

    reverse_iterator rbegin() {
        return data_cnt_.rbegin();
    } // not tested -- trivial mapping
    const_reverse_iterator rbegin() const {
        return data_cnt_.rbegin();
    } // not tested -- trivial mapping

    reverse_iterator rend() {
        return data_cnt_.rend();
    } // not tested -- trivial mapping
    const_reverse_iterator rend() const {
        return data_cnt_.rend();
    } // not tested -- trivial mapping

    /***************************************************************************/
    // Insertion and removal

    /***************************************************************************/
    /**
	 * Inserts a given item at position pos. Behavior defaults
	 * to insert_noclone(pos,item).
	 *
	 * @param pos The position where the item should be inserted
	 * @param item_ptr The item to be inserted into the collection
	 */
    iterator insert(iterator pos, std::shared_ptr<T> item_ptr) {
        return this->insert_noclone(pos, item_ptr);
    }

    /* ------------------------------------------------------------------------------------------------
	 * Tested via insert_noclone
	 * ------------------------------------------------------------------------------------------------
	 */

    /***************************************************************************/
    /**
	 * Inserts a given item at position pos. Checks whether the item actually points
	 * somewhere. Note that the shared_ptr will be inserted itself. Hence any change you
	 * might make to the object pointed to will also affect the item in the collection.
	 *
	 * @param pos The position where the item should be inserted
	 * @param item_ptr The item to be inserted into the collection
	 */
    iterator insert_noclone(iterator pos, std::shared_ptr<T> item_ptr) {
        if(not item_ptr) { // Check that item actually contains something useful
            throw geneva_exception(
                g_error_streamer(DO_LOG, time_and_place)
                << "In GParameterTCollectionT<T>::insert_noclone(pos, item_ptr):"
                << "Tried to insert an empty smart pointer." << std::endl
            );
        }

        return data_cnt_.insert(pos, item_ptr);
    }

    /* ------------------------------------------------------------------------------------------------
	 * Throwing tested in GTestIndividual1::specificTestsFailuresExpected_GUnitTests()
	 * Tested in GTestIndividual1::specificTestsNoFailureExpected_GUnitTests()
	 * ------------------------------------------------------------------------------------------------
	 */

    /***************************************************************************/
    /**
	 * Inserts a given item at position pos. Checks whether the item actually points
	 * somewhere. This function clones the item, hence changes to the argument after
	 * invocation of this function will not affect the item pointed to.
	 *
	 * @param pos The position where the item should be inserted
	 * @param item_ptr The item to be inserted into the collection
	 */
    iterator insert_clone(iterator pos, std::shared_ptr<T> const &item_ptr) {
        if(not item_ptr) { // Check that item actually contains something useful
            throw geneva_exception(
                g_error_streamer(DO_LOG, time_and_place)
                << "In GParameterTCollectionT<T>::insert_clone(pos, item_ptr):"
                << "Tried to insert an empty smart pointer." << std::endl
            );
        }

        return data_cnt_.insert(pos, item_ptr->T::template clone<T>());
    }

    /* ------------------------------------------------------------------------------------------------
	 * Throwing tested in GTestIndividual1::specificTestsFailuresExpected_GUnitTests()
	 * Tested in GTestIndividual1::specificTestsNoFailureExpected_GUnitTests()
	 * ------------------------------------------------------------------------------------------------
	 */

    /***************************************************************************/
    /**
	 * Inserts a given amount of items at position pos. Defaults to
	 * insert_clone(pos, amount, item_ptr)
	 *
	 * @param pos The position where items should be inserted
	 * @param amount The amount of items to be inserted
	 * @param item_ptr The item to be inserted into the collection
	 */
    void insert(iterator pos, size_type amount, std::shared_ptr<T> const &item_ptr) {
        this->insert_clone(pos, amount, item_ptr);
    }

    /* ------------------------------------------------------------------------------------------------
	 * Tested via insert_clone
	 * ------------------------------------------------------------------------------------------------
	 */

    /***************************************************************************/
    /**
	 * Inserts a given amount of items at position pos. Will always clone.
	 *
	 * @param pos The position where items should be inserted
	 * @param amount The amount of items to be inserted
	 * @param item_ptr The item to be inserted into the collection
	 */
    void insert_clone(iterator pos, size_type amount, std::shared_ptr<T> const &item_ptr) {
        if(not item_ptr) { // Check that item actually contains something useful
            throw geneva_exception(
                g_error_streamer(DO_LOG, time_and_place)
                << "In GParameterTCollectionT<T>::insert_clone(pos, amount, item):" << std::endl
                << "Tried to insert an empty smart pointer." << std::endl
            );
        }

        std::size_t iterator_pos =
            pos - data_cnt_.begin(); // NOLINT(cppcoreguidelines-init-variables)
        for(std::size_t i = 0; i < amount; i++) {
            // Note that we re-calculate the iterator, as it is not clear whether it remains valid
            data_cnt_.insert(data_cnt_.begin() + iterator_pos, item_ptr->T::template clone<T>());
        }
    }

    /* ------------------------------------------------------------------------------------------------
	 * Throwing tested in GTestIndividual1::specificTestsFailuresExpected_GUnitTests()
	 * Tested in GTestIndividual1::specificTestsNoFailureExpected_GUnitTests()
	 * ------------------------------------------------------------------------------------------------
	 */

    /***************************************************************************/
    /**
	 * Inserts a given amount of items as of position pos. item_ptr will be cloned
	 * and added to the collection itself. Note that changes made to item_ptr's object
	 * after a call to this function will also affect the container.
	 *
	 * @param pos The position where items should be inserted
	 * @param amount The amount of items to be inserted
	 * @param item_ptr The item to be inserted into the collection
	 */
    void insert_noclone(iterator pos, size_type amount, std::shared_ptr<T> item_ptr) {
        if(not item_ptr) { // Check that item actually contains something useful
            throw geneva_exception(
                g_error_streamer(DO_LOG, time_and_place)
                << "In GParameterTCollectionT<T>::insert_noclone(pos, amount, item):" << std::endl
                << "Tried to insert an empty smart pointer." << std::endl
            );
        }

        std::size_t iterator_pos =
            pos - data_cnt_.begin(); // NOLINT(cppcoreguidelines-init-variables)
        // Create (amount-1) clones
        for(std::size_t i = 0; i < amount - 1; i++) {
            // Note that we re-calculate the iterator, as it is not clear whether it remains valid
            data_cnt_.insert(data_cnt_.begin() + iterator_pos, item_ptr->T::template clone<T>());
        }
        // Add the argument
        data_cnt_.insert(data_cnt_.begin() + iterator_pos, item_ptr);
    }

    /* ------------------------------------------------------------------------------------------------
	 * Throwing tested in GTestIndividual1::specificTestsFailuresExpected_GUnitTests()
	 * Tested in GTestIndividual1::specificTestsNoFailureExpected_GUnitTests()
	 * ------------------------------------------------------------------------------------------------
	 */

    /***************************************************************************/
    /**
	 * Ads a shared_ptr object to the  back of the vector. The function defaults to
	 * push_back_noclone
	 *
	 * @param item_ptr The item to be appended to the collection
	 */
    void push_back(std::shared_ptr<T> item_ptr) {
        this->push_back_noclone(item_ptr);
    }

    /* ------------------------------------------------------------------------------------------------
	 * Tested via push_back_noclone
	 * ------------------------------------------------------------------------------------------------
	 */

    /***************************************************************************/
    /**
	 * Ads a shared_ptr object to the  back of the vector. Note that the shared_ptr
	 * will inserted itself. Hence any Change you might make to the object pointed
	 * to will also affect the item in the collection.
	 *
	 * @param item_ptr The item to be appended to the collection
	 */
    void push_back_noclone(std::shared_ptr<T> item_ptr) {
        if(not item_ptr) { // Check that item actually contains something useful
            throw geneva_exception(
                g_error_streamer(DO_LOG, time_and_place)
                << "In GParameterTCollectionT<T>::push_back(item):" << std::endl
                << "Tried to insert an empty smart pointer." << std::endl
            );
        }

        data_cnt_.push_back(item_ptr);
    }

    /* ------------------------------------------------------------------------------------------------
	 * Throwing tested in GTestIndividual1::specificTestsFailuresExpected_GUnitTests()
	 * Tested in GTestIndividual1::specificTestsNoFailureExpected_GUnitTests()
	 * ------------------------------------------------------------------------------------------------
	 */

    /***************************************************************************/
    /**
	 * Ads a shared_ptr object to the  back of the vector. The object pointed to
	 * will be cloned. Hence changes to it after a call to this function will not
	 * affect the item stored in the collection.
	 *
	 * @param item_ptr The item to be appended to the collection
	 */
    void push_back_clone(std::shared_ptr<T> const &item_ptr) {
        if(not item_ptr) { // Check that item actually contains something useful
            throw geneva_exception(
                g_error_streamer(DO_LOG, time_and_place)
                << "In GStdPtrVectorInterface<T>::push_back_clone(item):" << std::endl
                << "Tried to insert an empty smart pointer." << std::endl
            );
        }

        data_cnt_.push_back(item_ptr->T::template clone<T>());
    }

    /* ------------------------------------------------------------------------------------------------
	 * Throwing tested in GTestIndividual1::specificTestsFailuresExpected_GUnitTests()
	 * Tested in GTestIndividual1::specificTestsNoFailureExpected_GUnitTests()
	 * ------------------------------------------------------------------------------------------------
	 */

    /***************************************************************************/
    // Removal at a given position or in a range
    iterator erase(iterator pos) {
        return data_cnt_.erase(pos);
    } // not tested -- trivial mapping
    iterator erase(iterator from, iterator to) {
        return data_cnt_.erase(from, to);
    } // not tested -- trivial mapping

    // Removing an element from the end of the vector
    void pop_back() {
        data_cnt_.pop_back();
    } // not tested -- trivial mapping

    /***************************************************************************/
    /**
	 * Resizing the vector. This function will clone the first item in the collection, if available.
	 */
    void resize(size_type amount) {
        if(this->empty() && amount != 0) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, time_and_place)
                << "In GStdPtrVectorInterface<T>::resize(size_type):" << std::endl
                << "Tried to increase the size even though the vector is empty." << std::endl
                << "Use a resize-version that allows you to specify the objects" << std::endl
                << "to be added." << std::endl
            );
        }

        this->resize_clone(amount, this->at(0));
    }

    /* ------------------------------------------------------------------------------------------------
	 * Tested in GTestIndividual1::specificTestsNoFailureExpected_GUnitTests()
	 * Throwing tested in GTestIndividual1::specificTestsFailuresExpected_GUnitTests()
	 * ------------------------------------------------------------------------------------------------
	 */

    /***************************************************************************/
    /**
	 * Resizing the vector, initialization with item. This function is a front end
	 * to resize_clone()
	 *
	 * @param amount The new desired size of the vector
	 * @param item An item that should be used for initialization of new items, if any
	 */
    void resize(size_type amount, std::shared_ptr<T> item_ptr) {
        resize_clone(amount, item_ptr);
    }

    /* ------------------------------------------------------------------------------------------------
	 * Tested via resize_clone(amount, item)
	 * ------------------------------------------------------------------------------------------------
	 */

    /***************************************************************************/
    /**
	 * Resizing the vector, initialization with item. This function does nothing
	 * if amount is the same as data.size(). Note that item_ptr will become part
	 * of the collection. Hence changes to the object pointed to will also affect
	 * the collection. If amount would increase the collection size by more than one,
	 * additional added items will need to be cloned nonetheless.
	 *
	 * @param amount The new desired size of the vector
	 * @param item An item that should be used for initialization of new items, if any
	 */
    void resize_noclone(size_type amount, std::shared_ptr<T> item_ptr) {
        std::size_t dataSize = data_cnt_.size(); // NOLINT(cppcoreguidelines-init-variables)

        if(amount < dataSize)
            data_cnt_.resize(amount);
        else if(amount > dataSize) {
            // Check that item is not empty
            if(not item_ptr) { // Check that item actually contains something useful
                throw geneva_exception(
                    g_error_streamer(DO_LOG, time_and_place)
                    << "In GParameterTCollectionT<T>::resize(amount, item):" << std::endl
                    << "Tried to insert an empty smart pointer." << std::endl
                );
            }

            // Create a (amount - dataSize -1) clones
            for(std::size_t i = dataSize; i < amount - 1; i++) {
                data_cnt_.push_back(item_ptr->T::template clone<T>());
            }

            // Finally add item_ptr
            data_cnt_.push_back(item_ptr);
        }
    }

    /* ------------------------------------------------------------------------------------------------
	 * Tested in GTestIndividual1::specificTestsNoFailureExpected_GUnitTests()
	 * Throwing tested in GTestIndividual1::specificTestsFailuresExpected_GUnitTests()
	 * ------------------------------------------------------------------------------------------------
	 */

    /***************************************************************************/
    /**
	 * Resizing the vector, initialization with item. This function does nothing
	 * if amount is the same as data.size(). item_ptr will be cloned. Hence
	 * changes to the object pointed to will not affect the collection.
	 *
	 * @param amount The new desired size of the vector
	 * @param item An item that should be used for initialization of new items, if any
	 */
    void resize_clone(size_type amount, std::shared_ptr<T> item_ptr) {
        std::size_t dataSize = data_cnt_.size(); // NOLINT(cppcoreguidelines-init-variables)

        if(amount < dataSize)
            data_cnt_.resize(amount);
        else if(amount > dataSize) {
            // Check that item is not empty
            if(not item_ptr) { // Check that item actually contains something useful
                throw geneva_exception(
                    g_error_streamer(DO_LOG, time_and_place)
                    << "In GParameterTCollectionT<T>::resize(amount, item):" << std::endl
                    << "Tried to insert an empty smart pointer." << std::endl
                );
            }

            for(std::size_t i = dataSize; i < amount; i++) {
                data_cnt_.push_back(item_ptr->T::template clone<T>());
            }
        }
    }

    /* ------------------------------------------------------------------------------------------------
	 * Tested in GTestIndividual1::specificTestsNoFailureExpected_GUnitTests()
	 * Throwing tested in GTestIndividual1::specificTestsFailuresExpected_GUnitTests()
	 * ------------------------------------------------------------------------------------------------
	 */

    /***************************************************************************/
    /**
	 * Fills the collection with empty smart pointers. This is meant for situations
	 * where we want to first resize the collection to a given size and then assign
	 * data items to each position.
	 */
    void resize_empty(size_type amount) {
        std::size_t dataSize = data_cnt_.size(); // NOLINT(cppcoreguidelines-init-variables)
        if(amount < dataSize) {
            data_cnt_.resize(amount);
        }
        else if(amount > dataSize) { // Add empty smart pointers
            for(std::size_t i = dataSize; i < amount; i++) {
                data_cnt_.push_back(std::shared_ptr<T>());
            }
        }
    }

    /***************************************************************************/
    /** @brief Clearing the data vector */
    void clear() {
        data_cnt_.clear();
    } // Not tested -- trivial mapping

    /***************************************************************************/
    /**
	 * Creates a copy of the data vector. It is assumed that cp is empty or that
	 * all data in it can be deleted.
	 *
	 * @param cp A reference to a vector that will hold a copy of our local data vector
	 */
    void getDataCopy(std::vector<std::shared_ptr<T>> &cp) const {
        cp.clear();
        for(const auto &item : data_cnt_) {
            cp.push_back(item->T::template clone<T>());
        }
    }

    /***************************************************************************/
    /**
	 * Performs a cross-over operation at a given position. Note: We do not require
	 * the two vectors to be of the same size
	 *
	 * @param cp A copy of another GStdPtrVectorInterfaceT<T, B> object
	 * @param pos The position as of which the cross-over should be performed
	 */
    void crossOver(GPtrVectorT<T, B> &cp, const std::size_t &pos) {
        // Find out the minimum size of both vectors
        std::size_t minSize = (std::min)(this->size(), cp.size());

#ifdef DEBUG
        // Do some error checking
        if(pos >= minSize) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, time_and_place)
                << "In GPtrVectorT::crossOver(cp,pos): Error!" << std::endl
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

    /***************************************************************************/
    /**
	 * Returns a view on the vector's content, filtering out only items of specific
	 * type.
	 *
	 * @param target A vector to which pointers with the derived type are attached
	 */
    template <typename derivedType>
    void attachViewTo(std::vector<std::shared_ptr<derivedType>> &target) {
        for(auto &item_ptr : data_cnt_) {
            std::shared_ptr<derivedType> derived_item_ptr =
                std::dynamic_pointer_cast<derivedType>(item_ptr);
            if(derived_item_ptr) {
                target.push_back(derived_item_ptr);
            }
        }
    }

    /***************************************************************************/
    /////////////////////////////////////////////////////////////////////////////
    /***************************************************************************/
    /** An iterator that filters and converts elements to a derived shared_ptr type */
    template <typename derivedType>
    class conversion_iterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type        = std::shared_ptr<derivedType>;
        using difference_type   = std::ptrdiff_t;
        using pointer           = std::shared_ptr<derivedType>;
        using reference         = std::shared_ptr<derivedType>;

        explicit conversion_iterator(typename std::vector<std::shared_ptr<T>>::iterator const &end)
          : end_(end) {}

        conversion_iterator() = delete;

        conversion_iterator<derivedType> &
        operator=(typename std::vector<std::shared_ptr<T>>::iterator const &current) {
            current_pos_ = current;
            while(current_pos_ != end_ &&
                  not(valid_ptr_ = std::dynamic_pointer_cast<derivedType>(*current_pos_))) {
                ++current_pos_;
            }
            return *this;
        }

        bool operator!=(typename std::vector<std::shared_ptr<T>>::iterator const &other) const {
            return current_pos_ != other;
        }

        std::shared_ptr<derivedType> operator*() const {
#ifdef DEBUG
            if(current_pos_ == end_) {
                throw geneva_exception(
                    g_error_streamer(DO_LOG, time_and_place)
                    << "In conversion_iterator::operator*(): Error:" << std::endl
                    << "current position at end of sequence" << std::endl
                );
            }
            if(!valid_ptr_) {
                throw geneva_exception(
                    g_error_streamer(DO_LOG, time_and_place)
                    << "In conversion_iterator::operator*(): Error: empty pointer" << std::endl
                );
            }
#endif /* DEBUG */
            return valid_ptr_;
        }

        conversion_iterator<derivedType> &operator++() {
            while(current_pos_ != end_) {
                ++current_pos_;
                if(current_pos_ != end_ &&
                   (valid_ptr_ = std::dynamic_pointer_cast<derivedType>(*current_pos_)))
                    break;
            }
            return *this;
        }

        void resetEndPosition(typename std::vector<std::shared_ptr<T>>::iterator const &end) {
            end_ = end;
        }

    private:
        typename std::vector<std::shared_ptr<T>>::iterator current_pos_;
        typename std::vector<std::shared_ptr<T>>::iterator end_;
        std::shared_ptr<derivedType>                       valid_ptr_;
    };

protected:
    /** @brief Applies modifications to this object. This is needed for testing purposes */
    // Note to self: changes to GStdPtrVectorInterface should be minimal and not involve objects pointed to
    virtual bool modify_GUnitTests_() { /* nothing here yet */
        return false;
    }
    /** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
    virtual void specificTestsNoFailureExpected_GUnitTests_() { /* nothing here yet */
    }
    /** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
    virtual void specificTestsFailuresExpected_GUnitTests_() { /* nothing here yet */
    }

    std::vector<std::shared_ptr<T>> data_cnt_;
};

/******************************************************************************/
/**
 * The destructor -- purely virtual to make this an abstract base class
 */
template <typename T, typename B>
inline GPtrVectorT<T, B>::~GPtrVectorT() {
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
template <typename T, typename B>
struct is_abstract<Gem::Common::GPtrVectorT<T, B>> : public boost::true_type { /* nothing */
};
template <typename T, typename B>
struct is_abstract<const Gem::Common::GPtrVectorT<T, B>> : public boost::true_type { /* nothing */
};
} /* namespace boost::serialization */

/******************************************************************************/
