/**
 * @file GConstrainedIntegerT.hpp
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
#include <vector>
#include <sstream>
#include <cmath>
#include <typeinfo>
#include <limits>
#include <utility> // for std::pair

// Includes check for correct Boost version(s)
#include "GGlobalDefines.hpp"

// Boost headers go here

#include <boost/cstdint.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>
#include <boost/cast.hpp> // For boost::numeric_cast<>
#include <boost/limits.hpp>
#include <boost/concept_check.hpp>

#ifndef GCONSTRAINEDINTEGERT_HPP_
#define GCONSTRAINEDINTEGERT_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif

// GenEvA headers go here
#include "GObject.hpp"
#include "GConstrainedNumT.hpp"
#include "GExceptions.hpp"

namespace Gem
{
namespace GenEvA
{

/******************************************************************************/
/* The GConstrainedIntegerT class represents an integer type, such as an int or a long,
 * equipped with the ability to adapt itself. The value range can have an upper and a lower
 * limit.  Adapted values will only appear inside the given range to the user. Note that
 * appropriate adaptors (see e.g the GInt32FlipAdaptor class) need to be loaded in order
 * to benefit from the adaption capabilities. Both boundaries are inclusive, i.e.
 * [lower:upper]. We currently only allow signed integers.
 */
template <typename T>
class GConstrainedIntegerT
	:public GConstrainedNumT<T>
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int) {
		using boost::serialization::make_nvp;

		// Save data
		ar & make_nvp("GParameterT_T", boost::serialization::base_object<GConstrainedNumT<T> >(*this));
	}
	///////////////////////////////////////////////////////////////////////

	// Make sure this class can only be instantiated if T has an integer type
	BOOST_CONCEPT_ASSERT((boost::SignedInteger<T>));

public:
	/****************************************************************************/
	/**
	 * The default constructor
	 */
	GConstrainedIntegerT()
		: GConstrainedNumT<T>()
    { /* nothing */ }

	/****************************************************************************/
	/**
	 * A constructor that initializes the value only. The boundaries will
	 * be set to the maximum and minimum values of the corresponding type.
	 *
	 * @param val The desired external value of this object
	 */
	GConstrainedIntegerT(const T& val)
		: GConstrainedNumT<T>(val)
	{ /* nothing */	}

	/****************************************************************************/
	/**
	 * Initializes the boundaries and sets the value to a random number. randomInit_()
	 * for this function to work properly.
	 *
	 * @param lowerBoundary The lower boundary of the value range
	 * @param upperBoundary The upper boundary of the value range
	 */
	GConstrainedIntegerT(const T& lowerBoundary, const T& upperBoundary)
		: GConstrainedNumT<T>(lowerBoundary, upperBoundary)
	{ /* nothing */	}

	/****************************************************************************/
	/**
	 * Initialization with value and boundaries.
	 *
	 * @param val The desired value of this object
	 * @param lowerBoundary The lower boundary of the value range
	 * @param upperBoundary The upper boundary of the value range
	 */
	GConstrainedIntegerT(const T& val, const T& lowerBoundary, const T& upperBoundary)
		: GConstrainedNumT<T>(val, lowerBoundary, upperBoundary)
	{ /* nothing */	}

	/****************************************************************************/
	/**
	 * A standard copy constructor. Most work is done by the parent
	 * classes, we only need to copy the allowed value range.
	 *
	 * @param cp Another GConstrainedNumT<T> object
	 */
	GConstrainedIntegerT(const GConstrainedIntegerT<T>& cp)
		: GConstrainedNumT<T>(cp)
	{ /* nothing */ }

	/****************************************************************************/
	/**
	 * The standard destructor
	 */
	virtual ~GConstrainedIntegerT()
	{ /* nothing */	}

	/****************************************************************************/
	/**
	 * A standard assignment operator for GConstrainedIntegerT<T> objects
	 *
	 * @param cp A constant reference to another GConstrainedIntegerT<T> object
	 * @return A constant reference to this object
	 */
	const GConstrainedIntegerT<T>& operator=(const GConstrainedIntegerT<T>& cp) {
		GConstrainedIntegerT<T>::load_(&cp);
		return *this;
	}

	/****************************************************************************/
    /**
     * Checks equality of this object with another.
     *
     * @param cp A constant reference to another GConstrainedIntegerT<T> object
     * @return A boolean indicating whether both objects are equal
     */
	bool operator==(const GConstrainedIntegerT<T>& cp) const {
		using namespace Gem::Common;
		// Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
		return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GConstrainedIntegerT<T>::operator==","cp", CE_SILENT);
	}

	/****************************************************************************/
    /**
     * Checks inequality of this object with another.
     *
     * @param cp A constant reference to another GConstrainedIntegerT<T> object
     * @return A boolean indicating whether both objects are inequal
     */
	bool operator!=(const GConstrainedIntegerT<T>& cp) const {
		using namespace Gem::Common;
		// Means: The expectation of inequality was fulfilled, as no error text was emitted (which converts to "true")
		return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GConstrainedIntegerT<T>::operator!=","cp", CE_SILENT);
	}

	/****************************************************************************/
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
		const GConstrainedIntegerT<T>  *p_load = GObject::conversion_cast<GConstrainedIntegerT<T> >(&cp);

		// Will hold possible deviations from the expectation, including explanations
	    std::vector<boost::optional<std::string> > deviations;

		// Check our parent class'es data ...
		deviations.push_back(GConstrainedNumT<T>::checkRelationshipWith(cp, e, limit, "GConstrainedIntegerT<T>", y_name, withMessages));

		// ... no local data

		return evaluateDiscrepancies("GConstrainedIntegerT<T>", caller, deviations, e);
	}

	/****************************************************************************/
	/**
	 * The transfer function needed to calculate the externally visible value.
	 *
	 * @param val The value to which the transformation should be applied
	 * @return The transformed value
	 */
	virtual T transfer(const T& val) const {
		// Find out the size of the confined area

		if(val >= getLowerBoundary() && val <= getUpperBoundary()) {
			return val;
		}
		else if(val < getLowerBoundary()) {
			// Find out how many full value ranges val is
			// below the lower boundary
		}
		else { // val > getUpperBoundary()
			// Find out how many full value ranges val is
			// above the upper boundary
		}
	}

protected:
	/****************************************************************************/
	/**
	 * Loads the data of another GConstrainedIntegerT<T>, camouflaged as a GObject.
	 *
	 * @param cp Another GConstrainedIntegerT<T> object, camouflaged as a GObject
	 */
	virtual void load_(const GObject *cp) {
		// Convert GObject pointer to local format
		const GConstrainedIntegerT<T> *p_load	= GObject::conversion_cast<GConstrainedIntegerT<T> >(cp);

		// Load our parent class'es data ...
		GConstrainedNumT<T>::load_(cp);

		// no local data
	}

	/****************************************************************************/
	/** @brief Create a deep copy of this object -- Re-implement this in derived classes */
	virtual GObject *clone_() const = 0;

	/****************************************************************************/
	/**
	 * Randomly initializes the parameter (within its limits)
	 */
	virtual void randomInit_() {

	}

private:

#ifdef GENEVATESTING
public:
	/****************************************************************************/
	/**
	 * Applies modifications to this object. This is needed for testing purposes
	 *
	 * @return A boolean which indicates whether modifications were made
	 */
	virtual bool modify_GUnitTests() {
		bool result;

		// Call the parent classes' functions
		if(GParameterT<T>::modify_GUnitTests()) result = true;

		return result;
	}

	/****************************************************************************/
	/**
	 * Performs self tests that are expected to succeed. This is needed for testing purposes
	 */
	virtual void specificTestsNoFailureExpected_GUnitTests() {
		// Call the parent classes' functions
		GParameterT<T>::specificTestsNoFailureExpected_GUnitTests();
	}

	/****************************************************************************/
	/**
	 * Performs self tests that are expected to fail. This is needed for testing purposes
	 */
	virtual void specificTestsFailuresExpected_GUnitTests() {
		// Call the parent classes' functions
		GParameterT<T>::specificTestsFailuresExpected_GUnitTests();
	}

#endif /* GENEVATESTING */
};

} /* namespace GenEvA */
} /* namespace Gem */

/******************************************************************************/
// The content of BOOST_SERIALIZATION_ASSUME_ABSTRACT(T)

namespace boost {
	namespace serialization {
		template<typename T>
		struct is_abstract<Gem::GenEvA::GConstrainedIntegerT<T> > : public boost::true_type {};
		template<typename T>
		struct is_abstract< const Gem::GenEvA::GConstrainedIntegerT<T> > : public boost::true_type {};
	}
}

#endif /* GCONSTRAINEDINTEGERT_HPP_ */
