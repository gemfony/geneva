/**
 * @file GIntNumCollectionT.hpp
 */

/*
 * Copyright (C) Authors of the Geneva library collection and Karlsruhe
 * Institute of Technology (University of the State of Baden-Wuerttemberg
 * and National Laboratory of the Helmholtz Association).
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: info [at] gemfony (dot) com
 *
 * This file is part of the Geneva library collection
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
#include <string>
#include <sstream>
#include <vector>

// Includes check for correct Boost version(s)
#include "common/GGlobalDefines.hpp"

// Boost header files go here

#include <boost/cstdint.hpp>
#include <boost/type_traits.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/cast.hpp>
#include <boost/mpl/assert.hpp>

#ifndef GINTNUMCOLLECTIONT_HPP_
#define GINTNUMCOLLECTIONT_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif


// Geneva header files go here

#include "GNumCollectionT.hpp"

namespace Gem {
namespace Geneva {

/*****************************************************************************************/
/**
 * A collection of integer objects without boundaries
 */
template <typename int_type>
class GIntNumCollectionT
	:public GNumCollectionT<int_type>
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int){
	  using boost::serialization::make_nvp;

	  ar & make_nvp("GNumCollectionT_intType", boost::serialization::base_object<GNumCollectionT<int_type> >(*this));
	}
	///////////////////////////////////////////////////////////////////////

	// Make sure this class can only be instantiated if int_type is a *signed* integer type
	BOOST_MPL_ASSERT((boost::is_signed<int_type>));

public:
	/*************************************************************************************/
	/**
	 * The default constructor
	 */
	GIntNumCollectionT()
	{ /* nothing */ }

	/*************************************************************************************/
	/**
	 * The copy constructor
	 *
	 * @param cp A copy of another GIntNumCollectionT<int_type> object
	 */
	GIntNumCollectionT(const GIntNumCollectionT<int_type>& cp)
		: GNumCollectionT<int_type>(cp)
	{ /* nothing */ }

	/*************************************************************************************/
	/**
	 * Allows to specify the boundaries for random initialization
	 *
	 * @param min The lower boundary for random entries
	 * @param max The upper boundary for random entries
	 */
	GIntNumCollectionT(
			const int_type& min
			, const int_type& max
			, typename boost::enable_if<boost::is_integral<int_type> >::type* dummy = 0
	)
		: GNumCollectionT<int_type> (min, max)
	{ /* nothing */ }

	/*************************************************************************************/
	/**
	 * Initialization with a number of random values in a given range
	 *
	 * @param nval The amount of random values
	 * @param min The minimum random value
	 * @param max The maximum random value
	 */
	GIntNumCollectionT(
			const std::size_t& nval
			, const int_type& min
			, const int_type& max
			, typename boost::enable_if<boost::is_integral<int_type> >::type* dummy = 0
	)
		: GNumCollectionT<int_type>(min, max)
	{
		// uniform_int produces random numbers that include the upper boundary
		for(std::size_t i= 0; i<nval; i++) this->push_back(this->GParameterBase::gr->uniform_int(min,max));
	}

	/*************************************************************************************/
	/**
	 * The destructor
	 */
	virtual ~GIntNumCollectionT()
	{ /* nothing */ }

	/*************************************************************************************/
	/**
	 * A standard assignment operator.
	 *
	 * @param cp A copy of another GIntNumCollectionT<int_type> object
	 * @return A constant reference to this object
	 */
	const GIntNumCollectionT<int_type>& operator=(const GIntNumCollectionT<int_type>& cp){
		GIntNumCollectionT<int_type>::load_(&cp);
		return *this;
	}

	/*************************************************************************************/
	/**
	 * Checks for equality with another GIntNumCollectionT<int_type> object
	 *
	 * @param  cp A constant reference to another GIntNumCollectionT<int_type> object
	 * @return A boolean indicating whether both objects are equal
	 */
	bool operator==(const GIntNumCollectionT<int_type>& cp) const {
		using namespace Gem::Common;
		// Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
		return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GIntNumCollectionT<int_type>::operator==","cp", CE_SILENT);
	}

	/*************************************************************************************/
	/**
	 * Checks for inequality with another GIntNumCollectionT<int_type> object
	 *
	 * @param  cp A constant reference to another GIntNumCollectionT<int_type> object
	 * @return A boolean indicating whether both objects are in-equal
	 */
	bool operator!=(const GIntNumCollectionT<int_type>& cp) const {
		using namespace Gem::Common;
		// Means: The expectation of in-equality was fulfilled, if no error text was emitted (which converts to "true")
		return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GIntNumCollectionT<int_type>::operator!=","cp", CE_SILENT);
	}

	/*************************************************************************************/
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
	boost::optional<std::string> checkRelationshipWith(const GObject& cp,
			const Gem::Common::expectation& e,
			const double& limit,
			const std::string& caller,
			const std::string& y_name,
			const bool& withMessages) const
	{
	    using namespace Gem::Common;

	    // Check that we are not accidently assigning this object to itself
	    GObject::selfAssignmentCheck<GIntNumCollectionT<int_type> >(&cp);

		// Will hold possible deviations from the expectation, including explanations
	    std::vector<boost::optional<std::string> > deviations;

		// Check our parent class'es data ...
		deviations.push_back(GNumCollectionT<int_type>::checkRelationshipWith(cp, e, limit, "GIntNumCollectionT<int_type>", y_name, withMessages));

		// no local data ...

		return evaluateDiscrepancies("GIntNumCollectionT<int_type>", caller, deviations, e);
	}


protected:
	/*************************************************************************************/
	/**
	 * Loads the data of another GObject
	 *
	 * @param cp A copy of another GIntNumCollectionT<int_type>  object, camouflaged as a GObject
	 */
	void load_(const GObject* cp){
	    // Check that we are not accidently assigning this object to itself
	    GObject::selfAssignmentCheck<GIntNumCollectionT<int_type> >(cp);

		// Load our parent class'es data ...
		GNumCollectionT<int_type>::load_(cp);

		// ... no local data
	}

	/*************************************************************************************/
	/** @brief Creates a deep clone of this object. Pureley virtual, needs to be defined by derived classes */
	virtual GObject* clone_() const = 0;

	/*************************************************************************************/
	/**
	 * Triggers random initialization of the parameter collection. Note that this
	 * function assumes that the collection has been completely set up. Data
	 * that is added later will remain unaffected.
	 */
	void randomInit_() {
		int_type lowerBoundary = GNumCollectionT<int_type>::getLowerInitBoundary();
		int_type upperBoundary = GNumCollectionT<int_type>::getUpperInitBoundary();

		typename GIntNumCollectionT<int_type>::iterator it;
		for(it=this->begin(); it!=this->end(); ++it) {
			(*it)=this->GParameterBase::gr->uniform_int(lowerBoundary, upperBoundary);
		}
	}

	/* ----------------------------------------------------------------------------------
	 * Tested in GIntNumCollectionT<int_type>::specificTestsNoFailuresExpected_GUnitTests()
	 * ----------------------------------------------------------------------------------
	 */

#ifdef GENEVATESTING
public:
	/*************************************************************************************/
	/**
	 * Applies modifications to this object. This is needed for testing purposes
	 */
	virtual bool modify_GUnitTests() {
		bool result = false;

		// Call the parent class'es function
		if(GNumCollectionT<int_type>::modify_GUnitTests()) result = true;

		return result;
	}

	/*************************************************************************************/
	/**
	 * Performs self tests that are expected to succeed. This is needed for testing purposes
	 */
	virtual void specificTestsNoFailureExpected_GUnitTests() {
		// A few general settings
		const std::size_t nItems = 100;
		const int_type LOWERINITBOUNDARY =  int_type(0); // non-negative value, as int_type might be negative
		const int_type UPPERINITBOUNDARY = int_type(10);
		const int_type FIXEDVALUEINIT = int_type(1);

		// Call the parent class'es function
		GNumCollectionT<int_type>::specificTestsNoFailureExpected_GUnitTests();

		//------------------------------------------------------------------------------

		{ // Initialize with a fixed value, then check setting and retrieval of boundaries and random initialization
			boost::shared_ptr<GIntNumCollectionT<int_type> > p_test1 = this->GObject::clone<GIntNumCollectionT<int_type> >();
			boost::shared_ptr<GIntNumCollectionT<int_type> > p_test2 = this->GObject::clone<GIntNumCollectionT<int_type> >();

			// Make sure p_test1 and p_test2 are empty
			BOOST_CHECK_NO_THROW(p_test1->clear());
			BOOST_CHECK_NO_THROW(p_test2->clear());

			// Add a few items
			for(std::size_t i=0; i<nItems; i++) {
				p_test1->push_back(2*UPPERINITBOUNDARY); // Make sure random initialization cannot randomly leave the value unchanged
			}

			// Set initialization boundaries
			BOOST_CHECK_NO_THROW(p_test1->GNumCollectionT<int_type>::setInitBoundaries(LOWERINITBOUNDARY, UPPERINITBOUNDARY));

			// Check that the boundaries have been set as expected
			BOOST_CHECK(p_test1->GNumCollectionT<int_type>::getLowerInitBoundary() == LOWERINITBOUNDARY);
			BOOST_CHECK(p_test1->GNumCollectionT<int_type>::getUpperInitBoundary() == UPPERINITBOUNDARY);

			// Load the data of p_test1 into p_test2
			BOOST_CHECK_NO_THROW(p_test2->load(p_test1));
			// Cross check that both are indeed equal
			BOOST_CHECK(*p_test1 == *p_test2);

			// Randomly initialize one of the two objects. Note: we are using the protected function rather than the "global" function
			BOOST_CHECK_NO_THROW(p_test1->randomInit_());

			// Check that the object has indeed changed
			BOOST_CHECK(*p_test1 != *p_test2);

			// Check that the values of p_test1 are inside of the allowed boundaroes
			for(std::size_t i=0; i<nItems; i++) {
				BOOST_CHECK(p_test1->at(i) != p_test2->at(i));
				BOOST_CHECK(p_test1->at(i) >= LOWERINITBOUNDARY);
				BOOST_CHECK(p_test1->at(i) <= UPPERINITBOUNDARY);
			}
		}

		//------------------------------------------------------------------------------

		{ // Check that the fp-family of functions doesn't have an effect on this object
			boost::shared_ptr<GIntNumCollectionT<int_type> > p_test1 = this->GObject::clone<GIntNumCollectionT<int_type> >();
			boost::shared_ptr<GIntNumCollectionT<int_type> > p_test2 = this->GObject::clone<GIntNumCollectionT<int_type> >();
			boost::shared_ptr<GIntNumCollectionT<int_type> > p_test3 = this->GObject::clone<GIntNumCollectionT<int_type> >();

			// Add a few items to p_test1
			for(std::size_t i=0; i<nItems; i++) {
				p_test1->push_back(FIXEDVALUEINIT);
			}

			// Load into p_test2 and p_test3 and test equality
			BOOST_CHECK_NO_THROW(p_test2->load(p_test1));
			BOOST_CHECK_NO_THROW(p_test3->load(p_test1));
			BOOST_CHECK(*p_test2 == *p_test1);
			BOOST_CHECK(*p_test3 == *p_test1);
			BOOST_CHECK(*p_test3 == *p_test2);

			// Check that initialization with a fixed floating point value has no effect on this object
			BOOST_CHECK_NO_THROW(p_test2->fpFixedValueInit(2.));
			BOOST_CHECK(*p_test2 == *p_test1);

			// Check that multiplication with a fixed floating point value has no effect on this object
			BOOST_CHECK_NO_THROW(p_test2->fpMultiplyBy(2.));
			BOOST_CHECK(*p_test2 == *p_test1);

			// Check that a component-wise multiplication with a random fp value in a given range does not have an effect on this object
			BOOST_CHECK_NO_THROW(p_test2->fpMultiplyByRandom(1., 2.));
			BOOST_CHECK(*p_test2 == *p_test1);

			// Check that a component-wise multiplication with a random fp value in the range [0:1[ does not have an effect on this object
			BOOST_CHECK_NO_THROW(p_test2->fpMultiplyByRandom());
			BOOST_CHECK(*p_test2 == *p_test1);

			// Check that adding p_test1 to p_test3 does not have an effect
			BOOST_CHECK_NO_THROW(p_test3->fpAdd(p_test1));
			BOOST_CHECK(*p_test3 == *p_test2);

			// Check that subtracting p_test1 from p_test3 does not have an effect
			BOOST_CHECK_NO_THROW(p_test3->fpSubtract(p_test1));
			BOOST_CHECK(*p_test3 == *p_test2);
		}

		//------------------------------------------------------------------------------
	}

	/*************************************************************************************/
	/**
	 * Performs self tests that are expected to fail. This is needed for testing purposes
	 */
	virtual void specificTestsFailuresExpected_GUnitTests() {
		// Call the parent class'es function
		GNumCollectionT<int_type>::specificTestsFailuresExpected_GUnitTests();
	}

	/*************************************************************************************/
#endif /* GENEVATESTING */
};

/*****************************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

/*****************************************************************************************/
// The content of BOOST_SERIALIZATION_ASSUME_ABSTRACT(T)

namespace boost {
	namespace serialization {
		template<typename int_type>
		struct is_abstract<Gem::Geneva::GIntNumCollectionT<int_type> > : public boost::true_type {};
		template<typename int_type>
		struct is_abstract< const Gem::Geneva::GIntNumCollectionT<int_type> > : public boost::true_type {};
	}
}

/*****************************************************************************************/

#endif /* GINTNUMCOLLECTIONT_HPP_ */
