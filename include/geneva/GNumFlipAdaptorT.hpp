/**
 * @file GNumFlipAdaptorT.hpp
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
#include "common/GExceptions.hpp"
#include "geneva/GAdaptorT.hpp"
#include "geneva/GConstrainedDoubleObject.hpp"
#include "geneva/GDoubleGaussAdaptor.hpp"
#include "geneva/GObject.hpp"
#include "geneva/GOptimizationEnums.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * GNumFlipAdaptorT represents an adaptor used for the adaption of numeric
 * types, by flipping a number to the next larger or smaller one. The unerlying
 * type needs to be specified as a template parameter.
 */
template<typename num_type>
class GNumFlipAdaptorT
	:public GAdaptorT<num_type>
{
	 ///////////////////////////////////////////////////////////////////////
	 friend class boost::serialization::access;

	 template<typename Archive>
	 void serialize(Archive & ar, const unsigned int) {
		 using boost::serialization::make_nvp;
		 ar
		 & make_nvp("GAdaptorT", boost::serialization::base_object<GAdaptorT<num_type>>(*this));
	 }
	 ///////////////////////////////////////////////////////////////////////

	 // Make sure this class can only be instantiated with num_type as an arithmetic type
	 static_assert(
		 std::is_arithmetic<num_type>::value
		 , "num_type should be an arithmetic type"
	 );

public:
	 /***************************************************************************/
	 /**
	  * The standard constructor.
	  */
	 GNumFlipAdaptorT()
		 : GAdaptorT<num_type> (DEFAULTADPROB)
	 { /* nothing */	}

	 /***************************************************************************/
	 /**
	  * This constructor takes an argument, that specifies the (initial) probability
	  * for the adaption of an integer or bit value
	  *
	  * @param prob The probability for a flip
	  */
	 explicit GNumFlipAdaptorT(const double& adProb)
		 : GAdaptorT<num_type>(adProb)
	 { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * A standard copy constructor.
	  *
	  * @param cp Another GNumFlipAdaptorT object
	  */
	 GNumFlipAdaptorT(const GNumFlipAdaptorT<num_type>& cp)
		 : GAdaptorT<num_type>(cp)
	 { /* nothing */	}

	 /***************************************************************************/
	 /**
	  * The standard destructor. Empty, as we have no local, dynamically
	  * allocated data.
	  */
	 virtual ~GNumFlipAdaptorT()
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

		 // Check that we are dealing with a GNumFlipAdaptorT<num_type> reference independent of this object and convert the pointer
		 const GNumFlipAdaptorT<num_type> *p_load = Gem::Common::g_convert_and_compare<GObject, GNumFlipAdaptorT<num_type>>(cp, this);

		 GToken token("GNumFlipAdaptorT<num_type>", e);

		 // Compare our parent data ...
		 Gem::Common::compare_base<GAdaptorT<num_type>>(IDENTITY(*this, *p_load), token);

		 // ... no local data

		 // React on deviations from the expectation
		 token.evaluate();
	 }

	 /***************************************************************************/
	 /**
	  * Retrieves the id of the adaptor. Purely virtual, as we do not want this class to be
	  * instantiated.
	  *
	  * @return The id of the adaptor
	  */
	 Gem::Geneva::adaptorId getAdaptorId() const override = 0;

	 /* ----------------------------------------------------------------------------------
	  * Tested in GInt32FlipAdaptor::specificTestsNoFailuresExpected_GUnitTests()
	  * Tested in GBooleanAdaptor::specificTestsNoFailuresExpected_GUnitTests()
	  * ----------------------------------------------------------------------------------
	  */

	 /***************************************************************************/
	 /**
	  * Emits a name for this class / object
	  */
	 std::string name() const override {
		 return std::string("GNumFlipAdaptorT");
	 }

protected:
	 /***************************************************************************/
	 /**
    * Allows to randomly initialize parameter members. No local data, hence no
    * action taken.
    */
	 bool randomInit(Gem::Hap::GRandomBase&) override {
		 return false;
	 }

	 /***************************************************************************/
	 /**
	  * This function loads the data of another GNumFlipAdaptorT, camouflaged as a GObject.
	  *
	  * @param A copy of another GNumFlipAdaptorT, camouflaged as a GObject
	  */
	 void load_(const GObject *cp) override	{
		 // Convert the pointer to our target type and check for self-assignment
		 const GNumFlipAdaptorT<num_type> * p_load = Gem::Common::g_convert_and_compare<GObject, GNumFlipAdaptorT<num_type>>(cp, this);

		 // Load the data of our parent class ...
		 GAdaptorT<num_type>::load_(cp);

		 // no local data
	 }

	 /***************************************************************************/
	 /**
	  * Flip the value up or down by 1, depending on a random number.
	  *
	  * @param value The bit value to be adapted
	  * @param range A typical range for the parameter with type T (unused here)
	  */
	 inline virtual void customAdaptions(
		 num_type& value
		 , const num_type& range
		 , Gem::Hap::GRandomBase& gr
	 ) override {
		 using namespace Gem::Common;
		 using namespace Gem::Hap;

		 if(GAdaptorT<num_type>::m_weighted_bool(gr, std::bernoulli_distribution::param_type(0.5))){
			 value += 1;
		 } else {
			 value -= 1;
		 }
	 }

	 /* ----------------------------------------------------------------------------------
	  * Tested in GAdaptorT<num_type>::specificTestsNoFailuresExpected_GUnitTests()
	  * ----------------------------------------------------------------------------------
	  */

private:
	 /***************************************************************************/
	 /**
	  * This function creates a deep copy of this object. The function is purely virtual, as we
	  * do not want this class to be instantiated. Use the derived classes instead.
	  *
	  * @return A deep copy of this object
	  */
	 GObject *clone_() const override = 0;

public:
	 /***************************************************************************/
	 /**
	  * Applies modifications to this object. This is needed for testing purposes
	  *
	  * @return A boolean which indicates whether modifications were made
	  */
	 bool modify_GUnitTests() override {
#ifdef GEM_TESTING
		 using boost::unit_test_framework::test_suite;
		 using boost::unit_test_framework::test_case;

		 bool result = false;

		 // Call the parent classes' functions
		 if(GAdaptorT<num_type>::modify_GUnitTests()) result = true;

		 // no local data -- nothing to change

		 return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		 Gem::Common::condnotset("GNumFlipAdaptorT<>::modify_GUnitTests", "GEM_TESTING");
		return false;
#endif /* GEM_TESTING */
	 }

	 /***************************************************************************/
	 /**
	  * Performs self tests that are expected to succeed. This is needed for testing purposes
	  */
	 void specificTestsNoFailureExpected_GUnitTests() override {
#ifdef GEM_TESTING
		 using boost::unit_test_framework::test_suite;
		 using boost::unit_test_framework::test_case;

		 // Call the parent classes' functions
		 GAdaptorT<num_type>::specificTestsNoFailureExpected_GUnitTests();

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		 Gem::Common::condnotset("GNumFlipAdaptorT<>::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
	 }

	 /***************************************************************************/
	 /**
	  * Performs self tests that are expected to fail. This is needed for testing purposes
	  */
	 void specificTestsFailuresExpected_GUnitTests() override {
#ifdef GEM_TESTING
		 using boost::unit_test_framework::test_suite;
		 using boost::unit_test_framework::test_case;

		 // Call the parent classes' functions
		 GAdaptorT<num_type>::specificTestsFailuresExpected_GUnitTests();

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		 Gem::Common::condnotset("GNumFlipAdaptorT<>::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
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
template<typename num_type>
struct is_abstract<Gem::Geneva::GNumFlipAdaptorT<num_type>> : public boost::true_type {};
template<typename num_type>
struct is_abstract< const Gem::Geneva::GNumFlipAdaptorT<num_type>> : public boost::true_type {};
}
}

/******************************************************************************/

