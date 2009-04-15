/**
 * @file GStdSimpleVectorInterfaceT.hpp
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
#include <cmath>
#include <algorithm>

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

#ifndef GSTDSIMPLEVECTORINTERFACET_HPP_
#define GSTDSIMPLEVECTORINTERFACET_HPP_

#include "GObject.hpp"

namespace Gem {
namespace GenEvA {

/********************************************************************************/
/**
 * This class implements most important functions of the std::vector
 * class. It is intended to hold basic types or types that can treated
 * like simple types.
 */
template <typename T>
class GStdSimpleVectorInterfaceT
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
	GStdSimpleVectorInterfaceT() { /* nothing */ }

	/*****************************************************************************/
	/**
	 * Copy construction
	 *
	 * @param cp A constant reference to another GStdSimpleVectorInterfaceT object
	 */
	GStdSimpleVectorInterfaceT(const GStdSimpleVectorInterfaceT<T>& cp) { data =cp.data;	}

	/*****************************************************************************/
	/**
	 * The destructor.
	 */
	virtual ~GStdSimpleVectorInterfaceT() { data.clear();	}

	/*****************************************************************************/
	/**
	 * Assginment operator
	 */
	const GStdSimpleVectorInterfaceT& operator=(const GStdSimpleVectorInterfaceT<T>& cp) {
		this->operator=(cp.data);
		return cp;
	}

	/*****************************************************************************/
	/**
	 * Checks for equality with another GStdSimpleVectorInterfaceT<T> object
	 */
	inline bool operator==(const GStdSimpleVectorInterfaceT<T>& cp) const {
		return this->isEqualTo(cp);
	}

	/*****************************************************************************/
	/**
	 * Checks inquality with another GStdSimpleVectorInterfaceT<T> object
	 */
	inline bool operator!=(const GStdSimpleVectorInterfaceT<T>& cp) const {
		return ! this->isEqualTo(cp);
	}

	/*****************************************************************************/
	/**
	 * Checks for equality with another GStdSimpleVectorInterfaceT<T> object
	 */
	inline bool isEqualTo(const GStdSimpleVectorInterfaceT<T>& cp) const {
		return data==cp.data;
	}

	/*****************************************************************************/
	/**
	 * Checks for equality with a std::vector<T> object
	 */
	inline bool isEqualTo(const std::vector<T>& cp_data) const{
		return data == cp_data;
	}

	/*****************************************************************************/
	/**
	 * Checks for similarity with another GStdSimpleVectorInterfaceT<T> object.
	 */
	inline bool isSimilarTo(const GStdSimpleVectorInterfaceT<T>& cp, const double& limit) const {
		return this->isSimilarTo(cp.data, limit);
	}

	/*****************************************************************************/
	/**
	 * Checks for similarity with another std::vector<T>  object. A specialized
	 * version of this function exists for typeof(T) == typeof(double)
	 */
	inline bool isSimilarTo(const std::vector<T>& cp_data, const double&) const {
		if(	data != cp_data) return false;
		return true;
	}

	/*****************************************************************************/
	/**
	 * operator==
	 */
	inline bool operator==(const std::vector<T>& cp_data) const {
		return cp_data == data;
	}

	/*****************************************************************************/
	/**
	 * operator!=
	 */
	inline bool operator!=(const std::vector<T>& cp_data)  const {
		return !operator==(cp_data);
	}

	/*****************************************************************************/
	// Typedefs
	typedef typename std::vector<T>::value_type value_type;
	typedef typename std::vector<T>::reference reference;
	typedef typename std::vector<T>::const_reference const_reference;

	typedef typename std::vector<T>::iterator iterator;
	typedef typename std::vector<T>::const_iterator const_iterator;
	typedef typename std::vector<T>::reverse_iterator reverse_iterator;
	typedef typename std::vector<T>::const_reverse_iterator const_reverse_iterator;

	typedef typename std::vector<T>::size_type size_type;
	typedef typename std::vector<T>::difference_type difference_type;

	/*****************************************************************************/
	// Non modifying access
	inline size_type size() const { return data.size(); }
	inline bool empty() const { return data.empty(); }
	inline size_type max_size() const { return data.max_size(); }

	inline size_type capacity() const { return data.capacity(); }
	inline void reserve(size_type amount) { data.reserve(amount); }

	/*****************************************************************************/
	/**
	 * Counts the elements whose content is equal to item.
	*
	 * @param item The item to be counted in the collection
	 * @return The number of items found
	 */
	inline size_type count(const T& item) const { return std::count(data.begin(), data.end(), item); }

	/*****************************************************************************/
	/**
	 * Searches for item in the entire range of the vector. Needs to be
	 * re-implemented here, as we are dealing with a collection of smart pointers
	 * and we do not want to compare the pointers themselves.
	 */
	inline const_iterator find(const T& item) const {
		return std::find(data.begin(), data.end(), item);
	}

	/*****************************************************************************/

	// Modifying functions
	inline void swap(std::vector<T>& cont) { std::swap(data, cont); }

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
	inline iterator insert(iterator pos, const T& item) { return data.insert(pos, item); }

	/*****************************************************************************/
	/**
	 * Inserts a given amount of items after position pos.
	 */
	inline void insert(iterator pos, size_type amount, const T& item) {	return data.insert(pos,amount,item); }

	/*****************************************************************************/
	// Adding simple items to the  back of the vector
	inline void push_back(const T& item){ data.push_back(item); }

	/*****************************************************************************/

	// Removal at a given position or in a range
	inline iterator erase(iterator pos) { return data.erase(pos); }
	inline iterator erase(iterator from, iterator to) { return data.erase(from, to); }

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
	inline void resize(size_type amount, const T& item) { data.resize(amount, item);	}

	/*****************************************************************************/
	/** @brief Clearing the data vector */
	inline void clear() { data.clear(); }

	/*****************************************************************************/
	/**
	 * Assignment of a std::vector<T> . As the vector contains smart
	 * pointers, we cannot just copy the pointers themselves but need to copy their content.
	 * We assume here that T has a load() function, as is common for GObject-derivatives.
	 * We also assume that T is copy-constructable.
	 *
	 * @param cp A constant reference to another std::vector<T>
	 * @return The argument of this function (a std::vector<T>)
	 */
	inline const std::vector<T>& operator=(const std::vector<T>& cp) {
		data=cp;
		return cp;
	}

	/*****************************************************************************/
	/**
	 * Creates a copy of the data vector. It is assumed that cp is empty or that
	 * all data in it can be deleted.
	 *
	 * @param cp A reference to a vector that will hold a copy of our local data vector
	 */
	inline void getDataCopy(std::vector<T>& cp){	cp=data; 	}

	/*****************************************************************************/

protected:
	std::vector<T> data;

	/** @brief Intentionally make this object purely virtual, for performance reasons */
	virtual void dummyFunction() = 0;
};

/********************************************************************************/
/** @brief Specialization for typeof(T) == typof(double) */
template <> bool GStdSimpleVectorInterfaceT<double>::isSimilarTo(const std::vector<double>& cp_data, const double& limit) const;

} /* namespace GenEvA */
} /* namespace Gem */

/**************************************************************************************************/
/**
 * @brief The content of the BOOST_SERIALIZATION_ASSUME_ABSTRACT(T) macro. Needed for Boost.Serialization
 */
namespace boost {
  namespace serialization {
    template<typename T>
    struct is_abstract<Gem::GenEvA::GStdSimpleVectorInterfaceT<T> > : boost::true_type {};
    template<typename T>
    struct is_abstract< const Gem::GenEvA::GStdSimpleVectorInterfaceT<T> > : boost::true_type {};
  }
}

/**************************************************************************************************/

#endif /* GSTDSIMPLEVECTORINTERFACET_HPP_ */
