/**
 * @file GStdSimpleVectorInterfaceT.hpp
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
#include <cmath>
#include <algorithm>

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

#ifndef GSTDSIMPLEVECTORINTERFACET_HPP_
#define GSTDSIMPLEVECTORINTERFACET_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif


#include "GObject.hpp"
#include "GHelperFunctionsT.hpp"

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

      ar & BOOST_SERIALIZATION_NVP(data);
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
	GStdSimpleVectorInterfaceT(const GStdSimpleVectorInterfaceT<T>& cp)
		: data(cp.data)
	{ /* nothing */ }

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
	bool operator==(const GStdSimpleVectorInterfaceT<T>& cp) const {
		return this->checkIsEqualTo(cp, boost::logic::indeterminate);
	}

	/*****************************************************************************/
	/**
	 * Checks inequality with another GStdSimpleVectorInterfaceT<T> object
	 */
	bool operator!=(const GStdSimpleVectorInterfaceT<T>& cp) const {
		return ! this->checkIsEqualTo(cp, boost::logic::indeterminate);
	}

	/*****************************************************************************/
	/**
	 * operator==
	 */
	bool operator==(const std::vector<T>& cp_data) const {
		return this->checkIsEqualTo(cp_data, boost::logic::indeterminate);
	}

	/*****************************************************************************/
	/**
	 * operator!=
	 */
	bool operator!=(const std::vector<T>& cp_data)  const {
		return ! this->checkIsEqualTo(cp_data, boost::logic::indeterminate);
	}

	/*****************************************************************************/
	/**
	 * Checks for equality with another GStdSimpleVectorInterfaceT<T> object
	 */
	bool checkIsEqualTo(const GStdSimpleVectorInterfaceT<T>& cp, const boost::logic::tribool& expected = boost::logic::indeterminate) const {
		return this->checkIsEqualTo(cp.data, expected);
	}

	/*****************************************************************************/
	/**
	 * Checks for equality with a std::vector<T> object
	 */
	bool checkIsEqualTo(const std::vector<T>& cp_data, const boost::logic::tribool& expected = boost::logic::indeterminate) const{
		using namespace Gem::Util;

		std::string className = std::string("GStdSimpleVectorInterfaceT<") + typeid(T).name() + ">";
		if(checkForInequality(className, this->data, cp_data,"data", "cp_data", expected)) return false;

		return true;
	}

	/*****************************************************************************/
	/**
	 * Checks for similarity with another GStdSimpleVectorInterfaceT<T> object.
	 */
	bool checkIsSimilarTo(const GStdSimpleVectorInterfaceT<T>& cp, const double& limit, const boost::logic::tribool& expected = boost::logic::indeterminate) const {
		return this->checkIsSimilarTo(cp.data, limit, expected);
	}

	/*****************************************************************************/
	/**
	 * Checks for similarity with another std::vector<T>  object. A specialized
	 * version of this function exists for typeof(T) == typeof(double)
	 */
	bool checkIsSimilarTo(const std::vector<T>& cp_data, const double& limit, const boost::logic::tribool& expected = boost::logic::indeterminate) const {
		using namespace Gem::Util;

		std::string className = std::string("GStdSimpleVectorInterfaceT<") + typeid(T).name() + ">";
		if(checkForDissimilarity(className, this->data, cp_data, limit, "data", "cp_data", expected)) return false;

		return true;
	}

	/*****************************************************************************/
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
	boost::optional<std::string> checkRelationshipWith(const std::vector<T>& cp_data,
			const Gem::Util::expectation& e,
			const double& limit,
			const std::string& caller,
			const std::string& y_name,
			const bool& withMessages) const
	{
	    using namespace Gem::Util;
	    using namespace Gem::Util::POD;

		// Will hold possible deviations from the expectation, including explanations
	    std::vector<boost::optional<std::string> > deviations;

	    // Assemble a suitable caller string
	    std::string className = std::string("GStdSimpleVectorInterfaceT<") + typeid(T).name() + ">";

		// No parent class to check ...

		// Check local data
		deviations.push_back(checkExpectation(withMessages, className , this->data, cp_data, "data", "cp_data", e , limit));

		return evaluateDiscrepancies(className, caller, deviations, e);
	}

	/*****************************************************************************/
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
	boost::optional<std::string> checkRelationshipWith(const GStdSimpleVectorInterfaceT<T>& cp,
			const Gem::Util::expectation& e,
			const double& limit,
			const std::string& caller,
			const std::string& y_name,
			const bool& withMessages) const
	{
	    using namespace Gem::Util;
	    using namespace Gem::Util::POD;

		// Will hold possible deviations from the expectation, including explanations
	    std::vector<boost::optional<std::string> > deviations;

	    // Assemble a suitable caller string
	    std::string className = std::string("GStdSimpleVectorInterfaceT<") + typeid(T).name() + ">";

		// No parent class to check ...

		// Check local data
		deviations.push_back(checkExpectation(withMessages, className , this->data, cp.data, "data", "cp.data", e , limit));

		return evaluateDiscrepancies(className, caller, deviations, e);
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
	size_type size() const { return data.size(); }
	bool empty() const { return data.empty(); }
	size_type max_size() const { return data.max_size(); }

	size_type capacity() const { return data.capacity(); }
	void reserve(size_type amount) { data.reserve(amount); }

	/*****************************************************************************/
	/**
	 * Counts the elements whose content is equal to item.
	*
	 * @param item The item to be counted in the collection
	 * @return The number of items found
	 */
	size_type count(const T& item) const { return std::count(data.begin(), data.end(), item); }

	/*****************************************************************************/
	/**
	 * Searches for item in the entire range of the vector. Needs to be
	 * re-implemented here, as we are dealing with a collection of smart pointers
	 * and we do not want to compare the pointers themselves.
	 */
	const_iterator find(const T& item) const {
		return std::find(data.begin(), data.end(), item);
	}

	/*****************************************************************************/

	// Modifying functions
	void swap(std::vector<T>& cont) { std::swap(data, cont); }

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

	/*****************************************************************************/
	// Insertion and removal

	/**
	 * Inserts a given item at position pos. Checks whether the item actually points
	 * somewhere.
	 */
	iterator insert(iterator pos, const T& item) { return data.insert(pos, item); }

	/*****************************************************************************/
	/**
	 * Inserts a given amount of items after position pos.
	 */
	void insert(iterator pos, size_type amount, const T& item) {	return data.insert(pos,amount,item); }

	/*****************************************************************************/
	// Adding simple items to the  back of the vector
	void push_back(const T& item){ data.push_back(item); }

	/*****************************************************************************/

	// Removal at a given position or in a range
	iterator erase(iterator pos) { return data.erase(pos); }
	iterator erase(iterator from, iterator to) { return data.erase(from, to); }

	// Removing an element from the end of the vector
	void pop_back(){ data.pop_back(); }

	/*****************************************************************************/
	/**
	 * Resizing the vector, initialization with item. This function does nothing
	 * if amount is the same as data.size(). We assume in this function that
	 * T is copy-constructible.
	 *
	 * @param amount The new desired size of the vector
	 * @param item An item that should be used for initialization of new items, if any
	 */
	void resize(size_type amount, const T& item) { data.resize(amount, item);	}

	/*****************************************************************************/
	/** @brief Clearing the data vector */
	void clear() { data.clear(); }

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
	const std::vector<T>& operator=(const std::vector<T>& cp) {
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
	void getDataCopy(std::vector<T>& cp) const {	cp=data; 	}

	/*****************************************************************************/

protected:
	std::vector<T> data;

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
    struct is_abstract<Gem::GenEvA::GStdSimpleVectorInterfaceT<T> > : public boost::true_type {};
    template<typename T>
    struct is_abstract< const Gem::GenEvA::GStdSimpleVectorInterfaceT<T> > : public boost::true_type {};
  }
}

/**************************************************************************************************/

#endif /* GSTDSIMPLEVECTORINTERFACET_HPP_ */
