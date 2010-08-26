/**
 * @file GNumFPT.hpp
 */

/*
 * Copyright (C) Authors of the Geneva library collection and Karlsruhe
 * Institute of Technology (University of the State of Baden-Wuerttemberg
 * and National Laboratory of the Helmholtz Association).
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: info [at] gemfony (dot) com
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
#include <string>
#include <sstream>
#include <vector>

// Includes check for correct Boost version(s)
#include "common/GGlobalDefines.hpp"

// Boost headers go here
#include <boost/cstdint.hpp>

#ifndef GNUMFPT_HPP_
#define GNUMFPT_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif


// Geneva headers go here
#include "GNumT.hpp"

namespace Gem {
namespace Geneva {

/**********************************************************************/
/**
 * This class represents floating point values. The most likely type to be stored
 * in this class is a double. It adds floating point initialization and multiplication
 * to GNumT
 */
template <typename T>
class GNumFPT
	: public GNumT<T>
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int) {
		using boost::serialization::make_nvp;
		ar & make_nvp("GNumT",	boost::serialization::base_object<GNumT<T> >(*this));
	}
	///////////////////////////////////////////////////////////////////////

public:
	/** @brief Specifies the type of parameters stored in this collection */
	typedef T collection_type;

	/******************************************************************/
	/**
	 * The default constructor.
	 */
	GNumFPT()
		: GNumT<T> ()
	{ /* nothing */ }

	/*****************************************************************/
	/*
	 * Initialize with a single value
	 *
	 * @param val The value used for the initialization
	 */
	explicit GNumFPT(const T& val)
		: GNumT<T>(val)
	{ /* nothing */ }

	/******************************************************************/
	/**
	 * Initialize with a random value within given boundaries
	 *
	 * @param min The lower boundary for random entries
	 * @param max The upper boundary for random entries
	 */
	GNumFPT(const T& min, const T& max)
		: GNumT<T> (min, max)
	{ /* nothing */ }

	/******************************************************************/
	/**
	 * The standard copy constructor
	 */
	GNumFPT(const GNumFPT<T>& cp)
		: GNumT<T> (cp)
	{ /* nothing */ }

	/******************************************************************/
	/**
	 * The standard destructor
	 */
	virtual ~GNumFPT()
	{ /* nothing */ }

	/******************************************************************/
	/**
	 * The standard assignment operator.
	 *
	 * @param cp A copy of another GDoubleCollection object
	 * @return A constant reference to this object
	 */
	const GNumFPT& operator=(const GNumFPT<T>& cp){
		GNumFPT<T>::load_(&cp);
		return *this;
	}

	/******************************************************************/
	/**
	 * An assignment operator for the contained value type
	 */
	virtual T operator=(const T& val) {
		return GNumT<T>::operator=(val);
	}

	/******************************************************************/
	/**
	 * Checks for equality with another GNumFPT<T> object
	 *
	 * @param  cp A constant reference to another GNumFPT<T> object
	 * @return A boolean indicating whether both objects are equal
	 */
	bool operator==(const GNumFPT<T>& cp) const {
		using namespace Gem::Common;
		// Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
		return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GNumFPT<T>::operator==","cp", CE_SILENT);
	}

	/******************************************************************/
	/**
	 * Checks for inequality with another GNumFPT<T> object
	 *
	 * @param  cp A constant reference to another GNumFPT<T> object
	 * @return A boolean indicating whether both objects are inequal
	 */
	bool operator!=(const GNumFPT<T>& cp) const {
		using namespace Gem::Common;
		// Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
		return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GNumFPT<T>::operator!=","cp", CE_SILENT);
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
		const GNumFPT<T>  *p_load = GObject::conversion_cast<GNumFPT<T> >(&cp);

		// Will hold possible deviations from the expectation, including explanations
	    std::vector<boost::optional<std::string> > deviations;

		// Check our parent class'es data ...
		deviations.push_back(GNumT<T>::checkRelationshipWith(cp, e, limit, "GNumFPT<T>", y_name, withMessages));

		// no local data

		return evaluateDiscrepancies("GNumFPT<T>", caller, deviations, e);
	}

	/******************************************************************/
	/**
	 * Initializes floating-point-based parameters with a given value.
	 *
	 * @param val The value to use for the initialization
	 */
	virtual void fpFixedValueInit(const float& val) {
		GParameterT<T>::setValue(T(val));
	}

	/******************************************************************/
	/**
	 * Multiplies floating-point-based parameters with a given value
	 *
	 * @param val The value to be multiplied with the parameter
	 */
	virtual void fpMultiplyBy(const float& val) {
		GParameterT<T>::setValue(GParameterT<T>::value() * T(val));
	}

	/******************************************************************/
	/**
	 * Multiplies with a random floating point number in a given range.
	 *
	 * @param min The lower boundary for random number generation
	 * @param max The upper boundary for random number generation
	 */
	void fpMultiplyByRandom(const float& min, const float& max)	{
		using namespace Gem::Hap;
		GRandomT<RANDOMLOCAL, T, boost::int32_t> gr;
		GParameterT<T>::setValue(GParameterT<T>::value() * gr.uniform_real(T(min), T(max)));
	}

	/******************************************************************/
	/**
	 * Multiplies with a random floating point number in the range [0, 1[.
	 */
	void fpMultiplyByRandom() {
		using namespace Gem::Hap;
		GRandomT<RANDOMLOCAL, T, boost::int32_t> gr;
		GParameterT<T>::setValue(GParameterT<T>::value() * gr.uniform_01());
	}

	/******************************************************************/
	/**
	 * Adds the floating point parameters of another GParameterBase
	 * object to this one.
	 *
	 * @oaram p A boost::shared_ptr to another GParameterBase object
	 */
	void fpAdd(boost::shared_ptr<GParameterBase> p_base) {
		// We first need to convert p_base into the local type
		boost::shared_ptr<GNumFPT<T> > p = GParameterBase::parameterbase_cast<GNumFPT<T> >(p_base);
		GParameterT<T>::setValue(GParameterT<T>::value() + p->value());
	}

	/******************************************************************/
	/**
	 * Subtracts the floating point parameters of another GParameterBase
	 * object from this one.
	 *
	 * @oaram p A boost::shared_ptr to another GParameterBase object
	 */
	void fpSubtract(boost::shared_ptr<GParameterBase> p_base) {
		// We first need to convert p_base into the local type
		boost::shared_ptr<GNumFPT<T> > p = GParameterBase::parameterbase_cast<GNumFPT<T> >(p_base);
		GParameterT<T>::setValue(GParameterT<T>::value() - p->value());
	}

protected:
	/******************************************************************/
	/**
	 * Loads the data of another GNumFPT<T> object,
	 * camouflaged as a GObject. We have no local data, so
	 * all we need to do is to the standard identity check,
	 * preventing that an object is assigned to itself.
	 *
	 * @param cp A copy of another GNumFPT<T> object, camouflaged as a GObject
	 */
	virtual void load_(const GObject *cp){
		// Convert cp into local format
		const GNumFPT<T> *p_load = GObject::conversion_cast<GNumFPT<T> >(cp);

		// Load our parent class'es data ...
		GNumT<T>::load_(cp);

		// no local data ...
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
	/**
	 * Triggers random initialization of the parameter
	 */
	virtual void randomInit_() {
		using namespace Gem::Hap;

		T lowerBoundary = GNumT<T>::getLowerInitBoundary();
		T upperBoundary = GNumT<T>::getUpperInitBoundary();

		GRandomT<RANDOMLOCAL, T, boost::int32_t> gr;

		GParameterT<T>::setValue(gr.uniform_real(lowerBoundary, upperBoundary));
	}

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
		if(GNumT<T>::modify_GUnitTests()) result = true;

		return result;
	}

	/******************************************************************/
	/**
	 * Performs self tests that are expected to succeed. This is needed for testing purposes
	 */
	virtual void specificTestsNoFailureExpected_GUnitTests() {
		// Call the parent classes' functions
		GNumT<T>::specificTestsNoFailureExpected_GUnitTests();
	}

	/******************************************************************/
	/**
	 * Performs self tests that are expected to fail. This is needed for testing purposes
	 */
	virtual void specificTestsFailuresExpected_GUnitTests() {
		// Call the parent classes' functions
		GNumT<T>::specificTestsFailuresExpected_GUnitTests();
	}

#endif /* GENEVATESTING */
};

} /* namespace Geneva */
} /* namespace Gem */

/**********************************************************************/
// The content of BOOST_SERIALIZATION_ASSUME_ABSTRACT(T)

namespace boost {
	namespace serialization {
		template<typename T>
		struct is_abstract<Gem::Geneva::GNumFPT<T> > : public boost::true_type {};
		template<typename T>
		struct is_abstract< const Gem::Geneva::GNumFPT<T> > : public boost::true_type {};
	}
}
/**********************************************************************/

#endif /* GNUMFPT_HPP_ */
