/**
 * @file GFPNumCollectionT.hpp
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

#ifndef GNUMCOLLECTIONFPT_HPP_
#define GNUMCOLLECTIONFPT_HPP_


// Geneva header files go here
#include "geneva/GObject.hpp"
#include "geneva/GDoubleGaussAdaptor.hpp"
#include "geneva/GNumCollectionT.hpp"
#include "common/GExceptions.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * This class represents a collection of floating point values, all modified
 * using the same algorithm. The most likely type to be stored in this
 * class is a double.
 */
template <typename fp_type>
class GFPNumCollectionT
	: public GNumCollectionT<fp_type>
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int) {
		using boost::serialization::make_nvp;
		ar
			& make_nvp("GNumCollectionT_fpType", boost::serialization::base_object<GNumCollectionT<fp_type> >(*this));
	}
	///////////////////////////////////////////////////////////////////////

	// Make sure this class can only be instantiated if fp_type really is a floating point type
	BOOST_MPL_ASSERT((boost::is_floating_point<fp_type>));

public:
	/***************************************************************************/
	/**
	 * The default constructor.
	 */
	GFPNumCollectionT()
		: GNumCollectionT<fp_type> ()
	{ /* nothing */ }

	/***************************************************************************/
	/**
	 * Initialization with a number of random values in a given range
	 *
	 * @param nval The amount of random values
	 * @param min The minimum random value
	 * @param max The maximum random value
	 */
	GFPNumCollectionT(
		const std::size_t& nval
		, const fp_type& min
		, const fp_type& max
	)
		: GNumCollectionT<fp_type>(nval, min, min, max) // The vector is preset to nval entries with value "min"
	{
		// Assign random values to each position
		typename GFPNumCollectionT<fp_type>::iterator it;
		for(it=this->begin(); it!=this->end(); ++it) {
			*it = this->GObject::gr_ptr()->Gem::Hap::GRandomBase::template uniform_real<fp_type>(min,max);
		}
	}

	/***************************************************************************/
	/**
	 * Initialization with a number of items of predefined value. We
	 * enforce setting of the lower and upper boundaries for random initialization,
	 * as these double up as the preferred value range in some optimization algorithms,
	 * such as swarm algorithms.
	 *
	 * @param nval The number of variables to be stored in the collection
	 * @param val  The value to be used for their initialization
	 */
	GFPNumCollectionT(
		const std::size_t& nval
		, const fp_type& val
		, const fp_type& min
		, const fp_type& max
	)
		: GNumCollectionT<fp_type>(nval, val, min, max)
	{ /* nothing */ }

	/***************************************************************************/
	/**
	 * The standard copy constructor
	 */
	GFPNumCollectionT(const GFPNumCollectionT<fp_type>& cp)
		: GNumCollectionT<fp_type> (cp)
	{ /* nothing */ }

	/***************************************************************************/
	/**
	 * The standard destructor
	 */
	virtual ~GFPNumCollectionT()
	{ /* nothing */ }

	/***************************************************************************/
	/**
	 * The standard assignment operator
	 */
	const GFPNumCollectionT<fp_type>& operator=(const GFPNumCollectionT<fp_type>& cp) {
		this->load_(&cp);
		return *this;
	}

	/***************************************************************************/
	/**
	 * Checks for equality with another GFPNumCollectionT<fp_type> object
	 *
	 * @param  cp A constant reference to another GFPNumCollectionT<fp_type> object
	 * @return A boolean indicating whether both objects are equal
	 */
	bool operator==(const GFPNumCollectionT<fp_type>& cp) const {
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
	 * Checks for inequality with another GFPNumCollectionT<fp_type> object
	 *
	 * @param  cp A constant reference to another GFPNumCollectionT<fp_type> object
	 * @return A boolean indicating whether both objects are inequal
	 */
	bool operator!=(const GFPNumCollectionT<fp_type>& cp) const {
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

		// Check that we are dealing with a GFPNumCollectionT<fp_type> reference independent of this object and convert the pointer
		const GFPNumCollectionT<fp_type> *p_load = Gem::Common::g_convert_and_compare<GObject, GFPNumCollectionT<fp_type> >(cp, this);

		GToken token("GFPNumCollectionT<fp_type>", e);

		// Compare our parent data ...
		Gem::Common::compare_base<GNumCollectionT<fp_type> >(IDENTITY(*this, *p_load), token);

		// ... no local data

		// React on deviations from the expectation
		token.evaluate();
	}

	/***************************************************************************/
	/**
	 * Emits a name for this class / object
	 */
	virtual std::string name() const override {
		return std::string("GFPNumCollectionT");
	}

protected:
	/***************************************************************************/
	/**
	 * Loads the data of another GFPNumCollectionT<fp_type> object,
	 * camouflaged as a GObject. We have no local data, so
	 * all we need to do is to the standard identity check,
	 * preventing that an object is assigned to itself.
	 *
	 * @param cp A copy of another GFPNumCollectionT<fp_type> object, camouflaged as a GObject
	 */
	virtual void load_(const GObject *cp) override {
		// Check that we are dealing with a GFPNumCollectionT<fp_type> reference independent of this object and convert the pointer
		const GFPNumCollectionT<fp_type> *p_load = Gem::Common::g_convert_and_compare<GObject, GFPNumCollectionT<fp_type> >(cp, this);

		// Load our parent class'es data ...
		GNumCollectionT<fp_type>::load_(cp);

		// no local data ...
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
	/**
	 * Triggers random initialization of the parameter collection. Note
	 * that this function assumes that the collection has been completely
	 * set up. Data that is added later will remain unaffected.
	 */
	virtual bool randomInit_(const activityMode& am) override {
		bool randomized = false;

		fp_type lowerBoundary = GNumCollectionT<fp_type>::getLowerInitBoundary();
		fp_type upperBoundary = GNumCollectionT<fp_type>::getUpperInitBoundary();

		typename GFPNumCollectionT<fp_type>::iterator it;
		for(it=this->begin(); it!=this->end(); ++it) {
			(*it)=this->GObject::gr_ptr()->Gem::Hap::GRandomBase::template uniform_real<fp_type>(lowerBoundary, upperBoundary);
			randomized = true;
		}

		return randomized;
	}

	/* ----------------------------------------------------------------------------------
	 * Tested in GFPNumCollectionT<fp_type>::specificTestsNoFailuresExpected_GUnitTests()
	 * ----------------------------------------------------------------------------------
	 */

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
		if(GNumCollectionT<fp_type>::modify_GUnitTests()) result = true;

		return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		condnotset("GFPNumCollectionT::modify_GUnitTests", "GEM_TESTING");
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
		GNumCollectionT<fp_type>::specificTestsNoFailureExpected_GUnitTests();

		// A few settings
		const std::size_t nItems = 100;
		const fp_type LOWERINITBOUNDARY = -10.1;
		const fp_type UPPERINITBOUNDARY =  10.1;
		const fp_type FIXEDVALUEINIT = 1.;
		const fp_type MULTVALUE = 3.;
		const fp_type RANDLOWERBOUNDARY = 0.;
		const fp_type RANDUPPERBOUNDARY = 10.;

		//------------------------------------------------------------------------------

		{ // Check initialization with a fixed value, setting and retrieval of boundaries and random initialization
			std::shared_ptr<GFPNumCollectionT<fp_type> > p_test1 = this->GObject::clone<GFPNumCollectionT<fp_type> >();
			std::shared_ptr<GFPNumCollectionT<fp_type> > p_test2 = this->GObject::clone<GFPNumCollectionT<fp_type> >();

			// Make sure p_test1 and p_test2 are empty
			BOOST_CHECK_NO_THROW(p_test1->clear());
			BOOST_CHECK_NO_THROW(p_test2->clear());

			// Add a few items
			for(std::size_t i=0; i<nItems; i++) {
				p_test1->push_back(fp_type(0));
				p_test2->push_back(fp_type(0));
			}

			// Initialize with a fixed value
			BOOST_CHECK_NO_THROW(p_test1->GParameterBase::template fixedValueInit<fp_type>(FIXEDVALUEINIT, ALLPARAMETERS));
			BOOST_CHECK_NO_THROW(p_test2->GParameterBase::template fixedValueInit<fp_type>(FIXEDVALUEINIT, ALLPARAMETERS));

			// Check that values have indeed been set
			for(std::size_t i=0; i<nItems; i++) {
				BOOST_CHECK(p_test1->at(i) == FIXEDVALUEINIT);
				BOOST_CHECK(p_test2->at(i) == FIXEDVALUEINIT);
			}

			// Set initialization boundaries
			BOOST_CHECK_NO_THROW(p_test1->setInitBoundaries(LOWERINITBOUNDARY, UPPERINITBOUNDARY));
			BOOST_CHECK_NO_THROW(p_test2->setInitBoundaries(LOWERINITBOUNDARY, UPPERINITBOUNDARY));

			// Randomly initialize one of the two objects. Note: we are using the protected function rather than the "global" function
			BOOST_CHECK_NO_THROW(p_test1->randomInit_(ALLPARAMETERS));

			// Check that the object has indeed changed
			BOOST_CHECK(*p_test1 != *p_test2);

			// Check that each value is different and that the values of p_test1 are inside of the allowed boundaroes
			for(std::size_t i=0; i<nItems; i++) {
				BOOST_CHECK(p_test1->at(i) != p_test2->at(i));
				BOOST_CHECK(p_test1->at(i) >= LOWERINITBOUNDARY);
				BOOST_CHECK(p_test1->at(i) <= UPPERINITBOUNDARY);
			}
		}

		//------------------------------------------------------------------------------

		{ // Test multiplication with a fixed value
			std::shared_ptr<GFPNumCollectionT<fp_type> > p_test1 = this->GObject::clone<GFPNumCollectionT<fp_type> >();
			std::shared_ptr<GFPNumCollectionT<fp_type> > p_test2 = this->GObject::clone<GFPNumCollectionT<fp_type> >();

			// Make sure p_test1 and p_test2 are empty
			BOOST_CHECK_NO_THROW(p_test1->clear());

			// Add a few items
			for(std::size_t i=0; i<nItems; i++) {
				p_test1->push_back(fp_type(0));
			}

			// Initialize with a fixed value
			BOOST_CHECK_NO_THROW(p_test1->GParameterBase::template fixedValueInit<fp_type>(FIXEDVALUEINIT, ALLPARAMETERS));

			// Set initialization boundaries
			BOOST_CHECK_NO_THROW(p_test1->setInitBoundaries(LOWERINITBOUNDARY, UPPERINITBOUNDARY));

			// Randomly initialize one of the two objects. Note: we are using the protected function rather than the "global" function
			BOOST_CHECK_NO_THROW(p_test1->randomInit_(ALLPARAMETERS));

			// Load the data into p_test2 and check that both objects are equal
			BOOST_CHECK_NO_THROW(p_test2->load(p_test1));
			BOOST_CHECK(*p_test1 == *p_test2);

			// Multiply p_test1 with a fixed value
			BOOST_CHECK_NO_THROW(p_test1->GParameterBase::template multiplyBy<fp_type>(MULTVALUE, ALLPARAMETERS));

			// Check that the multiplication has succeeded
			for(std::size_t i=0; i<nItems; i++) {
				BOOST_CHECK(p_test1->at(i) == MULTVALUE * p_test2->at(i));
			}
		}

		//------------------------------------------------------------------------------

		{ // Test multiplication with a random value in fixed range
			std::shared_ptr<GFPNumCollectionT<fp_type> > p_test1 = this->GObject::clone<GFPNumCollectionT<fp_type> >();
			std::shared_ptr<GFPNumCollectionT<fp_type> > p_test2 = this->GObject::clone<GFPNumCollectionT<fp_type> >();

			// Make sure p_test1 and p_test2 are empty
			BOOST_CHECK_NO_THROW(p_test1->clear());

			// Add a few items
			for(std::size_t i=0; i<nItems; i++) {
				p_test1->push_back(fp_type(0));
			}

			// Initialize with a fixed value
			BOOST_CHECK_NO_THROW(p_test1->GParameterBase::template fixedValueInit<fp_type>(1., ALLPARAMETERS));

			// Multiply with random values in a given range
			BOOST_CHECK_NO_THROW(p_test1->GParameterBase::template multiplyByRandom<fp_type>(RANDLOWERBOUNDARY, RANDUPPERBOUNDARY, ALLPARAMETERS));

			// Check that all values are in this range
			for(std::size_t i=0; i<nItems; i++) {
				BOOST_CHECK(p_test1->at(i) >= RANDLOWERBOUNDARY);
				BOOST_CHECK(p_test1->at(i) <= RANDUPPERBOUNDARY);
			}
		}

		//------------------------------------------------------------------------------

		{ // Test multiplication with a random value in the range [0:1[
			std::shared_ptr<GFPNumCollectionT<fp_type> > p_test1 = this->GObject::clone<GFPNumCollectionT<fp_type> >();
			std::shared_ptr<GFPNumCollectionT<fp_type> > p_test2 = this->GObject::clone<GFPNumCollectionT<fp_type> >();

			// Make sure p_test1 and p_test2 are empty
			BOOST_CHECK_NO_THROW(p_test1->clear());

			// Add a few items
			for(std::size_t i=0; i<nItems; i++) {
				p_test1->push_back(fp_type(0));
			}

			// Initialize with a fixed value
			BOOST_CHECK_NO_THROW(p_test1->GParameterBase::template fixedValueInit<fp_type>(1., ALLPARAMETERS));

			// Multiply with random values in a given range
			BOOST_CHECK_NO_THROW(p_test1->GParameterBase::template multiplyByRandom<fp_type>(ALLPARAMETERS));

			// Check that all values are in this range
			for(std::size_t i=0; i<nItems; i++) {
				BOOST_CHECK(p_test1->at(i) >= 0.);
				BOOST_CHECK(p_test1->at(i) <= 1.);
			}
		}

		//------------------------------------------------------------------------------

		{ // Test addition of other GFPNumCollectionT<fp_type> objets
			std::shared_ptr<GFPNumCollectionT<fp_type> > p_test1 = this->GObject::clone<GFPNumCollectionT<fp_type> >();
			std::shared_ptr<GFPNumCollectionT<fp_type> > p_test2 = this->GObject::clone<GFPNumCollectionT<fp_type> >();
			std::shared_ptr<GFPNumCollectionT<fp_type> > p_test3 = this->GObject::clone<GFPNumCollectionT<fp_type> >();

			// Make sure all clones are empty
			BOOST_CHECK_NO_THROW(p_test1->clear());
			BOOST_CHECK_NO_THROW(p_test2->clear());
			BOOST_CHECK_NO_THROW(p_test3->clear());

			// Add a few items
			for(std::size_t i=0; i<nItems; i++) {
				p_test1->push_back(fp_type(0));
			}

			// Set initialization boundaries
			BOOST_CHECK_NO_THROW(p_test1->setInitBoundaries(LOWERINITBOUNDARY, UPPERINITBOUNDARY));

			// Load the data of p_test_1 into p_test2
			BOOST_CHECK_NO_THROW(p_test2->load(p_test1));

			// Randomly initialize p_test1 and p_test2, so that both objects are different
			BOOST_CHECK_NO_THROW(p_test1->randomInit_(ALLPARAMETERS));
			BOOST_CHECK_NO_THROW(p_test2->randomInit_(ALLPARAMETERS));

			// Check that they are indeed different
			BOOST_CHECK(*p_test1 != *p_test2);

			// Load p_test2's data into p_test_3
			BOOST_CHECK_NO_THROW(p_test3->load(p_test2));

			// Add p_test1 to p_test3
			BOOST_CHECK_NO_THROW(p_test3->GParameterBase::template add<fp_type>(p_test1, ALLPARAMETERS));

			// Cross check that for each i p_test3[i] == p_test1[i] + p_test2[i]
			for(std::size_t i=0; i<nItems; i++) {
				BOOST_CHECK(p_test3->at(i) == p_test1->at(i) + p_test2->at(i));
			}
		}

		//------------------------------------------------------------------------------

		{ // Test subtraction of other GFPNumCollectionT<fp_type> objets
			std::shared_ptr<GFPNumCollectionT<fp_type> > p_test1 = this->GObject::clone<GFPNumCollectionT<fp_type> >();
			std::shared_ptr<GFPNumCollectionT<fp_type> > p_test2 = this->GObject::clone<GFPNumCollectionT<fp_type> >();
			std::shared_ptr<GFPNumCollectionT<fp_type> > p_test3 = this->GObject::clone<GFPNumCollectionT<fp_type> >();

			// Make sure all clones are empty
			BOOST_CHECK_NO_THROW(p_test1->clear());
			BOOST_CHECK_NO_THROW(p_test2->clear());
			BOOST_CHECK_NO_THROW(p_test3->clear());

			// Add a few items
			for(std::size_t i=0; i<nItems; i++) {
				p_test1->push_back(fp_type(0));
			}

			// Set initialization boundaries
			BOOST_CHECK_NO_THROW(p_test1->setInitBoundaries(LOWERINITBOUNDARY, UPPERINITBOUNDARY));

			// Load the data of p_test_1 into p_test2
			BOOST_CHECK_NO_THROW(p_test2->load(p_test1));

			// Randomly initialize p_test1 and p_test2, so that both objects are different
			BOOST_CHECK_NO_THROW(p_test1->randomInit_(ALLPARAMETERS));
			BOOST_CHECK_NO_THROW(p_test2->randomInit_(ALLPARAMETERS));

			// Check that they are indeed different
			BOOST_CHECK(*p_test1 != *p_test2);

			// Load p_test2's data into p_test_3
			BOOST_CHECK_NO_THROW(p_test3->load(p_test2));

			// Add p_test1 to p_test3
			BOOST_CHECK_NO_THROW(p_test3->GParameterBase::template subtract<fp_type>(p_test1, ALLPARAMETERS));

			// Cross check that for each i p_test3[i] == p_test1[i] - p_test2[i]
			for(std::size_t i=0; i<nItems; i++) {
				BOOST_CHECK(p_test3->at(i) == p_test2->at(i) - p_test1->at(i));
			}
		}

		//------------------------------------------------------------------------------

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
		condnotset("GFPNumCollectionT<>::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
	}

	/***************************************************************************/
	/**
	 * Performs self tests that are expected to fail. This is needed for testing purposes
	 */
	virtual void specificTestsFailuresExpected_GUnitTests() override {
#ifdef GEM_TESTING
		// A few settings
		const std::size_t nItems = 100;

		// Call the parent classes' functions
		GNumCollectionT<fp_type>::specificTestsFailuresExpected_GUnitTests();

		//------------------------------------------------------------------------------

		{ // Check that adding another object of different size throws
			std::shared_ptr<GFPNumCollectionT<fp_type> > p_test1 = this->GObject::clone<GFPNumCollectionT<fp_type> >();
			std::shared_ptr<GFPNumCollectionT<fp_type> > p_test2 = this->GObject::clone<GFPNumCollectionT<fp_type> >();

			// Add a few items to p_test1, but not to p_test2
			for(std::size_t i=0; i<nItems; i++) {
				p_test1->push_back(fp_type(0));
			}

			BOOST_CHECK_THROW(p_test1->GParameterBase::template add<fp_type>(p_test2, ALLPARAMETERS), Gem::Common::gemfony_error_condition);
		}

		//------------------------------------------------------------------------------

		{ // Check that subtracting another object of different size throws
			std::shared_ptr<GFPNumCollectionT<fp_type> > p_test1 = this->GObject::clone<GFPNumCollectionT<fp_type> >();
			std::shared_ptr<GFPNumCollectionT<fp_type> > p_test2 = this->GObject::clone<GFPNumCollectionT<fp_type> >();

			// Add a few items to p_test1, but not to p_test2
			for(std::size_t i=0; i<nItems; i++) {
				p_test1->push_back(fp_type(0));
			}

			BOOST_CHECK_THROW(p_test1->GParameterBase::template subtract<fp_type>(p_test2, ALLPARAMETERS), Gem::Common::gemfony_error_condition);
		}

		//------------------------------------------------------------------------------

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
		condnotset("GFPNumCollectionT<>::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
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
template<typename fp_type>
struct is_abstract<Gem::Geneva::GFPNumCollectionT<fp_type> > : public boost::true_type {};
template<typename fp_type>
struct is_abstract< const Gem::Geneva::GFPNumCollectionT<fp_type> > : public boost::true_type {};
}
}
/******************************************************************************/

#endif /* GNUMCOLLECTIONFPT_HPP_ */
