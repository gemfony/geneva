/**
 * @file GNumIntT.hpp
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

#ifndef GNUMINTT_HPP_
#define GNUMINTT_HPP_

// Geneva headers go here

#include "geneva/GNumT.hpp"
#include "geneva/GInt32GaussAdaptor.hpp"
#include "geneva/GInt32FlipAdaptor.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * This class encapsulates a single integer value, which can assume different integer types.
 * The reason for this class is that there might be applications where one might want different
 * adaptor characteristics for different values. This cannot be done with a GIntCollectionT.
 */
template <typename int_type>
class GNumIntT
	:public GNumT<int_type>
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int){
	  using boost::serialization::make_nvp;

	  ar & make_nvp("GNumT", boost::serialization::base_object<GNumT<int_type> >(*this));
	}
	///////////////////////////////////////////////////////////////////////

	// Make sure this class can only be instantiated if int_type is a *signed* integer type
	BOOST_MPL_ASSERT((boost::is_signed<int_type>));

public:
	/** @brief Specifies the type of parameters stored in this object */
	typedef int_type parameter_type;

	/***************************************************************************/
	/**
	 * The default constructor
	 */
	GNumIntT()
		: GNumT<int_type> ()
	{ /* nothing */ }

	/***************************************************************************/
	/**
	 * The copy constructor
	 *
	 * @param cp A constant reference to another GNumIntT<int_type> object
	 */
	GNumIntT(const GNumIntT<int_type>& cp)
		: GNumT<int_type> (cp)
	{ /* nothing */ }

	/***************************************************************************/
	/**
	 * Initialization by contained value
	 *
	 * @param val The value used for the initialization
	 */
	explicit GNumIntT(const int_type& val)
		: GNumT<int_type>(val)
	{ /* nothing */ }

	/***************************************************************************/
	/**
	 * Initialization by random number in a given range. Note that we
	 * use the local randomInit_ function in order to avoid trying to
	 * call any purely virtual functions from the constructor.
	 *
	 * @param min The lower boundary for random entries
	 * @param max The upper boundary for random entries
	 */
	GNumIntT(
      const int_type& min
      , const int_type& max
	)
		: GNumT<int_type> (min, max)
	{
		GNumIntT<int_type>::randomInit(ACTIVEONLY);
	}

	/***************************************************************************/
	/**
	 * Initialization with a fixed value, plus the boundaries for random
	 * initialization.
	 *
	 * @param val The value to be assigned to the object
	 * @param min The lower boundary for random entries
	 * @param max The upper boundary for random entries
	 */
	GNumIntT(
      const int_type& val
      , const int_type& min
      , const int_type& max
	)
		: GNumT<int_type> (min, max)
	{
		GParameterT<int_type>::setValue(val);
	}

	/***************************************************************************/
	/**
	 * The destructor
	 */
	virtual ~GNumIntT()
	{ /* nothing */ }

   /***************************************************************************/
   /**
    * The standard assignment operator
    */
   const GNumIntT<int_type>& operator=(const GNumIntT<int_type>& cp) {
      this->load_(&cp);
      return *this;
   }

	/***************************************************************************/
	/**
	 * An assignment operator for the contained value type
	 *
	 * @param val The value to be assigned to this object
	 * @return The value that was assigned to this object
	 */
	virtual int_type operator=(const int_type& val) {
		return GNumT<int_type>::operator=(val);
	}

   /***************************************************************************/
   /**
    * Checks for equality with another GNumIntT<int_type> object
    *
    * @param  cp A constant reference to another GNumIntT<int_type> object
    * @return A boolean indicating whether both objects are equal
    */
   bool operator==(const GNumIntT<int_type>& cp) const {
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
    * Checks for inequality with another GNumIntT<int_type> object
    *
    * @param  cp A constant reference to another GNumIntT<int_type> object
    * @return A boolean indicating whether both objects are inequal
    */
   bool operator!=(const GNumIntT<int_type>& cp) const {
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
      const GNumIntT<int_type>  *p_load = GObject::gobject_conversion<GNumIntT<int_type> >(&cp);

      GToken token("GNumIntT<int_type>", e);

      // Compare our parent data ...
      Gem::Common::compare_base<GNumT<int_type> >(IDENTITY(*this, *p_load), token);

      // ... no local data

      // React on deviations from the expectation
      token.evaluate();
   }

   /***************************************************************************/
   /**
    * Emits a name for this class / object
    */
   virtual std::string name() const override {
      return std::string("GNumIntT");
   }

protected:
	/***************************************************************************/
	/**
	 * Loads the data of another GNumIntT<int_type> object,
	 * camouflaged as a GObject. We have no local data, so
	 * all we need to do is to the standard identity check,
	 * preventing that an object is assigned to itself.
	 *
	 * @param cp A copy of another GNumIntT<int_type> object, camouflaged as a GObject
	 */
	virtual void load_(const GObject *cp) override {
		// Convert cp into local format
		const GNumIntT<int_type> *p_load = GObject::gobject_conversion<GNumIntT<int_type> >(cp);

		// Load our parent class'es data ...
		GNumT<int_type>::load_(cp);

		// no local data ...
	}

	/***************************************************************************/
	/** @brief Creates a deep clone of this object. Needs to be redefined in derived classes */
	virtual GObject* clone_() const = 0;

	/***************************************************************************/
	/**
	 * Triggers random initialization of the parameter collection
	 */
	virtual bool randomInit_(const activityMode&) override {
		int_type lowerBoundary = GNumT<int_type>::getLowerInitBoundary();
		int_type upperBoundary = GNumT<int_type>::getUpperInitBoundary();

		// uniform_int produces random numbers that include the upper boundary.
		GParameterT<int_type>::setValue(this->GObject::gr_ptr()->uniform_int(lowerBoundary, upperBoundary));

		return true;
	}

	/* ----------------------------------------------------------------------------------
	 * Tested in GInt32Object::specificTestsNoFailuresExpected_GUnitTests()
	 * ----------------------------------------------------------------------------------
	 */

public:
	/***************************************************************************/
	/**
	 * Applies modifications to this object. This is needed for testing purposes
	 */
	virtual bool modify_GUnitTests() override {
#ifdef GEM_TESTING
		bool result = false;

		// Call the parent classes' functions
		if(GNumT<int_type>::modify_GUnitTests()) result = true;

		return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		condnotset("GNumIntT<>::modify_GUnitTests", "GEM_TESTING");
		return false;
#endif /* GEM_TESTING */
	}

	/***************************************************************************/
	/**
	 * Performs self tests that are expected to succeed. This is needed for testing purposes
	 */
	virtual void specificTestsNoFailureExpected_GUnitTests() override {
#ifdef GEM_TESTING
		// A few settings
		const std::size_t nTests = 10000;
		const int_type LOWERINITBOUNDARY = int_type(0); // >= 0, as int_type might be unsigned
		const int_type UPPERINITBOUNDARY =  int_type(10);
		const int_type FIXEDVALUEINIT = int_type(1);

		// Call the parent classes' functions
		GNumT<int_type>::specificTestsNoFailureExpected_GUnitTests();

		//------------------------------------------------------------------------------

		{ // Initialize with a fixed value, then check setting and retrieval of boundaries and random initialization
			std::shared_ptr<GNumIntT<int_type> > p_test1 = this->GObject::clone<GNumIntT<int_type> >();
			std::shared_ptr<GNumIntT<int_type> > p_test2 = this->GObject::clone<GNumIntT<int_type> >();

			// Assign a boolean value true
			BOOST_CHECK_NO_THROW(*p_test1 = 2*UPPERINITBOUNDARY); // Make sure random initialization cannot randomly result in an unchanged value
			// Cross-check
			BOOST_CHECK(p_test1->value() == 2*UPPERINITBOUNDARY);

			// Set initialization boundaries
			BOOST_CHECK_NO_THROW(p_test1->setInitBoundaries(LOWERINITBOUNDARY, UPPERINITBOUNDARY));

			// Check that the boundaries have been set as expected
			BOOST_CHECK(p_test1->getLowerInitBoundary() == LOWERINITBOUNDARY);
			BOOST_CHECK(p_test1->getUpperInitBoundary() == UPPERINITBOUNDARY);

			// Load the data of p_test1 into p_test2
			BOOST_CHECK_NO_THROW(p_test2->load(p_test1));
			// Cross check that both are indeed equal
			BOOST_CHECK(*p_test1 == *p_test2);

			// Check that the values of p_test1 are inside of the allowed boundaries
			for(std::size_t i=0; i<nTests; i++) {
				BOOST_CHECK_NO_THROW(p_test1->randomInit_(ALLPARAMETERS));
				BOOST_CHECK(p_test1->value() >= LOWERINITBOUNDARY);
				BOOST_CHECK(p_test1->value() <= UPPERINITBOUNDARY);
				BOOST_CHECK(p_test1->value() != p_test2->value());
			}
		}

		//------------------------------------------------------------------------------

		{ // Check that the fp-family of functions doesn't have an effect on this object
			std::shared_ptr<GNumIntT<int_type> > p_test1 = this->GObject::clone<GNumIntT<int_type> >();
			std::shared_ptr<GNumIntT<int_type> > p_test2 = this->GObject::clone<GNumIntT<int_type> >();
			std::shared_ptr<GNumIntT<int_type> > p_test3 = this->GObject::clone<GNumIntT<int_type> >();

			// Assign a boolean value true
			BOOST_CHECK_NO_THROW(*p_test1 = FIXEDVALUEINIT); // Make sure random initialization cannot randomly result in an unchanged value
			// Cross-check
			BOOST_CHECK(p_test1->value() == FIXEDVALUEINIT);

			// Load into p_test2 and p_test3 and test equality
			BOOST_CHECK_NO_THROW(p_test2->load(p_test1));
			BOOST_CHECK_NO_THROW(p_test3->load(p_test1));
			BOOST_CHECK(*p_test2 == *p_test1);
			BOOST_CHECK(*p_test3 == *p_test1);
			BOOST_CHECK(*p_test3 == *p_test2);

			// Check that initialization with a fixed floating point value has no effect on this object
			BOOST_CHECK_NO_THROW(p_test2->GParameterBase::template fixedValueInit<double>(2., ALLPARAMETERS));
			BOOST_CHECK(*p_test2 == *p_test1);

			// Check that multiplication with a fixed floating point value has no effect on this object
			BOOST_CHECK_NO_THROW(p_test2->GParameterBase::template multiplyBy<double>(2., ALLPARAMETERS));
			BOOST_CHECK(*p_test2 == *p_test1);

			// Check that a component-wise multiplication with a random fp value in a given range does not have an effect on this object
			BOOST_CHECK_NO_THROW(p_test2->GParameterBase::template multiplyByRandom<double>(1., 2., ALLPARAMETERS));
			BOOST_CHECK(*p_test2 == *p_test1);

			// Check that a component-wise multiplication with a random fp value in the range [0:1[ does not have an effect on this object
			BOOST_CHECK_NO_THROW(p_test2->GParameterBase::template multiplyByRandom<double>(ALLPARAMETERS));
			BOOST_CHECK(*p_test2 == *p_test1);

			// Check that adding p_test1 to p_test3 does not have an effect
			BOOST_CHECK_NO_THROW(p_test3->GParameterBase::template add<double>(p_test1, ALLPARAMETERS));
			BOOST_CHECK(*p_test3 == *p_test2);

			// Check that subtracting p_test1 from p_test3 does not have an effect
			BOOST_CHECK_NO_THROW(p_test3->GParameterBase::template subtract<double>(p_test1, ALLPARAMETERS));
			BOOST_CHECK(*p_test3 == *p_test2);
		}

		//------------------------------------------------------------------------------

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
      condnotset("GNumIntT<>::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
	}

	/***************************************************************************/
	/**
	 * Performs self tests that are expected to fail. This is needed for testing purposes
	 */
	virtual void specificTestsFailuresExpected_GUnitTests() override {
#ifdef GEM_TESTING
		// Call the parent classes' functions
		GNumT<int_type>::specificTestsFailuresExpected_GUnitTests();

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
      condnotset("GNumIntT<>::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
	}

	/***************************************************************************/

};

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

/******************************************************************************/
// The content of BOOST_SERIALIZATION_ASSUME_ABSTRACT(T)

namespace boost {
	namespace serialization {
		template<typename int_type>
		struct is_abstract<Gem::Geneva::GNumIntT<int_type> > : public boost::true_type {};
		template<typename int_type>
		struct is_abstract< const Gem::Geneva::GNumIntT<int_type> > : public boost::true_type {};
	}
}

/******************************************************************************/

#endif /* GNUMINTT_HPP_ */
