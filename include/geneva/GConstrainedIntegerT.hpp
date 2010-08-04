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
#include "common/GGlobalDefines.hpp"

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

// Geneva headers go here
#include "common/GExceptions.hpp"
#include "hap/GRandomT.hpp"
#include "GConstrainedNumT.hpp"
#include "GObject.hpp"

namespace Gem
{
namespace Geneva
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
		ar & make_nvp("GConstrainedNumT_T", boost::serialization::base_object<GConstrainedNumT<T> >(*this));
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
	explicit GConstrainedIntegerT(const T& val)
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
	 * A standard assignment operator for T values. Note that this function
	 * will throw an exception if the new value is not in the allowed value range.
	 *
	 * @param The desired new external value
	 * @return The new external value of this object
	 */
	virtual T operator=(const T& val) {
		return GConstrainedNumT<T>::operator=(val);
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

		if(val >= GConstrainedNumT<T>::getLowerBoundary() && val <= GConstrainedNumT<T>::getUpperBoundary()) {
			return val;
		}
		else {
			// The result
			std::size_t mapping = 0;

			// Find out the size of the value range. Note that both boundaries
			// are included, so that we need to add 1 to the difference.
			std::size_t value_range = GConstrainedNumT<T>::getUpperBoundary() - GConstrainedNumT<T>::getLowerBoundary() + 1;

			if(val < GConstrainedNumT<T>::getLowerBoundary()) {
				// Find out how many full value ranges val is below the lower boundary.
				// We use integer division here, so 13/4 would be 3.
				std::size_t nBelowLowerBoundary = (GConstrainedNumT<T>::getLowerBoundary() - (val + 1)) / value_range;

				// We are dealing with descending (nBelowLowerBoundary is even) and
				// ascending ranges (nBelowLowerBoundary is odd), which need to be treated differently
				if(nBelowLowerBoundary%2 == 0) { // nBelowLowerBoundary is even
					// Transfer the value into the allowed region
					mapping = val + (value_range * (nBelowLowerBoundary + 1));
				}
				else { // nBelowLowerBoundary is odd
					// Transfer the value into the allowed region
					mapping = val + (value_range * (nBelowLowerBoundary + 1));

					// Revert the value to a descending sequence
					mapping = revert(mapping);
				}
			}
			else { // val > getUpperBoundary()
				// Find out how many full value ranges val is above the upper boundary.
				// We use integer division here, so 13/4 would be 3.
				std::size_t nAboveUpperBoundary = (val - GConstrainedNumT<T>::getUpperBoundary() - 1) / value_range;

				// We are dealing with descending (nAboveUpperBoundary is even) and
				// ascending ranges (nAboveUpperBoundary is odd), which need to be treated differently
				if(nAboveUpperBoundary%2 == 0) { // nAboveUpperBoundary is even
					// Transfer into the allowed region
					mapping = val - (value_range * (nAboveUpperBoundary + 1));

					// Revert, as we are dealing with a descending value range
					mapping = revert(mapping);
				}
				else { // nAboveUpperBoundary is odd
					// Transfer into the allowed region
					mapping = val - (value_range * (nAboveUpperBoundary + 1));
				}
			}

			// Reset internal value -- possible because it is declared mutable in
			// GParameterT<T>. Resetting the internal value prevents divergence through
			// extensive mutation and also speeds up the previous part of the transfer
			// function
			GParameterT<T>::setValue_(mapping);

			return mapping;
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
		const GConstrainedIntegerT<T> *p_load = GObject::conversion_cast<GConstrainedIntegerT<T> >(cp);

		// Load our parent class'es data ...
		GConstrainedNumT<T>::load_(cp);

		// no local data
	}

	/****************************************************************************/
	/** @brief Create a deep copy of this object */
	virtual GObject *clone_() const = 0;

	/****************************************************************************/
	/**
	 * Randomly initializes the parameter (within its limits)
	 */
	virtual void randomInit_() {
		using namespace Gem::Hap;
		GRandomT<RANDOMLOCAL> gr;
		setValue(gr.uniform_int(GConstrainedNumT<T>::getLowerBoundary(), GConstrainedNumT<T>::getUpperBoundary()));
	}

private:
	/****************************************************************************/
	/**
	 * Reverts the value to descending order. Note: No check is made whether the value
	 * is indeed in the allowed region.
	 *
	 * @param value The value to be reverted
	 * @return The reverted value
	 */
	T revert(const T& value) const {
		T position = value - GConstrainedNumT<T>::getLowerBoundary();
		T reverted = GConstrainedNumT<T>::getUpperBoundary() - position;
		return reverted;
	}

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
		if(GConstrainedNumT<T>::modify_GUnitTests()) result = true;

		return result;
	}

	/****************************************************************************/
	/**
	 * Performs self tests that are expected to succeed. This is needed for testing purposes
	 */
	virtual void specificTestsNoFailureExpected_GUnitTests() {
		using namespace Gem::Hap;
		GRandomT<RANDOMLOCAL, double, T> gr;

		// Call the parent classes' functions
		GConstrainedNumT<T>::specificTestsNoFailureExpected_GUnitTests();

		// Clone the current object, so we can always recover from failures
		boost::shared_ptr<GConstrainedIntegerT<T> > p = this->GObject::clone<GConstrainedIntegerT<T> >();

		// Make sure we start with the maximum range
		p->resetBoundaries();

		const T minLower = -50;
		const T maxLower =  50;
		const T minUpper =  25; // Allow some overlap
		const T maxUpper = 125;

		// Check a number of times that the transfer function only returns items
		// in the allowed value range
		for(std::size_t i=0; i<100; i++) {
			T lowerBoundary = gr.uniform_int(minLower, maxLower);
			T upperBoundary;
			while((upperBoundary = gr.uniform_int(minUpper, maxUpper)) <= lowerBoundary);

			p->setValue(lowerBoundary);
			p->setBoundaries(lowerBoundary, upperBoundary);

			// Check that there are no values outside of the allowed range
			for(std::size_t j=0; j<1000; j++) {
				T probe = gr.uniform_int(-10000, 10000);
				T mapping = p->transfer(probe);
				BOOST_CHECK(mapping >= lowerBoundary && mapping <= upperBoundary);
			}

			// Make sure we start again with the maximum range
			p->resetBoundaries();
		}
	}

	/****************************************************************************/
	/**
	 * Performs self tests that are expected to fail. This is needed for testing purposes
	 */
	virtual void specificTestsFailuresExpected_GUnitTests() {
		// Call the parent classes' functions
		GConstrainedNumT<T>::specificTestsFailuresExpected_GUnitTests();
	}

#endif /* GENEVATESTING */
};

} /* namespace Geneva */
} /* namespace Gem */

/******************************************************************************/
// The content of BOOST_SERIALIZATION_ASSUME_ABSTRACT(T)

namespace boost {
	namespace serialization {
		template<typename T>
		struct is_abstract<Gem::Geneva::GConstrainedIntegerT<T> > : public boost::true_type {};
		template<typename T>
		struct is_abstract< const Gem::Geneva::GConstrainedIntegerT<T> > : public boost::true_type {};
	}
}

#endif /* GCONSTRAINEDINTEGERT_HPP_ */
