/**
 * @file GConstrainedFPNumCollectionT.hpp
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt)
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) eu
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
 * http://www.gemfony.eu .
 */

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard header files go here
#include <string>
#include <iostream>
#include <sstream>
#include <vector>

// Boost header files go here
#include <boost/math/special_functions/next.hpp> // Needed so we can calculate the next representable value smaller than a given upper boundary
#include <boost/cstdint.hpp>

#ifndef GCONSTRAINEDFPNUMCOLLECTIONT_HPP_
#define GCONSTRAINEDFPNUMCOLLECTIONT_HPP_

// Geneva header files go here
#include "common/GExceptions.hpp"
#include "geneva/GDoubleGaussAdaptor.hpp"
#include "geneva/GObject.hpp"
#include "geneva/GConstrainedNumCollectionT.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
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
		ar
		& make_nvp("GConstrainedNumCollectionT",
		      boost::serialization::base_object<GConstrainedNumCollectionT<fp_type> >(*this));
	}
	///////////////////////////////////////////////////////////////////////

	// Make sure this class can only be instantiated if fp_type really is a floating point type
	BOOST_MPL_ASSERT((boost::is_floating_point<fp_type>));

public:
	/** @brief Specifies the type of parameters stored in this collection */
	typedef fp_type collection_type;

	/***************************************************************************/
	/**
	 * Initialize the lower and upper boundaries for data members of this class.
	 * Then set all positions to random values.
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
		: GConstrainedNumCollectionT<fp_type> (size, lowerBoundary, boost::math::float_prior<fp_type>(upperBoundary)) // Note that we define the upper boundary as "open"
	{
		// Assign random values to each position
		typename GConstrainedFPNumCollectionT<fp_type>::iterator it;
		for(it=this->begin(); it!=this->end(); ++it) {
			*it = this->GParameterBase::gr->Gem::Hap::GRandomBase::template uniform_real<fp_type>(lowerBoundary,upperBoundary);
		}
	}

	/***************************************************************************/
	/**
	 * Initialize the lower and upper boundaries for data members of this class.
	 * Set all positions to the same value. Note that we take the liberty to adapt val,
	 * if it is equal to the unmodified upper boundary. Otherwise you will get an
    * error, where what you likely really meant was to start with the
    * upper boundary.
	 *
	 * @param size The desired size of the collection
	 * @param val The value to be assigned to all positions
	 * @param lowerBoundary The lower boundary for data members
	 * @param upperBoundary The upper boundary for data members
	 */
	GConstrainedFPNumCollectionT (
      const std::size_t& size
      , const fp_type& val
      , const fp_type& lowerBoundary
      , const fp_type& upperBoundary
	)
		: GConstrainedNumCollectionT<fp_type> (
		      size
		      , (val==upperBoundary?boost::math::float_prior<fp_type>(val):val)
		      , lowerBoundary
		      , boost::math::float_prior<fp_type>(upperBoundary)
		  ) // Note that we define the upper boundary as "open"
	{ /* nothing */ }

	/***************************************************************************/
	/**
	 * The standard copy constructor
	 */
	GConstrainedFPNumCollectionT(const GConstrainedFPNumCollectionT<fp_type>& cp)
		: GConstrainedNumCollectionT<fp_type> (cp)
	{ /* nothing */ }

	/***************************************************************************/
	/**
	 * The standard destructor
	 */
	virtual ~GConstrainedFPNumCollectionT()
	{ /* nothing */ }

   /***************************************************************************/
   /**
    * The standard assignment operator
    */
   const GConstrainedFPNumCollectionT<fp_type>& operator=(
      const GConstrainedFPNumCollectionT<fp_type>& cp
   ) {
      this->load_(&cp);
      return *this;
   }

   /***************************************************************************/
   /**
    * Checks for equality with another GConstrainedFPNumCollectionT<fp_type> object
    *
    * @param  cp A constant reference to another GConstrainedFPNumCollectionT<fp_type> object
    * @return A boolean indicating whether both objects are equal
    */
   bool operator==(const GConstrainedFPNumCollectionT<fp_type>& cp) const {
      using namespace Gem::Common;
      // Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
      return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GConstrainedFPNumCollectionT<fp_type>::operator==","cp", CE_SILENT);
   }

   /***************************************************************************/
   /**
    * Checks for inequality with another GConstrainedFPNumCollectionT<fp_type> object
    *
    * @param  cp A constant reference to another GConstrainedFPNumCollectionT<fp_type> object
    * @return A boolean indicating whether both objects are inequal
    */
   bool operator!=(const GConstrainedFPNumCollectionT<fp_type>& cp) const {
      using namespace Gem::Common;
      // Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
      return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GConstrainedFPNumCollectionT<fp_type>::operator==","cp", CE_SILENT);
   }


	/***************************************************************************/
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
	virtual boost::optional<std::string> checkRelationshipWith(
      const GObject& cp
      , const Gem::Common::expectation& e
      , const double& limit
      , const std::string& caller
      , const std::string& y_name
      , const bool& withMessages
	) const OVERRIDE {
	    using namespace Gem::Common;

		// Check that we are indeed dealing with a GParamterBase reference
		const GConstrainedFPNumCollectionT<fp_type>  *p_load = GObject::gobject_conversion<GConstrainedFPNumCollectionT<fp_type> >(&cp);

		// Will hold possible deviations from the expectation, including explanations
	    std::vector<boost::optional<std::string> > deviations;

		// Check our parent class'es data ...
		deviations.push_back(GConstrainedNumCollectionT<fp_type>::checkRelationshipWith(cp, e, limit, "GConstrainedFPNumCollectionT<fp_type>", y_name, withMessages));

		// ... no local data

		return evaluateDiscrepancies("GConstrainedFPNumCollectionT<fp_type>", caller, deviations, e);
	}

	/****************************************************************************/
	/**
	 * The transfer function needed to calculate the externally visible value.
	 *
	 * @param val The value to which the transformation should be applied
	 * @return The transformed value
	 */
	virtual fp_type transfer(const fp_type& val) const OVERRIDE {
		fp_type lowerBoundary = GConstrainedNumCollectionT<fp_type>::getLowerBoundary();
		fp_type upperBoundary = GConstrainedNumCollectionT<fp_type>::getUpperBoundary();

		if(val >= lowerBoundary && val < upperBoundary) {
			return val;
		}
		else {
			// Find out which region the value is in (compare figure transferFunction.pdf
			// that should have been delivered with this software). Note that boost::numeric_cast<>
			// may throw - exceptions must be caught in surrounding functions.
			boost::int32_t region = 0;

#ifdef DEBUG
			region =	boost::numeric_cast<boost::int32_t>(Gem::Common::gfloor((fp_type(val) - fp_type(lowerBoundary)) / (fp_type(upperBoundary) - fp_type(lowerBoundary))));
#else
			region =	static_cast<boost::int32_t>(Gem::Common::gfloor((fp_type(val) - fp_type(lowerBoundary)) / (fp_type(upperBoundary) - fp_type(lowerBoundary))));
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

   /***************************************************************************/
   /**
    * Emits a name for this class / object
    */
   virtual std::string name() const OVERRIDE {
      return std::string("GConstrainedFPNumCollectionT");
   }

protected:
	/***************************************************************************/
	/**
	 * Loads the data of another GConstrainedFPNumCollectionT<fp_type> object,
	 * camouflaged as a GObject. We have no local data, so
	 * all we need to do is to the standard identity check,
	 * preventing that an object is assigned to itself.
	 *
	 * @param cp A copy of another GConstrainedFPNumCollectionT<fp_type> object, camouflaged as a GObject
	 */
	virtual void load_(const GObject *cp) OVERRIDE {
		// Convert cp into local format
		const GConstrainedFPNumCollectionT<fp_type> *p_load = GObject::gobject_conversion<GConstrainedFPNumCollectionT<fp_type> >(cp);

		// Load our parent class'es data ...
		GConstrainedNumCollectionT<fp_type>::load_(cp);

		// ... no local data
	}

	/***************************************************************************/
	/** @brief Creates a deep copy of this object */
	virtual GObject *clone_() const = 0;

	/***************************************************************************/
	/**
	 * Triggers random initialization of the parameter collection
	 */
	virtual void randomInit_(const activityMode&) OVERRIDE {
		for(std::size_t pos=0; pos<this->size(); pos++) {
			this->setValue(
				pos
				, this->GParameterBase::gr->Gem::Hap::GRandomBase::template uniform_real<fp_type>(
						GConstrainedNumCollectionT<fp_type>::getLowerBoundary()
						, GConstrainedNumCollectionT<fp_type>::getUpperBoundary()
				)
			);
		}
	}

	/***************************************************************************/
	/**
	 * The default constructor. Intentionally protected, as it is only
	 * needed for de-serialization and as the basis for derived class'es
	 * default constructors.
	 */
	GConstrainedFPNumCollectionT()
		: GConstrainedNumCollectionT<fp_type> ()
	{ /* nothing */ }

public:
	/***************************************************************************/
	/**
	 * Applies modifications to this object. This is needed for testing purposes
	 *
	 * @return A boolean which indicates whether modifications were made
	 */
	virtual bool modify_GUnitTests() OVERRIDE {
#ifdef GEM_TESTING
		bool result = false;

		// Call the parent classes' functions
		if(GConstrainedNumCollectionT<fp_type>::modify_GUnitTests()) result = true;

		return result;
#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GConstrainedFPNumCollectionT<>::modify_GUnitTests", "GEM_TESTING");
   return false;
#endif /* GEM_TESTING */
	}

	/***************************************************************************/
	/**
	 * Performs self tests that are expected to succeed. This is needed for testing purposes
	 */
	virtual void specificTestsNoFailureExpected_GUnitTests() OVERRIDE {
#ifdef GEM_TESTING
		// Call the parent classes' functions
		GConstrainedNumCollectionT<fp_type>::specificTestsNoFailureExpected_GUnitTests();
#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GConstrainedFPNumCollectionT<>::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
	}

	/***************************************************************************/
	/**
	 * Performs self tests that are expected to fail. This is needed for testing purposes
	 */
	virtual void specificTestsFailuresExpected_GUnitTests() OVERRIDE {
#ifdef GEM_TESTING
		// Call the parent classes' functions
		GConstrainedNumCollectionT<fp_type>::specificTestsFailuresExpected_GUnitTests();
#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GConstrainedFPNumCollectionT<>::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
	}
};

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

/******************************************************************************/
// The content of BOOST_SERIALIZATION_ASSUME_ABSTRACT(T)

namespace boost {
	namespace serialization {
		template<typename fp_type>
		struct is_abstract<Gem::Geneva::GConstrainedFPNumCollectionT<fp_type> > : public boost::true_type {};
		template<typename fp_type>
		struct is_abstract< const Gem::Geneva::GConstrainedFPNumCollectionT<fp_type> > : public boost::true_type {};
	}
}
/******************************************************************************/

#endif /* GCONSTRAINEDFPNUMCOLLECTIONT_HPP_ */
