/**
 * @file GMutableSetT.hpp
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

// Boost header files go here

#ifndef GMUTABLESETT_HPP_
#define GMUTABLESETT_HPP_

// Geneva headers go here
#include "hap/GRandomT.hpp"
#include "common/GHelperFunctionsT.hpp"
#include "common/GExceptions.hpp"
#include "geneva/GObject.hpp"
#include "geneva/GOptimizableEntity.hpp"
#include "geneva/GParameterBase.hpp"
#include "geneva/GStdPtrVectorInterfaceT.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * This class forms the basis for many user-defined individuals. It acts as a collection
 * of different parameter sets. User individuals can thus contain a mix of parameters of
 * different types. Derived classes must implement a useful operator=(). It is also assumed
 * that template arguments have the GObject and the GMutableI interfaces.
 */
template <typename T>
class GMutableSetT:
	public GOptimizableEntity,
	public GStdPtrVectorInterfaceT<T>
{
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template<typename Archive>
    G_API_GENEVA void serialize(Archive & ar, const unsigned int){
      using boost::serialization::make_nvp;

      ar
      & BOOST_SERIALIZATION_BASE_OBJECT_NVP(GOptimizableEntity)
      & make_nvp("GStdPtrVectorInterfaceT_T", boost::serialization::base_object<GStdPtrVectorInterfaceT<T> >(*this));
    }
    ///////////////////////////////////////////////////////////////////////

public:
	/***************************************************************************/
	/**
	 * The default constructor. No local data, hence nothing to do.
	 */
    G_API_GENEVA GMutableSetT()
		: GOptimizableEntity()
		, GStdPtrVectorInterfaceT<T>()
	{ /* nothing */	}

   /***************************************************************************/
   /**
    * Initialization with the number of fitness criteria
    *
    * @param nFitnessCriteria The number of fitness criteria used by this object
    */
    G_API_GENEVA GMutableSetT(const std::size_t& nFitnessCriteria)
      : GOptimizableEntity(nFitnessCriteria)
      , GStdPtrVectorInterfaceT<T>()
   { /* nothing */   }

	/***************************************************************************/
	/**
	 * The copy constructor. Note that we cannot rely on the operator=() of the vector
	 * here, as we do not know the actual type of T objects.
	 *
	 * @param cp A copy of another GMutableSetT<T> object
	 */
    G_API_GENEVA GMutableSetT(const GMutableSetT<T>& cp)
		: GOptimizableEntity(cp)
		, GStdPtrVectorInterfaceT<T>(cp)
	{ /* nothing */ }

	/***************************************************************************/
	/**
	 * The destructor. As we use smart pointers, we do not need to take care of the
	 * data in the vector ourselves.
	 */
	virtual G_API_GENEVA ~GMutableSetT()
	{ /* nothing */ }

	/***************************************************************************/
	/**
	 * Checks for equality with another GNumCollectionT<T> object
	 *
	 * @param  cp A constant reference to another GMutableSetT<T> object
	 * @return A boolean indicating whether both objects are equal
	 */
	G_API_GENEVA bool operator==(const GMutableSetT<T>& cp) const {
		using namespace Gem::Common;
		// Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
		return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GMutableSetT<T>::operator==","cp", CE_SILENT);
	}

	/***************************************************************************/
	/**
	 * Checks for inequality with another GMutableSetT<T> object
	 *
	 * @param  cp A constant reference to another GMutableSetT<T> object
	 * @return A boolean indicating whether both objects are inequal
	 */
	G_API_GENEVA bool operator!=(const GMutableSetT<T>& cp) const {
		using namespace Gem::Common;
		// Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
		return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GMutableSetT<T>::operator!=","cp", CE_SILENT);
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
	G_API_GENEVA boost::optional<std::string> checkRelationshipWith(
      const GObject& cp
      , const Gem::Common::expectation& e
      , const double& limit
      , const std::string& caller
      , const std::string& y_name
      , const bool& withMessages
	) const OVERRIDE {
	    using namespace Gem::Common;

		// Check that we are indeed dealing with a GParamterBase reference
		const GMutableSetT<T> *p_load = GObject::gobject_conversion<GMutableSetT<T> >(&cp);

		// Will hold possible deviations from the expectation, including explanations
	    std::vector<boost::optional<std::string> > deviations;

		// Check our parent class'es data ...
		deviations.push_back(GOptimizableEntity::checkRelationshipWith(cp, e, limit, "GMutableSetT<T>", y_name, withMessages));
		deviations.push_back(GStdPtrVectorInterfaceT<T>::checkRelationshipWith(*p_load, e, limit, "GMutableSetT<T>", y_name, withMessages));

		// no local data ...

		return evaluateDiscrepancies("GMutableSetT<T>", caller, deviations, e);
	}

	/***************************************************************************/
	/**
	 * Swap another object's vector with ours. We need to set the dirty flag of both
	 * individuals in this case.
	 */
	inline G_API_GENEVA void swap(GMutableSetT<T>& cp) {
		GStdPtrVectorInterfaceT<T>::swap(cp.data);
		GOptimizableEntity::setDirtyFlag();
		cp.setDirtyFlag();
	}

	/* ----------------------------------------------------------------------------------
	 * Tested in GTestIndividual1::specificTestsNoFailureExpected_GUnitTests()
	 * ----------------------------------------------------------------------------------
	 */

	/***************************************************************************/
	/**
	 * Adds local configuration options to a GParserBuilder object
	 *
	 * @param gpb The GParserBuilder object to which configuration options should be added
	 */
	virtual G_API_GENEVA void addConfigurationOptions (
		Gem::Common::GParserBuilder& gpb
	) OVERRIDE {
		// Call our parent class'es function
		GOptimizableEntity::addConfigurationOptions(gpb);

		// No local data
	}

   /***************************************************************************/
   /**
    * Emits a name for this class / object
    */
   virtual G_API_GENEVA std::string name() const OVERRIDE {
      return std::string("GMutableSetT");
   }

protected:
	/***************************************************************************/
   /**
   * A random number generator. Note that the actual calculation is possibly
   * done in a random number server, depending on the defines you have chosen.
   */
#ifdef GEM_GENEVA_USE_LOCAL_RANDOM_ADAPTION /* produce random numbers locally */
	Gem::Hap::GRandomT<Gem::Hap::RANDOMLOCAL> gr;
#else /* act as a proxy, take random numbers from a factory */
	Gem::Hap::GRandomT<Gem::Hap::RANDOMPROXY> gr;
#endif /* USEPROXYRANDOM */

	/***************************************************************************/
	/**
	 * Loads the data of another GParameterBase object, camouflaged as a GObject
	 *
	 * @param cp A copy of another GMutableSetT object, camouflaged as a GObject
	 */
	virtual G_API_GENEVA void load_(const GObject* cp) OVERRIDE {
		// Convert cp into local format
	  const GMutableSetT<T> *p_load = this->template gobject_conversion<GMutableSetT<T> >(cp);

	  // No local data - load the parent class'es data
	  GOptimizableEntity::load_(cp);
	  GStdPtrVectorInterfaceT<T>::operator=(*p_load);
	}

	/***************************************************************************/
	/**
	 * Re-implementation of a corresponding function in GStdPtrVectorInterface.
	 * Make the vector wrapper purely virtual allows the compiler to perform
	 * further optimizations.
	 */
	virtual G_API_GENEVA void dummyFunction() OVERRIDE { /* nothing */ }

public:
	/***************************************************************************/
	/**
	 * Applies modifications to this object. This is needed for testing purposes
	 *
	 * @return A boolean which indicates whether modifications were made
	 */
	virtual G_API_GENEVA bool modify_GUnitTests() OVERRIDE {
#ifdef GEM_TESTING
		using boost::unit_test_framework::test_suite;
		using boost::unit_test_framework::test_case;

		bool result = false;

		// Call the parent classes' functions
		if(GOptimizableEntity::modify_GUnitTests()) result = true;
		if(GStdPtrVectorInterfaceT<T>::modify_GUnitTests()) result = true;

		// Try to change the objects contained in the collection
		typename GMutableSetT<T>::iterator it;
		for(it=this->begin(); it!=this->end(); ++it) {
			if((*it)->modify_GUnitTests()) result = true;
		}

		return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GMutableSetT<>::modify_GUnitTests", "GEM_TESTING");
   return false;
#endif /* GEM_TESTING */
	}

	/***************************************************************************/
	/**
	 * Performs self tests that are expected to succeed. This is needed for testing purposes
	 */
	virtual G_API_GENEVA void specificTestsNoFailureExpected_GUnitTests() OVERRIDE {
#ifdef GEM_TESTING
		using boost::unit_test_framework::test_suite;
		using boost::unit_test_framework::test_case;

		// Call the parent classes' functions
		GOptimizableEntity::specificTestsNoFailureExpected_GUnitTests();
		GStdPtrVectorInterfaceT<T>::specificTestsNoFailureExpected_GUnitTests();

		// no local data, nothing to test

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GMutableSetT<>::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
	}

	/***************************************************************************/
	/**
	 * Performs self tests that are expected to fail. This is needed for testing purposes
	 */
	virtual G_API_GENEVA void specificTestsFailuresExpected_GUnitTests() OVERRIDE {
#ifdef GEM_TESTING
		using boost::unit_test_framework::test_suite;
		using boost::unit_test_framework::test_case;

		// Call the parent classes' functions
		GOptimizableEntity::specificTestsFailuresExpected_GUnitTests();
		GStdPtrVectorInterfaceT<T>::specificTestsFailuresExpected_GUnitTests();

		// no local data, nothing to test

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GMutableSetT<>::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
	}

	/***************************************************************************/
};

} /* namespace Geneva */
} /* namespace Gem */

/******************************************************************************/
/**
 * @brief The content of the BOOST_SERIALIZATION_ASSUME_ABSTRACT(T) macro. Needed for Boost.Serialization
 */
namespace boost {
  namespace serialization {
    template<typename T>
    struct is_abstract<Gem::Geneva::GMutableSetT<T> > : public boost::true_type {};
    template<typename T>
    struct is_abstract< const Gem::Geneva::GMutableSetT<T> > : public boost::true_type {};
  }
}

/******************************************************************************/

#endif /* GMUTABLESETT_HPP_ */
