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
		GParameterTCollectionT<T>::load(&cp);
		return *this;
	}

	/*******************************************************************************************/
	/**
	 * Creates a deep clone of this object.
	 */
	virtual GObject* clone() {
		return new GParameterTCollectionT<T>(*this);
	}

	/*******************************************************************************************/
	/**
	 * Checks for equality with another GParameterTCollectionT<T> object
	 *
	 * @param  cp A constant reference to another GParameterTCollectionT object
	 * @return A boolean indicating whether both objects are equal
	 */
	bool operator==(const GParameterTCollectionT<T>& cp) const {
		return GParameterTCollectionT<T>::isEqualTo(cp);
	}

	/*******************************************************************************************/
	/**
	 * Checks for inequality with another GParameterTCollectionT<T> object
	 *
	 * @param  cp A constant reference to another GParameterTCollectionT object
	 * @return A boolean indicating whether both objects are inequal
	 */
	bool operator!=(const GParameterTCollectionT<T>& cp) const {
		return !GParameterTCollectionT<T>::isEqualTo(cp);
	}

	/*******************************************************************************************/
	/**
	 * Checks for equality with another GParameterTCollectionT<T> object. This function
	 * assumes that T has an isEqualTo function itself.
	 *
	 * @param  cp A constant reference to another GParameterTCollectionT<T> object
	 * @return A boolean indicating whether both objects are equal
	 */
	bool isEqualTo(const GParameterTCollectionT<T>& cp) const {
		// Check equality of the parent class
		if(!GParameterBase::isEqualTo(cp)) return false;

		// Then check our own, local data
		typename std::vector<boost::shared_ptr<T> >::const_iterator it;
		typename std::vector<boost::shared_ptr<T> >::const_iterator cp_it;
		for(it=data.begin(), cp_it=cp.data.begin(); it!=data.end(), cp_it!=cp.data.end(); ++it, ++cp_it) {
			if(!(*it)->isEqualTo(**cp_it)) return false;
		}

		return true;
	}

	/*******************************************************************************************/
	/**
	 * Checks for similarity with another GParameterTCollectionT<T> object.  This function
	 * assumes that T has an isSimilarTo function itself.
	 *
	 * @param  cp A constant reference to another GParameterTCollectionT<T> object
	 * @param limit A double value specifying the acceptable level of differences of floating point values
	 * @return A boolean indicating whether both objects are similar to each other
	 */
	bool isSimilarTo(const GParameterTCollectionT<T>& cp, const double& limit=0) const {
		// Check similarity of the parent class
		if(!GParameterBase::isSimilarTo(cp, limit)) return false;

		// Then check our own, local data
		typename std::vector<boost::shared_ptr<T> >::const_iterator it;
		typename std::vector<boost::shared_ptr<T> >::const_iterator cp_it;
		for(it=data.begin(), cp_it=cp.data.begin(); it!=data.end(), cp_it!=cp.data.end(); ++it, ++cp_it) {
			if(!(*it)->isSimilarTo(**cp_it, limit)) return false;
		}

		return true;
	}

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
	 * Counts the elements whose content is equal to item.
	 * Needs to be re-implemented here, as we are dealing with a collection of smart pointers
	 * and we do not want to compare the pointers themselves. Note that boost::bind will
	 * not accept the expression *_1, hence we need to employ a small helper function, which
	 * does the de-referencing for us. Also note that we assume that item has an operator== .
	 *
	 * @param item The item to be counted in the collection
	 */
	inline size_type count(const T& item) const {
		return std::count_if(data.begin(), data.end(),
				                          boost::bind(std::equal_to<T>(), item, boost::bind(Gem::Util::dereference<T>, _1)));
	}

	/*******************************************************************************************/
	/**
	 * Counts the elements whose content is equal to the content of item.
	 * Needs to be re-implemented here, as we are dealing with a collection of smart pointers
	 * and we do not want to compare the pointers themselves. Note that boost::bind will
	 * not accept the expression *_1, hence we need to employ a small helper function, which
	 * does the de-referencing for us. Also note that we assume that item has an operator== .
	 *
	 * @param item The item to be counted in the collection
	 */
	inline size_type count(const boost::shared_ptr<T>& item) const {
		if(!item) { // Check that item actually contains something useful
			std::ostringstream error;
			error << "In GParameterTCollectionT<T>::count(item): Error!"
				     << "Tried to count an empty smart pointer." << std::endl;

			throw(Gem::GenEvA::geneva_error_condition(error.str()));
		}

		return std::count_if(data.begin(), data.end(),
				                          boost::bind(std::equal_to<T>(), boost::bind(Gem::Util::dereference<T>, item),
				                        		                                                boost::bind(Gem::Util::dereference<T>, _1)));
	}

	/*******************************************************************************************/
	/**
	 * Searches for item in the entire range of the vector. Needs to be
	 * re-implemented here, as we are dealing with a collection of smart pointers
	 * and we do not want to compare the pointers themselves.
	 */
	inline const_iterator find(const T& item) const {
		return std::find_if(data.begin(), data.end(),
				                       boost::bind(std::equal_to<T>(),item, boost::bind(Gem::Util::dereference<T>, _1)));
	}

	/*******************************************************************************************/
	/**
	 * Searches for the content of item in the entire range of the vector. Needs to be
	 * re-implemented here, as we are dealing with a collection of smart pointers
	 * and we do not want to compare the pointers themselves.
	 */
	inline const_iterator find(const boost::shared_ptr<T>& item) const {
		if(!item) { // Check that item actually contains something useful
			std::ostringstream error;
			error << "In GParameterTCollectionT<T>::find(item): Error!"
				     << "Tried to find an empty smart pointer." << std::endl;

			throw(Gem::GenEvA::geneva_error_condition(error.str()));
		}

		return std::find_if(data.begin(), data.end(),
				                       boost::bind(std::equal_to<T>(), boost::bind(Gem::Util::dereference<T>, item),
                                                                                             boost::bind(Gem::Util::dereference<T>, _1)));
	}

	/*******************************************************************************************/
	// Comparison

	/**
	 * operator==, modified to check the content of the smart pointers
	 */
	bool operator==(const std::vector<boost::shared_ptr<T> >& cp_data) {
		// Check sizes
		if(data.size() != cp_data.size()) return false;

		// Check the content
		typename std::vector<boost::shared_ptr<T> >::const_iterator cp_it;
		typename std::vector<boost::shared_ptr<T> >::iterator it;
		for(cp_it=cp_data.begin(), it=data.begin(); cp_it != cp_data.end(), it!=data.end(); ++cp_it, ++it) {
			if(!(*it)->isEqualTo(**cp_it)) return false;
		}

		return true;
	}

	/**
	 * operator!=, modified to check the content of the smart pointers
	 */
	bool operator!=(const std::vector<boost::shared_ptr<T> >& cp_data) {
		return !operator==(cp_data);
	}

	/*******************************************************************************************/

	// Modifying functions
	inline void swap(std::vector<boost::shared_ptr<T> >& cont) { data.swap(cont); }

	// Modifying functions
	inline void swap(GParameterTCollectionT<T>& cp) { data.swap(cp.data); }

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

	// Insertion and removal

	/*******************************************************************************************/
	/**
	 * Inserts a given item at position pos. Checks whether the item actually points
	 * somewhere.
	 */
	inline iterator insert(iterator pos, const T& item) {
		boost::shared_ptr<T> item_ptr(new T(item));
		return data.insert(pos, item_ptr);
	}

	/*******************************************************************************************/
	/**
	 * Inserts a given item at position pos. Checks whether the item actually points
	 * somewhere.
	 */
	inline iterator insert(iterator pos, const boost::shared_ptr<T>& item) {
		if(!item) { // Check that item actually contains something useful
			std::ostringstream error;
			error << "In GParameterTCollectionT<T>::insert(pos, item): Error!"
				     << "Tried to insert an empty smart pointer." << std::endl;

			throw(Gem::GenEvA::geneva_error_condition(error.str()));
		}

		return data.insert(pos, item);
	}

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
	 * Inserts a given amount of items after position pos.
	 */
	inline void insert(iterator pos, size_type amount, const T& item) {
		std::size_t iterator_pos = pos - data.begin();
		for(std::size_t i=0; i<amount; i++) {
			boost::shared_ptr<T> p(new T(item));
			 // Note that we re-calculate the iterator, as it is not clear whether it remains valid
			data.insert(data.begin() + iterator_pos, p);
		}
	}

	/*******************************************************************************************/
	/**
	 * Inserts a given amount of items after position pos.
	 */
	inline void insert(iterator pos, size_type amount, const boost::shared_ptr<T>& item_ptr) {
		if(!item_ptr) { // Check that item actually contains something useful
			std::ostringstream error;
			error << "In GParameterTCollectionT<T>::insert(pos, amount, item): Error!"
				     << "Tried to insert an empty smart pointer." << std::endl;

			throw(Gem::GenEvA::geneva_error_condition(error.str()));
		}

		std::size_t iterator_pos = pos - data.begin();
		for(std::size_t i=0; i<amount; i++) {
			 // Note that we re-calculate the iterator, as it is not clear whether it remains valid
			data.insert(data.begin() + iterator_pos,  item_ptr);
		}
	}

	/*******************************************************************************************/
	// Adding shared_ptr objects to the  back of the vector
	inline void push_back(const boost::shared_ptr<T>& item){
		if(!item) { // Check that item actually contains something useful
			std::ostringstream error;
			error << "In GParameterTCollectionT<T>::push_back(item): Error!"
				     << "Tried to insert an empty smart pointer." << std::endl;

			throw(Gem::GenEvA::geneva_error_condition(error.str()));
		}

		data.push_back(item);
	}

	/*******************************************************************************************/
	// Adding simple items to the  back of the vector
	inline void push_back(const T& item){
		boost::shared_ptr<T> item_ptr(new T(item));
		data.push_back(item_ptr);
	}

	/*******************************************************************************************/

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
	inline void resize(size_type amount, const boost::shared_ptr<T>& item) {
		std::size_t dataSize = data.size();

		if(amount < dataSize)
			data.resize(amount);
		else if(amount > dataSize) {
			// Check that item is not empty
			if(!item) { // Check that item actually contains something useful
				std::ostringstream error;
				error << "In GParameterTCollectionT<T>::resize(amount, item): Error!"
					     << "Tried to insert an empty smart pointer." << std::endl;

				throw(Gem::GenEvA::geneva_error_condition(error.str()));
			}

			for(std::size_t i=dataSize; i<amount; i++) {
				data.push_back(item);
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
	inline void resize(size_type amount, const T& item) {
		std::size_t dataSize = data.size();

		if(amount < dataSize)
			data.resize(amount);
		else if(amount > dataSize) {
			for(std::size_t i=dataSize; i<amount; i++) {
				boost::shared_ptr<T> p(new T(item));
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
		std::size_t cpSize = cp.size();

		if(cpSize == localSize) { // The most likely case
			for(it=data.begin(), cp_it=cp.begin(); it!=data.end(), cp_it!=cp.end(); ++it, ++cp_it) **it = **cp_it;
		}
		else if(cpSize > localSize) {
			// First copy the initial elements
			for(it=data.begin(), cp_it=cp.begin(); it!=data.end(); ++it, ++cp_it) **it = **cp_it;

			// Then attach the remaining objects from cp
			for(cp_it=cp.begin()+localSize; cp_it != cp.end(); ++cp_it) {
				boost::shared_ptr<T> p(new T(**cp_it)); // **cp_it is of type T
				data.push_back(p);
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

#endif /* GPARAMETERTCOLLECTIONT_HPP_ */
