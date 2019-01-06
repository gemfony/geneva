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

// Standard headers go here
#include <type_traits>

// Boost headers go here

// Geneva headers go here
#include "geneva/GNumT.hpp"
#include "geneva/GInt32GaussAdaptor.hpp"
#include "geneva/GInt32FlipAdaptor.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * This class encapsulates a single integer value, which can assume different integer types.
 * The reason for this class is that there might be applications where one might want different
 * adaptor characteristics for different values. This cannot be done with a GIntCollectionT.
 */
template <typename int_type>
class GNumIntT
	: public GNumT<int_type>
{
	 ///////////////////////////////////////////////////////////////////////
	 friend class boost::serialization::access;

	 template<typename Archive>
	 void serialize(Archive & ar, const unsigned int){
		 using boost::serialization::make_nvp;

		 ar & make_nvp("GNumT", boost::serialization::base_object<GNumT<int_type>>(*this));
	 }
	 ///////////////////////////////////////////////////////////////////////

	 // Make sure this class can only be instantiated if int_type is a *signed* integer type
	 static_assert(
		 std::is_signed<int_type>::value
		 , "int_type should be a signed iteger type"
	 );

public:
	 /** @brief Specifies the type of parameters stored in this object */
	 using parameter_type = int_type;

	 /***************************************************************************/
	 /**
	  * The default constructor
	  */
	 GNumIntT() = default;

	 /***************************************************************************/
	 /**
	  * The copy constructor
	  *
	  * @param cp A constant reference to another GNumIntT<int_type> object
	  */
	 GNumIntT(const GNumIntT<int_type>& cp) = default;

	 /***************************************************************************/
	 /**
	  * Initialization by contained value
	  *
	  * @param val The value used for the initialization
	  */
	 explicit GNumIntT(const int_type& val)
		 : GNumT<int_type>(val)
	 { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * Initialization by random number in a given range.
	  *
	  * @param min The lower boundary for random entries
	  * @param max The upper boundary for random entries
	  */
	 GNumIntT(
		 const int_type& min
		 , const int_type& max
	 )
		 : GNumT<int_type> (min, max)
	 {
		 Gem::Hap::GRandomT<Gem::Hap::RANDFLAVOURS::RANDOMLOCAL> gr;
		 GNumIntT<int_type>::randomInit(activityMode::ACTIVEONLY, gr);
	 }

	 /***************************************************************************/
	 /**
	  * Initialization with a fixed value, plus the boundaries for random
	  * initialization.
	  *
	  * @param val The value to be assigned to the object
	  * @param min The lower boundary for random entries
	  * @param max The upper boundary for random entries
	  */
	 GNumIntT(
		 const int_type& val
		 , const int_type& min
		 , const int_type& max
	 )
		 : GNumT<int_type> (min, max)
	 {
		 GParameterT<int_type>::setValue(val);
	 }

	 /***************************************************************************/
	 /**
	  * The destructor
	  */
	 ~GNumIntT() override = default;

	 /***************************************************************************/
	 /**
	  * An assignment operator for the contained value type
	  *
	  * @param val The value to be assigned to this object
	  * @return The value that was assigned to this object
	  */
	 GNumT<int_type>& operator=(const int_type& val) override {
		 GNumT<int_type>::operator=(val);
		 return *this;
	 }

protected:
	 /***************************************************************************/
	 /**
	  * Loads the data of another GNumIntT<int_type> object,
	  * camouflaged as a GObject. We have no local data, so
	  * all we need to do is to the standard identity check,
	  * preventing that an object is assigned to itself.
	  *
	  * @param cp A copy of another GNumIntT<int_type> object, camouflaged as a GObject
	  */
	 void load_(const GObject *cp) override {
		 // Check that we are dealing with a GNumIntT<int_type> reference independent of this object and convert the pointer
		 const GNumIntT<int_type> *p_load = Gem::Common::g_convert_and_compare<GObject, GNumIntT<int_type>>(cp, this);

		 // Load our parent class'es data ...
		 GNumT<int_type>::load_(cp);

		 // no local data ...
	 }

	/***************************************************************************/
	/** @brief Allow access to this classes compare_ function */
	friend void Gem::Common::compare_base_t<GNumIntT<int_type>>(
		GNumIntT<int_type> const &
		, GNumIntT<int_type> const &
		, Gem::Common::GToken &
	);

	/***************************************************************************/
	/**
     * Searches for compliance with expectations with respect to another object
     * of the same type
     *
     * @param cp A constant reference to another GObject object
     * @param e The expected outcome of the comparison
     * @param limit The maximum deviation for floating point values (important for similarity checks)
     */
	void compare_(
		const GObject& cp
		, const Gem::Common::expectation& e
		, const double& limit
	) const override {
		using namespace Gem::Common;

		// Check that we are dealing with a GNumIntT<int_type> reference independent of this object and convert the pointer
		const GNumIntT<int_type> *p_load = Gem::Common::g_convert_and_compare<GObject, GNumIntT<int_type>>(cp, this);

		GToken token("GNumIntT<int_type>", e);

		// Compare our parent data ...
		Gem::Common::compare_base_t<GNumT<int_type>>(*this, *p_load, token);

		// ... no local data

		// React on deviations from the expectation
		token.evaluate();
	}

	/***************************************************************************/
	 /**
	  * Triggers random initialization of the parameter collection
	  */
	 bool randomInit_(
		 const activityMode& am
		 , Gem::Hap::GRandomBase& gr
	 ) override {
		 int_type lowerBoundary = GNumT<int_type>::getLowerInitBoundary();
		 int_type upperBoundary = GNumT<int_type>::getUpperInitBoundary();

		 typename std::uniform_int_distribution<int_type> uniform_int(lowerBoundary, upperBoundary);

		 // uniform_int produces random numbers that include the upper boundary.
		 GParameterT<int_type>::setValue(uniform_int(gr));

		 return true;
	 }

	/***************************************************************************/
	/**
     * Applies modifications to this object. This is needed for testing purposes
     */
	bool modify_GUnitTests_() override {
#ifdef GEM_TESTING
		bool result = false;

		// Call the parent classes' functions^
		if(GNumT<int_type>::modify_GUnitTests_()) result = true;

		return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		Gem::Common::condnotset("GNumIntT<>::modify_GUnitTests", "GEM_TESTING");
		return false;
#endif /* GEM_TESTING */
	}

	/***************************************************************************/
	/**
     * Performs self tests that are expected to succeed. This is needed for testing purposes
     */
	void specificTestsNoFailureExpected_GUnitTests_() override {
#ifdef GEM_TESTING
		// A few settings
		const std::size_t nTests = 10000;
		const int_type LOWERINITBOUNDARY = int_type(0); // >= 0, as int_type might be unsigned
		const int_type UPPERINITBOUNDARY =  int_type(10);
		const int_type FIXEDVALUEINIT = int_type(1);

		// Call the parent classes' functions
		GNumT<int_type>::specificTestsNoFailureExpected_GUnitTests_();

		// A random generator
		Gem::Hap::GRandomT<Gem::Hap::RANDFLAVOURS::RANDOMPROXY> gr;

		//------------------------------------------------------------------------------

		{ // Initialize with a fixed value, then check setting and retrieval of boundaries and random initialization
			std::shared_ptr<GNumIntT<int_type>> p_test1 = this->template clone<GNumIntT<int_type>>();
			std::shared_ptr<GNumIntT<int_type>> p_test2 = this->template clone<GNumIntT<int_type>>();

			// Assign a boolean value true
			BOOST_CHECK_NO_THROW(*p_test1 = 2*UPPERINITBOUNDARY); // Make sure random initialization cannot randomly result in an unchanged value
			// Cross-check
			BOOST_CHECK(p_test1->value() == 2*UPPERINITBOUNDARY);

			// Set initialization boundaries
			BOOST_CHECK_NO_THROW(p_test1->setInitBoundaries(LOWERINITBOUNDARY, UPPERINITBOUNDARY));

			// Check that the boundaries have been set as expected
			BOOST_CHECK(p_test1->getLowerInitBoundary() == LOWERINITBOUNDARY);
			BOOST_CHECK(p_test1->getUpperInitBoundary() == UPPERINITBOUNDARY);

			// Load the data of p_test1 into p_test2
			BOOST_CHECK_NO_THROW(p_test2->load(p_test1));
			// Cross check that both are indeed equal
			BOOST_CHECK(*p_test1 == *p_test2);

			// Check that the values of p_test1 are inside of the allowed boundaries
			for(std::size_t i=0; i<nTests; i++) {
				BOOST_CHECK_NO_THROW(p_test1->randomInit_(activityMode::ALLPARAMETERS, gr));
				BOOST_CHECK(p_test1->value() >= LOWERINITBOUNDARY);
				BOOST_CHECK(p_test1->value() <= UPPERINITBOUNDARY);
				BOOST_CHECK(p_test1->value() != p_test2->value());
			}
		}

		//------------------------------------------------------------------------------

		{ // Check that the fp-family of functions doesn't have an effect on this object
			std::shared_ptr<GNumIntT<int_type>> p_test1 = this->template clone<GNumIntT<int_type>>();
			std::shared_ptr<GNumIntT<int_type>> p_test2 = this->template clone<GNumIntT<int_type>>();
			std::shared_ptr<GNumIntT<int_type>> p_test3 = this->template clone<GNumIntT<int_type>>();

			// Assign a boolean value true
			BOOST_CHECK_NO_THROW(*p_test1 = FIXEDVALUEINIT); // Make sure random initialization cannot randomly result in an unchanged value
			// Cross-check
			BOOST_CHECK(p_test1->value() == FIXEDVALUEINIT);

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
		Gem::Common::condnotset("GNumIntT<>::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
	}

	/***************************************************************************/
	/**
     * Performs self tests that are expected to fail. This is needed for testing purposes
     */
	void specificTestsFailuresExpected_GUnitTests_() override {
#ifdef GEM_TESTING
		// Call the parent classes' functions
		GNumT<int_type>::specificTestsFailuresExpected_GUnitTests_();

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		Gem::Common::condnotset("GNumIntT<>::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
	}

	/***************************************************************************/

private:
	 /***************************************************************************/
	 /**
	  * Emits a name for this class / object
	  */
	 std::string name_() const override {
		 return std::string("GNumIntT");
	 }

	 /***************************************************************************/
	 /** @brief Creates a deep clone of this object. Needs to be redefined in derived classes */
	 GObject* clone_() const override = 0;
};

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

/******************************************************************************/
// The content of BOOST_SERIALIZATION_ASSUME_ABSTRACT(T)

namespace boost {
namespace serialization {
template<typename int_type>
struct is_abstract<Gem::Geneva::GNumIntT<int_type>> : public boost::true_type {};
template<typename int_type>
struct is_abstract< const Gem::Geneva::GNumIntT<int_type>> : public boost::true_type {};
}
}

/******************************************************************************/

