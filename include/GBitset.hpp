/**
 * @file GBitset.hpp
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

// Boost header files go here

#include <boost/version.hpp>
#include "GGlobalDefines.hpp"
#if BOOST_VERSION < ALLOWED_BOOST_VERSION
#error "Error: Boost has incorrect version !"
#endif /* BOOST_VERSION */

#include <boost/cstdint.hpp>

#ifndef GBITSET_HPP_
#define GBITSET_HPP_

// GenEvA header files go here
#include "GParameterBaseWithAdaptorsT.hpp"
#include "GObject.hpp"

namespace Gem {
namespace GenEvA {

/***********************************************************************************************/
/**
 * A wrapper for a std::bitset
 */
template <class N>
class GBitset
	:public GParameterBaseWithAdaptorsT<bool>
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int version) {
		using boost::serialization::make_nvp;
		ar & make_nvp("GParameterBaseWithAdaptorsT_T", boost::serialization::base_object<GParameterBaseWithAdaptorsT<bool> >(*this));
		ar & make_nvp("data_T", data);
	}
	///////////////////////////////////////////////////////////////////////

public:
	/*******************************************************************************************/
	/**
	 * The default constructor
	 */
	GBitset() :
		GParameterBaseWithAdaptorsT<bool> () { /* nothing */
	}

	/*******************************************************************************************/
	/**
	 * The copy constructor
	 *
	 * @param cp A copy of another GBitset<N> object
	 */
	GBitset(const GBitset<N>& cp) :
		GParameterBaseWithAdaptorsT<bool> (cp) {
		typename std::vector<T>::const_iterator cit;
		for (cit = cp.data.begin(); cit != cp.data.end(); ++cit) data.push_back(*cit);
	}

	/*******************************************************************************************/
	/**
	 * The standard destructor
	 */
	virtual ~GBitset()
	{ /* nothing */ }

	/*******************************************************************************************/
	/**
	 * A standard assignment operator.
	 *
	 * @param cp A copy of another GBitset<N> object
	 * @return A constant reference to this object
	 */
	const GBitset<N>& operator=(const GBitset<N>& cp)
	{
		GBitset<N>::load(cp);
		return *this;
	}

	/*******************************************************************************************/
	/**
	 * Creates a deep clone of this object. Purely virtual.
	 */
	virtual GObject* clone() = 0;

	/*******************************************************************************************/
	/**
	 * Loads the data of another GBitset<N> object, camouflaged as a GObject
	 *
	 * @param cp A copy of another GBitset<N> object, camouflaged as a GObject
	 */
	virtual void load(const GObject* cp) {
		// Convert cp into local format
		const GBitset<N> *gpct = this->conversion_cast(cp, this);

		// Load our parent class'es data ...
		GParameterBaseWithAdaptorsT<bool>::load(cp);

		// ... and then our own data
		typename std::vector<T>::iterator it;
		typename std::vector<T>::const_iterator cit;

		for (cit = gpct->data.begin(), it = data.begin();
		     cit != gpct->data.end(), it!= data.end();
		     ++it, ++cit)
		{ (*it) = (*cit); } // Note that this assignment assumes that T has an operator=

		std::size_t gpct_sz = gpct->data.size(), this_sz = data.size();
		if (gpct_sz == this_sz)	return; // Likely the most probable case
		else if (gpct_sz > this_sz) {
			for (cit = gpct->data.begin() + this_sz; cit != gpct->data.end(); ++cit) {
				data.push_back(*cit);
			}
		} else if (this_sz > gpct_sz)
			data.resize(gpct_sz); // get rid of the surplus elements
	}

	/*******************************************************************************************/
	/**
	 * Allows to mutate the values stored in this class. If more than one adaptor was registered,
	 * all will be applied to the value. applyFirstAdaptor and applyAllAdaptors expects a reference
	 * to a vector<T>. As we are derived from this class, we can just pass a reference to ourselves
	 * to the functions.
	 */
	virtual void mutate() {
		if (GParameterBaseWithAdaptorsT<bool>::numberOfAdaptors() == 1) {
			GParameterBaseWithAdaptorsT<bool>::applyFirstAdaptor(data);
		} else {
			GParameterBaseWithAdaptorsT<bool>::applyAllAdaptors(data);
		}
	}

	/*******************************************************************************************/
	//////////////////////// std::vector<T> interface (incomplete) //////////////////////////////
	/*******************************************************************************************/
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

	// Non modifying access
	inline size_type size() const { return data.size(); }
	inline bool empty() const { return data.empty(); }
	inline size_type max_size() const { return data.max_size(); }

	inline size_type capacity() const { return data.capacity(); }
	inline void reserve(size_type amount) { data.reserve(amount); }

	inline size_type count(const T& item) const { return data.count(item); }

	inline iterator find(const T& item) { return data.find(item); }
	inline const_iterator find(const T& item) const { return data.find(item); }

	// Modifying functions
	inline void swap(std::vector<T>& cont) { data.swap(cont); }

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
	inline iterator insert(iterator pos) { return data.insert(pos); }
	inline iterator insert(iterator pos, const T& item) { return data.insert(pos,item); }
	inline void insert(iterator pos, size_type amount, const T& item) { data.insert(pos,amount,item); }

	// Adding to the  back of the vector
	inline void push_back(const T& item){ data.push_back(item); }

	// Removal at a given position or in a range
	inline iterator erase(iterator pos) { return data.erase(pos); }
	inline iterator erase(iterator from, iterator to) { return data.erase(from,to); }

	// Removing an element from the end of the vector
	inline void pop_back(){ data.pop_back(); }

	// Resizing the vector
	inline void resize(size_type amount) { data.resize(amount); }
	inline void resize(size_type amount, T item) { data.resize(amount, item); }

	// Resizing to size 0
	inline void clear() { data.clear(); }

	/*******************************************************************************************/
	/////////////////////////////////////////////////////////////////////////////////////////////
	/*******************************************************************************************/

protected:
	/*******************************************************************************************/
	/**
	 * The main data set stored in this class
	 */
	std::bitset<N> data;
	/*******************************************************************************************/
};

} /* namespace GenEvA */
} /* namespace Gem */

/**************************************************************************************************/
/**
 * @brief The content of the BOOST_SERIALIZATION_ASSUME_ABSTRACT(T) macro. Needed for Boost.Serialization
 */
namespace boost {
  namespace serialization {
    template<class N>
    struct is_abstract<Gem::GenEvA::GBitset<N> > : boost::true_type {};
    template<class N>
    struct is_abstract< const Gem::GenEvA::GBitset<N> > : boost::true_type {};
  }
}

/**************************************************************************************************/

#endif /* GBITSET_HPP_ */
