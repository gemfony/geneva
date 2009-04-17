/**
 * @file GStdPtrVectorInterfaceT.hpp
 */

/* Copyright (C) 2009 Dr. Ruediger Berlich
 *
 * This file is part of Geneva, Gemfony scientific's optimization library.
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
 */

// Standard header files go here
#include <sstream>
#include <vector>
#include <typeinfo>

// Boost header files go here

#include <boost/version.hpp>
#include "GGlobalDefines.hpp"
#if BOOST_VERSION < ALLOWED_BOOST_VERSION
#error "Error: Boost has incorrect version !"
#endif /* BOOST_VERSION */

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

#ifndef GSTDPTRVECTORINTERFACET_HPP_
#define GSTDPTRVECTORINTERFACET_HPP_

#include "GObject.hpp"

namespace Gem {
namespace GenEvA {

/********************************************************************************/
/**
 * This class implements most important functions of the std::vector
 * class. It is intended to hold boost::shared_ptr smart pointers. Hence
 * special implementations of some functions are needed. Furthermore,
 * using this class prevents us from having to derive directly from a
 * std::vector, which has a non-virtual destructor. Note that we assume here
 * that T holds a complex type, such as a class.  T must implement
 * the Geneva-style isEqualTo and isSimilarTo functions.
 *
 * Some std::vector functions can not be implemented, as they require
 * the data in this class to be default-constructible. As this class can hold
 * smart pointers with purely virtual base pointers, this cannot be done.
 */
template <typename T>
class GStdPtrVectorInterfaceT
{
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template<typename Archive>
    void serialize(Archive & ar, const unsigned int){
      using boost::serialization::make_nvp;
      ar & make_nvp("data_T",data);
    }
    ///////////////////////////////////////////////////////////////////////

public:
	/*****************************************************************************/
	/**
	 * The default constructor
	 */
	GStdPtrVectorInterfaceT() { /* nothing */ }

	/*****************************************************************************/
	/**
	 * Copy construction
	 *
	 * @param cp A constant reference to another GStdPtrVectorInterfaceT object
	 */
	GStdPtrVectorInterfaceT(const GStdPtrVectorInterfaceT<T>& cp) {
		typename std::vector<boost::shared_ptr<T> >::const_iterator cp_it;
		for(cp_it=cp.data.begin(); cp_it!=cp.data.end(); ++cp_it)
			data.push_back((*cp_it)->GObject::clone_bptr_cast<T>());
	}

	/*****************************************************************************/
	/**
	 * The destructor. Destruction of the objects pointed to by the smart
	 * pointers will be taken care of by boost::shared_ptr<T>.
	 */
	virtual ~GStdPtrVectorInterfaceT() {
		data.clear();
	}

	/*****************************************************************************/
	/**
	 * Assginment operator
	 */
	const GStdPtrVectorInterfaceT& operator=(const GStdPtrVectorInterfaceT<T>& cp) {
		this->operator=(cp.data);
		return cp;
	}

	/*****************************************************************************/
	/**
	 * Checks for equality with another GStdPtrVectorInterfaceT<T> object
	 */
	bool operator==(const GStdPtrVectorInterfaceT<T>& cp) const {
		return this->checkIsEqualTo(cp);
	}

	/*****************************************************************************/
	/**
	 * Checks inquality with another GStdPtrVectorInterfaceT<T> object
	 */
	bool operator!=(const GStdPtrVectorInterfaceT<T>& cp) const {
		return ! this->checkIsEqualTo(cp);
	}

	/*****************************************************************************/
	/**
	 * Checks for equality with another GStdPtrVectorInterfaceT<T> object
	 */
	bool checkIsEqualTo(const GStdPtrVectorInterfaceT<T>& cp) const {
		return this->operator==(cp.data);
	}

	/*****************************************************************************/
	/**
	 * Checks for similarity with another GStdPtrVectorInterfaceT<T> object.
	 */
	bool checkIsSimilarTo(const GStdPtrVectorInterfaceT<T>& cp, const double& limit) const {
		return this->checkIsSimilarTo(cp.data, limit);
	}

	/*****************************************************************************/
	/**
	 * Checks for similarity with another std::vector<boost::shared_ptr<T> > object.
	 * Note that we assume here that T actually implements a isSimilarTo function.
	 */
	bool checkIsSimilarTo(const std::vector<boost::shared_ptr<T> >& cp_data, const double& limit) const {
		// Check sizes
		if(data.size() != cp_data.size()) return false;

		// Check the content
		typename std::vector<boost::shared_ptr<T> >::const_iterator cp_it;
		typename std::vector<boost::shared_ptr<T> >::const_iterator it;
		for(cp_it=cp_data.begin(), it=data.begin(); cp_it != cp_data.end(), it!=data.end(); ++cp_it, ++it) {
			if(!(*it)->isSimilarTo(**cp_it, limit)) return false;
		}

		return true;
	}

	/*****************************************************************************/
	/**
	 * operator==, modified to check the content of the smart pointers
	 */
	bool operator==(const std::vector<boost::shared_ptr<T> >& cp_data) const {
		// Check sizes
		if(data.size() != cp_data.size()) return false;

		// Check the content
		typename std::vector<boost::shared_ptr<T> >::const_iterator cp_it;
		typename std::vector<boost::shared_ptr<T> >::const_iterator it;
		for(cp_it=cp_data.begin(), it=data.begin(); cp_it != cp_data.end(), it!=data.end(); ++cp_it, ++it) {
			if(!(*it)->isEqualTo(**cp_it)) return false;
		}

		return true;
	}

	/*****************************************************************************/
	/**
	 * operator!=, modified to check the content of the smart pointers
	 */
	bool operator!=(const std::vector<boost::shared_ptr<T> >& cp_data) const {
		return !operator==(cp_data);
	}

	/*****************************************************************************/
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

	/*****************************************************************************/
	// Non modifying access
	inline size_type size() const { return data.size(); }
	inline bool empty() const { return data.empty(); }
	inline size_type max_size() const { return data.max_size(); }

	inline size_type capacity() const { return data.capacity(); }
	inline void reserve(size_type amount) { data.reserve(amount); }

	/*****************************************************************************/
	/**
	 * A small helper class that compares two items and checks for
	 * equality, depending on the current mode
	 */
	template <typename item_type>
	class vi_equal_to {
	public:
		typedef bool result_type;

		bool operator() (const item_type& item, const boost::shared_ptr<T>& cont_item) {
			bool result = false;
#ifdef DEBUG
			try {
				result = (item == *(boost::dynamic_pointer_cast<item_type>(cont_item)));
			}
			catch(...) {
				std::ostringstream error;
				error << "Unknown error in bool vi_equal_to::operator()" << std::endl;
				throw(Gem::GenEvA::geneva_error_condition(error.str()));
			}
#else
			result = (item == *(boost::static_pointer_cast<item_type>(cont_item)));
#endif
			return result;
		}
	};

	/*****************************************************************************/
	/**
	 * Counts the elements whose content is equal to item.
	 * Needs to be re-implemented here, as we are dealing with a collection of smart pointers
	 * and we do not want to compare the pointers themselves.
	 *
	 * @param item The item to be counted in the collection
	 */
	template <typename item_type>
	inline size_type count(const item_type& item) const {
		if(typeid(item_type) == typeid(T)) {
			return std::count_if(data.begin(), data.end(), boost::bind(std::equal_to<T>(), item, boost::bind(Gem::Util::dereference<T>, _1)));
		}
		else {
			return std::count_if(data.begin(), data.end(), boost::bind(vi_equal_to<item_type>(), item, _1));
		}
	}

	/*****************************************************************************/
	/**
	 * Counts the elements whose content is equal to the content of item.
	 * Needs to be re-implemented here, as we are dealing with a collection of smart pointers
	 * and we do not want to compare the pointers themselves.
	 *
	 * @param item The item to be counted in the collection
	 */
	template <typename item_type>
	inline size_type count(const boost::shared_ptr<item_type>& item) const {
		if(!item) { // Check that item actually contains something useful
			std::ostringstream error;
			error << "In GParameterTCollectionT<T>::count(item): Error!"
				     << "Tried to count an empty smart pointer." << std::endl;

			throw(Gem::GenEvA::geneva_error_condition(error.str()));
		}

		if(typeid(item_type) == typeid(T)) {
			return std::count_if(data.begin(), data.end(),
					boost::bind(std::equal_to<T>(), boost::bind(Gem::Util::dereference<T>, item),
							boost::bind(Gem::Util::dereference<T>, _1)));
		}
		else {
			return std::count_if(data.begin(), data.end(),
					boost::bind(vi_equal_to<item_type>(), boost::bind(Gem::Util::dereference<item_type>, item), _1));
		}
	}

	/*****************************************************************************/
	/**
	 * Searches for item in the entire range of the vector. Needs to be
	 * re-implemented here, as we are dealing with a collection of smart pointers
	 * and we do not want to compare the pointers themselves.
	 */
	template <typename item_type>
	inline const_iterator find(const item_type& item) const {
		if(typeid(item_type) == typeid(T)) {
			return std::find_if(data.begin(), data.end(),
					boost::bind(std::equal_to<T>(),item, boost::bind(Gem::Util::dereference<T>, _1)));
		}
		else {
			return std::find_if(data.begin(), data.end(), boost::bind(vi_equal_to<item_type>(), item, _1));
		}
	}

	/*****************************************************************************/
	/**
	 * Searches for the content of item in the entire range of the vector. Needs to be
	 * re-implemented here, as we are dealing with a collection of smart pointers
	 * and we do not want to compare the pointers themselves.
	 */
	template <typename item_type>
	inline const_iterator find(const boost::shared_ptr<item_type>& item) const {
		if(!item) { // Check that item actually contains something useful
			std::ostringstream error;
			error << "In GParameterTCollectionT<T>::find(item): Error!"
				     << "Tried to find an empty smart pointer." << std::endl;

			throw(Gem::GenEvA::geneva_error_condition(error.str()));
		}

		if(typeid(item_type) == typeid(T)) {
			return std::find_if(data.begin(), data.end(),
					boost::bind(std::equal_to<T>(), boost::bind(Gem::Util::dereference<T>, item),
							boost::bind(Gem::Util::dereference<T>, _1)));
		}
		else {
			return std::find_if(data.begin(), data.end(),
					boost::bind(vi_equal_to<item_type>(), boost::bind(Gem::Util::dereference<item_type>, item), _1));
		}
	}

	/*****************************************************************************/

	// Modifying functions
	inline void swap(std::vector<boost::shared_ptr<T> >& cont) { data.swap(cont); }

	// Access to elements (unchecked / checked)
	inline reference operator[](std::size_t pos) { return data[pos]; }
	inline const reference operator[](std::size_t pos) const { return data[pos]; }
	inline reference at(std::size_t pos) { return data.at(pos); }
	inline const reference at(std::size_t pos) const { return data.at(pos); }

	inline reference front() { return data.front(); }
	inline const_reference front() const { return data.front(); }

	inline reference back() { return data.back(); }
	inline const_reference back() const { return data.back(); }

	// Iterators
	inline iterator begin() { return data.begin(); }
	inline const_iterator begin() const { return data.begin(); }

	inline iterator end() { return data.end(); }
	inline const_iterator end() const { return data.end(); }

	inline reverse_iterator rbegin() { return data.rbegin(); }
	inline const_reverse_iterator rbegin() const { return data.rbegin(); }

	inline reverse_iterator rend() { return data.rend(); }
	inline const_reverse_iterator rend() const { return data.rend(); }

	/*****************************************************************************/
	// Insertion and removal

	/**
	 * Inserts a given item at position pos. Checks whether the item actually points
	 * somewhere.
	 */
	inline iterator insert(iterator pos, const T& item) {
		return data.insert(pos, item.GObject::clone_bptr_cast<T>());
	}

	/*****************************************************************************/
	/**
	 * Inserts a given item at position pos. Checks whether the item actually points
	 * somewhere.
	 */
	inline iterator insert(iterator pos, boost::shared_ptr<T> item) {
		if(!item) { // Check that item actually contains something useful
			std::ostringstream error;
			error << "In GParameterTCollectionT<T>::insert(pos, item): Error!"
				     << "Tried to insert an empty smart pointer." << std::endl;

			throw(Gem::GenEvA::geneva_error_condition(error.str()));
		}

		return data.insert(pos, item->GObject::clone_bptr_cast<T>());
	}

	/*****************************************************************************/
	/**
	 * Inserts a given amount of items after position pos.
	 */
	inline void insert(iterator pos, size_type amount, const T& item) {
		std::size_t iterator_pos = pos - data.begin();
		for(std::size_t i=0; i<amount; i++) {
			 // Note that we re-calculate the iterator, as it is not clear whether it remains valid
			data.insert(data.begin() + iterator_pos, item.GObject::clone_bptr_cast<T>());
		}
	}

	/*****************************************************************************/
	/**
	 * Inserts a given amount of items after position pos.
	 */
	inline void insert(iterator pos, size_type amount, boost::shared_ptr<T> item_ptr) {
		if(!item_ptr) { // Check that item actually contains something useful
			std::ostringstream error;
			error << "In GParameterTCollectionT<T>::insert(pos, amount, item): Error!"
				     << "Tried to insert an empty smart pointer." << std::endl;

			throw(Gem::GenEvA::geneva_error_condition(error.str()));
		}

		std::size_t iterator_pos = pos - data.begin();
		for(std::size_t i=0; i<amount; i++) {
			 // Note that we re-calculate the iterator, as it is not clear whether it remains valid
			data.insert(data.begin() + iterator_pos,  item_ptr->GObject::clone_bptr_cast<T>());
		}
	}

	/*****************************************************************************/
	// Adding shared_ptr objects to the  back of the vector.
	inline void push_back(boost::shared_ptr<T> item_ptr){
		if(!item_ptr) { // Check that item actually contains something useful
			std::ostringstream error;
			error << "In GParameterTCollectionT<T>::push_back(item): Error!"
				     << "Tried to insert an empty smart pointer." << std::endl;

			throw(Gem::GenEvA::geneva_error_condition(error.str()));
		}

		data.push_back(item_ptr->GObject::clone_bptr_cast<T>());
	}

	/*****************************************************************************/
	// Adding simple items to the  back of the vector. Note
	// that this function does not clone the object.
	inline void push_back(const T& item){
		data.push_back(item.GObject::clone_bptr_cast<T>());
	}

	/*****************************************************************************/

	// Removal at a given position or in a range
	inline iterator erase(iterator pos) { return data.erase(pos); }
	inline iterator erase(iterator from, iterator to) { return data.erase(from,to); }

	// Removing an element from the end of the vector
	inline void pop_back(){ data.pop_back(); }

	/*****************************************************************************/
	/**
	 * Resizing the vector, initialization with item. This function does nothing
	 * if amount is the same as data.size(). We assume in this function that
	 * T is copy-constructible.
	 *
	 * @param amount The new desired size of the vector
	 * @param item An item that should be used for initialization of new items, if any
	 */
	inline void resize(size_type amount, boost::shared_ptr<T> item_ptr) {
		std::size_t dataSize = data.size();

		if(amount < dataSize)
			data.resize(amount);
		else if(amount > dataSize) {
			// Check that item is not empty
			if(!item_ptr) { // Check that item actually contains something useful
				std::ostringstream error;
				error << "In GParameterTCollectionT<T>::resize(amount, item): Error!"
					     << "Tried to insert an empty smart pointer." << std::endl;

				throw(Gem::GenEvA::geneva_error_condition(error.str()));
			}

			for(std::size_t i=dataSize; i<amount; i++) {
				data.push_back(item_ptr->GObject::clone_bptr_cast<T>());
			}
		}
	}

	/*****************************************************************************/
	/**
	 * Resizing the vector, initialization with item. This function does nothing
	 * if amount is the same as data.size(). We assume in this function that
	 * T is copy-constructible.
	 *
	 * @param amount The new desired size of the vector
	 * @param item An item that should be used for initialization of new items, if any
	 */
	inline void resize(size_type amount, const T& item) {
		std::size_t dataSize = data.size();

		if(amount < dataSize)
			data.resize(amount);
		else if(amount > dataSize) {
			for(std::size_t i=dataSize; i<amount; i++) {
				data.push_back(item.GObject::clone_bptr_cast<T>());
			}
		}
	}

	/*****************************************************************************/
	/** @brief Clearing the data vector */
	inline void clear() { data.clear(); }

	/*****************************************************************************/
	/**
	 * Assignment of a std::vector<boost::shared_ptr<T> > . As the vector contains smart
	 * pointers, we cannot just copy the pointers themselves but need to copy their content.
	 * We assume here that T has a load() function, as is common for GObject-derivatives.
	 * We also assume that T is copy-constructable.
	 *
	 * @param cp A constant reference to another std::vector<boost::shared_ptr<T> >
	 * @return The argument of this function (a std::vector<boost::shared_ptr<T> >)
	 */
	inline const std::vector<boost::shared_ptr<T> >& operator=(const std::vector<boost::shared_ptr<T> >& cp) {
		typename std::vector<boost::shared_ptr<T> >::const_iterator cp_it;
		typename std::vector<boost::shared_ptr<T> >::iterator it;

		std::size_t localSize = data.size();
		std::size_t cpSize = cp.size();

		if(cpSize == localSize) { // The most likely case
			for(it=data.begin(), cp_it=cp.begin(); it!=data.end(), cp_it!=cp.end(); ++it, ++cp_it) **it = **cp_it;
		}
		else if(cpSize > localSize) {
			// First copy the initial elements
			for(it=data.begin(), cp_it=cp.begin(); it!=data.end(); ++it, ++cp_it) **it = **cp_it;

			// Then attach the remaining objects from cp
			for(cp_it=cp.begin()+localSize; cp_it != cp.end(); ++cp_it) {
				data.push_back((*cp_it)->GObject::clone_bptr_cast<T>());
			}
		}
		else if(cpSize < localSize) {
			// First get rid of surplus items
			data.resize(cpSize);

			// Then copy the elements
			for(it=data.begin(), cp_it=cp.begin(); it!=data.end(), cp_it!=cp.end(); ++it, ++cp_it) **it = **cp_it;
		}

		return cp;
	}

	/*****************************************************************************/
	/**
	 * Creates a copy of the data vector. It is assumed that cp is empty or that
	 * all data in it can be deleted.
	 *
	 * @param cp A reference to a vector that will hold a copy of our local data vector
	 */
	void getDataCopy(std::vector<boost::shared_ptr<T> >& cp){
		cp.clear();
		typename std::vector<boost::shared_ptr<T> >::iterator it;
		for(it=data.begin(); it!= data.end(); ++it)
			cp.push_back(boost::shared_ptr<T>((*it)->GObject::clone_ptr_cast<T>()));
	}

	/*****************************************************************************/

protected:
	std::vector<boost::shared_ptr<T> > data;

	/** @brief Intentionally make this object purely virtual, for performance reasons */
	virtual void dummyFunction() = 0;
};

/********************************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */

/**************************************************************************************************/
/**
 * @brief The content of the BOOST_SERIALIZATION_ASSUME_ABSTRACT(T) macro. Needed for Boost.Serialization
 */
namespace boost {
  namespace serialization {
    template<typename T>
    struct is_abstract<Gem::GenEvA::GStdPtrVectorInterfaceT<T> > : boost::true_type {};
    template<typename T>
    struct is_abstract< const Gem::GenEvA::GStdPtrVectorInterfaceT<T> > : boost::true_type {};
  }
}

/**************************************************************************************************/

#endif /* GSTDPTRVECTORINTERFACET_HPP_ */
