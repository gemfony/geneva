/**
 * @file GIdentityAdaptorT.hpp
 */

/* Copyright (C) Dr. Ruediger Berlich and Karlsruhe Institute of Technology
 * (University of the State of Baden-Wuerttemberg and National Laboratory
 * of the Helmholtz Association)
 *
 * Contact: info [at] gemfony (dot) com
 *
 * This file is part of the Geneva library, Gemfony scientific's optimization
 * library.
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
#include <cmath>
#include <string>
#include <sstream>

// Includes check for correct Boost version(s)
#include "common/GGlobalDefines.hpp"

// Boost headers go here
#include <boost/cstdint.hpp>
#include <boost/limits.hpp>
#include <boost/cast.hpp>

#ifndef GIDENTITYADAPTORT_HPP_
#define GIDENTITYADAPTORT_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif

// Geneva headers go here
#include "common/GExceptions.hpp"
#include "GAdaptorT.hpp"
#include "GObject.hpp"
#include "GOptimizationEnums.hpp"

namespace Gem {
namespace Geneva {

/************************************************************************************************/
/**
 * GIdentityAdaptorT simply returns the unchanged value. It is used as the default adaptor, when
 * no adaptor has been registered or if certain values should remain unchanged.
 */
template<typename T>
class GIdentityAdaptorT:
public GAdaptorT<T>
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;
	template<typename Archive>
	void serialize(Archive & ar, const unsigned int) {
		using boost::serialization::make_nvp;
		ar & make_nvp("GAdaptorT", boost::serialization::base_object<GAdaptorT<T> >(*this));
	}
	///////////////////////////////////////////////////////////////////////

public:
	/********************************************************************************************/
	/**
	 * The standard constructor.
	 */
	GIdentityAdaptorT()
	  : GAdaptorT<T> ()
	 {
		GAdaptorT<T>::setAdaptionMode(false);

#ifdef DEBUG
		if(typeid(T) != typeid(double) &&
		   typeid(T) != typeid(bool) &&
		   typeid(T) != typeid(boost::int32_t)) {
				std::ostringstream error;
				error << "In GIdentityAdaptorT<T>::GIdentityAdaptorT() : Error!" << std::endl
					  << "Class was instantiated with a type it was not designed for." << std::endl
					  << "Typeid.name() is " << typeid(T).name() << std::endl;
				throw(Gem::Common::gemfony_error_condition(error.str()));
		}
#endif
	 }

	/********************************************************************************************/
	/**
	 * A standard copy constructor.
	 *
	 * @param cp Another GIdentityAdaptorT object
	 */
	GIdentityAdaptorT(const GIdentityAdaptorT<T>& cp)
	  : GAdaptorT<T>(cp)
	 {
		GAdaptorT<T>::setAdaptionMode(false);
	 }

	/********************************************************************************************/
	/**
	 * The standard destructor. Empty, as we have no local, dynamically
	 * allocated data.
	 */
	~GIdentityAdaptorT()
	{ /* nothing */ }

	/********************************************************************************************/
	/**
	 * A standard assignment operator for GIdentityAdaptorT objects,
	 *
	 * @param cp A copy of another GIdentityAdaptorT object
	 */
	const GIdentityAdaptorT<T>& operator=(const GIdentityAdaptorT<T>& cp)
	{
		GIdentityAdaptorT<T>::load_(&cp);
		return *this;
	}

	/********************************************************************************************/
	/**
	 * Checks for equality with another GIdentityAdaptorT<T> object
	 *
	 * @param  cp A constant reference to another GIdentityAdaptorT<T> object
	 * @return A boolean indicating whether both objects are equal
	 */
	bool operator==(const GIdentityAdaptorT<T>& cp) const {
		using namespace Gem::Common;
		// Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
		return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GIdentityAdaptorT<T>::operator==","cp", CE_SILENT);
	}

	/********************************************************************************************/
	/**
	 * Checks for inequality with another GIdentityAdaptorT<T> object
	 *
	 * @param  cp A constant reference to another GIdentityAdaptorT<T> object
	 * @return A boolean indicating whether both objects are inequal
	 */
	bool operator!=(const GIdentityAdaptorT<T>& cp) const {
		using namespace Gem::Common;
		// Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
		return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GIdentityAdaptorT<T>::operator!=","cp", CE_SILENT);
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
		const GIdentityAdaptorT<T>  *p_load = GObject::conversion_cast<GIdentityAdaptorT<T> >(&cp);

		// Will hold possible deviations from the expectation, including explanations
	    std::vector<boost::optional<std::string> > deviations;

		// Check our parent class'es data ...
		deviations.push_back(GAdaptorT<T>::checkRelationshipWith(cp, e, limit, "GIdentityAdaptor", y_name, withMessages));

		// no local data ...

		return evaluateDiscrepancies("GIdentityAdaptor", caller, deviations, e);
	}


	/********************************************************************************************/
	/**
	 * Retrieves the id of the adaptor.
	 *
	 * @return The id of the adaptor
	 */
	virtual Gem::Geneva::adaptorId getAdaptorId() const {
		return GIDENTITYADAPTOR;
	}

	/********************************************************************************************/
	/**
	 * Prevents the adaption mode to be reset. This function is a trap.
	 *
	 * @param adaptionMode The desired mode (always/never/with a given probability)
	 */
	virtual void setAdaptionMode(boost::logic::tribool adaptionMode) {
		std::ostringstream error;
		error << "In GIdentityAdaptor::setAdaptionMode(): Error!" << std::endl
			  << "This function should not have been called for this adaptor" << std::endl;
		throw(Gem::Common::gemfony_error_condition(error.str()));
	}

protected:
	/********************************************************************************************/
	/**
	 * This function loads the data of another GIdentityAdaptorT, camouflaged as a GObject.
	 *
	 * @param A copy of another GIdentityAdaptorT, camouflaged as a GObject
	 */
	void load_(const GObject *cp)
	{
		// Check that this object is not accidently assigned to itself
		GObject::selfAssignmentCheck<GIdentityAdaptorT<T> >(cp);

		// Load the data of our parent class ...
		GAdaptorT<T>::load_(cp);

		// no local data
	}

	/********************************************************************************************/
	/**
	 * This function creates a deep copy of this object
	 *
	 * @return A deep copy of this object
	 */
	virtual GObject *clone_() const	{
		return new GIdentityAdaptorT<T>(*this);
	}

	/********************************************************************************************/
	/**
	 * The identity adaptor does not change its arguments
	 *
	 * @param value The value to be adapted
	 */
	virtual void customAdaptions(T& value) {
		return; // nothing
	}

#ifdef GENEVATESTING
public:
	/***********************************************************************************/
	/**
	 * Applies modifications to this object. This is needed for testing purposes
	 *
	 * @return A boolean which indicates whether modifications were made
	 */
	virtual bool modify_GUnitTests() {
		bool result = false;

		// Call the parent classes' functions
		if(GAdaptorT<T>::modify_GUnitTests()) result = true;

		return result;
	}

	/***********************************************************************************/
	/**
	 * Performs self tests that are expected to succeed. This is needed for testing purposes
	 */
	virtual void specificTestsNoFailureExpected_GUnitTests() {
		// Call the parent classes' functions
		GAdaptorT<T>::specificTestsNoFailureExpected_GUnitTests();
	}

	/***********************************************************************************/
	/**
	 * Performs self tests that are expected to fail. This is needed for testing purposes
	 */
	virtual void specificTestsFailuresExpected_GUnitTests() {
		// Call the parent classes' functions
		GAdaptorT<T>::specificTestsFailuresExpected_GUnitTests();
	}

#endif /* GENEVATESTING */
};

} /* namespace Geneva */
} /* namespace Gem */

#endif /* GIDENTITYADAPTORT_HPP_ */
