/**
 * @file GConstrainedFPT.hpp
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
#include <boost/utility/enable_if.hpp>
#include <boost/type_traits.hpp>

#ifndef GCONSTRAINEDFPT_HPP_
#define GCONSTRAINEDFPT_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif

// Geneva headers go here
#include "common/GExceptions.hpp"
#include "common/GMathHelperFunctions.hpp"
#include "hap/GRandomT.hpp"
#include "GConstrainedNumT.hpp"
#include "GObject.hpp"

namespace Gem
{
namespace Geneva
{

/******************************************************************************/
/* The GConstrainedFPT class represents an integer type, such as an int or a long,
 * equipped with the ability to adapt itself. The value range can have an upper and a lower
 * limit.  Adapted values will only appear inside the given range to the user. Note that
 * appropriate adaptors (see e.g the GInt32FlipAdaptor class) need to be loaded in order
 * to benefit from the adaption capabilities. Both boundaries are inclusive, i.e.
 * [lower:upper]. We currently only allow signed integers.
 */
template <typename T>
class GConstrainedFPT
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

public:
	/****************************************************************************/
	/**
	 * The default constructor
	 */
	GConstrainedFPT()
		: GConstrainedNumT<T>()
    { /* nothing */ }

	/****************************************************************************/
	/**
	 * A constructor that initializes the value only. The boundaries will
	 * be set to the maximum and minimum values of the corresponding type.
	 *
	 * @param val The desired external value of this object
	 */
	explicit GConstrainedFPT (const T& val)
		: GConstrainedNumT<T>(val)
	{ /* nothing */	}

	/****************************************************************************/
	/**
	 * Initializes the boundaries and sets the value to the lower boundary.
	 *
	 * @param lowerBoundary The lower boundary of the value range
	 * @param upperBoundary The upper boundary of the value range
	 */
	GConstrainedFPT (
			const T& lowerBoundary
		  , const T& upperBoundary
	)
		: GConstrainedNumT<T>(lowerBoundary, upperBoundary)
	{ /* nothing */	}

	/****************************************************************************/
	/**
	 * Initialization with value and boundaries. We need somewhat tighter
	 * constraints for the allowed value range than implemented in the
	 * parent class.
	 *
	 * @param val The desired value of this object
	 * @param lowerBoundary The lower boundary of the value range
	 * @param upperBoundary The upper boundary of the value range
	 */
	GConstrainedFPT (
			const T& val
		  , const T& lowerBoundary
		  , const T& upperBoundary
	)
		: GConstrainedNumT<T>(val, lowerBoundary, upperBoundary)
	{
		// The upper boundary is not included for floating point values
		if(val >= upperBoundary) {
			std::ostringstream error;
			error << "In GConstrainedFPT<T>::GConstrainedFPT(val,lower,upper): Error!" << std::endl
				  << "Assigned value " << val << " is outside of its allowed boundaries: " << std::endl
				  << "lowerBoundary = " << lowerBoundary << std::endl
				  << "upperBoundary = " << upperBoundary << std::endl;
			throw(Gem::Common::gemfony_error_condition(error.str()));
		}
	}

	/****************************************************************************/
	/**
	 * A standard copy constructor. Most work is done by the parent
	 * classes, we only need to copy the allowed value range.
	 *
	 * @param cp Another GConstrainedNumT<T> object
	 */
	GConstrainedFPT (const GConstrainedFPT<T>& cp)
		: GConstrainedNumT<T>(cp)
	{ /* nothing */ }

	/****************************************************************************/
	/**
	 * The standard destructor
	 */
	virtual ~GConstrainedFPT()
	{ /* nothing */	}

	/****************************************************************************/
	/**
	 * A standard assignment operator for GConstrainedFPT<T> objects
	 *
	 * @param cp A constant reference to another GConstrainedFPT<T> object
	 * @return A constant reference to this object
	 */
	const GConstrainedFPT<T>& operator= (const GConstrainedFPT<T>& cp) {
		GConstrainedFPT<T>::load_(&cp);
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
		// The upper boundary is not included for fp values. Hence
		// we need to enforce a more stringent check than is used
		// in GConstrainedNumT.
		if(val >= GConstrainedNumT<T>::getUpperBoundary()) {
			std::ostringstream error;
			error << "In GConstrainedFPT<T>::operator=(val): Error!" << std::endl
				  << "Assigned value " << val << " is outside of its allowed boundaries: " << std::endl
				  << "lowerBoundary = " << GConstrainedNumT<T>::getLowerBoundary() << std::endl
				  << "upperBoundary = " << GConstrainedNumT<T>::getUpperBoundary() << std::endl;
			throw(Gem::Common::gemfony_error_condition(error.str()));
		}

		return GConstrainedNumT<T>::operator=(val);
	}

	/****************************************************************************/
    /**
     * Checks equality of this object with another.
     *
     * @param cp A constant reference to another GConstrainedFPT<T> object
     * @return A boolean indicating whether both objects are equal
     */
	bool operator==(const GConstrainedFPT<T>& cp) const {
		using namespace Gem::Common;
		// Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
		return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GConstrainedFPT<T>::operator==","cp", CE_SILENT);
	}

	/****************************************************************************/
    /**
     * Checks inequality of this object with another.
     *
     * @param cp A constant reference to another GConstrainedFPT<T> object
     * @return A boolean indicating whether both objects are inequal
     */
	bool operator!= (const GConstrainedFPT<T>& cp) const {
		using namespace Gem::Common;
		// Means: The expectation of inequality was fulfilled, as no error text was emitted (which converts to "true")
		return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GConstrainedFPT<T>::operator!=","cp", CE_SILENT);
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
	boost::optional<std::string> checkRelationshipWith (
			const GObject& cp
		  , const Gem::Common::expectation& e
		  , const double& limit
		  , const std::string& caller
		  , const std::string& y_name
		  , const bool& withMessages
	) const	{
	    using namespace Gem::Common;

		// Check that we are indeed dealing with a GParamterBase reference
		const GConstrainedFPT<T>  *p_load = GObject::conversion_cast<GConstrainedFPT<T> >(&cp);

		// Will hold possible deviations from the expectation, including explanations
	    std::vector<boost::optional<std::string> > deviations;

		// Check our parent class'es data ...
		deviations.push_back(GConstrainedNumT<T>::checkRelationshipWith(cp, e, limit, "GConstrainedFPT<T>", y_name, withMessages));

		// ... no local data

		return evaluateDiscrepancies("GConstrainedFPT<T>", caller, deviations, e);
	}

	/****************************************************************************/
	/**
	 * Sets the boundaries of this object and does corresponding
	 * error checks. This is an overloaded function from the parent
	 * class, which enforces a more stringent check, as the upper
	 * boundary is not included for fp values.
	 *
	 * @param lower The new lower boundary for this object
	 * @param upper The new upper boundary for this object
	 */
	virtual void setBoundaries(const T& lower, const T& upper) {
		const T currentValue = GParameterT<T>::value();

		// Check that the value is inside the allowed range
		if(currentValue < lower || currentValue >= upper){
			std::ostringstream error;
			error << "In GConstrainedFPT<T>::setBoundaries(const T&, const T&) : Error!" << std::endl
				  << "with typeid(T).name() = " << typeid(T).name() << std::endl
				  << "Attempt to set new boundaries [" << lower << ":" << upper << "]" << std::endl
				  << "with existing value  " << currentValue << " outside of this range." << std::endl;

			// throw an exception.
			throw Gem::Common::gemfony_error_condition(error.str());
		}

		GConstrainedNumT<T>::setBoundaries(lower, upper);
	}

	/****************************************************************************/
	/**
	 * Allows to set the value. Sets the boundaries of this object and does corresponding
	 * error checks. This is an overloaded function from the parent
	 * class, which enforces a more stringent check, as the upper
	 * boundary is not included for fp values.
	 *
	 * @param val The new T value stored in this class
	 */
	virtual void setValue(const T& val)  {
		// Do some error checking
		if(val < GConstrainedNumT<T>::getLowerBoundary() || val >= GConstrainedNumT<T>::getUpperBoundary()) {
			std::ostringstream error;
			error << "In GConstrainedFPT<T>::setValue(val): Error!" << std::endl
				  << "Assigned value " << val << " is outside of its allowed boundaries: " << std::endl
				  << "lowerBoundary = " << GConstrainedNumT<T>::getLowerBoundary() << std::endl
				  << "upperBoundary = " << GConstrainedNumT<T>::getUpperBoundary() << std::endl;
			throw(Gem::Common::gemfony_error_condition(error.str()));
		}

		// O.k., set the value
		GConstrainedNumT<T>::setValue(val);
	}

	/****************************************************************************/
	/**
	 * Allows to set the value of this object together with its boundaries.
	 * Sets the boundaries of this object and does corresponding error checks.
	 * This is an overloaded function from the parent class, which enforces a
	 * more stringent check, as the upper boundary is not included for fp values.
	 *
	 * @param val The desired value of this object
	 * @param lowerBoundary The lower boundary of the value range
	 * @param upperBoundary The upper boundary of the value range
	 */
	virtual void setValue(const T& val, const T& lowerBoundary, const T& upperBoundary) {
		// Is the desired new value in the allowed range ?
		if(val < lowerBoundary || val >= upperBoundary) {
			std::ostringstream error;
			error << "In GConstrainedFPT<T>::setValue(val,lower,upper): Error!" << std::endl
				  << "Assigned value " << val << " is outside of its allowed boundaries: " << std::endl
				  << "lowerBoundary = " << lowerBoundary << std::endl
				  << "upperBoundary = " << upperBoundary << std::endl;
			throw(Gem::Common::gemfony_error_condition(error.str()));
		}

		// O.k., set the value
		GConstrainedNumT<T>::setValue(val);
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

		if(val >= GConstrainedNumT<T>::getLowerBoundary() && val < GConstrainedNumT<T>::getUpperBoundary()) {
			return val;
		}
		else {
			// Find out which region the value is in (compare figure transferFunction.pdf
			// that should have been delivered with this software). Note that boost::numeric_cast<>
			// may throw - exceptions must be caught in surrounding functions.
			boost::int32_t region = 0.;
			T lowerBoundary = GConstrainedNumT<T>::getLowerBoundary();
			T upperBoundary = GConstrainedNumT<T>::getUpperBoundary();

#ifdef DEBUG
			region =	boost::numeric_cast<boost::int32_t>(Gem::Common::GFloor((T(val) - T(lowerBoundary)) / (T(upperBoundary) - T(lowerBoundary))));
#else
			region =	static_cast<boost::int32_t>(Gem::Common::GFloor((T(val) - T(lowerBoundary)) / (T(upperBoundary) - T(lowerBoundary))));
#endif

			// Check whether we are in an odd or an even range and calculate the
			// external value accordingly
			T mapping = T(0.);
			if(region%2 == 0) { // can it be divided by 2 ? Region 0,2,... or a negative even range
				mapping = val - T(region) * (GConstrainedNumT<T>::getUpperBoundary() - GConstrainedNumT<T>::getLowerBoundary());
			} else { // Range 1,3,... or a negative odd range
				mapping = -val + (T(region-1)*(GConstrainedNumT<T>::getUpperBoundary() - GConstrainedNumT<T>::getLowerBoundary()) + 2*GConstrainedNumT<T>::getUpperBoundary());
			}

			return mapping;
		}

		// Make the compiler happy
		return T(0.);
	}

	/****************************************************************************/
	/**
	 * Initializes double-based parameters with a given value. Allows e.g. to set all
	 * floating point parameters to 0. Note that, contrary to the usual behavior,
	 * we accept initialization outside of the allowed boundaries. However, the internal
	 * representation will then be transferred back to an external value in the allowed
	 * value range.
	 *
	 * @param val The value to be assigned to the parameters
	 */
	virtual void fpFixedValueInit(const float& val)
	{
		GParameterT<T>::setValue(transfer(T(val)));
	}

	/****************************************************************************/
	/**
	 * Multiplies double-based parameters with a given value. Note that the resulting
	 * internal value may well be outside of the allowed boundaries. However, the internal
	 * representation will then be transferred back to an external value in the allowed
	 * value range.
	 */
	virtual void fpMultiplyBy(const float& val) {
		GParameterT<T>::setValue(transfer(T(val) * GParameterT<T>::value()));
	}

	/****************************************************************************/
	/**
	 * Multiplies with a random floating point number in a given range.  Note that the resulting
	 * internal value may well be outside of the allowed boundaries. However, the internal
	 * representation will then be transferred back to an external value in the allowed
	 * value range.
	 *
	 * @param min The lower boundary for random number generation
	 * @param max The upper boundary for random number generation
	 */
	virtual void fpMultiplyByRandom(const float& min, const float& max)	{
		using namespace Gem::Hap;
		GRandomT<RANDOMLOCAL, T, boost::int32_t> gr;
		GParameterT<T>::setValue(transfer(GParameterT<T>::value() * gr.uniform_real(T(min), T(max))));
	}

	/****************************************************************************/
	/**
	 * Multiplies with a random floating point number in the range [0, 1[.  Note that the resulting
	 * internal value may well be outside of the allowed boundaries. However, the internal
	 * representation will then be transferred back to an external value in the allowed
	 * value range.
	 */
	virtual void fpMultiplyByRandom() {
		using namespace Gem::Hap;
		GRandomT<RANDOMLOCAL, T, boost::int32_t> gr;
		GParameterT<T>::setValue(transfer(GParameterT<T>::value() * gr.uniform_01()));
	}

	/****************************************************************************/
	/**
	 * Adds the floating point parameters of another GParameterBase object to this one.
	 * Note that the resulting internal value may well be outside of the allowed boundaries.
	 * However, the internal representation will then be transferred back to an external
	 * value in the allowed value range.
	 *
	 * @oaram p A boost::shared_ptr to another GParameterBase object
	 */
	virtual void fpAdd(boost::shared_ptr<GParameterBase> p_base) {
		// We first need to convert p_base into the local type
		boost::shared_ptr<GConstrainedFPT<T> > p = GParameterBase::parameterbase_cast<GConstrainedFPT<T> >(p_base);
		GParameterT<T>::setValue(transfer(GParameterT<T>::value() + p->value()));
	}

	/****************************************************************************/
	/**
	 * Subtracts the floating point parameters of another GParameterBase object
	 * from this one. Note that the resulting internal value may well be outside of
	 * the allowed boundaries. However, the internal representation will then be
	 * transferred back to an external value in the allowed value range.
	 *
	 * @oaram p A boost::shared_ptr to another GParameterBase object
	 */
	virtual void fpSubtract(boost::shared_ptr<GParameterBase> p_base) {
		// We first need to convert p_base into the local type
		boost::shared_ptr<GConstrainedFPT<T> > p = GParameterBase::parameterbase_cast<GConstrainedFPT<T> >(p_base);
		GParameterT<T>::setValue(transfer(GParameterT<T>::value() - p->value()));
	}

protected:
	/****************************************************************************/
	/**
	 * Loads the data of another GConstrainedFPT<T>, camouflaged as a GObject.
	 *
	 * @param cp Another GConstrainedFPT<T> object, camouflaged as a GObject
	 */
	virtual void load_(const GObject *cp) {
		// Convert GObject pointer to local format
		const GConstrainedFPT<T> *p_load = GObject::conversion_cast<GConstrainedFPT<T> >(cp);

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
		GRandomT<RANDOMLOCAL, T, boost::int32_t> gr;
		setValue(gr.uniform_real(GConstrainedNumT<T>::getLowerBoundary(), GConstrainedNumT<T>::getUpperBoundary()));
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
		bool result = false;

		// Call the parent classes' functions
		if(GConstrainedNumT<T>::modify_GUnitTests()) result = true;

		return result;
	}

	/****************************************************************************/
	/**
	 * Performs self tests that are expected to succeed. This is needed for testing purposes
	 */
	virtual void specificTestsNoFailureExpected_GUnitTests() {
		// Call the parent classes' functions
		GConstrainedNumT<T>::specificTestsNoFailureExpected_GUnitTests();
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
		struct is_abstract<Gem::Geneva::GConstrainedFPT<T> > : public boost::true_type {};
		template<typename T>
		struct is_abstract< const Gem::Geneva::GConstrainedFPT<T> > : public boost::true_type {};
	}
}

#endif /* GCONSTRAINEDFPT_HPP_ */
