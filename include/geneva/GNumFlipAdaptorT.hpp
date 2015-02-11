/**
 * @file GNumFlipAdaptorT.hpp
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

// Standard headers go here

// Boost headers go here

#ifndef GNUMFLIPADAPTORT_HPP_
#define GNUMFLIPADAPTORT_HPP_

// Geneva headers go here
#include "common/GExceptions.hpp"
#include "geneva/GAdaptorT.hpp"
#include "geneva/GConstrainedDoubleObject.hpp"
#include "geneva/GDoubleGaussAdaptor.hpp"
#include "geneva/GObject.hpp"
#include "geneva/GOptimizationEnums.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * GNumFlipAdaptorT represents an adaptor used for the adaption of numeric
 * types, by flipping a number to the next larger or smaller one. The unerlying
 * type needs to be specified as a template parameter.
 */
template<typename num_type>
class G_API GNumFlipAdaptorT
	:public GAdaptorT<num_type>
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int) {
		using boost::serialization::make_nvp;
		ar & make_nvp("GAdaptorT", boost::serialization::base_object<GAdaptorT<num_type> >(*this));
	}
	///////////////////////////////////////////////////////////////////////

public:
	/***************************************************************************/
	/**
	 * The standard constructor.
	 */
	GNumFlipAdaptorT()
		: GAdaptorT<num_type> (DEFAULTADPROB)
	{ /* nothing */	}

	/***************************************************************************/
	/**
	 * This constructor takes an argument, that specifies the (initial) probability
	 * for the adaption of an integer or bit value
	 *
	 * @param prob The probability for a flip
	 */
	explicit GNumFlipAdaptorT(const double& prob)
		: GAdaptorT<num_type>(prob)
	{ /* nothing */ }

	/***************************************************************************/
	/**
	 * A standard copy constructor.
	 *
	 * @param cp Another GNumFlipAdaptorT object
	 */
	GNumFlipAdaptorT(const GNumFlipAdaptorT<num_type>& cp)
		: GAdaptorT<num_type>(cp)
	{ /* nothing */	}

	/***************************************************************************/
	/**
	 * The standard destructor. Empty, as we have no local, dynamically
	 * allocated data.
	 */
	virtual ~GNumFlipAdaptorT()
	{ /* nothing */ }

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
	boost::optional<std::string> checkRelationshipWith(
      const GObject& cp
      , const Gem::Common::expectation& e
      , const double& limit
      , const std::string& caller
      , const std::string& y_name
      , const bool& withMessages
	) const OVERRIDE {
	    using namespace Gem::Common;

		// Check that we are indeed dealing with a GParamterBase reference
		const GNumFlipAdaptorT<num_type>  *p_load = GObject::gobject_conversion<GNumFlipAdaptorT<num_type> >(&cp);

		// Will hold possible deviations from the expectation, including explanations
	    std::vector<boost::optional<std::string> > deviations;

		// Check our parent class'es data ...
		deviations.push_back(GAdaptorT<num_type>::checkRelationshipWith(cp, e, limit, "GNumFlipAdaptorT<num_type>", y_name, withMessages));

		// no local data ...

		return evaluateDiscrepancies("GNumFlipAdaptorT<num_type>", caller, deviations, e);
	}

	/***************************************************************************/
	/**
	 * Retrieves the id of the adaptor. Purely virtual, as we do not want this class to be
	 * instantiated.
	 *
	 * @return The id of the adaptor
	 */
	virtual Gem::Geneva::adaptorId getAdaptorId() const = 0;

	/* ----------------------------------------------------------------------------------
	 * Tested in GInt32FlipAdaptor::specificTestsNoFailuresExpected_GUnitTests()
	 * Tested in GBooleanAdaptor::specificTestsNoFailuresExpected_GUnitTests()
	 * ----------------------------------------------------------------------------------
	 */

   /***************************************************************************/
   /**
    * Emits a name for this class / object
    */
   virtual std::string name() const OVERRIDE {
      return std::string("GNumFlipAdaptorT");
   }

protected:
	/***************************************************************************/
	/**
	 * This function loads the data of another GNumFlipAdaptorT, camouflaged as a GObject.
	 *
	 * @param A copy of another GNumFlipAdaptorT, camouflaged as a GObject
	 */
	void load_(const GObject *cp) OVERRIDE	{
		// Check that this object is not accidently assigned to itself
		GObject::selfAssignmentCheck<GNumFlipAdaptorT<num_type> >(cp);

		// Load the data of our parent class ...
		GAdaptorT<num_type>::load_(cp);

		// no local data
	}

	/***************************************************************************/
	/**
	 * This function creates a deep copy of this object. The function is purely virtual, as we
	 * do not want this class to be instantiated. Use the derived classes instead.
	 *
	 * @return A deep copy of this object
	 */
	virtual GObject *clone_() const = 0;

	/***************************************************************************/
	/**
	 * We want to flip the value only in a given percentage of cases. Thus
	 * we calculate a probability between 0 and 1 and compare it with the desired
	 * adaption probability. Please note that evenRandom returns a value in the
	 * range of [0,1[, so we make a tiny error here.
	 *
	 * @param value The bit value to be adapted
	 * @param range A typical range for the parameter with type T (unused here)
	 */
	inline virtual void customAdaptions(
      num_type& value
      , const num_type& range
   ) OVERRIDE {
		bool up = this->gr->uniform_bool();
		if(up){
			value += 1;
		} else {
			value -= 1;
		}
	}

	/* ----------------------------------------------------------------------------------
	 * Tested in GAdaptorT<num_type>::specificTestsNoFailuresExpected_GUnitTests()
	 * ----------------------------------------------------------------------------------
	 */

public:
	/***************************************************************************/
	/**
	 * Applies modifications to this object. This is needed for testing purposes
	 *
	 * @return A boolean which indicates whether modifications were made
	 */
	virtual bool modify_GUnitTests() OVERRIDE {
#ifdef GEM_TESTING
		using boost::unit_test_framework::test_suite;
		using boost::unit_test_framework::test_case;

		bool result = false;

		// Call the parent classes' functions
		if(GAdaptorT<num_type>::modify_GUnitTests()) result = true;

		// no local data -- nothing to change

		return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		condnotset("GNumFlipAdaptorT<>::modify_GUnitTests", "GEM_TESTING");
		return false;
#endif /* GEM_TESTING */
	}

	/***************************************************************************/
	/**
	 * Performs self tests that are expected to succeed. This is needed for testing purposes
	 */
	virtual void specificTestsNoFailureExpected_GUnitTests() OVERRIDE {
#ifdef GEM_TESTING
		using boost::unit_test_framework::test_suite;
		using boost::unit_test_framework::test_case;

		// Call the parent classes' functions
		GAdaptorT<num_type>::specificTestsNoFailureExpected_GUnitTests();

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
      condnotset("GNumFlipAdaptorT<>::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
	}

	/***************************************************************************/
	/**
	 * Performs self tests that are expected to fail. This is needed for testing purposes
	 */
	virtual void specificTestsFailuresExpected_GUnitTests() OVERRIDE {
#ifdef GEM_TESTING
		using boost::unit_test_framework::test_suite;
		using boost::unit_test_framework::test_case;

		// Call the parent classes' functions
		GAdaptorT<num_type>::specificTestsFailuresExpected_GUnitTests();

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
      condnotset("GNumFlipAdaptorT<>::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
	}

   /***************************************************************************/
};

/******************************************************************************/
/**
 * Overload for num_type == bool . inline needed to satisfy MSVC restriction.
 */
template<>
inline void GNumFlipAdaptorT<bool>::customAdaptions(bool& value, const bool& range) {
   value==true?value=false:value=true;
}

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

/******************************************************************************/
// The content of BOOST_SERIALIZATION_ASSUME_ABSTRACT(T)

namespace boost {
	namespace serialization {
		template<typename num_type>
		struct is_abstract<Gem::Geneva::GNumFlipAdaptorT<num_type> > : public boost::true_type {};
		template<typename num_type>
		struct is_abstract< const Gem::Geneva::GNumFlipAdaptorT<num_type> > : public boost::true_type {};
	}
}

/******************************************************************************/

#endif /* GNUMFLIPADAPTORT_HPP_ */
