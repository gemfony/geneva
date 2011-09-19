/**
 * @file GNumBiGaussAdaptorT.hpp
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
 */
template<typename num_type, typename fp_type>
class GNumBiGaussAdaptorT :public GAdaptorT<num_type>
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int) {
		using boost::serialization::make_nvp;

		// Save all necessary data
		ar & make_nvp("GAdaptorT_num", boost::serialization::base_object<GAdaptorT<num_type> >(*this))
		   & BOOST_SERIALIZATION_NVP(useSymmetricSigmas_)
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
		: GAdaptorT<num_type> ()
		, useSymmetricSigmas_(true)
		, sigma1_(DEFAULTSIGMA)
		, sigmaSigma1_(DEFAULTSIGMASIGMA)
		, minSigma1_(DEFAULTMINSIGMA)
		, maxSigma1_(DEFAULTMAXSIGMA)
		, sigma2_(DEFAULTSIGMA)
		, sigmaSigma2_(DEFAULTSIGMASIGMA)
		, minSigma2_(DEFAULTMINSIGMA)
		, maxSigma2_(DEFAULTMAXSIGMA)
		, delta_(DEFAULTDELTA)
		, sigmaDelta_(DEFAULTSIGMADELTA)
		, minDelta_(DEFAULTMINDELTA)
		, maxDelta_(DEFAULTMAXDELTA)
	{ /* nothing */ }

	/********************************************************************************************/
	/**
	 * Initialization of the parent class'es adaption probability.
	 *
	 * @param probability The likelihood for a adaption actually taking place
	 */
	GNumBiGaussAdaptorT(const double& probability)
		: GAdaptorT<num_type> (probability)
		, useSymmetricSigmas_(true)
		, sigma1_(DEFAULTSIGMA)
		, sigmaSigma1_(DEFAULTSIGMASIGMA)
		, minSigma1_(DEFAULTMINSIGMA)
		, maxSigma1_(DEFAULTMAXSIGMA)
		, sigma2_(DEFAULTSIGMA)
		, sigmaSigma2_(DEFAULTSIGMASIGMA)
		, minSigma2_(DEFAULTMINSIGMA)
		, maxSigma2_(DEFAULTMAXSIGMA)
		, delta_(DEFAULTDELTA)
		, sigmaDelta_(DEFAULTSIGMADELTA)
		, minDelta_(DEFAULTMINDELTA)
		, maxDelta_(DEFAULTMAXDELTA)
	{ /* nothing */ }

	/********************************************************************************************/
	/**
	 * A standard copy constructor. It assumes that the values of the other object are correct
	 * and does no additional error checks.
	 *
	 * @param cp Another GNumBiGaussAdaptorT object
	 */
	GNumBiGaussAdaptorT(const GNumBiGaussAdaptorT<num_type, fp_type>& cp)
		: GAdaptorT<num_type> (cp)
		, useSymmetricSigmas_(true)
		, sigma1_(cp.sigma1_)
		, sigmaSigma1_(cp.sigmaSigma1_)
		, minSigma1_(cp.minSigma1_)
		, maxSigma1_(cp.maxSigma1_)
		, sigma2_(cp.sigma2_)
		, sigmaSigma2_(cp.sigmaSigma2_)
		, minSigma2_(cp.minSigma2_)
		, maxSigma2_(cp.maxSigma2_)
		, delta_(cp.delta_)
		, sigmaDelta_(cp.sigmaDelta_)
		, minDelta_(cp.minDelta_)
		, maxDelta_(cp.maxDelta_)
	{ /* nothing */	}

	/********************************************************************************************/
	/**
	 * The standard destructor. Empty, as we have no local, dynamically
	 * allocated data.
	 */
	virtual ~GNumBiGaussAdaptorT()
	{ /* nothing */ }

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
	boost::optional<std::string> checkRelationshipWith(
			const GObject& cp
			, const Gem::Common::expectation& e
			, const double& limit
			, const std::string& caller
			, const std::string& y_name
			, const bool& withMessages
	) const {
	    using namespace Gem::Common;

		// Check that we are indeed dealing with a GParamterBase reference
		const GNumBiGaussAdaptorT<num_type, fp_type>  *p_load = GObject::gobject_conversion<GNumBiGaussAdaptorT<num_type, fp_type> >(&cp);

		// Will hold possible deviations from the expectation, including explanations
	    std::vector<boost::optional<std::string> > deviations;

		// Check our parent class'es data ...
		deviations.push_back(GAdaptorT<num_type>::checkRelationshipWith(cp, e, limit, "GNumBiGaussAdaptorT<num_type, fp_type>", y_name, withMessages));

		// ... and then our local data
		deviations.push_back(checkExpectation(withMessages, "GNumBiGaussAdaptorT<num_type, fp_type>", useSymmetricSigmas_, p_load->useSymmetricSigmas_, "useSymmetricSigmas_", "p_load->useSymmetricSigmas_", e , limit));
		deviations.push_back(checkExpectation(withMessages, "GNumBiGaussAdaptorT<num_type, fp_type>", sigma1_, p_load->sigma1_, "sigma1_", "p_load->sigma1_", e , limit));
		deviations.push_back(checkExpectation(withMessages, "GNumBiGaussAdaptorT<num_type, fp_type>", sigmaSigma1_, p_load->sigmaSigma1_, "sigmaSigma1_", "p_load->sigmaSigma1_", e , limit));
		deviations.push_back(checkExpectation(withMessages, "GNumBiGaussAdaptorT<num_type, fp_type>", minSigma1_, p_load->minSigma1_, "minSigma1_", "p_load->minSigma1_", e , limit));
		deviations.push_back(checkExpectation(withMessages, "GNumBiGaussAdaptorT<num_type, fp_type>", maxSigma1_, p_load->maxSigma1_, "maxSigma1_", "p_load->maxSigma1_", e , limit));
		deviations.push_back(checkExpectation(withMessages, "GNumBiGaussAdaptorT<num_type, fp_type>", sigma2_, p_load->sigma2_, "sigma2_", "p_load->sigma2_", e , limit));
		deviations.push_back(checkExpectation(withMessages, "GNumBiGaussAdaptorT<num_type, fp_type>", sigmaSigma2_, p_load->sigmaSigma2_, "sigmaSigma2_", "p_load->sigmaSigma2_", e , limit));
		deviations.push_back(checkExpectation(withMessages, "GNumBiGaussAdaptorT<num_type, fp_type>", minSigma2_, p_load->minSigma2_, "minSigma2_", "p_load->minSigma2_", e , limit));
		deviations.push_back(checkExpectation(withMessages, "GNumBiGaussAdaptorT<num_type, fp_type>", maxSigma2_, p_load->maxSigma2_, "maxSigma2_", "p_load->maxSigma2_", e , limit));
		deviations.push_back(checkExpectation(withMessages, "GNumBiGaussAdaptorT<num_type, fp_type>", delta_, p_load->delta_, "delta_", "p_load->delta_", e , limit));
		deviations.push_back(checkExpectation(withMessages, "GNumBiGaussAdaptorT<num_type, fp_type>", sigmaDelta_, p_load->sigmaDelta_, "sigmaDelta_", "p_load->sigmaDelta_", e , limit));
		deviations.push_back(checkExpectation(withMessages, "GNumBiGaussAdaptorT<num_type, fp_type>", minDelta_, p_load->minDelta_, "minDelta_", "p_load->minDelta_", e , limit));
		deviations.push_back(checkExpectation(withMessages, "GNumBiGaussAdaptorT<num_type, fp_type>", maxDelta_, p_load->maxDelta_, "maxDelta_", "p_load->maxDelta_", e , limit));


		return evaluateDiscrepancies("GNumBiGaussAdaptorT<num_type, fp_type>", caller, deviations, e);
	}

	/********************************************************************************************/
	/**
	 * Determines whether the two sigmas of the double-gaussian should be identical
	 *
	 * @param useSymmetricSigmas A boolean which determines whether the two sigmas of the double-gaussian should be identical
	 */
	void setUseSymmetricSigmas(const bool& useSymmetricSigmas) {
		useSymmetricSigmas_ = useSymmetricSigmas;
	}

	/********************************************************************************************/
	/**
	 * Retrieves the value of the "useSymmetricSigmas_" variable
	 *
	 * @return The value of the "useSymmetricSigmas_" variable
	 */
	bool getUseSymmetricSigmas() const {
		return useSymmetricSigmas_;
	}

	/********************************************************************************************/
	/**
	 * This function sets the value of the sigma1_ parameter.
	 *
	 * @param sigma1 The new value of the sigma_ parameter
	 */
	void setSigma1(const fp_type& sigma1)
	{
		// Sigma1 must be in the allowed value range
		if(sigma1 < minSigma1_ || sigma1 > maxSigma1_)
		{
			raiseException(
					"In GNumBiGaussAdaptorT<num_type, fp_type>::setSigma1(const fp_type&):" << std::endl
					<< "sigma1 is not in the allowed range: " << std::endl
					<< minSigma1_ << " <= " << sigma1 << " < " << maxSigma1_ << std::endl
					<< "If you want to use these values you need to" << std::endl
					<< "adapt the allowed range first."
			);
		}

		sigma1_ = sigma1;
	}

	/********************************************************************************************/
	/**
	 * Retrieves the current value of sigma1_.
	 *
	 * @return The current value of sigma1_
	 */
	fp_type getSigma1() const  {
		return sigma1_;
	}

	/********************************************************************************************/
	/**
	 * Sets the allowed value range of sigma1_. A minimum sigma1 of 0 will silently be adapted
	 * to a very small value (DEFAULTMINSIGMA), as otherwise adaptions would stop entirely,
	 * which does not make sense.  Using 0. as lower boundary is however allowed for practical
	 * reasons. Note that this function will also adapt sigma1 itself, if it falls outside of the
	 * allowed range.
	 *
	 * @param minSigma1 The minimum allowed value of sigma1_
	 * @param maxSigma1 The maximum allowed value of sigma1_
	 */
	void setSigma1Range(const fp_type& minSigma1, const fp_type& maxSigma1){
		fp_type tmpMinSigma1 = minSigma1;

		// Silently adapt minSigma1, if necessary. A value of 0. does not make sense.
		if(tmpMinSigma1>=fp_type(0.) && tmpMinSigma1<fp_type(DEFAULTMINSIGMA)) tmpMinSigma1 = fp_type(DEFAULTMINSIGMA);

		// Do some error checks
		if(tmpMinSigma1<=0. || tmpMinSigma1 >= maxSigma1){
			raiseException(
					"In GNumBiGaussAdaptorT::setSigma1Range(const fp_type&, const fp_type&):" << std::endl
					<< "Invalid values for minSigma1 and maxSigma1 given:" << std::endl
					<< tmpMinSigma1 << "," << maxSigma1
			);
		} // maxSigma1 will automatically be > 0. now

		minSigma1_ = tmpMinSigma1;
		maxSigma1_ = maxSigma1;

		// Rectify sigma1_, if necessary
		if(sigma1_<minSigma1_) sigma1_ = minSigma1_;
		else if(sigma1_>maxSigma1_) sigma1_ = maxSigma1_;
	}

	/********************************************************************************************/
	/**
	 * Retrieves the allowed value range for sigma1. You can retrieve the values
	 * like this: getSigma1Range().first , getSigmaRange().second .
	 *
	 * @return The allowed value range for sigma1
	 */
	boost::tuple<fp_type,fp_type> getSigma1Range() const  {
		return boost::make_tuple<fp_type,fp_type>(minSigma1_, maxSigma1_);
	}

	/********************************************************************************************/
	/**
	 * This function sets the values of the sigmaSigma1_ parameter. Values <= 0 mean "do not adapt
	 * sigma1_". If you do want to prevent adaption of sigma1, you can also use the
	 * GAdaptorT<T>::setAdaptionThreshold() function. It determines, after how many calls the
	 * internal parameters of the adaption should be adapted. If set to 0, no adaption takes place.
	 *
	 * @param sigmaSigma1 The new value of the sigmaSigma1_ parameter
	 */
	void setSigma1AdaptionRate(const fp_type& sigmaSigma1)
	{
		sigmaSigma1_ = sigmaSigma1;
	}

	/********************************************************************************************/
	/**
	 * Retrieves the value of sigmaSigma1_ .
	 *
	 * @return The value of the sigmaSigma1_ parameter
	 */
	fp_type getSigma1AdaptionRate() const  {
		return sigmaSigma1_;
	}

	/********************************************************************************************/
	/**
	 * Convenience function that lets users set all relevant parameters of the sigma1_ parameter
	 * at once
	 *
	 * @param sigma1 The initial value for the sigma1_ parameter
	 * @param sigmaSigma1 The initial value for the sigmaSigma1_ parameter
	 * @param minSigma1 The minimal value allowed for sigma1_
	 * @param minSigma1 The maximum value allowed for sigma1_
	 */
	void setAllSigma1(const fp_type& sigma1, const fp_type& sigmaSigma1, const fp_type& minSigma1, const fp_type& maxSigma1)
	{
		setSigma1AdaptionRate(sigmaSigma1);
		setSigma1Range(minSigma1, maxSigma1);
		setSigma1(sigma1);
	}

	/********************************************************************************************/
	/**
	 * This function sets the value of the sigma2_ parameter. Note that this function
	 * will silently set a 0 sigma2 to a very small value.
	 *
	 * @param sigma2 The new value of the sigma_ parameter
	 */
	void setSigma2(const fp_type& sigma2)
	{
		// Sigma1 must be in the allowed value range
		if(sigma2 < minSigma2_ || sigma2 > maxSigma2_)
		{
			raiseException(
					"In GNumBiGaussAdaptorT<num_type, fp_type>::setSigma2(const fp_type&):" << std::endl
					<< "sigma2 is not in the allowed range: " << std::endl
					<< minSigma2_ << " <= " << sigma2 << " < " << maxSigma2_ << std::endl
					<< "If you want to use these values you need to" << std::endl
					<< "adapt the allowed range first."
			);
		}

		sigma2_ = sigma2;
	}

	/********************************************************************************************/
	/**
	 * Retrieves the current value of sigma2_.
	 *
	 * @return The current value of sigma2_
	 */
	fp_type getSigma2() const  {
		return sigma2_;
	}

	/********************************************************************************************/
	/**
	 * Sets the allowed value range of sigma2_. A minimum sigma2_ of 0 will silently be adapted
	 * to a very small value (DEFAULTMINSIGMA), as otherwise adaptions would stop entirely,
	 * which does not make sense.  Using 0. as lower boundary is however allowed for practical
	 * reasons. Note that this function will also adapt sigma2 itself, if it falls outside of the
	 * allowed range.
	 *
	 * @param minSigma2 The minimum allowed value of sigma2_
	 * @param maxSigma2 The maximum allowed value of sigma2_
	 */
	void setSigma2Range(const fp_type& minSigma2, const fp_type& maxSigma2){
		fp_type tmpMinSigma2 = minSigma2;

		// Silently adapt minSigma2, if necessary. A value of 0. does not make sense.
		if(tmpMinSigma2>=fp_type(0.) && tmpMinSigma2<fp_type(DEFAULTMINSIGMA)) tmpMinSigma2 = fp_type(DEFAULTMINSIGMA);

		// Do some error checks
		if(tmpMinSigma2<=0. || tmpMinSigma2 >= maxSigma2){
			raiseException(
					"In GNumBiGaussAdaptorT::setSigma2Range(const fp_type&, const fp_type&):" << std::endl
					<< "Invalid values for minSigma2 and maxSigma2 given:" << std::endl
					<< tmpMinSigma2 << "," << maxSigma2
			);
		} // maxSigma1 will automatically be > 0. now

		minSigma2_ = tmpMinSigma2;
		maxSigma2_ = maxSigma2;

		// Rectify sigma1_, if necessary
		if(sigma2_<minSigma2_) sigma2_ = minSigma2_;
		else if(sigma2_>maxSigma2_) sigma2_ = maxSigma2_;
	}

	/********************************************************************************************/
	/**
	 * Retrieves the allowed value range for sigma2. You can retrieve the values
	 * like this: getSigma2Range().first , getSigmaRange().second .
	 *
	 * @return The allowed value range for sigma2
	 */
	boost::tuple<fp_type,fp_type> getSigma2Range() const  {
		return boost::make_tuple<fp_type,fp_type>(minSigma2_, maxSigma2_);
	}

	/********************************************************************************************/
	/**
	 * This function sets the values of the sigmaSigma2_ parameter. Values <= 0 mean "do not adapt
	 * sigma2_". If you do want to prevent adaption of sigma1, you can also use the
	 * GAdaptorT<T>::setAdaptionThreshold() function. It determines, after how many calls the
	 * internal parameters of the adaption should be adapted. If set to 0, no adaption takes place.
	 *
	 * @param sigmaSigma2 The new value of the sigmaSigma2_ parameter
	 */
	void setSigma2AdaptionRate(const fp_type& sigmaSigma2)
	{
		sigmaSigma2_ = sigmaSigma2;
	}

	/********************************************************************************************/
	/**
	 * Retrieves the value of sigmaSigma2_ .
	 *
	 * @return The value of the sigmaSigma2_ parameter
	 */
	fp_type getSigma2AdaptionRate() const  {
		return sigmaSigma2_;
	}

	/********************************************************************************************/
	/**
	 * Convenience function that lets users set all relevant parameters of the sigma2_ parameter
	 * at once
	 *
	 * @param sigma2 The initial value for the sigma2_ parameter
	 * @param sigmaSigma2 The initial value for the sigmaSigma2_ parameter
	 * @param minSigma2 The minimal value allowed for sigma2_
	 * @param minSigma2 The maximum value allowed for sigma2_
	 */
	void setAllSigma2(const fp_type& sigma2, const fp_type& sigmaSigma2, const fp_type& minSigma2, const fp_type& maxSigma2)
	{
		setSigma2AdaptionRate(sigmaSigma2);
		setSigma2Range(minSigma2, maxSigma2);
		setSigma2(sigma2);
	}

	/********************************************************************************************/
	/**
	 * This function sets the value of the delta_ parameter.
	 *
	 * @param delta The new value of the sigma_ parameter
	 */
	void setDelta(const fp_type& delta)
	{
		// Delta must be in the allowed value range
		if(delta < minDelta_ || delta > maxDelta_)
		{
			raiseException(
					"In GNumBiGaussAdaptorT::setDelta(const fp_type&):" << std::endl
					<< "delta is not in the allowed range: " << std::endl
					<< minDelta_ << " <= " << delta << " < " << maxDelta_ << std::endl
					<< "If you want to use these values you need to" << std::endl
					<< "adapt the allowed range first."
			);
		}

		delta_ = delta;
	}

	/********************************************************************************************/
	/**
	 * Retrieves the current value of delta_.
	 *
	 * @return The current value of delta_
	 */
	fp_type getDelta() const  {
		return delta_;
	}

	/********************************************************************************************/
	/**
	 * Sets the allowed value range of delta_. A minimum delta of 0 will silently be adapted
	 * to DEFAULTMINSIGMA, if that value is > 0. Note that this function will also adapt delta
	 * itself, if it falls outside of the allowed range.
	 *
	 * @param minDelta The minimum allowed value of delta_
	 * @param maxDelta The maximum allowed value of delta_
	 */
	void setDeltaRange(const fp_type& minDelta, const fp_type& maxDelta){
		fp_type tmpMinDelta = minDelta;

		// Silently adapt tmpMinDelta, if necessary. A value of 0 is allowed
		// (distance of 0 between both peaks).
		if(tmpMinDelta>fp_type(0.) && tmpMinDelta<fp_type(DEFAULTMINDELTA)) {
			tmpMinDelta = fp_type(DEFAULTMINDELTA);
		}

		// Do some error checks
		if(tmpMinDelta<0. || tmpMinDelta >= maxDelta){
			raiseException(
					"In GNumBiGaussAdaptorT<num_type, fp_type>::setDeltaRange(const fp_type&, const fp_type&):" << std::endl
					<< "Invalid values for minDelta and maxDelta given:" << std::endl
					<< tmpMinDelta << "," << maxDelta
			);
		} // maxDelta will automatically be > 0. now

		minDelta_ = tmpMinDelta;
		maxDelta_ = maxDelta;

		// Adapt delta, if necessary
		if(delta_ < minDelta_) delta_ = minDelta_;
		else if(delta_>maxDelta_) delta_ = maxDelta_;
	}

	/********************************************************************************************/
	/**
	 * Retrieves the allowed value range for delta. You can retrieve the values
	 * like this: getDeltaRange().first , getSigmaRange().second .
	 *
	 * @return The allowed value range for delta
	 */
	boost::tuple<fp_type,fp_type> getDeltaRange() const  {
		return boost::make_tuple<fp_type,fp_type>(minDelta_, maxDelta_);
	}

	/********************************************************************************************/
	/**
	 * This function sets the values of the sigmaSigma2_ parameter. Values <= 0 mean "do not adapt
	 * delta_". If you do want to prevent adaption of delta_, you can also use the
	 * GAdaptorT<T>::setAdaptionThreshold() function. It determines, after how many calls the
	 * internal parameters of the adaption should be adapted. If set to 0, no adaption takes place.
	 *
	 * @param sigmaDelta The new value of the sigmaDelta_ parameter
	 */
	void setDeltaAdaptionRate(const fp_type& sigmaDelta)
	{
		sigmaDelta_ = sigmaDelta;
	}

	/********************************************************************************************/
	/**
	 * Retrieves the value of sigmaDelta_ .
	 *
	 * @return The value of the sigmaDelta_ parameter
	 */
	fp_type getDeltaAdaptionRate() const  {
		return sigmaDelta_;
	}

	/********************************************************************************************/
	/**
	 * Convenience function that lets users set all relevant parameters of the delta_ parameter
	 * at once
	 *
	 * @param delta The initial value for the delta_ parameter
	 * @param sigmaDelta The initial value for the sigmaDelta_ parameter
	 * @param minDelta The minimal value allowed for delta_
	 * @param minDelta The maximum value allowed for delta_
	 */
	void setAllDelta(const fp_type& delta, const fp_type& sigmaDelta, const fp_type& minDelta, const fp_type& maxDelta)
	{
		setDeltaAdaptionRate(sigmaDelta);
		setDeltaRange(minDelta, maxDelta);
		setDelta(delta);
	}

	/***********************************************************************************/
	/** @brief Retrieves the id of the adaptor */
	virtual Gem::Geneva::adaptorId getAdaptorId() const = 0;

protected:
	/********************************************************************************************/
	// For performance reasons, so we do not have to go through access functions
	bool useSymmetricSigmas_; ///< Determines whether the sigmas of both gaussians should be the sam
	fp_type sigma1_; ///< The width of the first gaussian used to adapt values
	fp_type sigmaSigma1_; ///< affects sigma1_ adaption
	fp_type minSigma1_; ///< minimum allowed value for sigma1_
	fp_type maxSigma1_; ///< maximum allowed value for sigma1_
	fp_type sigma2_; ///< The width of the second gaussian used to adapt values
	fp_type sigmaSigma2_; ///< affects sigma2_ adaption
	fp_type minSigma2_; ///< minimum allowed value for sigma2_
	fp_type maxSigma2_; ///< maximum allowed value for sigma2_
	fp_type delta_; ///< The distance between both gaussians
	fp_type sigmaDelta_; ///< affects the adaption of delta_
	fp_type minDelta_; ///< minimum allowed value for delta_
	fp_type maxDelta_; ///< maximum allowed value for delta_

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
		const GNumBiGaussAdaptorT<num_type, fp_type> *p_load = GObject::gobject_conversion<GNumBiGaussAdaptorT<num_type, fp_type> >(cp);

		// Load the data of our parent class ...
		GAdaptorT<num_type>::load_(cp);

		// ... and then our own data
		useSymmetricSigmas_ = p_load->useSymmetricSigmas_;
		sigma1_ = p_load->sigma1_;
		sigmaSigma1_ = p_load->sigmaSigma1_;
		minSigma1_ = p_load->minSigma1_;
		maxSigma1_ = p_load->maxSigma1_;
		sigma2_ = p_load->sigma2_;
		sigmaSigma2_ = p_load->sigmaSigma2_;
		minSigma2_ = p_load->minSigma2_;
		maxSigma2_ = p_load->maxSigma2_;
		delta_ = p_load->delta_;
		sigmaDelta_ = p_load->sigmaDelta_;
		minDelta_ = p_load->minDelta_;
		maxDelta_ = p_load->maxDelta_;
	}

	/********************************************************************************************/
	/** @brief This function creates a deep copy of this object */
	virtual GObject *clone_() const = 0;

	/********************************************************************************************/
	/**
	 * This adaptor allows the evolutionary adaption of sigma_. This allows the
	 * algorithm to adapt to changing geometries of the quality surface.
	 *
	 * ATTENTION: sigma/delta may become 0 here ??!?
	 */
	virtual void adaptAdaption()
	{
		// We do not want to favor the decrease or increase of sigma1/2 and delta, hence we choose
		// randomly whether to multiply or divide. TODO: cross-check.
		if(sigmaSigma1_ > fp_type(0.)) {
			sigma1_ *= exp(GAdaptorT<num_type>::gr->normal_distribution(sigmaSigma1_)*(GAdaptorT<num_type>::gr->uniform_bool()?1:-1));
		}
		if(sigmaSigma2_ > fp_type(0.)){
			sigma2_ *= exp(GAdaptorT<num_type>::gr->normal_distribution(sigmaSigma2_)*(GAdaptorT<num_type>::gr->uniform_bool()?1:-1));
		}
		if(sigmaDelta_ > fp_type(0.)) {
			delta_ *= exp(GAdaptorT<num_type>::gr->normal_distribution(sigmaDelta_)*(GAdaptorT<num_type>::gr->uniform_bool()?1:-1));
		}

		// make sure valued don't get out of range
		if(sigma1_ < minSigma1_) sigma1_ = minSigma1_;
		else if(sigma1_ > maxSigma1_) sigma1_ = maxSigma1_;

		if(sigma2_ < minSigma2_) sigma2_ = minSigma2_;
		else if(sigma2_ > maxSigma2_) sigma2_ = maxSigma2_;

		if(delta_ < minDelta_) delta_ = minDelta_;
		else if(delta_ > maxDelta_) delta_ = maxDelta_;

		// Make sure that the appropriate actions are performed by the parent class
		GAdaptorT<num_type>::adaptAdaption();
	}

	/********************************************************************************************/
	/**
	 * The actual adaption of the supplied value takes place here. Purely virtual, as the actual
	 * adaptions are defined in the derived classes.
	 *
	 * @param value The value that is going to be adapted in situ
	 */
	virtual void customAdaptions(num_type&) = 0;

#ifdef GEM_TESTING
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
		if(GAdaptorT<num_type>::modify_GUnitTests()) result = true;

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
		GAdaptorT<num_type>::specificTestsNoFailureExpected_GUnitTests();
	}

	/***********************************************************************************/
	/**
	 * Performs self tests that are expected to fail. This is needed for testing purposes
	 */
	virtual void specificTestsFailuresExpected_GUnitTests() {
		using boost::unit_test_framework::test_suite;
		using boost::unit_test_framework::test_case;

		// Call the parent classes' functions
		GAdaptorT<num_type>::specificTestsFailuresExpected_GUnitTests();
	}

#endif /* GEM_TESTING */
};

/************************************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

/************************************************************************************************/
// The content of BOOST_SERIALIZATION_ASSUME_ABSTRACT(T)
namespace boost {
	namespace serialization {
		template<typename num_type, typename fp_type>
		struct is_abstract< Gem::Geneva::GNumBiGaussAdaptorT<num_type, fp_type> > : public boost::true_type {};
		template<typename num_type, typename fp_type>
		struct is_abstract< const Gem::Geneva::GNumBiGaussAdaptorT<num_type, fp_type> > : public boost::true_type {};
	}
}

#endif /* GNUMBIGAUSSADAPTORT_HPP_ */
