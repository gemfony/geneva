/**
 * @file GNumBiGaussAdaptorT.hpp
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt) and Karlsruhe
 * Institute of Technology (University of the State of Baden-Wuerttemberg
 * and National Laboratory of the Helmholtz Association).
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) com
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



// Standard headers go here

// Boost headers go here

#ifndef GNUMBIGAUSSADAPTORT_HPP_
#define GNUMBIGAUSSADAPTORT_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif


// Geneva headers go here
#include "geneva/GAdaptorT.hpp"

namespace Gem {
namespace Geneva {

/************************************************************************************************/
/**
 * GNumBiGaussAdaptorT is used for the adaption of numeric types, by the addition of random numbers
 * distributed as two adjacent gaussians. Different numeric types may be used, including Boost's
 * integer representations. The type used needs to be specified as a template parameter. In comparison
 * to GNumGaussAdaptorT, an additional parameter "delta" is added, which represents the distance between
 * both gaussians. Just like sigma, delta can be subject to mutations. It is also possible to use
 * two different sigma/sigmaSigma values and adaption rates for both gaussians. Note that this adaptor
 * is experimental. Your mileage may vary.
 *
 * NOTE: THIS CLASS ISN'T FINISHED YET. DON'T USE.
 */
template<typename T>
class GNumBiGaussAdaptorT :public GAdaptorT<T>
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int) {
		using boost::serialization::make_nvp;

		// Save all necessary data
		ar & make_nvp("GAdaptorT_num", boost::serialization::base_object<GAdaptorT<T> >(*this))
		   & BOOST_SERIALIZATION_NVP(sigma1_)
		   & BOOST_SERIALIZATION_NVP(sigmaSigma1_)
		   & BOOST_SERIALIZATION_NVP(minSigma1_)
		   & BOOST_SERIALIZATION_NVP(maxSigma1_)
		   & BOOST_SERIALIZATION_NVP(sigma2_)
		   & BOOST_SERIALIZATION_NVP(sigmaSigma2_)
		   & BOOST_SERIALIZATION_NVP(minSigma2_)
		   & BOOST_SERIALIZATION_NVP(maxSigma2_)
		   & BOOST_SERIALIZATION_NVP(delta_)
		   & BOOST_SERIALIZATION_NVP(sigmaDelta_)
		   & BOOST_SERIALIZATION_NVP(minDelta_)
		   & BOOST_SERIALIZATION_NVP(maxDelta_);
	}
	///////////////////////////////////////////////////////////////////////

public:
	/********************************************************************************************/
	/**
	 * The standard constructor.
	 */
	GNumBiGaussAdaptorT()
		: GAdaptorT<T> ()
		, sigma_(DEFAULTSIGMA)
		, sigmaSigma_(DEFAULTSIGMASIGMA)
		, minSigma_(DEFAULTMINSIGMA)
		, maxSigma_(DEFAULTMAXSIGMA)
	{ /* nothing */ }

	/********************************************************************************************/
	/**
	 * Initialization of the parent class'es adaption probability.
	 *
	 * @param probability The likelihood for a adaption actually taking place
	 */
	GNumBiGaussAdaptorT(const double& probability)
		: GAdaptorT<T> (probability)
		, sigma_(DEFAULTSIGMA)
		, sigmaSigma_(DEFAULTSIGMASIGMA)
		, minSigma_(DEFAULTMINSIGMA)
		, maxSigma_(DEFAULTMAXSIGMA)
	{ /* nothing */ }

	/********************************************************************************************/
	/**
	 * This constructor lets a user set all sigma parameters in one go.
	 *
	 * @param sigma The initial value for the sigma_ parameter
	 * @param sigmaSigma The initial value for the sigmaSigma_ parameter
	 * @param minSigma The minimal value allowed for sigma_
	 * @param maxSigma The maximal value allowed for sigma_
	 */
	GNumBiGaussAdaptorT(const double& sigma, const double& sigmaSigma,
				const double& minSigma, const double& maxSigma)
		: GAdaptorT<T> ()
		, sigma_(DEFAULTSIGMA)
		, sigmaSigma_(DEFAULTSIGMASIGMA)
		, minSigma_(DEFAULTMINSIGMA)
		, maxSigma_(DEFAULTMAXSIGMA)
	{
		// These functions do error checks on their values
		setSigmaAdaptionRate(sigmaSigma);
		setSigmaRange(minSigma, maxSigma);
		setSigma(sigma);
	}

	/********************************************************************************************/
	/**
	 * This constructor lets a user set all parameters in one go.
	 *
	 * @param sigma The initial value for the sigma_ parameter
	 * @param sigmaSigma The initial value for the sigmaSigma_ parameter
	 * @param minSigma The minimal value allowed for sigma_
	 * @param maxSigma The maximal value allowed for sigma_
	 * @param probability The likelihood for a adaption actually taking place
	 */
	GNumBiGaussAdaptorT(const double& sigma, const double& sigmaSigma,
				const double& minSigma, const double& maxSigma,
				const double& probability)
		: GAdaptorT<T> (probability)
		, sigma_(DEFAULTSIGMA)
		, sigmaSigma_(DEFAULTSIGMASIGMA)
		, minSigma_(DEFAULTMINSIGMA)
		, maxSigma_(DEFAULTMAXSIGMA)
	{
		// These functions do error checks on their values
		setSigmaAdaptionRate(sigmaSigma);
		setSigmaRange(minSigma, maxSigma);
		setSigma(sigma);
	}

	/********************************************************************************************/
	/**
	 * A standard copy constructor. It assumes that the values of the other object are correct
	 * and does no additional error checks.
	 *
	 * @param cp Another GNumBiGaussAdaptorT object
	 */
	GNumBiGaussAdaptorT(const GNumBiGaussAdaptorT<T>& cp)
		: GAdaptorT<T> (cp)
		, sigma_(cp.sigma_)
		, sigmaSigma_(cp.sigmaSigma_)
		, minSigma_(cp.minSigma_)
		, maxSigma_(cp.maxSigma_)
	{ /* nothing */	}

	/********************************************************************************************/
	/**
	 * The standard destructor. Empty, as we have no local, dynamically
	 * allocated data.
	 */
	~GNumBiGaussAdaptorT()
	{ /* nothing */ }

	/********************************************************************************************/
	/**
	 * A standard assignment operator for GNumBiGaussAdaptorT objects,
	 *
	 * @param cp A copy of another GNumBiGaussAdaptorT object
	 */
	const GNumBiGaussAdaptorT<T>& operator=(const GNumBiGaussAdaptorT<T>& cp)
	{
		GNumBiGaussAdaptorT<T>::load_(&cp);
		return *this;
	}

	/********************************************************************************************/
	/**
	 * Checks for equality with another GNumBiGaussAdaptorT<T> object
	 *
	 * @param  cp A constant reference to another GNumBiGaussAdaptorT<T> object
	 * @return A boolean indicating whether both objects are equal
	 */
	bool operator==(const GNumBiGaussAdaptorT<T>& cp) const {
		using namespace Gem::Common;
		// Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
		return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GNumBiGaussAdaptorT<T>::operator==","cp", CE_SILENT);
	}

	/********************************************************************************************/
	/**
	 * Checks for inequality with another GNumBiGaussAdaptorT<T> object
	 *
	 * @param  cp A constant reference to another GNumBiGaussAdaptorT<T> object
	 * @return A boolean indicating whether both objects are inequal
	 */
	bool operator!=(const GNumBiGaussAdaptorT<T>& cp) const {
		using namespace Gem::Common;
		// Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
		return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GNumBiGaussAdaptorT<T>::operator!=","cp", CE_SILENT);
	}

	/********************************************************************************************/
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
		const GNumBiGaussAdaptorT<T>  *p_load = GObject::conversion_cast<GNumBiGaussAdaptorT<T> >(&cp);

		// Will hold possible deviations from the expectation, including explanations
	    std::vector<boost::optional<std::string> > deviations;

		// Check our parent class'es data ...
		deviations.push_back(GAdaptorT<T>::checkRelationshipWith(cp, e, limit, "GNumBiGaussAdaptorT<T>", y_name, withMessages));

		// ... and then our local data
		deviations.push_back(checkExpectation(withMessages, "GNumBiGaussAdaptorT<T>", sigma_, p_load->sigma_, "sigma_", "p_load->sigma_", e , limit));
		deviations.push_back(checkExpectation(withMessages, "GNumBiGaussAdaptorT<T>", sigmaSigma_, p_load->sigmaSigma_, "sigmaSigma_", "p_load->sigmaSigma_", e , limit));
		deviations.push_back(checkExpectation(withMessages, "GNumBiGaussAdaptorT<T>", minSigma_, p_load->minSigma_, "minSigma_", "p_load->minSigma_", e , limit));
		deviations.push_back(checkExpectation(withMessages, "GNumBiGaussAdaptorT<T>", maxSigma_, p_load->maxSigma_, "maxSigma_", "p_load->maxSigma_", e , limit));

		return evaluateDiscrepancies("GNumBiGaussAdaptorT<T>", caller, deviations, e);
	}

	/********************************************************************************************/
	/**
	 * This function sets the value of the sigma_ parameter. Note that this function
	 * will silently set a 0 sigma to a very small value.
	 *
	 * @param sigma The new value of the sigma_ parameter
	 */
	void setSigma(const double& sigma)
	{
		if(sigma < 0.) {
			raiseException(
					"In GNumBiGaussAdaptorT::setSigma(const double&):" << std::endl
					<< "sigma is negative: " << sigma
			);
		}

		double tmpSigma;
		if(sigma == 0.) tmpSigma = DEFAULTMINSIGMA;
		else tmpSigma = sigma;

		// Sigma must be in the allowed value range
		if(tmpSigma < minSigma_ || tmpSigma > maxSigma_)
		{
			raiseException(
					"In GNumBiGaussAdaptorT::setSigma(const double&):" << std::endl
					<< "sigma is not in the allowed range: " << std::endl
					<< tmpSigma << " " << minSigma_ << " " << maxSigma_ << std::endl
					<< "If you want to use these values you need to" << std::endl
					<< "adapt the allowed range first."
			);
		}

		sigma_ = tmpSigma;
	}

	/********************************************************************************************/
	/**
	 * Retrieves the current value of sigma_.
	 *
	 * @return The current value of sigma_
	 */
	double getSigma() const  {
		return sigma_;
	}

	/********************************************************************************************/
	/**
	 * Sets the allowed value range of sigma_. A minimum sigma of 0 will silently be adapted
	 * to a very small value (DEFAULTMINSIGMA), as otherwise adaptions would stop entirely,
	 * which does not make sense.  Using 0. as lower boundary is however allowed for practical
	 * reasons. Note that this function will also adapt sigma itself, if it falls outside of the allowed
	 * range.
	 *
	 * @param minSigma The minimum allowed value of sigma_
	 * @param minSigma The maximum allowed value of sigma_
	 */
	void setSigmaRange(const double& minSigma, const double& maxSigma){
		double tmpMinSigma = minSigma;
		// Silently adapt minSigma, if necessary. A value of 0. does not make sense.
		if(minSigma == 0.) tmpMinSigma = DEFAULTMINSIGMA;

		// Do some error checks
		if(tmpMinSigma<=0. || minSigma >= maxSigma){ // maxSigma will automatically be > 0. now
			raiseException(
					"In GNumBiGaussAdaptorT::setSigmaRange(const double&, const double&):" << std::endl
					<< "Invalid values for minSigma and maxSigma given:" << tmpMinSigma << " " << maxSigma
			);
		}

		minSigma_ = tmpMinSigma;
		maxSigma_ = maxSigma;

		// Adapt sigma, if necessary
		if(sigma_ < minSigma_) sigma_ = minSigma_;
		else if(sigma_>maxSigma_) sigma_ = maxSigma_;
	}

	/* ----------------------------------------------------------------------------------
	 * Setting of valid ranges is tested in GNumBiGaussAdaptorT<T>::specificTestsNoFailuresExpected_GUnitTests()
	 * Setting of invalid ranges is tested in GNumBiGaussAdaptorT<T>::specificTestsFailuresExpected_GUnitTests()
	 * ----------------------------------------------------------------------------------
	 */

	/********************************************************************************************/
	/**
	 * Retrieves the allowed value range for sigma. You can retrieve the values
	 * like this: getSigmaRange().first , getSigmaRange().second .
	 *
	 * @return The allowed value range for sigma
	 */
	std::pair<double,double> getSigmaRange() const  {
		return std::make_pair(minSigma_, maxSigma_);
	}

	/* ----------------------------------------------------------------------------------
	 * Tested in GNumBiGaussAdaptorT<T>::specificTestsNoFailuresExpected_GUnitTests()
	 * ----------------------------------------------------------------------------------
	 */

	/********************************************************************************************/
	/**
	 * This function sets the values of the sigmaSigma_ parameter and the
	 * minimal value allowed for sigma_. 0 is not allowed. If you do want to
	 * prevent adaption of sigma, you can use the GAdaptorT<T>::setAdaptionThreshold()
	 * function. It determines, after how many adaptions the internal parameters
	 * of the adaption should be adapted. If set to 0, no adaption takes place.
	 *
	 * @param sigmaSigma The new value of the sigmaSigma_ parameter
	 */
	void setSigmaAdaptionRate(const double& sigmaSigma)
	{
		// A value of sigmaSigma <= 0. is not useful.
		if(sigmaSigma <= 0.)
		{
			raiseException(
					"In GNumBiGaussAdaptorT::setSigmaSigma(double, double):" << std::endl
					<< "Bad value for sigmaSigma given: " << sigmaSigma
			);
		}

		sigmaSigma_ = sigmaSigma;
	}

	/* ----------------------------------------------------------------------------------
	 * Setting of valid adaption rates is tested in GNumBiGaussAdaptorT<T>::specificTestsNoFailuresExpected_GUnitTests()
	 * Setting of invalid adaption rates is tested in GNumBiGaussAdaptorT<T>::specificTestsFailuresExpected_GUnitTests()
	 * ----------------------------------------------------------------------------------
	 */

	/********************************************************************************************/
	/**
	 * Retrieves the value of sigmaSigma_ .
	 *
	 * @return The value of the sigmaSigma_ parameter
	 */
	double getSigmaAdaptionRate() const  {
		return sigmaSigma_;
	}

	/* ----------------------------------------------------------------------------------
	 * Retrieval of adaption rates is tested in GNumBiGaussAdaptorT<T>::specificTestsNoFailuresExpected_GUnitTests()
	 * ----------------------------------------------------------------------------------
	 */

	/********************************************************************************************/
	/**
	 * Convenience function that lets users set all relevant parameters of this class
	 * at once.
	 *
	 * @param sigma The initial value for the sigma_ parameter
	 * @param sigmaSigma The initial value for the sigmaSigma_ parameter
	 * @param minSigma The minimal value allowed for sigma_
	 * @param minSigma The maximum value allowed for sigma_
	 */
	void setAll(const double& sigma, const double& sigmaSigma, const double& minSigma, const double& maxSigma)
	{
		setSigmaAdaptionRate(sigmaSigma);
		setSigmaRange(minSigma, maxSigma);
		setSigma(sigma);
	}

	/* ----------------------------------------------------------------------------------
	 * Tested in GNumBiGaussAdaptorT<T>::specificTestsNoFailuresExpected_GUnitTests()
	 * ----------------------------------------------------------------------------------
	 */

	/***********************************************************************************/
	/**
	 * Retrieves the id of the adaptor. Specializations of this function
	 * exist.
	 *
	 * @return The id of the adaptor
	 */
	virtual Gem::Geneva::adaptorId getAdaptorId() const {
		raiseException(
				"In Gem::Geneva::adaptorId GNumBiGaussAdaptorT::getAdaptorId():" << std::endl
				<< "Function used with a type it was not designed for"
		);
	}

protected:
	/********************************************************************************************/
	/**
	 * This function loads the data of another GNumBiGaussAdaptorT, camouflaged as a GObject.
	 * We assume that the values given to us by the other object are correct and do no error checks.
	 *
	 * @param A copy of another GNumBiGaussAdaptorT, camouflaged as a GObject
	 */
	void load_(const GObject *cp)
	{
		// Convert GObject pointer to local format
		const GNumBiGaussAdaptorT<T> *p_load = GObject::conversion_cast<GNumBiGaussAdaptorT<T> >(cp);

		// Load the data of our parent class ...
		GAdaptorT<T>::load_(cp);

		// ... and then our own data
		sigma_ = p_load->sigma_;
		sigmaSigma_ = p_load->sigmaSigma_;
		minSigma_ = p_load->minSigma_;
		maxSigma_ = p_load->maxSigma_;
	}

	/********************************************************************************************/
	/**
	 * This function creates a deep copy of this object. Purely virtual so this class cannot
	 * be instantiated directly.
	 *
	 * @return A deep copy of this object
	 */
	virtual GObject *clone_() const = 0;

	/********************************************************************************************/
	/**
	 * This adaptor allows the evolutionary adaption of sigma_. This allows the
	 * algorithm to adapt to changing geometries of the quality surface.
	 */
	virtual void adaptAdaption()
	{
		// We do not want to favor the decrease or increase of sigma, hence we choose
		// randomly whether to multiply or divide. TODO: cross-check.
		sigma_ *= exp(GAdaptorT<T>::gr->normal_distribution(sigmaSigma_)*(GAdaptorT<T>::gr->uniform_bool()?1:-1));

		// make sure sigma_ doesn't get out of range
		if(sigma_ < minSigma_) sigma_ = minSigma_;
		else if(sigma_ > maxSigma_) sigma_ = maxSigma_;

		// Make sure that the appropriate actions are performed by the parent class
		GAdaptorT<T>::adaptAdaption();
	}

	/********************************************************************************************/
	/**
	 * The actual adaption of the supplied value takes place here. Purely virtual, as the actual
	 * adaptions are defined in the derived classes.
	 *
	 * @param value The value that is going to be adapted in situ
	 */
	virtual void customAdaptions(T&) = 0;

protected: // For performance reasons, so we do not have to go through access functions
	/********************************************************************************************/
	double sigma1_; ///< The width of the first gaussian used to adapt values
	double sigmaSigma1_; ///< affects sigma1_ adaption
	double minSigma1_; ///< minimum allowed value for sigma1_
	double maxSigma1_; ///< maximum allowed value for sigma1_
	double sigma2_; ///< The width of the second gaussian used to adapt values
	double sigmaSigma2_; ///< affects sigma2_ adaption
	double minSigma2_; ///< minimum allowed value for sigma2_
	double maxSigma2_; ///< maximum allowed value for sigma2_
	double delta_; ///< The distance between both gaussians
	double sigmaDelta_; ///< affects the adaption of delta_
	double minDelta_; ///< minimum allowed value for delta_
	double maxDelta_; ///< maximum allowed value for delta_

#ifdef GENEVATESTING
public:
	/***********************************************************************************/
	/**
	 * Applies modifications to this object. This is needed for testing purposes
	 *
	 * @return A boolean which indicates whether modifications were made
	 */
	virtual bool modify_GUnitTests() {
		using boost::unit_test_framework::test_suite;
		using boost::unit_test_framework::test_case;

		bool result = false;

		// Call the parent classes' functions
		if(GAdaptorT<T>::modify_GUnitTests()) result = true;

		// A relatively harmless change
		sigmaSigma1_ *= 1.1;
		result = true;

		return result;
	}

	/***********************************************************************************/
	/**
	 * Performs self tests that are expected to succeed. This is needed for testing purposes
	 */
	virtual void specificTestsNoFailureExpected_GUnitTests() {
		using boost::unit_test_framework::test_suite;
		using boost::unit_test_framework::test_case;

		// Call the parent classes' functions
		GAdaptorT<T>::specificTestsNoFailureExpected_GUnitTests();
	}

	/***********************************************************************************/
	/**
	 * Performs self tests that are expected to fail. This is needed for testing purposes
	 */
	virtual void specificTestsFailuresExpected_GUnitTests() {
		using boost::unit_test_framework::test_suite;
		using boost::unit_test_framework::test_case;

		// Call the parent classes' functions
		GAdaptorT<T>::specificTestsFailuresExpected_GUnitTests();d
	}

#endif /* GENEVATESTING */
};

/************************************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

/************************************************************************************************/
// The content of BOOST_SERIALIZATION_ASSUME_ABSTRACT(T)
namespace boost {
	namespace serialization {
		template<typename T>
		struct is_abstract< Gem::Geneva::GNumBiGaussAdaptorT<T> > : public boost::true_type {};
		template<typename T>
		struct is_abstract< const Gem::Geneva::GNumBiGaussAdaptorT<T> > : public boost::true_type {};
	}
}

#endif /* GNUMBIGAUSSADAPTORT_HPP_ */
