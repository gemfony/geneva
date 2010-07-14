/**
 * @file GStdPtrVectorInterfaceT.hpp
 */

/* Copyright (C) Dr. Ruediger Berlich and Karlsruhe Institute of Technology
 * (University of the State of Baden-Wuerttemberg and National Laboratory
 * of the Helmholtz Association)
 *
 * Contact: info [at] gemfony (dot) com
 *
 * This file is part of the Geneva library, Gemfony scientific's optimization
 * library.
 *
 * Geneva is free software: you can redistribute it and/or modify
 * it under the terms of version 3 of the GNU Affero General Public License
 * as published by the Free Software Foundation.
 *
 * Geneva is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with the Geneva library.  If not, see <http://www.gnu.org/licenses/>.
 *
 * For further information on Gemfony scientific and Geneva, visit
 * http://www.gemfony.com .
 */

// Standard header files go here
#include <sstream>
#include <vector>
#include <typeinfo>

// Includes check for correct Boost version(s)
#include "GGlobalDefines.hpp"

// Boost header files go here

#include <boost/cstdint.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/lexical_cast.hpp>
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
#include <boost/logic/tribool.hpp>
#include <boost/bind.hpp>
#include <boost/optional.hpp>
#include <boost/iterator/iterator_facade.hpp>

#ifndef GSTDPTRVECTORINTERFACET_HPP_
#define GSTDPTRVECTORINTERFACET_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif


#include "GObject.hpp"
#include "GObjectExpectationChecksT.hpp"
#include "GHelperFunctionsT.hpp"

namespace Gem {
namespace GenEvA {

/******************************************************************************************************/
/**
 * This class implements the most important functions of the std::vector
 * class. It is intended to hold boost::shared_ptr smart pointers. Hence
 * special implementations of some functions are needed. Furthermore,
 * using this class prevents us from having to derive directly from a
 * std::vector, which has a non-virtual destructor. Note that we assume here
 * that T holds a complex type, such as a class.  T must implement
 * the interface "usual" for Geneva-GObject derivatives, in particular T must
 * implement the clone() function.
 *
 * Some std::vector functions can not be fully implemented, as they require
 * the data in this class to be default-constructible. As this class can hold
 * smart pointers with purely virtual base pointers, this cannot be done. One
 * important example is the resize(std::size_t) function, which would need to
 * add default-constructed T() objects, if the requested size is larger than
 * the current one.
 */
template <typename T>
class GStdPtrVectorInterfaceT
{
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template<typename Archive>
    void serialize(Archive & ar, const unsigned int){
      using boost::serialization::make_nvp;

      ar & BOOST_SERIALIZATION_NVP(data);
    }
    ///////////////////////////////////////////////////////////////////////

public:
	/**************************************************************************************************/
	/**
	 * The default constructor
	 */
	GStdPtrVectorInterfaceT() { /* nothing */ }

	/**************************************************************************************************/

	/**
	 * Copy construction. The content of the smart pointers is cloned (if content is
	 * available).
	 *
	 * @param cp A constant reference to another GStdPtrVectorInterfaceT object
	 */
	GStdPtrVectorInterfaceT(const GStdPtrVectorInterfaceT<T>& cp) {
		typename std::vector<boost::shared_ptr<T> >::const_iterator cp_it;
		for(cp_it=cp.data.begin(); cp_it!=cp.data.end(); ++cp_it) {
			data.push_back((*cp_it)->GObject::clone<T>());
		}
	}

	/**************************************************************************************************/
	/**
	 * The destructor. Destruction of the objects will be taken care of
	 * by boost::shared_ptr<T>.
	 */
	virtual ~GStdPtrVectorInterfaceT() {
		data.clear();
	}

	/**************************************************************************************************/
	/**
	 * Assginment operator
	 *
	 * @param cp A copy of another GStdPtrVectorInterfaceT<T> object
	 * @return The argument of this function
	 */
	const GStdPtrVectorInterfaceT& operator=(const GStdPtrVectorInterfaceT<T>& cp) {
		this->operator=(cp.data);
		return cp;
	}

	/**************************************************************************************************/
	/**
	 * Assignment of a std::vector<boost::shared_ptr<T> > . As the vector contains smart
	 * pointers, we cannot just copy the pointers themselves but need to copy their content.
	 *
	 * @param cp A constant reference to another std::vector<boost::shared_ptr<T> >
	 * @return The argument of this function
	 */
	const std::vector<boost::shared_ptr<T> >& operator=(const std::vector<boost::shared_ptr<T> >& cp) {
		typename std::vector<boost::shared_ptr<T> >::const_iterator cp_it;
		typename std::vector<boost::shared_ptr<T> >::iterator it;

		std::size_t localSize = data.size();
		std::size_t cpSize = cp.size();

		if(cpSize == localSize) { // The most likely case
			for(it=data.begin(), cp_it=cp.begin(); it!=data.end(), cp_it!=cp.end(); ++it, ++cp_it) {
				(*it)->GObject::load(*cp_it);
			}
		}
		else if(cpSize > localSize) {
			// First copy the initial elements
			for(it=data.begin(), cp_it=cp.begin(); it!=data.end(); ++it, ++cp_it) {
				(*it)->GObject::load(*cp_it);
			}

			// Then attach the remaining objects from cp
			for(cp_it=cp.begin()+localSize; cp_it != cp.end(); ++cp_it) {
				data.push_back((*cp_it)->GObject::clone<T>());
			}
		}
		else if(cpSize < localSize) {
			// First get rid of surplus items
			data.resize(cpSize);

			// Then copy the elements
			for(it=data.begin(), cp_it=cp.begin(); it!=data.end(), cp_it!=cp.end(); ++it, ++cp_it) {
				(*it)->GObject::load(*cp_it);
			}
		}

		return cp;
	}

	/**************************************************************************************************/
	/**
	 * Checks whether a given expectation for the relationship between this object and another object
	 * is fulfilled.
	 *
	 * @param cp A constant reference to another object, camouflaged as a GObject
	 * @param e The expected outcome of the comparison
	 * @param limit The maximum deviation for floating point values (important for similarity checks)
	 * @param caller An identifier for the calling entity
	 * @param y_name An identifier for the object that should be compared to this one
	 * @param withMessages Whether or not information should be emitted in case of deviations from the expected outcome
	 * @return A boost::optional<std::string> object that holds a descriptive string if expectations were not met
	 */
	boost::optional<std::string> checkRelationshipWith(const std::vector<boost::shared_ptr<T> >& cp_data,
			const Gem::Common::expectation& e,
			const double& limit,
			const std::string& caller,
			const std::string& y_name,
			const bool& withMessages) const
	{
	    using namespace Gem::Common;

		// Will hold possible deviations from the expectation, including explanations
	    std::vector<boost::optional<std::string> > deviations;

	    // Assemble a suitable caller string
	    std::string className = std::string("GStdPtrVectorInterfaceT<") + typeid(T).name() + ">";

		// No parent class to check ...

		// Check local data
		deviations.push_back(checkExpectation(withMessages, className , this->data, cp_data, "data", "cp_data", e , limit));

		return evaluateDiscrepancies(className, caller, deviations, e);
	}

	/**************************************************************************************************/
	/**
	 * Checks whether a given expectation for the relationship between this object and another object
	 * is fulfilled.
	 *
	 * @param cp A constant reference to another object, camouflaged as a GObject
	 * @param e The expected outcome of the comparison
	 * @param limit The maximum deviation for floating point values (important for similarity checks)
	 * @param caller An identifier for the calling entity
	 * @param y_name An identifier for the object that should be compared to this one
	 * @param withMessages Whether or not information should be emitted in case of deviations from the expected outcome
	 * @return A boost::optional<std::string> object that holds a descriptive string if expectations were not met
	 */
	boost::optional<std::string> checkRelationshipWith(const GStdPtrVectorInterfaceT<T>& cp,
			const Gem::Common::expectation& e,
			const double& limit,
			const std::string& caller,
			const std::string& y_name,
			const bool& withMessages) const
	{
	    using namespace Gem::Common;

		// Will hold possible deviations from the expectation, including explanations
	    std::vector<boost::optional<std::string> > deviations;

	    // Assemble a suitable caller string
	    std::string className = std::string("GStdPtrVectorInterfaceT<") + typeid(T).name() + ">";

		// No parent class to check ...

		// Check local data
		deviations.push_back(checkExpectation(withMessages, className , this->data, cp.data, "data", "cp.data", e , limit));

		return evaluateDiscrepancies(className, caller, deviations, e);
	}

	/**************************************************************************************************/
	// Typedefs
	typedef typename std::vector<boost::shared_ptr<T> >::value_type value_type;
	typedef typename std::vector<boost::shared_ptr<T> >::reference reference;
	typedef typename std::vector<boost::shared_ptr<T> >::const_reference const_reference;

	typedef typename std::vector<boost::shared_ptr<T> >::iterator iterator;
	typedef typename std::vector<boost::shared_ptr<T> >::const_iterator const_iterator;
	typedef typename std::vector<boost::shared_ptr<T> >::reverse_iterator reverse_iterator;
	typedef typename std::vector<boost::shared_ptr<T> >::const_reverse_iterator const_reverse_iterator;

	typedef typename std::vector<boost::shared_ptr<T> >::size_type size_type;
	typedef typename std::vector<boost::shared_ptr<T> >::difference_type difference_type;

	/**************************************************************************************************/
	// Non modifying access
	size_type size() const { return data.size(); }
	bool empty() const { return data.empty(); }
	size_type max_size() const { return data.max_size(); }

	size_type capacity() const { return data.capacity(); }
	void reserve(size_type amount) { data.reserve(amount); }

	/**************************************************************************************************/
	/**
	 * A small helper class that compares two items and checks for equality, depending on the current mode
	 */
	template <typename item_type>
	class vi_equal_to {
	public:
		typedef bool result_type;

		bool operator() (const boost::shared_ptr<item_type>& item, const boost::shared_ptr<T>& cont_item)  const{
			bool result = false;
#ifdef DEBUG
			try {
				result = (*item == *(boost::dynamic_pointer_cast<item_type>(cont_item)));
			}
			catch(...) {
				std::ostringstream error;
				error << "Unknown error in bool vi_equal_to::operator()" << std::endl;
				throw(Gem::Common::gemfony_error_condition(error.str()));
			}
#else
			result = (*item == *(boost::static_pointer_cast<item_type>(cont_item)));
#endif
			return result;
		}
	};

	/**************************************************************************************************/
	/**
	 * A small helper class that compares two items of identical type
	 * and checks for equality, depending on the current mode
	 */
	class same_equal_to {
	public:
		typedef bool result_type;

		bool operator() (const boost::shared_ptr<T>& item, const boost::shared_ptr<T>& cont_item)  const{
			return (*item == *cont_item);
		}
	};

	/**************************************************************************************************/
	/**
	 * Counts the elements whose content is equal to the content of item.
	 * Needs to be re-implemented here, as we are dealing with a collection of smart pointers
	 * and we do not want to compare the pointers themselves.
	 *
	 * @param item The item to be counted in the collection
	 */
	template <typename item_type>
	size_type count(const boost::shared_ptr<item_type>& item) const {
		if(!item) { // Check that item actually contains something useful
			std::ostringstream error;
			error << "In GParameterTCollectionT<T>::count(item): Error!"
				     << "Tried to count an empty smart pointer." << std::endl;

			throw(Gem::Common::gemfony_error_condition(error.str()));
		}

		if(typeid(item_type) == typeid(T)) {
			return std::count_if(data.begin(), data.end(), boost::bind(same_equal_to(), item, _1));
		}
		else {
			return std::count_if(data.begin(), data.end(),  boost::bind(vi_equal_to<item_type>(), item, _1));
		}
	}

	/**************************************************************************************************/
	/**
	 * Searches for the content of item in the entire range of the vector. Needs to be
	 * re-implemented here, as we are dealing with a collection of smart pointers
	 * and we do not want to compare the pointers themselves.
	 */
	template <typename item_type>
	const_iterator find(const boost::shared_ptr<item_type>& item) const {
		if(!item) { // Check that item actually contains something useful
			std::ostringstream error;
			error << "In GParameterTCollectionT<T>::find(item): Error!"
				     << "Tried to find an empty smart pointer." << std::endl;

			throw(Gem::Common::gemfony_error_condition(error.str()));
		}

		if(typeid(item_type) == typeid(T)) {
			return std::find_if(data.begin(), data.end(), boost::bind(same_equal_to(), item, _1));
		}
		else {
			return std::find_if(data.begin(), data.end(), boost::bind(vi_equal_to<item_type>(), item, _1));
		}
	}

	/**************************************************************************************************/
	// Modifying functions

	// Exchange of two data sets
	void swap(std::vector<boost::shared_ptr<T> >& cont) { data.swap(cont); }

	// Access to elements (unchecked / checked)
	reference operator[](std::size_t pos) { return data[pos]; }
	const_reference operator[](std::size_t pos) const { return data[pos]; }

	reference at(std::size_t pos) { return data.at(pos); }
	const_reference at(std::size_t pos) const { return data.at(pos); }

	reference front() { return data.front(); }
	const_reference front() const { return data.front(); }

	reference back() { return data.back(); }
	const_reference back() const { return data.back(); }

	// Iterators
	iterator begin() { return data.begin(); }
	const_iterator begin() const { return data.begin(); }

	iterator end() { return data.end(); }
	const_iterator end() const { return data.end(); }

	reverse_iterator rbegin() { return data.rbegin(); }
	const_reverse_iterator rbegin() const { return data.rbegin(); }

	reverse_iterator rend() { return data.rend(); }
	const_reverse_iterator rend() const { return data.rend(); }

	/**************************************************************************************************/
	// Insertion and removal


	/**************************************************************************************************/
	/**
	 * Inserts a given item at position pos. Behavior defaults
	 * to isert_noclone(pos,item).
	 *
	 * @param pos The position where the item should be inserted
	 * @param item_ptr The item to be inserted into the collection
	 */
	iterator insert(iterator pos, boost::shared_ptr<T> item_ptr) {
		return this->insert_noclone(pos, item_ptr);
	}

	/**************************************************************************************************/
	/**
	 * Inserts a given item at position pos. Checks whether the item actually points
	 * somewhere. Note that the shared_ptr will inserted itself. Hence any Change you
	 * might make to the object pointed to will also affect the item in the collection.
	 *
	 * @param pos The position where the item should be inserted
	 * @param item_ptr The item to be inserted into the collection
	 */
	iterator insert_noclone(iterator pos, boost::shared_ptr<T> item_ptr) {
		if(!item_ptr) { // Check that item actually contains something useful
			std::ostringstream error;
			error << "In GParameterTCollectionT<T>::insert_noclone(pos, item_ptr): Error!"
				     << "Tried to insert an empty smart pointer." << std::endl;

			throw(Gem::Common::gemfony_error_condition(error.str()));
		}

		return data.insert(pos, item_ptr);
	}

	/**************************************************************************************************/
	/**
	 * Inserts a given item at position pos. Checks whether the item actually points
	 * somewhere. This function clones the item, hence changes to the argument after
	 * invocation of this function will not affect the item pointed to.
	 *
	 * @param pos The position where the item should be inserted
	 * @param item_ptr The item to be inserted into the collection
	 */
	iterator insert_clone(iterator pos, boost::shared_ptr<T> item_ptr) {
		if(!item_ptr) { // Check that item actually contains something useful
			std::ostringstream error;
			error << "In GParameterTCollectionT<T>::insert_clone(pos, item_ptr): Error!"
				     << "Tried to insert an empty smart pointer." << std::endl;

			throw(Gem::Common::gemfony_error_condition(error.str()));
		}

		return data.insert(pos, item_ptr->GObject::clone<T>());
	}

	/**************************************************************************************************/
	/**
	 * Inserts a given amount of items at position pos. Defaults to
	 * insert_clone(pos, amount, item_ptr)
	 *
	 * @param pos The position where items should be inserted
	 * @param amount The amount of items to be inserted
	 * @param item_ptr The item to be inserted into the collection
	 */
	void insert(iterator pos, size_type amount, boost::shared_ptr<T> item_ptr) {
		this->insert_clone(pos, amount, item_ptr);
	}

	/**************************************************************************************************/
	/**
	 * Inserts a given amount of items at position pos. Will always clone.
	 *
	 * @param pos The position where items should be inserted
	 * @param amount The amount of items to be inserted
	 * @param item_ptr The item to be inserted into the collection
	 */
	void insert_clone(iterator pos, size_type amount, boost::shared_ptr<T> item_ptr) {
		if(!item_ptr) { // Check that item actually contains something useful
			std::ostringstream error;
			error << "In GParameterTCollectionT<T>::insert_clone(pos, amount, item): Error!"
				     << "Tried to insert an empty smart pointer." << std::endl;

			throw(Gem::Common::gemfony_error_condition(error.str()));
		}

		std::size_t iterator_pos = pos - data.begin();
		for(std::size_t i=0; i<amount; i++) {
			 // Note that we re-calculate the iterator, as it is not clear whether it remains valid
			data.insert(data.begin() + iterator_pos,  item_ptr->GObject::clone<T>());
		}
	}

	/**************************************************************************************************/
	/**
	 * Inserts a given amount of items at position pos. Will not clone the argument.
	 * Note that changes made to item_ptr's object after a call to this function will
	 * also affect the container.
	 *
	 * @param pos The position where items should be inserted
	 * @param amount The amount of items to be inserted
	 * @param item_ptr The item to be inserted into the collection
	 */
	void insert_noclone(iterator pos, size_type amount, boost::shared_ptr<T> item_ptr) {
		if(!item_ptr) { // Check that item actually contains something useful
			std::ostringstream error;
			error << "In GParameterTCollectionT<T>::insert_noclone(pos, amount, item): Error!"
				     << "Tried to insert an empty smart pointer." << std::endl;

			throw(Gem::Common::gemfony_error_condition(error.str()));
		}

		std::size_t iterator_pos = pos - data.begin();
		// Create (amount-1) clones
		for(std::size_t i=0; i<amount-1; i++) {
			 // Note that we re-calculate the iterator, as it is not clear whether it remains valid
			data.insert(data.begin() + iterator_pos,  item_ptr->GObject::clone<T>());
		}
		// Add the argument
		data.insert(data.begin() + iterator_pos, item_ptr);
	}

	/**************************************************************************************************/
	/**
	 * Ads a shared_ptr object to the  back of the vector. The function defaults to
	 * push_back_noclone
	 *
	 * @param item_ptr The item to be appended to the collection
	 */
	void push_back(boost::shared_ptr<T> item_ptr){
		this->push_back_noclone(item_ptr);
	}

	/**************************************************************************************************/
	/**
	 * Ads a shared_ptr object to the  back of the vector. Note that the shared_ptr
	 * will inserted itself. Hence any Change you might make to the object pointed
	 * to will also affect the item in the collection.
	 *
	 * @param item_ptr The item to be appended to the collection
	 */
	void push_back_noclone(boost::shared_ptr<T> item_ptr){
		if(!item_ptr) { // Check that item actually contains something useful
			std::ostringstream error;
			error << "In GParameterTCollectionT<T>::push_back(item): Error!"
				     << "Tried to insert an empty smart pointer." << std::endl;

			throw(Gem::Common::gemfony_error_condition(error.str()));
		}

		data.push_back(item_ptr);
	}

	/**************************************************************************************************/
	/**
	 * Ads a shared_ptr object to the  back of the vector. The object pointed to
	 * will be cloned. Hence changes to it after a call to this function will not
	 * affect the item stored in the collection.
	 *
	 * @param item_ptr The item to be appended to the collection
	 */
	void push_back_clone(boost::shared_ptr<T> item_ptr){
		if(!item_ptr) { // Check that item actually contains something useful
			std::ostringstream error;
			error << "In GStdPtrVectorInterface<T>::push_back_clone(item): Error!"
				     << "Tried to insert an empty smart pointer." << std::endl;

			throw(Gem::Common::gemfony_error_condition(error.str()));
		}

		data.push_back(item_ptr->GObject::clone<T>());
	}

	/**************************************************************************************************/
	// Removal at a given position or in a range
	iterator erase(iterator pos) { return data.erase(pos); }
	iterator erase(iterator from, iterator to) { return data.erase(from,to); }

	// Removing an element from the end of the vector
	void pop_back(){ data.pop_back(); }

	/**************************************************************************************************/
	/**
	 * Resizing the vector. An increase in size is only allowed if at least one item
	 * is already stored in the collection. The first stored item will then be cloned
	 * the required number of times. The function will throw if an attempt is made to
	 * increase the size of the vector, if it was empty before. The reason is that this
	 * class may hold boost::shared_ptr objects with purely virtual objects. These cannot
	 * be default constructed, which is required if we increase the size of the vector,
	 * and no items in the collection can be used as a template.
	 */
	void resize(size_type amount) {
		if(this->empty() && amount != 0) {
			std::ostringstream error;
			error << "In GStdPtrVectorInterface<T>::resize(size_type): Error!"
				     << "Tried to increase the size even though the vector is empty." << std::endl
				     << "Use a resize-version that allows you to specify the objects" << std::endl
				     << "to be added." << std::endl;
			throw(Gem::Common::gemfony_error_condition(error.str()));
		}

		this->resize_clone(amount, this->at(0));
	}

	/**************************************************************************************************/
	/**
	 * Resizing the vector, initialization with item. This function is a front end
	 * to resize_clone()
	 *
	 * @param amount The new desired size of the vector
	 * @param item An item that should be used for initialization of new items, if any
	 */
	void resize(size_type amount, boost::shared_ptr<T> item_ptr) {
		resize_clone(amount, item_ptr);
	}

	/**************************************************************************************************/
	/**
	 * Resizing the vector, initialization with item. This function does nothing
	 * if amount is the same as data.size(). Note that item_ptr will become part
	 * of the collection. Hence changes to the object pointed to will also affect
	 * the collection.
	 *
	 * @param amount The new desired size of the vector
	 * @param item An item that should be used for initialization of new items, if any
	 */
	void resize_noclone(size_type amount, boost::shared_ptr<T> item_ptr) {
		std::size_t dataSize = data.size();

		if(amount < dataSize)
			data.resize(amount);
		else if(amount > dataSize) {
			// Check that item is not empty
			if(!item_ptr) { // Check that item actually contains something useful
				std::ostringstream error;
				error << "In GParameterTCollectionT<T>::resize(amount, item): Error!"
					     << "Tried to insert an empty smart pointer." << std::endl;

				throw(Gem::Common::gemfony_error_condition(error.str()));
			}

			// Create a (amount - dataSize -1) clones
			for(std::size_t i=dataSize; i<amount-1; i++) {
				data.push_back(item_ptr->GObject::clone<T>());
			}

			// Finally add item_ptr
			data.push_back(item_ptr);
		}
	}

	/**************************************************************************************************/
	/**
	 * Resizing the vector, initialization with item. This function does nothing
	 * if amount is the same as data.size(). item_ptr will be cloned. Hence
	 * changes to the object pointed to will not affect the collection.
	 *
	 * @param amount The new desired size of the vector
	 * @param item An item that should be used for initialization of new items, if any
	 */
	void resize_clone(size_type amount, boost::shared_ptr<T> item_ptr) {
		std::size_t dataSize = data.size();

		if(amount < dataSize)
			data.resize(amount);
		else if(amount > dataSize) {
			// Check that item is not empty
			if(!item_ptr) { // Check that item actually contains something useful
				std::ostringstream error;
				error << "In GParameterTCollectionT<T>::resize(amount, item): Error!"
					     << "Tried to insert an empty smart pointer." << std::endl;

				throw(Gem::Common::gemfony_error_condition(error.str()));
			}

			for(std::size_t i=dataSize; i<amount; i++) {
				data.push_back(item_ptr->GObject::clone<T>());
			}
		}
	}

	/**************************************************************************************************/
	/** @brief Clearing the data vector */
	void clear() { data.clear(); }

	/**************************************************************************************************/
	/**
	 * Creates a copy of the data vector. It is assumed that cp is empty or that
	 * all data in it can be deleted.
	 *
	 * @param cp A reference to a vector that will hold a copy of our local data vector
	 */
	void getDataCopy(std::vector<boost::shared_ptr<T> >& cp) const {
		cp.clear();
		typename std::vector<boost::shared_ptr<T> >::const_iterator it;
		for(it=data.begin(); it!= data.end(); ++it)
			cp.push_back((*it)->GObject::clone<T>());
	}

	/**************************************************************************************************/
	/**
	 * Returns a view on the vector's content, filtering out only items of specific
	 * type.
	 *
	 * @param target A vector to which pointers with the derived type are attached
	 */
	template <typename derivedType>
	void attachViewTo(std::vector<boost::shared_ptr<derivedType> >& target) {
		typename std::vector<boost::shared_ptr<T> >::iterator it;
		for(it = data.begin(); it!=data.end(); ++it) {
			if(boost::shared_ptr<derivedType> p = boost::dynamic_pointer_cast<derivedType>(*it)) {
				target.push_back(p);
			}
		}
	}

	/**************************************************************************************************/
	////////////////////////////////////////////////////////////////////////////////////////////////////
	/**************************************************************************************************/
	/** An iterator implementation that facilitates access to derived elements */
	template <typename derivedType>
	class conversion_iterator:
		public boost::iterator_facade<
		conversion_iterator<derivedType>
		, boost::shared_ptr<T>
		, boost::forward_traversal_tag
		, boost::shared_ptr<derivedType>
		>
	{
	public:
		/**********************************************************************************************/
		/**
		 * The standard constructor. The iterator needs to know about the end of the
		 * sequence so it can skip items not fitting the derivation pattern.
		 *
		 * @param end The end of the iteration sequence
		 */
		conversion_iterator(typename std::vector<boost::shared_ptr<T> >::iterator const& end)
			:end_(end)
		 { /* nothing */ }

		/**********************************************************************************************/
		/**
		 * We need to be able to assign values to the iterator, e.g. in a for loop.
		 *
		 * @param current The value to assign to this iterator
		 */
		void operator=(typename std::vector<boost::shared_ptr<T> >::iterator const& current) {
			current_ = current;
			// Skip to first "good" entry
			while(current_ != end_ && !(p=boost::dynamic_pointer_cast<derivedType>(*current_))) {
				++current_;
			}
		}

		/**********************************************************************************************/
		/**
		 * We need to test whether we have reached the end of the sequence, e.g. in a for loop.
		 *
		 * @param other The iterator to check for inequality
		 * @return A boolean indicating whether this iterator's value is inequal with the other iterator
		 */
		bool operator!=(typename std::vector<boost::shared_ptr<T> >::iterator const& other) const {
			return current_ != other;
		}

		/**********************************************************************************************/
		/**
		 * This iterator internally stores a copy of the end of the sequence it iterates over. If
		 * the size of the sequence changes, so does the end point. Hence users need to adapt the
		 * end-point that is stored internally in this class (and which was set with the constructor
		 * in the first place.
		 *
		 * @param end The new end of the sequence
		 */
		void resetEndPosition(typename std::vector<boost::shared_ptr<T> >::iterator const& end) {
			end_ = end;
		}

		/**********************************************************************************************/
	private:
		friend class boost::iterator_core_access; ///< Boost's iterator classes need access to the internals of this class

		/**********************************************************************************************/
		/**
		 * The default constructor. Intentionally left undefined and private, so it cannot be
		 * instantiated.
		 */
		conversion_iterator();

		/**********************************************************************************************/
		/**
		 * This is a standard function required by boost's iterator_facade class.
		 *
		 * @return A boost::shared_ptr holding the derived object
		 */
		boost::shared_ptr<derivedType> dereference() const {
#ifdef DEBUG
			if(current_ == end_) {
				std::ostringstream error;
				error << "In conversion_iterator::dereference(): Error:" << std::endl
					  << "current position at end of sequence" << std::endl;
				throw(Gem::Common::gemfony_error_condition(error.str()));
			}
#endif /* DEBUG */

			return p;
#ifdef DEBUG
			if(p) return p;
			else {
				if(!p) {
					std::ostringstream error;
					error << "In conversion_iterator::dereference(): Error: empty pointer" << std::endl;
					throw(Gem::Common::gemfony_error_condition(error.str()));
				}
			}
#endif /* DEBUG*/
		}

		/**********************************************************************************************/
		/**
		 * Checks for equality with another iterator
		 *
		 * @param other The item that should be checked for equality
		 * @return A boolean indicating whether equality was found
		 */
		bool equal(typename std::vector<boost::shared_ptr<T> >::iterator const& other) const {
			return current_ == other;
		}

		/**********************************************************************************************/
		/**
		 * This function increments the iterator position, possibly skipping items, should they
		 * not meet the derivation pattern.
		 */
		void increment() {
			while (current_ != end_) {
				++current_;
				if(current_!=end_ && (p = boost::dynamic_pointer_cast<derivedType>(*current_))) break;
			}
		}

		/**********************************************************************************************/
		typename std::vector<boost::shared_ptr<T> >::iterator current_; ///< Marks the current position in the iteration sequence
		typename std::vector<boost::shared_ptr<T> >::iterator end_; ///< Marks the end of the iteration sequence

		boost::shared_ptr<derivedType> p; ///< Temporary which holds the current valid pointer
	};

#ifdef GENEVATESTING
	/**************************************************************************************************/
	////////////////////////////////////////////////////////////////////////////////////////////////////
	/**************************************************************************************************/
	/** @brief Applies modifications to this object. This is needed for testing purposes */
	virtual bool modify_GUnitTests() { /* nothing here yet */ return false; }
	/** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
	virtual void specificTestsNoFailureExpected_GUnitTests() { /* nothing here yet */ }
	/** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
	virtual void specificTestsFailuresExpected_GUnitTests() { /* nothing here yet */ }
#endif /* GENEVATESTING */

protected:
	std::vector<boost::shared_ptr<T> > data;

	/** @brief Intentionally make this object purely virtual, for performance reasons */
	virtual void dummyFunction() = 0;

private:
	/**************************************************************************************************/
	/** @brief Intentionally left undefined */
	bool operator==(const GStdPtrVectorInterfaceT<T>&) const;
	/** @brief Intentionally left undefined */
	bool operator!=(const GStdPtrVectorInterfaceT<T>&) const;
	/** @brief Intentionally left undefined */
	bool operator==(const std::vector<boost::shared_ptr<T> >&) const;
	/** @brief Intentionally left undefined */
	bool operator!=(const std::vector<boost::shared_ptr<T> >&) const;
};

/**************************************************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */

/**************************************************************************************************/
/**
 * @brief The content of the BOOST_SERIALIZATION_ASSUME_ABSTRACT(T) macro. Needed for Boost.Serialization
 */
namespace boost {
  namespace serialization {
    template<typename T>
    struct is_abstract<Gem::GenEvA::GStdPtrVectorInterfaceT<T> > : public boost::true_type {};
    template<typename T>
    struct is_abstract< const Gem::GenEvA::GStdPtrVectorInterfaceT<T> > : public boost::true_type {};
  }
}

/**************************************************************************************************/

#endif /* GSTDPTRVECTORINTERFACET_HPP_ */
