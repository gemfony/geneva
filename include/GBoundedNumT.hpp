/**
 * @file GBoundedNumT.hpp
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

#ifndef GBOUNDEDNUMT_HPP_
#define GBOUNDEDNUMT_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif

// GenEvA headers go here
#include "GObject.hpp"
#include "GParameterBaseWithAdaptorsT.hpp"
#include "GParameterT.hpp"
#include "GenevaExceptions.hpp"

namespace Gem
{
namespace GenEvA
{

/******************************************************************************/
/* The GBoundedNumT class represents a numeric value, such as an int or a double,
 * equipped with the ability to mutate itself. The value range can have an upper and a lower
 * limit.  Mutated values will only appear inside the given range to the user, while they are
 * internally represented as a continuous range of values. Note that appropriate adaptors
 * (see e.g the GDoubleGaussAdaptor class) need to be loaded in order to benefit from the
 * mutation capabilities.
 */
template <typename T>
class GBoundedNumT
:public GParameterT<T>
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int){
		using boost::serialization::make_nvp;

		// Save data
		ar & make_nvp("GParameterT_T", boost::serialization::base_object<GParameterT<T> >(*this));
		ar & BOOST_SERIALIZATION_NVP(internalValue_)
		   & BOOST_SERIALIZATION_NVP(lowerBoundary_)
		   & BOOST_SERIALIZATION_NVP(upperBoundary_);
	}
	///////////////////////////////////////////////////////////////////////

public:
	/** @brief The default constructor */
	GBoundedNumT();
	/** @brief Initialization with value only */
	explicit GBoundedNumT(const T& val);
	/** @brief Initialization with boundaries only */
	GBoundedNumT(const T&, const T&);
	/** @brief Initialization with value and boundaries */
	GBoundedNumT(const T&, const T&, const T&);
	/** @brief A standard copy constructor */
	GBoundedNumT(const GBoundedNumT<T>&);
	/** @brief The standard destructor */
	virtual ~GBoundedNumT();

	/****************************************************************************/
	/**
	 * A standard assignment operator for GBoundedNumT<T> objects
	 *
	 * @param cp A constant reference to another GBoundedNumT<T> object
	 * @return A constant reference to this object
	 */
	const GBoundedNumT<T>& operator=(const GBoundedNumT<T>& cp) {
		GBoundedNumT<T>::load(&cp);
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
	virtual const T& operator=(const T& val) {
		this->setExternalValue(val);
		return val;
	}

	/****************************************************************************/
    /**
     * Checks equality of this object with another.
     *
     * @param cp A constant reference to another GBoundedNumT<T> object
     * @return A boolean indicating whether both objects are equal
     */
	bool operator==(const GBoundedNumT<T>& cp) const {
		using namespace Gem::Util;
		// Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
		return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GBoundedNumT<T>::operator==","cp", CE_SILENT);
	}

	/****************************************************************************/
    /**
     * Checks inequality of this object with another.
     *
     * @param cp A constant reference to another GBoundedNumT<T> object
     * @return A boolean indicating whether both objects are inequal
     */
	bool operator!=(const GBoundedNumT<T>& cp) const {
		using namespace Gem::Util;
		// Means: The expectation of inequality was fulfilled, as no error text was emitted (which converts to "true")
		return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GBoundedNumT<T>::operator!=","cp", CE_SILENT);
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
			const Gem::Util::expectation& e,
			const double& limit,
			const std::string& caller,
			const std::string& y_name,
			const bool& withMessages) const
	{
	    using namespace Gem::Util;
	    using namespace Gem::Util::POD;

		// Check that we are indeed dealing with a GParamterBase reference
		const GBoundedNumT<T>  *p_load = GObject::conversion_cast(&cp,  this);

		// Will hold possible deviations from the expectation, including explanations
	    std::vector<boost::optional<std::string> > deviations;

		// Check our parent class'es data ...
		deviations.push_back(GParameterT<T>::checkRelationshipWith(cp, e, limit, "GBoundedNumT<T>", y_name, withMessages));

		// ... and then our local data
		deviations.push_back(checkExpectation(withMessages, "GBoundedNumT<T>", lowerBoundary_, p_load->lowerBoundary_, "lowerBoundary_", "p_load->lowerBoundary_", e , limit));
		deviations.push_back(checkExpectation(withMessages, "GBoundedNumT<T>", upperBoundary_, p_load->upperBoundary_, "upperBoundary_", "p_load->upperBoundary_", e , limit));
		deviations.push_back(checkExpectation(withMessages, "GBoundedNumT<T>", internalValue_, p_load->internalValue_, "internalValue_", "p_load->internalValue_", e , limit));

		return evaluateDiscrepancies("GBoundedNumT<T>", caller, deviations, e);
	}

	/****************************************************************************/
	/**
	 * Loads the data of another GBoundedNumT<T>, camouflaged as a GObject.
	 *
	 * @param cp Another GBoundedNumT<T> object, camouflaged as a GObject
	 */
	virtual void load(const GObject *cp) {
		// Convert GObject pointer to local format
		const GBoundedNumT<T> *p_load	= this->conversion_cast(cp, this);

		// Load our parent class'es data ...
		GParameterT<T>::load(cp);

		// ... and then our own
		lowerBoundary_ = p_load->lowerBoundary_;
		upperBoundary_ = p_load->upperBoundary_;
		internalValue_ = p_load->internalValue_;
	}

	/****************************************************************************/
    /**
     * Retrieves the lower boundary
     *
     * @return The value of the lower boundary
     */
	T getLowerBoundary() const  {
    	return lowerBoundary_;
	}

	/****************************************************************************/
    /**
     * Retrieves the upper boundary
     *
     * @return The value of the upper boundary
     */
	T getUpperBoundary() const  {
    	return upperBoundary_;
	}

	/****************************************************************************/
	/**
	 * Resets the boundaries to the maximum allowed value. This is a
	 * trap. Use the specializations instead.
	 */
	void resetBoundaries() {
		std::ostringstream error;
		error << "In GBoundedNumT<T>::resetBoundaries(): Error!" << std::endl
		<< "This class seems to have been instantiated with types" << std::endl
		<< "it was not designed for:" << std::endl
		<< "typeid(T).name() = " << typeid(T).name() << std::endl;

		throw(Gem::GenEvA::geneva_error_condition(error.str()));
	}

	/****************************************************************************/
	/**
	 * Sets the boundaries of this object and does corresponding
	 * error checks. If the current value is below or above the new
	 * boundaries, this function will throw. Set the external value to
	 * a new value between the new boundaries before calling this
	 * function. Note that this class does not accept boundaries that
	 * exceed half of the maximum value of a double.
	 *
	 * @param lower The new lower boundary for this object
	 * @param upper The new upper boundary for this object
	 */
	void setBoundaries(const T& lower, const T& upper) {
		T currentValue = this->value();

		// Check that the boundaries make sense
		if(lower >= upper) {
			std::ostringstream error;
			error << "In GBoundedNumT<T>::setBoundaries(const T&, const T&)" << std::endl
				     << "with typeid(T).name() = " << typeid(T).name() << " : Error" << std::endl
				     << "Lower and/or upper boundary has invalid value : " << lower << " " << upper << std::endl;

			throw(Gem::GenEvA::geneva_error_condition(error.str()));
		}

		// Check size of the boundaries
		if(double(lower) <= -0.5*std::numeric_limits<double>::max() || double(upper) >= 0.5*std::numeric_limits<double>::max()) {
			std::ostringstream error;
			error << "In GBoundedNumT<T>::setBoundaries(const T&, const T&)" << std::endl
				     << "with typeid(T).name() = " << typeid(T).name() << " : Error" << std::endl
				     << "Lower and/or upper boundaries have too high values: " << lower << " " << upper << std::endl;

			throw(Gem::GenEvA::geneva_error_condition(error.str()));
		}

		// Check that the value is inside the allowed range
		if(currentValue < lower || currentValue > upper){
			std::ostringstream error;
			error << "In GBoundedNumT<T>::setBoundaries(const T&, const T&) : Error!" << std::endl
				      << "with typeid(T).name() = " << typeid(T).name() << std::endl
				      << "Attempt to set new boundaries [" << lower << ":" << upper << "]" << std::endl
				      << "with existing value  " << currentValue << " outside of this range." << std::endl;

			// throw an exception. Add some information so that if the exception
			// is caught through a base object, no information is lost.
			throw geneva_error_condition(error.str());
		}

		lowerBoundary_ = lower;
		upperBoundary_ = upper;

		// Restore the original external value
		this->setExternalValue(currentValue);
	}

	/****************************************************************************/
	/**
	 * This function allows automatic conversion from GBoundedNumT<T> to T..
	 * This allows us to define only few operators, as the bulk of the work will be
	 * done by automatic conversions done by the C++ compiler.
	 */
	operator T () const {
		return this->value();
	}

	/****************************************************************************/
	/**
	 * Mutates this object. It is the internal representation of the class'es value
	 * that gets mutated. This value is then "translated" into the external value (stored
	 * in GParameterT<T>, which is set accordingly.
	 */
	virtual void mutateImpl() {
		this->applyAdaptor(internalValue_);

		// Then calculate the corresponding external value and set it accordingly
		T externalValue = calculateExternalValue(internalValue_);

		// Set the external value accordingly.
		setExternalValue(externalValue);
	}

	/****************************************************************************/
	/**
	 * Does the actual mapping from internal to external value. No error
	 * checks are done, as it is assumed that all values of boundaries have been
	 * checked when they were set.
	 *
	 * @param in The internal value that is to be converted to an external value
	 * @return The externally visible representation of in
	 */
	T calculateExternalValue(const T& in) {
		// Find out which region the value is in (compare figure transferFunction.pdf
		// that should have been delivered with this software . Note that numeric_cast
		// may throw - exceptions must be caught in surrounding functions.
		boost::int32_t region =
			boost::numeric_cast<boost::int32_t>(std::floor((double(in) - double(lowerBoundary_)) / (double(upperBoundary_) - double(lowerBoundary_))));

			// Check whether we are in an odd or an even range and calculate the
			// external value accordingly
			double externalValue = 0.;
			if(region%2 == 0){ // can it be divided by 2 ? Region 0,2,... or a negative even range
				externalValue = in - region * (upperBoundary_ - lowerBoundary_);
			}
			else{ // Range 1,3,... or a negative odd range
				externalValue = -in + ((region-1)*(upperBoundary_ - lowerBoundary_) + 2*upperBoundary_);
			}

			return T(externalValue);
	}

	/****************************************************************************/
	/**
	 * Retrieves the internal representation of our value
	 *
	 * @return The current value of internalValue_
	 */
	T getInternalValue() const  {
		return internalValue_;
	}

protected:
	/****************************************************************************/
	/**
	 * Create a deep copy of this object. Basically this is a fabric function.
	 * Purely virtual as this class is never meant to be instantiated directly.
	 *
	 * @return A newly generated GBoundedNumT<T> copy of this object
	 */
	virtual GObject *clone_() const = 0;

private:
	/****************************************************************************/
	/**
	 * Sets the internal value in such a way that the user-visible
	 * value is set to "val". GBoundedNumT<T> performs a linear transformation
	 * from inner to outer value inside of the value range.
	 *
	 * @param val The desired new external value
	 * @return The former value
	 */
	T setExternalValue(const T& val) {
		T previous = this->value();

		// Check the lower and upper boundaries
		if(upperBoundary_ <= lowerBoundary_){
			std::ostringstream error;
			error << "In GBoundedNumT<T>::setExternalValue() : Error!" << std::endl
			<< "with typeid(T).name() = " << typeid(T).name() << std::endl
			<< "Got invalid upper and/or lower boundaries" << std::endl
			<< "lowerBoundary_ = " << lowerBoundary_ << std::endl
			<< "upperBoundary_ = " << upperBoundary_ << std::endl;

			// throw an exception. Add some information so that if the exception
			// is caught through a base object, no information is lost.
			throw geneva_error_condition(error.str());
		}

		// Check that the value is inside the allowed range
		if(val < lowerBoundary_ || val > upperBoundary_){
			std::ostringstream error;
			error << "In GBoundedNumT<T>::setExternalValue() : Error!" << std::endl
			<< "with typeid(T).name() = " << typeid(T).name() << std::endl
			<< "Attempt to set external value " << val << std::endl
			<< "outside of allowed range " << "[" << lowerBoundary_ << ":" << upperBoundary_ << "]" << std::endl;

			// throw an exception. Add some information so that if the exception
			// is caught through a base object, no information is lost.
			throw geneva_error_condition(error.str());
		}

		// The transfer function in this area is just f(x)=x, so we can just
		// assign the external to the internal value.
		internalValue_ = val;
		this->setValue(val);

		return previous;
	}

	/****************************************************************************/
	T lowerBoundary_, upperBoundary_; ///< The upper and lower allowed boundaries for our external value
	T internalValue_; ///< The internal representation of this class'es value
};

/******************************************************************************/
// Specializations for some "allowed" types
template <>  GBoundedNumT<double>::GBoundedNumT();
template <>  GBoundedNumT<boost::int32_t>::GBoundedNumT();
template <> GBoundedNumT<double>::GBoundedNumT(const double&);
template <> GBoundedNumT<boost::int32_t>::GBoundedNumT(const boost::int32_t&);
template <> GBoundedNumT<double>::GBoundedNumT(const double&, const double&);
template <> GBoundedNumT<boost::int32_t>::GBoundedNumT(const boost::int32_t&, const boost::int32_t&);
template <> void GBoundedNumT<double>::resetBoundaries();
template <> void GBoundedNumT<boost::int32_t>::resetBoundaries();

/******************************************************************************/
/**
 * The default constructor. As this class uses the adaptor scheme
 * (see GTemplateAdaptor<T>), you will need to add your own adaptors,
 * such as the GDoubleGaussAdaptor. This function is a trap. Use one of the
 * provided specializations instead. Non-inline definition in order to circumvent
 * deficiencies of g++ 3.4.6
 */
template <typename T>
GBoundedNumT<T>::GBoundedNumT()
	: GParameterT<T>(T(0))
	, lowerBoundary_(T(0))
	, upperBoundary_(T(1))
	, internalValue_(T(0))
{
	std::ostringstream error;
	error << "In GBoundedNumT<T>::GBoundedNumT(): Error!" << std::endl
			 << "This class seems to have been instantiated with types" << std::endl
			 << "it was not designed for:" << std::endl
			 << "typeid(T).name() = " << typeid(T).name() << std::endl;

	throw(Gem::GenEvA::geneva_error_condition(error.str()));
}

/******************************************************************************/
/**
 * A constructor that initializes the external value only. The boundaries will
 * be set to the maximum and minimum values of the corresponding type.
 * This function is a trap. Use one of the provided specializations instead.
 * Non-inline definition in order to circumvent deficiencies of g++ 3.4.6
 *
 * @param val The desired external value of this object
 */
template <typename T>
GBoundedNumT<T>::GBoundedNumT(const T& val)
	: GParameterT<T>(T(0))
	, lowerBoundary_(T(0))
	, upperBoundary_(T(1))
	, internalValue_(T(0))
{
	std::ostringstream error;
	error << "In GBoundedNumT<T>::GBoundedNumT(const T&): Error!" << std::endl
			 << "This class seems to have been instantiated with types" << std::endl
			 << "it was not designed for:" << std::endl
			 << "typeid(T).name() = " << typeid(T).name() << std::endl;

	throw(Gem::GenEvA::geneva_error_condition(error.str()));
}

/******************************************************************************/
/**
 * Initializes the boundaries and sets the external value to a random number
 * inside the allowed value range. This function is a trap. Use one of the
 * provided specializations instead. Non-inline definition in order to circumvent
 * deficiencies of g++ 3.4.6
 *
 * @param lowerBoundary The lower boundary of the value range
 * @param upperBoundary The upper boundary of the value range
 */
template <typename T>
GBoundedNumT<T>::GBoundedNumT(const T& lowerBoundary, const T& upperBoundary)
	: GParameterT<T>(T(0))
	, lowerBoundary_(lowerBoundary)
	, upperBoundary_(upperBoundary)
	, internalValue_(T(0))
{
	std::ostringstream error;
	error << "In GBoundedNumT<T>::GBoundedNumT(const T&, const T&): Error!" << std::endl
	<< "This class seems to have been instantiated with types" << std::endl
	<< "it was not designed for:" << std::endl
	<< "typeid(T).name() = " << typeid(T).name() << std::endl;

	throw(Gem::GenEvA::geneva_error_condition(error.str()));
}
/******************************************************************************/
/**
 * Initialize with a given value and the allowed value range. Non-inline definition
 * in order to circumvent deficiencies of g++ 3.4.6
 *
 * @param val Initialization value
 * @param lowerBoundary The lower boundary of the value range
 * @param upperBoundary The upper boundary of the value range
 */
template <typename T>
GBoundedNumT<T>::GBoundedNumT(const T& val, const T& lowerBoundary, const T& upperBoundary)
	: GParameterT<T>(T(0))
	, lowerBoundary_(lowerBoundary)
	, upperBoundary_(upperBoundary)
	, internalValue_(T(0))
{
	setExternalValue(val);
}

/******************************************************************************/
/**
 * A standard copy constructor. Most work is done by the parent
 * classes, we only need to copy the internal value and the allowed
 * value range. Non-inline definition in order to circumvent deficiencies
 * of g++ 3.4.6
 *
 * @param cp Another GBoundedNumT<T> object
 */
template <typename T>
GBoundedNumT<T>::GBoundedNumT(const GBoundedNumT<T>& cp)
	: GParameterT<T>(cp)
	, lowerBoundary_(cp.lowerBoundary_)
	, upperBoundary_(cp.upperBoundary_)
	, internalValue_(cp.internalValue_)
{ /* nothing */ }

/******************************************************************************/
/**
 * A standard destructor. No local, dynamically allocated data,
 * hence nothing to do.  Non-inline definition in order to circumvent
 * deficiencies of g++ 3.4.6
 */
template <typename T>
GBoundedNumT<T>::~GBoundedNumT()
{ /* nothing */ }

/******************************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */

/******************************************************************************/
// The content of BOOST_SERIALIZATION_ASSUME_ABSTRACT(T)

namespace boost {
	namespace serialization {
		template<typename T>
		struct is_abstract<Gem::GenEvA::GBoundedNumT<T> > : public boost::true_type {};
		template<typename T>
		struct is_abstract< const Gem::GenEvA::GBoundedNumT<T> > : public boost::true_type {};
	}
}

/******************************************************************************/

#endif /* GBOUNDEDNUMT_HPP_ */
