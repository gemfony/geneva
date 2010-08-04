/**
 * @file GIntFlipAdaptorT.hpp
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
#include <utility> // For std::pair

// Includes check for correct Boost version(s)
#include "common/GGlobalDefines.hpp"

// Boost headers go here

#include <boost/cstdint.hpp>
#include <boost/limits.hpp>
#include <boost/cast.hpp>

#ifndef GINTFLIPADAPTORT_HPP_
#define GINTFLIPADAPTORT_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif

// Geneva headers go here
#include "common/GExceptions.hpp"
#include "GAdaptorT.hpp"
#include "GConstrainedDouble.hpp"
#include "GDoubleGaussAdaptor.hpp"
#include "GObject.hpp"
#include "GOptimizationEnums.hpp"

namespace Gem {
namespace Geneva {

/************************************************************************************************/
/**
 * GIntFlipAdaptorT represents an adaptor used for the adaption of integer
 * types, by flipping an integer number to the next larger or smaller number.
 * The integer type used needs to be specified as a template parameter. Note
 * that a specialization of this class, as defined in GIntFlipAdaptorT.cpp,
 * allows to deal with booleans instead of "standard" integer types.
 */
template<typename T>
class GIntFlipAdaptorT:
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
	GIntFlipAdaptorT()
		: GAdaptorT<T> (DEFAULTBITADPROB)
	{ /* nothing */	}

	/********************************************************************************************/
	/**
	 * This constructor takes an argument, that specifies the (initial) probability
	 * for the adaption of an integer or bit value
	 *
	 * @param prob The probability for a flip
	 */
	explicit GIntFlipAdaptorT(const double& prob)
		: GAdaptorT<T>(prob)
	{ /* nothing */ }

	/********************************************************************************************/
	/**
	 * A standard copy constructor.
	 *
	 * @param cp Another GIntFlipAdaptorT object
	 */
	GIntFlipAdaptorT(const GIntFlipAdaptorT<T>& cp)
		: GAdaptorT<T>(cp)
	{ /* nothing */	}

	/********************************************************************************************/
	/**
	 * The standard destructor. Empty, as we have no local, dynamically
	 * allocated data.
	 */
	~GIntFlipAdaptorT()
	{ /* nothing */ }

	/********************************************************************************************/
	/**
	 * A standard assignment operator for GIntFlipAdaptorT objects,
	 *
	 * @param cp A copy of another GIntFlipAdaptorT object
	 */
	const GIntFlipAdaptorT<T>& operator=(const GIntFlipAdaptorT<T>& cp)
	{
		GIntFlipAdaptorT<T>::load_(&cp);
		return *this;
	}

	/********************************************************************************************/
	/**
	 * Checks for equality with another GIntFlipAdaptorT<T> object
	 *
	 * @param  cp A constant reference to another GIntFlipAdaptorT<T> object
	 * @return A boolean indicating whether both objects are equal
	 */
	bool operator==(const GIntFlipAdaptorT<T>& cp) const {
		using namespace Gem::Common;
		// Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
		return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GIntFlipAdaptorT<T>::operator==","cp", CE_SILENT);
	}

	/********************************************************************************************/
	/**
	 * Checks for inequality with another GIntFlipAdaptorT<T> object
	 *
	 * @param  cp A constant reference to another GIntFlipAdaptorT<T> object
	 * @return A boolean indicating whether both objects are inequal
	 */
	bool operator!=(const GIntFlipAdaptorT<T>& cp) const {
		using namespace Gem::Common;
		// Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
		return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GIntFlipAdaptorT<T>::operator!=","cp", CE_SILENT);
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
		const GIntFlipAdaptorT<T>  *p_load = GObject::conversion_cast<GIntFlipAdaptorT<T> >(&cp);

		// Will hold possible deviations from the expectation, including explanations
	    std::vector<boost::optional<std::string> > deviations;

		// Check our parent class'es data ...
		deviations.push_back(GAdaptorT<T>::checkRelationshipWith(cp, e, limit, "GIntFlipAdaptor", y_name, withMessages));

		// no local data ...

		return evaluateDiscrepancies("GIntFlipAdaptor", caller, deviations, e);
	}

	/********************************************************************************************/
	/**
	 * Retrieves the id of the adaptor. Purely virtual, as we do not want this class to be
	 * instantiated.
	 *
	 * @return The id of the adaptor
	 */
	virtual Gem::Geneva::adaptorId getAdaptorId() const = 0;

#ifdef GENEVATESTING
	/*******************************************************************************************/
	/**
	 * Applies modifications to this object. This is needed for testing purposes
	 *
	 * @return A boolean which indicates whether modifications were made
	 */
	virtual bool modify_GUnitTests() {
		bool result;

		// Call the parent classes' functions
		if(GAdaptorT<T>::modify_GUnitTests()) result = true;

		return result;
	}

	/*******************************************************************************************/
	/**
	 * Performs self tests that are expected to succeed. This is needed for testing purposes
	 */
	virtual void specificTestsNoFailureExpected_GUnitTests() {
		// Call the parent classes' functions
		GAdaptorT<T>::specificTestsNoFailureExpected_GUnitTests();
	}

	/*******************************************************************************************/
	/**
	 * Performs self tests that are expected to fail. This is needed for testing purposes
	 */
	virtual void specificTestsFailuresExpected_GUnitTests() {
		// Call the parent classes' functions
		GAdaptorT<T>::specificTestsFailuresExpected_GUnitTests();
	}

#endif /* GENEVATESTING */

protected:
	/********************************************************************************************/
	/**
	 * This function loads the data of another GIntFlipAdaptorT, camouflaged as a GObject.
	 *
	 * @param A copy of another GIntFlipAdaptorT, camouflaged as a GObject
	 */
	void load_(const GObject *cp)
	{
		// Check that this object is not accidently assigned to itself
		GObject::selfAssignmentCheck<GIntFlipAdaptorT<T> >(cp);

		// Load the data of our parent class ...
		GAdaptorT<T>::load_(cp);

		// no local data
	}

	/********************************************************************************************/
	/**
	 * This function creates a deep copy of this object. The function is purely virtual, as we
	 * do not want this class to be instantiated. Use the derived classes instead.
	 *
	 * @return A deep copy of this object
	 */
	virtual GObject *clone_() const = 0;

	/********************************************************************************************/
	/**
	 * We want to flip the value only in a given percentage of cases. Thus
	 * we calculate a probability between 0 and 1 and compare it with the desired
	 * adaption probability. Please note that evenRandom returns a value in the
	 * range of [0,1[, so we make a tiny error here. This function assumes
	 * an integer type. It hence flips the value up or down. A specialization
	 * for booleans is provided in GIntFlipAdaptorT.cpp .
	 *
	 * @param value The bit value to be adapted
	 */
	virtual void customAdaptions(T& value) {
		bool up = this->gr.uniform_bool();
		if(up){
#if defined (CHECKOVERFLOWS)
			if(std::numeric_limits<T>::max() == value) {
#ifdef DEBUG
				std::cout << "Warning: Had to change adaption due to overflow in GIntFlipAdaptorT<>::customAdaptions()" << std::endl;
#endif
				value -= 1;
			}
			else value += 1;
#else
			value += 1;
#endif
		}
		else {
#if defined (CHECKOVERFLOWS)
			if(std::numeric_limits<T>::min() == value) {
#ifdef DEBUG
				std::cout << "Warning: Had to change adaption due to underflow in GIntFlipAdaptorT<>::customAdaptions()" << std::endl;
#endif
				value += 1;
			}
			else value -= 1;
#else
			value -= 1;
#endif
		}
	}
};

/************************************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

/********************************************************************************************/
// The content of BOOST_SERIALIZATION_ASSUME_ABSTRACT(T)

namespace boost {
	namespace serialization {
		template<typename T>
		struct is_abstract<Gem::Geneva::GIntFlipAdaptorT<T> > : public boost::true_type {};
		template<typename T>
		struct is_abstract< const Gem::Geneva::GIntFlipAdaptorT<T> > : public boost::true_type {};
	}
}

/********************************************************************************************/

#endif /* GINTFLIPADAPTORT_HPP_ */
