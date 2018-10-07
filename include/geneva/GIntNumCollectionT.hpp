/**
 * @file
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

#pragma once

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard header files go here

// Boost header files go here

// Geneva header files go here

#include "geneva/GNumCollectionT.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
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

		 ar
		 & make_nvp("GNumCollectionT_intType", boost::serialization::base_object<GNumCollectionT<int_type>>(*this));
	 }
	 ///////////////////////////////////////////////////////////////////////

	 // Make sure this class can only be instantiated if int_type is a *signed* integer type
	 static_assert(
		 std::is_signed<int_type>::value
		 , "int_type should be a signed integer type"
	 );

public:
	 /***************************************************************************/
	 /**
	  * The default constructor
	  */
	 GIntNumCollectionT()
	 { /* nothing */ }

	 /***************************************************************************/
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
	 )
		 : GNumCollectionT<int_type>(nval, min, min, max) // Initialization of a vector with nval variables of value "min"
	 {
		 Gem::Hap::GRandomT<Gem::Hap::RANDFLAVOURS::RANDOMLOCAL> gr;
		 typename std::uniform_int_distribution<int_type> uniform_int(min, max);

		 // Fill the vector with random values
		 typename GIntNumCollectionT<int_type>::iterator it;
		 for(it=this->begin(); it!=this->end(); ++it) {
			 *it = uniform_int(gr);
		 }
	 }

	 /***************************************************************************/
	 /**
	  * Specifies the size of the data vector and an item to be assigned
	  * to each position. We enforce setting of the lower and upper boundaries
	  * for random initialization, as these may double up as the preferred value
	  * range in some optimization algorithms.
	  *
	  * @param nval The amount of random values
	  * @param min The minimum random value
	  * @param max The maximum random value
	  */
	 GIntNumCollectionT(
		 const std::size_t& nval
		 , const int_type& val
		 , const int_type& min
		 , const int_type& max
	 )
		 : GNumCollectionT<int_type>(nval, val, min, max)
	 { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * The copy constructor
	  *
	  * @param cp A copy of another GIntNumCollectionT<int_type> object
	  */
	 GIntNumCollectionT(const GIntNumCollectionT<int_type>& cp)
		 : GNumCollectionT<int_type>(cp)
	 { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * The destructor
	  */
	 virtual ~GIntNumCollectionT()
	 { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * Searches for compliance with expectations with respect to another object
	  * of the same type
	  *
	  * @param cp A constant reference to another GObject object
	  * @param e The expected outcome of the comparison
	  * @param limit The maximum deviation for floating point values (important for similarity checks)
	  */
	 virtual void compare(
		 const GObject& cp
		 , const Gem::Common::expectation& e
		 , const double& limit
	 ) const override {
		 using namespace Gem::Common;

		 // Check that we are dealing with a GIntNumCollectionT<int_type> reference independent of this object and convert the pointer
		 const GIntNumCollectionT<int_type> *p_load = Gem::Common::g_convert_and_compare<GObject, GIntNumCollectionT<int_type>>(cp, this);

		 GToken token("GIntNumCollectionT<int_type>", e);

		 // Compare our parent data ...
		 Gem::Common::compare_base<GNumCollectionT<int_type>>(IDENTITY(*this, *p_load), token);

		 // ... no local data

		 // React on deviations from the expectation
		 token.evaluate();
	 }

	 /***************************************************************************/
	 /**
	  * Emits a name for this class / object
	  */
	 std::string name() const override {
		 return std::string("GIntNumCollectionT");
	 }

protected:
	 /***************************************************************************/
	 /**
	  * Loads the data of another GObject
	  *
	  * @param cp A copy of another GIntNumCollectionT<int_type>  object, camouflaged as a GObject
	  */
	 void load_(const GObject* cp) override {
		 // Convert the pointer to our target type and check for self-assignment
		 const GIntNumCollectionT<int_type> * p_load = Gem::Common::g_convert_and_compare<GObject, GIntNumCollectionT<int_type>>(cp, this);

		 // Load our parent class'es data ...
		 GNumCollectionT<int_type>::load_(cp);

		 // ... no local data
	 }

	 /***************************************************************************/
	 /**
	  * Triggers random initialization of the parameter collection. Note that this
	  * function assumes that the collection has been completely set up. Data
	  * that is added later will remain unaffected.
	  */
	 bool randomInit_(
		 const activityMode& am
		 , Gem::Hap::GRandomBase& gr
	 ) override {
		 int_type lowerBoundary = GNumCollectionT<int_type>::getLowerInitBoundary();
		 int_type upperBoundary = GNumCollectionT<int_type>::getUpperInitBoundary();

		 typename std::uniform_int_distribution<int_type> uniform_int(lowerBoundary, upperBoundary);
		 typename GIntNumCollectionT<int_type>::iterator it;
		 for(it=this->begin(); it!=this->end(); ++it) {
			 (*it)=uniform_int(gr);
		 }

		 return true;
	 }

	 /* ----------------------------------------------------------------------------------
	  * Tested in GIntNumCollectionT<int_type>::specificTestsNoFailuresExpected_GUnitTests()
	  * ----------------------------------------------------------------------------------
	  */

private:
	 /***************************************************************************/
	 /** @brief Creates a deep clone of this object. Purely virtual, needs to be defined by derived classes */
	 GObject* clone_() const override = 0;

public:
	 /***************************************************************************/
	 /**
	  * Applies modifications to this object. This is needed for testing purposes
	  */
	 bool modify_GUnitTests() override {
#ifdef GEM_TESTING
		 bool result = false;

		 // Call the parent class'es function
		 if(GNumCollectionT<int_type>::modify_GUnitTests()) result = true;

		 return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		 Gem::Common::condnotset("GIntNumCollectionT<>::modify_GUnitTests", "GEM_TESTING");
		return false;
#endif /* GEM_TESTING */
	 }

	 /***************************************************************************/
	 /**
	  * Performs self tests that are expected to succeed. This is needed for testing purposes
	  */
	 void specificTestsNoFailureExpected_GUnitTests() override {
#ifdef GEM_TESTING
		 // A few general settings
		 const std::size_t nItems = 100;
		 const int_type LOWERINITBOUNDARY =  int_type(0); // non-negative value, as int_type might be negative
		 const int_type UPPERINITBOUNDARY = int_type(10);
		 const int_type FIXEDVALUEINIT = int_type(1);

		 // Call the parent class'es function
		 GNumCollectionT<int_type>::specificTestsNoFailureExpected_GUnitTests();

		 // A random generator
		 Gem::Hap::GRandomT<Gem::Hap::RANDFLAVOURS::RANDOMPROXY> gr;

		 //------------------------------------------------------------------------------

		 { // Initialize with a fixed value, then check setting and retrieval of boundaries and random initialization
			 std::shared_ptr<GIntNumCollectionT<int_type>> p_test1 = this->template clone<GIntNumCollectionT<int_type>>();
			 std::shared_ptr<GIntNumCollectionT<int_type>> p_test2 = this->template clone<GIntNumCollectionT<int_type>>();

			 // Make sure p_test1 and p_test2 are empty
			 BOOST_CHECK_NO_THROW(p_test1->clear());
			 BOOST_CHECK_NO_THROW(p_test2->clear());

			 // Add a few items
			 for(std::size_t i=0; i<nItems; i++) {
				 p_test1->push_back(2*UPPERINITBOUNDARY); // Make sure random initialization cannot randomly leave the value unchanged
			 }

			 // Set initialization boundaries
			 BOOST_CHECK_NO_THROW(p_test1->setInitBoundaries(LOWERINITBOUNDARY, UPPERINITBOUNDARY));

			 // Check that the boundaries have been set as expected
			 BOOST_CHECK(p_test1->getLowerInitBoundary() == LOWERINITBOUNDARY);
			 BOOST_CHECK(p_test1->getUpperInitBoundary() == UPPERINITBOUNDARY);

			 // Load the data of p_test1 into p_test2
			 BOOST_CHECK_NO_THROW(p_test2->load(p_test1));
			 // Cross check that both are indeed equal
			 BOOST_CHECK(*p_test1 == *p_test2);

			 // Randomly initialize one of the two objects. Note: we are using the protected function rather than the "global" function
			 BOOST_CHECK_NO_THROW(p_test1->randomInit_(activityMode::ALLPARAMETERS, gr));

			 // Check that the object has indeed changed
			 BOOST_CHECK(*p_test1 != *p_test2);

			 // Check that the values of p_test1 are inside of the allowed boundaries
			 for(std::size_t i=0; i<nItems; i++) {
				 BOOST_CHECK(p_test1->at(i) != p_test2->at(i));
				 BOOST_CHECK(p_test1->at(i) >= LOWERINITBOUNDARY);
				 BOOST_CHECK(p_test1->at(i) <= UPPERINITBOUNDARY);
			 }
		 }

		 //------------------------------------------------------------------------------

		 { // Check that the fp-family of functions doesn't have an effect on this object
			 std::shared_ptr<GIntNumCollectionT<int_type>> p_test1 = this->template clone<GIntNumCollectionT<int_type>>();
			 std::shared_ptr<GIntNumCollectionT<int_type>> p_test2 = this->template clone<GIntNumCollectionT<int_type>>();
			 std::shared_ptr<GIntNumCollectionT<int_type>> p_test3 = this->template clone<GIntNumCollectionT<int_type>>();

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
			 BOOST_CHECK_NO_THROW(p_test2->template fixedValueInit<double>(2., activityMode::ALLPARAMETERS));
			 BOOST_CHECK(*p_test2 == *p_test1);

			 // Check that multiplication with a fixed floating point value has no effect on this object
			 BOOST_CHECK_NO_THROW(p_test2->template multiplyBy<double>(2., activityMode::ALLPARAMETERS));
			 BOOST_CHECK(*p_test2 == *p_test1);

			 // Check that a component-wise multiplication with a random fp value in a given range does not have an effect on this object
			 BOOST_CHECK_NO_THROW(p_test2->template multiplyByRandom<double>(1., 2., activityMode::ALLPARAMETERS, gr));
			 BOOST_CHECK(*p_test2 == *p_test1);

			 // Check that a component-wise multiplication with a random fp value in the range [0:1[ does not have an effect on this object
			 BOOST_CHECK_NO_THROW(p_test2->template multiplyByRandom<double>(activityMode::ALLPARAMETERS, gr));
			 BOOST_CHECK(*p_test2 == *p_test1);

			 // Check that adding p_test1 to p_test3 does not have an effect
			 BOOST_CHECK_NO_THROW(p_test3->template add<double>(p_test1, activityMode::ALLPARAMETERS));
			 BOOST_CHECK(*p_test3 == *p_test2);

			 // Check that subtracting p_test1 from p_test3 does not have an effect
			 BOOST_CHECK_NO_THROW(p_test3->template subtract<double>(p_test1, activityMode::ALLPARAMETERS));
			 BOOST_CHECK(*p_test3 == *p_test2);
		 }

		 //------------------------------------------------------------------------------

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		 Gem::Common::condnotset("GIntNumCollectionT<>::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
	 }

	 /***************************************************************************/
	 /**
	  * Performs self tests that are expected to fail. This is needed for testing purposes
	  */
	 void specificTestsFailuresExpected_GUnitTests() override {
#ifdef GEM_TESTING
		 // Call the parent class'es function
		 GNumCollectionT<int_type>::specificTestsFailuresExpected_GUnitTests();

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		 Gem::Common::condnotset("GIntNumCollectionT<>::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
	 }

	 /***************************************************************************/
};

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

/******************************************************************************/
// The content of BOOST_SERIALIZATION_ASSUME_ABSTRACT(T)

namespace boost {
namespace serialization {
template<typename int_type>
struct is_abstract<Gem::Geneva::GIntNumCollectionT<int_type>> : public boost::true_type {};
template<typename int_type>
struct is_abstract< const Gem::Geneva::GIntNumCollectionT<int_type>> : public boost::true_type {};
}
}

/******************************************************************************/

