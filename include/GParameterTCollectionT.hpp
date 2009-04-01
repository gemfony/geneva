/**
 * @file GParameterTCollectionT.hpp
 */

/* Copyright (C) 2004-2008 Dr. Ruediger Berlich
 * Copyright (C) 2007-2008 Forschungszentrum Karlsruhe GmbH
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
#include <algorithm>

// Boost header files go here

#include <boost/version.hpp>
#include "GGlobalDefines.hpp"
#if BOOST_VERSION < ALLOWED_BOOST_VERSION
#error "Error: Boost has incorrect version !"
#endif /* BOOST_VERSION */

#include <boost/cstdint.hpp>
#include <boost/shared_ptr.hpp>

#ifndef GPARAMETERTCOLLECTIONT_HPP_
#define GPARAMETERTCOLLECTIONT_HPP_

// GenEvA header files go here
#include "GParameterBase.hpp"
#include "GParameterT.hpp"
#include "GHelperFunctionsT.hpp"

namespace Gem {
namespace GenEvA {

/***********************************************************************************************/
/**
 * This class shares many similarities with the GParameterCollectionT class. Instead
 * of individual values that can be modified with adaptors, however, it assumes that
 * the objects stored in it have their own mutate() function. Consequently it is not
 * necessary to store adaptors locally. This class has been designed as a collection
 * of GParameterT objects, hence the name.  As an example, one can create a collection
 * of GBoundedDouble objects with this class rather than a simple GDoubleCollection.
 * In order to facilitate memory management, the GParameterT objects are stored
 * in boost::shared_ptr objects.
 */
template<typename T>
class GParameterTCollectionT
	:public GParameterBase
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int version) {
		using boost::serialization::make_nvp;
		ar & make_nvp("GParameterBase", boost::serialization::base_object<GParameterBase>(*this));
		ar & make_nvp("data_T", data);
	}
	///////////////////////////////////////////////////////////////////////

public:
	/*******************************************************************************************/
	/**
	 * The default constructor
	 */
	GParameterTCollectionT()
		:GParameterBase()
	{ /* nothing */ }

	/*******************************************************************************************/
	/**
	 * The copy constructor
	 *
	 * @param cp A copy of another GParameterTCollectionT<T> object
	 */
	GParameterTCollectionT(const GParameterTCollectionT<T>& cp)
		:GParameterBase (cp)
	{
		// All data is stored in the data vector
		Gem::Util::copySmartPointerVector(cp.data, data);
	}

	/*******************************************************************************************/
	/**
	 * The standard destructor
	 */
	virtual ~GParameterTCollectionT()
	{ /* nothing */ }

	/*******************************************************************************************/
	/**
	 * A standard assignment operator.
	 *
	 * @param cp A copy of another GParameterTCollectionT<T> object
	 * @return A constant reference to this object
	 */
	const GParameterTCollectionT<T>& operator=(const GParameterTCollectionT<T>& cp)
	{
		GParameterTCollectionT<T>::load(cp);
		return *this;
	}

	/*******************************************************************************************/
	/**
	 * Creates a deep clone of this object. Purely virtual, so this class cannot be
	 * be instantiated directly.
	 */
	virtual GObject* clone() = 0;

	/*******************************************************************************************/
	/**
	 * Loads the data of another GParameterTCollectionT<T> object, camouflaged as a GObject
	 *
	 * @param cp A copy of another GParameterTCollectionT<T> object, camouflaged as a GObject
	 */
	virtual void load(const GObject* cp) {
		// Convert cp into local format
		const GParameterTCollectionT<T> *gptct = this->conversion_cast(cp, this);

		// Load our parent class'es data ...
		GParameterBase::load(cp);

		// ... then copy the vector. (Implemented in file GHelperFunctionsT.hpp)
		Gem::Util::copySmartPointerVector(gptct->data, data);
	}

	/*******************************************************************************************/
	/**
	 * Allows to mutate the values stored in this class. We assume here that
	 * each item has its own mutate function. Hence we do not need to use or
	 * store own adaptors.
	 */
	virtual void mutate() {
		typename std::vector<boost::shared_ptr<T> >::iterator it;
		for(it=data.begin(); it!=data.end(); ++it) (*it)->mutate();
	}

	/*******************************************************************************************/
	/**
	 * Creates a copy of the data vector. Note that this function assumes that the parameters
	 * stored in this class are basic values, such as doubles. If T is a class type, then a suitable
	 * operator= needs to be defined for these types.
	 *
	 * @param cp A reference to a vector that will hold a copy of our local data vector
	 */
	void getDataCopy(std::vector<T>& cp){
		cp = data;
	}

	/*******************************************************************************************/
	//////////////////////// std::vector<T> interface (incomplete) //////////////////////////////
	/*******************************************************************************************/
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

	// Non modifying access
	inline size_type size() const { return data.size(); }
	inline bool empty() const { return data.empty(); }
	inline size_type max_size() const { return data.max_size(); }

	inline size_type capacity() const { return data.capacity(); }
	inline void reserve(size_type amount) { data.reserve(amount); }

	/*******************************************************************************************/
	/**
	 * Counts the elements whose content is equal to the content of item.
	 * Needs to be re-implemented here, as we are dealing with a collection of smart pointers
	 * and we do not want to compare the pointers themselves.
	 */
	inline size_type count(const T& item) const {
		size_type num;
		std::count_if(data.begin(), data.end(), boost::bind(std::equal_to<T>(), _1, item), num);
		return num;
	}

	/*******************************************************************************************/
	/**
	 * Searches for the content of item in the entire range. Needs to be
	 * re-implemented here, as we are dealing with a collection of smart pointers
	 * and we do not want to compare the pointers themselves.
	 */
	inline iterator find(const boost::shared_ptr<T>& item) {
		return std::find_if(data.begin(), data.end(), boost::bind(std::equal_to<T>(),_1,item));
	}

	/*******************************************************************************************/
	/**
	 * Searches for the content of item in the entire range. Needs to be
	 * re-implemented here, as we are dealing with a collection of smart pointers
	 * and we do not want to compare the pointers themselves.
	 */
	inline const_iterator find(const boost::shared_ptr<T>& item) const {
		return std::find_if(data.begin(), data.end(), boost::bind(std::equal_to<T>(),_1,item));
	}

	/*******************************************************************************************/

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

	inline iterator rbegin() { return data.rbegin(); }
	inline const_iterator rbegin() const { return data.rbegin(); }

	inline iterator rend() { return data.rend(); }
	inline const_iterator rend() const { return data.rend(); }

	// Insertion and removal

	inline iterator insert(iterator pos, const boost::shared_ptr<T>& item) { return data.insert(pos, item); }

	/*******************************************************************************************/
	/**
	 * Inserts a default-constructed item at position pos
	 */
	inline iterator insert(iterator pos) {
		boost::shared_ptr<T> p(new T());
		return data.insert(pos, p);
	}

	/*******************************************************************************************/
	/**
	 * Inserts a given amount of item after position pos.
	 */
	inline void insert(iterator pos, size_type amount, const T& item) {
		for(std::size_t i=0; i<amount; i++) {
			boost::shared_ptr<T> p(new T(*item));
			 // Note that we re-calculate the iterator, as it is not clear whether it remains valid
			data.insert(data.begin()+pos, p);
		}
	}

	/*******************************************************************************************/

	// Adding to the  back of the vector
	inline void push_back(const boost::shared_ptr<T>& item){ data.push_back(item); }

	// Removal at a given position or in a range
	inline iterator erase(iterator pos) { return data.erase(pos); }
	inline iterator erase(iterator from, iterator to) { return data.erase(from,to); }

	// Removing an element from the end of the vector
	inline void pop_back(){ data.pop_back(); }

	/*******************************************************************************************/
	/**
	 * Resizing the vector. We initialize the smart pointers content with
	 * default-constructed Ts. This obviously assumes that T is
	 * default-constructible. This function does nothing
	 * if amount is the same as data.size().
	 *
	 * @param amount The new desired size of the vector
	 */
	inline void resize(size_type amount) {
		std::size_t dataSize = data.size();

		if(amount < dataSize)
			data.resize(amount);
		else {
			for(std::size_t i=dataSize; i<amount; i++) {
				boost::shared_ptr<T> p(new T());
				data.push_back(p);
			}
		}
	}

	/*******************************************************************************************/
	/**
	 * Resizing the vector, initialization with item. This function does nothing
	 * if amount is the same as data.size(). We assume in this function that
	 * T is copy-constructible.
	 *
	 * @param amount The new desired size of the vector
	 * @param item An item that should be used for initialization of new items, if any
	 */
	inline void resize(size_type amount, boost::shared_ptr<T> item) {
		std::size_t dataSize = data.size();

		if(amount < dataSize)
			data.resize(amount);
		else if(amount > dataSize) {
			for(std::size_t i=dataSize; i<amount; i++) {
				boost::shared_ptr<T> p(new T(*item));
				data.push_back(p);
			}
		}
	}

	/*******************************************************************************************/

	/** @brief Clearing the data vector */
	inline void clear() { data.clear(); }

	/*******************************************************************************************/
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
		std::size_t cpSize = cp.data.size();

		if(cpSize == localSize) { // The most likely case
			for(it=data.begin(), cp_it=cp.data.begin(); it!=data.end(), cp_it!=cp.data.end(); ++it, ++cp_it)
				(*it)->load(*cp_it);
		}
		else if(cpSize > localSize) {
			// First copy the initial elements
			for(it=data.begin(), cp_it=cp.data.begin(); it!=data.end(); ++it, ++cp_it)
				(*it)->load(*cp_it);

			// Then attach the remaining objects from cp
			for(cp_it=cp.data.begin()+localSize; cp_it != cp.data.end(); ++cp_it) {
				boost::shared_ptr<T> p(new T(**cp_it)); // **cp_it is of type T
				data.push_back(p);
			}
		}
		else if(cpSize < localSize) {
			// First get rid of surplus items
			data.resize(cpSize);

			// Then copy the elements
			for(it=data.begin(), cp_it=cp.data.begin(); it!=data.end(), cp_it!=cp.data.end(); ++it, ++cp_it)
				(*it)->load(*cp_it);
		}

		return cp;
	}

	/*******************************************************************************************/
	/////////////////////////////////////////////////////////////////////////////////////////////
	/*******************************************************************************************/

protected:
	std::vector<boost::shared_ptr<T> > data; ///< The main data set stored in this class.
};


/***********************************************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */

/**************************************************************************************************/
/**
 * @brief The content of the BOOST_SERIALIZATION_ASSUME_ABSTRACT(T) macro. Needed for Boost.Serialization
 */
namespace boost {
  namespace serialization {
    template<typename T>
    struct is_abstract<Gem::GenEvA::GParameterTCollectionT<T> > : boost::true_type {};
    template<typename T>
    struct is_abstract< const Gem::GenEvA::GParameterTCollectionT<T> > : boost::true_type {};
  }
}

/**************************************************************************************************/

#endif /* GPARAMETERTCOLLECTIONT_HPP_ */
