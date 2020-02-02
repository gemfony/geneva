/**
 * @file
 */

/********************************************************************************
 *
 * This file is part of the Geneva library collection. The following license
 * applies to this file:
 *
 * ------------------------------------------------------------------------------
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * ------------------------------------------------------------------------------
 *
 * Note that other files in the Geneva library collection may use a different
 * license. Please see the licensing information in each file.
 *
 ********************************************************************************
 *
 * Geneva was started by Dr. Rüdiger Berlich and was later maintained together
 * with Dr. Ariel Garcia under the auspices of Gemfony scientific. For further
 * information on Gemfony scientific, see http://www.gemfomy.eu .
 *
 * The majority of files in Geneva was released under the Apache license v2.0
 * in February 2020.
 *
 * See the NOTICE file in the top-level directory of the Geneva library
 * collection for a list of contributors and copyright information.
 *
 ********************************************************************************/

#pragma once

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard header files go here

// Boost header files go here

// Geneva header files go here
#include "common/GTypeToStringT.hpp"
#include "common/GExceptions.hpp"
#include "geneva/GObject.hpp"
#include "geneva/GParameterBaseWithAdaptorsT.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * A class holding a single, mutable parameter - usually just an atomic value (double, long,
 * boolean, ...).
 */
template <typename T>
class GParameterT
	 : public GParameterBaseWithAdaptorsT<T>
{
	 ///////////////////////////////////////////////////////////////////////
	 friend class boost::serialization::access;

	 template<typename Archive>
	 void serialize(Archive & ar, const unsigned int){
		 using boost::serialization::make_nvp;

		 ar
		 & make_nvp("GParameterBaseWithAdaptors_T", boost::serialization::base_object<GParameterBaseWithAdaptorsT<T>>(*this))
		 & BOOST_SERIALIZATION_NVP(m_val);
	 }
	 ///////////////////////////////////////////////////////////////////////

public:
	 /** @brief Used to identify the type supplied to this object */
	 using p_type = T;

	 /***************************************************************************/
	 /** The default constructor */
	 GParameterT() = default;

	 /***************************************************************************/
	 /**
	  * Initialization by contained value.
	  *
	  * @param val The new value of val_
	  */
	 explicit GParameterT(const T& val)
		 : GParameterBaseWithAdaptorsT<T>()
		 , m_val(val)
	 { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * The copy constructor.
	  *
	  * @param cp A copy of another GParameterT<T> object
	  */
	 GParameterT(const GParameterT<T>& cp) = default;

	 /***************************************************************************/
	 /**
	  * The destructor
	  */
	 ~GParameterT() override = default;

	 /***************************************************************************/
	 /**
	  * An assignment operator that allows us to set val_ . Note that the value is returned as
	  * a copy, not a reference. Hence we assume here that val_ is copy-constructible.
	  *
	  * @param val The new value for val_
	  * @return The new value of val_
	  */
	 virtual GParameterT<T>& operator=(const T& val) BASE {
		 setValue(val);
		 return *this;
	 }

	 /***************************************************************************/
	 /**
	  * Allows to set the internal (and usually externally visible) value. Note
	  * that we assume here that T has an operator=() or is a basic value type, such as double
	  * or int.
	  *
	  * @param val The new T value stored in this class
	  */
	 virtual void setValue(const T& val) BASE {
		 m_val = val;
	 }

	 /* ----------------------------------------------------------------------------------
	  * Tested in GDoubleObject::specificTestsNoFailureExpected_GUnitTests()
	  * ----------------------------------------------------------------------------------
	  */

	 /***************************************************************************/
	 /**
	  * Automatic conversion to the target type
	  */
	 operator T() const {
		 return this->value();
	 }

	 /* ----------------------------------------------------------------------------------
	  * Tested in GDoubleObject::specificTestsNoFailureExpected_GUnitTests()
	  * ----------------------------------------------------------------------------------
	  */

	 /***************************************************************************/
	 /**
	  * Retrieval of the value
	  *
	  * @return The value of val_
	  */
	 virtual T value() const BASE {
		 return m_val;
	 }

	 /* ----------------------------------------------------------------------------------
	  * Tested in GDoubleObject::specificTestsNoFailureExpected_GUnitTests()
	  * ----------------------------------------------------------------------------------
	  */

	 /***************************************************************************/
	 /**
	  * Converts the local data to a boost::property_tree node
	  *
	  * @param ptr The boost::property_tree object the data should be saved to
	  * @param baseName The name assigned to the object
	  */
	 void toPropertyTree (
		 pt::ptree& ptr
		 , const std::string& baseName
	 ) const override {
		 ptr.put(baseName + ".name", this->getParameterName());
		 ptr.put(baseName + ".type", this->name());
		 ptr.put(baseName + ".baseType", Gem::Common::GTypeToStringT<T>::value());
		 ptr.put(baseName + ".isLeaf", this->isLeaf());
		 ptr.put(baseName + ".nVals", 1);
		 ptr.put(baseName + ".values.value0", this->value());
		 ptr.put(baseName + ".initRandom", false); // Unused for the creation of a property tree
		 ptr.put(baseName + ".adaptionsActive", this->adaptionsActive());
	 }

	 /***************************************************************************/
	 /**
	  * Lets the audience know whether this is a leaf or a branch object
	  */
	 bool isLeaf() const override {
		 return true;
	 }

protected:
	 /***************************************************************************/
	 /**
	  * Gives derived classes access to the internal value. A constant function is needed to
	  * allow resetting the value in the GConstrained family of classes from within the value()
	  * function (which by design should be constant). Still, users should be aware that generally
	  * setting of values is not a "const" action, so this function is protected.
	  *
	  * @param val The new T value stored in this class
	  */
	 void setValue_(const T& val) const {
		 m_val = val;
	 }

	 /* ----------------------------------------------------------------------------------
	  * Tested in GDoubleObject::specificTestsNoFailureExpected_GUnitTests()
	  * ----------------------------------------------------------------------------------
	  */

	 /***************************************************************************/
	 /**
	  * Loads the data of another GObject
	  *
	  * @param cp A copy of another GParameterT<T> object, camouflaged as a GObject
	  */
	 void load_(const GObject* cp) override {
		 // Check that we are dealing with a  GParameterT<T> reference independent of this object and convert the pointer
		 const GParameterT<T> *p_load = Gem::Common::g_convert_and_compare<GObject,  GParameterT<T>>(cp, this);

		 // Load our parent class'es data ...
		 GParameterBaseWithAdaptorsT<T>::load_(cp);

		 // ... and then our own data
		 m_val = p_load->m_val;
	 }

	/** @brief Allow access to this classes compare_ function */
	friend void Gem::Common::compare_base_t<GParameterT<T>>(
		GParameterT<T> const &
		, GParameterT<T> const &
		, Gem::Common::GToken &
	);

	/***************************************************************************/
	/**
     * Searches for compliance with expectations with respect to another object
     * of the same type
     *
     * @param cp A constant reference to another GObject object
     * @param e The expected outcome of the comparison
     * @param limit The maximum deviation for floating point values (important for similarity checks)
     */
	void compare_(
		const GObject& cp
		, const Gem::Common::expectation& e
		, const double& limit
	) const override {
		using namespace Gem::Common;

		// Check that we are dealing with a  GParameterT<T> reference independent of this object and convert the pointer
		const  GParameterT<T> *p_load = Gem::Common::g_convert_and_compare<GObject,  GParameterT<T>>(cp, this);

		GToken token("GParameterT<T>", e);

		// Compare our parent data ...
		Gem::Common::compare_base_t<GParameterBaseWithAdaptorsT<T>>(*this, *p_load, token);

		// ... and then the local data
		compare_t(IDENTITY(m_val, p_load->m_val), token);

		// React on deviations from the expectation
		token.evaluate();
	}

	 /***************************************************************************/

	 /** @brief Triggers random initialization of the parameter(-collection) */
	 bool randomInit_(
		 const activityMode&
		 , Gem::Hap::GRandomBase&
	 ) override = 0;

	/***************************************************************************/
	/**
     * Applies modifications to this object. This is needed for testing purposes
     *
     * @return A boolean which indicates whether modifications were made
     */
	bool modify_GUnitTests_() override {
#ifdef GEM_TESTING
		bool result = false;

		// Call the parent classes' functions
		if(GParameterBaseWithAdaptorsT<T>::modify_GUnitTests_()) result = true;

		return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		Gem::Common::condnotset("GParameterT<>::modify_GUnitTests", "GEM_TESTING");
		return false;
#endif /* GEM_TESTING */
	}

	/***************************************************************************/
	/**
     * Performs self tests that are expected to succeed. This is needed for testing purposes
     */
	void specificTestsNoFailureExpected_GUnitTests_() override {
#ifdef GEM_TESTING
		// Call the parent classes' functions
		GParameterBaseWithAdaptorsT<T>::specificTestsNoFailureExpected_GUnitTests_();

		// All tests of our local functions are made in derived classes

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		Gem::Common::condnotset("GParameterT<>::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
	}

	/***************************************************************************/
	/**
     * Performs self tests that are expected to fail. This is needed for testing purposes
     */
	void specificTestsFailuresExpected_GUnitTests_() override {
#ifdef GEM_TESTING
		// Call the parent classes' functions
		GParameterBaseWithAdaptorsT<T>::specificTestsFailuresExpected_GUnitTests_();

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		Gem::Common::condnotset("GParameterT<>::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
	}

	 /***************************************************************************/
	 /**
	  * The internal representation of our value. Mutability is needed as in some cases value
	  * calculation implies resetting of the internal value. We nevertheless want to be able
	  * to call the value() function from constant functions. Declared protected so some derived
	  * classes can (re-)set the value from a const function without forcing us to declare
	  * setValue() const.
	  */
	 mutable T m_val = Gem::Common::GDefaultValueT<T>::value();

private:
	 /***************************************************************************/
	 /**
	  * Emits a name for this class / object
	  */
	 std::string name_() const override {
		 return std::string("GParameterT");
	 }

	 /***************************************************************************/
	 /** @brief Creates a deep clone of this object */
	 GObject* clone_() const override = 0;

	/***************************************************************************/
	/**
     * Allows to adapt the value stored in this class.
     *
     * @return The number of adaptions that were performed
     */
	std::size_t adapt_(
		Gem::Hap::GRandomBase& gr
	) override {
		return GParameterBaseWithAdaptorsT<T>::applyAdaptor(
			m_val
			, this->range()
			, gr
		);
	}
};

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

/******************************************************************************/
// The content of BOOST_SERIALIZATION_ASSUME_ABSTRACT(T)

namespace boost {
namespace serialization {
template<typename T>
struct is_abstract<Gem::Geneva::GParameterT<T>> : public boost::true_type {};
template<typename T>
struct is_abstract< const Gem::Geneva::GParameterT<T>> : public boost::true_type {};
}
}

/******************************************************************************/

