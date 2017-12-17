/**
 * @file GNumGaussAdaptorT.hpp
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
#include <tuple>

// Boost headers go here

#ifndef GNUMGAUSSADAPTORT_HPP_
#define GNUMGAUSSADAPTORT_HPP_

// Geneva headers go here
#include "geneva/GAdaptorT.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * GNumGaussAdaptorT represents an adaptor used for the adaption of numeric
 * types, by the addition of gaussian-distributed random numbers. Different numeric
 * types may be used, including Boost's integer representations.
 * The type used needs to be specified as a template parameter.
 */
template<typename num_type, typename fp_type>
class GNumGaussAdaptorT
	:public GAdaptorT<num_type, fp_type>
{
	 ///////////////////////////////////////////////////////////////////////
	 friend class boost::serialization::access;

	 template<typename Archive>
	 void serialize(Archive & ar, const unsigned int) {
		 using boost::serialization::make_nvp;

		 ar
		 & make_nvp("GAdaptorT_num", boost::serialization::base_object<GAdaptorT<num_type>>(*this))
		 & BOOST_SERIALIZATION_NVP(sigma_)
		 & BOOST_SERIALIZATION_NVP(sigma_reset_)
		 & BOOST_SERIALIZATION_NVP(sigmaSigma_)
		 & BOOST_SERIALIZATION_NVP(minSigma_)
		 & BOOST_SERIALIZATION_NVP(maxSigma_);
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
	  * The standard constructor.
	  */
	 GNumGaussAdaptorT()
		 : GAdaptorT<num_type> ()
			, sigma_(fp_type(DEFAULTSIGMA))
			, sigma_reset_(sigma_)
			, sigmaSigma_(fp_type(DEFAULTSIGMASIGMA))
			, minSigma_(fp_type(DEFAULTMINSIGMA))
			, maxSigma_(fp_type(DEFAULTMAXSIGMA))
	 { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * Initialization of the parent class'es adaption probability.
	  *
	  * @param probability The likelihood for a adaption actually taking place
	  */
	 GNumGaussAdaptorT(const double& probability)
		 : GAdaptorT<num_type> (probability)
			, sigma_(fp_type(DEFAULTSIGMA))
			, sigma_reset_(sigma_)
			, sigmaSigma_(fp_type(DEFAULTSIGMASIGMA))
			, minSigma_(fp_type(DEFAULTMINSIGMA))
			, maxSigma_(fp_type(DEFAULTMAXSIGMA))
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
	 GNumGaussAdaptorT (
		 const fp_type& sigma
		 , const fp_type& sigmaSigma
		 , const fp_type& minSigma
		 , const fp_type& maxSigma
	 )
		 : GAdaptorT<num_type> ()
			, sigma_(fp_type(DEFAULTSIGMA))
			, sigma_reset_(fp_type(DEFAULTSIGMA))
			, sigmaSigma_(fp_type(DEFAULTSIGMASIGMA))
			, minSigma_(fp_type(DEFAULTMINSIGMA))
			, maxSigma_(fp_type(DEFAULTMAXSIGMA))
	 {
		 // These functions do error checks on their values
		 setSigmaAdaptionRate(sigmaSigma);
		 setSigmaRange(minSigma, maxSigma);
		 setSigma(sigma); // Must be set last so an error check for compliance with the boundaries can be made

		 sigma_reset_ = sigma_;
	 }

	 /***************************************************************************/
	 /**
	  * This constructor lets a user set all parameters in one go.
	  *
	  * @param sigma The initial value for the sigma_ parameter
	  * @param sigmaSigma The initial value for the sigmaSigma_ parameter
	  * @param minSigma The minimal value allowed for sigma_
	  * @param maxSigma The maximal value allowed for sigma_
	  * @param probability The likelihood for a adaption actually taking place
	  */
	 GNumGaussAdaptorT (
		 const fp_type& sigma
		 , const fp_type& sigmaSigma
		 , const fp_type& minSigma
		 , const fp_type& maxSigma
		 , const double& probability
	 )
		 : GAdaptorT<num_type> (probability)
			, sigma_(fp_type(DEFAULTSIGMA))
			, sigma_reset_(fp_type(DEFAULTSIGMA))
			, sigmaSigma_(fp_type(DEFAULTSIGMASIGMA))
			, minSigma_(fp_type(DEFAULTMINSIGMA))
			, maxSigma_(fp_type(DEFAULTMAXSIGMA))
	 {
		 // These functions do error checks on their values
		 setSigmaAdaptionRate(sigmaSigma);
		 setSigmaRange(minSigma, maxSigma);
		 setSigma(sigma); // Must be set last so an error check for compliance with the boundaries can be made

		 sigma_reset_ = sigma_;
	 }

	 /***************************************************************************/
	 /**
	  * A standard copy constructor. It assumes that the values of the other object are correct
	  * and does no additional error checks.
	  *
	  * @param cp Another GNumGaussAdaptorT object
	  */
	 GNumGaussAdaptorT(const GNumGaussAdaptorT<num_type, fp_type>& cp)
		 : GAdaptorT<num_type> (cp)
			, sigma_(cp.sigma_)
			, sigma_reset_(cp.sigma_reset_)
			, sigmaSigma_(cp.sigmaSigma_)
			, minSigma_(cp.minSigma_)
			, maxSigma_(cp.maxSigma_)
	 { /* nothing */	}

	 /***************************************************************************/
	 /**
	  * The standard destructor. Empty, as we have no local, dynamically
	  * allocated data.
	  */
	 virtual ~GNumGaussAdaptorT()
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

		 // Check that we are dealing with a GNumGaussAdaptorT<num_type, fp_type> reference independent of this object and convert the pointer
		 const GNumGaussAdaptorT<num_type, fp_type> *p_load = Gem::Common::g_convert_and_compare<GObject, GNumGaussAdaptorT<num_type, fp_type>>(cp, this);

		 GToken token("GNumGaussAdaptorT<num_type, fp_type>", e);

		 // Compare our parent data ...
		 Gem::Common::compare_base<GAdaptorT<num_type>>(IDENTITY(*this, *p_load), token);

		 // ... and then the local data
		 compare_t(IDENTITY(sigma_, p_load->sigma_), token);
		 compare_t(IDENTITY(sigma_reset_, p_load->sigma_reset_), token);
		 compare_t(IDENTITY(sigmaSigma_, p_load->sigmaSigma_), token);
		 compare_t(IDENTITY(minSigma_, p_load->minSigma_), token);
		 compare_t(IDENTITY(maxSigma_, p_load->maxSigma_), token);

		 // React on deviations from the expectation
		 token.evaluate();
	 }

	 /***************************************************************************/
	 /**
	  * This function sets the value of the sigma_ parameter. It is recommended
	  * that the value lies in the range [0.:1.]. A value below 0 is not allowed.
	  * Sigma is interpreted as a percentage of the allowed or desired value range
	  * of the target variable. Setting the allowed value range will enforce
	  * a constraint of [0,1], so it is not necessary in this function.
	  *
	  * @param sigma The new value of the sigma_ parameter
	  */
	 void setSigma(const fp_type& sigma)
	 {
		 // Sigma must be in the allowed value range.
		 if(!Gem::Common::checkRangeCompliance<fp_type>(sigma, minSigma_, maxSigma_, "GNumGaussAdaptorT<>::setSigma(" + Gem::Common::to_string(sigma) + ")"))
		 {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG, time_and_place)
					 << "In GNumGaussAdaptorT::setSigma(const fp_type&):" << std::endl
					 << "sigma is not in the allowed range: " << std::endl
					 << minSigma_ << " <= " << sigma << " < " << maxSigma_ << std::endl
					 << "If you want to use these values you need to" << std::endl
					 << "adapt the allowed range first." << std::endl
			 );
		 }

		 sigma_ = sigma;
	 }

	 /***************************************************************************/
	 /**
	  * Retrieves the current value of "reset" sigma_.
	  *
	  * @return The current value of sigma_reset_
	  */
	 fp_type getResetSigma() const  {
		 return sigma_reset_;
	 }

	 /***************************************************************************/
	 /**
	  * This function sets the value of the sigma_reset_ parameter. It is used
	  * to rall back sigma_, if the optimization process has stalled
	  *
	  * @param sigma_reset The new value of the sigma_ parameter
	  */
	 void setResetSigma(const fp_type& sigma_reset)
	 {
		 // Sigma must be in the allowed value range.
		 if(!Gem::Common::checkRangeCompliance<fp_type>(sigma_reset, minSigma_, maxSigma_, "GNumGaussAdaptorT<>::setResetSigma(" + Gem::Common::to_string(sigma_reset) + ")"))
		 {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG, time_and_place)
					 << "In GNumGaussAdaptorT::setResetSigma(const fp_type&):" << std::endl
					 << "sigma_reset is not in the allowed range: " << std::endl
					 << minSigma_ << " <= " << sigma_reset << " < " << maxSigma_ << std::endl
					 << "If you want to use these values you need to" << std::endl
					 << "adapt the allowed range first." << std::endl
			 );
		 }

		 sigma_reset_ = sigma_reset;
	 }

	 /***************************************************************************/
	 /**
	  * Retrieves the current value of sigma_.
	  *
	  * @return The current value of sigma_
	  */
	 fp_type getSigma() const  {
		 return sigma_;
	 }

	 /***************************************************************************/
	 /**
	  * Sets the allowed value range of sigma_. A minimum sigma of 0 will silently be adapted
	  * to a very small value (DEFAULTMINSIGMA), as otherwise adaptions would stop entirely,
	  * which does not make sense.  Using 0. as lower boundary is however allowed for practical
	  * reasons. Note that this function will also adapt sigma itself, if it falls outside of the
	  * allowed range. It is not recommended (but not enforced) to set a maxSigma > 1, as sigma
	  * is interpreted as a percentage of the allowed or desired value range of the target variable.
	  *
	  * @param minSigma The minimum allowed value of sigma_
	  * @param maxSigma The maximum allowed value of sigma_
	  */
	 void setSigmaRange(
		 const fp_type& minSigma
		 , const fp_type& maxSigma
	 ){
		 using namespace Gem::Common;

		 if(minSigma < fp_type(0.) || minSigma > maxSigma || maxSigma > fp_type(1.)) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG, time_and_place)
					 << "In GNumGaussAdaptorT::setSigmaRange(const fp_type&, const fp_type&):" << std::endl
					 << "Invalid values for minSigma and maxSigma given: " << minSigma << " / " << maxSigma << std::endl
					 << "Expected a range [0:1]. Note: Sigma is a percentage of the allowed or" << std::endl
					 << "preferred value range."
			 );
		 }

		 minSigma_ = minSigma; if(minSigma_ < DEFAULTMINSIGMA) minSigma_ = DEFAULTMINSIGMA; // Silently adapt minSigma
		 maxSigma_ = maxSigma;

		 // Rectify sigma_ and reset_sigma_, if necessary
		 Gem::Common::enforceRangeConstraint<fp_type>(
			 sigma_
			 , Gem::Common::gmax(fp_type(minSigma_), fp_type(DEFAULTMINSIGMA))
			 , maxSigma_
			 , "GNumGaussAdaptorT<>::setSigmaRange() / 1"
		 );
		 Gem::Common::enforceRangeConstraint<fp_type>(
			 sigma_reset_
			 , Gem::Common::gmax(fp_type(minSigma_), fp_type(DEFAULTMINSIGMA))
			 , maxSigma_
			 , "GNumGaussAdaptorT<>::setSigmaRange() / 2"
		 );
	 }

	 /* ----------------------------------------------------------------------------------
	  * Setting of valid ranges is tested in GNumGaussAdaptorT<T>::specificTestsNoFailuresExpected_GUnitTests()
	  * Setting of invalid ranges is tested in GNumGaussAdaptorT<T>::specificTestsFailuresExpected_GUnitTests()
	  * ----------------------------------------------------------------------------------
	  */

	 /***************************************************************************/
	 /**
	  * Retrieves the allowed value range for sigma. You can retrieve the values
	  * like this: std::get<0>(getSigmaRange()) , std::get<1>(getSigmaRange()) .
	  *
	  * @return The allowed value range for sigma
	  */
	 std::tuple<fp_type,fp_type> getSigmaRange() const  {
		 return std::make_tuple(minSigma_, maxSigma_);
	 }

	 /* ----------------------------------------------------------------------------------
	  * Tested in GNumGaussAdaptorT<T>::specificTestsNoFailuresExpected_GUnitTests()
	  * ----------------------------------------------------------------------------------
	  */

	 /***************************************************************************/
	 /**
	  * This function sets the values of the sigmaSigma_ parameter. Values <= 0 mean "do not adapt
	  * sigma". If you do want to prevent adaption of sigma, you can also use the
	  * GAdaptorT<T>::setAdaptionThreshold() function. It determines, after how many calls the
	  * internal parameters of the adaption should be adapted. If set to 0, no adaption takes place.
	  *
	  * @param sigmaSigma The new value of the sigmaSigma_ parameter
	  */
	 void setSigmaAdaptionRate(const fp_type& sigmaSigma)
	 {
		 sigmaSigma_ = sigmaSigma;
	 }

	 /* ----------------------------------------------------------------------------------
	  * Setting of valid adaption rates is tested in GNumGaussAdaptorT<T>::specificTestsNoFailuresExpected_GUnitTests()
	  * Setting of invalid adaption rates is tested in GNumGaussAdaptorT<T>::specificTestsFailuresExpected_GUnitTests()
	  * ----------------------------------------------------------------------------------
	  */

	 /***************************************************************************/
	 /**
	  * Retrieves the value of sigmaSigma_ .
	  *
	  * @return The value of the sigmaSigma_ parameter
	  */
	 fp_type getSigmaAdaptionRate() const  {
		 return sigmaSigma_;
	 }

	 /* ----------------------------------------------------------------------------------
	  * Retrieval of adaption rates is tested in GNumGaussAdaptorT<T>::specificTestsNoFailuresExpected_GUnitTests()
	  * ----------------------------------------------------------------------------------
	  */

	 /***************************************************************************/
	 /**
	  * Convenience function that lets users set all relevant parameters of this class
	  * at once.
	  *
	  * @param sigma The initial value for the sigma_ parameter
	  * @param sigmaSigma The initial value for the sigmaSigma_ parameter
	  * @param minSigma The minimal value allowed for sigma_
	  * @param minSigma The maximum value allowed for sigma_
	  */
	 void setAll(
		 const fp_type& sigma
		 , const fp_type& sigmaSigma
		 , const fp_type& minSigma
		 , const fp_type& maxSigma
	 )	{
		 setSigmaAdaptionRate(sigmaSigma);
		 setSigmaRange(minSigma, maxSigma);
		 setSigma(sigma);
	 }

	 /* ----------------------------------------------------------------------------------
	  * Tested in GNumGaussAdaptorT<num_type, fp_type>::specificTestsNoFailuresExpected_GUnitTests()
	  * ----------------------------------------------------------------------------------
	  */

	 /***************************************************************************/
	 /**
	  * Prints diagnostic messages
	  *
	  * @return The diagnostic message
	  */
	 std::string printDiagnostics() const override {
		 std::ostringstream diag;
		 std::tuple<fp_type,fp_type> sigmaRange = getSigmaRange();

		 diag
			 << "Diagnostic message by GNumAdaptorT<num_type,fp_type>" << std::endl
			 << "with typeid(num_type).name() = " << typeid(num_type).name() << std::endl
			 << "and typeid(fp_type).name() = " << typeid(fp_type).name() << " :" << std::endl
			 << "getSigma() = " << getSigma() << std::endl
			 << "getResetSigma() = " << getResetSigma() << std::endl
			 << "getSigmaRange() = " << std::get<0>(sigmaRange) << " --> " << std::get<1>(sigmaRange) << std::endl
			 << "getSigmaAdaptionRate() = " << getSigmaAdaptionRate() << std::endl;

		 return diag.str();
	 }

	 /***************************************************************************/
	 /**
	  * @brief Retrieves the id of the adaptor. */
	 Gem::Geneva::adaptorId getAdaptorId() const override = 0;

	 /***************************************************************************/
	 /**
	  * Emits a name for this class / object
	  */
	 std::string name() const  override {
		 return std::string("GNumGaussAdaptorT");
	 }

	 /***************************************************************************/
	 /**
	  * Triggers updates when the optimization process has stalled. This function
	  * resets the sigma value to its original value and calls the parent class'es function
	  *
	  * @param nStalls The number of consecutive stalls up to this point
	  * @param range A typical value range for type T
	  * @return A boolean indicating whether updates were performed
	  */
	 virtual bool updateOnStall(
		 const std::size_t& nStalls
		 , const num_type& range
	 ) override {
		 // Call our parent class'es function
		 GAdaptorT<num_type>::updateOnStall(nStalls, range);

		 // Reset the adaption probability
		 if(sigma_ == sigma_reset_) {
			 return false;
		 } else {
			 sigma_ = sigma_reset_;
			 return true;
		 }
	 }

protected:
	 /***************************************************************************/
	 /**
	  * This function loads the data of another GNumGaussAdaptorT<num_type, fp_type>, camouflaged as a GObject.
	  * We assume that the values given to us by the other object are correct and do no error checks.
	  *
	  * @param A copy of another GNumGaussAdaptorT<num_type, fp_type>, camouflaged as a GObject
	  */
	 void load_(const GObject *cp) override {
		 // Check that we are dealing with a GNumGaussAdaptorT<num_type, fp_type> reference independent of this object and convert the pointer
		 const GNumGaussAdaptorT<num_type, fp_type> *p_load = Gem::Common::g_convert_and_compare<GObject, GNumGaussAdaptorT<num_type, fp_type>>(cp, this);

		 // Load the data of our parent class ...
		 GAdaptorT<num_type>::load_(cp);

		 // ... and then our own data
		 sigma_       = p_load->sigma_;
		 sigma_reset_ = p_load->sigma_reset_;
		 sigmaSigma_  = p_load->sigmaSigma_;
		 minSigma_    = p_load->minSigma_;
		 maxSigma_    = p_load->maxSigma_;
	 }

	 /***************************************************************************/
	 /**
	  * This adaptor allows the evolutionary adaption of sigma_. This allows the
	  * algorithm to adapt to changing geometries of the quality surface.
	  *
	  * @param range A typical range for the parameter with type num_type (unused here)
	  */
	 virtual void customAdaptAdaption(
		 const num_type& val
		 , Gem::Hap::GRandomBase& gr
	 ) override {
		 using namespace Gem::Common;
		 using namespace Gem::Hap;

		 // The following random distribution slightly favours values < 1. Selection pressure
		 // will keep the values higher if needed
		 sigma_ *= gexp(GAdaptorT<num_type, fp_type>::m_normal_distribution(gr, typename std::normal_distribution<fp_type>::param_type(0., gfabs(sigmaSigma_))));

		 // make sure sigma_ doesn't get out of range
		 Gem::Common::enforceRangeConstraint<fp_type>(sigma_, minSigma_, maxSigma_, "GNumGaussAdaptorT<>::customAdaptAdaption()", false /* silent */);
	 }

	 /***************************************************************************/
	 /**
	  * The actual adaption of the supplied value takes place here. Purely virtual, as the actual
	  * adaptions are defined in the derived classes.
	  *
	  * @param value The value that is going to be adapted in situ
	  */
	 void customAdaptions(num_type&, const num_type&, Gem::Hap::GRandomBase&) override = 0;

	 /***************************************************************************/
	 /**
	  * Allows to randomly initialize parameter members
	  */
	 virtual bool randomInit(
		 Gem::Hap::GRandomBase& gr
	 ) override {
		 using namespace Gem::Common;
		 using namespace Gem::Hap;

		 sigma_ = GAdaptorT<num_type, fp_type>::m_uniform_real_distribution(gr, typename std::uniform_real_distribution<fp_type>::param_type(minSigma_, maxSigma_));

		 return true;
	 }

	 /***************************************************************************/
	 /**
	  * Adds a given property value to the vector or returns false, if the property
	  * was not found.
	  */
	 virtual bool customQueryProperty (
		 const std::string& property
		 , std::vector<boost::any>& data
	 ) const override {
		 if(property == "sigma") {
			 data.push_back(boost::any(sigma_));
		 } else {
			 return false;
		 }

		 return true;
	 }

	 // "protected" for performance reasons, so we do not have to go through access functions
	 /***************************************************************************/
	 fp_type sigma_; ///< The width of the gaussian used to adapt values
	 fp_type sigma_reset_; ///< The value to which sigma_ will be reset if "updateOnStall()" is called
	 fp_type sigmaSigma_; ///< affects sigma_ adaption
	 fp_type minSigma_; ///< minimum allowed value for sigma_
	 fp_type maxSigma_; ///< maximum allowed value for sigma_

private:
	 /***************************************************************************/
	 /**
	  * This function creates a deep copy of this object. Purely virtual so this class cannot
	  * be instantiated directly.
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

		 // A relatively harmless change
		 sigmaSigma_ *= fp_type(1.1);
		 result = true;

		 return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		 Gem::Common::condnotset("GNumGaussAdaptorT<>::modify_GUnitTests", "GEM_TESTING");
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

		 // Get a random number generator
		 Gem::Hap::GRandomT<Gem::Hap::RANDFLAVOURS::RANDOMPROXY> gr;

		 //------------------------------------------------------------------------------

		 { // Test setting and retrieval of the sigma range
			 std::shared_ptr<GNumGaussAdaptorT<num_type, fp_type>> p_test = this->template clone<GNumGaussAdaptorT<num_type, fp_type>>();

			 for(fp_type dlower=fp_type(0.); dlower<fp_type(0.8); dlower+=fp_type(0.1)) {
				 fp_type dupper = Gem::Common::gmin(fp_type(2.)*dlower, fp_type(1.));
				 if(0==dupper) {
					 dupper = 1.;
				 }

				 BOOST_CHECK_NO_THROW(p_test->setSigmaRange(dlower, dupper));
				 typename std::tuple<fp_type, fp_type> range;
				 BOOST_CHECK_NO_THROW(range = p_test->getSigmaRange());

				 using namespace boost;

				 if(dlower == 0.) { // Account for the fact that a lower boundary of 0. will be silently changed
					 BOOST_CHECK_MESSAGE(
						 std::get<0>(range) == boost::numeric_cast<fp_type>(DEFAULTMINSIGMA)
						 , std::get<0>(range) << " / " << boost::numeric_cast<fp_type>(DEFAULTMINSIGMA)
					 );
					 BOOST_CHECK_MESSAGE(
						 std::get<1>(range) == boost::numeric_cast<fp_type>(1.)
						 , std::get<1>(range) << " / " << boost::numeric_cast<fp_type>(1.)
					 );
				 }
				 else {
					 BOOST_CHECK(std::get<0>(range) == dlower);
				 }
			 }

		 }

		 //------------------------------------------------------------------------------

		 { // Test that setting a sigma of 0. will result in a sigma with value DEFAULTMINSIGMA
			 std::shared_ptr<GNumGaussAdaptorT<num_type, fp_type>> p_test = this->template clone<GNumGaussAdaptorT<num_type, fp_type>>();

			 BOOST_CHECK_NO_THROW(p_test->setSigmaRange(fp_type(0.), fp_type(1.)));
			 BOOST_CHECK_NO_THROW(p_test->setSigma(fp_type(DEFAULTMINSIGMA)));
			 BOOST_CHECK(p_test->getSigma() == fp_type(DEFAULTMINSIGMA));
		 }

		 //------------------------------------------------------------------------------

		 { // Tests setting and retrieval of the sigma parameter
			 std::shared_ptr<GNumGaussAdaptorT<num_type, fp_type>> p_test = this->template clone<GNumGaussAdaptorT<num_type, fp_type>>();

			 BOOST_CHECK_NO_THROW(p_test->setSigmaRange(fp_type(0.), fp_type(1.)));

			 for(fp_type d=fp_type(0.1); d<fp_type(0.9); d+=fp_type(0.1)) {
				 BOOST_CHECK_NO_THROW(p_test->setSigma(d));
				 BOOST_CHECK(p_test->getSigma() == d);
			 }
		 }

		 //------------------------------------------------------------------------------

		 { // Test setting and retrieval of the sigma adaption rate
			 std::shared_ptr<GNumGaussAdaptorT<num_type, fp_type>> p_test = this->template clone<GNumGaussAdaptorT<num_type, fp_type>>();

			 for(fp_type d=fp_type(0.1); d<fp_type(0.9); d+=fp_type(0.1)) {
				 BOOST_CHECK_NO_THROW(p_test->setSigmaAdaptionRate(d));
				 BOOST_CHECK(p_test->getSigmaAdaptionRate() == d);
			 }
		 }

		 //------------------------------------------------------------------------------

		 { // Check that simultaneous setting of all "sigma-values" has an effect
			 using namespace boost;

			 std::shared_ptr<GNumGaussAdaptorT<num_type, fp_type>> p_test = this->template clone<GNumGaussAdaptorT<num_type, fp_type>>();

			 BOOST_CHECK_NO_THROW(p_test->setAll(fp_type(0.5), fp_type(0.8), fp_type(0.), fp_type(1.)));
			 BOOST_CHECK(p_test->getSigma() == fp_type(0.5));
			 BOOST_CHECK(p_test->getSigmaAdaptionRate() == fp_type(0.8));
			 std::tuple<fp_type, fp_type> range;
			 BOOST_CHECK_NO_THROW(range = p_test->getSigmaRange());
			 BOOST_CHECK(std::get<0>(range) == fp_type(DEFAULTMINSIGMA));
			 BOOST_CHECK(std::get<1>(range) == fp_type(1.));
		 }

		 //------------------------------------------------------------------------------

		 { // Test sigma adaption
			 std::shared_ptr<GNumGaussAdaptorT<num_type, fp_type>> p_test = this->template clone<GNumGaussAdaptorT<num_type, fp_type>>();

			 // true: Adaptions should happen always, independent of the adaption probability
			 BOOST_CHECK_NO_THROW (p_test->setAdaptionMode(true));

			 const fp_type minSigma = fp_type(0.0001);
			 const fp_type maxSigma = fp_type(1.);
			 const fp_type sigmaStart = fp_type(1.);
			 const fp_type sigmaSigma = fp_type(0.001);

			 BOOST_CHECK_NO_THROW(p_test->setSigmaRange(minSigma, maxSigma));
			 BOOST_CHECK_NO_THROW(p_test->setSigma(sigmaStart));
			 BOOST_CHECK_NO_THROW(p_test->setSigmaAdaptionRate(sigmaSigma));

			 fp_type oldSigma = p_test->getSigma();
			 fp_type newSigma = 0.;
			 BOOST_CHECK(oldSigma == sigmaStart);

			 std::size_t nTests = 10000;
			 std::size_t maxCounter = 0;
			 std::size_t maxMaxCounter = 500;
			 for(std::size_t i=0; i<nTests; i++) {
				 BOOST_CHECK_NO_THROW(p_test->adaptAdaption(num_type(1), gr));
				 BOOST_CHECK(newSigma = p_test->getSigma());
				 BOOST_CHECK(newSigma >= minSigma && newSigma <= maxSigma);

				 if(newSigma != minSigma && newSigma != maxSigma) {
					 BOOST_CHECK_MESSAGE (
						 newSigma != oldSigma
						 ,  "\n"
						 << "oldSigma = " << oldSigma << "\n"
						 << "newSigma = " << newSigma << "\n"
						 << "iteration = " << i << "\n"
					 );
					 oldSigma = newSigma;
				 }
				 else {
					 // We want to know how often we have exceeded the boundaries
					 maxCounter++;
				 }
			 }

			 BOOST_CHECK_MESSAGE (
				 maxCounter < maxMaxCounter
				 ,  "\n"
				 << "maxCounter = " << maxCounter << "\n"
				 << "maxMaxCounter = " << maxMaxCounter << "\n"
			 );
		 }

		 //------------------------------------------------------------------------------

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		 Gem::Common::condnotset("GNumGaussAdaptorT<>::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
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

		 //------------------------------------------------------------------------------

		 { // Test that setting a minimal sigma < 0. throws
			 std::shared_ptr<GNumGaussAdaptorT<num_type, fp_type>> p_test = this->template clone<GNumGaussAdaptorT<num_type, fp_type>>();

			 BOOST_CHECK_THROW(p_test->setSigmaRange(fp_type(-1.), fp_type(2.)), gemfony_exception);
		 }

		 //------------------------------------------------------------------------------

		 { // Test that setting a minimal sigma > the maximum sigma throws
			 std::shared_ptr<GNumGaussAdaptorT<num_type, fp_type>> p_test = this->template clone<GNumGaussAdaptorT<num_type, fp_type>>();

			 BOOST_CHECK_THROW(p_test->setSigmaRange(fp_type(2.), fp_type(1.)), gemfony_exception);
		 }

		 //------------------------------------------------------------------------------

		 { // Test that setting a negative sigma throws
			 std::shared_ptr<GNumGaussAdaptorT<num_type, fp_type>> p_test = this->template clone<GNumGaussAdaptorT<num_type, fp_type>>();

			 BOOST_CHECK_THROW(p_test->setSigma(fp_type(-1.)), gemfony_exception);
		 }

		 //------------------------------------------------------------------------------

		 { // Test that setting a sigma below the allowed range throws
			 std::shared_ptr<GNumGaussAdaptorT<num_type, fp_type>> p_test = this->template clone<GNumGaussAdaptorT<num_type, fp_type>>();

			 BOOST_CHECK_NO_THROW(p_test->setSigmaRange(fp_type(0.5), fp_type(1.)));
			 BOOST_CHECK_THROW(p_test->setSigma(fp_type(0.1)), gemfony_exception);
		 }

		 //------------------------------------------------------------------------------

		 { // Test that setting a sigma above the allowed range throws
			 std::shared_ptr<GNumGaussAdaptorT<num_type, fp_type>> p_test = this->template clone<GNumGaussAdaptorT<num_type, fp_type>>();

			 BOOST_CHECK_NO_THROW(p_test->setSigmaRange(fp_type(0.5), fp_type(1.)));
			 BOOST_CHECK_THROW(p_test->setSigma(fp_type(3.)), gemfony_exception);
		 }

		 //------------------------------------------------------------------------------

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		 Gem::Common::condnotset("GNumGaussAdaptorT<>::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
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
template<typename num_type, typename fp_type>
struct is_abstract< Gem::Geneva::GNumGaussAdaptorT<num_type, fp_type>> : public boost::true_type {};
template<typename num_type, typename fp_type>
struct is_abstract< const Gem::Geneva::GNumGaussAdaptorT<num_type, fp_type>> : public boost::true_type {};
}
}

#endif /* GNUMGAUSSADAPTORT_HPP_ */
