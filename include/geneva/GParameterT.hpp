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
class G_API GParameterT
	:public GParameterBaseWithAdaptorsT<T>
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int){
	  using boost::serialization::make_nvp;

	  ar
	  & make_nvp("GParameterBaseWithAdaptors_T", boost::serialization::base_object<GParameterBaseWithAdaptorsT<T> >(*this))
	  & BOOST_SERIALIZATION_NVP(val_);
	}
	///////////////////////////////////////////////////////////////////////

public:
	/** @brief Used to identify the type supplied to this object */
	typedef T p_type;

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
	{ /* nothing */   }

   /***************************************************************************/
	/**
	 * Initialization with boundaries
	 *
    * @param lowerBoundary Lower boundary of value- or initialization range
    * @param upperBoundary Upper boundary of value- or initialization range
	 */
   GParameterT(
      const T& lowerBoundary
      , const T& upperBoundary
   )
      : GParameterBaseWithAdaptorsT<T>(lowerBoundary, upperBoundary)
      , val_(Gem::Common::GDefaultValueT<T>::value())
   { /* nothing */   }

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
	 * A standard assignment operator.
	 *
	 * @param cp A copy of another GParameterT object
	 * @return A constant reference to this object
	 */
	const GParameterT<T>& operator=(const GParameterT<T>& cp) {
		GParameterT<T>::load(&cp);
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
		// Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
		return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GParameterT<T>::operator==","cp", CE_SILENT);
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
		// Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
		return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GParameterT<T>::operator!=","cp", CE_SILENT);
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
		const GParameterT<T>  *p_load = GObject::gobject_conversion<GParameterT<T> >(&cp);

		// Will hold possible deviations from the expectation, including explanations
	    std::vector<boost::optional<std::string> > deviations;

		// Check our parent class'es data ...
		deviations.push_back(GParameterBaseWithAdaptorsT<T>::checkRelationshipWith(cp, e, limit, "GParameterT<T>", y_name, withMessages));

		// ... and then our local data
		deviations.push_back(checkExpectation(withMessages, "GParameterT<T>", val_, p_load->val_, "val_", "p_load->val_", e , limit));

		return evaluateDiscrepancies("GParameterT<T>", caller, deviations, e);
	}

	/***************************************************************************/
	/**
	 * Allows to adapt the value stored in this class.
	 *
	 * @return The number of adaptions that were performed
	 */
	virtual std::size_t adaptImpl() OVERRIDE {
		return GParameterBaseWithAdaptorsT<T>::applyAdaptor(
         val_
         , this->range()
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
	) const OVERRIDE {
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
   virtual bool isLeaf() const OVERRIDE {
      return true;
   }

   /***************************************************************************/
   /**
    * Emits a name for this class / object
    */
   virtual std::string name() const OVERRIDE {
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
	virtual void load_(const GObject* cp) OVERRIDE {
		// Convert cp into local format
		const GParameterT<T> *p_load = GObject::gobject_conversion<GParameterT<T> >(cp);

		// Load our parent class'es data ...
		GParameterBaseWithAdaptorsT<T>::load_(cp);

		// ... and then our own data
		val_ = p_load->val_;
	}

	/***************************************************************************/
	/** @brief Creates a deep clone of this object */
	virtual GObject* clone_() const = 0;
	/** @brief Triggers random initialization of the parameter(-collection) */
	virtual void randomInit_(const activityMode&) = 0;

	/***************************************************************************/
	/**
	 * The internal representation of our value. Mutability is needed as in some cases value
	 * calculation implies resetting of the internal value. We nevertheless want to be able
	 * to call the value() function from constant functions. Declared protected so some derived
	 * classes can (re-)set the value from a const function without forcing us to declare
	 * setValue() const.
	 */
	mutable T val_;

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
		if(GParameterBaseWithAdaptorsT<T>::modify_GUnitTests()) result = true;

		return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GParameterT<>::modify_GUnitTests", "GEM_TESTING");
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
		GParameterBaseWithAdaptorsT<T>::specificTestsNoFailureExpected_GUnitTests();

		// All tests of our local functions are made in derived classes

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		condnotset("GParameterT<>::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
	}

	/***************************************************************************/
	/**
	 * Performs self tests that are expected to fail. This is needed for testing purposes
	 */
	virtual void specificTestsFailuresExpected_GUnitTests() OVERRIDE {
#ifdef GEM_TESTING
		// Call the parent classes' functions
		GParameterBaseWithAdaptorsT<T>::specificTestsFailuresExpected_GUnitTests();

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
      condnotset("GParameterT<>::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
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
		struct is_abstract<Gem::Geneva::GParameterT<T> > : public boost::true_type {};
		template<typename T>
		struct is_abstract< const Gem::Geneva::GParameterT<T> > : public boost::true_type {};
	}
}

/******************************************************************************/

#endif /* GPARAMETERT_HPP_ */
