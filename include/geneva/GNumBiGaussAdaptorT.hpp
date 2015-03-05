/**
 * @file GNumBiGaussAdaptorT.hpp
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

#ifndef GNUMBIGAUSSADAPTORT_HPP_
#define GNUMBIGAUSSADAPTORT_HPP_


// Geneva headers go here
#include "geneva/GAdaptorT.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
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
	G_API_GENEVA void serialize(Archive & ar, const unsigned int) {
		using boost::serialization::make_nvp;

		// Save all necessary data
		ar
		& make_nvp("GAdaptorT_num", boost::serialization::base_object<GAdaptorT<num_type> >(*this))
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
	/***************************************************************************/
	/**
	 * The standard constructor.
	 */
	G_API_GENEVA GNumBiGaussAdaptorT()
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

	/***************************************************************************/
	/**
	 * Initialization of the parent class'es adaption probability.
	 *
	 * @param probability The likelihood for a adaption actually taking place
	 */
	G_API_GENEVA GNumBiGaussAdaptorT(const double& probability)
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

	/***************************************************************************/
	/**
	 * A standard copy constructor. It assumes that the values of the other object are correct
	 * and does no additional error checks.
	 *
	 * @param cp Another GNumBiGaussAdaptorT object
	 */
	G_API_GENEVA GNumBiGaussAdaptorT(const GNumBiGaussAdaptorT<num_type, fp_type>& cp)
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

	/***************************************************************************/
	/**
	 * The standard destructor. Empty, as we have no local, dynamically
	 * allocated data.
	 */
	virtual G_API_GENEVA ~GNumBiGaussAdaptorT()
	{ /* nothing */ }

	/***************************************************************************/
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
	G_API_GENEVA boost::optional<std::string> checkRelationshipWith(
      const GObject& cp
      , const Gem::Common::expectation& e
      , const double& limit
      , const std::string& caller
      , const std::string& y_name
      , const bool& withMessages
	) const OVERRIDE {
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

	/***************************************************************************/
	/**
	 * Determines whether the two sigmas of the double-gaussian should be identical
	 *
	 * @param useSymmetricSigmas A boolean which determines whether the two sigmas of the double-gaussian should be identical
	 */
	G_API_GENEVA void setUseSymmetricSigmas(const bool& useSymmetricSigmas) {
		useSymmetricSigmas_ = useSymmetricSigmas;
	}

	/***************************************************************************/
	/**
	 * Retrieves the value of the "useSymmetricSigmas_" variable
	 *
	 * @return The value of the "useSymmetricSigmas_" variable
	 */
	G_API_GENEVA bool getUseSymmetricSigmas() const {
		return useSymmetricSigmas_;
	}

	/***************************************************************************/
	/**
	 * This function sets the value of the sigma1_ parameter. It is recommended
	 * that the value lies in the range [0.:1.]. A value below 0 is not allowed.
	 * Sigma is interpreted as a percentage of the allowed or desired value range
	 * of the target variable.
	 *
	 * @param sigma1 The new value of the sigma_ parameter
	 */
	G_API_GENEVA void setSigma1(const fp_type& sigma1) {
		// Sigma1 must be in the allowed value range
		if(sigma1 < minSigma1_ || sigma1 > maxSigma1_ || sigma1 < fp_type(0)) {
		   glogger
		   << "In GNumBiGaussAdaptorT<num_type, fp_type>::setSigma1(const fp_type&):" << std::endl
         << "sigma1 is not in the allowed range: " << std::endl
         << minSigma1_ << " <= " << sigma1 << " < " << maxSigma1_ << std::endl
         << "If you want to use these values you need to" << std::endl
         << "adapt the allowed range first." << std::endl
         << GEXCEPTION;
		}

		sigma1_ = sigma1;
	}

	/***************************************************************************/
	/**
	 * Retrieves the current value of sigma1_.
	 *
	 * @return The current value of sigma1_
	 */
	G_API_GENEVA fp_type getSigma1() const  {
		return sigma1_;
	}

	/***************************************************************************/
	/**
	 * Sets the allowed value range of sigma1_. A minimum sigma1 of 0 will silently be adapted
	 * to a very small value (DEFAULTMINSIGMA), as otherwise adaptions would stop entirely,
	 * which does not make sense.  Using 0. as lower boundary is however allowed for practical
	 * reasons. Note that this function will also adapt sigma1 itself, if it falls outside of the
	 * allowed range. It is not recommended (but not enforced) to set a maxSigma1 > 1, as sigma
	 * is interpreted as a percentage of the allowed or desired value range of the target variable.
	 *
	 * @param minSigma1 The minimum allowed value of sigma1_
	 * @param maxSigma1 The maximum allowed value of sigma1_
	 */
	G_API_GENEVA void setSigma1Range(
      const fp_type& minSigma1
      , const fp_type& maxSigma1
   ){
	   using namespace Gem::Common;

	   if(minSigma1 < fp_type(0.) || minSigma1 > maxSigma1 || maxSigma1 < boost::numeric_cast<fp_type>(DEFAULTMINSIGMA)) {
	      glogger
	      << "In GNumBiGaussAdaptorT::setSigma1Range(const fp_type&, const fp_type&):" << std::endl
         << "Invalid values for minSigma1 and maxSigma1 given: " << minSigma1 << " / " << maxSigma1 << std::endl
         << GEXCEPTION;
	   }

      minSigma1_ = minSigma1;
      maxSigma1_ = maxSigma1;

      // Silently adapt minSigma1_, if it is smaller than DEFAULTMINSIGMA. E.g., a value of 0 does not make sense
      if(minSigma1_ < fp_type(DEFAULTMINSIGMA)) {
         minSigma1_ = fp_type(DEFAULTMINSIGMA);
      }

		// Rectify sigma1_, if necessary
      enforceRangeConstraint(sigma1_, minSigma1_, maxSigma1_);
	}

	/***************************************************************************/
	/**
	 * Retrieves the allowed value range for sigma1. You can retrieve the values
	 * like this: getSigma1Range().first , getSigmaRange().second .
	 *
	 * @return The allowed value range for sigma1
	 */
	G_API_GENEVA boost::tuple<fp_type,fp_type> getSigma1Range() const  {
		return boost::make_tuple<fp_type,fp_type>(minSigma1_, maxSigma1_);
	}

	/***************************************************************************/
	/**
	 * This function sets the values of the sigmaSigma1_ parameter. Values <= 0 mean "do not adapt
	 * sigma1_". If you do want to prevent adaption of sigma1, you can also use the
	 * GAdaptorT<T>::setAdaptionThreshold() function. It determines, after how many calls the
	 * internal parameters of the adaption should be adapted. If set to 0, no adaption takes place.
	 *
	 * TODO: Cross-check suitable values
	 *
	 * @param sigmaSigma1 The new value of the sigmaSigma1_ parameter
	 */
	G_API_GENEVA void setSigma1AdaptionRate(const fp_type& sigmaSigma1)
	{
		sigmaSigma1_ = sigmaSigma1;
	}

	/***************************************************************************/
	/**
	 * Retrieves the value of sigmaSigma1_ .
	 *
	 * @return The value of the sigmaSigma1_ parameter
	 */
	G_API_GENEVA fp_type getSigma1AdaptionRate() const  {
		return sigmaSigma1_;
	}

	/***************************************************************************/
	/**
	 * Convenience function that lets users set all relevant parameters of the sigma1_ parameter
	 * at once
	 *
	 * @param sigma1 The initial value for the sigma1_ parameter
	 * @param sigmaSigma1 The initial value for the sigmaSigma1_ parameter
	 * @param minSigma1 The minimal value allowed for sigma1_
	 * @param minSigma1 The maximum value allowed for sigma1_
	 */
	G_API_GENEVA void setAllSigma1(
      const fp_type& sigma1
      , const fp_type& sigmaSigma1
      , const fp_type& minSigma1
      , const fp_type& maxSigma1
   ) {
		setSigma1AdaptionRate(sigmaSigma1);
		setSigma1Range(minSigma1, maxSigma1);
		setSigma1(sigma1);
	}

	/***************************************************************************/
	/**
	 * This function sets the value of the sigma2_ parameter. It is recommended
    * that the value lies in the range [0.:1.]. A value below 0 is not allowed.
    * Sigma is interpreted as a percentage of the allowed or desired value range
    * of the target variable.
	 *
	 * @param sigma2 The new value of the sigma_ parameter
	 */
	G_API_GENEVA void setSigma2(const fp_type& sigma2)
	{
      // Sigma2 must be in the allowed value range
	   if(sigma2 < minSigma2_ || sigma2 > maxSigma2_ || sigma2 < fp_type(0)) {
		   glogger
		   << "In GNumBiGaussAdaptorT<num_type, fp_type>::setSigma2(const fp_type&):" << std::endl
		   << "sigma2 is not in the allowed range: " << std::endl
         << minSigma2_ << " <= " << sigma2 << " < " << maxSigma2_ << std::endl
         << "If you want to use this value for sigma you need to" << std::endl
         << "adapt the allowed range first." << std::endl
         << GEXCEPTION;
		}

		sigma2_ = sigma2;
	}

	/***************************************************************************/
	/**
	 * Retrieves the current value of sigma2_.
	 *
	 * @return The current value of sigma2_
	 */
	G_API_GENEVA fp_type getSigma2() const  {
		return sigma2_;
	}

	/***************************************************************************/
	/**
	 * Sets the allowed value range of sigma2_. A minimum sigma2 of 0 will silently be adapted
    * to a very small value (DEFAULTMINSIGMA), as otherwise adaptions would stop entirely,
    * which does not make sense.  Using 0. as lower boundary is however allowed for practical
    * reasons. Note that this function will also adapt sigma2 itself, if it falls outside of the
    * allowed range. It is not recommended (but not enforced) to set a maxSigma2 > 1, as sigma
    * is interpreted as a percentage of the allowed or desired value range of the target variable.
	 *
	 * @param minSigma2 The minimum allowed value of sigma2_
	 * @param maxSigma2 The maximum allowed value of sigma2_
	 */
	G_API_GENEVA void setSigma2Range(const fp_type& minSigma2, const fp_type& maxSigma2){
	   using namespace Gem::Common;

      if(minSigma2 < fp_type(0.) || minSigma2 > maxSigma2 || maxSigma2 < boost::numeric_cast<fp_type>(DEFAULTMINSIGMA)) {
         glogger
         << "In GNumBiGaussAdaptorT::setSigma2Range(const fp_type&, const fp_type&):" << std::endl
         << "Invalid values for minSigma2 and maxSigma2 given: " << minSigma2 << " / " << maxSigma2 << std::endl
         << GEXCEPTION;
      }


      minSigma2_ = minSigma2;
      maxSigma2_ = maxSigma2;

      // Silently adapt minSigma1_, if it is smaller than DEFAULTMINSIGMA. E.g., a value of 0 does not make sense
      if(minSigma2_ < fp_type(DEFAULTMINSIGMA)) {
         minSigma2_ = fp_type(DEFAULTMINSIGMA);
      }

      // Rectify sigma1_, if necessary
      enforceRangeConstraint(sigma2_, minSigma2_, maxSigma2_);
	}

	/***************************************************************************/
	/**
	 * Retrieves the allowed value range for sigma2. You can retrieve the values
	 * like this: getSigma2Range().first , getSigmaRange().second .
	 *
	 * @return The allowed value range for sigma2
	 */
	G_API_GENEVA boost::tuple<fp_type,fp_type> getSigma2Range() const  {
		return boost::make_tuple<fp_type,fp_type>(minSigma2_, maxSigma2_);
	}

	/***************************************************************************/
	/**
	 * This function sets the values of the sigmaSigma2_ parameter. Values <= 0 mean "do not adapt
	 * sigma2_". If you do want to prevent adaption of sigma1, you can also use the
	 * GAdaptorT<T>::setAdaptionThreshold() function. It determines, after how many calls the
	 * internal parameters of the adaption should be adapted. If set to 0, no adaption takes place.
	 *
	 * @param sigmaSigma2 The new value of the sigmaSigma2_ parameter
	 */
	G_API_GENEVA void setSigma2AdaptionRate(const fp_type& sigmaSigma2)
	{
		sigmaSigma2_ = sigmaSigma2;
	}

	/***************************************************************************/
	/**
	 * Retrieves the value of sigmaSigma2_ .
	 *
	 * @return The value of the sigmaSigma2_ parameter
	 */
	G_API_GENEVA fp_type getSigma2AdaptionRate() const  {
		return sigmaSigma2_;
	}

	/***************************************************************************/
	/**
	 * Convenience function that lets users set all relevant parameters of the sigma2_ parameter
	 * at once
	 *
	 * @param sigma2 The initial value for the sigma2_ parameter
	 * @param sigmaSigma2 The initial value for the sigmaSigma2_ parameter
	 * @param minSigma2 The minimal value allowed for sigma2_
	 * @param minSigma2 The maximum value allowed for sigma2_
	 */
	G_API_GENEVA void setAllSigma2(
      const fp_type& sigma2
      , const fp_type& sigmaSigma2
      , const fp_type& minSigma2
      , const fp_type& maxSigma2
   ) {
		setSigma2AdaptionRate(sigmaSigma2);
		setSigma2Range(minSigma2, maxSigma2);
		setSigma2(sigma2);
	}

	/***************************************************************************/
	/**
	 * This function sets the value of the delta_ parameter. It is recommended
    * that the value lies in the range [0.:0.5]. A value below 0 is not allowed.
	 *
	 * @param delta The new value of the sigma_ parameter
	 */
	G_API_GENEVA void setDelta(const fp_type& delta)	{
		// Delta must be in the allowed value range
		if(delta < minDelta_ || delta > maxDelta_ || delta_ < fp_type(0))	{
		   glogger
		   << "In GNumBiGaussAdaptorT::setDelta(const fp_type&):" << std::endl
         << "delta is not in the allowed range: " << std::endl
         << minDelta_ << " <= " << delta << " < " << maxDelta_ << std::endl
         << "If you want to use these values you need to" << std::endl
         << "adapt the allowed range first." << std::endl
         << GEXCEPTION;
		}

		delta_ = delta;
	}

	/***************************************************************************/
	/**
	 * Retrieves the current value of delta_.
	 *
	 * @return The current value of delta_
	 */
	G_API_GENEVA fp_type getDelta() const  {
		return delta_;
	}

	/***************************************************************************/
	/**
	 * Sets the allowed value range of delta_. A minimum delta of 0 will silently be adapted
	 * to DEFAULTMINSIGMA, if that value is > 0. Note that this function will also adapt delta
	 * itself, if it falls outside of the allowed range. A maximum of 0.5 for maxDelta_ is
	 * recommended, but not enforced.delta is interpreted as a percentage of the allowed or
	 * desired value range of the target variable.
	 *
	 * @param minDelta The minimum allowed value of delta_
	 * @param maxDelta The maximum allowed value of delta_
	 */
	G_API_GENEVA void setDeltaRange(
      const fp_type& minDelta
      , const fp_type& maxDelta
   ){
      if(minDelta < fp_type(0.) || minDelta > maxDelta || maxDelta < boost::numeric_cast<fp_type>(DEFAULTMINDELTA)) {
         glogger
         << "In GNumBiGaussAdaptorT::setDeltaRange(const fp_type&, const fp_type&):" << std::endl
         << "Invalid values for minDelta and maxDelta given: " << minDelta << " / " << maxDelta << std::endl
         << GEXCEPTION;
      }

      minDelta_ = minDelta;
      maxDelta_ = maxDelta;

      // Note: In contrast to setSigmaXRange(...) we allow a delta < DEFAULTMINDELTA
      // (as long as it is >= 0), as a delta of 0 makes sense

      // Rectify delta_, if necessary
      if(delta_<minDelta_) {
         delta_ = minDelta_;
      } else if(delta_>maxDelta_) {
         delta_ = maxDelta_;
      }
	}

	/***************************************************************************/
	/**
	 * Retrieves the allowed value range for delta. You can retrieve the values
	 * like this: getDeltaRange().first , getSigmaRange().second .
	 *
	 * @return The allowed value range for delta
	 */
	G_API_GENEVA boost::tuple<fp_type,fp_type> getDeltaRange() const  {
		return boost::make_tuple<fp_type,fp_type>(minDelta_, maxDelta_);
	}

	/***************************************************************************/
	/**
	 * This function sets the values of the sigmaSigma2_ parameter. Values <= 0 mean "do not adapt
	 * delta_". If you do want to prevent adaption of delta_, you can also use the
	 * GAdaptorT<T>::setAdaptionThreshold() function. It determines, after how many calls the
	 * internal parameters of the adaption should be adapted. If set to 0, no adaption takes place.
	 *
	 * @param sigmaDelta The new value of the sigmaDelta_ parameter
	 */
	G_API_GENEVA void setDeltaAdaptionRate(const fp_type& sigmaDelta)
	{
		sigmaDelta_ = sigmaDelta;
	}

	/***************************************************************************/
	/**
	 * Retrieves the value of sigmaDelta_ .
	 *
	 * @return The value of the sigmaDelta_ parameter
	 */
	G_API_GENEVA fp_type getDeltaAdaptionRate() const  {
		return sigmaDelta_;
	}

	/***************************************************************************/
	/**
	 * Convenience function that lets users set all relevant parameters of the delta_ parameter
	 * at once
	 *
	 * @param delta The initial value for the delta_ parameter
	 * @param sigmaDelta The initial value for the sigmaDelta_ parameter
	 * @param minDelta The minimal value allowed for delta_
	 * @param minDelta The maximum value allowed for delta_
	 */
	G_API_GENEVA void setAllDelta(
      const fp_type& delta
      , const fp_type& sigmaDelta
      , const fp_type& minDelta
      , const fp_type& maxDelta
   ) {
		setDeltaAdaptionRate(sigmaDelta);
		setDeltaRange(minDelta, maxDelta);
		setDelta(delta);
	}

	/***********************************************************************************/
	/** @brief Retrieves the id of the adaptor */
	virtual G_API_GENEVA Gem::Geneva::adaptorId getAdaptorId() const = 0;

   /***************************************************************************/
   /**
    * Emits a name for this class / object
    */
   virtual G_API_GENEVA std::string name() const OVERRIDE {
      return std::string("GNumBiGaussAdaptorT");
   }

   /***************************************************************************/
   /**
    * Allows to randomly initialize parameter members
    */
   virtual G_API_GENEVA void randomInit() OVERRIDE {
      using namespace Gem::Hap;
      sigma1_ = this->gr->template uniform_real<fp_type>(minSigma1_, maxSigma1_);
      sigma2_ = this->gr->template uniform_real<fp_type>(minSigma2_, maxSigma2_);
      delta_  = this->gr->template uniform_real<fp_type>(minDelta_, maxDelta_);
   }

protected:
	/***************************************************************************/
	// For performance reasons, so we do not have to go through access functions
	bool useSymmetricSigmas_; ///< Determines whether the sigmas of both gaussians should be the same

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

	/***************************************************************************/
	/**
	 * This function loads the data of another GNumBiGaussAdaptorT, camouflaged as a GObject.
	 * We assume that the values given to us by the other object are correct and do no error checks.
	 *
	 * @param A copy of another GNumBiGaussAdaptorT, camouflaged as a GObject
	 */
	G_API_GENEVA void load_(const GObject *cp) OVERRIDE	{
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

   /***************************************************************************/
   /**
    * Adds a given property value to the vector or returns false, if the property
    * was not found.
    */
   virtual G_API_GENEVA bool customQueryProperty (
      const std::string& property
      , std::vector<boost::any>& data
   ) const OVERRIDE {
      if(property == "sigma1") {
         data.push_back(boost::any(sigma1_));
      } else if(property == "sigma2") {
         data.push_back(boost::any(sigma1_));
      } else if(property == "delta") {
         data.push_back(boost::any(delta_));
      } else {
         return false;
      }

      return true;
   }

	/***************************************************************************/
	/** @brief This function creates a deep copy of this object */
	virtual G_API_GENEVA GObject *clone_() const = 0;

	/***************************************************************************/
	/**
	 * This adaptor allows the evolutionary adaption of sigma_. This allows the
	 * algorithm to adapt to changing geometries of the quality surface.
	 *
	 * @param range A typical range for the parameter with type num_type (unused here)
	 */
	virtual G_API_GENEVA void customAdaptAdaption(const num_type&) OVERRIDE {
      using namespace Gem::Common;

      // The following random distribution slightly favours values < 1. Selection pressure
      // will keep the values higher if needed
      sigma1_ *= gexp(GAdaptorT<num_type>::gr->normal_distribution(gfabs(sigmaSigma1_)));
      sigma2_ *= gexp(GAdaptorT<num_type>::gr->normal_distribution(gfabs(sigmaSigma2_)));
      delta_  *= gexp(GAdaptorT<num_type>::gr->normal_distribution(gfabs(sigmaDelta_ )));

		// Make sure valued don't get out of range
      enforceRangeConstraint(sigma1_, minSigma1_, maxSigma1_);
      enforceRangeConstraint(sigma2_, minSigma2_, maxSigma2_);
      enforceRangeConstraint(delta_ , minDelta_ , maxDelta_);
	}

	/***************************************************************************/
	/**
	 * The actual adaption of the supplied value takes place here. Purely virtual, as the actual
	 * adaptions are defined in the derived classes.
	 *
	 * @param value The value that is going to be adapted in situ
	 * @param range A typical range for the parameter with type num_type (unused here)
	 */
	virtual G_API_GENEVA void customAdaptions(num_type&, const num_type&) = 0;

public:
	/***************************************************************************/
	/**
	 * Applies modifications to this object. This is needed for testing purposes
	 *
	 * @return A boolean which indicates whether modifications were made
	 */
	virtual G_API_GENEVA bool modify_GUnitTests() OVERRIDE {
#ifdef GEM_TESTING
		using boost::unit_test_framework::test_suite;
		using boost::unit_test_framework::test_case;

		bool result = false;

		// Call the parent classes' functions
		if(GAdaptorT<num_type>::modify_GUnitTests()) result = true;

		// A relatively harmless change
		sigmaSigma1_ *= 1.1;
		result = true;

		return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		condnotset("GNumBiGaussAdaptorT<>::modify_GUnitTests", "GEM_TESTING");
		return false;
#endif /* GEM_TESTING */
	}

	/***************************************************************************/
	/**
	 * Performs self tests that are expected to succeed. This is needed for testing purposes
	 */
	virtual G_API_GENEVA void specificTestsNoFailureExpected_GUnitTests() OVERRIDE {
#ifdef GEM_TESTING
		using boost::unit_test_framework::test_suite;
		using boost::unit_test_framework::test_case;

		// Call the parent classes' functions
		GAdaptorT<num_type>::specificTestsNoFailureExpected_GUnitTests();

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
      condnotset("GNumBiGaussAdaptorT<>::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
	}

	/***************************************************************************/
	/**
	 * Performs self tests that are expected to fail. This is needed for testing purposes
	 */
	virtual G_API_GENEVA void specificTestsFailuresExpected_GUnitTests() OVERRIDE {
#ifdef GEM_TESTING
		using boost::unit_test_framework::test_suite;
		using boost::unit_test_framework::test_case;

		// Call the parent classes' functions
		GAdaptorT<num_type>::specificTestsFailuresExpected_GUnitTests();

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
      condnotset("GNumBiGaussAdaptorT<>::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
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
		struct is_abstract< Gem::Geneva::GNumBiGaussAdaptorT<num_type, fp_type> > : public boost::true_type {};
		template<typename num_type, typename fp_type>
		struct is_abstract< const Gem::Geneva::GNumBiGaussAdaptorT<num_type, fp_type> > : public boost::true_type {};
	}
}

/******************************************************************************/

#endif /* GNUMBIGAUSSADAPTORT_HPP_ */
