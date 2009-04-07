/**
 * @file GParameterCollectionT.hpp
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
#include <cmath>

// Boost header files go here

#include <boost/version.hpp>
#include "GGlobalDefines.hpp"
#if BOOST_VERSION < ALLOWED_BOOST_VERSION
#error "Error: Boost has incorrect version !"
#endif /* BOOST_VERSION */

#include <boost/cstdint.hpp>

#ifndef GPARAMETERCOLLECTIONT_HPP_
#define GPARAMETERCOLLECTIONT_HPP_

// GenEvA header files go here
#include "GParameterBaseWithAdaptorsT.hpp"
#include "GObject.hpp"

namespace Gem {
namespace GenEvA {

/***********************************************************************************************/
/**
 * A class holding a collection of mutable parameters - usually just an atomic value (double,
 * long, bool, ...).
 */
template<typename T>
class GParameterCollectionT
	:public GParameterBaseWithAdaptorsT<T>
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int version) {
		using boost::serialization::make_nvp;
		ar & make_nvp("GParameterBaseWithAdaptorsT_T", boost::serialization::base_object<GParameterBaseWithAdaptorsT<T> >(*this));
		ar & make_nvp("data_T", data);
	}
	///////////////////////////////////////////////////////////////////////

public:
	/*******************************************************************************************/
	/**
	 * The default constructor
	 */
	GParameterCollectionT() :
		GParameterBaseWithAdaptorsT<T> ()
	{ /* nothing */ }

	/*******************************************************************************************/
	/**
	 * The copy constructor
	 *
	 * @param cp A copy of another GParameterCollectionT<T> object
	 */
	GParameterCollectionT(const GParameterCollectionT<T>& cp) :
		GParameterBaseWithAdaptorsT<T> (cp) {
		typename std::vector<T>::const_iterator cit;
		for (cit = cp.data.begin(); cit != cp.data.end(); ++cit) data.push_back(*cit);
	}

	/*******************************************************************************************/
	/**
	 * The standard destructor
	 */
	virtual ~GParameterCollectionT()
	{ /* nothing */ }

	/*******************************************************************************************/
	/**
	 * A standard assignment operator.
	 *
	 * @param cp A copy of another GParameterCollectionT object
	 * @return A constant reference to this object
	 */
	const GParameterCollectionT<T>& operator=(const GParameterCollectionT<T>& cp)
	{
		GParameterCollectionT<T>::load(cp);
		return *this;
	}

	/*******************************************************************************************/
	/**
	 * Checks for equality with another GParameterCollectionT<T> object
	 *
	 * @param  cp A constant reference to another GParameterCollectionT<T> object
	 * @return A boolean indicating whether both objects are equal
	 */
	bool operator==(const GParameterCollectionT<T>& cp) const {
		return GParameterCollectionT<T>::isEqualTo(cp);
	}

	/*******************************************************************************************/
	/**
	 * Checks for inequality with another GParameterCollectionT<T> object
	 *
	 * @param  cp A constant reference to another GParameterCollectionT<T> object
	 * @return A boolean indicating whether both objects are inequal
	 */
	bool operator!=(const GParameterCollectionT<T>& cp) const {
		return !GParameterCollectionT<T>::isEqualTo(cp);
	}

	/*******************************************************************************************/
	/**
	 * Checks for equality with another GParameterCollectionT<T> object
	 *
	 * @param  cp A constant reference to another GParameterCollectionT<T> object
	 * @return A boolean indicating whether both objects are equal
	 */
	virtual bool isEqualTo(const GParameterCollectionT<T>& cp) const {
		// Check equality of the parent class
		if(!GParameterBaseWithAdaptorsT<T>::isEqualTo(cp)) return false;

		typename std::vector<T>::const_iterator it;
		typename std::vector<T>::const_iterator cp_it;
		for(it=data.begin(), cp_it=cp.data.begin(); it!=data.end(), cp_it!=cp.data.end(); ++it, ++cp_it) {
			if(*it != *cp_it) return false;
		}

		return true;
	}

	/*******************************************************************************************/
	/**
	 * Checks for similarity with another GParameterCollectionT<T> object. We need to
	 * create a specialization for typeof(T) == typof(double), as we need to check for
	 * similarity in this case. For all other types we currently just check for equality.
	 *
	 * @param  cp A constant reference to another GParameterCollectionT<T> object
	 * @param limit A double value specifying the acceptable level of differences of floating point values
	 * @return A boolean indicating whether both objects are similar to each other
	 */
	virtual bool isSimilarTo(const GParameterCollectionT<T>& cp, const double& limit=0) const {
		// Check similarity of the parent class
		if(!GParameterBaseWithAdaptorsT<T>::isSimilarTo(cp, limit)) return false;

		typename std::vector<T>::const_iterator it;
		typename std::vector<T>::const_iterator cp_it;
		for(it=data.begin(), cp_it=cp.data.begin(); it!=data.end(), cp_it!=cp.data.end(); ++it, ++cp_it) {
			if(*it != *cp_it) return false;
		}

		return true;
	}

	/*******************************************************************************************/
	/**
	 * Creates a deep clone of this object. Purely virtual, so this class cannot be instantiated.
	 */
	virtual GObject* clone() const = 0;

	/*******************************************************************************************/
	/**
	 * Loads the data of another GParameterCollectionT<T> object, camouflaged as a GObject
	 *
	 * @param cp A copy of another GParameterCollectionT<T> object, camouflaged as a GObject
	 */
	virtual void load(const GObject* cp) {
		// Convert cp into local format
		const GParameterCollectionT<T> *gpct = this->conversion_cast(cp, this);

		// Load our parent class'es data ...
		GParameterBaseWithAdaptorsT<T>::load(cp);

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
		if (GParameterBaseWithAdaptorsT<T>::numberOfAdaptors() == 1) {
			GParameterBaseWithAdaptorsT<T>::applyFirstAdaptor(data);
		} else {
			GParameterBaseWithAdaptorsT<T>::applyAllAdaptors(data);
		}
	}

	/*******************************************************************************************/
	/**
	 * Creates a copy of the data vector. Note that this function assumes that the parameters
	 * stored in this class are basic values, such as doubles. If T is a class type, then a suitable
	 * operator= needs to be defined.
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

	// Assignment of a std::vector<T>
	inline const std::vector<T>& operator=(const std::vector<T>& cp) { data = cp; return cp;}

	/*******************************************************************************************/
	/////////////////////////////////////////////////////////////////////////////////////////////
	/*******************************************************************************************/

protected:
	/*******************************************************************************************/
	/**
	 * The main data set stored in this class. This class was derived from std::vector<T> in older
	 * versions of the GenEvA library. However, we want to avoid multiple inheritance in order to
	 * to allow an easier implementation of this library in other languages, such as C# or Java. And
	 * std::vector has a non-virtual destructor. Hence deriving from it is a) bad style and b) dangerous.
	 * Just like in the older setting, however, access to the data shall not be obstructed in any way.
	 * As a consequence, most standard algorithms and member functions of a std::vector can be used
	 * out of the box with this class, which also acts as a wrapper around the std::vector<T>. Note
	 * that it is assumed here that T is a basic type or behaves like one. E.g., making T a
	 * boost::shared_ptr<> of something would be a bad idea.
	 */
	std::vector<T> data;
	/*******************************************************************************************/
};

/*********************************************************************************************/
/** @brief A specialization for typeof(T) == typeof(double) */
template <> bool GParameterCollectionT<double>::isSimilarTo(const GParameterCollectionT<double>&, const double&) const;

/*********************************************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */

/**************************************************************************************************/
/**
 * @brief The content of the BOOST_SERIALIZATION_ASSUME_ABSTRACT(T) macro. Needed for Boost.Serialization
 */
namespace boost {
  namespace serialization {
    template<typename T>
    struct is_abstract<Gem::GenEvA::GParameterCollectionT<T> > : boost::true_type {};
    template<typename T>
    struct is_abstract< const Gem::GenEvA::GParameterCollectionT<T> > : boost::true_type {};
  }
}

/**************************************************************************************************/

#endif /* GPARAMETERCOLLECTIONT_HPP_ */
