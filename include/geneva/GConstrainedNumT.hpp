/**
 * @file GConstrainedNumT.hpp
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

#ifndef GCONSTRAINEDNUMT_HPP_
#define GCONSTRAINEDNUMT_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif

// Geneva headers go here
#include "common/GExceptions.hpp"
#include "hap/GRandomT.hpp"
#include "GObject.hpp"
#include "GParameterT.hpp"

namespace Gem
{
namespace Geneva
{

/******************************************************************************/
/* The GConstrainedNumT class represents a numeric value, such as an int or a double,
 * equipped with the ability to adapt itself. The value range can have an upper and a lower
 * limit.  Adapted values will only appear inside the given range to the user, while they are
 * internally represented as a continuous range of values. Note that appropriate adaptors
 * (see e.g the GDoubleGaussAdaptor class) need to be loaded in order to benefit from the
 * adaption capabilities. It depends on the implementation of derived classes whether boundaries
 * are inclusive or exclusive.
 */
template <typename T>
class GConstrainedNumT
:public GParameterT<T>
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int) {
		using boost::serialization::make_nvp;

		// Save data
		ar & make_nvp("GParameterT_T", boost::serialization::base_object<GParameterT<T> >(*this))
		   & BOOST_SERIALIZATION_NVP(lowerBoundary_)
		   & BOOST_SERIALIZATION_NVP(upperBoundary_);
	}
	///////////////////////////////////////////////////////////////////////

public:
	/****************************************************************************/
	/**
	 * The default constructor
	 */
	GConstrainedNumT()
		: GParameterT<T>(T(0))
		, lowerBoundary_(T(0))
		, upperBoundary_(T(1))
    { /* nothing */ }


	/****************************************************************************/
	/**
	 * A constructor that initializes the value only. The boundaries will
	 * be set to the maximum and minimum values of the corresponding type.
	 *
	 * @param val The desired external value of this object
	 */
	explicit GConstrainedNumT(const T& val)
		: GParameterT<T>(val)
		, lowerBoundary_(-std::numeric_limits<T>::max())
		, upperBoundary_( std::numeric_limits<T>::max())
	{ /* nothing */	}

	/****************************************************************************/
	/**
	 * Initializes the boundaries and sets the value to the lower boundary.
	 *
	 * @param lowerBoundary The lower boundary of the value range
	 * @param upperBoundary The upper boundary of the value range
	 */
	GConstrainedNumT(const T& lowerBoundary, const T& upperBoundary)
		: GParameterT<T>(lowerBoundary)
		, lowerBoundary_(lowerBoundary)
		, upperBoundary_(upperBoundary)
	{ /* nothing */ }

	/****************************************************************************/
	/**
	 * Initialization with value and boundaries.
	 *
	 * @param val The desired value of this object
	 * @param lowerBoundary The lower boundary of the value range
	 * @param upperBoundary The upper boundary of the value range
	 */
	GConstrainedNumT(const T& val, const T& lowerBoundary, const T& upperBoundary)
		: GParameterT<T>(val)
		, lowerBoundary_(lowerBoundary)
		, upperBoundary_(upperBoundary)
	{
		// Do some error checking
		if(lowerBoundary_ > upperBoundary_) {
			std::ostringstream error;
			error << "In GConstrainedNumT<T>::GConstrainedNumT(val,lower,upper): Error!" << std::endl
				  << "lowerBoundary_ = " << lowerBoundary_ << "is larger than" << std::endl
				  << "upperBoundary_ = " << upperBoundary_ << std::endl;
			throw(Gem::Common::gemfony_error_condition(error.str()));
		}

		if(!valIsCompatibleWithLower(val, lowerBoundary_) || !valIsCompatibleWithUpper(val, upperBoundary_)) {
			std::ostringstream error;
			error << "In GConstrainedNumT<T>::GConstrainedNumT(val,lower,upper): Error!" << std::endl
				  << "Assigned value " << val << " is outside of its allowed boundaries: " << std::endl
				  << "lowerBoundary_ = " << lowerBoundary_ << std::endl
				  << "upperBoundary_ = " << upperBoundary_ << std::endl;
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
	GConstrainedNumT(const GConstrainedNumT<T>& cp)
		: GParameterT<T>(cp)
		, lowerBoundary_(cp.lowerBoundary_)
		, upperBoundary_(cp.upperBoundary_)
	{ /* nothing */ }

	/****************************************************************************/
	/**
	 * The standard destructor
	 */
	virtual ~GConstrainedNumT()
	{ /* nothing */	}

	/****************************************************************************/
	/**
	 * A standard assignment operator for GConstrainedNumT<T> objects
	 *
	 * @param cp A constant reference to another GConstrainedNumT<T> object
	 * @return A constant reference to this object
	 */
	const GConstrainedNumT<T>& operator=(const GConstrainedNumT<T>& cp) {
		GConstrainedNumT<T>::load_(&cp);
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
		GConstrainedNumT<T>::setValue(val);
		return val;
	}

	/****************************************************************************/
    /**
     * Checks equality of this object with another.
     *
     * @param cp A constant reference to another GConstrainedNumT<T> object
     * @return A boolean indicating whether both objects are equal
     */
	bool operator==(const GConstrainedNumT<T>& cp) const {
		using namespace Gem::Common;
		// Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
		return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GConstrainedNumT<T>::operator==","cp", CE_SILENT);
	}

	/****************************************************************************/
    /**
     * Checks inequality of this object with another.
     *
     * @param cp A constant reference to another GConstrainedNumT<T> object
     * @return A boolean indicating whether both objects are inequal
     */
	bool operator!=(const GConstrainedNumT<T>& cp) const {
		using namespace Gem::Common;
		// Means: The expectation of inequality was fulfilled, as no error text was emitted (which converts to "true")
		return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GConstrainedNumT<T>::operator!=","cp", CE_SILENT);
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
	boost::optional<std::string> checkRelationshipWith(
			const GObject& cp,
			const Gem::Common::expectation& e,
			const double& limit,
			const std::string& caller,
			const std::string& y_name,
			const bool& withMessages
	) const	{
	    using namespace Gem::Common;

		// Check that we are indeed dealing with a GParamterBase reference
		const GConstrainedNumT<T>  *p_load = GObject::conversion_cast<GConstrainedNumT<T> >(&cp);

		// Will hold possible deviations from the expectation, including explanations
	    std::vector<boost::optional<std::string> > deviations;

		// Check our parent class'es data ...
		deviations.push_back(GParameterT<T>::checkRelationshipWith(cp, e, limit, "GConstrainedNumT<T>", y_name, withMessages));

		// ... and then our local data
		deviations.push_back(checkExpectation(withMessages, "GConstrainedNumT<T>", lowerBoundary_, p_load->lowerBoundary_, "lowerBoundary_", "p_load->lowerBoundary_", e , limit));
		deviations.push_back(checkExpectation(withMessages, "GConstrainedNumT<T>", upperBoundary_, p_load->upperBoundary_, "upperBoundary_", "p_load->upperBoundary_", e , limit));

		return evaluateDiscrepancies("GConstrainedNumT<T>", caller, deviations, e);
	}

	/****************************************************************************/
    /**
     * Retrieves the lower boundary
     *
     * @return The value of the lower boundary
     */
	T getLowerBoundary() const {
    	return lowerBoundary_;
	}

	/****************************************************************************/
    /**
     * Retrieves the upper boundary
     *
     * @return The value of the upper boundary
     */
	T getUpperBoundary() const {
    	return upperBoundary_;
	}

	/****************************************************************************/
	/**
	 * Resets the boundaries to the maximum allowed value.
	 */
	void resetBoundaries() {
		this->setBoundaries(-std::numeric_limits<T>::max(), std::numeric_limits<T>::max());
	}

	/****************************************************************************/
	/**
	 * Sets the boundaries of this object and does corresponding
	 * error checks. If the current value is below or above the new
	 * boundaries, this function will throw. Set the external value to
	 * a new value between the new boundaries before calling this
	 * function, or use the corresponding "setValue()" overload, which
	 * also allows setting of boundaries.
	 *
	 * @param lower The new lower boundary for this object
	 * @param upper The new upper boundary for this object
	 */
	virtual void setBoundaries(const T& lower, const T& upper) {
		const T currentValue = GParameterT<T>::value();

		// Check that the boundaries make sense
		if(lower > upper) {
			std::ostringstream error;
			error << "In GConstrainedNumT<T>::setBoundaries(const T&, const T&)" << std::endl
				     << "with typeid(T).name() = " << typeid(T).name() << " : Error" << std::endl
				     << "Lower and/or upper boundary has invalid value : " << lower << " " << upper << std::endl;

			throw(Gem::Common::gemfony_error_condition(error.str()));
		}

		// Check that the value is inside the allowed range
		if(!valIsCompatibleWithLower(currentValue, lower) || !valIsCompatibleWithUpper(currentValue, upper)){
			std::ostringstream error;
			error << "In GConstrainedNumT<T>::setBoundaries(const T&, const T&) : Error!" << std::endl
				  << "with typeid(T).name() = " << typeid(T).name() << std::endl
				  << "Attempt to set new boundaries [" << lower << ":" << upper << "]" << std::endl
				  << "with existing value  " << currentValue << " outside of this range." << std::endl;

			// throw an exception. Add some information so that if the exception
			// is caught through a base object, no information is lost.
			throw Gem::Common::gemfony_error_condition(error.str());
		}

		lowerBoundary_ = lower;
		upperBoundary_ = upper;

		// Re-set the internal representation of the value -- we might be in a different
		// region of the transformation internally, and the mapping will likely depend on
		// the boundaries.
		GParameterT<T>::setValue(currentValue);
	}

	/****************************************************************************/
	/**
	 * Allows to set the value. This function will throw if val is not in the currently
	 * assigned value range. Use the corresponding overload if you want to set the value
	 * together with its boundaries instead.
	 *
	 * @param val The new T value stored in this class
	 */
	virtual void setValue(const T& val)  {
		// Do some error checking
		if(!valIsCompatibleWithLower(val, lowerBoundary_) || !valIsCompatibleWithUpper(val, upperBoundary_)) {
			std::ostringstream error;
			error << "In GConstrainedNumT<T>::GConstrainedNumT(val,lower,upper): Error!" << std::endl
				  << "Assigned value " << val << " is outside of its allowed boundaries: " << std::endl
				  << "lowerBoundary_ = " << lowerBoundary_ << std::endl
				  << "upperBoundary_ = " << upperBoundary_ << std::endl;
			throw(Gem::Common::gemfony_error_condition(error.str()));
		}

		// O.k., assign value
		GParameterT<T>::setValue(val);
	}

	/****************************************************************************/
	/**
	 * Allows to set the value of this object together with its boundaries.
	 *
	 * @param val The desired value of this object
	 * @param lowerBoundary The lower boundary of the value range
	 * @param upperBoundary The upper boundary of the value range
	 */
	virtual void setValue(const T& val, const T& lowerBoundary, const T& upperBoundary) {
		// Do some error checking

		// Do the boundaries make sense ?
		if(lowerBoundary > upperBoundary) {
			std::ostringstream error;
			error << "In GConstrainedNumT<T>::setValue(val,lower,upper): Error!" << std::endl
				  << "lowerBoundary_ = " << lowerBoundary_ << "is larger than" << std::endl
				  << "upperBoundary_ = " << upperBoundary_ << std::endl;
			throw(Gem::Common::gemfony_error_condition(error.str()));
		}

		// O.k., assign the boundaries
		lowerBoundary_ = lowerBoundary;
		upperBoundary_ = upperBoundary;

		// Is the desired new value in the allowed range ?
		if(!valIsCompatibleWithLower(val, lowerBoundary_) || !valIsCompatibleWithUpper(val, upperBoundary_)) {
			std::ostringstream error;
			error << "In GConstrainedNumT<T>::setValue(val,lower,upper): Error!" << std::endl
				  << "Assigned value " << val << " is outside of its allowed boundaries: " << std::endl
				  << "lowerBoundary_ = " << lowerBoundary_ << std::endl
				  << "upperBoundary_ = " << upperBoundary_ << std::endl;
			throw(Gem::Common::gemfony_error_condition(error.str()));
		}

		// O.k., assign value
		GParameterT<T>::setValue(val);
	}

	/****************************************************************************/
	/**
	 * Retrieval of the value. This is an overloaded version of the original
	 * GParameterT<T>::value() function which applies a transformation, to be
	 * defined in derived classes.
	 *
	 * @return The transformed value of val_
	 */
	virtual T value() const {
		return this->transfer(GParameterT<T>::value());
	}

	/****************************************************************************/
	/**
	 * Retrieves GParameterT<T>'s internal value. Added here for compatibility
	 * with GBoundedNumT<T>.
	 */
	T getInternalValue() const {
		return GParameterT<T>::value();
	}

	/****************************************************************************/
	/** @brief The transfer function needed to calculate the externally visible
	 * value. Declared public so we can do tests of the value transformation. */
	virtual T transfer(const T&) const = 0;

protected:
	/****************************************************************************/
	/**
	 * Loads the data of another GConstrainedNumT<T>, camouflaged as a GObject.
	 *
	 * @param cp Another GConstrainedNumT<T> object, camouflaged as a GObject
	 */
	virtual void load_(const GObject *cp) {
		// Convert GObject pointer to local format
		const GConstrainedNumT<T> *p_load	= GObject::conversion_cast<GConstrainedNumT<T> >(cp);

		// Load our parent class'es data ...
		GParameterT<T>::load_(cp);

		// ... and then our own
		lowerBoundary_ = p_load->lowerBoundary_;
		upperBoundary_ = p_load->upperBoundary_;
	}

	/****************************************************************************/
	/**
	 * Checks whether a given value is compatible with the lower boundary.
	 *
	 * @param value The value to check against the lower boundary
	 * @return A boolean which indicates whether the value is compatible with the lower boundary
	 */
	virtual bool valIsCompatibleWithLower(const T& value, const T& lowerBoundary) const {
		return (value >= lowerBoundary);
	}

	/****************************************************************************/
	/**
	 * Checks whether a given value is compatible with the upper boundary. E.g.
	 * for floating-point values it may be useful to use < instead of <= for the
	 * comparison.
	 *
	 * @param value The value to check against the upper boundary
	 * @return A boolean which indicates whether the value is compatible with the upper boundary
	 */
	virtual bool valIsCompatibleWithUpper(const T& value, const T& upperBoundary) const {
		return (value <= upperBoundary);
	}

	/****************************************************************************/
	/** @brief Create a deep copy of this object. */
	virtual GObject *clone_() const = 0;
	/** @brief Randomly initializes the parameter (within its limits) */
	virtual void randomInit_() = 0;

private:
	/****************************************************************************/
	T lowerBoundary_; ///< The lower allowed boundary for our value
	T upperBoundary_; ///< The upper allowed boundary for our value


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

} /* namespace Geneva */
} /* namespace Gem */

/******************************************************************************/
// The content of BOOST_SERIALIZATION_ASSUME_ABSTRACT(T)

namespace boost {
	namespace serialization {
		template<typename T>
		struct is_abstract<Gem::Geneva::GConstrainedNumT<T> > : public boost::true_type {};
		template<typename T>
		struct is_abstract< const Gem::Geneva::GConstrainedNumT<T> > : public boost::true_type {};
	}
}

/******************************************************************************/

#endif /* GCONSTRAINEDNUMT_HPP_ */
