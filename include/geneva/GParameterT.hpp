/**
 * @file GParameterT.hpp
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

#ifndef GPARAMETERT_HPP_
#define GPARAMETERT_HPP_

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
	:public GParameterBaseWithAdaptorsT<T>
{
	 ///////////////////////////////////////////////////////////////////////
	 friend class boost::serialization::access;

	 template<typename Archive>
	 void serialize(Archive & ar, const unsigned int){
		 using boost::serialization::make_nvp;

		 ar
		 & make_nvp("GParameterBaseWithAdaptors_T", boost::serialization::base_object<GParameterBaseWithAdaptorsT<T>>(*this))
		 & BOOST_SERIALIZATION_NVP(val_);
	 }
	 ///////////////////////////////////////////////////////////////////////

public:
	 /** @brief Used to identify the type supplied to this object */
	 using p_type = T;

	 /***************************************************************************/
	 /** The default constructor */
	 GParameterT()
		 : GParameterBaseWithAdaptorsT<T>()
			, val_(Gem::Common::GDefaultValueT<T>::value())
	 { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * Initialization by contained value.
	  *
	  * @param val The new value of val_
	  */
	 GParameterT(const T& val)
		 : GParameterBaseWithAdaptorsT<T>()
			, val_(val)
	 { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * The copy constructor.
	  *
	  * @param cp A copy of another GParameterT<T> object
	  */
	 GParameterT(const GParameterT<T>& cp)
		 : GParameterBaseWithAdaptorsT<T>(cp)
			, val_(cp.val_)
	 { /* nothing */   }

	 /***************************************************************************/
	 /**
	  * The destructor
	  */
	 virtual ~GParameterT()
	 { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * The standard assignment operator
	  */
	 GParameterT<T>& operator=(const GParameterT<T>& cp) {
		 this->load_(&cp);
		 return *this;
	 }

	 /***************************************************************************/
	 /**
	  * An assignment operator that allows us to set val_ . Note that the value is returned as
	  * a copy, not a reference. Hence we assume here that val_ is copy-constructible.
	  *
	  * @param val The new value for val_
	  * @return The new value of val_
	  */
	 virtual T operator=(const T& val) {
		 setValue(val);
		 return val_;
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
		 val_ = val;
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
		 return val_;
	 }

	 /* ----------------------------------------------------------------------------------
	  * Tested in GDoubleObject::specificTestsNoFailureExpected_GUnitTests()
	  * ----------------------------------------------------------------------------------
	  */

	 /***************************************************************************/
	 /**
	  * Checks for equality with another GParameterT<T> object
	  *
	  * @param  cp A constant reference to another GParameterT<T> object
	  * @return A boolean indicating whether both objects are equal
	  */
	 bool operator==(const GParameterT<T>& cp) const {
		 using namespace Gem::Common;
		 try {
			 this->compare(cp, Gem::Common::expectation::CE_EQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
			 return true;
		 } catch(g_expectation_violation&) {
			 return false;
		 }
	 }

	 /***************************************************************************/
	 /**
	  * Checks for inequality with another GParameterT<T> object
	  *
	  * @param  cp A constant reference to another GParameterT<T> object
	  * @return A boolean indicating whether both objects are inequal
	  */
	 bool operator!=(const GParameterT<T>& cp) const {
		 using namespace Gem::Common;
		 try {
			 this->compare(cp, Gem::Common::expectation::CE_INEQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
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

		 // Check that we are dealing with a  GParameterT<T> reference independent of this object and convert the pointer
		 const  GParameterT<T> *p_load = Gem::Common::g_convert_and_compare<GObject,  GParameterT<T>>(cp, this);

		 GToken token("GParameterT<T>", e);

		 // Compare our parent data ...
		 Gem::Common::compare_base<GParameterBaseWithAdaptorsT<T>>(IDENTITY(*this, *p_load), token);

		 // ... and then the local data
		 compare_t(IDENTITY(val_, p_load->val_), token);

		 // React on deviations from the expectation
		 token.evaluate();
	 }

	 /***************************************************************************/
	 /**
	  * Allows to adapt the value stored in this class.
	  *
	  * @return The number of adaptions that were performed
	  */
	 virtual std::size_t adaptImpl(
		 Gem::Hap::GRandomBase& gr
	 ) override {
		 return GParameterBaseWithAdaptorsT<T>::applyAdaptor(
			 val_
			 , this->range()
			 , gr
		 );
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
	 virtual void toPropertyTree (
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

	 /***************************************************************************/
	 /**
	  * Emits a name for this class / object
	  */
	 std::string name() const override {
		 return std::string("GParameterT");
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
		 val_ = val;
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
		 val_ = p_load->val_;
	 }

	 /***************************************************************************/

	 /** @brief Triggers random initialization of the parameter(-collection) */
	 virtual bool randomInit_(
		 const activityMode&
		 , Gem::Hap::GRandomBase&
	 ) override = 0;

	 /***************************************************************************/
	 /**
	  * The internal representation of our value. Mutability is needed as in some cases value
	  * calculation implies resetting of the internal value. We nevertheless want to be able
	  * to call the value() function from constant functions. Declared protected so some derived
	  * classes can (re-)set the value from a const function without forcing us to declare
	  * setValue() const.
	  */
	 mutable T val_;

private:
	 /** @brief Creates a deep clone of this object */
	 GObject* clone_() const override = 0;

public:
	 /***************************************************************************/
	 /**
	  * Applies modifications to this object. This is needed for testing purposes
	  *
	  * @return A boolean which indicates whether modifications were made
	  */
	 bool modify_GUnitTests() override {
#ifdef GEM_TESTING
		 bool result = false;

		 // Call the parent classes' functions
		 if(GParameterBaseWithAdaptorsT<T>::modify_GUnitTests()) result = true;

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
	 void specificTestsNoFailureExpected_GUnitTests() override {
#ifdef GEM_TESTING
		 // Call the parent classes' functions
		 GParameterBaseWithAdaptorsT<T>::specificTestsNoFailureExpected_GUnitTests();

		 // All tests of our local functions are made in derived classes

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		 Gem::Common::condnotset("GParameterT<>::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
	 }

	 /***************************************************************************/
	 /**
	  * Performs self tests that are expected to fail. This is needed for testing purposes
	  */
	 void specificTestsFailuresExpected_GUnitTests() override {
#ifdef GEM_TESTING
		 // Call the parent classes' functions
		 GParameterBaseWithAdaptorsT<T>::specificTestsFailuresExpected_GUnitTests();

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		 Gem::Common::condnotset("GParameterT<>::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
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
template<typename T>
struct is_abstract<Gem::Geneva::GParameterT<T>> : public boost::true_type {};
template<typename T>
struct is_abstract< const Gem::Geneva::GParameterT<T>> : public boost::true_type {};
}
}

/******************************************************************************/

#endif /* GPARAMETERT_HPP_ */
