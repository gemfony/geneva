/**
 * @file GStdPtrVectorInterfaceT.hpp
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

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard header files go here
#include <sstream>
#include <vector>
#include <typeinfo>
#include <functional>
#include <type_traits>

// Boost header files go here

#include <boost/lexical_cast.hpp>
#include <boost/logic/tribool.hpp>
#include <boost/optional.hpp>
#include <boost/iterator/iterator_facade.hpp>

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

#ifndef GSTDPTRVECTORINTERFACET_HPP_
#define GSTDPTRVECTORINTERFACET_HPP_

// Geneva headers go here
#include "common/GCommonHelperFunctionsT.hpp"
#include "common/GLogger.hpp"
#include "common/GExpectationChecksT.hpp"

namespace Gem {
namespace Common {

/******************************************************************************/
/**
 * This class implements the most important functions of the std::vector
 * class. It is intended to hold std::shared_ptr smart pointers. Hence
 * special implementations of some functions are required. Furthermore,
 * using this class prevents us from having to derive directly from a
 * std::vector, which has a non-virtual destructor. Note that we assume here
 * that T holds a complex type, such as a class.  T must implement
 * the interface "usual" for Gemfony optimization library objects, in particular
 * T must implement the clone() and function.
 *
 * Some std::vector functions can not be fully implemented, as they require
 * the data in this class to be default-constructible. As this class can hold
 * smart pointers with purely virtual base pointers, this cannot be done. One
 * important example is the resize(std::size_t) function, which would need to
 * add default-constructed T() objects, if the requested size is larger than
 * the current one.
 */
template<typename T, typename B> // B stands for "base type"
class GStdPtrVectorInterfaceT {

	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive &ar, const unsigned int) {
		using boost::serialization::make_nvp;

		// Some preparation needed if this is a load operation.
		// This is needed to work around a problem in Boost 1.58
		// TODO: Check if this error still exists with Boost 1.59
		if (Archive::is_loading::value) {
			data.clear();
		}

		ar
		&BOOST_SERIALIZATION_NVP(data);
	}
	///////////////////////////////////////////////////////////////////////

	// Make sure B is a base class of T
	static_assert(std::is_base_of<B, T>::value, "B should be a base of T");

public:
	/***************************************************************************/
	/**
	 * The default constructor
	 */
	GStdPtrVectorInterfaceT() { /* nothing */ }

	/***************************************************************************/
	/**
	 * Copy construction. The content of the smart pointers is cloned (if content is
	 * available).
	 *
	 * @param cp A constant reference to another GStdPtrVectorInterfaceT object
	 */
	GStdPtrVectorInterfaceT(const GStdPtrVectorInterfaceT<T, B> &cp) {
		typename std::vector<std::shared_ptr < T>> ::const_iterator
		cp_it;
		for (cp_it = cp.data.begin(); cp_it != cp.data.end(); ++cp_it) {
			data.push_back((*cp_it)->T::template clone<T>());
		}
	}

	/***************************************************************************/
	/**
	 * The destructor. Destruction of the objects will be taken care of
	 * by std::shared_ptr<T>.
	 */
	virtual ~GStdPtrVectorInterfaceT() {
		data.clear();
	}

	/***************************************************************************/
	/**
	 * Assginment operator
	 *
	 * @param cp A copy of another GStdPtrVectorInterfaceT<T, B> object
	 * @return The argument of this function
	 */
	const GStdPtrVectorInterfaceT<T, B> &operator=(const GStdPtrVectorInterfaceT<T, B> &cp) {
		this->operator=(cp.data);
		return cp;
	}

	/***************************************************************************/
	/**
	 * Assignment of a std::vector<std::shared_ptr<T>> . As the vector contains smart
	 * pointers, we cannot just copy the pointers themselves but need to copy their content.
	 *
	 * @param cp A constant reference to another std::vector<std::shared_ptr<T>>
	 * @return The argument of this function
	 */
	const std::vector<std::shared_ptr < T>>& operator=(const std::vector<std::shared_ptr <T>>& cp) {
		typename std::vector<std::shared_ptr <T>>::const_iterator cp_it;
		typename std::vector<std::shared_ptr <T>>::iterator it;

		std::size_t localSize = data.size();
		std::size_t cpSize = cp.size();

		if (cpSize == localSize) { // The most likely case
			for (it = data.begin(), cp_it = cp.begin(); it != data.end(); ++it, ++cp_it) {
				(*it)->B::load(*cp_it);
			}
		}
		else if (cpSize > localSize) {
			// First copy the initial elements
			for (it = data.begin(), cp_it = cp.begin(); it != data.end(); ++it, ++cp_it) {
				(*it)->B::load(*cp_it);
			}

			// Then attach the remaining objects from cp
			for (cp_it = cp.begin() + localSize; cp_it != cp.end(); ++cp_it) {
				data.push_back((*cp_it)->T::template clone<T>());
			}
		}
		else if (cpSize < localSize) {
			// First get rid of surplus items
			data.resize(cpSize);

			// Then copy the elements
			for (it = data.begin(), cp_it = cp.begin(); it != data.end(); ++it, ++cp_it) {
				(*it)->B::load(*cp_it);
			}
		}

		return cp;
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
		const GStdPtrVectorInterfaceT<T, B> &cp
		, const Gem::Common::expectation &e
		, const double &limit
	) const BASE {
		Gem::Common::GToken token("GBaseEA::GEAOptimizationMonitor", e);
		Gem::Common::compare_t(IDENTITY(this->data, cp.data), token);
		token.evaluate();
	}

	/***************************************************************************/
	// Typedefs
	using value_type = typename std::vector<std::shared_ptr <T>>::value_type;
	using reference = typename std::vector<std::shared_ptr <T>>::reference;
	using const_reference = typename std::vector<std::shared_ptr <T>>::const_reference;

	using iterator = typename std::vector<std::shared_ptr <T>>::iterator;
	using const_iterator = typename std::vector<std::shared_ptr <T>>::const_iterator;
	using reverse_iterator = typename std::vector<std::shared_ptr <T>>::reverse_iterator;
	using const_reverse_iterator = typename std::vector<std::shared_ptr <T>>::const_reverse_iterator;

	using size_type = typename std::vector<std::shared_ptr <T>>::size_type;
	using difference_type = typename std::vector<std::shared_ptr <T>>::difference_type;

	/***************************************************************************/
	// Non modifying access
	size_type size() const { return data.size(); } // not tested -- trivial mapping
	bool empty() const { return data.empty(); } // not tested -- trivial mapping
	size_type max_size() const { return data.max_size(); } // not tested -- trivial mapping

	size_type capacity() const { return data.capacity(); } // not tested -- trivial mapping
	void reserve(size_type amount) { data.reserve(amount); } // not tested -- trivial mapping

	/***************************************************************************/
	/**
	 * Counts the elements whose content is equal to the content of item.
	 * Needs to be re-implemented here, as we are dealing with a collection of smart pointers
	 * and we do not want to compare the pointers themselves.
	 *
	 * @param item The item to be counted in the collection
	 */
	template<typename item_type>
	size_type count(
		const std::shared_ptr <item_type> &item,
		typename std::enable_if<std::is_base_of<T, item_type>::value>::type *dummy = nullptr
	) const {
		if (!item) { // Check that item actually contains something useful
			throw gemfony_exception(
				g_error_streamer(DO_LOG, time_and_place)
					<< "In GParameterTCollectionT<T>::count(item):"
					<< "Tried to count an empty smart pointer." << std::endl
			);
		}

		return boost::numeric_cast<size_type>(std::count_if(
			data.begin(), data.end(), [&item](const std::shared_ptr <T> &cont_item) -> bool {
				bool result = false;
#ifdef DEBUG
            try {
               result = (*item == *(std::dynamic_pointer_cast<item_type>(cont_item)));
            }
            catch(...) {
					throw gemfony_exception(
						g_error_streamer(DO_LOG, time_and_place)
							<< "Conversion error in GStdPtrVectorInterfaceT::count()" << std::endl
					);
            }
#else
				result = (*item == *(std::static_pointer_cast<item_type>(cont_item)));
#endif
				return result;
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
	template<typename item_type>
	const_iterator find(
		const std::shared_ptr <item_type> &item,
		typename std::enable_if<std::is_base_of<T, item_type>::value>::type *dummy = nullptr
	) const {
		if (!item) { // Check that item actually contains something useful
			throw gemfony_exception(
				g_error_streamer(DO_LOG, time_and_place)
					<< "In GParameterTCollectionT<T>::find(item):"
					<< "Tried to find an empty smart pointer." << std::endl
			);
		}

		return std::find_if(
			data.begin(), data.end(), [&item](const std::shared_ptr <T> &cont_item) -> bool {
				bool result = false;
#ifdef DEBUG
            try {
               result = (*item == *(std::dynamic_pointer_cast<item_type>(cont_item)));
            } catch(...) {
					throw gemfony_exception(
						g_error_streamer(DO_LOG, time_and_place)
							<< "Conversion error in GStdPtrVectorInterfaceT::find()" << std::endl
					);
            }
#else
				result = (*item == *(std::static_pointer_cast<item_type>(cont_item)));
#endif
				return result;
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
	template<typename target_type>
	std::shared_ptr <target_type> clone_at(std::size_t pos) const {
		return (data.at(pos))->T::template clone<target_type>();
	}

	/***************************************************************************/
	// Modifying functions

	// Exchange of two data sets
	void swap(std::vector<std::shared_ptr < T>

	>& cont) { data.swap(cont); } // not tested -- trivial mapping

	// Access to elements (unchecked / checked)
	reference operator[](std::size_t pos) { return data[pos]; } // not tested -- trivial mapping
	const_reference operator[](std::size_t pos) const { return data[pos]; } // not tested -- trivial mapping

	reference at(std::size_t pos) { return data.at(pos); } // not tested -- trivial mapping
	const_reference at(std::size_t pos) const { return data.at(pos); } // not tested -- trivial mapping

	reference front() { return data.front(); } // not tested -- trivial mapping
	const_reference front() const { return data.front(); } // not tested -- trivial mapping

	reference back() { return data.back(); } // not tested -- trivial mapping
	const_reference back() const { return data.back(); } // not tested -- trivial mapping

	// Iterators
	iterator begin() { return data.begin(); } // not tested -- trivial mapping
	const_iterator begin() const { return data.begin(); } // not tested -- trivial mapping

	iterator end() { return data.end(); } // not tested -- trivial mapping
	const_iterator end() const { return data.end(); } // not tested -- trivial mapping

	reverse_iterator rbegin() { return data.rbegin(); } // not tested -- trivial mapping
	const_reverse_iterator rbegin() const { return data.rbegin(); } // not tested -- trivial mapping

	reverse_iterator rend() { return data.rend(); } // not tested -- trivial mapping
	const_reverse_iterator rend() const { return data.rend(); } // not tested -- trivial mapping

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
	iterator insert(iterator pos, std::shared_ptr <T> item_ptr) {
		return this->insert_noclone(pos, item_ptr);
	}

	/* ------------------------------------------------------------------------------------------------
	 * Tested via insert_noclone
	 * ------------------------------------------------------------------------------------------------
	 */

	/***************************************************************************/
	/**
	 * Inserts a given item at position pos. Checks whether the item actually points
	 * somewhere. Note that the shared_ptr will inserted itself. Hence any Change you
	 * might make to the object pointed to will also affect the item in the collection.
	 *
	 * @param pos The position where the item should be inserted
	 * @param item_ptr The item to be inserted into the collection
	 */
	iterator insert_noclone(iterator pos, std::shared_ptr <T> item_ptr) {
		if (!item_ptr) { // Check that item actually contains something useful
			throw gemfony_exception(
				g_error_streamer(DO_LOG, time_and_place)
					<< "In GParameterTCollectionT<T>::insert_noclone(pos, item_ptr):"
					<< "Tried to insert an empty smart pointer." << std::endl
			);
		}

		return data.insert(pos, item_ptr);
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
	iterator insert_clone(iterator pos, std::shared_ptr <T> item_ptr) {
		if (!item_ptr) { // Check that item actually contains something useful
			throw gemfony_exception(
				g_error_streamer(DO_LOG, time_and_place)
					<< "In GParameterTCollectionT<T>::insert_clone(pos, item_ptr):"
					<< "Tried to insert an empty smart pointer." << std::endl
			);
		}

		return data.insert(pos, item_ptr->T::template clone<T>());
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
	void insert(iterator pos, size_type amount, std::shared_ptr <T> item_ptr) {
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
	void insert_clone(iterator pos, size_type amount, std::shared_ptr <T> item_ptr) {
		if (!item_ptr) { // Check that item actually contains something useful
			throw gemfony_exception(
				g_error_streamer(DO_LOG, time_and_place)
					<< "In GParameterTCollectionT<T>::insert_clone(pos, amount, item):" << std::endl
					<< "Tried to insert an empty smart pointer." << std::endl
			);
		}

		std::size_t iterator_pos = pos - data.begin();
		for (std::size_t i = 0; i < amount; i++) {
			// Note that we re-calculate the iterator, as it is not clear whether it remains valid
			data.insert(data.begin() + iterator_pos, item_ptr->T::template clone<T>());
		}
	}

	/* ------------------------------------------------------------------------------------------------
	 * Throwing tested in GTestIndividual1::specificTestsFailuresExpected_GUnitTests()
	 * Tested in GTestIndividual1::specificTestsNoFailureExpected_GUnitTests()
	 * ------------------------------------------------------------------------------------------------
	 */

	/***************************************************************************/
	/**
	 * Inserts a given amount of items at position pos. Will not clone the argument.
	 * Note that changes made to item_ptr's object after a call to this function will
	 * also affect the container.
	 *
	 * @param pos The position where items should be inserted
	 * @param amount The amount of items to be inserted
	 * @param item_ptr The item to be inserted into the collection
	 */
	void insert_noclone(iterator pos, size_type amount, std::shared_ptr <T> item_ptr) {
		if (!item_ptr) { // Check that item actually contains something useful
			throw gemfony_exception(
				g_error_streamer(DO_LOG, time_and_place)
					<< "In GParameterTCollectionT<T>::insert_noclone(pos, amount, item):" << std::endl
					<< "Tried to insert an empty smart pointer." << std::endl
			);
		}

		std::size_t iterator_pos = pos - data.begin();
		// Create (amount-1) clones
		for (std::size_t i = 0; i < amount - 1; i++) {
			// Note that we re-calculate the iterator, as it is not clear whether it remains valid
			data.insert(data.begin() + iterator_pos, item_ptr->T::template clone<T>());
		}
		// Add the argument
		data.insert(data.begin() + iterator_pos, item_ptr);
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
	void push_back(std::shared_ptr <T> item_ptr) {
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
	void push_back_noclone(std::shared_ptr <T> item_ptr) {
		if (!item_ptr) { // Check that item actually contains something useful
			throw gemfony_exception(
				g_error_streamer(DO_LOG, time_and_place)
					<< "In GParameterTCollectionT<T>::push_back(item):" << std::endl
					<< "Tried to insert an empty smart pointer." << std::endl
			);
		}

		data.push_back(item_ptr);
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
	void push_back_clone(std::shared_ptr <T> item_ptr) {
		if (!item_ptr) { // Check that item actually contains something useful
			throw gemfony_exception(
				g_error_streamer(DO_LOG, time_and_place)
					<< "In GStdPtrVectorInterface<T>::push_back_clone(item):" << std::endl
					<< "Tried to insert an empty smart pointer." << std::endl
			);
		}

		data.push_back(item_ptr->T::template clone<T>());
	}

	/* ------------------------------------------------------------------------------------------------
	 * Throwing tested in GTestIndividual1::specificTestsFailuresExpected_GUnitTests()
	 * Tested in GTestIndividual1::specificTestsNoFailureExpected_GUnitTests()
	 * ------------------------------------------------------------------------------------------------
	 */

	/***************************************************************************/
	// Removal at a given position or in a range
	iterator erase(iterator pos) { return data.erase(pos); }  // not tested -- trivial mapping
	iterator erase(iterator from, iterator to) { return data.erase(from, to); }  // not tested -- trivial mapping

	// Removing an element from the end of the vector
	void pop_back() { data.pop_back(); }  // not tested -- trivial mapping

	/***************************************************************************/
	/**
	 * Resizing the vector. This function will clone the first item in the collection, if available.
	 */
	void resize(size_type amount) {
		if (this->empty() && amount != 0) {
			throw gemfony_exception(
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
	void resize(size_type amount, std::shared_ptr <T> item_ptr) {
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
	void resize_noclone(size_type amount, std::shared_ptr <T> item_ptr) {
		std::size_t dataSize = data.size();

		if (amount < dataSize)
			data.resize(amount);
		else if (amount > dataSize) {
			// Check that item is not empty
			if (!item_ptr) { // Check that item actually contains something useful
				throw gemfony_exception(
					g_error_streamer(DO_LOG, time_and_place)
						<< "In GParameterTCollectionT<T>::resize(amount, item):" << std::endl
						<< "Tried to insert an empty smart pointer." << std::endl
				);
			}

			// Create a (amount - dataSize -1) clones
			for (std::size_t i = dataSize; i < amount - 1; i++) {
				data.push_back(item_ptr->T::template clone<T>());
			}

			// Finally add item_ptr
			data.push_back(item_ptr);
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
	void resize_clone(size_type amount, std::shared_ptr <T> item_ptr) {
		std::size_t dataSize = data.size();

		if (amount < dataSize)
			data.resize(amount);
		else if (amount > dataSize) {
			// Check that item is not empty
			if (!item_ptr) { // Check that item actually contains something useful
				throw gemfony_exception(
					g_error_streamer(DO_LOG, time_and_place)
						<< "In GParameterTCollectionT<T>::resize(amount, item):" << std::endl
						<< "Tried to insert an empty smart pointer." << std::endl
				);
			}

			for (std::size_t i = dataSize; i < amount; i++) {
				data.push_back(item_ptr->T::template clone<T>());
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
		std::size_t dataSize = data.size();
		if (amount < dataSize) {
			data.resize(amount);
		} else if (amount > dataSize) { // Add empty smart pointers
			for (std::size_t i = dataSize; i < amount; i++) {
				data.push_back(std::shared_ptr<T>());
			}
		}
	}

	/***************************************************************************/
	/** @brief Clearing the data vector */
	void clear() { data.clear(); } // Not tested -- trivial mapping

	/***************************************************************************/
	/**
	 * Creates a copy of the data vector. It is assumed that cp is empty or that
	 * all data in it can be deleted.
	 *
	 * @param cp A reference to a vector that will hold a copy of our local data vector
	 */
	void getDataCopy(std::vector<std::shared_ptr<T>>& cp) const {
		cp.clear();
		for(const auto& item: data) {
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
	void crossOver(GStdPtrVectorInterfaceT<T, B> &cp, const std::size_t &pos) {
		// Find out the minimum size of both vectors
		std::size_t minSize = (std::min)(this->size(), cp.size());

#ifdef DEBUG
		// Do some error checking
		if(pos >= minSize) {
			throw gemfony_exception(
				g_error_streamer(DO_LOG, time_and_place)
					<< "In GStdPtrVectorInterfaceT::crossOver(cp,pos): Error!" << std::endl
					<< "Invalid position " << pos << " / " << this->size() << " / " << cp.size() << std::endl
			);
		}
#endif /* DEBUG */

		// Swap the elements
		for (std::size_t i = pos; i < minSize; i++) {
			std::swap(this->at(i), cp.at(i));
		}

		// Move the elements of the longer vector over to the other
		// and remove the elements from the other vector
		if (this->size() > cp.size()) {
			// Attach elements to the other vector
			for (std::size_t i = cp.size(); i < this->size(); i++) {
				cp.push_back(this->at(i));
			}

			// Remove the surplus elements from this vector
			this->erase(this->begin() + minSize, this->end());
		} else if (cp.size() > this->size()) {
			// Attach elements to the other vector
			for (std::size_t i = this->size(); i < cp.size(); i++) {
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
	template<typename derivedType>
	void attachViewTo(std::vector<std::shared_ptr < derivedType>>& target) {
		typename std::vector<std::shared_ptr < T>> ::iterator it;
		for (it = data.begin(); it != data.end(); ++it) {
			std::shared_ptr <derivedType> p = std::dynamic_pointer_cast<derivedType>(*it);
			if (p) { target.push_back(p); }
		}
	}

	/***************************************************************************/
	/////////////////////////////////////////////////////////////////////////////
	/***************************************************************************/
	/** An iterator implementation that facilitates access to derived elements */
	template<typename derivedType>
	class conversion_iterator
		: public boost::iterator_facade<conversion_iterator<derivedType>, std::shared_ptr<T>, boost::forward_traversal_tag, std::shared_ptr<derivedType>>
	{
	public:
		/************************************************************************/
		/**
		 * The standard constructor. The iterator needs to know about the end of the
		 * sequence so it can skip items not fitting the derivation pattern.
		 *
		 * @param end The end of the iteration sequence
		 */
		conversion_iterator(typename std::vector<std::shared_ptr<T>>::iterator const &end)
			:end_(end)
		{ /* nothing */ }

		/************************************************************************/
		/**
		 * We need to be able to assign values to the iterator, e.g. in a for loop.
		 *
		 * @param current The value to assign to this iterator
		 */
		void operator=(typename std::vector<std::shared_ptr < T>>::iterator const &current) {
			current_ = current;
			// Skip to first "good" entry
			while (current_ != end_ && !(p = std::dynamic_pointer_cast<derivedType>(*current_))) {
				++current_;
			}
		}

		/************************************************************************/
		/**
		 * We need to test whether we have reached the end of the sequence, e.g. in a for loop.
		 *
		 * @param other The iterator to check for inequality
		 * @return A boolean indicating whether this iterator's value is inequal with the other iterator
		 */
		bool operator!=(typename std::vector<std::shared_ptr < T>>::iterator const &other) const {
			return current_ != other;
		}

		/************************************************************************/
		/**
		 * This iterator internally stores a copy of the end of the sequence it iterates over. If
		 * the size of the sequence changes, so does the end point. Hence users need to adapt the
		 * end-point that is stored internally in this class (and which was set with the constructor
		 * in the first place.
		 *
		 * @param end The new end of the sequence
		 */
		void resetEndPosition(typename std::vector<std::shared_ptr < T>>::iterator const &end) {
			end_ = end;
		}

	private:
		/************************************************************************/
		friend class boost::iterator_core_access; ///< Boost's iterator classes need access to the internals of this class

		/************************************************************************/
		/**
		 * Deleted default constructor
		 */
		conversion_iterator() = delete;

		/************************************************************************/
		/**
		 * This is a standard function required by boost's iterator_facade class.
		 *
		 * @return A std::shared_ptr holding the derived object
		 */
		std::shared_ptr <derivedType> dereference() const {
#ifdef DEBUG
			if(current_ == end_) {
				throw gemfony_exception(
					g_error_streamer(DO_LOG, time_and_place)
						<< "In conversion_iterator::dereference(): Error:" << std::endl
						<< "current position at end of sequence" << std::endl
				);
			}

			if(p) return p;
			else {
				throw gemfony_exception(
					g_error_streamer(DO_LOG, time_and_place)
						<< "In conversion_iterator::dereference(): Error: empty pointer" << std::endl
				);

			   return std::shared_ptr<derivedType>(); // Make the compiler happy / empty pointer
			}
#else
			return p;
#endif /* DEBUG */
		}

		/************************************************************************/
		/**
		 * Checks for equality with another iterator
		 *
		 * @param other The item that should be checked for equality
		 * @return A boolean indicating whether equality was found
		 */
		bool equal(typename std::vector<std::shared_ptr < T>>::iterator const &other) const {
			return current_ == other;
		}

		/************************************************************************/
		/**
		 * This function increments the iterator position, possibly skipping items, should they
		 * not meet the derivation pattern.
		 */
		void increment() {
			while (current_ != end_) {
				++current_;
				if (current_ != end_ && (p = std::dynamic_pointer_cast<derivedType>(*current_))) break;
			}
		}

		/************************************************************************/
		typename std::vector<std::shared_ptr <T>>::iterator current_; ///< Marks the current position in the iteration sequence
		typename std::vector<std::shared_ptr <T>>::iterator end_; ///< Marks the end of the iteration sequence

		std::shared_ptr <derivedType> p; ///< Temporary which holds the current valid pointer
	};

protected:
	std::vector<std::shared_ptr<T>> data;

	/** @brief Intentionally make this object purely virtual, for performance reasons */
	virtual void dummyFunction() = 0;

private:
	/***************************************************************************/
	/** @brief Intentionally left undefined */
	bool operator==(const GStdPtrVectorInterfaceT<T, B> &) const = delete;
	/** @brief Intentionally left undefined */
	bool operator!=(const GStdPtrVectorInterfaceT<T, B> &) const = delete;
	/** @brief Intentionally left undefined */
	bool operator==(const std::vector<std::shared_ptr <T>>&) const = delete;
	/** @brief Intentionally left undefined */
	bool operator!=(const std::vector<std::shared_ptr <T>>&) const = delete;

public:
	/** @brief Applies modifications to this object. This is needed for testing purposes */
	// Note to self: changes to GStdPtrVectorInterface should be minimal and not involve objects pointed to
	virtual bool modify_GUnitTests() BASE { /* nothing here yet */ return false; }

	/** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
	virtual void specificTestsNoFailureExpected_GUnitTests() BASE { /* nothing here yet */ }

	/** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
	virtual void specificTestsFailuresExpected_GUnitTests() BASE { /* nothing here yet */ }
};

/******************************************************************************/

} /* namespace Common */
} /* namespace Gem */

/******************************************************************************/
/**
 * @brief The content of the BOOST_SERIALIZATION_ASSUME_ABSTRACT(T) macro. Needed for Boost.Serialization
 */
namespace boost {
namespace serialization {
template<typename T, typename B>
struct is_abstract<Gem::Common::GStdPtrVectorInterfaceT<T, B>> : public boost::true_type {
};
template<typename T, typename B>
struct is_abstract<const Gem::Common::GStdPtrVectorInterfaceT<T, B>> : public boost::true_type {
};
}
}

/******************************************************************************/

#endif /* GSTDPTRVECTORINTERFACET_HPP_ */
