/**
 * @file
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

#pragma once

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard header files go here
#include <type_traits>

// Boost header files go here

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
template<typename num_type>
class GParameterCollectionT
	: public GParameterBaseWithAdaptorsT<num_type>
	  , public Gem::Common::GStdSimpleVectorInterfaceT<num_type>
{
	 ///////////////////////////////////////////////////////////////////////
	 friend class boost::serialization::access;

	 template<typename Archive>
	 void serialize(Archive & ar, const unsigned int) {
		 using boost::serialization::make_nvp;
		 ar
		 & make_nvp("GParameterBaseWithAdaptorsT_num_type", boost::serialization::base_object<GParameterBaseWithAdaptorsT<num_type>>(*this))
		 & make_nvp("GStdSimpleVectorInterfaceT_num_type", boost::serialization::base_object<Gem::Common::GStdSimpleVectorInterfaceT<num_type>>(*this));
	 }
	 ///////////////////////////////////////////////////////////////////////

	 // Make sure this class can only be instantiated with num_type as an arithmetic type
	 static_assert(
		 std::is_arithmetic<num_type>::value
		 , "num_type should be an arithmetic type"
	 );

public:
	 /***************************************************************************/
	 /**
	  * The default constructor
	  */
	 GParameterCollectionT()
		 : GParameterBaseWithAdaptorsT<num_type> ()
			, Gem::Common::GStdSimpleVectorInterfaceT<num_type>()
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
		 , const num_type& val
	 )
		 : GParameterBaseWithAdaptorsT<num_type> ()
			, Gem::Common::GStdSimpleVectorInterfaceT<num_type>(nval, val)
	 { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * The copy constructor
	  *
	  * @param cp A copy of another GParameterCollectionT<num_type> object
	  */
	 GParameterCollectionT(const GParameterCollectionT<num_type>& cp)
		 : GParameterBaseWithAdaptorsT<num_type> (cp)
			, Gem::Common::GStdSimpleVectorInterfaceT<num_type>(cp)
	 {  /* nothing */ }

	 /***************************************************************************/
	 /**
	  * The standard destructor
	  */
	 virtual ~GParameterCollectionT()
	 { /* nothing */ }

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

		 // Check that we are dealing with a  GParameterCollectionT<num_type> reference independent of this object and convert the pointer
		 const GParameterCollectionT<num_type> *p_load = Gem::Common::g_convert_and_compare<GObject,  GParameterCollectionT<num_type>>(cp, this);

		 GToken token("GParameterCollectionT<num_type>", e);

		 // Compare our parent data ...
		 Gem::Common::compare_base<GParameterBaseWithAdaptorsT<num_type>>(IDENTITY(*this, *p_load), token);

		 // We access the relevant data of one of the parent classes directly for simplicity reasons
		 compare_t(IDENTITY(this->data, p_load->data), token);

		 // React on deviations from the expectation
		 token.evaluate();
	 }

	 /***************************************************************************/
	 /**
	  * Allows to adapt the values stored in this class. applyAdaptor expects a reference
	  * to a std::vector<num_type>. As we are derived from a wrapper of this class, we can just pass
	  * a reference to its data vector to the function.
	  *
	  * @return The number of adaptions that were carried out
	  */
	 virtual std::size_t adaptImpl(
		 Gem::Hap::GRandomBase& gr
	 ) override {
		 return GParameterBaseWithAdaptorsT<num_type>::applyAdaptor(
			 Gem::Common::GStdSimpleVectorInterfaceT<num_type>::data
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
	  * Allows to identify whether we are dealing with a collection or an individual parameter
	  * (which is obviously not the case here). This function needs to be overloaded for parameter
	  * collections so that its inverse (GParameterBase::isParameterCollection() ) returns the
	  * correct value.
	  *
	  * @return A boolean indicating whether this GParameterBase-derivative is an individual parameter
	  */
	 bool isIndividualParameter() const override {
		 return false;
	 }

	 /***************************************************************************/
	 /**
	  * Swap another object's vector with ours
	  */
	 inline void swap(GParameterCollectionT<num_type>& cp) {
		 Gem::Common::GStdSimpleVectorInterfaceT<num_type>::swap(cp.data);
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
	 virtual num_type value(const std::size_t& pos) {
		 return this->at(pos);
	 }

	 /***************************************************************************/
	 /**
	  * Allows to set the internal (and usually externally visible) value at a given position. Note
	  * that we assume here that num_type has an operator=() or is a basic value type, such as double
	  * or int.
	  *
	  * @param pos The position at which the value shout be stored
	  * @param val The new num_type value stored in this class
	  */
	 virtual void setValue(const std::size_t& pos, const num_type& val)  {
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
	 ) const override {
#ifdef DEBUG
		 // Check that the object isn't empty
		 if(this->empty()) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG, time_and_place)
					 << "In GParameterCollectionT<num_type>::toPropertyTree(): Error!" << std::endl
					 << "Object is empty!" << std::endl
			 );
		 }
#endif /* DEBUG */

		 ptr.put(baseName + ".name", this->getParameterName());
		 ptr.put(baseName + ".type", this->name());
		 ptr.put(baseName + ".baseType", Gem::Common::GTypeToStringT<num_type>::value());
		 ptr.put(baseName + ".isLeaf", this->isLeaf());
		 ptr.put(baseName + ".nVals", this->size());

		 typename GParameterCollectionT<num_type>::const_iterator cit;
		 std::size_t pos;
		 for(cit=this->begin(); cit!=this->end(); ++cit) {
			 pos = std::distance(this->begin(), cit);
			 ptr.put(baseName + "values.value" + Gem::Common::to_string(pos), *cit);
		 }
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
		 return std::string("GParameterCollectionT");
	 }

protected:
	 /***************************************************************************/
	 /**
	  * Loads the data of another GParameterCollectionT<num_type> object, camouflaged as a GObject
	  *
	  * @param cp A copy of another GParameterCollectionT<num_type> object, camouflaged as a GObject
	  */
	 void load_(const GObject* cp) override {
		 // Check that we are dealing with a  GParameterCollectionT<num_type> reference independent of this object and convert the pointer
		 const GParameterCollectionT<num_type> *p_load = Gem::Common::g_convert_and_compare<GObject,  GParameterCollectionT<num_type>>(cp, this);

		 // Load our parent class'es data ...
		 GParameterBaseWithAdaptorsT<num_type>::load_(cp);
		 Gem::Common::GStdSimpleVectorInterfaceT<num_type>::operator=(*p_load);
	 }

	 /***************************************************************************/
	 /**
	  * Re-implementation of a corresponding function in GStdSimpleVectorInterface.
	  * Making the vector wrapper purely virtual allows the compiler to perform
	  * further optimizations.
	  */
	 void dummyFunction() override { /* nothing */ }

private:
	 /***************************************************************************/
	 /**
	  * Creates a deep clone of this object. Purely virtual, so this class cannot be instantiated.
	  */
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
		 if(GParameterBaseWithAdaptorsT<num_type>::modify_GUnitTests()) result = true;
		 if(Gem::Common::GStdSimpleVectorInterfaceT<num_type>::modify_GUnitTests()) result = true;

		 return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		 Gem::Common::condnotset("GParameterCollectionT<>::modify_GUnitTests", "GEM_TESTING");
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
		 GParameterBaseWithAdaptorsT<num_type>::specificTestsNoFailureExpected_GUnitTests();
		 Gem::Common::GStdSimpleVectorInterfaceT<num_type>::specificTestsNoFailureExpected_GUnitTests();

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		 Gem::Common::condnotset("GParameterCollectionT<>::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
	 }

	 /***************************************************************************/
	 /**
	  * Performs self tests that are expected to fail. This is needed for testing purposes
	  */
	 void specificTestsFailuresExpected_GUnitTests() override {
#ifdef GEM_TESTING
		 // Call the parent classes' functions
		 GParameterBaseWithAdaptorsT<num_type>::specificTestsFailuresExpected_GUnitTests();
		 Gem::Common::GStdSimpleVectorInterfaceT<num_type>::specificTestsFailuresExpected_GUnitTests();


#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		 Gem::Common::condnotset("GParameterCollectionT<>::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
	 }

	 /******************************************************************************/
};

} /* namespace Geneva */
} /* namespace Gem */

/******************************************************************************/
/**
 * @brief The content of the BOOST_SERIALIZATION_ASSUME_ABSTRACT(num_type) macro. Needed for Boost.Serialization
 */
namespace boost {
namespace serialization {
template<typename num_type>
struct is_abstract<Gem::Geneva::GParameterCollectionT<num_type>> : public boost::true_type {};
template<typename num_type>
struct is_abstract< const Gem::Geneva::GParameterCollectionT<num_type>> : public boost::true_type {};
}
}

/******************************************************************************/

