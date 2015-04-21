/**
 * @file GIntFlipAdaptorT.hpp
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

#ifndef GINTFLIPADAPTORT_HPP_
#define GINTFLIPADAPTORT_HPP_

// Geneva headers go here
#include "common/GExceptions.hpp"
#include "geneva/GNumFlipAdaptorT.hpp"
#include "geneva/GConstrainedDoubleObject.hpp"
#include "geneva/GDoubleGaussAdaptor.hpp"
#include "geneva/GObject.hpp"
#include "geneva/GOptimizationEnums.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * GIntFlipAdaptorT represents an adaptor used for the adaption of integer
 * types, by flipping an integer number to the next larger or smaller number.
 * The integer type used needs to be specified as a template parameter. Note
 * that a specialization of this class, as defined in GIntFlipAdaptorT.cpp,
 * allows to deal with booleans instead of "standard" integer types.
 */
template<typename int_type>
class GIntFlipAdaptorT
	:public GNumFlipAdaptorT<int_type>
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int) {
		using boost::serialization::make_nvp;
		ar
		& make_nvp("GNumFlipAdaptorT", boost::serialization::base_object<GNumFlipAdaptorT<int_type> >(*this));
	}
	///////////////////////////////////////////////////////////////////////

public:
	/***************************************************************************/
	/**
	 * The standard constructor.
	 */
	GIntFlipAdaptorT()
		: GNumFlipAdaptorT<int_type> ()
	{ /* nothing */	}

	/***************************************************************************/
	/**
	 * This constructor takes an argument, that specifies the (initial) probability
	 * for the adaption of an integer or bit value
	 *
	 * @param prob The probability for a flip
	 */
	explicit GIntFlipAdaptorT(const double& prob)
		: GNumFlipAdaptorT<int_type>(prob)
	{ /* nothing */ }

	/***************************************************************************/
	/**
	 * A standard copy constructor.
	 *
	 * @param cp Another GIntFlipAdaptorT object
	 */
	GIntFlipAdaptorT(const GIntFlipAdaptorT<int_type>& cp)
		: GNumFlipAdaptorT<int_type>(cp)
	{ /* nothing */	}

	/***************************************************************************/
	/**
	 * The standard destructor. Empty, as we have no local, dynamically
	 * allocated data.
	 */
	virtual ~GIntFlipAdaptorT()
	{ /* nothing */ }

   /***************************************************************************/
   /**
    * The standard assignment operator
    */
   const GIntFlipAdaptorT<int_type>& operator=(const GIntFlipAdaptorT<int_type>& cp) {
      this->load_(&cp);
      return *this;
   }

   /***************************************************************************/
   /**
    * Checks for equality with another GIntFlipAdaptorT<int_type> object
    *
    * @param  cp A constant reference to another GIntFlipAdaptorT<int_type> object
    * @return A boolean indicating whether both objects are equal
    */
   bool operator==(const GIntFlipAdaptorT<int_type>& cp) const {
      using namespace Gem::Common;
      try {
         this->compare(cp, CE_EQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
         return true;
      } catch(g_expectation_violation&) {
         return false;
      }
   }

   /***************************************************************************/
   /**
    * Checks for inequality with another GIntFlipAdaptorT<int_type> object
    *
    * @param  cp A constant reference to another GIntFlipAdaptorT<int_type> object
    * @return A boolean indicating whether both objects are inequal
    */
   bool operator!=(const GIntFlipAdaptorT<int_type>& cp) const {
      using namespace Gem::Common;
      try {
         this->compare(cp, CE_INEQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
         return true;
      } catch(g_expectation_violation&) {
         return false;
      }
   }

   /***************************************************************************/
   /**
    * Searches for compliance with expectations with respect to another object
    * of the same type
    *
    * @param cp A constant reference to another GObject object
    * @param e The expected outcome of the comparison
    * @param limit The maximum deviation for floating point values (important for similarity checks)
    */
   virtual void compare(
      const GObject& cp
      , const Gem::Common::expectation& e
      , const double& limit
   ) const override {
      using namespace Gem::Common;

      // Check that we are indeed dealing with a GAdaptorT reference
      const GIntFlipAdaptorT<int_type>  *p_load = GObject::gobject_conversion<GIntFlipAdaptorT<int_type> >(&cp);

      GToken token("GIntFlipAdaptorT<int_type>", e);

      // Compare our parent data ...
      Gem::Common::compare_base<GNumFlipAdaptorT<int_type> >(IDENTITY(*this, *p_load), token);

      //... no local data

      // React on deviations from the expectation
      token.evaluate();
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
   virtual std::string name() const override {
      return std::string("GIntFlipAdaptorT");
   }

   /***************************************************************************/
   /**
    * Allows to randomly initialize parameter members. No local data, hence no
    * action taken.
    */
   virtual bool randomInit() override {
      return false;
   }

protected:
	/***************************************************************************/
	/**
	 * This function loads the data of another GIntFlipAdaptorT, camouflaged as a GObject.
	 *
	 * @param A copy of another GIntFlipAdaptorT, camouflaged as a GObject
	 */
   void load_(const GObject *cp) override	{
		// Check that this object is not accidently assigned to itself
		GObject::selfAssignmentCheck<GIntFlipAdaptorT<int_type> >(cp);

		// Load the data of our parent class ...
		GNumFlipAdaptorT<int_type>::load_(cp);

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

public:
	/***************************************************************************/
	/**
	 * Applies modifications to this object. This is needed for testing purposes
	 *
	 * @return A boolean which indicates whether modifications were made
	 */
	virtual bool modify_GUnitTests() override {
#ifdef GEM_TESTING
      using boost::unit_test_framework::test_suite;
		using boost::unit_test_framework::test_case;

		bool result = false;

		// Call the parent classes' functions
		if(GNumFlipAdaptorT<int_type>::modify_GUnitTests()) result = true;

		// no local data -- nothing to change

		return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		condnotset("GIntFlipAdaptorT<>::modify_GUnitTests", "GEM_TESTING");
		return false;
#endif /* GEM_TESTING */
	}

	/***************************************************************************/
	/**
	 * Performs self tests that are expected to succeed. This is needed for testing purposes
	 */
	virtual void specificTestsNoFailureExpected_GUnitTests() override {
#ifdef GEM_TESTING
		using boost::unit_test_framework::test_suite;
		using boost::unit_test_framework::test_case;

		// Call the parent classes' functions
		GNumFlipAdaptorT<int_type>::specificTestsNoFailureExpected_GUnitTests();
#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		condnotset("GIntFlipAdaptorT<>::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
	}

	/***************************************************************************/
	/**
	 * Performs self tests that are expected to fail. This is needed for testing purposes
	 */
	virtual void specificTestsFailuresExpected_GUnitTests() override {
#ifdef GEM_TESTING
		using boost::unit_test_framework::test_suite;
		using boost::unit_test_framework::test_case;

		// Call the parent classes' functions
		GNumFlipAdaptorT<int_type>::specificTestsFailuresExpected_GUnitTests();

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		condnotset("GIntFlipAdaptorT<>::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
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
		template<typename int_type>
		struct is_abstract<Gem::Geneva::GIntFlipAdaptorT<int_type> > : public boost::true_type {};
		template<typename int_type>
		struct is_abstract< const Gem::Geneva::GIntFlipAdaptorT<int_type> > : public boost::true_type {};
	}
}

/******************************************************************************/

#endif /* GINTFLIPADAPTORT_HPP_ */
