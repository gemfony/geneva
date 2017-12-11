/**
 * @file GNumT.hpp
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
#include <type_traits>

// Boost headers go here

#ifndef GNUMT_HPP_
#define GNUMT_HPP_


// Geneva headers go here
#include "common/GTypeToStringT.hpp"
#include "geneva/GParameterT.hpp"

namespace Gem {
namespace Geneva {

const double DEFAULTLOWERINITBOUNDARYSINGLE=0.;
const double DEFAULTUPPERINITBOUNDARYSINGLE=1.;

/******************************************************************************/
/**
 * This class represents numeric values. The most likely types to be stored
 * in this class are double and std::int32_t . By using the framework provided
 * by GParameterT, this class becomes rather simple.
 */
template <typename T>
class GNumT
	: public GParameterT<T>
{
	 ///////////////////////////////////////////////////////////////////////
	 friend class boost::serialization::access;

	 template<typename Archive>
	 void serialize(Archive & ar, const unsigned int) {
		 using boost::serialization::make_nvp;
		 ar
		 & make_nvp("GParameterT",	boost::serialization::base_object<GParameterT<T>>(*this))
		 & BOOST_SERIALIZATION_NVP(lowerInitBoundary_)
		 & BOOST_SERIALIZATION_NVP(upperInitBoundary_);
	 }
	 ///////////////////////////////////////////////////////////////////////

	 // Make sure this class can only be instantiated with T as an arithmetic type
	 static_assert(
		 std::is_arithmetic<T>::value
		 , "T should be an arithmetic type"
	 );

public:
	 /** @brief Specifies the type of parameters stored in this collection */
	 using collection_type = T;

	 /***************************************************************************/
	 /**
	  * The default constructor.
	  */
	 GNumT()
		 : GParameterT<T> ()
			, lowerInitBoundary_(T(DEFAULTLOWERINITBOUNDARYSINGLE))
			, upperInitBoundary_(T(DEFAULTUPPERINITBOUNDARYSINGLE))
	 { /* nothing */ }

	 /*****************************************************************/
	 /*
	  * Initialize with a single value
	  *
	  * @param val The value used for the initialization
	  */
	 explicit GNumT(const T& val)
		 : GParameterT<T>(val)
			, lowerInitBoundary_(T(DEFAULTLOWERINITBOUNDARYSINGLE))
			, upperInitBoundary_(T(DEFAULTUPPERINITBOUNDARYSINGLE))
	 { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * Initialize the boundaries. The internal value will be
	  * initialized with the lower boundary.
	  *
	  * @param min The lower boundary for random entries
	  * @param max The upper boundary for random entries
	  */
	 GNumT(const T& min, const T& max)
		 : GParameterT<T> (min)
			, lowerInitBoundary_(min)
			, upperInitBoundary_(max)
	 { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * The standard copy constructor
	  */
	 GNumT(const GNumT<T>& cp)
		 : GParameterT<T> (cp)
			, lowerInitBoundary_(cp.lowerInitBoundary_)
			, upperInitBoundary_(cp.upperInitBoundary_)
	 { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * The standard destructor
	  */
	 virtual ~GNumT()
	 { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * The standard assignment operator
	  */
	 GNumT<T>& operator=(const GNumT<T>& cp) {
		 this->load_(&cp);
		 return *this;
	 }

	 /***************************************************************************/
	 /**
	  * An assignment operator for the contained value type
	  */
	 T operator=(const T& val) override {
		 return GParameterT<T>::operator=(val);
	 }

	 /***************************************************************************/
	 /**
	  * Checks for equality with another GNumT<T> object
	  *
	  * @param  cp A constant reference to another GNumT<T> object
	  * @return A boolean indicating whether both objects are equal
	  */
	 bool operator==(const GNumT<T>& cp) const {
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
	  * Checks for inequality with another GNumT<T> object
	  *
	  * @param  cp A constant reference to another GNumT<T> object
	  * @return A boolean indicating whether both objects are inequal
	  */
	 bool operator!=(const GNumT<T>& cp) const {
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

		 // Check that we are dealing with a GNumT<T> reference independent of this object and convert the pointer
		 const GNumT<T> *p_load = Gem::Common::g_convert_and_compare<GObject, GNumT<T>>(cp, this);

		 GToken token("GNumT<T>", e);

		 // Compare our parent data ...
		 Gem::Common::compare_base<GParameterT<T>>(IDENTITY(*this, *p_load), token);

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
	 void setInitBoundaries(const T& lowerInitBoundary, const T& upperInitBoundary) {
		 // Do some error checking
		 if(lowerInitBoundary >= upperInitBoundary) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG, time_and_place)
					 << "In GNumT<T>::setInitBoundaries():" << std::endl
					 << "Invalid boundaries provided: " << std::endl
					 << "lowerInitBoundary = " << lowerInitBoundary << std::endl
					 << "upperInitBoundary = " << upperInitBoundary << std::endl
			 );
		 }

		 lowerInitBoundary_ = lowerInitBoundary;
		 upperInitBoundary_ = upperInitBoundary;
	 }

	 /* ----------------------------------------------------------------------------------
	  * Tested GNumT<T>::specificTestsNoFailureExpected_GUnitTests()
	  * Setting of invalid boundaries is tested in GNumT<T>::specificTestsFailuresExpected_GUnitTests()
	  * ----------------------------------------------------------------------------------
	  */

	 /***************************************************************************/
	 /**
	  * Retrieves the value of the lower initialization boundary
	  *
	  * @return The value of the lower initialization boundary
	  */
	 T getLowerInitBoundary() const {
		 return lowerInitBoundary_;
	 }

	 /* ----------------------------------------------------------------------------------
	  * Tested GNumT<T>::specificTestsNoFailureExpected_GUnitTests()
	  * ----------------------------------------------------------------------------------
	  */

	 /***************************************************************************/
	 /**
	  * Retrieves the value of the upper initialization boundary
	  *
	  * @return The value of the upper initialization boundary
	  */
	 T getUpperInitBoundary() const {
		 return upperInitBoundary_;
	 }

	 /* ----------------------------------------------------------------------------------
	  * Tested GNumT<T>::specificTestsNoFailureExpected_GUnitTests()
	  * ----------------------------------------------------------------------------------
	  */

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
		 return std::string("GNumT");
	 }

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
		 ptr.put(baseName + ".lowerBoundary", this->getLowerInitBoundary());
		 ptr.put(baseName + ".upperBoundary", this->getUpperInitBoundary());
		 ptr.put(baseName + ".initRandom", false); // Unused for the creation of a property tree
		 ptr.put(baseName + ".adaptionsActive", this->adaptionsActive());
	 }

protected:
	 /***************************************************************************/
	 /**
	  * Loads the data of another GNumT<T> object,
	  * camouflaged as a GObject. We have no local data, so
	  * all we need to do is to the standard identity check,
	  * preventing that an object is assigned to itself.
	  *
	  * @param cp A copy of another GNumT<T> object, camouflaged as a GObject
	  */
	 void load_(const GObject *cp) override {
		 // Check that we are dealing with a GNumT<T> reference independent of this object and convert the pointer
		 const GNumT<T> *p_load = Gem::Common::g_convert_and_compare<GObject, GNumT<T>>(cp, this);

		 // Load our parent class'es data ...
		 GParameterT<T>::load_(cp);

		 // ... and then our local data
		 lowerInitBoundary_ = p_load->lowerInitBoundary_;
		 upperInitBoundary_ = p_load->upperInitBoundary_;
	 }

	 /***************************************************************************/
	 /**
	  * Returns a "comparative range". This is e.g. used to make Gauss-adaption
	  * independent of a parameters value range
	  */
	 T range() const override {
		 return upperInitBoundary_ - lowerInitBoundary_;
	 }

	 /***************************************************************************/
	 /** @brief Triggers random initialization of the parameter */
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
	 T lowerInitBoundary_; ///< The lower boundary for random initialization
	 T upperInitBoundary_; ///< The upper boundary for random initialization

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
		 if(GParameterT<T>::modify_GUnitTests()) result = true;

		 return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		 Gem::Common::condnotset("GNumT<>::modify_GUnitTests", "GEM_TESTING");
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
		 GParameterT<T>::specificTestsNoFailureExpected_GUnitTests();

		 // A few settings
		 const T LOWERTESTINITVAL = T(1); // Do not choose a negative value as T might be an unsigned type
		 const T UPPERTESTINITVAL = T(3);

		 //------------------------------------------------------------------------------

		 { // Test setting and retrieval of initialization boundaries
			 std::shared_ptr<GNumT<T>> p_test = this->template clone<GNumT<T>>();

			 // Set the boundaries
			 BOOST_CHECK_NO_THROW(p_test->setInitBoundaries(LOWERTESTINITVAL, UPPERTESTINITVAL));

			 // Check that these values have indeed been assigned
			 BOOST_CHECK(p_test->getLowerInitBoundary() == LOWERTESTINITVAL);
			 BOOST_CHECK(p_test->getUpperInitBoundary() == UPPERTESTINITVAL);
		 }

		 //------------------------------------------------------------------------------

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		 Gem::Common::condnotset("GNumT<>::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
	 }

	 /***************************************************************************/
	 /**
	  * Performs self tests that are expected to fail. This is needed for testing purposes
	  */
	 void specificTestsFailuresExpected_GUnitTests() override {
#ifdef GEM_TESTING
		 // Call the parent classes' functions
		 GParameterT<T>::specificTestsFailuresExpected_GUnitTests();

		 // A few settings
		 const T LOWERTESTINITVAL = T(1); // Do not choose a negative value as T might be an unsigned type
		 const T UPPERTESTINITVAL = T(3);

		 //------------------------------------------------------------------------------

		 { // Check that assignement of initialization boundaries throws for invalid boundaries
			 std::shared_ptr<GNumT<T>> p_test = this->template clone<GNumT<T>>();

			 BOOST_CHECK_THROW(p_test->setInitBoundaries(UPPERTESTINITVAL, LOWERTESTINITVAL), gemfony_exception);
		 }

		 //------------------------------------------------------------------------------


#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		 Gem::Common::condnotset("GNumT<>::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
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
struct is_abstract<Gem::Geneva::GNumT<T>> : public boost::true_type {};
template<typename T>
struct is_abstract< const Gem::Geneva::GNumT<T>> : public boost::true_type {};
}
}

/******************************************************************************/

#endif /* GNUMT_HPP_ */
