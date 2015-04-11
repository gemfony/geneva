/**
 * @file GParameterCollectionT.hpp
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

#ifndef GPARAMETERCOLLECTIONT_HPP_
#define GPARAMETERCOLLECTIONT_HPP_

// Geneva header files go here
#include "common/GTypeToStringT.hpp"
#include "common/GStdSimpleVectorInterfaceT.hpp"
#include "geneva/GObject.hpp"
#include "geneva/GParameterBaseWithAdaptorsT.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * A class holding a collection of mutable parameters - usually just an atomic value (double,
 * long, bool, ...).
 */
template<typename T>
class GParameterCollectionT
	:public GParameterBaseWithAdaptorsT<T>,
	 public Gem::Common::GStdSimpleVectorInterfaceT<T>
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int) {
		using boost::serialization::make_nvp;
		ar
		& make_nvp("GParameterBaseWithAdaptorsT_T", boost::serialization::base_object<GParameterBaseWithAdaptorsT<T> >(*this))
		& make_nvp("GStdSimpleVectorInterfaceT_T", boost::serialization::base_object<Gem::Common::GStdSimpleVectorInterfaceT<T> >(*this));
	}
	///////////////////////////////////////////////////////////////////////

	// Make sure this class can only be instantiated with T as an arithmetic type
	BOOST_MPL_ASSERT((boost::is_arithmetic<T>));

public:
	/***************************************************************************/
	/**
	 * The default constructor
	 */
	GParameterCollectionT()
		: GParameterBaseWithAdaptorsT<T> ()
		, Gem::Common::GStdSimpleVectorInterfaceT<T>()
	{ /* nothing */ }

	/***************************************************************************/
	/**
	 * Initialization with a number of variables of predefined values
	 *
	 * @param nval The number of values
	 * @param val  The value to be assigned to each position
	 */
	GParameterCollectionT(
      const std::size_t& nval
      , const T& val
   )
		: GParameterBaseWithAdaptorsT<T> ()
		, Gem::Common::GStdSimpleVectorInterfaceT<T>(nval, val)
	{ /* nothing */ }

	/***************************************************************************/
	/**
	 * The copy constructor
	 *
	 * @param cp A copy of another GParameterCollectionT<T> object
	 */
	GParameterCollectionT(const GParameterCollectionT<T>& cp)
		: GParameterBaseWithAdaptorsT<T> (cp)
		, Gem::Common::GStdSimpleVectorInterfaceT<T>(cp)
	{  /* nothing */ }

	/***************************************************************************/
	/**
	 * The standard destructor
	 */
	virtual ~GParameterCollectionT()
	{ /* nothing */ }

   /***************************************************************************/
   /**
    * The standard assignment operator
    */
   const GParameterCollectionT<T>& operator=(const GParameterCollectionT<T>& cp) {
      this->load_(&cp);
      return *this;
   }

   /***************************************************************************/
    /**
     * Checks for equality with another GParameterCollectionT<T> object
     *
     * @param  cp A constant reference to another GParameterCollectionT<T> object
     * @return A boolean indicating whether both objects are equal
     */
    bool operator==(const GParameterCollectionT<T>& cp) const {
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
     * Checks for inequality with another GParameterCollectionT<T> object
     *
     * @param  cp A constant reference to another GParameterCollectionT<T> object
     * @return A boolean indicating whether both objects are inequal
     */
    bool operator!=(const GParameterCollectionT<T>& cp) const {
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
   ) const OVERRIDE {
      using namespace Gem::Common;

      // Check that we are indeed dealing with a GAdaptorT reference
      const GParameterCollectionT<T>  *p_load = GObject::gobject_conversion<GParameterCollectionT<T> >(&cp);

      GToken token("GParameterCollectionT<T>", e);

      // Compare our parent data ...
      compare_base<GParameterBaseWithAdaptorsT<T> >(IDENTITY(*this, *p_load), token);

      // We access the relevant data of one of the parent classes directly for simplicity reasons
      compare_t(IDENTITY(this->data, p_load->data), token);

      // React on deviations from the expectation
      token.evaluate();
   }

	/***************************************************************************/
	/**
	 * Allows to adapt the values stored in this class. applyAdaptor expects a reference
	 * to a std::vector<T>. As we are derived from a wrapper of this class, we can just pass
	 * a reference to its data vector to the function.
	 *
	 * @return The number of adaptions that were carried out
	 */
	virtual std::size_t adaptImpl() OVERRIDE {
	   return GParameterBaseWithAdaptorsT<T>::applyAdaptor(
	         Gem::Common::GStdSimpleVectorInterfaceT<T>::data
         , this->range()
      );
	}

	/* ----------------------------------------------------------------------------------
	 * Tested in GDoubleObject::specificTestsNoFailureExpected_GUnitTests()
	 * ----------------------------------------------------------------------------------
	 */

	/***************************************************************************/
	/**
	 * Allows to identify whether we are dealing with a collection or an individual parameter
	 * (which is obviously not the case here). This function needs to be overloaded for parameter
	 * collections so that its inverse (GParameterBase::isParameterCollection() ) returns the
	 * correct value.
	 *
	 * @return A boolean indicating whether this GParameterBase-derivative is an individual parameter
	 */
	virtual bool isIndividualParameter() const OVERRIDE {
		return false;
	}

	/***************************************************************************/
	/**
	 * Swap another object's vector with ours
	 */
	inline void swap(GParameterCollectionT<T>& cp) {
	   Gem::Common::GStdSimpleVectorInterfaceT<T>::swap(cp.data);
	}

	/* ----------------------------------------------------------------------------------
	 * Tested in GDoubleObject::specificTestsNoFailureExpected_GUnitTests()
	 * ----------------------------------------------------------------------------------
	 */

	/***************************************************************************/
	/**
	 * Retrieval of the value at a given position
	 *
	 * @param pos The position for which the value needs to be returned
	 * @return The value of val_
	 */
	virtual T value(const std::size_t& pos) {
		return this->at(pos);
	}

	/***************************************************************************/
	/**
	 * Allows to set the internal (and usually externally visible) value at a given position. Note
	 * that we assume here that T has an operator=() or is a basic value type, such as double
	 * or int.
	 *
	 * @param pos The position at which the value shout be stored
	 * @param val The new T value stored in this class
	 */
	virtual void setValue(const std::size_t& pos, const T& val)  {
		this->at(pos) = val;

#ifdef DEBUG
		assert(this->at(pos) == val);
#endif
	}

	/***************************************************************************/
	/**
	 * Converts the local data to a boost::property_tree node
	 *
	 * @param ptr The boost::property_tree object the data should be saved to
	 * @param id The id assigned to this object
	 */
	virtual void toPropertyTree(
	      pt::ptree& ptr
	      , const std::string& baseName
	) const OVERRIDE {
#ifdef DEBUG
	   // Check that the object isn't empty
	   if(this->empty()) {
	      glogger
	      << "In GParameterCollectionT<T>::toPropertyTree(): Error!" << std::endl
	      << "Object is empty!" << std::endl
	      << GEXCEPTION;
	   }
#endif /* DEBUG */

      ptr.put(baseName + ".name", this->getParameterName());
      ptr.put(baseName + ".type", this->name());
      ptr.put(baseName + ".baseType", Gem::Common::GTypeToStringT<T>::value());
      ptr.put(baseName + ".isLeaf", this->isLeaf());
      ptr.put(baseName + ".nVals", this->size());

      typename GParameterCollectionT<T>::const_iterator cit;
      std::size_t pos;
      for(cit=this->begin(); cit!=this->end(); ++cit) {
         pos = cit - this->begin();
         ptr.put(baseName + "values.value" + boost::lexical_cast<std::string>(pos), *cit);
      }
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
      return std::string("GParameterCollectionT");
   }

protected:
	/***************************************************************************/
	/**
	 * Loads the data of another GParameterCollectionT<T> object, camouflaged as a GObject
	 *
	 * @param cp A copy of another GParameterCollectionT<T> object, camouflaged as a GObject
	 */
	virtual void load_(const GObject* cp) OVERRIDE {
		// Convert cp into local format and check for self-assignment
		const GParameterCollectionT<T> *p_load = GObject::gobject_conversion<GParameterCollectionT<T> >(cp);

		// Load our parent class'es data ...
		GParameterBaseWithAdaptorsT<T>::load_(cp);
		Gem::Common::GStdSimpleVectorInterfaceT<T>::operator=(*p_load);
	}

	/***************************************************************************/
	/**
	 * Creates a deep clone of this object. Purely virtual, so this class cannot be instantiated.
	 */
	virtual GObject* clone_() const = 0;

	/***************************************************************************/
	/**
	 * Re-implementation of a corresponding function in GStdSimpleVectorInterface.
	 * Making the vector wrapper purely virtual allows the compiler to perform
	 * further optimizations.
	 */
	virtual void dummyFunction() OVERRIDE { /* nothing */ }

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
		if(Gem::Common::GStdSimpleVectorInterfaceT<T>::modify_GUnitTests()) result = true;

		return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		condnotset("GParameterCollectionT<>::modify_GUnitTests", "GEM_TESTING");
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
		Gem::Common::GStdSimpleVectorInterfaceT<T>::specificTestsNoFailureExpected_GUnitTests();

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
      condnotset("GParameterCollectionT<>::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
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
		Gem::Common::GStdSimpleVectorInterfaceT<T>::specificTestsFailuresExpected_GUnitTests();


#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
      condnotset("GParameterCollectionT<>::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
	}

	/******************************************************************************/
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
    struct is_abstract<Gem::Geneva::GParameterCollectionT<T> > : public boost::true_type {};
    template<typename T>
    struct is_abstract< const Gem::Geneva::GParameterCollectionT<T> > : public boost::true_type {};
  }
}

/******************************************************************************/

#endif /* GPARAMETERCOLLECTIONT_HPP_ */
