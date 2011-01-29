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
		: GAdaptorT<T> (probability)
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
	GNumBiGaussAdaptorT(const GNumBiGaussAdaptorT<T>& cp)
		: GAdaptorT<T> (cp)
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
		deviations.push_back(checkExpectation(withMessages, "GNumBiGaussAdaptorT<T>", sigma1_, p_load->sigma1_, "sigma1_", "p_load->sigma1_", e , limit));
		deviations.push_back(checkExpectation(withMessages, "GNumBiGaussAdaptorT<T>", sigmaSigma1_, p_load->sigmaSigma1_, "sigmaSigma1_", "p_load->sigmaSigma1_", e , limit));
		deviations.push_back(checkExpectation(withMessages, "GNumBiGaussAdaptorT<T>", minSigma1_, p_load->minSigma1_, "minSigma1_", "p_load->minSigma1_", e , limit));
		deviations.push_back(checkExpectation(withMessages, "GNumBiGaussAdaptorT<T>", maxSigma1_, p_load->maxSigma1_, "maxSigma1_", "p_load->maxSigma1_", e , limit));
		deviations.push_back(checkExpectation(withMessages, "GNumBiGaussAdaptorT<T>", sigma2_, p_load->sigma2_, "sigma2_", "p_load->sigma2_", e , limit));
		deviations.push_back(checkExpectation(withMessages, "GNumBiGaussAdaptorT<T>", sigmaSigma2_, p_load->sigmaSigma2_, "sigmaSigma2_", "p_load->sigmaSigma2_", e , limit));
		deviations.push_back(checkExpectation(withMessages, "GNumBiGaussAdaptorT<T>", minSigma2_, p_load->minSigma2_, "minSigma2_", "p_load->minSigma2_", e , limit));
		deviations.push_back(checkExpectation(withMessages, "GNumBiGaussAdaptorT<T>", maxSigma2_, p_load->maxSigma2_, "maxSigma2_", "p_load->maxSigma2_", e , limit));
		deviations.push_back(checkExpectation(withMessages, "GNumBiGaussAdaptorT<T>", delta_, p_load->delta_, "delta_", "p_load->delta_", e , limit));
		deviations.push_back(checkExpectation(withMessages, "GNumBiGaussAdaptorT<T>", sigmaDelta_, p_load->sigmaDelta_, "sigmaDelta_", "p_load->sigmaDelta_", e , limit));
		deviations.push_back(checkExpectation(withMessages, "GNumBiGaussAdaptorT<T>", minDelta_, p_load->minDelta_, "minDelta_", "p_load->minDelta_", e , limit));
		deviations.push_back(checkExpectation(withMessages, "GNumBiGaussAdaptorT<T>", maxDelta_, p_load->maxDelta_, "maxDelta_", "p_load->maxDelta_", e , limit));


		return evaluateDiscrepancies("GNumBiGaussAdaptorT<T>", caller, deviations, e);
	}

	/********************************************************************************************/
	/**
	 * This function sets the value of the sigma1_ parameter. Note that this function
	 * will silently set a 0 sigma1 to a very small value.
	 *
	 * @param sigma1 The new value of the sigma_ parameter
	 */
	void setSigma1(const double& sigma1)
	{
		if(sigma1 < 0.) {
			raiseException(
					"In GNumBiGaussAdaptorT::setSigma1(const double&):" << std::endl
					<< "sigma1 is negative: " << sigma1
			);
		}

		double tmpSigma1;
		if(sigma1 == 0.) tmpSigma1 = DEFAULTMINSIGMA;
		else tmpSigma1 = sigma1;

		// Sigma1 must be in the allowed value range
		if(tmpSigma1 < minSigma1_ || tmpSigma1 > maxSigma1_)
		{
			raiseException(
					"In GNumBiGaussAdaptorT::setSigma1(const double&):" << std::endl
					<< "sigma1 is not in the allowed range: " << std::endl
					<< minSigma1_ << " <= " << tmpSigma1 << " < " << maxSigma1_ << std::endl
					<< "If you want to use these values you need to" << std::endl
					<< "adapt the allowed range first."
			);
		}

		sigma1_ = tmpSigma1;
	}

	/********************************************************************************************/
	/**
	 * Retrieves the current value of sigma1_.
	 *
	 * @return The current value of sigma1_
	 */
	double getSigma1() const  {
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
	void setSigma1Range(const double& minSigma1, const double& maxSigma1){
		double tmpMinSigma1 = minSigma1;
		// Silently adapt minSigma1, if necessary. A value of 0. does not make sense.
		if(minSigma1 == 0.) tmpMinSigma1 = DEFAULTMINSIGMA;

		// Do some error checks
		if(tmpMinSigma1<=0. || minSigma1 >= maxSigma1){ // maxSigma1 will automatically be > 0. now
			raiseException(
					"In GNumBiGaussAdaptorT::setSigma1Range(const double&, const double&):" << std::endl
					<< "Invalid values for minSigma1 and maxSigma1 given:" << std::endl
					<< tmpMinSigma1 << "," << maxSigma1
			);
		}

		minSigma1_ = tmpMinSigma1;
		maxSigma1_ = maxSigma1;

		// Adapt sigma1, if necessary
		if(sigma1_ < minSigma1_) sigma1_ = minSigma1_;
		else if(sigma1_>maxSigma1_) sigma1_ = maxSigma1_;
	}

	/********************************************************************************************/
	/**
	 * Retrieves the allowed value range for sigma1. You can retrieve the values
	 * like this: getSigma1Range().first , getSigmaRange().second .
	 *
	 * @return The allowed value range for sigma1
	 */
	std::pair<double,double> getSigma1Range() const  {
		return std::make_pair(minSigma1_, maxSigma1_);
	}

	/********************************************************************************************/
	/**
	 * This function sets the value of the sigmaSigma1_ parameter. Values <= 0 are not allowed.
	 * If you do want to prevent adaption of sigma1, you can use the GAdaptorT<T>::setAdaptionThreshold()
	 * function. It determines, after how many adaptions the internal parameters
	 * of the adaption should be adapted. If set to 0, no adaption takes place.
	 *
	 * @param sigmaSigma1 The new value of the sigmaSigma1_ parameter
	 */
	void setSigma1AdaptionRate(const double& sigmaSigma1)
	{
		// A value of sigmaSigma1 <= 0. is not useful.
		if(sigmaSigma1 <= 0.)
		{
			raiseException(
					"In GNumBiGaussAdaptorT::setSigmaAdaptionRate(double, double):" << std::endl
					<< "Bad value for sigmaSigma1 given: " << sigmaSigma1
			);
		}

		sigmaSigma1_ = sigmaSigma1;
	}

	/********************************************************************************************/
	/**
	 * Retrieves the value of sigmaSigma1_ .
	 *
	 * @return The value of the sigmaSigma1_ parameter
	 */
	double getSigma1AdaptionRate() const  {
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
	void setAllSigma1(const double& sigma1, const double& sigmaSigma1, const double& minSigma1, const double& maxSigma1)
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
	void setSigma2(const double& sigma2)
	{
		if(sigma2 < 0.) {
			raiseException(
					"In GNumBiGaussAdaptorT::setSigma2(const double&):" << std::endl
					<< "sigma2 is negative: " << sigma2
			);
		}

		double tmpSigma2;
		if(sigma2 == 0.) tmpSigma2 = DEFAULTMINSIGMA;
		else tmpSigma2 = sigma2;

		// Sigma2 must be in the allowed value range
		if(tmpSigma2 < minSigma2_ || tmpSigma2 > maxSigma2_)
		{
			raiseException(
					"In GNumBiGaussAdaptorT::setSigma2(const double&):" << std::endl
					<< "sigma2 is not in the allowed range: " << std::endl
					<< minSigma2_ << " <= " << tmpSigma2 << " < " << maxSigma2_ << std::endl
					<< "If you want to use these values you need to" << std::endl
					<< "adapt the allowed range first."
			);
		}

		sigma2_ = tmpSigma2;
	}

	/********************************************************************************************/
	/**
	 * Retrieves the current value of sigma2_.
	 *
	 * @return The current value of sigma2_
	 */
	double getSigma2() const  {
		return sigma2_;
	}

	/********************************************************************************************/
	/**
	 * Sets the allowed value range of sigma2_. A minimum sigma2 of 0 will silently be adapted
	 * to a very small value (DEFAULTMINSIGMA), as otherwise adaptions would stop entirely,
	 * which does not make sense.  Using 0. as lower boundary is however allowed for practical
	 * reasons. Note that this function will also adapt sigma2 itself, if it falls outside of the
	 * allowed range.
	 *
	 * @param minSigma2 The minimum allowed value of sigma2_
	 * @param maxSigma2 The maximum allowed value of sigma2_
	 */
	void setSigma2Range(const double& minSigma2, const double& maxSigma2){
		double tmpMinSigma2 = minSigma2;
		// Silently adapt minSigma2, if necessary. A value of 0. does not make sense.
		if(minSigma2 == 0.) tmpMinSigma2 = DEFAULTMINSIGMA;

		// Do some error checks
		if(tmpMinSigma2<=0. || minSigma2 >= maxSigma2){ // maxSigma2 will automatically be > 0. now
			raiseException(
					"In GNumBiGaussAdaptorT::setSigma2Range(const double&, const double&):" << std::endl
					<< "Invalid values for minSigma2 and maxSigma2 given:" << std::endl
					<< tmpMinSigma2 << "," << maxSigma2
			);
		}

		minSigma2_ = tmpMinSigma2;
		maxSigma2_ = maxSigma2;

		// Adapt sigma2, if necessary
		if(sigma2_ < minSigma2_) sigma2_ = minSigma2_;
		else if(sigma2_>maxSigma2_) sigma2_ = maxSigma2_;
	}

	/********************************************************************************************/
	/**
	 * Retrieves the allowed value range for sigma2. You can retrieve the values
	 * like this: getSigma2Range().first , getSigmaRange().second .
	 *
	 * @return The allowed value range for sigma2
	 */
	std::pair<double,double> getSigma2Range() const  {
		return std::make_pair(minSigma2_, maxSigma2_);
	}

	/********************************************************************************************/
	/**
	 * This function sets the value of the sigmaSigma2_ parameter. Values <= 0 are not allowed.
	 * If you do want to prevent adaption of sigma2, you can use the GAdaptorT<T>::setAdaptionThreshold()
	 * function. It determines, after how many adaptions the internal parameters
	 * of the adaption should be adapted. If set to 0, no adaption takes place.
	 *
	 * @param sigmaSigma2 The new value of the sigmaSigma2_ parameter
	 */
	void setSigma2AdaptionRate(const double& sigmaSigma2)
	{
		// A value of sigmaSigma2 <= 0. is not useful.
		if(sigmaSigma2 <= 0.)
		{
			raiseException(
					"In GNumBiGaussAdaptorT::setSigmaAdaptionRate(double, double):" << std::endl
					<< "Bad value for sigmaSigma2 given: " << sigmaSigma2
			);
		}

		sigmaSigma2_ = sigmaSigma2;
	}

	/********************************************************************************************/
	/**
	 * Retrieves the value of sigmaSigma2_ .
	 *
	 * @return The value of the sigmaSigma2_ parameter
	 */
	double getSigma2AdaptionRate() const  {
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
	void setAllSigma2(const double& sigma2, const double& sigmaSigma2, const double& minSigma2, const double& maxSigma2)
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
	void setDelta(const double& delta)
	{
		if(delta < 0.) {
			raiseException(
					"In GNumBiGaussAdaptorT::setDelta(const double&):" << std::endl
					<< "delta is negative: " << delta
			);
		}

		double tmpDelta;
		if(DEFAULTMINDELTA > 0. && delta == 0.) tmpDelta = DEFAULTMINDELTA;
		else tmpDelta = delta;

		// Delta must be in the allowed value range
		if(tmpDelta < minDelta_ || tmpDelta > maxDelta_)
		{
			raiseException(
					"In GNumBiGaussAdaptorT::setDelta(const double&):" << std::endl
					<< "delta is not in the allowed range: " << std::endl
					<< minDelta_ << " <= " << tmpDelta << " < " << maxDelta_ << std::endl
					<< "If you want to use these values you need to" << std::endl
					<< "adapt the allowed range first."
			);
		}

		delta_ = tmpDelta;
	}

	/********************************************************************************************/
	/**
	 * Retrieves the current value of delta_.
	 *
	 * @return The current value of delta_
	 */
	double getDelta() const  {
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
	void setDeltaRange(const double& minDelta, const double& maxDelta){
		double tmpMinDelta = minDelta;
		// Silently adapt minDelta, if necessary. A value of 0. does not make sense.
		if(DEFAULTMINDELTA > 0. && minDelta == 0.) tmpMinDelta = DEFAULTMINDELTA;

		// Do some error checks
		if(tmpMinDelta<=0. || minDelta >= maxDelta){ // maxDelta will automatically be > 0. now
			raiseException(
					"In GNumBiGaussAdaptorT::setDeltaRange(const double&, const double&):" << std::endl
					<< "Invalid values for minDelta and maxDelta given:" << std::endl
					<< tmpMinDelta << "," << maxDelta
			);
		}

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
	std::pair<double,double> getDeltaRange() const  {
		return std::make_pair(minDelta_, maxDelta_);
	}

	/********************************************************************************************/
	/**
	 * This function sets the value of the sigmaDelta_ parameter. Values <= 0 are not allowed.
	 * If you do want to prevent adaption of delta, you can use the GAdaptorT<T>::setAdaptionThreshold()
	 * function. It determines, after how many adaptions the internal parameters
	 * of the adaption should be adapted. If set to 0, no adaption takes place.
	 *
	 * @param sigmaDelta The new value of the sigmaDelta_ parameter
	 */
	void setDeltaAdaptionRate(const double& sigmaDelta)
	{
		// A value of sigmaDelta <= 0. is not useful.
		if(sigmaDelta <= 0.)
		{
			raiseException(
					"In GNumBiGaussAdaptorT::setSigmaAdaptionRate(double, double):" << std::endl
					<< "Bad value for sigmaDelta given: " << sigmaDelta
			);
		}

		sigmaDelta_ = sigmaDelta;
	}

	/********************************************************************************************/
	/**
	 * Retrieves the value of sigmaDelta_ .
	 *
	 * @return The value of the sigmaDelta_ parameter
	 */
	double getDeltaAdaptionRate() const  {
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
	void setAllDelta(const double& delta, const double& sigmaDelta, const double& minDelta, const double& maxDelta)
	{
		setDeltaAdaptionRate(sigmaDelta);
		setDeltaRange(minDelta, maxDelta);
		setDelta(delta);
	}

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
		sigma1_ *= exp(GAdaptorT<T>::gr->normal_distribution(sigmaSigma1_)*(GAdaptorT<T>::gr->uniform_bool()?1:-1));
		sigma2_ *= exp(GAdaptorT<T>::gr->normal_distribution(sigmaSigma2_)*(GAdaptorT<T>::gr->uniform_bool()?1:-1));
		delta_ *= exp(GAdaptorT<T>::gr->normal_distribution(sigmaDelta_)*(GAdaptorT<T>::gr->uniform_bool()?1:-1));

		// make sure valued don't get out of range
		if(sigma1_ < minSigma_) sigma1_ = minSigma1_;
		else if(sigma1_ > maxSigma1_) sigma1_ = maxSigma1_;

		if(sigma2_ < minSigma_) sigma2_ = minSigma2_;
		else if(sigma2_ > maxSigma2_) sigma2_ = maxSigma2_;

		if(delta_ < minDelta_) delta_ = minDelta_;
		else if(delta_ > maxDelta_) delta_ = maxDelta_;

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
