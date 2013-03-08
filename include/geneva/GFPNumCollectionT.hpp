/**
 * @file GFPNumCollectionT.hpp
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt)
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) com
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
 * http://www.gemfony.com .
 */


// Standard header files go here

// Boost header files go here

#ifndef GNUMCOLLECTIONFPT_HPP_
#define GNUMCOLLECTIONFPT_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif


// Geneva header files go here
#include "geneva/GObject.hpp"
#include "geneva/GDoubleGaussAdaptor.hpp"
#include "geneva/GNumCollectionT.hpp"
#include "common/GExceptions.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * This class represents a collection of floating point values, all modified
 * using the same algorithm. The most likely type to be stored in this
 * class is a double.
 */
template <typename fp_type>
class GFPNumCollectionT
	: public GNumCollectionT<fp_type>
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int) {
		using boost::serialization::make_nvp;
		ar & make_nvp("GNumCollectionT_fpType", boost::serialization::base_object<GNumCollectionT<fp_type> >(*this));
	}
	///////////////////////////////////////////////////////////////////////

	// Make sure this class can only be instantiated if fp_type really is a floating point type
	BOOST_MPL_ASSERT((boost::is_floating_point<fp_type>));

public:
	/***************************************************************************/
	/**
	 * The default constructor.
	 */
	GFPNumCollectionT()
		: GNumCollectionT<fp_type> ()
	{ /* nothing */ }

	/***************************************************************************/
	/**
	 * Initialization with a number of random values in a given range
	 *
	 * @param nval The amount of random values
	 * @param min The minimum random value
	 * @param max The maximum random value
	 */
	GFPNumCollectionT(
			const std::size_t& nval
			, const fp_type& min
			, const fp_type& max
	)
		: GNumCollectionT<fp_type>(nval, min, min, max) // The vector is preset to nval entries with value "min"
	{
		// Assign random values to each position
		typename GFPNumCollectionT<fp_type>::iterator it;
		for(it=this->begin(); it!=this->end(); ++it) {
			*it = this->GParameterBase::gr->Gem::Hap::GRandomBase::template uniform_real<fp_type>(min,max);
		}
	}

	/***************************************************************************/
	/**
	 * Initialization with a number of items of predefined value. We
	 * enforce setting of the lower and upper boundaries for random initialization,
	 * as these double up as the preferred value range in some optimization algorithms,
	 * such as swarm algorithms.
	 *
	 * @param nval The number of variables to be stored in the collection
	 * @param val  The value to be used for their initialization
	 */
	GFPNumCollectionT(
		const std::size_t& nval
		, const fp_type& val
		, const fp_type& min
		, const fp_type& max
	)
		: GNumCollectionT<fp_type>(nval, val, min, max)
	{ /* nothing */ }

	/***************************************************************************/
	/**
	 * The standard copy constructor
	 */
	GFPNumCollectionT(const GFPNumCollectionT<fp_type>& cp)
		: GNumCollectionT<fp_type> (cp)
	{ /* nothing */ }

	/***************************************************************************/
	/**
	 * The standard destructor
	 */
	virtual ~GFPNumCollectionT()
	{ /* nothing */ }

	/***************************************************************************/
	/**
	 * The standard assignment operator.
	 *
	 * @param cp A copy of another GDoubleCollection object
	 * @return A constant reference to this object
	 */
	const GFPNumCollectionT& operator=(const GFPNumCollectionT<fp_type>& cp){
		GFPNumCollectionT<fp_type>::load_(&cp);
		return *this;
	}

	/***************************************************************************/
	/**
	 * Checks for equality with another GFPNumCollectionT<fp_type> object
	 *
	 * @param  cp A constant reference to another GFPNumCollectionT<fp_type> object
	 * @return A boolean indicating whether both objects are equal
	 */
	bool operator==(const GFPNumCollectionT<fp_type>& cp) const {
		using namespace Gem::Common;
		// Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
		return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GFPNumCollectionT<fp_type>::operator==","cp", CE_SILENT);
	}

	/***************************************************************************/
	/**
	 * Checks for inequality with another GFPNumCollectionT<fp_type> object
	 *
	 * @param  cp A constant reference to another GFPNumCollectionT<fp_type> object
	 * @return A boolean indicating whether both objects are inequal
	 */
	bool operator!=(const GFPNumCollectionT<fp_type>& cp) const {
		using namespace Gem::Common;
		// Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
		return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GFPNumCollectionT<fp_type>::operator!=","cp", CE_SILENT);
	}

	/***************************************************************************/
	/**
	 * Initializes floating-point-based parameters with a given value.
	 *
	 * @param val The value to use for the initialization
	 */
	virtual void fpFixedValueInit(const float& val) {
		typename GFPNumCollectionT<fp_type>::iterator it;
		for(it=this->begin(); it!=this->end(); ++it) {
			(*it)=fp_type(val);
		}
	}

	/* ----------------------------------------------------------------------------------
	 * Tested in GFPNumCollectionT<fp_type>::specificTestsNoFailuresExpected_GUnitTests()
	 * ----------------------------------------------------------------------------------
	 */

	/***************************************************************************/
	/**
	 * Multiplies floating-point-based parameters with a given value
	 *
	 * @param val The value to be multiplied with the parameter
	 */
	virtual void fpMultiplyBy(const float& val) {
		typename GFPNumCollectionT<fp_type>::iterator it;
		for(it=this->begin(); it!=this->end(); ++it) {
			(*it) *= fp_type(val);
		}
	}

	/* ----------------------------------------------------------------------------------
	 * Tested in GFPNumCollectionT<fp_type>::specificTestsNoFailuresExpected_GUnitTests()
	 * ----------------------------------------------------------------------------------
	 */

	/***************************************************************************/
	/**
	 * Multiplies with a random floating point number in a given range.
	 *
	 * @param min The lower boundary for random number generation
	 * @param max The upper boundary for random number generation
	 */
	virtual void fpMultiplyByRandom(const float& min, const float& max)	{
		typename GFPNumCollectionT<fp_type>::iterator it;
		for(it=this->begin(); it!=this->end(); ++it) {
			(*it) *= this->GParameterBase::gr->Gem::Hap::GRandomBase::template uniform_real<fp_type>(fp_type(min), fp_type(max));
		}
	}

	/* ----------------------------------------------------------------------------------
	 * Tested in GFPNumCollectionT<fp_type>::specificTestsNoFailuresExpected_GUnitTests()
	 * ----------------------------------------------------------------------------------
	 */

	/***************************************************************************/
	/**
	 * Multiplies with a random floating point number in the range [0, 1[.
	 */
	virtual void fpMultiplyByRandom() {
		typename GFPNumCollectionT<fp_type>::iterator it;
		for(it=this->begin(); it!=this->end(); ++it) {
			(*it) *= this->GParameterBase::gr->Gem::Hap::GRandomBase::template uniform_01<fp_type>();
		}
	}

	/* ----------------------------------------------------------------------------------
	 * Tested in GFPNumCollectionT<fp_type>::specificTestsNoFailuresExpected_GUnitTests()
	 * ----------------------------------------------------------------------------------
	 */

	/***************************************************************************/
	/**
	 * Adds the floating point parameters of another GParameterBase object to this one.
	 *
	 * @oaram p A boost::shared_ptr to another GParameterBase object
	 */
	virtual void fpAdd(boost::shared_ptr<GParameterBase> p_base) {
		// We first need to convert p_base into the local type
		boost::shared_ptr<GFPNumCollectionT<fp_type> > p = GParameterBase::parameterbase_cast<GFPNumCollectionT<fp_type> >(p_base);

		// Do some error checking
		if(this->size() != p->size()) {
		   glogger
		   << "In GFPNumCollectionT<fp_type>::fpAdd():" << std::endl
         << "Collection sizes don't match: " << this->size() << " " << p->size() << std::endl
         << GEXCEPTION;
		}

		typename GFPNumCollectionT<fp_type>::iterator it, it_p;
		for(it=this->begin(), it_p=p->begin(); it!=this->end(); ++it, ++it_p) {
			(*it) += (*it_p);
		}
	}

	/* ----------------------------------------------------------------------------------
	 * Tested in GFPNumCollectionT<fp_type>::specificTestsNoFailuresExpected_GUnitTests()
	 * ----------------------------------------------------------------------------------
	 */

	/***************************************************************************/
	/**
	 * Subtracts the floating point parameters of another GParameterBase object
	 * from this one.
	 *
	 * @oaram p A boost::shared_ptr to another GParameterBase object
	 */
	virtual void fpSubtract(boost::shared_ptr<GParameterBase> p_base) {
		// We first need to convert p_base into the local type
		boost::shared_ptr<GFPNumCollectionT<fp_type> > p = GParameterBase::parameterbase_cast<GFPNumCollectionT<fp_type> >(p_base);

		// Do some error checking
		if(this->size() != p->size()) {
		   glogger
		   << "In GFPNumCollectionT<fp_type>::fpSubtract():" << std::endl
         << "Collection sizes don't match: " << this->size() << " " << p->size() << std::endl
         << GEXCEPTION;
		}

		typename GFPNumCollectionT<fp_type>::iterator it, it_p;
		for(it=this->begin(), it_p=p->begin(); it!=this->end(); ++it, ++it_p) {
			(*it) -= (*it_p);
		}
	}

	/* ----------------------------------------------------------------------------------
	 * Tested in GFPNumCollectionT<fp_type>::specificTestsNoFailuresExpected_GUnitTests()
	 * ----------------------------------------------------------------------------------
	 */

	/***************************************************************************/
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

		// Check that we are indeed dealing with a GParamterBase reference
		const GFPNumCollectionT<fp_type>  *p_load = GObject::gobject_conversion<GFPNumCollectionT<fp_type> >(&cp);

		// Will hold possible deviations from the expectation, including explanations
	    std::vector<boost::optional<std::string> > deviations;

		// Check our parent class'es data ...
		deviations.push_back(GNumCollectionT<fp_type>::checkRelationshipWith(cp, e, limit, "GFPNumCollectionT<fp_type>", y_name, withMessages));

		// no local data ...

		return evaluateDiscrepancies("GFPNumCollectionT<fp_type>", caller, deviations, e);
	}

   /***************************************************************************/
   /**
    * Emits a name for this class / object
    */
   virtual std::string name() const {
      return std::string("GFPNumCollectionT");
   }

protected:
	/***************************************************************************/
	/**
	 * Loads the data of another GFPNumCollectionT<fp_type> object,
	 * camouflaged as a GObject. We have no local data, so
	 * all we need to do is to the standard identity check,
	 * preventing that an object is assigned to itself.
	 *
	 * @param cp A copy of another GFPNumCollectionT<fp_type> object, camouflaged as a GObject
	 */
	virtual void load_(const GObject *cp){
		// Convert cp into local format
		const GFPNumCollectionT<fp_type> *p_load = GObject::gobject_conversion<GFPNumCollectionT<fp_type> >(cp);

		// Load our parent class'es data ...
		GNumCollectionT<fp_type>::load_(cp);

		// no local data ...
	}

	/***************************************************************************/
	/**
	 * Creates a deep copy of this object. Purely virtual as this class
	 * should not be instantiable.
	 *
	 * @return A pointer to a deep clone of this object
	 */
	virtual GObject *clone_() const = 0;

	/***************************************************************************/
	/**
	 * Triggers random initialization of the parameter collection. Note
	 * that this function assumes that the collection has been completely
	 * set up. Data that is added later will remain unaffected.
	 */
	virtual void randomInit_() {
		fp_type lowerBoundary = GNumCollectionT<fp_type>::getLowerInitBoundary();
		fp_type upperBoundary = GNumCollectionT<fp_type>::getUpperInitBoundary();

		typename GFPNumCollectionT<fp_type>::iterator it;
		for(it=this->begin(); it!=this->end(); ++it) {
			(*it)=this->GParameterBase::gr->Gem::Hap::GRandomBase::template uniform_real<fp_type>(lowerBoundary, upperBoundary);
		}
	}

	/* ----------------------------------------------------------------------------------
	 * Tested in GFPNumCollectionT<fp_type>::specificTestsNoFailuresExpected_GUnitTests()
	 * ----------------------------------------------------------------------------------
	 */

public:

	/***************************************************************************/
	/**
	 * Applies modifications to this object. This is needed for testing purposes
	 *
	 * @return A boolean which indicates whether modifications were made
	 */
	virtual bool modify_GUnitTests() {
#ifdef GEM_TESTING
      bool result = false;

		// Call the parent classes' functions
		if(GNumCollectionT<fp_type>::modify_GUnitTests()) result = true;

		return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		condnotset("GFPNumCollectionT::modify_GUnitTests", "GEM_TESTING");
		return false;
#endif /* GEM_TESTING */
	}

	/***************************************************************************/
	/**
	 * Performs self tests that are expected to succeed. This is needed for testing purposes
	 */
	virtual void specificTestsNoFailureExpected_GUnitTests() {
#ifdef GEM_TESTING
		// Call the parent classes' functions
		GNumCollectionT<fp_type>::specificTestsNoFailureExpected_GUnitTests();

		// A few settings
		const std::size_t nItems = 100;
		const fp_type LOWERINITBOUNDARY = -10.1;
		const fp_type UPPERINITBOUNDARY =  10.1;
		const fp_type FIXEDVALUEINIT = 1.;
		const fp_type MULTVALUE = 3.;
		const fp_type RANDLOWERBOUNDARY = 0.;
		const fp_type RANDUPPERBOUNDARY = 10.;

		//------------------------------------------------------------------------------

		{ // Check initialization with a fixed value, setting and retrieval of boundaries and random initialization
			boost::shared_ptr<GFPNumCollectionT<fp_type> > p_test1 = this->GObject::clone<GFPNumCollectionT<fp_type> >();
			boost::shared_ptr<GFPNumCollectionT<fp_type> > p_test2 = this->GObject::clone<GFPNumCollectionT<fp_type> >();

			// Make sure p_test1 and p_test2 are empty
			BOOST_CHECK_NO_THROW(p_test1->clear());
			BOOST_CHECK_NO_THROW(p_test2->clear());

			// Add a few items
			for(std::size_t i=0; i<nItems; i++) {
				p_test1->push_back(fp_type(0));
				p_test2->push_back(fp_type(0));
			}

			// Initialize with a fixed value
			BOOST_CHECK_NO_THROW(p_test1->fpFixedValueInit(FIXEDVALUEINIT));
			BOOST_CHECK_NO_THROW(p_test2->fpFixedValueInit(FIXEDVALUEINIT));

			// Check that values have indeed been set
			for(std::size_t i=0; i<nItems; i++) {
				BOOST_CHECK(p_test1->at(i) == FIXEDVALUEINIT);
				BOOST_CHECK(p_test2->at(i) == FIXEDVALUEINIT);
			}

			// Set initialization boundaries
			BOOST_CHECK_NO_THROW(p_test1->setInitBoundaries(LOWERINITBOUNDARY, UPPERINITBOUNDARY));
			BOOST_CHECK_NO_THROW(p_test2->setInitBoundaries(LOWERINITBOUNDARY, UPPERINITBOUNDARY));

			// Randomly initialize one of the two objects. Note: we are using the protected function rather than the "global" function
			BOOST_CHECK_NO_THROW(p_test1->randomInit_());

			// Check that the object has indeed changed
			BOOST_CHECK(*p_test1 != *p_test2);

			// Check that each value is different and that the values of p_test1 are inside of the allowed boundaroes
			for(std::size_t i=0; i<nItems; i++) {
				BOOST_CHECK(p_test1->at(i) != p_test2->at(i));
				BOOST_CHECK(p_test1->at(i) >= LOWERINITBOUNDARY);
				BOOST_CHECK(p_test1->at(i) <= UPPERINITBOUNDARY);
			}
		}

		//------------------------------------------------------------------------------

		{ // Test multiplication with a fixed value
			boost::shared_ptr<GFPNumCollectionT<fp_type> > p_test1 = this->GObject::clone<GFPNumCollectionT<fp_type> >();
			boost::shared_ptr<GFPNumCollectionT<fp_type> > p_test2 = this->GObject::clone<GFPNumCollectionT<fp_type> >();

			// Make sure p_test1 and p_test2 are empty
			BOOST_CHECK_NO_THROW(p_test1->clear());

			// Add a few items
			for(std::size_t i=0; i<nItems; i++) {
				p_test1->push_back(fp_type(0));
			}

			// Initialize with a fixed value
			BOOST_CHECK_NO_THROW(p_test1->fpFixedValueInit(FIXEDVALUEINIT));

			// Set initialization boundaries
			BOOST_CHECK_NO_THROW(p_test1->setInitBoundaries(LOWERINITBOUNDARY, UPPERINITBOUNDARY));

			// Randomly initialize one of the two objects. Note: we are using the protected function rather than the "global" function
			BOOST_CHECK_NO_THROW(p_test1->randomInit_());

			// Load the data into p_test2 and check that both objects are equal
			BOOST_CHECK_NO_THROW(p_test2->load(p_test1));
			BOOST_CHECK(*p_test1 == *p_test2);

			// Multiply p_test1 with a fixed value
			BOOST_CHECK_NO_THROW(p_test1->fpMultiplyBy(MULTVALUE));

			// Check that the multiplication has succeeded
			for(std::size_t i=0; i<nItems; i++) {
				BOOST_CHECK(p_test1->at(i) == MULTVALUE * p_test2->at(i));
			}
		}

		//------------------------------------------------------------------------------

		{ // Test multiplication with a random value in fixed range
			boost::shared_ptr<GFPNumCollectionT<fp_type> > p_test1 = this->GObject::clone<GFPNumCollectionT<fp_type> >();
			boost::shared_ptr<GFPNumCollectionT<fp_type> > p_test2 = this->GObject::clone<GFPNumCollectionT<fp_type> >();

			// Make sure p_test1 and p_test2 are empty
			BOOST_CHECK_NO_THROW(p_test1->clear());

			// Add a few items
			for(std::size_t i=0; i<nItems; i++) {
				p_test1->push_back(fp_type(0));
			}

			// Initialize with a fixed value
			BOOST_CHECK_NO_THROW(p_test1->fpFixedValueInit(1.));

			// Multiply with random values in a given range
			BOOST_CHECK_NO_THROW(p_test1->fpMultiplyByRandom(RANDLOWERBOUNDARY, RANDUPPERBOUNDARY));

			// Check that all values are in this range
			for(std::size_t i=0; i<nItems; i++) {
				BOOST_CHECK(p_test1->at(i) >= RANDLOWERBOUNDARY);
				BOOST_CHECK(p_test1->at(i) <= RANDUPPERBOUNDARY);
			}
		}

		//------------------------------------------------------------------------------

		{ // Test multiplication with a random value in the range [0:1[
			boost::shared_ptr<GFPNumCollectionT<fp_type> > p_test1 = this->GObject::clone<GFPNumCollectionT<fp_type> >();
			boost::shared_ptr<GFPNumCollectionT<fp_type> > p_test2 = this->GObject::clone<GFPNumCollectionT<fp_type> >();

			// Make sure p_test1 and p_test2 are empty
			BOOST_CHECK_NO_THROW(p_test1->clear());

			// Add a few items
			for(std::size_t i=0; i<nItems; i++) {
				p_test1->push_back(fp_type(0));
			}

			// Initialize with a fixed value
			BOOST_CHECK_NO_THROW(p_test1->fpFixedValueInit(1.));

			// Multiply with random values in a given range
			BOOST_CHECK_NO_THROW(p_test1->fpMultiplyByRandom());

			// Check that all values are in this range
			for(std::size_t i=0; i<nItems; i++) {
				BOOST_CHECK(p_test1->at(i) >= 0.);
				BOOST_CHECK(p_test1->at(i) <= 1.);
			}
		}

		//------------------------------------------------------------------------------

		{ // Test addition of other GFPNumCollectionT<fp_type> objets
			boost::shared_ptr<GFPNumCollectionT<fp_type> > p_test1 = this->GObject::clone<GFPNumCollectionT<fp_type> >();
			boost::shared_ptr<GFPNumCollectionT<fp_type> > p_test2 = this->GObject::clone<GFPNumCollectionT<fp_type> >();
			boost::shared_ptr<GFPNumCollectionT<fp_type> > p_test3 = this->GObject::clone<GFPNumCollectionT<fp_type> >();

			// Make sure all clones are empty
			BOOST_CHECK_NO_THROW(p_test1->clear());
			BOOST_CHECK_NO_THROW(p_test2->clear());
			BOOST_CHECK_NO_THROW(p_test3->clear());

			// Add a few items
			for(std::size_t i=0; i<nItems; i++) {
				p_test1->push_back(fp_type(0));
			}

			// Set initialization boundaries
			BOOST_CHECK_NO_THROW(p_test1->setInitBoundaries(LOWERINITBOUNDARY, UPPERINITBOUNDARY));

			// Load the data of p_test_1 into p_test2
			BOOST_CHECK_NO_THROW(p_test2->load(p_test1));

			// Randomly initialize p_test1 and p_test2, so that both objects are different
			BOOST_CHECK_NO_THROW(p_test1->randomInit_());
			BOOST_CHECK_NO_THROW(p_test2->randomInit_());

			// Check that they are indeed different
			BOOST_CHECK(*p_test1 != *p_test2);

			// Load p_test2's data into p_test_3
			BOOST_CHECK_NO_THROW(p_test3->load(p_test2));

			// Add p_test1 to p_test3
			BOOST_CHECK_NO_THROW(p_test3->fpAdd(p_test1));

			// Cross check that for each i p_test3[i] == p_test1[i] + p_test2[i]
			for(std::size_t i=0; i<nItems; i++) {
				BOOST_CHECK(p_test3->at(i) == p_test1->at(i) + p_test2->at(i));
			}
		}

		//------------------------------------------------------------------------------

		{ // Test subtraction of other GFPNumCollectionT<fp_type> objets
			boost::shared_ptr<GFPNumCollectionT<fp_type> > p_test1 = this->GObject::clone<GFPNumCollectionT<fp_type> >();
			boost::shared_ptr<GFPNumCollectionT<fp_type> > p_test2 = this->GObject::clone<GFPNumCollectionT<fp_type> >();
			boost::shared_ptr<GFPNumCollectionT<fp_type> > p_test3 = this->GObject::clone<GFPNumCollectionT<fp_type> >();

			// Make sure all clones are empty
			BOOST_CHECK_NO_THROW(p_test1->clear());
			BOOST_CHECK_NO_THROW(p_test2->clear());
			BOOST_CHECK_NO_THROW(p_test3->clear());

			// Add a few items
			for(std::size_t i=0; i<nItems; i++) {
				p_test1->push_back(fp_type(0));
			}

			// Set initialization boundaries
			BOOST_CHECK_NO_THROW(p_test1->setInitBoundaries(LOWERINITBOUNDARY, UPPERINITBOUNDARY));

			// Load the data of p_test_1 into p_test2
			BOOST_CHECK_NO_THROW(p_test2->load(p_test1));

			// Randomly initialize p_test1 and p_test2, so that both objects are different
			BOOST_CHECK_NO_THROW(p_test1->randomInit_());
			BOOST_CHECK_NO_THROW(p_test2->randomInit_());

			// Check that they are indeed different
			BOOST_CHECK(*p_test1 != *p_test2);

			// Load p_test2's data into p_test_3
			BOOST_CHECK_NO_THROW(p_test3->load(p_test2));

			// Add p_test1 to p_test3
			BOOST_CHECK_NO_THROW(p_test3->fpSubtract(p_test1));

			// Cross check that for each i p_test3[i] == p_test1[i] - p_test2[i]
			for(std::size_t i=0; i<nItems; i++) {
				BOOST_CHECK(p_test3->at(i) == p_test2->at(i) - p_test1->at(i));
			}
		}

		//------------------------------------------------------------------------------

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
		condnotset("GFPNumCollectionT<>::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
	}

	/***************************************************************************/
	/**
	 * Performs self tests that are expected to fail. This is needed for testing purposes
	 */
	virtual void specificTestsFailuresExpected_GUnitTests() {
#ifdef GEM_TESTING
		// A few settings
		const std::size_t nItems = 100;

		// Call the parent classes' functions
		GNumCollectionT<fp_type>::specificTestsFailuresExpected_GUnitTests();

		//------------------------------------------------------------------------------

		{ // Check that adding another object of different size throws
			boost::shared_ptr<GFPNumCollectionT<fp_type> > p_test1 = this->GObject::clone<GFPNumCollectionT<fp_type> >();
			boost::shared_ptr<GFPNumCollectionT<fp_type> > p_test2 = this->GObject::clone<GFPNumCollectionT<fp_type> >();

			// Add a few items to p_test1, but not to p_test2
			for(std::size_t i=0; i<nItems; i++) {
				p_test1->push_back(fp_type(0));
			}

			BOOST_CHECK_THROW(p_test1->fpAdd(p_test2), Gem::Common::gemfony_error_condition);
		}

		//------------------------------------------------------------------------------

		{ // Check that subtracting another object of different size throws
			boost::shared_ptr<GFPNumCollectionT<fp_type> > p_test1 = this->GObject::clone<GFPNumCollectionT<fp_type> >();
			boost::shared_ptr<GFPNumCollectionT<fp_type> > p_test2 = this->GObject::clone<GFPNumCollectionT<fp_type> >();

			// Add a few items to p_test1, but not to p_test2
			for(std::size_t i=0; i<nItems; i++) {
				p_test1->push_back(fp_type(0));
			}

			BOOST_CHECK_THROW(p_test1->fpSubtract(p_test2), Gem::Common::gemfony_error_condition);
		}

		//------------------------------------------------------------------------------

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
      condnotset("GFPNumCollectionT<>::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
	}

};

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

/******************************************************************************/
// The content of BOOST_SERIALIZATION_ASSUME_ABSTRACT(T)

namespace boost {
	namespace serialization {
		template<typename fp_type>
		struct is_abstract<Gem::Geneva::GFPNumCollectionT<fp_type> > : public boost::true_type {};
		template<typename fp_type>
		struct is_abstract< const Gem::Geneva::GFPNumCollectionT<fp_type> > : public boost::true_type {};
	}
}
/******************************************************************************/

#endif /* GNUMCOLLECTIONFPT_HPP_ */
