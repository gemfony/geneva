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

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard header files go here

// Boost header files go here

#ifndef GNUMCOLLECTIONT_HPP_
#define GNUMCOLLECTIONT_HPP_

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
 * class are double and boost::int32_t . By using the framework provided
 * by GParameterCollectionT, this class becomes rather simple.
 */
template <typename T>
class GNumCollectionT
	: public GParameterCollectionT<T>
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int) {
		using boost::serialization::make_nvp;
		ar
		& make_nvp("GParameterCollectionT",	boost::serialization::base_object<GParameterCollectionT<T>>(*this))
		& BOOST_SERIALIZATION_NVP(lowerInitBoundary_)
		& BOOST_SERIALIZATION_NVP(upperInitBoundary_);
	}
	///////////////////////////////////////////////////////////////////////

	// Make sure this class can only be instantiated with T as an arithmetic type
	BOOST_MPL_ASSERT((boost::is_arithmetic<T>));

public:
	/** @brief Specifies the type of parameters stored in this collection */
	typedef T collection_type;

	/***************************************************************************/
	/**
	 * The default constructor.
	 */
	GNumCollectionT()
		: GParameterCollectionT<T> ()
		, lowerInitBoundary_(T(DEFAULTLOWERINITBOUNDARYCOLLECTION))
		, upperInitBoundary_(T(DEFAULTUPPERINITBOUNDARYCOLLECTION))
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
		, const T& min
		, const T& max
		, typename boost::enable_if<boost::is_arithmetic<T>>::type* dummy = 0
	)
		: GParameterCollectionT<T> (nval, min)
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
		, const T& val
		, const T& min
		, const T& max
		, typename boost::enable_if<boost::is_arithmetic<T>>::type* dummy = 0
	)
		: GParameterCollectionT<T> (nval, val)
		, lowerInitBoundary_(min)
		, upperInitBoundary_(max)
	{ /* nothing */ }

	/***************************************************************************/
	/**
	 * The standard copy constructor
	 */
	GNumCollectionT(const GNumCollectionT<T>& cp)
		: GParameterCollectionT<T> (cp)
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
	 * The standard assignment operator
	 */
	const GNumCollectionT<T>& operator=(const GNumCollectionT<T>& cp) {
		this->load_(&cp);
		return *this;
	}

	/***************************************************************************/
	/**
	 * Checks for equality with another GNumCollectionT<T> object
	 *
	 * @param  cp A constant reference to another GNumCollectionT<T> object
	 * @return A boolean indicating whether both objects are equal
	 */
	bool operator==(const GNumCollectionT<T>& cp) const {
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
	 * Checks for inequality with another GNumCollectionT<T> object
	 *
	 * @param  cp A constant reference to another GNumCollectionT<T> object
	 * @return A boolean indicating whether both objects are inequal
	 */
	bool operator!=(const GNumCollectionT<T>& cp) const {
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

		// Check that we are dealing with a GNumCollectionT<T> reference independent of this object and convert the pointer
		const GNumCollectionT<T> *p_load = Gem::Common::g_convert_and_compare<GObject, GNumCollectionT<T>>(cp, this);

		GToken token("GNumCollectionT<T>", e);

		// Compare our parent data ...
		Gem::Common::compare_base<GParameterCollectionT<T>>(IDENTITY(*this, *p_load), token);

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
		const T& lowerInitBoundary
		, const T& upperInitBoundary
		, typename boost::enable_if<boost::is_arithmetic<T>>::type* dummy = 0
	) {
		// Do some error checking
		if(lowerInitBoundary >= upperInitBoundary) {
			glogger
			<< "In GNumCollectionT<T>::setInitBoundaries():" << std::endl
			<< "Invalid boundaries provided: " << std::endl
			<< "lowerInitBoundary = " << lowerInitBoundary << std::endl
			<< "upperInitBoundary = " << upperInitBoundary << std::endl
			<< GEXCEPTION;
		}

		lowerInitBoundary_ = lowerInitBoundary;
		upperInitBoundary_ = upperInitBoundary;
	}

	/* ----------------------------------------------------------------------------------
	 * Tested GFPNumCollectionT<T>::specificTestsNoFailureExpected_GUnitTests()
	 * Tested GNumCollectionT<T>::specificTestsNoFailureExpected_GUnitTests()
	 * Assignment of invalid boundaries Tested GNumCollectionT<T>::specificTestsFailuresExpected_GUnitTests()
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
	 * Tested GFPNumCollectionT<T>::specificTestsNoFailureExpected_GUnitTests()
	 * Tested GNumCollectionT<T>::specificTestsNoFailureExpected_GUnitTests()
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
	 * Tested GFPNumCollectionT<T>::specificTestsNoFailureExpected_GUnitTests()
	 * Tested GNumCollectionT<T>::specificTestsNoFailureExpected_GUnitTests()
	 * ----------------------------------------------------------------------------------
	 */

	/***************************************************************************/
	/**
	 * Emits a name for this class / object
	 */
	virtual std::string name() const override {
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
         glogger
         << "In GNumCollection<T>::toPropertyTree(): Error!" << std::endl
         << "Object is empty!" << std::endl
         << GEXCEPTION;
      }
#endif /* DEBUG */

		ptr.put(baseName + ".name", this->getParameterName());
		ptr.put(baseName + ".type", this->name());
		ptr.put(baseName + ".baseType", Gem::Common::GTypeToStringT<T>::value());
		ptr.put(baseName + ".isLeaf", this->isLeaf());
		ptr.put(baseName + ".nVals", this->size());

		typename GNumCollectionT<T>::const_iterator cit;
		std::size_t pos;
		for(cit=this->begin(); cit!=this->end(); ++cit) {
			pos = cit - this->begin();
			ptr.put(baseName + "values.value" + boost::lexical_cast<std::string>(pos), *cit);
		}
		ptr.put(baseName + ".lowerBoundary", this->getLowerInitBoundary());
		ptr.put(baseName + ".upperBoundary", this->getUpperInitBoundary());
		ptr.put(baseName + ".initRandom", false); // Unused for the creation of a property tree
		ptr.put(baseName + ".adaptionsActive", this->adaptionsActive());
	}

protected:
	/***************************************************************************/
	/**
	 * Loads the data of another GNumCollectionT<T> object,
	 * camouflaged as a GObject. We have no local data, so
	 * all we need to do is to the standard identity check,
	 * preventing that an object is assigned to itself.
	 *
	 * @param cp A copy of another GNumCollectionT<T> object, camouflaged as a GObject
	 */
	virtual void load_(const GObject *cp) override {
		// Check that we are dealing with a GNumCollectionT<T> reference independent of this object and convert the pointer
		const GNumCollectionT<T> *p_load = Gem::Common::g_convert_and_compare<GObject, GNumCollectionT<T>>(cp, this);

		// Load our parent class'es data ...
		GParameterCollectionT<T>::load_(cp);

		// ... and then our local data
		lowerInitBoundary_ = p_load->lowerInitBoundary_;
		upperInitBoundary_ = p_load->upperInitBoundary_;
	}

	/***************************************************************************/
	/**
	 * Returns a "comparative range". This is e.g. used to make Gauss-adaption
	 * independent of a parameters value range
	 */
	virtual T range() const override {
		return upperInitBoundary_ - lowerInitBoundary_;
	}

	/***************************************************************************/
	/**
	 * Creates a deep copy of this object. Purely virtual as this class
	 * should not be instantiable.
	 *
	 * @return A pointer to a deep clone of this object
	 */
	virtual GObject *clone_() const override = 0;

	/***************************************************************************/
	/** @brief Triggers random initialization of the parameter collection */
	virtual bool randomInit_(const activityMode&) override = 0;

private:
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
	virtual bool modify_GUnitTests() override {
#ifdef GEM_TESTING
		bool result = false;

		// Call the parent classes' functions
		if(GParameterCollectionT<T>::modify_GUnitTests()) result = true;

		return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		condnotset("GNumCollectionT<>::modify_GUnitTests", "GEM_TESTING");
		return false;
#endif /* GEM_TESTING */
	}

	/***************************************************************************/
	/**
	 * Performs self tests that are expected to succeed. This is needed for testing purposes
	 */
	virtual void specificTestsNoFailureExpected_GUnitTests() override {
#ifdef GEM_TESTING
		// Call the parent classes' functions
		GParameterCollectionT<T>::specificTestsNoFailureExpected_GUnitTests();

		// A few settings
		const T LOWERTESTINITVAL = T(1); // Do not choose a negative value as T might be an unsigned type
		const T UPPERTESTINITVAL = T(3);

		//------------------------------------------------------------------------------

		{ // Test setting and retrieval of initialization boundaries
			std::shared_ptr<GNumCollectionT<T>> p_test = this->GObject::template clone<GNumCollectionT<T>>();

			// Set the boundaries
			BOOST_CHECK_NO_THROW(p_test->setInitBoundaries(LOWERTESTINITVAL, UPPERTESTINITVAL));

			// Check that these values have indeed been assigned
			BOOST_CHECK(p_test->getLowerInitBoundary() == LOWERTESTINITVAL);
			BOOST_CHECK(p_test->getUpperInitBoundary() == UPPERTESTINITVAL);
		}

		//------------------------------------------------------------------------------

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
		condnotset("GNumCollectionT<>::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
	}

	/***************************************************************************/
	/**
	 * Performs self tests that are expected to fail. This is needed for testing purposes
	 */
	virtual void specificTestsFailuresExpected_GUnitTests() override {
#ifdef GEM_TESTING
		// Call the parent classes' functions
		GParameterCollectionT<T>::specificTestsFailuresExpected_GUnitTests();

		// A few settings
		const T LOWERTESTINITVAL = T(1); // Do not choose a negative value as T might be an unsigned type
		const T UPPERTESTINITVAL = T(3);

		//------------------------------------------------------------------------------

		{ // Check that assignement of initialization boundaries throws for invalid boundaries
			std::shared_ptr<GNumCollectionT<T>> p_test = this->GObject::template clone<GNumCollectionT<T>>();

			BOOST_CHECK_THROW(p_test->setInitBoundaries(UPPERTESTINITVAL, LOWERTESTINITVAL), Gem::Common::gemfony_error_condition);
		}

		//------------------------------------------------------------------------------

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
		condnotset("GNumCollectionT<>::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
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
template<typename T>
struct is_abstract<Gem::Geneva::GNumCollectionT<T>> : public boost::true_type {};
template<typename T>
struct is_abstract< const Gem::Geneva::GNumCollectionT<T>> : public boost::true_type {};
}
}
/******************************************************************************/

#endif /* GNUMCOLLECTIONT_HPP_ */
