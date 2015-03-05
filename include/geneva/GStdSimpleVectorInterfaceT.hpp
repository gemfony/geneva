/**
 * @file GStdSimpleVectorInterfaceT.hpp
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
#include <cmath>
#include <algorithm>

// Boost header files go here

#include <boost/cstdint.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/logic/tribool.hpp>
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

// Geneva headers go here

#include "common/GExceptions.hpp"
#include "common/GHelperFunctionsT.hpp"
#include "common/GLogger.hpp"
#include "common/GPODExpectationChecksT.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
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
    G_API_GENEVA void serialize(Archive & ar, const unsigned int){
      using boost::serialization::make_nvp;

      ar
      & BOOST_SERIALIZATION_NVP(data);
    }
    ///////////////////////////////////////////////////////////////////////

public:
   /***************************************************************************/
	/**
	 * The default constructor
	 */
    G_API_GENEVA GStdSimpleVectorInterfaceT() { /* nothing */ }

	/***************************************************************************/
	/**
	 * Initialization with a number of items of defined value
	 *
	 * @param nval The number of items to be added to the collection
	 * @param val  The value to be assigned to each position
	 */
    G_API_GENEVA GStdSimpleVectorInterfaceT(const std::size_t& nval, const T& val)
		: data(nval, val)
	{ /* nothing */ }

   /***************************************************************************/
	/**
	 * Copy construction
	 *
	 * @param cp A constant reference to another GStdSimpleVectorInterfaceT object
	 */
    G_API_GENEVA GStdSimpleVectorInterfaceT(const GStdSimpleVectorInterfaceT<T>& cp)
		: data(cp.data)
	{ /* nothing */ }

   /***************************************************************************/
	/**
	 * The destructor.
	 */
	virtual G_API_GENEVA ~GStdSimpleVectorInterfaceT() { data.clear();	}

   /***************************************************************************/
	/**
	 * Assginment operator
	 */
	G_API_GENEVA const GStdSimpleVectorInterfaceT& operator=(const GStdSimpleVectorInterfaceT<T>& cp) {
		this->operator=(cp.data);
		return cp;
	}

   /***************************************************************************/
	/**
	 * Checks whether a given expectation for the relationship between this object and another object
	 * is fulfilled.
	 *
	 * @param cp_data A vector of data to compared with our local data
	 * @param e The expected outcome of the comparison
	 * @param limit The maximum deviation for floating point values (important for similarity checks)
	 * @param caller An identifier for the calling entity
	 * @param y_name An identifier for the object that should be compared to this one
	 * @param withMessages Whether or not information should be emitted in case of deviations from the expected outcome
	 * @return A boost::optional<std::string> object that holds a descriptive string if expectations were not met
	 */
	virtual G_API_GENEVA boost::optional<std::string> checkRelationshipWith_base(
      const std::vector<T>& cp_data
      , const Gem::Common::expectation& e
      , const double& limit
      , const std::string& caller
      , const std::string& y_name
      , const bool& withMessages
	) const BASE {
	    using namespace Gem::Common;

		// Will hold possible deviations from the expectation, including explanations
	    std::vector<boost::optional<std::string> > deviations;

	    // Assemble a suitable caller string
	    std::string className = std::string("GStdSimpleVectorInterfaceT<") + typeid(T).name() + ">";

		// No parent class to check ...

		// Check local data
		deviations.push_back(checkExpectation(withMessages, className , this->data, cp_data, "data", "cp_data", e , limit));

		return evaluateDiscrepancies(className, caller, deviations, e);
	}

   /***************************************************************************/
	/**
	 * Checks whether a given expectation for the relationship between this object and another object
	 * is fulfilled.
	 *
	 * @param cp A constant reference to another object of type GStdSimpleVectorInterfaceT<T>
	 * @param e The expected outcome of the comparison
	 * @param limit The maximum deviation for floating point values (important for similarity checks)
	 * @param caller An identifier for the calling entity
	 * @param y_name An identifier for the object that should be compared to this one
	 * @param withMessages Whether or not information should be emitted in case of deviations from the expected outcome
	 * @return A boost::optional<std::string> object that holds a descriptive string if expectations were not met
	 */
	virtual G_API_GENEVA boost::optional<std::string> checkRelationshipWith_base(
      const GStdSimpleVectorInterfaceT<T>& cp
      , const Gem::Common::expectation& e
      , const double& limit
      , const std::string& caller
      , const std::string& y_name
      , const bool& withMessages
	) const BASE {
	    using namespace Gem::Common;

		// Will hold possible deviations from the expectation, including explanations
	    std::vector<boost::optional<std::string> > deviations;

	    // Assemble a suitable caller string
	    std::string className = std::string("GStdSimpleVectorInterfaceT<") + typeid(T).name() + ">";

		// No parent class to check ...

		// Check local data
		deviations.push_back(checkExpectation(withMessages, className , this->data, cp.data, "data", "cp.data", e , limit));

		return evaluateDiscrepancies(className, caller, deviations, e);
	}

   /***************************************************************************/
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

   /***************************************************************************/
	// Non modifying access
	G_API_GENEVA size_type size() const { return data.size(); } // Used/tested in GDoubleCollection::fillWithData()
	G_API_GENEVA bool empty() const { return data.empty(); } // Used/tested in GDoubleCollection::fillWithData()
	G_API_GENEVA size_type max_size() const { return data.max_size(); } // Used/tested in GDoubleCollection::specificTestsNoFailureExpected_GUnitTests()

	G_API_GENEVA size_type capacity() const { return data.capacity(); } // Used/tested in GDoubleCollection::specificTestsNoFailureExpected_GUnitTests()
	G_API_GENEVA void reserve(size_type amount) { data.reserve(amount); } // Used/tested in GDoubleCollection::specificTestsNoFailureExpected_GUnitTests()

   /***************************************************************************/
	/**
	 * Counts the elements whose content is equal to item.
	*
	 * @param item The item to be counted in the collection
	 * @return The number of items found
	 */
	G_API_GENEVA size_type count(const T& item) const { return std::count(data.begin(), data.end(), item); }

	/* ----------------------------------------------------------------------------
	 * Tested in GDoubleCollection::specificTestsNoFailureExpected_GUnitTests()
	 * ----------------------------------------------------------------------------
	 */

   /***************************************************************************/
	/**
	 * Searches for item in the entire range of the vector. Needs to be
	 * re-implemented here, as we are dealing with a collection of smart pointers
	 * and we do not want to compare the pointers themselves.
	 */
	G_API_GENEVA const_iterator find(const T& item) const {
		return std::find(data.begin(), data.end(), item);
	}

	/* ----------------------------------------------------------------------------
	 * Tested in GDoubleCollection::specificTestsNoFailureExpected_GUnitTests()
	 * ----------------------------------------------------------------------------
	 */

   /***************************************************************************/

	// Modifying functions
	G_API_GENEVA void swap(std::vector<T>& cont) { std::swap(data, cont); } // untested (likely irrelevant)

	// Access to elements (unchecked / checked)
	G_API_GENEVA reference operator[](std::size_t pos) { return data[pos]; } // Tested in GDoubleCollection::specificTestsNoFailureExpected_GUnitTests()
	G_API_GENEVA const_reference operator[](std::size_t pos) const { return data[pos]; }

	G_API_GENEVA reference at(std::size_t pos) { return data.at(pos); } // Tested in GDoubleCollection::specificTestsNoFailureExpected_GUnitTests()
	G_API_GENEVA const_reference at(std::size_t pos) const { return data.at(pos); }

	G_API_GENEVA reference front() { return data.front(); } // Tested in GDoubleCollection::specificTestsNoFailureExpected_GUnitTests()
	G_API_GENEVA const_reference front() const { return data.front(); }

	G_API_GENEVA reference back() { return data.back(); } // Tested in GDoubleCollection::specificTestsNoFailureExpected_GUnitTests()
	G_API_GENEVA const_reference back() const { return data.back(); }

	// Iterators
	G_API_GENEVA iterator begin() { return data.begin(); }
	G_API_GENEVA const_iterator begin() const { return data.begin(); } // Tested in GDoubleCollection::specificTestsNoFailureExpected_GUnitTests()

	G_API_GENEVA iterator end() { return data.end(); } // Tested in GDoubleCollection::specificTestsNoFailureExpected_GUnitTests()
	G_API_GENEVA const_iterator end() const { return data.end(); }

	G_API_GENEVA reverse_iterator rbegin() { return data.rbegin(); } // untested (likely irrelevant)
	G_API_GENEVA const_reverse_iterator rbegin() const { return data.rbegin(); }

	G_API_GENEVA reverse_iterator rend() { return data.rend(); } // untested (likely irrelevant)
	G_API_GENEVA const_reverse_iterator rend() const { return data.rend(); }

   /***************************************************************************/
	// Insertion and removal

	/**
	 * Inserts a given item at position pos. Checks whether the item actually points
	 * somewhere.
	 */
	G_API_GENEVA iterator insert(iterator pos, const T& item) { return data.insert(pos, item); }

	/* ----------------------------------------------------------------------------
	 * Tested in GDoubleCollection::specificTestsNoFailureExpected_GUnitTests()
	 * ----------------------------------------------------------------------------
	 */

   /***************************************************************************/
	/**
	 * Inserts a given amount of items after position pos.
	 */
	G_API_GENEVA void insert(iterator pos, size_type amount, const T& item) { data.insert(pos,amount,item); }

	/* ----------------------------------------------------------------------------
	 * Tested in GDoubleCollection::specificTestsNoFailureExpected_GUnitTests()
	 * ----------------------------------------------------------------------------
	 */

   /***************************************************************************/
	// Adding simple items to the  back of the vector
	G_API_GENEVA void push_back(const T& item){ data.push_back(item); } // Used/tested in GDoubleCollection::fillWithData()

   /***************************************************************************/

	// Removal at a given position or in a range
	G_API_GENEVA iterator erase(iterator pos) { return data.erase(pos); } // Tested in GDoubleCollection::specificTestsNoFailureExpected_GUnitTests()
	G_API_GENEVA iterator erase(iterator from, iterator to) { return data.erase(from, to); } // Tested in GDoubleCollection::specificTestsNoFailureExpected_GUnitTests()

	// Removing an element from the end of the vector
	G_API_GENEVA void pop_back(){ data.pop_back(); } // Tested in GDoubleCollection::specificTestsNoFailureExpected_GUnitTests()

   /***************************************************************************/
	/**
	 * Resizing the vector, initialization with item. This function does nothing
	 * if amount is the same as data.size(). We assume in this function that
	 * T is copy-constructible.
	 *
	 * @param amount The new desired size of the vector
	 * @param item An item that should be used for initialization of new items, if any
	 */
	G_API_GENEVA void resize(size_type amount, const T& item) { data.resize(amount, item);} // Tested in GDoubleCollection::specificTestsNoFailureExpected_GUnitTests()

	/***************************************************************************/
	/**
	 * Resize the vector without "template" items. This essentially means that
	 * T will be default-constructed. For numeric values this will usually mean
	 * T(0).
	 */
	G_API_GENEVA void resize(size_type amount) { data.resize(amount); }

   /***************************************************************************/
	/** @brief Clearing the data vector */
	G_API_GENEVA void clear() { data.clear(); } // Used/tested in GDoubleCollection::fillWithData()

   /***************************************************************************/
	/**
	 * Assignment of a std::vector<T>
	 *
	 * @param cp A constant reference to another std::vector<T>
	 * @return The argument of this function (a std::vector<T>)
	 */
	G_API_GENEVA const std::vector<T>& operator=(const std::vector<T>& cp) {
		data=cp;
		return cp;
	}

	/* ----------------------------------------------------------------------------
	 * Tested in GDoubleCollection::specificTestsNoFailureExpected_GUnitTests()
	 * ----------------------------------------------------------------------------
	 */

   /***************************************************************************/
	/**
	 * Creates a copy of the data vector. It is assumed that cp is empty or that
	 * all data in it can be deleted.
	 *
	 * @param cp A reference to a vector that will hold a copy of our local data vector
	 */
	G_API_GENEVA void getDataCopy(std::vector<T>& cp) const { cp=data; 	}  // Tested in GDoubleCollection::specificTestsNoFailureExpected_GUnitTests()

   /***************************************************************************/
	/**
	 * Performs a cross-over operation at a given position. Note: We do NOT require
	 * the two vectors to be of the same size
	 *
	 * @param cp A copy of another GStdSimpleVectorInterfaceT<T> object
	 * @param pos The position as of which the cross-over should be performed
	 */
	G_API_GENEVA void crossOver(GStdSimpleVectorInterfaceT<T>& cp, const std::size_t& pos) {
		// Find out the minimum size of both vectors
		std::size_t minSize = (std::min)(this->size(), cp.size());

#ifdef DEBUG
		// Do some error checking
		if(pos >= minSize) {
		   glogger
         << "In GStdSimpleVectorInterfaceT::crossOver(cp,pos): Error!" << std::endl
         << "Invalid position " << pos << " / " << this->size() << " / " << cp.size() << std::endl
         << GEXCEPTION;
		}
#endif /* DEBUG */

		// Swap the elements
		for(std::size_t i=pos; i<minSize; i++) {
			std::swap(this->at(i), cp.at(i));
		}

		// Move the elements of the longer vector over to the other
		// and remove the elements from the other vector
		if(this->size() > cp.size()) {
			// Attach elements to the other vector
			for(std::size_t i=cp.size(); i<this->size(); i++) {
				cp.push_back(this->at(i));
			}

			// Remove the surplus elements from this vector
			this->erase(this->begin()+minSize, this->end());
		} else if (cp.size() > this->size()) {
			// Attach elements to the other vector
			for(std::size_t i=this->size(); i<cp.size(); i++) {
				this->push_back(cp.at(i));
			}

			// Remove the surplus elements from this vector
			cp.erase(cp.begin()+minSize, cp.end());
		}

		// Nothing to do if both vectors have the same size
	}

protected:
	std::vector<T> data;

	/** @brief Intentionally make this object purely virtual, for performance reasons */
	virtual G_API_GENEVA void dummyFunction() = 0;

private:
	/** @brief Checks for equality with another GStdSimpleVectorInterfaceT<T> object. Intentionally left undefined */
	G_API_GENEVA bool operator==(const GStdSimpleVectorInterfaceT<T>& cp) const;
	/** @brief Checks inequality with another GStdSimpleVectorInterfaceT<T> object. Intentionally left undefined */
	G_API_GENEVA bool operator!=(const GStdSimpleVectorInterfaceT<T>& cp) const;
	/** @brief Checks for equality with a std::vector<T> object. Intentionally left undefined */
	G_API_GENEVA bool operator==(const std::vector<T>& cp_data) const;
	/** @brief Checks for inequality with a std::vector<T> object. Intentionally left undefined */
	G_API_GENEVA bool operator!=(const std::vector<T>& cp_data) const;

public:
   /***************************************************************************/
	/** @brief Applies modifications to this object. This is needed for testing purposes */
	virtual G_API_GENEVA bool modify_GUnitTests() BASE { /* nothing here yet */ return false; }
	/** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
	virtual G_API_GENEVA void specificTestsNoFailureExpected_GUnitTests() BASE { /* nothing here yet */ }
	/** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
	virtual G_API_GENEVA void specificTestsFailuresExpected_GUnitTests() BASE { /* nothing here yet */  }
   /***************************************************************************/
};

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

/******************************************************************************/
/**
 * @brief The content of the BOOST_SERIALIZATION_ASSUME_ABSTRACT(T) macro. Needed for Boost.Serialization
 */
namespace boost {
  namespace serialization {
    template<typename T>
    struct is_abstract<Gem::Geneva::GStdSimpleVectorInterfaceT<T> > : public boost::true_type {};
    template<typename T>
    struct is_abstract< const Gem::Geneva::GStdSimpleVectorInterfaceT<T> > : public boost::true_type {};
  }
}

/******************************************************************************/

#endif /* GSTDSIMPLEVECTORINTERFACET_HPP_ */
