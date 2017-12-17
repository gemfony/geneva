/**
 * @file GFPGaussAdaptorT.hpp
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

#ifndef GFPGAUSSADAPTORT_HPP_
#define GFPGAUSSADAPTORT_HPP_

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard headers go here
#include <type_traits>

// Boost headers go here

// Geneva headers go here
#include "geneva/GNumGaussAdaptorT.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * The GFPGaussAdaptorT represents an adaptor used for the adaption of
 * floating point values through the addition of gaussian-distributed random numbers.
 * See the documentation of GNumGaussAdaptorT<T> for further information on adaptors
 * in the Geneva context. This class is at the core of evolutionary strategies,
 * as implemented by this library. It is now implemented through a generic
 * base class that can also be used to adapt other numeric types.
 */
template<typename fp_type>
class GFPGaussAdaptorT
	:public GNumGaussAdaptorT<fp_type, fp_type>
{
	 ///////////////////////////////////////////////////////////////////////
	 friend class boost::serialization::access;

	 template<typename Archive>
	 void serialize(Archive & ar, const unsigned int){
		 using boost::serialization::make_nvp;

		 ar
		 & make_nvp("GNumGaussAdaptorT_fp_type", boost::serialization::base_object<GNumGaussAdaptorT<fp_type, fp_type>>(*this));
	 }
	 ///////////////////////////////////////////////////////////////////////

	 // Make sure this class can only be instantiated if fp_type really is a floating point type
	 static_assert(
		 std::is_floating_point<fp_type>::value
		 , "fp_type should be a floating point type"
	 );

public:
	 /***************************************************************************/
	 /**
	  * The default constructor
	  */
	 GFPGaussAdaptorT()
	 { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * The copy constructor
	  *
	  * @param cp A copy of another GFPGaussAdaptorT object
	  */
	 GFPGaussAdaptorT(const GFPGaussAdaptorT<fp_type>& cp)
		 : GNumGaussAdaptorT<fp_type, fp_type>(cp)
	 { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * Initialization with a adaption probability
	  *
	  * @param adProb The adaption probability
	  */
	 explicit GFPGaussAdaptorT(const double& adProb)
		 : GNumGaussAdaptorT<fp_type, fp_type>(adProb)
	 { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * Initialization with a number of values belonging to
	  * the width of the gaussian.
	  *
	  * @param sigma The initial value for the sigma_ parameter
	  * @param sigmaSigma The initial value for the sigmaSigma_ parameter
	  * @param minSigma The minimal value allowed for sigma_
	  * @param maxSigma The maximal value allowed for sigma_
	  */
	 GFPGaussAdaptorT(
		 const fp_type& sigma
		 , const fp_type& sigmaSigma
		 , const fp_type& minSigma
		 , const fp_type& maxSigma
	 )
		 : GNumGaussAdaptorT<fp_type, fp_type> (sigma, sigmaSigma, minSigma, maxSigma)
	 { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * This constructor lets a user set all sigma parameters, as well as the adaption
	  * probability in one go.
	  *
	  * @param sigma The initial value for the sigma_ parameter
	  * @param sigmaSigma The initial value for the sigmaSigma_ parameter
	  * @param minSigma The minimal value allowed for sigma_
	  * @param maxSigma The maximal value allowed for sigma_
	  * @param adProb The adaption probability
	  */
	 GFPGaussAdaptorT(
		 const fp_type& sigma
		 , const fp_type& sigmaSigma
		 , const fp_type& minSigma
		 , const fp_type& maxSigma
		 , const double& adProb
	 )
		 : GNumGaussAdaptorT<fp_type, fp_type> (sigma, sigmaSigma, minSigma, maxSigma, adProb)
	 { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * The destructor
	  */
	 virtual ~GFPGaussAdaptorT()
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

		 // Check that we are dealing with a GFPGaussAdaptorT<fp_type> reference independent of this object and convert the pointer
		 const GFPGaussAdaptorT<fp_type> *p_load = Gem::Common::g_convert_and_compare<GObject, GFPGaussAdaptorT<fp_type>>(cp, this);

		 GToken token("GFPGaussAdaptorT<fp_type>", e);

		 // Compare our parent data ...
		 Gem::Common::compare_base<GNumGaussAdaptorT<fp_type, fp_type>>(IDENTITY(*this, *p_load), token);

		 // ... no local data

		 // React on deviations from the expectation
		 token.evaluate();
	 }

	 /***************************************************************************/
	 /** @brief Retrieves the id of this adaptor */
	 Gem::Geneva::adaptorId getAdaptorId() const override = 0;

	 /***************************************************************************/
	 /**
	  * Emits a name for this class / object
	  */
	 std::string name() const override {
		 return std::string("GFPGaussAdaptorT");
	 }

protected:
	 /***************************************************************************/
	 /**
	  * Loads the data of another object of this type
	  *
	  * @param cp A copy of another GFPGaussAdaptorT<fp_type> object, camouflaged as a GObject
	  */
	 void load_(const GObject* cp) override {
		 // Convert the pointer to our target type and check for self-assignment
		 const GFPGaussAdaptorT<fp_type> * p_load = Gem::Common::g_convert_and_compare<GObject, GFPGaussAdaptorT<fp_type>>(cp, this);

		 // Load our parent class'es data ...
		 GNumGaussAdaptorT<fp_type, fp_type>::load_(cp);

		 // ... no local data
	 }

	 /***************************************************************************/
	 /**
	  * The actual adaption of the supplied value takes place here.
	  *
	  * @param value The value that is going to be adapted in situ
	  * @param range A typical range for the parameter with type num_type
	  */
	 virtual void customAdaptions(
		 fp_type& value
		 , const fp_type& range
		 , Gem::Hap::GRandomBase& gr
	 ) override {
		 using namespace Gem::Common;
		 using namespace Gem::Hap;

		 // adapt the value in situ. Note that this changes
		 // the argument of this function
		 value += (range * GAdaptorT<fp_type, fp_type>::m_normal_distribution(gr, typename std::normal_distribution<fp_type>::param_type(0., GNumGaussAdaptorT<fp_type, fp_type>::sigma_)));
	 }

	 /* ----------------------------------------------------------------------------------
	  * - Tested in GNumGaussAdaptorT<T>::specificTestsNoFailuresExpected_GUnitTests()
	  * ----------------------------------------------------------------------------------
	  */

private:
	 /***************************************************************************/
	 /** @brief Creates a deep clone of this object. */
	 GObject* clone_() const override = 0;

public:
	 /***************************************************************************/
	 /**
	  * Applies modifications to this object. This is needed for testing purposes
	  */
	 bool modify_GUnitTests() override {
#ifdef GEM_TESTING
		 using boost::unit_test_framework::test_suite;
		 using boost::unit_test_framework::test_case;

		 bool result = false;

		 // Call the parent class'es function
		 if(GNumGaussAdaptorT<fp_type, fp_type>::modify_GUnitTests()) result = true;

		 return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		 Gem::Common::condnotset("GFPGaussAdaptorT<>::modify_GUnitTests", "GEM_TESTING");
		return false;
#endif /* GEM_TESTING */
	 }

	 /***************************************************************************/
	 /**
	  * Performs self tests that are expected to succeed. This is needed for
	  * testing purposes
	  */
	 void specificTestsNoFailureExpected_GUnitTests() override {
#ifdef GEM_TESTING
		 using boost::unit_test_framework::test_suite;
		 using boost::unit_test_framework::test_case;

		 // Call the parent class'es function
		 GNumGaussAdaptorT<fp_type, fp_type>::specificTestsNoFailureExpected_GUnitTests();

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
		 Gem::Common::condnotset("GFPGaussAdaptorT<>::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
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

		 // Call the parent class'es function
		 GNumGaussAdaptorT<fp_type, fp_type>::specificTestsFailuresExpected_GUnitTests();

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
		 Gem::Common::condnotset("GFPGaussAdaptorT<>::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
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
struct is_abstract<Gem::Geneva::GFPGaussAdaptorT<fp_type>> : public boost::true_type {};
template<typename fp_type>
struct is_abstract< const Gem::Geneva::GFPGaussAdaptorT<fp_type>> : public boost::true_type {};
}
}
/******************************************************************************/

#endif /* GFPGAUSSADAPTORT_HPP_ */
