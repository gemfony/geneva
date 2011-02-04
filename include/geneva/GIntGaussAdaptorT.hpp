/**
 * @file GIntGaussAdaptorT.hpp
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

#ifndef GINTGAUSSADAPTORT_HPP_
#define GINTGAUSSADAPTORT_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif


// Geneva headers go here

#include "geneva/GNumGaussAdaptorT.hpp"

namespace Gem {
namespace Geneva {

/*************************************************************************/
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

	  ar & make_nvp("GNumGaussAdaptorT_int", boost::serialization::base_object<GNumGaussAdaptorT<int_type, double> >(*this));
	}
	///////////////////////////////////////////////////////////////////////

public:
	/*********************************************************************/
	/**
	 * The default constructor
	 */
	GIntGaussAdaptorT()
		: GNumGaussAdaptorT<int_type, double>(DEFAULTINT32SIGMA, DEFAULTSIGMASIGMA, DEFAULTMINSIGMA, DEFAULTMAXSIGMA)
    { /* nothing */ }

	/*********************************************************************/
	/**
	 * The copy constructor
	 *
	 * @param cp A copy of another GIntGaussAdaptorT<int_type> object
	 */
	GIntGaussAdaptorT(const GIntGaussAdaptorT<int_type>& cp)
		: GNumGaussAdaptorT<int_type, double>(cp)
    { /* nothing */ }

	/*********************************************************************/
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

	/*********************************************************************/
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

	/*********************************************************************/
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

	/*********************************************************************/
	/**
	 * The destructor
	 */
	virtual ~GIntGaussAdaptorT()
	{ /* nothing */ }

	/*********************************************************************/
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
	) const {
	    using namespace Gem::Common;

	    // Check that we are not accidently assigning this object to itself
	    GObject::selfAssignmentCheck<GIntGaussAdaptorT<int_type> >(&cp);

		// Will hold possible deviations from the expectation, including explanations
	    std::vector<boost::optional<std::string> > deviations;

		// Check our parent class'es data ...
		deviations.push_back(GNumGaussAdaptorT<int_type, double>::checkRelationshipWith(cp, e, limit, "GIntGaussAdaptorT<int_type>", y_name, withMessages));

		// no local data ...

		return evaluateDiscrepancies("GIntGaussAdaptorT<int_type>", caller, deviations, e);
	}

	/*********************************************************************/
	/** @brief Retrieves the id of this adaptor */
	virtual Gem::Geneva::adaptorId getAdaptorId() const = 0;

protected:
	/*********************************************************************/
	/**
	 * Loads the data of another GObject
	 *
	 * @param cp A copy of another GIntGaussAdaptorT<int_type> object, camouflaged as a GObject
	 */
	virtual void load_(const GObject* cp) {
	    // Check that we are not accidently assigning this object to itself
	    GObject::selfAssignmentCheck<GIntGaussAdaptorT<int_type> >(cp);

		// Load our parent class'es data ...
		GNumGaussAdaptorT<int_type, double>::load_(cp);

		// ... no local data
	}

	/*********************************************************************/
	/** @brief Creates a deep clone of this object. */
	virtual GObject* clone_() const = 0;

	/*********************************************************************/
	/**
	 * The actual adaption of the supplied value takes place here.
	 *
	 * @param value The value that is going to be adapted in situ
	 */
	virtual void customAdaptions(int_type& value) {
		// adapt the value in situ. Note that this changes
		// the argument of this function
	#if defined (DEBUG)
		int_type addition = boost::numeric_cast<int_type>(this->gr->normal_distribution(GNumGaussAdaptorT<int_type, double>::sigma_));
	#else
		int_type addition = static_cast<int_type>(this->gr->normal_distribution(GNumGaussAdaptorT<int_type, double>::sigma_));
	#endif /* DEBUG */

		if(addition == 0) { // Enforce a minimal change of 1.
			this->gr->uniform_bool()?(addition=1):(addition=-1);
		}

	#if defined (CHECKOVERFLOWS)
		// Prevent over- and underflows.
		if(value >= 0){
			if(addition >= 0 && (std::numeric_limits<int_type>::max()-value < addition)) {
	#ifdef DEBUG
				std::cout << "Warning in GInt32GaussAdaptor::customAdaptions(): Had to change adaption due to overflow" << std::endl;
	#endif
				addition *= -1;
			}
		}
		else { // < 0
			if(addition < 0 && (std::numeric_limits<int_type>::min()-value > addition)) {
	#ifdef DEBUG
				std::cout << "Warning in GInt32GaussAdaptor::customAdaptions(): Had to change adaption due to underflow" << std::endl;
	#endif
				addition *= -1;
			}
		}
	#endif /* CHECKOVERFLOWS  */

		value += addition;
	}

	/* ----------------------------------------------------------------------------------
	 * - Tested in GNumGaussAdaptorT<int_type, double>::specificTestsNoFailuresExpected_GUnitTests()
	 * ----------------------------------------------------------------------------------
	 */

#ifdef GENEVATESTING
public:
	/*********************************************************************/
	/**
	 * Applies modifications to this object. This is needed for testing purposes
	 *
	 * @return A boolean which indicates whether modifications were made
	 */
	virtual bool modify_GUnitTests() {
		bool result = false;

		// Call the parent class'es function
		if(GNumGaussAdaptorT<int_type, double>::modify_GUnitTests()) result = true;

		return result;
	}

	/*********************************************************************/
	/**
	 * Performs self tests that are expected to succeed. This is needed for testing purposes
	 */
	virtual void specificTestsNoFailureExpected_GUnitTests() {
		// Call the parent class'es function
		GNumGaussAdaptorT<int_type, double>::specificTestsNoFailureExpected_GUnitTests();

		//------------------------------------------------------------------------------
		// nothing yet
		//------------------------------------------------------------------------------
	}

	/*********************************************************************/
	/**
	 * Performs self tests that are expected to fail. This is needed for testing purposes
	 */
	virtual void specificTestsFailuresExpected_GUnitTests() {
		// Call the parent class'es function
		GNumGaussAdaptorT<int_type, double>::specificTestsFailuresExpected_GUnitTests();
	}
#endif /* GENEVATESTING */
};


} /* namespace Geneva */
} /* namespace Gem */

/*************************************************************************/
// The content of BOOST_SERIALIZATION_ASSUME_ABSTRACT(T)

namespace boost {
	namespace serialization {
		template<typename int_type>
		struct is_abstract<Gem::Geneva::GIntGaussAdaptorT<int_type> > : public boost::true_type {};
		template<typename int_type>
		struct is_abstract< const Gem::Geneva::GIntGaussAdaptorT<int_type> > : public boost::true_type {};
	}
}
/*************************************************************************/

#endif /* GINTGAUSSADAPTORT_HPP_ */
