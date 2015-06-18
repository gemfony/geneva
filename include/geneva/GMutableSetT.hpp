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
#include "common/GStdPtrVectorInterfaceT.hpp"
#include "geneva/GObject.hpp"
#include "geneva/GOptimizableEntity.hpp"
#include "geneva/GParameterBase.hpp"

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
	public Gem::Common::GStdPtrVectorInterfaceT<T, Gem::Geneva::GObject>
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int){
		using boost::serialization::make_nvp;

		ar
		& BOOST_SERIALIZATION_BASE_OBJECT_NVP(GOptimizableEntity)
		& make_nvp("GStdPtrVectorInterfaceT_T", boost::serialization::base_object<Gem::Common::GStdPtrVectorInterfaceT<T, Gem::Geneva::GObject> >(*this));
	}
	///////////////////////////////////////////////////////////////////////

public:
	/***************************************************************************/
	/**
	 * The default constructor. No local data, hence nothing to do.
	 */
	GMutableSetT()
		: GOptimizableEntity()
		, Gem::Common::GStdPtrVectorInterfaceT<T, Gem::Geneva::GObject>()
	{ /* nothing */	}

	/***************************************************************************/
	/**
	 * Initialization with the number of fitness criteria
	 *
	 * @param nFitnessCriteria The number of fitness criteria used by this object
	 */
	GMutableSetT(const std::size_t& nFitnessCriteria)
		: GOptimizableEntity(nFitnessCriteria)
		, Gem::Common::GStdPtrVectorInterfaceT<T, Gem::Geneva::GObject>()
	{ /* nothing */   }

	/***************************************************************************/
	/**
	 * The copy constructor. Note that we cannot rely on the operator=() of the vector
	 * here, as we do not know the actual type of T objects.
	 *
	 * @param cp A copy of another GMutableSetT<T> object
	 */
	GMutableSetT(const GMutableSetT<T>& cp)
		: GOptimizableEntity(cp)
		, Gem::Common::GStdPtrVectorInterfaceT<T, Gem::Geneva::GObject>(cp)
	{ /* nothing */ }

	/***************************************************************************/
	/**
	 * The destructor. As we use smart pointers, we do not need to take care of the
	 * data in the vector ourselves.
	 */
	virtual ~GMutableSetT()
	{ /* nothing */ }

	/***************************************************************************/
	/**
	 * The standard assignment operator
	 */
	const GMutableSetT<T>& operator=(const GMutableSetT<T>& cp) {
		this->load_(&cp);
		return *this;
	}

	/***************************************************************************/
	/**
	 * Checks for equality with another GMutableSetT<T> object
	 *
	 * @param  cp A constant reference to another GMutableSetT<T> object
	 * @return A boolean indicating whether both objects are equal
	 */
	bool operator==(const GMutableSetT<T>& cp) const {
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
	 * Checks for inequality with another GMutableSetT<T> object
	 *
	 * @param  cp A constant reference to another GMutableSetT<T> object
	 * @return A boolean indicating whether both objects are inequal
	 */
	bool operator!=(const GMutableSetT<T>& cp) const {
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

		// Check that we are dealing with a GMutableSetT<T> reference independent of this object and convert the pointer
		const GMutableSetT<T> *p_load = Gem::Common::g_convert_and_compare<GObject, GMutableSetT<T> >(cp, this);

		GToken token("GMutableSetT<T>", e);

		// Compare our parent data ...
		Gem::Common::compare_base<GOptimizableEntity>(IDENTITY(*this, *p_load), token);

		// ... and then the local data
		compare_t(IDENTITY(this->data,  p_load->data), token);

		// React on deviations from the expectation
		token.evaluate();
	}

	/***************************************************************************/
	/**
	 * Swap another object's vector with ours. We need to set the dirty flag of both
	 * individuals in this case.
	 */
	inline void swap(GMutableSetT<T>& cp) {
		Gem::Common::GStdPtrVectorInterfaceT<T, Gem::Geneva::GObject>::swap(cp.data);
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
	virtual void addConfigurationOptions (
		Gem::Common::GParserBuilder& gpb
	) override {
		// Call our parent class'es function
		GOptimizableEntity::addConfigurationOptions(gpb);

		// No local data
	}

	/***************************************************************************/
	/**
	 * Emits a name for this class / object
	 */
	virtual std::string name() const override {
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
	virtual void load_(const GObject* cp) override {
		// Convert cp into local format
		const GMutableSetT<T> *p_load = this->template gobject_conversion<GMutableSetT<T> >(cp);

		// No local data - load the parent class'es data
		GOptimizableEntity::load_(cp);
		Gem::Common::GStdPtrVectorInterfaceT<T, Gem::Geneva::GObject>::operator=(*p_load);
	}

	/***************************************************************************/
	/**
	 * Re-implementation of a corresponding function in GStdPtrVectorInterface.
	 * Make the vector wrapper purely virtual allows the compiler to perform
	 * further optimizations.
	 */
	virtual void dummyFunction() override { /* nothing */ }

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
		if(GOptimizableEntity::modify_GUnitTests()) result = true;
		if(Gem::Common::GStdPtrVectorInterfaceT<T, Gem::Geneva::GObject>::modify_GUnitTests()) result = true;

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
	virtual void specificTestsNoFailureExpected_GUnitTests() override {
#ifdef GEM_TESTING
		using boost::unit_test_framework::test_suite;
		using boost::unit_test_framework::test_case;

		// Call the parent classes' functions
		GOptimizableEntity::specificTestsNoFailureExpected_GUnitTests();
		Gem::Common::GStdPtrVectorInterfaceT<T, Gem::Geneva::GObject>::specificTestsNoFailureExpected_GUnitTests();

		// no local data, nothing to test

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		condnotset("GMutableSetT<>::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
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
		GOptimizableEntity::specificTestsFailuresExpected_GUnitTests();
		Gem::Common::GStdPtrVectorInterfaceT<T, Gem::Geneva::GObject>::specificTestsFailuresExpected_GUnitTests();

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
