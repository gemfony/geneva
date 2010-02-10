/**
 * @file GParameterT.hpp
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

// Standard header files go here
#include <sstream>
#include <cmath>

// Includes check for correct Boost version(s)
#include "GGlobalDefines.hpp"

// Boost header files go here

#include <boost/cstdint.hpp>

#ifndef GPARAMETERT_HPP_
#define GPARAMETERT_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif


// GenEvA header files go here
#include "GParameterBaseWithAdaptorsT.hpp"
#include "GObject.hpp"
#include "GenevaExceptions.hpp"

namespace Gem {
namespace GenEvA {

/*******************************************************************************************/
/**
 * A class holding a single, mutable parameter - usually just an atomic value (double, long,
 * GenEvA::bit, ...). The class is non-virtual, so that it is possible to store simple values
 * in this class without too much fuss.
 */
template <typename T>
class GParameterT
	:public GParameterBaseWithAdaptorsT<T>
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int){
	  using boost::serialization::make_nvp;

	  ar & make_nvp("GParameterBaseWithAdaptors_T", boost::serialization::base_object<GParameterBaseWithAdaptorsT<T> >(*this))
	     & BOOST_SERIALIZATION_NVP(val_);
	}
	///////////////////////////////////////////////////////////////////////

public:
	/** @brief Used to identify the type supplied to this object */
	typedef T p_type;

	/** @brief The default constructor */
	GParameterT();
	/** @brief The copy constructor */
	GParameterT(const GParameterT<T>& cp);
	/** @brief Initialization by contained value */
	explicit GParameterT(const T& val);
	/** @brief The destructor */
	virtual ~GParameterT();

	/*******************************************************************************************/
	/**
	 * A standard assignment operator.
	 *
	 * @param cp A copy of another GParameterT object
	 * @return A constant reference to this object
	 */
	const GParameterT<T>& operator=(const GParameterT<T>& cp){
		GParameterT<T>::load(&cp);
		return *this;
	}

	/*******************************************************************************************/
	/**
	 * An assignment operator that allows us to set val_
	 *
	 * @param val The new value for val_
	 * @return The new value of val_
	 */
	virtual const T& operator=(const T& val){
		val_ = val;
		return val_;
	}

	/*******************************************************************************************/
	/**
	 * Automatic conversion to the target type
	 */
	operator T() const{
		return this->value();
	}

	/*******************************************************************************************/
	/**
	 * Retrieval of the value
	 *
	 * @return The value of val_
	 */
	T value() const{
		return val_;
	}

	/*******************************************************************************************/
	/**
	 * Checks for equality with another GParameterT<T> object
	 *
	 * @param  cp A constant reference to another GParameterT<T> object
	 * @return A boolean indicating whether both objects are equal
	 */
	bool operator==(const GParameterT<T>& cp) const {
		using namespace Gem::Util;
		// Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
		return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GParameterT<T>::operator==","cp", CE_SILENT);
	}

	/*******************************************************************************************/
	/**
	 * Checks for inequality with another GParameterT<T> object
	 *
	 * @param  cp A constant reference to another GParameterT<T> object
	 * @return A boolean indicating whether both objects are inequal
	 */
	bool operator!=(const GParameterT<T>& cp) const {
		using namespace Gem::Util;
		// Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
		return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GParameterT<T>::operator!=","cp", CE_SILENT);
	}

	/*******************************************************************************************/
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
		const GParameterT<T>  *p_load = GObject::conversion_cast<GParameterT<T> >(&cp);

		// Will hold possible deviations from the expectation, including explanations
	    std::vector<boost::optional<std::string> > deviations;

		// Check our parent class'es data ...
		deviations.push_back(GParameterBaseWithAdaptorsT<T>::checkRelationshipWith(cp, e, limit, "GParameterT<T>", y_name, withMessages));

		// ... and then our local data
		deviations.push_back(checkExpectation(withMessages, "GParameterT<T>", val_, p_load->val_, "val_", "p_load->val_", e , limit));

		return evaluateDiscrepancies("GParameterT<T>", caller, deviations, e);
	}

	/*******************************************************************************************/
	/**
	 * Allows to mutate the value stored in this class.
	 */
	virtual void mutateImpl(){
		GParameterBaseWithAdaptorsT<T>::applyAdaptor(val_);
	}

	/*******************************************************************************************/
	/**
	 * Applies modifications to this object. This is needed for testing purposes
	 *
	 * @return A boolean which indicates whether modifications were made
	 */
	virtual bool modify_GUnitTests() {
		bool result;

		// Call the parent classes' functions
		if(GParameterBaseWithAdaptorsT<T>::modify_GUnitTests()) result = true;

		return result;
	}

	/*******************************************************************************************/
	/**
	 * Performs self tests that are expected to succeed. This is needed for testing purposes
	 */
	virtual void specificTestsNoFailureExpected_GUnitTests() {
		// Call the parent classes' functions
		GParameterBaseWithAdaptorsT<T>::specificTestsNoFailureExpected_GUnitTests();
	}

	/*******************************************************************************************/
	/**
	 * Performs self tests that are expected to fail. This is needed for testing purposes
	 */
	virtual void specificTestsFailuresExpected_GUnitTests() {
		// Call the parent classes' functions
		GParameterBaseWithAdaptorsT<T>::specificTestsFailuresExpected_GUnitTests();
	}

protected:
	/*******************************************************************************************/
	/**
	 * Loads the data of another GObject
	 *
	 * @param cp A copy of another GParameterT<T> object, camouflaged as a GObject
	 */
	virtual void load_(const GObject* cp){
		// Convert cp into local format
		const GParameterT<T> *p_load = GObject::conversion_cast<GParameterT<T> >(cp);

		// Load our parent class'es data ...
		GParameterBaseWithAdaptorsT<T>::load_(cp);

		// ... and then our own data
		val_ = p_load->val_;
	}

	/*******************************************************************************************/
	/**
	 * Creates a deep clone of this object. Purely virtual, needs to be implemented in derived
	 * classes.
	 *
	 * @return A copy of this object, camouflaged as a GObject
	 */
	virtual GObject* clone_() const = 0;

	/*******************************************************************************************/
	/**
	 * Allows derivatives to set the internal value. Note that we assume here that T has an
	 * operator=() or is a basic value type, such as double or int.
	 *
	 * @param val The new T value stored in this class
	 */
	void setValue(const T& val)  {
		val_ = val;
	}

private:
	/*******************************************************************************************/

	T val_; ///< The internal representation of our value. It is protected as it needs to accessible to derived classes.
};


/*********************************************************************************************/
/**
 * The default constructor. Non-inline definition in order to circumvent a g++ 3.4.6 deficiency.
 */
template <typename T>
GParameterT<T>::GParameterT()
	: GParameterBaseWithAdaptorsT<T>()
	, val_(T(0))
{ /* nothing */ }

/*********************************************************************************************/
/**
 * The copy constructor.  Non-inline definition in order to circumvent a g++ 3.4.6 deficiency.
 *
 * @param cp A copy of another GParameterT<T> object
 */
template <typename T>
GParameterT<T>::GParameterT(const GParameterT<T>& cp)
	: GParameterBaseWithAdaptorsT<T>(cp)
	, val_(cp.val_)
{ /* nothing */	}

/*********************************************************************************************/
/**
 * Initialization by contained value.Non-inline definition in order to circumvent a g++ 3.4.6 deficiency.
 *
 * @param val The new value of val_
 */
template <typename T>
GParameterT<T>::GParameterT(const T& val)
	: GParameterBaseWithAdaptorsT<T>()
	, val_(val)
{ /* nothing */	}

/*********************************************************************************************/
/**
 * The standard destructor.Non-inline definition in order to circumvent a g++ 3.4.6 deficiency.
 */
template <typename T>
GParameterT<T>:: ~GParameterT()
{ /* nothing */ }

/*********************************************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */

/********************************************************************************************/
// The content of BOOST_SERIALIZATION_ASSUME_ABSTRACT(T)

namespace boost {
	namespace serialization {
		template<typename T>
		struct is_abstract<Gem::GenEvA::GParameterT<T> > : public boost::true_type {};
		template<typename T>
		struct is_abstract< const Gem::GenEvA::GParameterT<T> > : public boost::true_type {};
	}
}

/********************************************************************************************/

#endif /* GPARAMETERT_HPP_ */
