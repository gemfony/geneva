/**
 * @file GConstrainedNumCollectionT.hpp
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


// Standard header files go here
#include <string>
#include <sstream>
#include <vector>

// Includes check for correct Boost version(s)
#include "common/GGlobalDefines.hpp"

// Boost header files go here

#include <boost/cstdint.hpp>

#ifndef GCONSTRAINEDNUMCOLLECTIONT_HPP_
#define GCONSTRAINEDNUMCOLLECTIONT_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif


// Geneva header files go here
#include "common/GExceptions.hpp"
#include "GDoubleGaussAdaptor.hpp"
#include "GObject.hpp"
#include "GParameterCollectionT.hpp"

namespace Gem {
namespace Geneva {

/**********************************************************************/
/**
 * This class represents a collection of numeric values with common
 * boundaries, all modified using the same algorithm. The most likely types to
 * be stored in this class are double and boost::int32_t .
 *
 * TODO: Access to the data members of this class requires interaction with
 * the transfer function. Hence things like the iterator, the subscript operator []
 * or the at() function would need to be modified accordingly. This is not currently
 * implemented. This class is not usable in its current form.
 */
template <typename T>
class GConstrainedNumCollectionT
	: public GParameterCollectionT<T>
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int) {
		using boost::serialization::make_nvp;
		ar & make_nvp("GParameterCollectionT",	boost::serialization::base_object<GParameterCollectionT<T> >(*this))
		   & BOOST_SERIALIZATION_NVP(lowerBoundary_)
		   & BOOST_SERIALIZATION_NVP(upperBoundary_);
	}
	///////////////////////////////////////////////////////////////////////

public:
	/** @brief Specifies the type of parameters stored in this collection */
	typedef T collection_type;

	/******************************************************************/
	/**
	 * The default constructor.
	 */
	GConstrainedNumCollectionT()
		: GParameterCollectionT<T> ()
		, lowerBoundary_(T(0))
		, upperBoundary_(T(1))
	{ /* nothing */ }

	/******************************************************************/
	/**
	 * Initialize the lower and upper boundaries for data members of this class
	 *
	 * @param min The lower boundary for data members
	 * @param max The upper boundary for data members
	 */
	GConstrainedNumCollectionT(const T& min, const T& max)
		: GParameterCollectionT<T> ()
		, lowerBoundary_(T(0))
		, upperBoundary_(T(1))
	{ /* nothing */ }

	/******************************************************************/
	/**
	 * The standard copy constructor
	 */
	GConstrainedNumCollectionT(const GConstrainedNumCollectionT<T>& cp)
		: GParameterCollectionT<T> (cp)
	{ /* nothing */ }

	/******************************************************************/
	/**
	 * The standard destructor
	 */
	virtual ~GConstrainedNumCollectionT()
	{ /* nothing */ }

	/******************************************************************/
	/**
	 * The standard assignment operator.
	 *
	 * @param cp A copy of another GDoubleCollection object
	 * @return A constant reference to this object
	 */
	const GConstrainedNumCollectionT& operator=(const GConstrainedNumCollectionT<T>& cp){
		GConstrainedNumCollectionT<T>::load_(&cp);
		return *this;
	}

	/******************************************************************/
	/**
	 * Checks for equality with another GConstrainedNumCollectionT<T> object
	 *
	 * @param  cp A constant reference to another GConstrainedNumCollectionT<T> object
	 * @return A boolean indicating whether both objects are equal
	 */
	bool operator==(const GConstrainedNumCollectionT<T>& cp) const {
		using namespace Gem::Common;
		// Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
		return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GConstrainedNumCollectionT<T>::operator==","cp", CE_SILENT);
	}

	/******************************************************************/
	/**
	 * Checks for inequality with another GConstrainedNumCollectionT<T> object
	 *
	 * @param  cp A constant reference to another GConstrainedNumCollectionT<T> object
	 * @return A boolean indicating whether both objects are inequal
	 */
	bool operator!=(const GConstrainedNumCollectionT<T>& cp) const {
		using namespace Gem::Common;
		// Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
		return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GConstrainedNumCollectionT<T>::operator!=","cp", CE_SILENT);
	}

	/******************************************************************/
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
		const GConstrainedNumCollectionT<T>  *p_load = GObject::conversion_cast<GConstrainedNumCollectionT<T> >(&cp);

		// Will hold possible deviations from the expectation, including explanations
	    std::vector<boost::optional<std::string> > deviations;

		// Check our parent class'es data ...
		deviations.push_back(GParameterCollectionT<T>::checkRelationshipWith(cp, e, limit, "GConstrainedNumCollectionT<T>", y_name, withMessages));

		// ... and then our local data
		deviations.push_back(checkExpectation(withMessages, "GConstrainedNumCollectionT<T>", lowerBoundary_, p_load->lowerBoundary_, "lowerBoundary_", "p_load->lowerBoundary_", e , limit));
		deviations.push_back(checkExpectation(withMessages, "GConstrainedNumCollectionT<T>", upperBoundary_, p_load->upperBoundary_, "upperBoundary_", "p_load->upperBoundary_", e , limit));

		return evaluateDiscrepancies("GConstrainedNumCollectionT<T>", caller, deviations, e);
	}

	/////////////////////////////////////////////////////////////////////////////
	// The following was taken verbatim from GConstrainedNumT.hpp as a reminder
	// that "something similar" must be added.

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
			raiseException(
					"In GConstrainedNumT<T>::setBoundaries(const T&, const T&)" << std::endl
					<< "with typeid(T).name() = " << typeid(T).name() << " :" << std::endl
					<< "Lower and/or upper boundary has invalid value : " << lower << " " << upper
			);
		}

		// Check that the value is inside the allowed range
		if(currentValue < lower || currentValue > upper){
			raiseException(
					"In GConstrainedNumT<T>::setBoundaries(const T&, const T&) :" << std::endl
					<< "with typeid(T).name() = " << typeid(T).name() << std::endl
					<< "Attempt to set new boundaries [" << lower << ":" << upper << "]" << std::endl
					<< "with existing value  " << currentValue << " outside of this range."
			);

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
		if(val < lowerBoundary_ || val > upperBoundary_) {
			raiseException(
					"In GConstrainedNumT<T>::setValue(val):" << std::endl
					<< "Assigned value " << val << " is outside of its allowed boundaries: " << std::endl
					<< "lowerBoundary_ = " << lowerBoundary_ << std::endl
					<< "upperBoundary_ = " << upperBoundary_
			);
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
			raiseException(
					"In GConstrainedNumT<T>::setValue(val,lower,upper):" << std::endl
					<< "lowerBoundary_ = " << lowerBoundary_ << "is larger than" << std::endl
					<< "upperBoundary_ = " << upperBoundary_
			);
		}

		// O.k., assign the boundaries
		lowerBoundary_ = lowerBoundary;
		upperBoundary_ = upperBoundary;

		// Is the desired new value in the allowed range ?
		if(val < lowerBoundary_ || val > upperBoundary_) {
			raiseException(
					"In GConstrainedNumT<T>::setValue(val,lower,upper):" << std::endl
					<< "Assigned value " << val << " is outside of its allowed boundaries: " << std::endl
					<< "lowerBoundary_ = " << lowerBoundary_ << std::endl
					<< "upperBoundary_ = " << upperBoundary_
			);
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
		T mapping = transfer(GParameterT<T>::value());

		// Reset internal value -- possible because it is declared mutable in
		// GParameterT<T>. Resetting the internal value prevents divergence through
		// extensive mutation and also speeds up the previous part of the transfer
		// function
		GParameterT<T>::setValue_(mapping);

		return mapping;
	}

	/****************************************************************************/
	/**
	 * Retrieves GParameterT<T>'s internal value. Added here for compatibility
	 * reasons.
	 */
	T getInternalValue() const {
		return GParameterT<T>::value();
	}

	/****************************************************************************/
	/** @brief The transfer function needed to calculate the externally visible
	 * value. Declared public so we can do tests of the value transformation. */
	virtual T transfer(const T&) const = 0;

	// See above for an important remark!
	/////////////////////////////////////////////////////////////////////////////

protected:
	/******************************************************************/
	/**
	 * Loads the data of another GConstrainedNumCollectionT<T> object,
	 * camouflaged as a GObject. We have no local data, so
	 * all we need to do is to the standard identity check,
	 * preventing that an object is assigned to itself.
	 *
	 * @param cp A copy of another GConstrainedNumCollectionT<T> object, camouflaged as a GObject
	 */
	virtual void load_(const GObject *cp){
		// Convert cp into local format
		const GConstrainedNumCollectionT<T> *p_load = GObject::conversion_cast<GConstrainedNumCollectionT<T> >(cp);

		// Load our parent class'es data ...
		GParameterCollectionT<T>::load_(cp);

		// ... and then our local data
		lowerBoundary_ = p_load->lowerBoundary_;
		upperBoundary_ = p_load->upperBoundary_;
	}

	/******************************************************************/
	/**
	 * Creates a deep copy of this object. Purely virtual as this class
	 * should not be instantiable.
	 *
	 * @return A pointer to a deep clone of this object
	 */
	virtual GObject *clone_() const = 0;

	/******************************************************************/
	/** @brief Triggers random initialization of the parameter collection */
	virtual void randomInit_() = 0;

private:
	/******************************************************************/
	T lowerBoundary_; ///< The lower allowed boundary for our value
	T upperBoundary_; ///< The upper allowed boundary for our value

#ifdef GENEVATESTING
public:
	/******************************************************************/
	/**
	 * Applies modifications to this object. This is needed for testing purposes
	 *
	 * @return A boolean which indicates whether modifications were made
	 */
	virtual bool modify_GUnitTests() {
		bool result = false;

		// Call the parent classes' functions
		if(GParameterCollectionT<T>::modify_GUnitTests()) result = true;

		return result;
	}

	/******************************************************************/
	/**
	 * Performs self tests that are expected to succeed. This is needed for testing purposes
	 */
	virtual void specificTestsNoFailureExpected_GUnitTests() {
		// Call the parent classes' functions
		GParameterCollectionT<T>::specificTestsNoFailureExpected_GUnitTests();
	}

	/******************************************************************/
	/**
	 * Performs self tests that are expected to fail. This is needed for testing purposes
	 */
	virtual void specificTestsFailuresExpected_GUnitTests() {
		// Call the parent classes' functions
		GParameterCollectionT<T>::specificTestsFailuresExpected_GUnitTests();
	}

#endif /* GENEVATESTING */

};

/**********************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

/**********************************************************************/
// The content of BOOST_SERIALIZATION_ASSUME_ABSTRACT(T)

namespace boost {
	namespace serialization {
		template<typename T>
		struct is_abstract<Gem::Geneva::GConstrainedNumCollectionT<T> > : public boost::true_type {};
		template<typename T>
		struct is_abstract< const Gem::Geneva::GConstrainedNumCollectionT<T> > : public boost::true_type {};
	}
}
/**********************************************************************/

#endif /* GCONSTRAINEDNUMCOLLECTIONT_HPP_ */
