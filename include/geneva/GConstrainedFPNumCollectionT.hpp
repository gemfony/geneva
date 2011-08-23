/**
 * @file GConstrainedFPNumCollectionT.hpp
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt)
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) com
 *
 * This file is part of the Geneva library collection.
 *
 * Geneva was developed with kind support from Karlsruhe Institute of
 * Technology (KIT) and Steinbuch Centre for Computing (SCC). Further
 * information about KIT and SCC can be found at http://www.kit.edu/english
 * and http://scc.kit.edu .
 *
 * Geneva is free software: you can redistribute and/or modify it under
 * the terms of version 3 of the GNU Affero General Public License
 * as published by the Free Software Foundation.
 *
 * Geneva is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with the Geneva library. If not, see <http://www.gnu.org/licenses/>.
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
#include <boost/math/special_functions/next.hpp> // Needed so we can calculate the next representable value smaller than a given upper boundary
#include <boost/cstdint.hpp>

#ifndef GCONSTRAINEDFPNUMCOLLECTIONT_HPP_
#define GCONSTRAINEDFPNUMCOLLECTIONT_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif


// Geneva header files go here
#include "common/GExceptions.hpp"
#include "geneva/GDoubleGaussAdaptor.hpp"
#include "geneva/GObject.hpp"
#include "geneva/GConstrainedNumCollectionT.hpp"

namespace Gem {
namespace Geneva {

/**********************************************************************/
/**
 * This class represents a collection of floating point values with common
 * boundaries, all modified using the same algorithm. The most likely types to
 * be stored in this class are double values. Note: If you want
 * to access or set the transformed value, use the value() and setValue()
 * functions. Using the subscript operator or at() function, or the
 * native iterator, will give you the "raw" data only.
 */
template <typename fp_type>
class GConstrainedFPNumCollectionT
	: public GConstrainedNumCollectionT<fp_type>
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int) {
		using boost::serialization::make_nvp;
		ar & make_nvp("GConstrainedNumCollectionT",	boost::serialization::base_object<GConstrainedNumCollectionT<fp_type> >(*this))
		   & BOOST_SERIALIZATION_NVP(upper_closed_);
	}
	///////////////////////////////////////////////////////////////////////

	// Make sure this class can only be instantiated if fp_type really is a floating point type
	BOOST_MPL_ASSERT((boost::is_floating_point<fp_type>));

public:
	/** @brief Specifies the type of parameters stored in this collection */
	typedef fp_type collection_type;

	/******************************************************************/
	/**
	 * Initialize the lower and upper boundaries for data members of this class
	 *
	 * @param size The desired size of the collection
	 * @param lowerBoundary The lower boundary for data members
	 * @param upperBoundary The upper boundary for data members
	 */
	GConstrainedFPNumCollectionT (
			const std::size_t& size
			, const fp_type& lowerBoundary
			, const fp_type& upperBoundary
	)
		: GConstrainedNumCollectionT<fp_type> (size, lowerBoundary, upperBoundary)
		, upper_closed_(fp_type(0))
	{
		// Do some error checking.

		// The following is a stronger requirement than used in our
		// parent class, as the "real" upper boundary is defined by "upper_closed_".
		if(lowerBoundary >= upperBoundary) {
			raiseException(
					"In GConstrainedFPNumCollectionT<fp_type>::GConstrainedFPNumCollectionT(lower,upper):" << std::endl
					<< "lowerBoundary = " << lowerBoundary << "is >= than" << std::endl
					<< "upperBoundary = " << upperBoundary
			);
		}

		// Assign a correct value to the upper_closed_ variable
		upper_closed_ = boost::math::float_prior<fp_type>(GConstrainedNumCollectionT<fp_type>::getUpperBoundary());
	}

	/******************************************************************/
	/**
	 * The standard copy constructor
	 */
	GConstrainedFPNumCollectionT(const GConstrainedFPNumCollectionT<fp_type>& cp)
		: GConstrainedNumCollectionT<fp_type> (cp)
		, upper_closed_(cp.upper_closed_)
	{ /* nothing */ }

	/******************************************************************/
	/**
	 * The standard destructor
	 */
	virtual ~GConstrainedFPNumCollectionT()
	{ /* nothing */ }

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
	boost::optional<std::string> checkRelationshipWith(
			const GObject& cp
			, const Gem::Common::expectation& e
			, const double& limit
			, const std::string& caller
			, const std::string& y_name
			, const bool& withMessages
	) const {
	    using namespace Gem::Common;

		// Check that we are indeed dealing with a GParamterBase reference
		const GConstrainedFPNumCollectionT<fp_type>  *p_load = GObject::gobject_conversion<GConstrainedFPNumCollectionT<fp_type> >(&cp);

		// Will hold possible deviations from the expectation, including explanations
	    std::vector<boost::optional<std::string> > deviations;

		// Check our parent class'es data ...
		deviations.push_back(GConstrainedNumCollectionT<fp_type>::checkRelationshipWith(cp, e, limit, "GConstrainedFPNumCollectionT<fp_type>", y_name, withMessages));

		// ... and then our local data
		deviations.push_back(checkExpectation(withMessages, "GConstrainedFPNumCollectionT<fp_type>", upper_closed_, p_load->upper_closed_, "upper_closed_", "p_load->upper_closed_", e , limit));

		return evaluateDiscrepancies("GConstrainedFPNumCollectionT<fp_type>", caller, deviations, e);
	}

	/****************************************************************************/
	/**
	 * The transfer function needed to calculate the externally visible value.
	 *
	 * @param val The value to which the transformation should be applied
	 * @return The transformed value
	 */
	virtual fp_type transfer(const fp_type& val) const {
		fp_type lowerBoundary = GConstrainedNumCollectionT<fp_type>::getLowerBoundary();
		fp_type upperBoundary = upper_closed_; // We want turning points to be at the next representable fp value below the assigned upper boundary

		if(val >= lowerBoundary && val < upperBoundary) {
			return val;
		}
		else {
			// Find out which region the value is in (compare figure transferFunction.pdf
			// that should have been delivered with this software). Note that boost::numeric_cast<>
			// may throw - exceptions must be caught in surrounding functions.
			boost::int32_t region = 0;

#ifdef DEBUG
			region =	boost::numeric_cast<boost::int32_t>(Gem::Common::GFloor((fp_type(val) - fp_type(lowerBoundary)) / (fp_type(upperBoundary) - fp_type(lowerBoundary))));
#else
			region =	static_cast<boost::int32_t>(Gem::Common::GFloor((fp_type(val) - fp_type(lowerBoundary)) / (fp_type(upperBoundary) - fp_type(lowerBoundary))));
#endif

			// Check whether we are in an odd or an even range and calculate the
			// external value accordingly
			fp_type mapping = fp_type(0.);
			if(region%2 == 0) { // can it be divided by 2 ? Region 0,2,... or a negative even range
				mapping = val - fp_type(region) * (upperBoundary - lowerBoundary);
			} else { // Range 1,3,... or a negative odd range
				mapping = -val + (fp_type(region-1)*(upperBoundary - lowerBoundary) + 2*upperBoundary);
			}

			return mapping;
		}

		// Make the compiler happy
		return fp_type(0.);
	}

	/****************************************************************************/
	/**
	 * Initializes floating-point-based parameters with a given value. Allows e.g. to set all
	 * floating point parameters to 0. Note that, contrary to the usual behavior,
	 * we accept initialization outside of the allowed boundaries. However, the internal
	 * representation will then be transferred back to an external value in the allowed
	 * value range.
	 *
	 * @param val The value to be assigned to the parameters
	 */
	virtual void fpFixedValueInit(const float& val)
	{
		for(std::size_t pos=0; pos<this->size(); pos++) {
			GParameterCollectionT<fp_type>::setValue(pos, transfer(fp_type(val)));
		}
	}

	/****************************************************************************/
	/**
	 * Multiplies floating-point-based parameters with a given value. Note that the resulting
	 * internal value may well be outside of the allowed boundaries. However, the internal
	 * representation will then be transferred back to an external value in the allowed
	 * value range.
	 *
	 * @param val The value to be multiplied with a set of parameters
	 */
	virtual void fpMultiplyBy(const float& val) {
		for(std::size_t pos=0; pos<this->size(); pos++) {
			GParameterCollectionT<fp_type>::setValue(
					pos
					, transfer(fp_type(val) *
							   this->value(pos))
			);
		}
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
		for(std::size_t pos=0; pos<this->size(); pos++) {
			GParameterCollectionT<fp_type>::setValue(
					pos
					, transfer(
							this->value(pos) *
							this->GParameterBase::gr->Gem::Hap::GRandomBase::uniform_real<fp_type>(fp_type(min), fp_type(max))
					)
			);
		}
	}

	/****************************************************************************/
	/**
	 * Multiplies with a random floating point number in the range [0, 1[.  Note that the resulting
	 * internal value may well be outside of the allowed boundaries. However, the internal
	 * representation will then be transferred back to an external value in the allowed
	 * value range.
	 */
	virtual void fpMultiplyByRandom() {
		for(std::size_t pos=0; pos<this->size(); pos++) {
			GParameterCollectionT<fp_type>::setValue(
					pos
					, transfer(this->value(pos) * this->GParameterBase::gr->Gem::Hap::GRandomBase::uniform_01<fp_type>())
			);
		}
	}

	/****************************************************************************/
	/**
	 * Adds the floating point parameters of another GParameterBase object to this one.
	 * Note that the resulting internal value may well be outside of the allowed boundaries.
	 * However, the internal representation will then be transferred back to an external
	 * value in the allowed value range.
	 *
	 * @oaram p_base A boost::shared_ptr to another GParameterBase object
	 */
	virtual void fpAdd(boost::shared_ptr<GParameterBase> p_base) {
		// We first need to convert p_base into the local type
		boost::shared_ptr<GConstrainedFPNumCollectionT<fp_type> > p
			= GParameterBase::parameterbase_cast<GConstrainedFPNumCollectionT<fp_type> >(p_base);

#ifdef DEBUG
		// Cross-check that the sizes match
		if(this->size() != p->size()) {
			raiseException(
					"In GConstrainedFPNumCollectionT<fp_type>::fpAdd():" << std::endl
					<< "Sizes of vectors don't match: " << this->size() << "/" << p->size()
			);
		}
#endif

		for(std::size_t pos=0; pos<this->size(); pos++) {
			GParameterCollectionT<fp_type>::setValue(
					pos
					, transfer(GParameterCollectionT<fp_type>::value(pos) + p->value(pos))
			);
		}
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
		boost::shared_ptr<GConstrainedFPNumCollectionT<fp_type> > p
			= GParameterBase::parameterbase_cast<GConstrainedFPNumCollectionT<fp_type> >(p_base);

#ifdef DEBUG
		// Cross-check that the sizes match
		if(this->size() != p->size()) {
			raiseException(
					"In GConstrainedFPNumCollectionT<fp_type>::fpAdd():" << std::endl
					<< "Sizes of vectors don't match: " << this->size() << "/" << p->size()
			);
		}
#endif

		for(std::size_t pos=0; pos<this->size(); pos++) {
			GParameterCollectionT<fp_type>::setValue(
					pos
					, transfer(GParameterCollectionT<fp_type>::value(pos) - p->value(pos))
			);
		}
	}

protected:
	/******************************************************************/
	/**
	 * Loads the data of another GConstrainedFPNumCollectionT<fp_type> object,
	 * camouflaged as a GObject. We have no local data, so
	 * all we need to do is to the standard identity check,
	 * preventing that an object is assigned to itself.
	 *
	 * @param cp A copy of another GConstrainedFPNumCollectionT<fp_type> object, camouflaged as a GObject
	 */
	virtual void load_(const GObject *cp){
		// Convert cp into local format
		const GConstrainedFPNumCollectionT<fp_type> *p_load = GObject::gobject_conversion<GConstrainedFPNumCollectionT<fp_type> >(cp);

		// Load our parent class'es data ...
		GConstrainedNumCollectionT<fp_type>::load_(cp);

		// ... and then our local data
		upper_closed_ = p_load->upper_closed_;
	}

	/******************************************************************/
	/** @brief Creates a deep copy of this object */
	virtual GObject *clone_() const = 0;

	/******************************************************************/
	/**
	 * Triggers random initialization of the parameter collection
	 */
	virtual void randomInit_() {
		for(std::size_t pos=0; pos<this->size(); pos++) {
			this->setValue(
				pos
				, this->GParameterBase::gr->Gem::Hap::GRandomBase::uniform_real<fp_type>(
						GConstrainedNumCollectionT<fp_type>::getLowerBoundary()
						, GConstrainedNumCollectionT<fp_type>::getUpperBoundary()
				)
			);
		}
	}

	/******************************************************************/
	/**
	 * The default constructor. Intentionally protected, as it is only
	 * needed for de-serialization and as the basis for derived class'es
	 * default constructors.
	 */
	GConstrainedFPNumCollectionT()
		: GConstrainedNumCollectionT<fp_type> ()
		, upper_closed_(fp_type(0))
	{
		// Assign a correct value to the upper_closed_ variable
		upper_closed_ = boost::math::float_prior<fp_type>(GConstrainedNumCollectionT<fp_type>::getUpperBoundary());
	}

private:
	/******************************************************************/

	fp_type upper_closed_; //< The next floating point value directly before an upper boundary

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
		if(GConstrainedNumCollectionT<fp_type>::modify_GUnitTests()) result = true;

		return result;
	}

	/******************************************************************/
	/**
	 * Performs self tests that are expected to succeed. This is needed for testing purposes
	 */
	virtual void specificTestsNoFailureExpected_GUnitTests() {
		// Call the parent classes' functions
		GConstrainedNumCollectionT<fp_type>::specificTestsNoFailureExpected_GUnitTests();
	}

	/******************************************************************/
	/**
	 * Performs self tests that are expected to fail. This is needed for testing purposes
	 */
	virtual void specificTestsFailuresExpected_GUnitTests() {
		// Call the parent classes' functions
		GConstrainedNumCollectionT<fp_type>::specificTestsFailuresExpected_GUnitTests();
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
		template<typename fp_type>
		struct is_abstract<Gem::Geneva::GConstrainedFPNumCollectionT<fp_type> > : public boost::true_type {};
		template<typename fp_type>
		struct is_abstract< const Gem::Geneva::GConstrainedFPNumCollectionT<fp_type> > : public boost::true_type {};
	}
}
/**********************************************************************/

#endif /* GCONSTRAINEDFPNUMCOLLECTIONT_HPP_ */
