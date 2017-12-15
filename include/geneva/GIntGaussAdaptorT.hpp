/**
 * @file GIntGaussAdaptorT.hpp
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

// Standard headers go here

// Boost headers go here

#ifndef GINTGAUSSADAPTORT_HPP_
#define GINTGAUSSADAPTORT_HPP_

// Geneva headers go here

#include "geneva/GNumGaussAdaptorT.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * The GIntGaussAdaptorT<> class represents an adaptor used for the adaption of
 * integer values through the addition of gaussian-distributed random numbers.
 * See the documentation of GAdaptorT<T> for further information on adaptors
 * in the Geneva context. Most functionality is currently implemented in the
 * GNumGaussAdaptorT parent-class. Note that, for the purpose of adapting integer
 * values, it is generally not useful to choose very small sigma values for the
 * gaussian. A value of 1 might be a good choice. Similarly, the minSigma
 * parameter should be set accordingly, so sigma cannot get too small when
 * being adapted.
 */
template <typename int_type>
class GIntGaussAdaptorT
	:public GNumGaussAdaptorT<int_type, double>
{
	 ///////////////////////////////////////////////////////////////////////
	 friend class boost::serialization::access;

	 template<typename Archive>
	 void serialize(Archive & ar, const unsigned int){
		 using boost::serialization::make_nvp;

		 ar
		 & make_nvp("GNumGaussAdaptorT_int", boost::serialization::base_object<GNumGaussAdaptorT<int_type, double>>(*this));
	 }
	 ///////////////////////////////////////////////////////////////////////

public:
	 /***************************************************************************/
	 /**
	  * The default constructor
	  */
	 GIntGaussAdaptorT()
		 : GNumGaussAdaptorT<int_type, double>(DEFAULTINT32SIGMA, DEFAULTSIGMASIGMA, DEFAULTMINSIGMA, DEFAULTMAXSIGMA)
	 { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * The copy constructor
	  *
	  * @param cp A copy of another GIntGaussAdaptorT<int_type> object
	  */
	 GIntGaussAdaptorT(const GIntGaussAdaptorT<int_type>& cp)
		 : GNumGaussAdaptorT<int_type, double>(cp)
	 { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * Initialization with a adaption probability.  Note that we need to use a different default
	  * value for sigma, as there is a "natural" gap of 1 between integers, and the DEFAULTSIGMA
	  * might not be suitable for us.
	  *
	  * @param adProb The adaption probability
	  */
	 explicit GIntGaussAdaptorT(const double& adProb)
		 : GNumGaussAdaptorT<int_type, double>(DEFAULTINT32SIGMA, DEFAULTSIGMASIGMA, DEFAULTMINSIGMA, DEFAULTMAXSIGMA, adProb)
	 { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * This constructor lets a user set all sigma parameters in one go.
	  *
	  * @param sigma The initial value for the sigma_ parameter
	  * @param sigmaSigma The initial value for the sigmaSigma_ parameter
	  * @param minSigma The minimal value allowed for sigma_
	  * @param maxSigma The maximal value allowed for sigma_
	  */
	 GIntGaussAdaptorT(
		 const double& sigma
		 , const double& sigmaSigma
		 , const double& minSigma
		 , const double& maxSigma
	 )
		 : GNumGaussAdaptorT<int_type, double> (sigma, sigmaSigma, minSigma, maxSigma)
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
	 GIntGaussAdaptorT(
		 const double& sigma
		 , const double& sigmaSigma
		 , const double& minSigma
		 , const double& maxSigma
		 , const double& adProb
	 )
		 : GNumGaussAdaptorT<int_type, double> (sigma, sigmaSigma, minSigma, maxSigma, adProb)
	 { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * The destructor
	  */
	 virtual ~GIntGaussAdaptorT()
	 { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * The standard assignment operator
	  */
	 GIntGaussAdaptorT<int_type>& operator=(const GIntGaussAdaptorT<int_type>& cp) {
		 this->load_(&cp);
		 return *this;
	 }

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

		 // Check that we are dealing with a GIntGaussAdaptorT<int_type> reference independent of this object and convert the pointer
		 const GIntGaussAdaptorT<int_type> *p_load = Gem::Common::g_convert_and_compare<GObject, GIntGaussAdaptorT<int_type>>(cp, this);

		 GToken token("GIntGaussAdaptorT<int_type>", e);

		 // Compare our parent data ...
		 Gem::Common::compare_base<GNumGaussAdaptorT<int_type, double>>(IDENTITY(*this, *p_load), token);

		 // // ... no local data

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
		 return std::string("GIntGaussAdaptorT");
	 }

protected:
	 /***************************************************************************/
	 /**
	  * Loads the data of another GObject
	  *
	  * @param cp A copy of another GIntGaussAdaptorT<int_type> object, camouflaged as a GObject
	  */
	 void load_(const GObject* cp) override {
		 // Convert the pointer to our target type and check for self-assignment
		 const GIntGaussAdaptorT<int_type> * p_load = Gem::Common::g_convert_and_compare<GObject, GIntGaussAdaptorT<int_type>>(cp, this);

		 // Load our parent class'es data ...
		 GNumGaussAdaptorT<int_type, double>::load_(cp);

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
		 int_type& value
		 , const int_type& range
		 , Gem::Hap::GRandomBase& gr
	 ) override {
		 using namespace Gem::Common;
		 using namespace Gem::Hap;

		 // Calculate a suitable addition to the current parameter value
		 int_type addition = static_cast<int_type>(
			 static_cast<double>(range)
			 * GAdaptorT<int_type, double>::m_normal_distribution(
				 gr
				 , std::normal_distribution<double>::param_type(0., this->getSigma())
			 )
		 );

		 if(addition == 0) { // Enforce a minimal change of 1.
			 bool flipDirection = GAdaptorT<int_type, double>::m_weighted_bool(gr, std::bernoulli_distribution::param_type(0.5));
			 addition = flipDirection?1:-1;
		 }

		 // adapt the value in situ. Note that this changes
		 // the argument of this function
		 value += addition;
	 }

	 /* ----------------------------------------------------------------------------------
	  * - Tested in GNumGaussAdaptorT<int_type, double>::specificTestsNoFailuresExpected_GUnitTests()
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
	  *
	  * @return A boolean which indicates whether modifications were made
	  */
	 bool modify_GUnitTests() override {
#ifdef GEM_TESTING
		 bool result = false;

		 // Call the parent class'es function
		 if(GNumGaussAdaptorT<int_type, double>::modify_GUnitTests()) result = true;

		 return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		 Gem::Common::condnotset("GIntGaussAdaptorT<>::modify_GUnitTests", "GEM_TESTING");
		return false;
#endif /* GEM_TESTING */
	 }

	 /***************************************************************************/
	 /**
	  * Performs self tests that are expected to succeed. This is needed for testing purposes
	  */
	 void specificTestsNoFailureExpected_GUnitTests() override {
#ifdef GEM_TESTING
		 // Call the parent class'es function
		 GNumGaussAdaptorT<int_type, double>::specificTestsNoFailureExpected_GUnitTests();

		 //------------------------------------------------------------------------------
		 // nothing yet
		 //------------------------------------------------------------------------------

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		 Gem::Common::condnotset("GIntGaussAdaptorT<>::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
	 }

	 /***************************************************************************/
	 /**
	  * Performs self tests that are expected to fail. This is needed for testing purposes
	  */
	 void specificTestsFailuresExpected_GUnitTests() override {
#ifdef GEM_TESTING
		 // Call the parent class'es function
		 GNumGaussAdaptorT<int_type, double>::specificTestsFailuresExpected_GUnitTests();
#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		 Gem::Common::condnotset("GIntGaussAdaptorT<>::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
	 }

	 /***************************************************************************/
};


} /* namespace Geneva */
} /* namespace Gem */

/******************************************************************************/
// The content of BOOST_SERIALIZATION_ASSUME_ABSTRACT(T)

namespace boost {
namespace serialization {
template<typename int_type>
struct is_abstract<Gem::Geneva::GIntGaussAdaptorT<int_type>> : public boost::true_type {};
template<typename int_type>
struct is_abstract< const Gem::Geneva::GIntGaussAdaptorT<int_type>> : public boost::true_type {};
}
}
/******************************************************************************/

#endif /* GINTGAUSSADAPTORT_HPP_ */
