/**
 * @file GNumFPT.hpp
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

// Boost headers go here

#ifndef GNUMFPT_HPP_
#define GNUMFPT_HPP_

// Geneva headers go here
#include "geneva/GNumT.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * This class represents floating point values. The most likely type to be stored
 * in this class is a double. It adds floating point initialization and multiplication
 * to GNumT
 */
template <typename fp_type>
class GNumFPT
	: public GNumT<fp_type>
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int) {
		using boost::serialization::make_nvp;
		ar & make_nvp("GNumT",	boost::serialization::base_object<GNumT<fp_type>>(*this));
	}
	///////////////////////////////////////////////////////////////////////

	// Make sure this class can only be instantiated if fp_type really is a floating point type
	BOOST_MPL_ASSERT((boost::is_floating_point<fp_type>));

public:
	/** @brief Specifies the type of parameters stored in this object */
	typedef fp_type parameter_type;

	/***************************************************************************/
	/**
	 * The default constructor.
	 */
	GNumFPT()
		: GNumT<fp_type> ()
	{ /* nothing */ }

	/***************************************************************************/
	/*
	 * Initialize with a single value
	 *
	 * @param val The value used for the initialization
	 */
	explicit GNumFPT(const fp_type& val)
		: GNumT<fp_type>(val)
	{ /* nothing */ }

	/***************************************************************************/
	/**
	 * Initialize with a random value within given boundaries. Note that
	 * we use the local randomInit_ function in order not to call any
	 * purely virtual functions.
	 *
	 * @param min The lower boundary for random entries
	 * @param max The upper boundary for random entries
	 */
	GNumFPT (
		const fp_type& min
		, const fp_type& max
	)
		: GNumT<fp_type> (min, max)
	{
		GNumFPT<fp_type>::randomInit(ACTIVEONLY);
	}

	/***************************************************************************/
	/**
	 * Initialize with a fixed value. Note that we enforce initialization
	 * of the initialization boundaries as well, as these may play a role
	 * in some optimization algorithms. Note that we do not enforce that the
	 * assigned value lies inside these boundaries, as they are meant for
	 * random initialization only.
	 *
	 * @param val The value to be assigned to the object
	 * @param min The lower boundary for random entries
	 * @param max The upper boundary for random entries
	 */
	GNumFPT (
		const fp_type& val
		, const fp_type& min
		, const fp_type& max
	)
		: GNumT<fp_type> (min, max)
	{
		GParameterT<fp_type>::setValue(val);
	}

	/***************************************************************************/
	/**
	 * The standard copy constructor
	 *
	 * @param cp A constant reference to another GNumFPT<fp_type> object
	 */
	GNumFPT(const GNumFPT<fp_type>& cp)
		: GNumT<fp_type> (cp)
	{ /* nothing */ }

	/***************************************************************************/
	/**
	 * The standard destructor
	 */
	virtual ~GNumFPT()
	{ /* nothing */ }

	/***************************************************************************/
	/**
	 * The standard assignment operator
	 */
	const GNumFPT<fp_type>& operator=(const GNumFPT<fp_type>& cp) {
		this->load_(&cp);
		return *this;
	}

	/***************************************************************************/
	/**
	 * An assignment operator for the contained value type
	 *
	 * @param val The value to be assigned to this object
	 * @return The value that was assigned to this object
	 */
	virtual fp_type operator=(const fp_type& val) override {
		return GNumT<fp_type>::operator=(val);
	}

	/***************************************************************************/
	/**
	 * Checks for equality with another GNumFPT<fp_type> object
	 *
	 * @param  cp A constant reference to another GNumFPT<fp_type> object
	 * @return A boolean indicating whether both objects are equal
	 */
	bool operator==(const GNumFPT<fp_type>& cp) const {
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
	 * Checks for inequality with another GNumFPT<fp_type> object
	 *
	 * @param  cp A constant reference to another GNumFPT<fp_type> object
	 * @return A boolean indicating whether both objects are inequal
	 */
	bool operator!=(const GNumFPT<fp_type>& cp) const {
		using namespace Gem::Common;
		try {
			this->compare(cp, CE_INEQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
			return true;
		} catch(g_expectation_violation& g) {
			glogger
			<< g << GLOGGING;
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

		// Check that we are dealing with a GNumFPT<fp_type> reference independent of this object and convert the pointer
		const GNumFPT<fp_type> *p_load = Gem::Common::g_convert_and_compare<GObject, GNumFPT<fp_type>>(cp, this);

		GToken token("GNumFPT<fp_type>", e);

		// Compare our parent data ...
		Gem::Common::compare_base<GNumT<fp_type>>(IDENTITY(*this, *p_load), token);

		// ... no local data

		// React on deviations from the expectation
		token.evaluate();
	}

	/***************************************************************************/
	/**
	 * Emits a name for this class / object
	 */
	virtual std::string name() const override {
		return std::string("GNumFPT");
	}

protected:
	/***************************************************************************/
	/**
	 * Loads the data of another GNumFPT<fp_type> object,
	 * camouflaged as a GObject. We have no local data, so
	 * all we need to do is to the standard identity check,
	 * preventing that an object is assigned to itself.
	 *
	 * @param cp A copy of another GNumFPT<fp_type> object, camouflaged as a GObject
	 */
	virtual void load_(const GObject *cp) override {
		// Check that we are dealing with a GNumFPT<fp_type> reference independent of this object and convert the pointer
		const GNumFPT<fp_type> *p_load = Gem::Common::g_convert_and_compare<GObject, GNumFPT<fp_type>>(cp, this);

		// Load our parent class'es data ...
		GNumT<fp_type>::load_(cp);

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
	 * Triggers random initialization of the parameter
	 */
	virtual bool randomInit_(const activityMode&) override {
		fp_type lowerBoundary = GNumT<fp_type>::getLowerInitBoundary();
		fp_type upperBoundary = GNumT<fp_type>::getUpperInitBoundary();
		GParameterT<fp_type>::setValue(
			this->GObject::gr_ptr()->Gem::Hap::GRandomBase::template uniform_real<fp_type>(lowerBoundary, upperBoundary)
		);

		return true;
	}

	/* ----------------------------------------------------------------------------------
	 * Tested in GNumFPT<fp_type>::specificTestsNoFailuresExpected_GUnitTests()
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
		if(GNumT<fp_type>::modify_GUnitTests()) result = true;

		return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		condnotset("GNumFPT<>::modify_GUnitTests", "GEM_TESTING");
		return false;
#endif /* GEM_TESTING */
	}

	/***************************************************************************/
	/**
	 * Performs self tests that are expected to succeed. This is needed for testing purposes
	 */
	virtual void specificTestsNoFailureExpected_GUnitTests() override {
#ifdef GEM_TESTING
		// A few settings
		const std::size_t nTests = 100;
		const fp_type LOWERINITBOUNDARY = -10.1;
		const fp_type UPPERINITBOUNDARY =  10.1;
		const fp_type FIXEDVALUEINIT = 1.;
		const fp_type MULTVALUE = 3.;
		const fp_type RANDLOWERBOUNDARY = 0.;
		const fp_type RANDUPPERBOUNDARY = 10.;

		// Call the parent classes' functions
		GNumT<fp_type>::specificTestsNoFailureExpected_GUnitTests();

		//------------------------------------------------------------------------------

		{ // Check initialization with a fixed value, setting and retrieval of boundaries and random initialization
			std::shared_ptr<GNumFPT<fp_type>> p_test1 = this->GObject::clone<GNumFPT<fp_type>>();
			std::shared_ptr<GNumFPT<fp_type>> p_test2 = this->GObject::clone<GNumFPT<fp_type>>();

			// Assign a defined start value
			p_test1->setValue(fp_type(0));

			// Initialize with a fixed value
			BOOST_CHECK_NO_THROW(p_test1->GParameterBase::template fixedValueInit<fp_type>(boost::numeric_cast<fp_type>(2.*UPPERINITBOUNDARY), ALLPARAMETERS)); // Make sure the parameters indeed change

			// Check that the value has indeed been set.
			BOOST_CHECK_MESSAGE(
					fabs(p_test1->value() - fp_type(2.*UPPERINITBOUNDARY)) < pow(10., -6)
					,  "\n"
					<< std::setprecision(10)
					<< "p_test1->value() = " << p_test1->value() << "\n"
					<< "2.*UPPERINITBOUNDARY = " << 2.*UPPERINITBOUNDARY << "\n"
					<< "fabs(p_test1->value() - 2.*UPPERINITBOUNDARY) = " << fabs(p_test1->value() - 2.*UPPERINITBOUNDARY) << "\n"
					<< "pow(10., -8) = " << pow(10., -8) << "\n"
			);

			// Set initialization boundaries
			BOOST_CHECK_NO_THROW(p_test1->setInitBoundaries(LOWERINITBOUNDARY, UPPERINITBOUNDARY));

			// Cross-check the boundaries
			BOOST_CHECK(p_test1->getLowerInitBoundary() == LOWERINITBOUNDARY);
			BOOST_CHECK(p_test1->getUpperInitBoundary() == UPPERINITBOUNDARY);

			// Check that each value is different and that the values of p_test1 are inside of the allowed boundaries
			for(std::size_t i=0; i<nTests; i++) {
				// Load p_test1 into p_test2
				BOOST_CHECK_NO_THROW(p_test2->load(p_test1));
				// Cross-check that both objects are equal
				BOOST_CHECK(*p_test1 == *p_test2);

				// Randomly initialize one of the two objects. Note: we are using the protected function rather than the "global" function
				BOOST_CHECK_NO_THROW(p_test2->randomInit_(ALLPARAMETERS));

				// Check that the object has indeed changed
				BOOST_CHECK(*p_test2 != *p_test1);

				BOOST_CHECK(p_test2->value() != p_test1->value());
				BOOST_CHECK(p_test2->value() >= LOWERINITBOUNDARY);
				BOOST_CHECK(p_test2->value() <= UPPERINITBOUNDARY);
			}
		}

		//------------------------------------------------------------------------------

		{ // Test multiplication with a fixed value
			std::shared_ptr<GNumFPT<fp_type>> p_test1 = this->GObject::clone<GNumFPT<fp_type>>();
			std::shared_ptr<GNumFPT<fp_type>> p_test2 = this->GObject::clone<GNumFPT<fp_type>>();

			// Initialize with a fixed value
			BOOST_CHECK_NO_THROW(p_test1->GParameterBase::template fixedValueInit<fp_type>(FIXEDVALUEINIT, ALLPARAMETERS));

			// Check that this value has been set
			BOOST_CHECK(p_test1->value() == FIXEDVALUEINIT);

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
			BOOST_CHECK(p_test1->value() == MULTVALUE * p_test2->value());
		}

		//------------------------------------------------------------------------------

		{ // Test multiplication with a random value in fixed range
			std::shared_ptr<GNumFPT<fp_type>> p_test1 = this->GObject::clone<GNumFPT<fp_type>>();

			// Initialize with a fixed value
			BOOST_CHECK_NO_THROW(p_test1->GParameterBase::template fixedValueInit<fp_type>(1., ALLPARAMETERS)); // 1. chosen so we see the multiplication value of the random number generator

			// Check that this value has been set
			BOOST_CHECK(p_test1->value() == 1.);

			// Multiply with random values in a given range
			BOOST_CHECK_NO_THROW(p_test1->GParameterBase::template multiplyByRandom<fp_type>(RANDLOWERBOUNDARY, RANDUPPERBOUNDARY, ALLPARAMETERS));

			// Check that all values are in the allowed range
			BOOST_CHECK(p_test1->value() >= RANDLOWERBOUNDARY);
			BOOST_CHECK(p_test1->value() <= RANDUPPERBOUNDARY);
		}

		//------------------------------------------------------------------------------

		{ // Test multiplication with a random value in the range [0:1[
			std::shared_ptr<GNumFPT<fp_type>> p_test1 = this->GObject::clone<GNumFPT<fp_type>>();

			// Initialize with a fixed value
			BOOST_CHECK_NO_THROW(p_test1->GParameterBase::template fixedValueInit<fp_type>(1., ALLPARAMETERS)); // 1. chosen so we see the multiplication value of the random number generator

			// Check that this value has been set
			BOOST_CHECK(p_test1->value() == 1.);

			// Multiply with random values in a given range
			BOOST_CHECK_NO_THROW(p_test1->GParameterBase::template multiplyByRandom<fp_type>(ALLPARAMETERS));

			// Check that all values are in the allowed range
			BOOST_CHECK(p_test1->value() >= 0.);
			BOOST_CHECK(p_test1->value() <= 1.);
		}

		//------------------------------------------------------------------------------

		{ // Test addition of other GNumFPT<fp_type> objets
			std::shared_ptr<GNumFPT<fp_type>> p_test1 = this->GObject::clone<GNumFPT<fp_type>>();
			std::shared_ptr<GNumFPT<fp_type>> p_test2 = this->GObject::clone<GNumFPT<fp_type>>();
			std::shared_ptr<GNumFPT<fp_type>> p_test3 = this->GObject::clone<GNumFPT<fp_type>>();

			// Initialize with a fixed value
			BOOST_CHECK_NO_THROW(p_test1->GParameterBase::template fixedValueInit<fp_type>(0., ALLPARAMETERS));
			BOOST_CHECK(p_test1->value() == 0.);

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

			// Cross-check that the addition has worked
			BOOST_CHECK(p_test3->value() == p_test1->value() + p_test2->value());
		}

		//------------------------------------------------------------------------------

		{ // Test subtraction of other GNumFPT<fp_type> objects
			std::shared_ptr<GNumFPT<fp_type>> p_test1 = this->GObject::clone<GNumFPT<fp_type>>();
			std::shared_ptr<GNumFPT<fp_type>> p_test2 = this->GObject::clone<GNumFPT<fp_type>>();
			std::shared_ptr<GNumFPT<fp_type>> p_test3 = this->GObject::clone<GNumFPT<fp_type>>();

			// Initialize with a fixed value
			BOOST_CHECK_NO_THROW(p_test1->GParameterBase::template fixedValueInit<fp_type>(0., ALLPARAMETERS));
			BOOST_CHECK(p_test1->value() == 0.);

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

			// Subtract p_test1 from p_test3
			BOOST_CHECK_NO_THROW(p_test3->GParameterBase::template subtract<fp_type>(p_test1, ALLPARAMETERS));

			// Cross-check that the addition has worked. Note that we do need to take into
			// account effects of floating point accuracy
			BOOST_CHECK(p_test3->value() == (p_test2->value() - p_test1->value()));
		}

		//------------------------------------------------------------------------------

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		condnotset("GNumFPT<>::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
	}

	/***************************************************************************/
	/**
	 * Performs self tests that are expected to fail. This is needed for testing purposes
	 */
	virtual void specificTestsFailuresExpected_GUnitTests() override {
#ifdef GEM_TESTING
		// Call the parent classes' functions
		GNumT<fp_type>::specificTestsFailuresExpected_GUnitTests();

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		condnotset("GNumFPT<>::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
	}

	/***************************************************************************/
};

} /* namespace Geneva */
} /* namespace Gem */

/******************************************************************************/
// The content of BOOST_SERIALIZATION_ASSUME_ABSTRACT(T)

namespace boost {
namespace serialization {
template<typename fp_type>
struct is_abstract<Gem::Geneva::GNumFPT<fp_type>> : public boost::true_type {};
template<typename fp_type>
struct is_abstract< const Gem::Geneva::GNumFPT<fp_type>> : public boost::true_type {};
}
}
/******************************************************************************/

#endif /* GNUMFPT_HPP_ */
