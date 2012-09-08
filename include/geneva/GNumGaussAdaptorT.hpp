/**
 * @file GNumGaussAdaptorT.hpp
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
#include <boost/tuple/tuple.hpp>
#include <boost/tuple/tuple_io.hpp>

#ifndef GNUMGAUSSADAPTORT_HPP_
#define GNUMGAUSSADAPTORT_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif


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
class GNumGaussAdaptorT :public GAdaptorT<num_type>
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int) {
		using boost::serialization::make_nvp;

		// Save all necessary data
		ar & make_nvp("GAdaptorT_num", boost::serialization::base_object<GAdaptorT<num_type> >(*this))
		   & BOOST_SERIALIZATION_NVP(sigma_)
		   & BOOST_SERIALIZATION_NVP(sigmaSigma_)
		   & BOOST_SERIALIZATION_NVP(minSigma_)
		   & BOOST_SERIALIZATION_NVP(maxSigma_);
	}
	///////////////////////////////////////////////////////////////////////

	// Make sure this class can only be instantiated if fp_type really is a floating point type
	BOOST_MPL_ASSERT((boost::is_floating_point<fp_type>));

public:
	/***************************************************************************/
	/**
	 * The standard constructor.
	 */
	GNumGaussAdaptorT()
		: GAdaptorT<num_type> ()
		, sigma_(fp_type(DEFAULTSIGMA))
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
		, sigmaSigma_(fp_type(DEFAULTSIGMASIGMA))
		, minSigma_(fp_type(DEFAULTMINSIGMA))
		, maxSigma_(fp_type(DEFAULTMAXSIGMA))
	{
		// These functions do error checks on their values
		setSigmaAdaptionRate(sigmaSigma);
		setSigmaRange(minSigma, maxSigma);
		setSigma(sigma);
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
		, sigmaSigma_(fp_type(DEFAULTSIGMASIGMA))
		, minSigma_(fp_type(DEFAULTMINSIGMA))
		, maxSigma_(fp_type(DEFAULTMAXSIGMA))
	{
		// These functions do error checks on their values
		setSigmaAdaptionRate(sigmaSigma);
		setSigmaRange(minSigma, maxSigma);
		setSigma(sigma);
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
		const GNumGaussAdaptorT<num_type, fp_type>  *p_load = GObject::gobject_conversion<GNumGaussAdaptorT<num_type, fp_type> >(&cp);

		// Will hold possible deviations from the expectation, including explanations
	    std::vector<boost::optional<std::string> > deviations;

		// Check our parent class'es data ...
		deviations.push_back(GAdaptorT<num_type>::checkRelationshipWith(cp, e, limit, "GNumGaussAdaptorT<num_type, fp_type>", y_name, withMessages));

		// ... and then our local data
		deviations.push_back(checkExpectation(withMessages, "GNumGaussAdaptorT<num_type, fp_type>", sigma_, p_load->sigma_, "sigma_", "p_load->sigma_", e , limit));
		deviations.push_back(checkExpectation(withMessages, "GNumGaussAdaptorT<num_type, fp_type>", sigmaSigma_, p_load->sigmaSigma_, "sigmaSigma_", "p_load->sigmaSigma_", e , limit));
		deviations.push_back(checkExpectation(withMessages, "GNumGaussAdaptorT<num_type, fp_type>", minSigma_, p_load->minSigma_, "minSigma_", "p_load->minSigma_", e , limit));
		deviations.push_back(checkExpectation(withMessages, "GNumGaussAdaptorT<num_type, fp_type>", maxSigma_, p_load->maxSigma_, "maxSigma_", "p_load->maxSigma_", e , limit));

		return evaluateDiscrepancies("GNumGaussAdaptorT<num_type, fp_type>", caller, deviations, e);
	}

	/***************************************************************************/
	/**
	 * This function sets the value of the sigma_ parameter.
	 *
	 * @param sigma The new value of the sigma_ parameter
	 */
	void setSigma(const fp_type& sigma)
	{
		// Sigma must be in the allowed value range.
		if(sigma < minSigma_ || sigma > maxSigma_)
		{
			raiseException(
					"In GNumGaussAdaptorT::setSigma(const fp_type&):" << std::endl
					<< "sigma is not in the allowed range: " << std::endl
					<< minSigma_ << " <= " << sigma << " < " << maxSigma_ << std::endl
					<< "If you want to use these values you need to" << std::endl
					<< "adapt the allowed range first."
			);
		}

		sigma_ = sigma;
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
	 * reasons. Note that this function will also adapt sigma itself, if it falls outside of the allowed
	 * range.
	 *
	 * @param minSigma The minimum allowed value of sigma_
	 * @param maxSigma The maximum allowed value of sigma_
	 */
	void setSigmaRange(const fp_type& minSigma, const fp_type& maxSigma){
		fp_type tmpMinSigma = minSigma;

		// Silently adapt minSigma, if necessary. A value of 0. does not make sense.
		if(tmpMinSigma>=fp_type(0.) && tmpMinSigma<fp_type(DEFAULTMINSIGMA)) tmpMinSigma = fp_type(DEFAULTMINSIGMA);

		// Do some error checks
		if(tmpMinSigma<fp_type(0.) || tmpMinSigma >= maxSigma){
			raiseException(
					"In GNumGaussAdaptorT<num_type, fp_type>::setSigmaRange(const fp_type&, const fp_type&):" << std::endl
					<< "Invalid values for minSigma and maxSigma given:" << tmpMinSigma << " " << maxSigma
			);
		} // maxSigma will automatically be > 0. now

		minSigma_ = tmpMinSigma;
		maxSigma_ = maxSigma;

		// Adapt sigma, if necessary
		if(sigma_ < minSigma_) sigma_ = minSigma_;
		else if(sigma_>maxSigma_) sigma_ = maxSigma_;
	}

	/* ----------------------------------------------------------------------------------
	 * Setting of valid ranges is tested in GNumGaussAdaptorT<T>::specificTestsNoFailuresExpected_GUnitTests()
	 * Setting of invalid ranges is tested in GNumGaussAdaptorT<T>::specificTestsFailuresExpected_GUnitTests()
	 * ----------------------------------------------------------------------------------
	 */

	/***************************************************************************/
	/**
	 * Retrieves the allowed value range for sigma. You can retrieve the values
	 * like this: boost::get<0>(getSigmaRange()) , boost::get<1>(getSigmaRange()) .
	 *
	 * @return The allowed value range for sigma
	 */
	boost::tuple<fp_type,fp_type> getSigmaRange() const  {
		return boost::make_tuple<fp_type, fp_type>(minSigma_, maxSigma_);
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
	void setAll(const fp_type& sigma, const fp_type& sigmaSigma, const fp_type& minSigma, const fp_type& maxSigma)
	{
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
	virtual std::string printDiagnostics() const {
		std::ostringstream diag;
		boost::tuple<fp_type,fp_type> sigmaRange = getSigmaRange();

		diag << "Diagnostic message by GNumAdaptorT<num_type,fp_type>" << std::endl
			 << "with typeid(num_type).name() = " << typeid(num_type).name() << std::endl
			 << "and typeid(fp_type).name() = " << typeid(fp_type).name() << " :" << std::endl
			 << "getSigma() = " << getSigma() << std::endl
			 << "getSigmaRange() = " << boost::get<0>(sigmaRange) << " --> " << boost::get<1>(sigmaRange) << std::endl
		     << "getSigmaAdaptionRate() = " << getSigmaAdaptionRate() << std::endl;

		return diag.str();
	}

	/***************************************************************************/
	/**
	 * @brief Retrieves the id of the adaptor. */
	virtual Gem::Geneva::adaptorId getAdaptorId() const = 0;

protected:
	/***************************************************************************/
	/**
	 * This function loads the data of another GNumGaussAdaptorT<num_type, fp_type>, camouflaged as a GObject.
	 * We assume that the values given to us by the other object are correct and do no error checks.
	 *
	 * @param A copy of another GNumGaussAdaptorT<num_type, fp_type>, camouflaged as a GObject
	 */
	void load_(const GObject *cp)
	{
		// Convert GObject pointer to local format
		const GNumGaussAdaptorT<num_type, fp_type> *p_load = GObject::gobject_conversion<GNumGaussAdaptorT<num_type, fp_type> >(cp);

		// Load the data of our parent class ...
		GAdaptorT<num_type>::load_(cp);

		// ... and then our own data
		sigma_ = p_load->sigma_;
		sigmaSigma_ = p_load->sigmaSigma_;
		minSigma_ = p_load->minSigma_;
		maxSigma_ = p_load->maxSigma_;
	}

	/***************************************************************************/
	/**
	 * This function creates a deep copy of this object. Purely virtual so this class cannot
	 * be instantiated directly.
	 *
	 * @return A deep copy of this object
	 */
	virtual GObject *clone_() const = 0;

	/***************************************************************************/
	/**
	 * This adaptor allows the evolutionary adaption of sigma_. This allows the
	 * algorithm to adapt to changing geometries of the quality surface.
	 */
	virtual void customAdaptAdaption()
	{
	    using namespace Gem::Common;

		// We do not want to favor the decrease or increase of sigma, hence we choose
		// randomly whether to multiply or divide. TODO: cross-check.
		if(sigmaSigma_ > fp_type(0.)) sigma_ *= GExp(GAdaptorT<num_type>::gr->normal_distribution(sigmaSigma_)*(GAdaptorT<num_type>::gr->uniform_bool()?fp_type(1):fp_type(-1)));

		// make sure sigma_ doesn't get out of range
		if(sigma_ < minSigma_) sigma_ = minSigma_;
		else if(sigma_ > maxSigma_) sigma_ = maxSigma_;
	}

	/***************************************************************************/
	/**
	 * The actual adaption of the supplied value takes place here. Purely virtual, as the actual
	 * adaptions are defined in the derived classes.
	 *
	 * @param value The value that is going to be adapted in situ
	 */
	virtual void customAdaptions(num_type&) = 0;

protected: // For performance reasons, so we do not have to go through access functions
	/***************************************************************************/
	fp_type sigma_; ///< The width of the gaussian used to adapt values
	fp_type sigmaSigma_; ///< affects sigma_ adaption
	fp_type minSigma_; ///< minimum allowed value for sigma_
	fp_type maxSigma_; ///< maximum allowed value for sigma_

#ifdef GEM_TESTING
public:
	/***************************************************************************/
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
		sigmaSigma_ *= fp_type(1.1);
		result = true;

		return result;
	}

	/***************************************************************************/
	/**
	 * Performs self tests that are expected to succeed. This is needed for testing purposes
	 */
	virtual void specificTestsNoFailureExpected_GUnitTests() {
		using boost::unit_test_framework::test_suite;
		using boost::unit_test_framework::test_case;

		// Call the parent classes' functions
		GAdaptorT<num_type>::specificTestsNoFailureExpected_GUnitTests();

		//------------------------------------------------------------------------------

		{ // Test setting and retrieval of the sigma range
			boost::shared_ptr<GNumGaussAdaptorT<num_type, fp_type> > p_test = this->GObject::clone<GNumGaussAdaptorT<num_type, fp_type> >();

			for(fp_type dlower=fp_type(0.); dlower<fp_type(1.); dlower+=fp_type(0.1)) {
				fp_type dupper = fp_type(2.)*dlower;

				BOOST_CHECK_NO_THROW(p_test->setSigmaRange(dlower, dlower==fp_type(0.)?fp_type(1.):dupper));
				typename boost::tuple<fp_type, fp_type> range;
				BOOST_CHECK_NO_THROW(range = p_test->getSigmaRange());

				using namespace boost;

				if(dlower == 0.) { // Account for the fact that a lower boundary of 0. will be silently changed
					BOOST_CHECK(boost::get<0>(range) == boost::numeric_cast<fp_type>(DEFAULTMINSIGMA));
					BOOST_CHECK(boost::get<1>(range) == boost::numeric_cast<fp_type>(1.));
				}
				else {
					BOOST_CHECK(boost::get<0>(range) == dlower);
					BOOST_CHECK(fabs(boost::get<1>(range) - 2.*dlower) < fp_type(pow(10,-8))); // Take into account rounding errors
				}
			}

		}

		//------------------------------------------------------------------------------

		{ // Test that setting a sigma of 0. will result in a sigma with value DEFAULTMINSIGMA
			boost::shared_ptr<GNumGaussAdaptorT<num_type, fp_type> > p_test = this->GObject::clone<GNumGaussAdaptorT<num_type, fp_type> >();

			BOOST_CHECK_NO_THROW(p_test->setSigmaRange(fp_type(0.), fp_type(2.)));
			BOOST_CHECK_NO_THROW(p_test->setSigma(fp_type(DEFAULTMINSIGMA)));
			BOOST_CHECK(p_test->getSigma() == fp_type(DEFAULTMINSIGMA));
		}

		//------------------------------------------------------------------------------

		{ // Tests setting and retrieval of the sigma parameter
			boost::shared_ptr<GNumGaussAdaptorT<num_type, fp_type> > p_test = this->GObject::clone<GNumGaussAdaptorT<num_type, fp_type> >();

			BOOST_CHECK_NO_THROW(p_test->setSigmaRange(fp_type(0.), fp_type(2.)));

			for(fp_type d=fp_type(0.1); d<fp_type(1.9); d+=fp_type(0.1)) {
				BOOST_CHECK_NO_THROW(p_test->setSigma(d));
				BOOST_CHECK(p_test->getSigma() == d);
			}
		}

		//------------------------------------------------------------------------------

		{ // Test setting and retrieval of the sigma adaption rate
			boost::shared_ptr<GNumGaussAdaptorT<num_type, fp_type> > p_test = this->GObject::clone<GNumGaussAdaptorT<num_type, fp_type> >();

			for(fp_type d=fp_type(0.1); d<fp_type(1.9); d+=fp_type(0.1)) {
				BOOST_CHECK_NO_THROW(p_test->setSigmaAdaptionRate(d));
				BOOST_CHECK(p_test->getSigmaAdaptionRate() == d);
			}
		}

		//------------------------------------------------------------------------------

		{ // Check that simultaneous setting of all "sigma-values" has an effect
			using namespace boost;

			boost::shared_ptr<GNumGaussAdaptorT<num_type, fp_type> > p_test = this->GObject::clone<GNumGaussAdaptorT<num_type, fp_type> >();

			BOOST_CHECK_NO_THROW(p_test->setAll(fp_type(0.5), fp_type(0.8), fp_type(0.), fp_type(1.)));
			BOOST_CHECK(p_test->getSigma() == fp_type(0.5));
			BOOST_CHECK(p_test->getSigmaAdaptionRate() == fp_type(0.8));
			boost::tuple<fp_type, fp_type> range;
			BOOST_CHECK_NO_THROW(range = p_test->getSigmaRange());
			BOOST_CHECK(boost::get<0>(range) == fp_type(DEFAULTMINSIGMA));
			BOOST_CHECK(boost::get<1>(range) == fp_type(1.));
		}

		//------------------------------------------------------------------------------

		{ // Test sigma adaption
			boost::shared_ptr<GNumGaussAdaptorT<num_type, fp_type> > p_test = this->GObject::clone<GNumGaussAdaptorT<num_type, fp_type> >();

			// true: Adaptions should happen always, independent of the adaption probability
			BOOST_CHECK_NO_THROW (p_test->setAdaptionMode(true));

			const fp_type minSigma = fp_type(0.0001);
			const fp_type maxSigma = fp_type(5);
			const fp_type sigmaStart = fp_type(1);
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
				BOOST_CHECK_NO_THROW(p_test->adaptAdaption());
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
	}

	/***************************************************************************/
	/**
	 * Performs self tests that are expected to fail. This is needed for testing purposes
	 */
	virtual void specificTestsFailuresExpected_GUnitTests() {
		using boost::unit_test_framework::test_suite;
		using boost::unit_test_framework::test_case;

		// Call the parent classes' functions
		GAdaptorT<num_type>::specificTestsFailuresExpected_GUnitTests();

		//------------------------------------------------------------------------------

		{ // Test that setting a minimal sigma < 0. throws
			boost::shared_ptr<GNumGaussAdaptorT<num_type, fp_type> > p_test = this->GObject::clone<GNumGaussAdaptorT<num_type, fp_type> >();

			BOOST_CHECK_THROW(p_test->setSigmaRange(fp_type(-1.), fp_type(2.)), Gem::Common::gemfony_error_condition);
		}

		//------------------------------------------------------------------------------

		{ // Test that setting a minimal sigma > the maximum sigma throws
			boost::shared_ptr<GNumGaussAdaptorT<num_type, fp_type> > p_test = this->GObject::clone<GNumGaussAdaptorT<num_type, fp_type> >();

			BOOST_CHECK_THROW(p_test->setSigmaRange(fp_type(2.), fp_type(1.)), Gem::Common::gemfony_error_condition);
		}

		//------------------------------------------------------------------------------

		{ // Test that setting a negative sigma throws
			boost::shared_ptr<GNumGaussAdaptorT<num_type, fp_type> > p_test = this->GObject::clone<GNumGaussAdaptorT<num_type, fp_type> >();

			BOOST_CHECK_THROW(p_test->setSigma(fp_type(-1.)), Gem::Common::gemfony_error_condition);
		}

		//------------------------------------------------------------------------------

		{ // Test that setting a sigma below the allowed range throws
			boost::shared_ptr<GNumGaussAdaptorT<num_type, fp_type> > p_test = this->GObject::clone<GNumGaussAdaptorT<num_type, fp_type> >();

			BOOST_CHECK_NO_THROW(p_test->setSigmaRange(fp_type(0.5), fp_type(2.)));
			BOOST_CHECK_THROW(p_test->setSigma(fp_type(0.1)), Gem::Common::gemfony_error_condition);
		}

		//------------------------------------------------------------------------------

		{ // Test that setting a sigma above the allowed range throws
			boost::shared_ptr<GNumGaussAdaptorT<num_type, fp_type> > p_test = this->GObject::clone<GNumGaussAdaptorT<num_type, fp_type> >();

			BOOST_CHECK_NO_THROW(p_test->setSigmaRange(fp_type(0.5), fp_type(2.)));
			BOOST_CHECK_THROW(p_test->setSigma(fp_type(3.)), Gem::Common::gemfony_error_condition);
		}

		//------------------------------------------------------------------------------

	}

#endif /* GEM_TESTING */
};

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

/******************************************************************************/
// The content of BOOST_SERIALIZATION_ASSUME_ABSTRACT(T)
namespace boost {
	namespace serialization {
		template<typename num_type, typename fp_type>
		struct is_abstract< Gem::Geneva::GNumGaussAdaptorT<num_type, fp_type> > : public boost::true_type {};
		template<typename num_type, typename fp_type>
		struct is_abstract< const Gem::Geneva::GNumGaussAdaptorT<num_type, fp_type> > : public boost::true_type {};
	}
}

#endif /* GNUMGAUSSADAPTORT_HPP_ */
