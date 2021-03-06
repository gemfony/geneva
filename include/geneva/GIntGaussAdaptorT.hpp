/********************************************************************************
 *
 * This file is part of the Geneva library collection. The following license
 * applies to this file:
 *
 * ------------------------------------------------------------------------------
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * ------------------------------------------------------------------------------
 *
 * Note that other files in the Geneva library collection may use a different
 * license. Please see the licensing information in each file.
 *
 ********************************************************************************
 *
 * Geneva was started by Dr. Rüdiger Berlich and was later maintained together
 * with Dr. Ariel Garcia under the auspices of Gemfony scientific. For further
 * information on Gemfony scientific, see http://www.gemfomy.eu .
 *
 * The majority of files in Geneva was released under the Apache license v2.0
 * in February 2020.
 *
 * See the NOTICE file in the top-level directory of the Geneva library
 * collection for a list of contributors and copyright information.
 *
 ********************************************************************************/

#pragma once

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard headers go here

// Boost headers go here

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
	: public GNumGaussAdaptorT<int_type, double>
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
	 GIntGaussAdaptorT(const GIntGaussAdaptorT<int_type>& cp) = default;

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
	 ~GIntGaussAdaptorT() = default;

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
	/** @brief Allow access to this classes compare_ function */
	friend void Gem::Common::compare_base_t<GIntGaussAdaptorT<int_type>>(
		GIntGaussAdaptorT<int_type> const &
		, GIntGaussAdaptorT<int_type> const &
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

		// Check that we are dealing with a GIntGaussAdaptorT<int_type> reference independent of this object and convert the pointer
		const GIntGaussAdaptorT<int_type> *p_load = Gem::Common::g_convert_and_compare<GObject, GIntGaussAdaptorT<int_type>>(cp, this);

		GToken token("GIntGaussAdaptorT<int_type>", e);

		// Compare our parent data ...
		Gem::Common::compare_base_t<GNumGaussAdaptorT<int_type, double>>(*this, *p_load, token);

		// // ... no local data

		// React on deviations from the expectation
		token.evaluate();
	}

	/***************************************************************************/
	 /**
	  * The actual adaption of the supplied value takes place here.
	  *
	  * @param value The value that is going to be adapted in situ
	  * @param range A typical range for the parameter with type num_type
	  */
	 void customAdaptions(
		 int_type& value
		 , const int_type& range
		 , Gem::Hap::GRandomBase& gr
	 ) override {
		 using namespace Gem::Common;
		 using namespace Gem::Hap;

		 // Calculate a suitable addition to the current parameter value
		 auto addition = static_cast<int_type>(
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

	/***************************************************************************/
	/**
     * Applies modifications to this object. This is needed for testing purposes
     *
     * @return A boolean which indicates whether modifications were made
     */
	bool modify_GUnitTests_() override {
#ifdef GEM_TESTING
		bool result = false;

		// Call the parent class'es function
		if(GNumGaussAdaptorT<int_type, double>::modify_GUnitTests_()) result = true;

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
	void specificTestsNoFailureExpected_GUnitTests_() override {
#ifdef GEM_TESTING
		// Call the parent class'es function
		GNumGaussAdaptorT<int_type, double>::specificTestsNoFailureExpected_GUnitTests_();

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
	void specificTestsFailuresExpected_GUnitTests_() override {
#ifdef GEM_TESTING
		// Call the parent class'es function
		GNumGaussAdaptorT<int_type, double>::specificTestsFailuresExpected_GUnitTests_();
#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		Gem::Common::condnotset("GIntGaussAdaptorT<>::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
	}

	/***************************************************************************/

private:
	/***************************************************************************/
	/** @brief Retrieves the id of this adaptor */
	Gem::Geneva::adaptorId getAdaptorId_() const override = 0;

	 /***************************************************************************/
	 /**
	  * Emits a name for this class / object
	  */
	 std::string name_() const override {
		 return std::string("GIntGaussAdaptorT");
	 }

	 /***************************************************************************/
	 /** @brief Creates a deep clone of this object. */
	 GObject* clone_() const override = 0;
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
