/**
 * @file GNumCollectionT.hpp
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

#ifndef GNUMCOLLECTIONT_HPP_
#define GNUMCOLLECTIONT_HPP_

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard header files go here
#include <type_traits>

// Boost header files go here

// Geneva header files go here
#include "common/GExceptions.hpp"
#include "common/GTypeToStringT.hpp"
#include "geneva/GObject.hpp"
#include "geneva/GDoubleGaussAdaptor.hpp"
#include "geneva/GParameterCollectionT.hpp"

namespace Gem {
namespace Geneva {

const double DEFAULTLOWERINITBOUNDARYCOLLECTION=0.;
const double DEFAULTUPPERINITBOUNDARYCOLLECTION=1.;

/******************************************************************************/
/**
 * This class represents a collection of numeric values, all modified
 * using the same algorithm. The most likely types to be stored in this
 * class are double and std::int32_t . By using the framework provided
 * by GParameterCollectionT, this class becomes rather simple.
 */
template <typename num_type>
class GNumCollectionT
	: public GParameterCollectionT<num_type>
{
	 ///////////////////////////////////////////////////////////////////////
	 friend class boost::serialization::access;

	 template<typename Archive>
	 void serialize(Archive & ar, const unsigned int) {
		 using boost::serialization::make_nvp;
		 ar
		 & make_nvp("GParameterCollectionT",	boost::serialization::base_object<GParameterCollectionT<num_type>>(*this))
		 & BOOST_SERIALIZATION_NVP(lowerInitBoundary_)
		 & BOOST_SERIALIZATION_NVP(upperInitBoundary_);
	 }
	 ///////////////////////////////////////////////////////////////////////

	 // Make sure this class can only be instantiated with num_type as an arithmetic type
	 static_assert(
		 std::is_arithmetic<num_type>::value
		 , "num_type should be an arithmetic type"
	 );


public:
	 /** @brief Specifies the type of parameters stored in this collection */
	 using collection_type = num_type;

	 /***************************************************************************/
	 /**
	  * The default constructor.
	  */
	 GNumCollectionT()
		 : GParameterCollectionT<num_type> ()
			, lowerInitBoundary_(num_type(DEFAULTLOWERINITBOUNDARYCOLLECTION))
			, upperInitBoundary_(num_type(DEFAULTUPPERINITBOUNDARYCOLLECTION))
	 { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * Specifies the boundaries for random initialization and initializes
	  * the data vector with a given size. Note that we need to care for the
	  * assignment of random values ourself in derived classes.
	  *
	  * @param nval The number of entries in the vector
	  * @param min The lower boundary for random entries
	  * @param max The upper boundary for random entries
	  */
	 GNumCollectionT(
		 const std::size_t& nval
		 , const num_type& min
		 , const num_type& max
		 , typename std::enable_if<std::is_arithmetic<num_type>::value>::type *dummy = nullptr
	 )
		 : GParameterCollectionT<num_type> (nval, min)
			, lowerInitBoundary_(min)
			, upperInitBoundary_(max)
	 { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * Specifies the size of the data vector and an item to be assigned
	  * to each position. We enforce setting of the lower and upper boundaries
	  * for random initialization, as these double up as the preferred value
	  * range in some optimization algorithms, such as swarm algorithms.
	  *
	  * @param nval The number of entries in the vector
	  * @param val  The value to be assigned to each position
	  * @param min The lower boundary for random entries
	  * @param max The upper boundary for random entries
	  */
	 GNumCollectionT(
		 const std::size_t& nval
		 , const num_type& val
		 , const num_type& min
		 , const num_type& max
		 , typename std::enable_if<std::is_arithmetic<num_type>::value>::type *dummy = nullptr
	 )
		 : GParameterCollectionT<num_type> (nval, val)
			, lowerInitBoundary_(min)
			, upperInitBoundary_(max)
	 { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * The standard copy constructor
	  */
	 GNumCollectionT(const GNumCollectionT<num_type>& cp)
		 : GParameterCollectionT<num_type> (cp)
			, lowerInitBoundary_(cp.lowerInitBoundary_)
			, upperInitBoundary_(cp.upperInitBoundary_)
	 { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * The standard destructor
	  */
	 virtual ~GNumCollectionT()
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

		 // Check that we are dealing with a GNumCollectionT<num_type> reference independent of this object and convert the pointer
		 const GNumCollectionT<num_type> *p_load = Gem::Common::g_convert_and_compare<GObject, GNumCollectionT<num_type>>(cp, this);

		 GToken token("GNumCollectionT<num_type>", e);

		 // Compare our parent data ...
		 Gem::Common::compare_base<GParameterCollectionT<num_type>>(IDENTITY(*this, *p_load), token);

		 // ... and then the local data
		 compare_t(IDENTITY(lowerInitBoundary_, p_load->lowerInitBoundary_), token);
		 compare_t(IDENTITY(upperInitBoundary_, p_load->upperInitBoundary_), token);

		 // React on deviations from the expectation
		 token.evaluate();
	 }

	 /***************************************************************************/
	 /**
	  * Sets the initialization boundaries
	  *
	  * @param lowerInitBoundary The lower boundary for random initialization
	  * @param upperInitBoundary The upper boundary for random initialization
	  */
	 void setInitBoundaries(
		 const num_type& lowerInitBoundary
		 , const num_type& upperInitBoundary
		 , typename std::enable_if<std::is_arithmetic<num_type>::value>::type *dummy = nullptr
	 ) {
		 // Do some error checking
		 if(lowerInitBoundary >= upperInitBoundary) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG, time_and_place)
					 << "In GNumCollectionT<num_type>::setInitBoundaries():" << std::endl
					 << "Invalid boundaries provided: " << std::endl
					 << "lowerInitBoundary = " << lowerInitBoundary << std::endl
					 << "upperInitBoundary = " << upperInitBoundary << std::endl
			 );
		 }

		 lowerInitBoundary_ = lowerInitBoundary;
		 upperInitBoundary_ = upperInitBoundary;
	 }

	 /* ----------------------------------------------------------------------------------
	  * Tested GFPNumCollectionT<num_type>::specificTestsNoFailureExpected_GUnitTests()
	  * Tested GNumCollectionT<num_type>::specificTestsNoFailureExpected_GUnitTests()
	  * Assignment of invalid boundaries Tested GNumCollectionT<num_type>::specificTestsFailuresExpected_GUnitTests()
	  * ----------------------------------------------------------------------------------
	  */

	 /***************************************************************************/
	 /**
	  * Retrieves the value of the lower initialization boundary
	  *
	  * @return The value of the lower initialization boundary
	  */
	 num_type getLowerInitBoundary() const {
		 return lowerInitBoundary_;
	 }

	 /* ----------------------------------------------------------------------------------
	  * Tested GFPNumCollectionT<num_type>::specificTestsNoFailureExpected_GUnitTests()
	  * Tested GNumCollectionT<num_type>::specificTestsNoFailureExpected_GUnitTests()
	  * ----------------------------------------------------------------------------------
	  */

	 /***************************************************************************/
	 /**
	  * Retrieves the value of the upper initialization boundary
	  *
	  * @return The value of the upper initialization boundary
	  */
	 num_type getUpperInitBoundary() const {
		 return upperInitBoundary_;
	 }

	 /* ----------------------------------------------------------------------------------
	  * Tested GFPNumCollectionT<num_type>::specificTestsNoFailureExpected_GUnitTests()
	  * Tested GNumCollectionT<num_type>::specificTestsNoFailureExpected_GUnitTests()
	  * ----------------------------------------------------------------------------------
	  */

	 /***************************************************************************/
	 /**
	  * Emits a name for this class / object
	  */
	 std::string name() const override {
		 return std::string("GNumCollectionT");
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
					 << "In GNumCollection<num_type>::toPropertyTree(): Error!" << std::endl
					 << "Object is empty!" << std::endl
			 );
		 }
#endif /* DEBUG */

		 ptr.put(baseName + ".name", this->getParameterName());
		 ptr.put(baseName + ".type", this->name());
		 ptr.put(baseName + ".baseType", Gem::Common::GTypeToStringT<num_type>::value());
		 ptr.put(baseName + ".isLeaf", this->isLeaf());
		 ptr.put(baseName + ".nVals", this->size());

		 typename GNumCollectionT<num_type>::const_iterator cit;
		 std::size_t pos;
		 for(cit=this->begin(); cit!=this->end(); ++cit) {
			 pos = cit - this->begin();
			 ptr.put(baseName + "values.value" + Gem::Common::to_string(pos), *cit);
		 }
		 ptr.put(baseName + ".lowerBoundary", this->getLowerInitBoundary());
		 ptr.put(baseName + ".upperBoundary", this->getUpperInitBoundary());
		 ptr.put(baseName + ".initRandom", false); // Unused for the creation of a property tree
		 ptr.put(baseName + ".adaptionsActive", this->adaptionsActive());
	 }

protected:
	 /***************************************************************************/
	 /**
	  * Loads the data of another GNumCollectionT<num_type> object,
	  * camouflaged as a GObject. We have no local data, so
	  * all we need to do is to the standard identity check,
	  * preventing that an object is assigned to itself.
	  *
	  * @param cp A copy of another GNumCollectionT<num_type> object, camouflaged as a GObject
	  */
	 void load_(const GObject *cp) override {
		 // Check that we are dealing with a GNumCollectionT<num_type> reference independent of this object and convert the pointer
		 const GNumCollectionT<num_type> *p_load = Gem::Common::g_convert_and_compare<GObject, GNumCollectionT<num_type>>(cp, this);

		 // Load our parent class'es data ...
		 GParameterCollectionT<num_type>::load_(cp);

		 // ... and then our local data
		 lowerInitBoundary_ = p_load->lowerInitBoundary_;
		 upperInitBoundary_ = p_load->upperInitBoundary_;
	 }

	 /***************************************************************************/
	 /**
	  * Returns a "comparative range". This is e.g. used to make Gauss-adaption
	  * independent of a parameters value range
	  */
	 num_type range() const override {
		 return upperInitBoundary_ - lowerInitBoundary_;
	 }

	 /***************************************************************************/
	 /** @brief Triggers random initialization of the parameter collection */
	 virtual bool randomInit_(
		 const activityMode&
		 , Gem::Hap::GRandomBase&
	 ) override = 0;

private:
	 /***************************************************************************/
	 /**
	  * Creates a deep copy of this object. Purely virtual as this class
	  * should not be instantiable.
	  *
	  * @return A pointer to a deep clone of this object
	  */
	 GObject *clone_() const override = 0;

	 /***************************************************************************/
	 num_type lowerInitBoundary_; ///< The lower boundary for random initialization
	 num_type upperInitBoundary_; ///< The upper boundary for random initialization

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
		 if(GParameterCollectionT<num_type>::modify_GUnitTests()) result = true;

		 return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		 Gem::Common::condnotset("GNumCollectionT<>::modify_GUnitTests", "GEM_TESTING");
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
		 GParameterCollectionT<num_type>::specificTestsNoFailureExpected_GUnitTests();

		 // A few settings
		 const num_type LOWERTESTINITVAL = num_type(1); // Do not choose a negative value as num_type might be an unsigned type
		 const num_type UPPERTESTINITVAL = num_type(3);

		 //------------------------------------------------------------------------------

		 { // Test setting and retrieval of initialization boundaries
			 std::shared_ptr<GNumCollectionT<num_type>> p_test = this->template clone<GNumCollectionT<num_type>>();

			 // Set the boundaries
			 BOOST_CHECK_NO_THROW(p_test->setInitBoundaries(LOWERTESTINITVAL, UPPERTESTINITVAL));

			 // Check that these values have indeed been assigned
			 BOOST_CHECK(p_test->getLowerInitBoundary() == LOWERTESTINITVAL);
			 BOOST_CHECK(p_test->getUpperInitBoundary() == UPPERTESTINITVAL);
		 }

		 //------------------------------------------------------------------------------

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
		 Gem::Common::condnotset("GNumCollectionT<>::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
	 }

	 /***************************************************************************/
	 /**
	  * Performs self tests that are expected to fail. This is needed for testing purposes
	  */
	 void specificTestsFailuresExpected_GUnitTests() override {
#ifdef GEM_TESTING
		 // Call the parent classes' functions
		 GParameterCollectionT<num_type>::specificTestsFailuresExpected_GUnitTests();

		 // A few settings
		 const num_type LOWERTESTINITVAL = num_type(1); // Do not choose a negative value as num_type might be an unsigned type
		 const num_type UPPERTESTINITVAL = num_type(3);

		 //------------------------------------------------------------------------------

		 { // Check that assignement of initialization boundaries throws for invalid boundaries
			 std::shared_ptr<GNumCollectionT<num_type>> p_test = this->template clone<GNumCollectionT<num_type>>();

			 BOOST_CHECK_THROW(p_test->setInitBoundaries(UPPERTESTINITVAL, LOWERTESTINITVAL), gemfony_exception);
		 }

		 //------------------------------------------------------------------------------

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
		 Gem::Common::condnotset("GNumCollectionT<>::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
	 }

	 /***************************************************************************/
};

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

/******************************************************************************/
// The content of BOOST_SERIALIZATION_ASSUME_ABSTRACT(num_type)

namespace boost {
namespace serialization {
template<typename num_type>
struct is_abstract<Gem::Geneva::GNumCollectionT<num_type>> : public boost::true_type {};
template<typename num_type>
struct is_abstract< const Gem::Geneva::GNumCollectionT<num_type>> : public boost::true_type {};
}
}
/******************************************************************************/

#endif /* GNUMCOLLECTIONT_HPP_ */
