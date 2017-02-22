/**
 * @file GConstrainedFPT.hpp
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
#include <cmath>
#include <cfloat>

// Boost headers go here
#include <boost/math/special_functions/next.hpp> // Needed so we can calculate the next representable value smaller than a given upper boundary

#ifndef GCONSTRAINEDFPT_HPP_
#define GCONSTRAINEDFPT_HPP_

// Geneva headers go here
#include "geneva/GObject.hpp"
#include "geneva/GConstrainedNumT.hpp"
#include "geneva/GParameterBase.hpp"
#include "common/GExceptions.hpp"
#include "common/GMathHelperFunctions.hpp"
#include "hap/GRandomBase.hpp"
#include "hap/GRandomT.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
/* The GConstrainedFPT class represents a floating point type, such as a double,
 * equipped with the ability to adapt itself. The value range can have an upper and a lower
 * limit.  Adapted values will only appear inside the given range to the user. Note that
 * appropriate adaptors (see e.g the GDoubleGaussAdaptor class) need to be loaded in order
 * to benefit from the adaption capabilities.
 */
template <typename fp_type>
class GConstrainedFPT
	:public GConstrainedNumT<fp_type>
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int) {
		using boost::serialization::make_nvp;

		// Save data
		ar
			& make_nvp("GConstrainedNumT_T", boost::serialization::base_object<GConstrainedNumT<fp_type>>(*this));
	}
	///////////////////////////////////////////////////////////////////////

	// Make sure this class can only be instantiated if fp_type really is a floating point type
	static_assert(
		std::is_floating_point<fp_type>::value
		, "fp_type must be a floating point type"
	);

public:
	/***************************************************************************/
	/**
	 * The default constructor.
	 */
	GConstrainedFPT()
		: GConstrainedNumT<fp_type>()
	{ /* nothing */ }

	/***************************************************************************/
	/**
	 * A constructor that initializes the value only. The boundaries will
	 * be set to the maximum and minimum values of the corresponding type.
	 *
	 * @param val The desired external value of this object
	 */
	explicit GConstrainedFPT (const fp_type& val)
		: GConstrainedNumT<fp_type>(val)
	{ /* nothing */ }

	/***************************************************************************/
	/**
	 * Initializes the boundaries and assigns a random value.
	 *
	 * @param lowerBoundary The lower boundary of the value range
	 * @param upperBoundary The upper boundary of the value range
	 */
	GConstrainedFPT (const fp_type& lowerBoundary , const fp_type& upperBoundary)
		: GConstrainedNumT<fp_type>(lowerBoundary, boost::math::float_prior<fp_type>(upperBoundary))
	{
		Gem::Hap::GRandomT<Gem::Hap::RANDFLAVOURS::RANDOMLOCAL> gr;
		typename std::uniform_real_distribution<fp_type> uniform_real_distribution(lowerBoundary, upperBoundary);
		GParameterT<fp_type>::setValue(uniform_real_distribution(gr));
	}

	/***************************************************************************/
	/**
	 * Initialization with value and boundaries. We need somewhat tighter
	 * constraints for the allowed value range than implemented in the
	 * parent class. Note that we take the liberty to adapt val, if it is
	 * equal to the unmodified upper boundary. Otherwise you will get an
	 * error, where what you likely really meant was to start with the
	 * upper boundary.
	 *
	 * @param val The desired value of this object
	 * @param lowerBoundary The lower boundary of the value range
	 * @param upperBoundary The upper boundary of the value range
	 */
	GConstrainedFPT (
		const fp_type& val
		, const fp_type& lowerBoundary
		, const fp_type& upperBoundary
	)
		: GConstrainedNumT<fp_type>(lowerBoundary, boost::math::float_prior<fp_type>(upperBoundary))
	{
		if(val == upperBoundary) {
			GConstrainedNumT<fp_type>::setValue(boost::math::float_prior<fp_type>(upperBoundary));
		} else {
			GConstrainedNumT<fp_type>::setValue(val);
		}
	}

	/***************************************************************************/
	/**
	 * A standard copy constructor. Most work is done by the parent
	 * classes.
	 *
	 * @param cp Another GConstrainedNumT<fp_type> object
	 */
	GConstrainedFPT (const GConstrainedFPT<fp_type>& cp)
		: GConstrainedNumT<fp_type>(cp)
	{ /* nothing */ }

	/***************************************************************************/
	/**
	 * The standard destructor
	 */
	virtual ~GConstrainedFPT()
	{ /* nothing */	}

	/***************************************************************************/
	/**
	 * The standard assignment operator
	 */
	const GConstrainedFPT<fp_type>& operator=(const GConstrainedFPT<fp_type>& cp) {
		this->load_(&cp);
		return *this;
	}

	/***************************************************************************/
	/**
	 * Checks for equality with another GConstrainedFPT<fp_type> object
	 *
	 * @param  cp A constant reference to another GConstrainedFPT<fp_type> object
	 * @return A boolean indicating whether both objects are equal
	 */
	bool operator==(const GConstrainedFPT<fp_type>& cp) const {
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
	 * Checks for inequality with another GConstrainedFPT<fp_type> object
	 *
	 * @param  cp A constant reference to another GConstrainedFPT<fp_type> object
	 * @return A boolean indicating whether both objects are inequal
	 */
	bool operator!=(const GConstrainedFPT<fp_type>& cp) const {
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

		// Check that we are dealing with a GConstrainedFPT<fp_type>  reference independent of this object and convert the pointer
		const GConstrainedFPT<fp_type>  *p_load = Gem::Common::g_convert_and_compare<GObject, GConstrainedFPT<fp_type>>(cp, this);

		GToken token("GConstrainedFPT<fp_type>", e);

		// Compare our parent data ...
		Gem::Common::compare_base<GConstrainedNumT<fp_type>>(IDENTITY(*this, *p_load), token);

		// React on deviations from the expectation
		token.evaluate();
	}

	/***************************************************************************/
	/**
	 * A standard assignment operator for T values.
	 *
	 * @param The desired new external value
	 * @return The new external value of this object
	 */
	virtual fp_type operator=(const fp_type& val) override {
		fp_type tmpVal = val;
		if(val==boost::math::float_next<fp_type>(this->getUpperBoundary())) {
			tmpVal = boost::math::float_prior<fp_type>(val);
		}

		return GConstrainedNumT<fp_type>::operator=(tmpVal);
	}

	/* ----------------------------------------------------------------------------------
	 * Throwing tested in GConstrainedFPT<fp_type>::specificTestsFailuresExpected_GUnitTests()
	 * Tested in GConstrainedFPT<fp_type>::specificTestsNoFailuresExpected_GUnitTests()
	 * ----------------------------------------------------------------------------------
	 */

	/***************************************************************************/
	/**
	 * Allows to set the value. Has the same constraints as the parent class'es function,
	 * applies additional restrictions.  Note that we take the liberty to adapt val, if it
	 * is equal to the unmodified upper boundary. Otherwise you will get an error,
	 * where what you likely really meant was to start with the upper boundary.
	 *
	 * @param val The new fp_type value stored in this class
	 */
	virtual void setValue(const fp_type& val) override {
		fp_type tmpVal = val;
		if(val==boost::math::float_next<fp_type>(this->getUpperBoundary())) {
			tmpVal = boost::math::float_prior<fp_type>(val);
		}

		GConstrainedNumT<fp_type>::setValue(tmpVal);
	}

	/* ----------------------------------------------------------------------------------
	 * Throwing tested in GConstrainedFPT<fp_type>::specificTestsFailuresExpected_GUnitTests()
	 * Tested in GConstrainedFPT<fp_type>::specificTestsNoFailuresExpected_GUnitTests()
	 * ----------------------------------------------------------------------------------
	 */

	/***************************************************************************/
	/**
	 * Allows to set the value of this object together with its boundaries. Note
	 * that we take the liberty to adapt val, if it is equal to the unmodified upper
	 * boundary. Otherwise you will get an error, where what you likely really meant
	 * was to start with the upper boundary.
	 *
	 * @param val The desired value of this object
	 * @param lowerBoundary The lower boundary of the value range
	 * @param upperBoundary The upper boundary of the value range
	 */
	virtual void setValue(
		const fp_type& val
		, const fp_type& lowerBoundary
		, const fp_type& upperBoundary
	) override {
		fp_type tmpVal = val;
		if(val==upperBoundary) {
			tmpVal = boost::math::float_prior<fp_type>(val);
		}

		GConstrainedNumT<fp_type>::setValue(
			tmpVal
			, lowerBoundary
			, boost::math::float_prior<fp_type>(upperBoundary)
		);
	}

	/* ----------------------------------------------------------------------------------
	 * Throwing tested in GConstrainedFPT<fp_type>::specificTestsFailuresExpected_GUnitTests()
	 * Tested in GConstrainedFPT<fp_type>::specificTestsNoFailuresExpected_GUnitTests()
	 * ----------------------------------------------------------------------------------
	 */

	/***************************************************************************/
	/**
	 * Sets the boundaries of this object. This function differs from the parent
	 * class'es function in that it calculates an additional quantity, the closed
	 * upper boundary (upper is assumed to be an open, i.e. non-inclusive boundary).
	 *
	 * @param lower The new lower boundary for this object
	 * @param upper The new upper boundary for this object
	 */
	virtual void setBoundaries(const fp_type& lowerBoundary, const fp_type& upperBoundary) override {
		// Set the actual boundaries
		GConstrainedNumT<fp_type>::setBoundaries(lowerBoundary, boost::math::float_prior<fp_type>(upperBoundary));
	}

	/***************************************************************************/
	/**
	 * The transfer function needed to calculate the externally visible value.
	 * Note that in GConstrainedNumT<>::value() val is shifted to the
	 * "mapping" value, so it doesn't get too large. This happens centrally,
	 * as it is also relevant for the integer case. We calculate in long double
	 * precision here in order to avoid as much as possible numeric instabilities.
	 * Likewise we use int64_t for the region.
	 *
	 * @param val The value to which the transformation should be applied
	 * @return The transformed value
	 */
	virtual fp_type transfer(const fp_type& val) const  override {
		// Check if val has a suitable value
#ifdef DEBUG
		switch(std::fpclassify(val)) {
			case FP_NORMAL:
			case FP_ZERO:      {
				/* nothing */
			} break;

			case FP_INFINITE:  {
				glogger
				<< "In GConstrainedFPT::transfer(): Error" << std::endl
				<< "val is infinite" << std::endl
				<< GEXCEPTION;
			} break;

			case FP_NAN:       {
				glogger
				<< "In GConstrainedFPT::transfer(): Error" << std::endl
				<< "val is NaN" << std::endl
				<< GEXCEPTION;
			} break;

			case FP_SUBNORMAL: {
				glogger
				<< "In GConstrainedFPT::transfer(): Error" << std::endl
				<< "val is subnormal" << std::endl
				<< GEXCEPTION;
			} break;

			default:           {
				glogger
				<< "In GConstrainedFPT::transfer(): Error" << std::endl
				<< "Unknown value type" << std::endl
				<< GEXCEPTION;
			}
		}
#endif /* DEBUG */

		long double localVal      = boost::numeric_cast<long double>(val);
		long double lowerBoundary = boost::numeric_cast<long double>(GConstrainedNumT<fp_type>::getLowerBoundary());
		long double upperBoundary = boost::numeric_cast<long double>(GConstrainedNumT<fp_type>::getUpperBoundary());

		if(localVal >= lowerBoundary && localVal < upperBoundary) {
			return val; // no cast needed
		} else {
			// Find out which region the value is in (compare figure transferFunction.pdf
			// that should have been delivered with this software). Note that boost::numeric_cast<>
			// may throw - exceptions must be caught in surrounding functions.
			std::int64_t region = 0;

#ifdef DEBUG
			long double fp_region = Gem::Common::gfloor((localVal - (long double)(lowerBoundary)) / ((long double)(upperBoundary) - (long double)(lowerBoundary)));

			if(Gem::Common::gfabs(fp_region) < boost::numeric_cast<long double>((std::numeric_limits<std::int64_t>::max)())) {
				// We need floor here, as an integer cast rounds towards 0, which would be wrong for negative values of val
				region = boost::numeric_cast<std::int64_t>(fp_region);
			} else {
				glogger
				<< "In GConstrainedFPT::transfer(): Error" << std::endl
				<< "fp_region = " << fp_region << " is too large and cannot be" << std::endl
				<< "converted to a std::int64_t, which has a maximum value of " << (std::numeric_limits<std::int64_t>::max)() << std::endl
				<< GEXCEPTION;
			}
#else   /* DEBUG */
			region =	static_cast<std::int64_t>(Gem::Common::gfloor((localVal - (long double)(lowerBoundary)) / ((long double)(upperBoundary) - (long double)(lowerBoundary))));
#endif  /* DEBUG */

			// Check whether we are in an odd or an even range and calculate the
			// external value accordingly
			long double mapping = (long double)(0.);
			if(region%2 == 0) { // can it be divided by 2 ? Region 0,2,... or a negative even range
				mapping = localVal - (long double)(region) * (upperBoundary - lowerBoundary);
			} else { // Range 1,3,... or a negative odd range
				mapping = -localVal + ((long double)(region-1)*(upperBoundary - lowerBoundary) + 2*upperBoundary);
			}

			// fabs(mapping) will always be <= fabs(val), so this cast should never fail (if val was a valid fp value)
			return boost::numeric_cast<fp_type>(mapping);
		}

		// Make the compiler happy
		return fp_type(0.);
	}

	/* ----------------------------------------------------------------------------------
	 * Tested in GConstrainedFPT<fp_type>::specificTestsNoFailuresExpected_GUnitTests()
	 * ----------------------------------------------------------------------------------
	 */

	/***************************************************************************/
	/**
	 * Emits a name for this class / object
	 */
	virtual std::string name() const override {
		return std::string("GConstrainedFPT");
	}

protected:
	/***************************************************************************/
	/**
	 * Loads the data of another GConstrainedFPT<fp_type>, camouflaged as a GObject.
	 *
	 * @param cp Another GConstrainedFPT<fp_type> object, camouflaged as a GObject
	 */
	virtual void load_(const GObject *cp) override {
		// Check that we are dealing with a GConstrainedFPT<fp_type>  reference independent of this object and convert the pointer
		const GConstrainedFPT<fp_type> *p_load = Gem::Common::g_convert_and_compare<GObject, GConstrainedFPT<fp_type>>(cp, this);

		// Load our parent class'es data ...
		GConstrainedNumT<fp_type>::load_(cp);

		// no local data ...
	}

	/***************************************************************************/
	/** @brief Create a deep copy of this object */
	virtual GObject *clone_() const override = 0;

	/***************************************************************************/
	/**
	 * Randomly initializes the parameter (within its limits)
	 */
	virtual bool randomInit_(
		const activityMode&
		, Gem::Hap::GRandomBase& gr
	) override {
		typename std::uniform_real_distribution<fp_type> uniform_real_distribution(
			GConstrainedNumT<fp_type>::getLowerBoundary()
			, GConstrainedNumT<fp_type>::getUpperBoundary()
		);

		this->setValue(uniform_real_distribution(gr));

		return true;
	}

	/* ----------------------------------------------------------------------------------
	 * Tested in GConstrainedFPT<fp_type>::specificTestsNoFailuresExpected_GUnitTests()
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
		if(GConstrainedNumT<fp_type>::modify_GUnitTests()) result = true;

		return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		condnotset("GConstrainedFPT<>::modify_GUnitTests", "GEM_TESTING");
		return false;
#endif /* GEM_TESTING */
	}

	/***************************************************************************/
	/**
	 * Performs self tests that are expected to succeed. This is needed for testing purposes
	 */
	virtual void specificTestsNoFailureExpected_GUnitTests() override {
#ifdef GEM_TESTING
		// Some general settings
		const std::size_t nTests = 10000;
		const fp_type testVal = fp_type(42);
		const fp_type testVal2 = fp_type(17);
		const fp_type lowerBoundary = fp_type(0);
		const fp_type upperBoundary = fp_type(100);
		const fp_type lowerRandomBoundary = fp_type(-100000);
		const fp_type upperRandomBoundary = fp_type( 100000);

		// Call the parent classes' functions
		GConstrainedNumT<fp_type>::specificTestsNoFailureExpected_GUnitTests();

		// A random generator
		Gem::Hap::GRandomT<Gem::Hap::RANDFLAVOURS::RANDOMPROXY> gr;

		//------------------------------------------------------------------------------

		{ // Check that assignment of a value with operator= works both for set and unset boundaries
			std::shared_ptr<GConstrainedFPT<fp_type>> p_test = this->GObject::clone<GConstrainedFPT<fp_type>>();

			// Reset the boundaries so we are free to do what we want
			BOOST_CHECK_NO_THROW(p_test->resetBoundaries());

			// Assign a value with operator=
			BOOST_CHECK_NO_THROW(*p_test = testVal2);

			// Check the value
			BOOST_CHECK(p_test->value() == testVal2);

			// Assign boundaries and values
			BOOST_CHECK_NO_THROW(p_test->setValue(testVal2, lowerBoundary, upperBoundary));

			// Check the value again
			BOOST_CHECK(p_test->value() == testVal2);

			// Assign a value with operator=
			BOOST_CHECK_NO_THROW(*p_test = testVal);

			// Check the value again, should have changed
			BOOST_CHECK(p_test->value() == testVal);
		}

		//------------------------------------------------------------------------------

		{ // Check that assignment of a value with setValue(val) works both for set and unset boundaries
			std::shared_ptr<GConstrainedFPT<fp_type>> p_test = this->GObject::clone<GConstrainedFPT<fp_type>>();

			// Reset the boundaries so we are free to do what we want
			BOOST_CHECK_NO_THROW(p_test->resetBoundaries());

			// Assign a value
			BOOST_CHECK_NO_THROW(p_test->setValue(testVal2));

			// Check the value
			BOOST_CHECK(p_test->value() == testVal2);

			// Assign new boundaries
			BOOST_CHECK_NO_THROW(p_test->setBoundaries(lowerBoundary, upperBoundary));

			// Cross-check that boundaries are o.k.
			BOOST_CHECK(p_test->getLowerBoundary() == lowerBoundary);
			BOOST_CHECK(p_test->getUpperBoundary() == boost::math::float_prior<fp_type>(upperBoundary));

			// Check the value again
			BOOST_CHECK(p_test->value() == testVal2);

			// Assign a new value
			BOOST_CHECK_NO_THROW(p_test->setValue(testVal));

			// Check the value again, should have changed
			BOOST_CHECK(p_test->value() == testVal);
		}

		//------------------------------------------------------------------------------

		{ // Check that simultaneous assignment of a valid value and boundaries works
			std::shared_ptr<GConstrainedFPT<fp_type>> p_test = this->GObject::clone<GConstrainedFPT<fp_type>>();

			// Reset the boundaries so we are free to do what we want
			BOOST_CHECK_NO_THROW(p_test->resetBoundaries());

			// Assign boundaries and values
			BOOST_CHECK_NO_THROW(p_test->setValue(testVal, lowerBoundary, upperBoundary));

			// Cross-check that value and boundaries are o.k.
         BOOST_CHECK_MESSAGE(
               p_test->getLowerBoundary() == lowerBoundary
               ,  "\n"
               << std::setprecision(16)
               << "Invalid lower boundary found:\n"
               << "getLowerBoundary() = " << p_test->getLowerBoundary()
               << "expected " << lowerBoundary << "\n"
         );

         BOOST_CHECK_MESSAGE(
               p_test->getUpperBoundary() == boost::math::float_prior<fp_type>(upperBoundary)
               ,  "\n"
               << std::setprecision(16)
               << "Invalid upper boundary found:\n"
               << "getUpperBoundary() = " << p_test->getUpperBoundary() << "\n"
               << "expected " << boost::math::float_prior<fp_type>(upperBoundary) << "\n"
               << "Difference is " << p_test->getUpperBoundary() - boost::math::float_prior<fp_type>(upperBoundary) << "\n"
         );


			BOOST_CHECK(p_test->value() == testVal);
		}

		//------------------------------------------------------------------------------

		{ // Check a number of times that calls to the transfer function do not lie outside of the allowed boundaries
			std::shared_ptr<GConstrainedFPT<fp_type>> p_test = this->GObject::clone<GConstrainedFPT<fp_type>>();

			fp_type result = 0.;
			for(fp_type offset = fp_type(-100); offset < fp_type(100); offset += fp_type(10)) {
				fp_type tmpLowerBoundary = lowerBoundary + offset;
				fp_type tmpUpperBoundary = upperBoundary + offset;

				// Assign valid boundaries and value
				BOOST_CHECK_NO_THROW(p_test->setValue(tmpLowerBoundary, tmpLowerBoundary, tmpUpperBoundary));

				typename std::uniform_real_distribution<fp_type> uniform_real_distribution(lowerRandomBoundary, upperRandomBoundary);
				for(std::size_t i=0; i<nTests; i++) {
					fp_type randomValue = uniform_real_distribution(gr);

					BOOST_CHECK_NO_THROW(result = p_test->transfer(randomValue));
					BOOST_CHECK_MESSAGE(
							result >= tmpLowerBoundary && result < tmpUpperBoundary
							,  "\n"
							<< std::setprecision(6)
							<< "randomValue = " << randomValue << "\n"
							<< "after transfer = " << result << "\n"
							<< "lowerBoundary = " << tmpLowerBoundary << "\n"
							<< "upperBoundary = " << tmpUpperBoundary << "\n"
					);
				}
			}
		}

		//------------------------------------------------------------------------------

		{ // Test initialization with a single "fixed" value (chosen randomly in a given range)
			std::shared_ptr<GConstrainedFPT<fp_type>> p_test = this->GObject::clone<GConstrainedFPT<fp_type>>();

			// Assign a valid value and boundaries
			BOOST_CHECK_NO_THROW(p_test->setValue(testVal, lowerBoundary, upperBoundary));

			typename std::uniform_real_distribution<fp_type> uniform_real_distribution(lowerRandomBoundary, upperRandomBoundary);
			for(std::size_t i=0; i<nTests; i++) {
				fp_type randomValue = uniform_real_distribution(gr);

				// Randomly initialize with a "fixed" value
				BOOST_CHECK_NO_THROW(p_test->GParameterBase::template fixedValueInit<fp_type>(randomValue, activityMode::ALLPARAMETERS));

				// Check that the external value is inside of the allowed value range
				// Check that the value is still in the allowed range
				BOOST_CHECK_MESSAGE(
						p_test->value() >= lowerBoundary && p_test->value() < upperBoundary
						,  "\n"
						<< std::setprecision(10)
						<< "p_test->value() = " << p_test->value() << "\n"
						<< "lowerBoundary = " << lowerBoundary << "\n"
						<< "upperBoundary = " << upperBoundary << "\n"
				);
			}
		}

		//------------------------------------------------------------------------------

		{ // Test multiplication with a single floating point value that won't make the internal value leave the boundaries
			std::shared_ptr<GConstrainedFPT<fp_type>> p_test = this->GObject::clone<GConstrainedFPT<fp_type>>();

			// Reset the boundaries so we are free to do what we want
			BOOST_CHECK_NO_THROW(p_test->resetBoundaries());

			// Assign a value
			BOOST_CHECK_NO_THROW(p_test->setValue(fp_type(1), fp_type(0), fp_type(100)));

			for(std::size_t i=1; i<99; i++) {
				// Multiply by the counter variable
				BOOST_CHECK_NO_THROW(p_test->GParameterBase::template multiplyBy<fp_type>(fp_type(i), activityMode::ALLPARAMETERS));

				// Check that the external value is in the expected range
				BOOST_CHECK_MESSAGE (
						fabs(p_test->value() - fp_type(i)) < pow(10, -8)  // This also means that the value has changed from its start value 1
						,  "\n"
						<< std::setprecision(10)
						<< "p_test->value() = " << p_test->value() << "\n"
						<< "fp_type(i) = " << fp_type(i) << "\n"
						<< "pow(10, -8) = " << pow(10, -8) << "\n"
				);

				// Check that the internal value is in the expected range
				BOOST_CHECK_MESSAGE (
						fabs(p_test->getInternalValue() - fp_type(i)) < pow(10, -8)
						,  "\n"
						<< std::setprecision(10)
						<< "p_test->getInternalValue() = " << p_test->getInternalValue() << "\n"
						<< "fp_type(i) = " << fp_type(i) << "\n"
						<< "pow(10, -8) = " << pow(10, -8) << "\n"
				);

				// Reset the value
				BOOST_CHECK_NO_THROW(p_test->setValue(fp_type(1)));
			}
		}

		//------------------------------------------------------------------------------


		{ // Test multiplication with a single floating point value that will make the internal value leave its boundaries
			std::shared_ptr<GConstrainedFPT<fp_type>> p_test = this->GObject::clone<GConstrainedFPT<fp_type>>();

			// Assign boundaries and values
			BOOST_CHECK_NO_THROW(p_test->setValue(fp_type(1), lowerBoundary, upperBoundary));

			typename std::uniform_real_distribution<fp_type> uniform_real_distribution(lowerRandomBoundary, upperRandomBoundary);
			for(std::size_t i=0; i<nTests; i++) {
				// Multiply with a random value in a very wide
				BOOST_CHECK_NO_THROW(p_test->GParameterBase::template multiplyBy<fp_type>(uniform_real_distribution(gr), activityMode::ALLPARAMETERS));

				// Check that the value is still in the allowed range
				BOOST_CHECK_MESSAGE(
						p_test->value() >= lowerBoundary && p_test->value() < upperBoundary
						,  "\n"
						<< std::setprecision(10)
						<< "p_test->value() = " << p_test->value() << "\n"
						<< "lowerBoundary = " << lowerBoundary << "\n"
						<< "upperBoundary = " << upperBoundary << "\n"
				);

				// Reset the value
				BOOST_CHECK_NO_THROW(p_test->setValue(fp_type(1)));
			}
		}

		//------------------------------------------------------------------------------

		{ // Check multiplication with a random number in a wide range that might make the internal value leave its boundaries
			std::shared_ptr<GConstrainedFPT<fp_type>> p_test = this->GObject::clone<GConstrainedFPT<fp_type>>();

			// Assign boundaries and values
			BOOST_CHECK_NO_THROW(p_test->setValue(fp_type(1), lowerBoundary, upperBoundary));

			for(std::size_t i=0; i<nTests; i++) {
				// Multiply with a random value in a very wide
				BOOST_CHECK_NO_THROW(p_test->GParameterBase::template multiplyByRandom<fp_type>(lowerRandomBoundary, upperRandomBoundary, activityMode::ALLPARAMETERS, gr));

				// Check that the value is still in the allowed range
				BOOST_CHECK_MESSAGE(
						p_test->value() >= lowerBoundary && p_test->value() < upperBoundary
						,  "\n"
						<< std::setprecision(10)
						<< "p_test->value() = " << p_test->value() << "\n"
						<< "lowerBoundary = " << lowerBoundary << "\n"
						<< "upperBoundary = " << upperBoundary << "\n"
				);

				// Reset the value
				BOOST_CHECK_NO_THROW(p_test->setValue(fp_type(1)));
			}
		}

		//------------------------------------------------------------------------------

		{ // Check multiplication with a random number in the range [0:1[. As the value used for the
		  // basis of this multiplication is the lower boundary, multiplication will bring the internal
		  // value outside of the external boundaries
			std::shared_ptr<GConstrainedFPT<fp_type>> p_test = this->GObject::clone<GConstrainedFPT<fp_type>>();

			// Assign boundaries and values
			BOOST_CHECK_NO_THROW(p_test->setValue(lowerBoundary, lowerBoundary, upperBoundary));

			for(std::size_t i=0; i<nTests; i++) {
				// Multiply with a random value in a very wide
				BOOST_CHECK_NO_THROW(p_test->GParameterBase::template multiplyByRandom<fp_type>(activityMode::ALLPARAMETERS, gr));

				// Check that the value is still in the allowed range
				BOOST_CHECK_MESSAGE(
						p_test->value() >= lowerBoundary && p_test->value() < upperBoundary
						,  "\n"
						<< std::setprecision(10)
						<< "p_test->value() = " << p_test->value() << "\n"
						<< "lowerBoundary = " << lowerBoundary << "\n"
						<< "upperBoundary = " << upperBoundary << "\n"
				);

				// Reset the value
				BOOST_CHECK_NO_THROW(p_test->setValue(lowerBoundary));
			}
		}

		//------------------------------------------------------------------------------

		{ // Test adding of objects with fpAdd. We try to stay inside of the value range
			const fp_type lower = fp_type(-10000.), upper = fp_type(10000.);

			std::shared_ptr<GConstrainedFPT<fp_type>> p_test1 = this->GObject::clone<GConstrainedFPT<fp_type>>();
			std::shared_ptr<GConstrainedFPT<fp_type>> p_test2 = this->GObject::clone<GConstrainedFPT<fp_type>>();

			// Reset the boundaries so we are free to do what we want
			BOOST_CHECK_NO_THROW(p_test1->resetBoundaries());

			// Assign a value and boundaries
			BOOST_CHECK_NO_THROW(p_test1->setValue(lower, lower, upper));

			// Load p_test1 into p_test2
			BOOST_CHECK_NO_THROW(p_test2->load(p_test1));

			// Assign a value of 1 to p_test2
			BOOST_CHECK_NO_THROW(p_test2->GParameterBase::template fixedValueInit<fp_type>(fp_type(1.), activityMode::ALLPARAMETERS));

			fp_type currentVal = fp_type(-10000.);
			for(std::int32_t i=-9999; i<9999; i++) {
				BOOST_CHECK_NO_THROW(p_test1->GParameterBase::template add<fp_type>(p_test2, activityMode::ALLPARAMETERS));
				currentVal += fp_type(1.);
				BOOST_CHECK(p_test1->value() == currentVal);
			}
		}

		//------------------------------------------------------------------------------

		{ // Test subtraction of objects with fpSubtract. We try to stay inside of the value range
			const fp_type lower = fp_type(-10000.), upper = fp_type(10000.);

			std::shared_ptr<GConstrainedFPT<fp_type>> p_test1 = this->GObject::clone<GConstrainedFPT<fp_type>>();
			std::shared_ptr<GConstrainedFPT<fp_type>> p_test2 = this->GObject::clone<GConstrainedFPT<fp_type>>();

			// Reset the boundaries so we are free to do what we want
			BOOST_CHECK_NO_THROW(p_test1->resetBoundaries());

			// Assign a value and boundaries
			BOOST_CHECK_NO_THROW(p_test1->setValue(upper - fp_type(1.), lower, upper));

			// Load p_test1 into p_test2
			BOOST_CHECK_NO_THROW(p_test2->load(p_test1));

			// Assign a value of 1 to p_test2
			BOOST_CHECK_NO_THROW(p_test2->GParameterBase::template fixedValueInit<fp_type>(fp_type(1.), activityMode::ALLPARAMETERS));

			fp_type currentVal = fp_type(upper - fp_type(1));
			for(std::int32_t i=9999; i>=-9998; i--) {
				BOOST_CHECK_NO_THROW(p_test1->GParameterBase::template subtract<fp_type>(p_test2, activityMode::ALLPARAMETERS));
				currentVal -= fp_type(1.);
				BOOST_CHECK(p_test1->value() == currentVal);
			}
		}

		//------------------------------------------------------------------------------

		{ // Test random initialization, as well as adding and subtraction of random values, which may leave the value range
			const fp_type lower = fp_type(-10000.), upper = fp_type(10000.);

			std::shared_ptr<GConstrainedFPT<fp_type>> p_test1 = this->GObject::clone<GConstrainedFPT<fp_type>>();
			std::shared_ptr<GConstrainedFPT<fp_type>> p_test2 = this->GObject::clone<GConstrainedFPT<fp_type>>();

			// Assign a value and boundaries
			BOOST_CHECK_NO_THROW(p_test1->setValue(fp_type(0.), lower, upper));
			BOOST_CHECK_NO_THROW(p_test2->setValue(fp_type(0.), lower, upper));

			// Repeatedly add and subtract a randomly initialized p_test2 from p_test1
			for(std::size_t i=0; i<nTests; i++) {
				// Randomly initialize p_test2
				BOOST_CHECK_NO_THROW(p_test2->randomInit_(activityMode::ALLPARAMETERS, gr));

				fp_type firstValue = p_test2->value();

				// Inside of the allowed value range ?
				BOOST_CHECK(firstValue >= lower);
				BOOST_CHECK(firstValue  < upper);

				// Add to p_test1
				BOOST_CHECK_NO_THROW(p_test1->GParameterBase::template add<fp_type>(p_test2, activityMode::ALLPARAMETERS));

				// Check that p_test1 is still inside of the allowed value range
				BOOST_CHECK(p_test1->value() >= lower);
				BOOST_CHECK(p_test1->value()  < upper);

				// Randomly initialize p_test2 again
				BOOST_CHECK_NO_THROW(p_test2->randomInit_(activityMode::ALLPARAMETERS, gr));

				fp_type secondValue = p_test2->value();

				// Inside of the allowed value range ?
				BOOST_CHECK(secondValue >= lower);
				BOOST_CHECK(secondValue  < upper);

				// Has the value changed at all ?
				BOOST_CHECK(firstValue != secondValue);

				// Subtract from p_test1
				BOOST_CHECK_NO_THROW(p_test1->GParameterBase::template subtract<fp_type>(p_test2, activityMode::ALLPARAMETERS));

				// Check that p_test1 is still inside of the allowed value range
				BOOST_CHECK(p_test1->value() >= lower);
				BOOST_CHECK(p_test1->value()  < upper);
			}
		}

		//------------------------------------------------------------------------------

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
		condnotset("GConstrainedFPT<>::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
	}

	/***************************************************************************/
	/**
	 * Performs self tests that are expected to fail. This is needed for testing purposes
	 */
	virtual void specificTestsFailuresExpected_GUnitTests() override {
#ifdef GEM_TESTING
		// Some general settings
		const fp_type testVal = fp_type(42);
		const fp_type lowerBoundary = fp_type(0);
		const fp_type upperBoundary = fp_type(100);

		// Call the parent classes' functions
		GConstrainedNumT<fp_type>::specificTestsFailuresExpected_GUnitTests();

		//------------------------------------------------------------------------------

		{ // Check that assignment of a value equal to the upper boundary with setValue(val, lower, upper) throws
			std::shared_ptr<GConstrainedFPT<fp_type>> p_test = this->GObject::clone<GConstrainedFPT<fp_type>>();

			// Reset the boundaries so we are free to do what we want
			BOOST_CHECK_NO_THROW(p_test->resetBoundaries());

			// Set value, upper and lower boundaries; should throw, as value >= upperBoundary
			BOOST_CHECK_THROW(p_test->setValue(1.1*upperBoundary, lowerBoundary, upperBoundary), Gem::Common::gemfony_error_condition);
		}

		//------------------------------------------------------------------------------

		{ // Check that assignment of a value equal to the upper boundary with setValue() throws
			std::shared_ptr<GConstrainedFPT<fp_type>> p_test = this->GObject::clone<GConstrainedFPT<fp_type>>();

			// Reset the boundaries so we are free to do what we want
			BOOST_CHECK_NO_THROW(p_test->resetBoundaries());

			// Set value, upper and lower boundaries
			BOOST_CHECK_NO_THROW(p_test->setValue(testVal, lowerBoundary, upperBoundary));

			// Try to set a value equal to the upper boundary, should throw
			BOOST_CHECK_THROW(p_test->setValue(1.1*upperBoundary), Gem::Common::gemfony_error_condition);
		}

		//------------------------------------------------------------------------------

		{ // Check that setting an upper boundary <= lower boundary with setBoundaries(lower, upper) throws
			std::shared_ptr<GConstrainedFPT<fp_type>> p_test = this->GObject::clone<GConstrainedFPT<fp_type>>();

			// Reset the boundaries so we are free to do what we want
			BOOST_CHECK_NO_THROW(p_test->resetBoundaries());

			// Try to set an upper boundary == lower boundary
			BOOST_CHECK_THROW(p_test->setBoundaries(lowerBoundary, lowerBoundary), Gem::Common::gemfony_error_condition);
		}

		//------------------------------------------------------------------------------

		{ // Check that setting an upper boundary <= lower boundary with setValue(val, lower, upper) throws
			std::shared_ptr<GConstrainedFPT<fp_type>> p_test = this->GObject::clone<GConstrainedFPT<fp_type>>();

			// Reset the boundaries so we are free to do what we want
			BOOST_CHECK_NO_THROW(p_test->resetBoundaries());

			// Try to set an upper boundary == lower boundary
			BOOST_CHECK_THROW(p_test->setValue(lowerBoundary, lowerBoundary, lowerBoundary), Gem::Common::gemfony_error_condition);
		}

		//------------------------------------------------------------------------------

		{ // Check that setting an upper boundary larger than the allowed value (see GConstrainedValueLimit<T>) with the setValue(val, lower, upper) function throws
			std::shared_ptr<GConstrainedFPT<fp_type>> p_test = this->GObject::clone<GConstrainedFPT<fp_type>>();

			// Reset the boundaries so we are free to do what we want
			BOOST_CHECK_NO_THROW(p_test->resetBoundaries());

			// Check that the boundaries have the expected values
			BOOST_CHECK(p_test->getLowerBoundary() == GConstrainedValueLimitT<fp_type>::lowest());
			BOOST_CHECK(p_test->getUpperBoundary() ==  boost::math::float_prior<fp_type>(GConstrainedValueLimitT<fp_type>::highest()));

			// Try to set a boundary to a bad value
			BOOST_CHECK_THROW(p_test->setValue(lowerBoundary, lowerBoundary, boost::numeric::bounds<fp_type>::highest()), Gem::Common::gemfony_error_condition);
		}

		//------------------------------------------------------------------------------

		{ // Check that setting a lower boundary smaller than the allowed value (see GConstrainedValueLimit<T>)  with the setValue(val, lower, upper) function throws
			std::shared_ptr<GConstrainedFPT<fp_type>> p_test = this->GObject::clone<GConstrainedFPT<fp_type>>();

			// Reset the boundaries so we are free to do what we want
			BOOST_CHECK_NO_THROW(p_test->resetBoundaries());

			// Check that the boundaries have the expected values
			BOOST_CHECK(p_test->getLowerBoundary() == GConstrainedValueLimitT<fp_type>::lowest());
			BOOST_CHECK(p_test->getUpperBoundary() ==  boost::math::float_prior<fp_type>(GConstrainedValueLimitT<fp_type>::highest()));

			// Try to set a boundary to a bad value
			BOOST_CHECK_THROW(p_test->setValue(0., boost::numeric::bounds<fp_type>::lowest(), upperBoundary), Gem::Common::gemfony_error_condition);
		}

		//------------------------------------------------------------------------------

		{ // Check that setting an upper boundary larger than the allowed value (see GConstrainedValueLimit<T>) with the setBoundaries(lower, upper) function throws
			std::shared_ptr<GConstrainedFPT<fp_type>> p_test = this->GObject::clone<GConstrainedFPT<fp_type>>();

			// Reset the boundaries so we are free to do what we want
			BOOST_CHECK_NO_THROW(p_test->resetBoundaries());

			// Check that the boundaries have the expected values
			BOOST_CHECK(p_test->getLowerBoundary() == GConstrainedValueLimitT<fp_type>::lowest());
			BOOST_CHECK(p_test->getUpperBoundary() ==  boost::math::float_prior<fp_type>(GConstrainedValueLimitT<fp_type>::highest()));

			// Try to set a boundary to a bad value
			BOOST_CHECK_THROW(p_test->setBoundaries(lowerBoundary, boost::numeric::bounds<fp_type>::highest()), Gem::Common::gemfony_error_condition);
		}

		//------------------------------------------------------------------------------

		{ // Check that setting a lower boundary smaller than the allowed value (see GConstrainedValueLimit<T>) with the setBoundaries(lower, upper) function throws
			std::shared_ptr<GConstrainedFPT<fp_type>> p_test = this->GObject::clone<GConstrainedFPT<fp_type>>();

			// Reset the boundaries so we are free to do what we want
			BOOST_CHECK_NO_THROW(p_test->resetBoundaries());

			// Check that the boundaries have the expected values
			BOOST_CHECK(p_test->getLowerBoundary() == GConstrainedValueLimitT<fp_type>::lowest());
			BOOST_CHECK(p_test->getUpperBoundary() ==  boost::math::float_prior<fp_type>(GConstrainedValueLimitT<fp_type>::highest()));

			// Try to set a boundary to a bad value
			BOOST_CHECK_THROW(p_test->setBoundaries(boost::numeric::bounds<fp_type>::lowest(), upperBoundary), Gem::Common::gemfony_error_condition);
		}

		//------------------------------------------------------------------------------

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
		condnotset("GConstrainedFPT<>::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
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
struct is_abstract<Gem::Geneva::GConstrainedFPT<fp_type>> : public boost::true_type {};
template<typename fp_type>
struct is_abstract< const Gem::Geneva::GConstrainedFPT<fp_type>> : public boost::true_type {};
}
}

/******************************************************************************/

#endif /* GCONSTRAINEDFPT_HPP_ */
