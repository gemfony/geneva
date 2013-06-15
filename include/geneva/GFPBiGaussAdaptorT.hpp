/**
 * @file GFPBiGaussAdaptorT.hpp
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

#ifndef GFPBIGAUSSADAPTORT_HPP_
#define GFPBIGAUSSADAPTORT_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif


// Geneva headers go here
#include "geneva/GNumBiGaussAdaptorT.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * GFPBiGaussAdaptorT is used for the adaption of numeric types, by the addition of random numbers
 * distributed as two adjacent gaussians. Different numeric types may be used, including Boost's
 * integer representations. The type used needs to be specified as a template parameter. In comparison
 * to GNumGaussAdaptorT, an additional parameter "delta" is added, which represents the distance between
 * both gaussians. Just like sigma, delta can be subject to mutations. It is also possible to use
 * two different sigma/sigmaSigma values and adaption rates for both gaussians. Note that this adaptor
 * is experimental. Your mileage may vary.
 */
template<typename fp_type>
class GFPBiGaussAdaptorT :public GNumBiGaussAdaptorT<fp_type, fp_type>
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int) {
		using boost::serialization::make_nvp;

		// Save all necessary data
		ar & make_nvp("GAdaptorT_num", boost::serialization::base_object<GNumBiGaussAdaptorT<fp_type, fp_type> >(*this));
	}
	///////////////////////////////////////////////////////////////////////

	// Make sure this class can only be instantiated if fp_type really is a floating point type
	BOOST_MPL_ASSERT((boost::is_floating_point<fp_type>));

public:
	/***************************************************************************/
	/**
	 * The standard constructor.
	 */
	GFPBiGaussAdaptorT()
		: GNumBiGaussAdaptorT<fp_type, fp_type> ()
	{ /* nothing */ }

	/***************************************************************************/
	/**
	 * Initialization of the parent class'es adaption probability.
	 *
	 * @param probability The likelihood for a adaption actually taking place
	 */
	GFPBiGaussAdaptorT(const double& probability)
		: GNumBiGaussAdaptorT<fp_type, fp_type> (probability)
	{ /* nothing */ }

	/***************************************************************************/
	/**
	 * A standard copy constructor. It assumes that the values of the other object are correct
	 * and does no additional error checks.
	 *
	 * @param cp Another GFPBiGaussAdaptorT object
	 */
	GFPBiGaussAdaptorT(const GFPBiGaussAdaptorT<fp_type>& cp)
		: GNumBiGaussAdaptorT<fp_type, fp_type> (cp)
	{ /* nothing */	}

	/***************************************************************************/
	/**
	 * The standard destructor. Empty, as we have no local, dynamically
	 * allocated data.
	 */
	virtual ~GFPBiGaussAdaptorT()
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
	virtual boost::optional<std::string> checkRelationshipWith(
			const GObject& cp
			, const Gem::Common::expectation& e
			, const double& limit
			, const std::string& caller
			, const std::string& y_name
			, const bool& withMessages
	) const OVERRIDE	{
	    using namespace Gem::Common;

		// Check that we are indeed dealing with a GParamterBase reference
		const GFPBiGaussAdaptorT<fp_type>  *p_load = GObject::gobject_conversion<GFPBiGaussAdaptorT<fp_type> >(&cp);

		// Will hold possible deviations from the expectation, including explanations
	    std::vector<boost::optional<std::string> > deviations;

		// Check our parent class'es data ...
		deviations.push_back(GNumBiGaussAdaptorT<fp_type, fp_type>::checkRelationshipWith(cp, e, limit, "GNumBiGaussAdaptorT<fp_type, fp_type>", y_name, withMessages));

		// ... no local data


		return evaluateDiscrepancies("GFPBiGaussAdaptorT<fp_type>", caller, deviations, e);
	}

	/***********************************************************************************/
	/** @brief Retrieves the id of the adaptor */
	virtual Gem::Geneva::adaptorId getAdaptorId() const = 0;

   /***************************************************************************/
   /**
    * Emits a name for this class / object
    */
   virtual std::string name() const OVERRIDE {
      return std::string("GFPBiGaussAdaptorT");
   }

protected:
	/***************************************************************************/
	/**
	 * This function loads the data of another GFPBiGaussAdaptorT, camouflaged as a GObject.
	 * We assume that the values given to us by the other object are correct and do no error checks.
	 *
	 * @param A copy of another GFPBiGaussAdaptorT, camouflaged as a GObject
	 */
	void load_(const GObject *cp) OVERRIDE	{
		// Convert GObject pointer to local format
		const GFPBiGaussAdaptorT<fp_type> *p_load = GObject::gobject_conversion<GFPBiGaussAdaptorT<fp_type> >(cp);

		// Load the data of our parent class ...
		GNumBiGaussAdaptorT<fp_type, fp_type>::load_(cp);

		// no local data ...
	}

	/***************************************************************************/
	/** @brief This function creates a deep copy of this object */
	virtual GObject *clone_() const = 0;

	/***************************************************************************/
	/**
	 * The actual adaption of the supplied value takes place here
	 *
	 * @param value The value that is going to be adapted in situ
	 */
	virtual void customAdaptions(fp_type& value) OVERRIDE {
		if(GNumBiGaussAdaptorT<fp_type, fp_type>::useSymmetricSigmas_) { // Should we use the same sigma for both gaussians ?
			// adapt the value in situ. Note that this changes
			// the argument of this function
#if defined (CHECKOVERFLOWS)
			// Prevent over- and underflows. Note that we currently do not check the
			// size of "addition" in comparison to "value".
			fp_type addition = this->gr->bi_normal_distribution(
						fp_type(0.)
						, GNumBiGaussAdaptorT<fp_type, fp_type>::sigma1_
						, GNumBiGaussAdaptorT<fp_type, fp_type>::delta_
			);

			if(value >= fp_type(0.)){
				if(addition >= fp_type(0.) && (boost::numeric::bounds<fp_type>::highest()-value < addition)) {
#ifdef DEBUG
					std::cout << "Warning in GFPBiGaussAdaptor<fp_type>::customAdaptions():"
							  << "Had to change adaption due to overflow" << std::endl
							  << "value = " << value << std::endl
							  << "addition = " << addition << std::endl
							  << "boost::numeric::bounds<fp_type>::highest() = " << boost::numeric::bounds<fp_type>::highest() << std::endl;
#endif
					addition *= fp_type(-1.);
				}
			}
			else { // value < 0
				if(addition < fp_type(0.) && (Gem::Common::GFabs(boost::numeric::bounds<fp_type>::lowest() - value) < Gem::Common::GFabs(addition))) {
#ifdef DEBUG
					std::cout << "Warning in GFPGaussAdaptorT<fp_type>::customAdaptions():" << std::endl
							  << "Had to change adaption due to underflow" << std::endl
							  << "value = " << value << std::endl
							  << "addition = " << addition << std::endl
							  << "boost::numeric::bounds<fp_type>::lowest() = " << boost::numeric::bounds<fp_type>::lowest() << std::endl;
#endif
					addition *= fp_type(-1.);
				}
			}

			value += addition;
#else
			// We do not check for over- or underflows for performance reasons.
			value += this->gr->bi_normal_distribution(
					fp_type(0.)
					, GNumBiGaussAdaptorT<fp_type, fp_type>::sigma1_
					, GNumBiGaussAdaptorT<fp_type, fp_type>::delta_
			);
#endif /* CHECKOVERFLOWS */
		} else { // We allow asymmetric sigmas, i.e. different widths of both gaussians
			// adapt the value in situ. Note that this changes
			// the argument of this function
#if defined (CHECKOVERFLOWS)
			// Prevent over- and underflows. Note that we currently do not check the
			// size of "addition" in comparison to "value".
			fp_type addition = this->gr->bi_normal_distribution(
						fp_type(0.)
						, GNumBiGaussAdaptorT<fp_type, fp_type>::sigma1_
						, GNumBiGaussAdaptorT<fp_type, fp_type>::sigma2_
						, GNumBiGaussAdaptorT<fp_type, fp_type>::delta_
			);

			if(value >= fp_type(0.)){
				if(addition >= fp_type(0.) && (boost::numeric::bounds<fp_type>::highest()-value < addition)) {
#ifdef DEBUG
					std::cout << "Warning in GFPBiGaussAdaptor<fp_type>::customAdaptions():"
							  << "Had to change adaption due to overflow" << std::endl
							  << "value = " << value << std::endl
							  << "addition = " << addition << std::endl
							  << "boost::numeric::bounds<fp_type>::highest() = " << boost::numeric::bounds<fp_type>::highest() << std::endl;
#endif
					addition *= fp_type(-1.);
				}
			}
			else { // value < 0
				if(addition < fp_type(0.) && (Gem::Common::GFabs(boost::numeric::bounds<fp_type>::lowest() - value) < Gem::Common::GFabs(addition))) {
#ifdef DEBUG
					std::cout << "Warning in GFPGaussAdaptorT<fp_type>::customAdaptions():" << std::endl
							  << "Had to change adaption due to underflow" << std::endl
							  << "value = " << value << std::endl
							  << "addition = " << addition << std::endl
							  << "boost::numeric::bounds<fp_type>::lowest() = " << boost::numeric::bounds<fp_type>::lowest() << std::endl;
#endif
					addition *= fp_type(-1.);
				}
			}

			value += addition;
#else
			// We do not check for over- or underflows for performance reasons.
			value += this->gr->bi_normal_distribution(
					fp_type(0.)
					, GNumBiGaussAdaptorT<fp_type, fp_type>::sigma1_
					, GNumBiGaussAdaptorT<fp_type, fp_type>::sigma2_
					, GNumBiGaussAdaptorT<fp_type, fp_type>::delta_
			);
#endif /* CHECKOVERFLOWS */
		}
	}

public:
	/***************************************************************************/
	/**
	 * Applies modifications to this object. This is needed for testing purposes
	 *
	 * @return A boolean which indicates whether modifications were made
	 */
	virtual bool modify_GUnitTests() OVERRIDE {
#ifdef GEM_TESTING

		using boost::unit_test_framework::test_suite;
		using boost::unit_test_framework::test_case;

		bool result = false;

		// Call the parent classes' functions
		if(GNumBiGaussAdaptorT<fp_type, fp_type>::modify_GUnitTests()) result = true;

		return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GFPBiGaussAdaptorT<>::modify_GUnitTests", "GEM_TESTING");
   return false;
#endif /* GEM_TESTING */
	}

	/***************************************************************************/
	/**
	 * Performs self tests that are expected to succeed. This is needed for testing purposes
	 */
	virtual void specificTestsNoFailureExpected_GUnitTests() OVERRIDE {
#ifdef GEM_TESTING

		using boost::unit_test_framework::test_suite;
		using boost::unit_test_framework::test_case;

		// Call the parent classes' functions
		GNumBiGaussAdaptorT<fp_type, fp_type>::specificTestsNoFailureExpected_GUnitTests();

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
		condnotset("GFPBiGaussAdaptorT<>::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
	}

	/***************************************************************************/
	/**
	 * Performs self tests that are expected to fail. This is needed for testing purposes
	 */
	virtual void specificTestsFailuresExpected_GUnitTests() OVERRIDE {
#ifdef GEM_TESTING

		using boost::unit_test_framework::test_suite;
		using boost::unit_test_framework::test_case;

		// Call the parent classes' functions
		GNumBiGaussAdaptorT<fp_type, fp_type>::specificTestsFailuresExpected_GUnitTests();

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
      condnotset("GFPBiGaussAdaptorT<>::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
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
		struct is_abstract< Gem::Geneva::GFPBiGaussAdaptorT<fp_type> > : public boost::true_type {};
		template<typename fp_type>
		struct is_abstract< const Gem::Geneva::GFPBiGaussAdaptorT<fp_type> > : public boost::true_type {};
	}
}

/******************************************************************************/

#endif /* GFPBIGAUSSADAPTORT_HPP_ */
