/********************************************************************************
 *
 * This file is part of the Geneva library collection. The following license
 * applies to this file:
 *
 * ------------------------------------------------------------------------------
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * ------------------------------------------------------------------------------
 *
 * Note that other files in the Geneva library collection may use a different
 * license. Please see the licensing information in each file.
 *
 ********************************************************************************
 *
 * Geneva was started by Dr. Rüdiger Berlich and was later maintained together
 * with Dr. Ariel Garcia under the auspices of Gemfony scientific. For further
 * information on Gemfony scientific, see http://www.gemfomy.eu .
 *
 * The majority of files in Geneva was released under the Apache license v2.0
 * in February 2020.
 *
 * See the NOTICE file in the top-level directory of the Geneva library
 * collection for a list of contributors and copyright information.
 *
 ********************************************************************************/

#include "geneva/GBooleanObject.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GBooleanObject)

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * Initialization by contained value
 *
 * @param val A value used for the initialization
 */
GBooleanObject::GBooleanObject(const bool &val)
	: GParameterT<bool>(val)
{ /* nothing */ }

// Tested in this file

/******************************************************************************/
/**
 * Initialization with a given probability for "true". E.g., a probability value
 * of 0.7 results in approimately 70% "true" values.
 *
 * @param prob The probability for the value "true"
 */
GBooleanObject::GBooleanObject(const double &probability) {
	Gem::Hap::GRandomT<Gem::Hap::RANDFLAVOURS::RANDOMLOCAL> gr;
	std::bernoulli_distribution bernoulli_distribution(probability);

	this->setValue(bernoulli_distribution(gr));
}

// Tested in this file

/******************************************************************************/
/**
 * An assignment operator
 *
 * @param val The value to be assigned to this object
 * @return The value that was just assigned to this object
 */
GBooleanObject& GBooleanObject::operator=(const bool &val) {
	GParameterT<bool>::operator=(val);
	return *this;
}

/******************************************************************************/
/**
 * Creates a deep clone of this object.
 *
 * @return A copy of this object, camouflaged as a GObject
 */
GObject *GBooleanObject::clone_() const {
	return new GBooleanObject(*this);
}

/******************************************************************************/
/**
 * Flips the value of this object
 */
void GBooleanObject::flip() {
	this->setValue(not this->value());
}

/******************************************************************************/
/**
 * Random initialization. This is a helper function, without it we'd
 * have to say things like "myGBooleanObject.GParameterBase::randomInit();".
 */
bool GBooleanObject::randomInit(
	const activityMode &am
	, Gem::Hap::GRandomBase& gr
) {
	return GParameterBase::randomInit(am, gr);
}

/* ----------------------------------------------------------------------------------
 * Tested in GInt32Collection::specificTestsNoFailuresExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Triggers random initialization of the parameter object, with a given likelihood structure
 */
bool GBooleanObject::randomInit(
	const double &probability
	, const activityMode &am
	, Gem::Hap::GRandomBase& gr
) {
	if (not GParameterBase::randomInitializationBlocked() && this->modifiableAmMatchOrHandover(am)) {
		return randomInit_(probability, am, gr);
	} else {
		return false;
	}
}

/* ----------------------------------------------------------------------------------
 * Tested in GInt32Collection::specificTestsNoFailuresExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Triggers random initialization of the parameter object
 */
bool GBooleanObject::randomInit_(
	const activityMode & am
	, Gem::Hap::GRandomBase& gr
) {
	std::bernoulli_distribution bernoulli_distribution; // defaults to 0.5
	this->setValue(bernoulli_distribution(gr));
	return true;
}

/* ----------------------------------------------------------------------------------
 * Tested in GInt32Collection::specificTestsNoFailuresExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Triggers random initialization of the parameter object, with a given likelihood structure.
 * This function holds the actual initialization logic, used in the publicly accessible
 * GBooleanObject::randomInit(const double& probability) function. A probability value of 0.7
 * results in approimately 70% "true" values.
 */
bool GBooleanObject::randomInit_(
	const double &probability
	, const activityMode &am
	, Gem::Hap::GRandomBase& gr
) {
	// Do some error checks
	if(not Gem::Common::checkRangeCompliance(probability, 0., 1., "GBooleanObject::randomInit_(probability)")) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GBooleanObject::randomInit_(probability): Error!" << std::endl
				<< "Probability " << probability << " not in allowed value range [0,1]" << std::endl
		);
	}

	std::bernoulli_distribution bernoulli_distribution(probability);
	this->setValue(bernoulli_distribution(gr));
	return true;
}

/* ----------------------------------------------------------------------------------
 * Tested in GInt32Collection::specificTestsNoFailuresExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/***************************************************************************/
/**
 * Returns a "comparative range". In the case of boolean values this must
 * be considered to be more of a "dummy".
 */
bool GBooleanObject::range() const {
	return true;
}

/******************************************************************************/
/**
 * Searches for compliance with expectations with respect to another object
 * of the same type
 *
 * @param cp A constant reference to another GObject object
 * @param e The expected outcome of the comparison
 * @param limit The maximum deviation for floating point values (important for similarity checks)
 */
void GBooleanObject::compare_(
	const GObject &cp, const Gem::Common::expectation &e, const double &limit
) const {
	using namespace Gem::Common;

	// Check that we are dealing with a GBooleanObject reference independent of this object and convert the pointer
	const GBooleanObject *p_load = Gem::Common::g_convert_and_compare<GObject, GBooleanObject >(cp, this);

	GToken token("GBooleanObject", e);

	// Compare our parent data ...
	Gem::Common::compare_base_t<GParameterT<bool>>(*this, *p_load, token);

	// ... no local data

	// React on deviations from the expectation
	token.evaluate();
}

/***********************************************************************************/
/**
 * Emits a name for this class / object
 */
std::string GBooleanObject::name_() const {
	return std::string("GBooleanObject");
}

/******************************************************************************/
/**
 * Attach our local value to the vector. This is used to collect all parameters of this type
 * in the sequence in which they were registered.
 *
 * @param parVec The vector to which the local value should be attached
 */
void GBooleanObject::booleanStreamline(
	std::vector<bool> &parVec, const activityMode &am
) const {
	parVec.push_back(this->value());
}

/******************************************************************************/
/**
 * Attach our local value to the map.
 *
 * @param parVec The map to which the local value should be attached
 */
void GBooleanObject::booleanStreamline(
	std::map<std::string, std::vector<bool>> &parVec, const activityMode &am
) const {
#ifdef DEBUG
	if((this->getParameterName()).empty()) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GBooleanObject::booleanStreamline(std::map<std::string, std::vector<bool>>& parVec) const: Error!" << std::endl
				<< "No name was assigned to the object" << std::endl
		);
	}
#endif /* DEBUG */

	std::vector<bool> parameters;
	parameters.push_back(this->value());
	parVec[this->getParameterName()] = parameters;
}

/******************************************************************************/
/**
 * Attach boundaries of type bool to the vectors. This function has been added for
 * completeness reasons only.
 *
 * @param lBndVec A vector of lower bool parameter boundaries
 * @param uBndVec A vector of upper bool parameter boundaries
 */
void GBooleanObject::booleanBoundaries(
	std::vector<bool> &lBndVec, std::vector<bool> &uBndVec, const activityMode &am
) const {
	lBndVec.push_back(false);
	uBndVec.push_back(true);
}


/******************************************************************************/
/**
 * Tell the audience that we own a bool value
 *
 * @param @param am An enum indicating whether only information about active, inactive or all parameters of this type should be extracted
 * @return The number of active, incactive or all float parameters
 */
std::size_t GBooleanObject::countBoolParameters(
	const activityMode &am
) const {
	return 1;
}

/******************************************************************************/
/**
 * Assigns part of a value vector to the parameter
 */
void GBooleanObject::assignBooleanValueVector(
	const std::vector<bool> &parVec, std::size_t &pos, const activityMode &am
) {
#ifdef DEBUG
	// Do we have a valid position ?
	if(pos >= parVec.size()) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GBooleanObject::assignBooleanValueVector(const std::vector<bool>&, std::size_t&):" << std::endl
				<< "Tried to access position beyond end of vector: " << parVec.size() << "/" << pos << std::endl
		);
	}
#endif

	this->setValue(parVec[pos]);
	pos++;
}

/******************************************************************************/
/**
 * Assigns part of a value map to the parameter
 */
void GBooleanObject::assignBooleanValueVectors(
	const std::map<std::string, std::vector<bool>> &parMap, const activityMode &am
) {
	this->setValue((Gem::Common::getMapItem<std::vector<bool>>(parMap, this->getParameterName())).at(0));
}

/******************************************************************************/
/**
 * Loads the data of another GObject
 *
 * @param cp A copy of another GBooleanObject object, camouflaged as a GObject
 */
void GBooleanObject::load_(const GObject *cp) {
	// Convert the pointer to our target type and check for self-assignment
	const auto * p_load = Gem::Common::g_convert_and_compare<GObject, GBooleanObject >(cp, this);

	// Load our parent class'es data ...
	GParameterT<bool>::load_(cp);

	// ... no local data
}

/******************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GBooleanObject::modify_GUnitTests_() {
#ifdef GEM_TESTING
	bool result = false;

	// Call the parent class'es function
	if (GParameterT<bool>::modify_GUnitTests_()) result = true;

	this->flip();
	result = true;

	return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
	Gem::Common::condnotset("GBooleanObject::modify_GUnitTests", "GEM_TESTING");
   return false;
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GBooleanObject::specificTestsNoFailureExpected_GUnitTests_() {
#ifdef GEM_TESTING
	// Some general settings
	const double LOWERBND = 0.8, UPPERBND = 1.2;
	const std::size_t nTests = 10000;

	// Make sure we have an appropriate adaptor loaded when performing these tests
	bool adaptorStored = false;
	std::shared_ptr <GAdaptorT<bool>> storedAdaptor;

	if (this->hasAdaptor()) {
		storedAdaptor = this->getAdaptor();
		adaptorStored = true;
	}

	std::shared_ptr <GBooleanAdaptor> gba_ptr(new GBooleanAdaptor(1.0));
	gba_ptr->setAdaptionThreshold(0); // Make sure the adaptor's internal parameters don't change through the adaption
	gba_ptr->setAdaptionMode(adaptionMode::ALWAYS); // Always adapt
	this->addAdaptor(gba_ptr);

	// Call the parent class'es function
	GParameterT<bool>::specificTestsNoFailureExpected_GUnitTests_();

	// A random generator
	Gem::Hap::GRandomT<Gem::Hap::RANDFLAVOURS::RANDOMPROXY> gr;

	// --------------------------------------------------------------------------

	{ // Test default constructor
		GBooleanObject gbo;
		BOOST_CHECK_MESSAGE (
			gbo.value() == Gem::Common::GDefaultValueT<bool>::value(), "\n"
			<< "gbo.value() = " << gbo.value()
			<< "DEFBOVAL = " <<
			Gem::Common::GDefaultValueT<bool>::value()
		);
	}

	// --------------------------------------------------------------------------

	{ // Test copy construction and construction with value
		GBooleanObject gbo1(false), gbo2(gbo1);

		BOOST_CHECK_MESSAGE (
			not gbo1.value() && gbo2.value() == gbo1.value(), "\n"
			<< "gbo1.value() = " << gbo1.value()
			<< "gbo2.value() = " << gbo2.value()
		);
	}

	// --------------------------------------------------------------------------

	{ // Check construction with a given probability for the value "true"
		std::size_t nTrue = 0, nFalse = 0;
		for (std::size_t i = 0; i < nTests; i++) {
			GBooleanObject gbo(0.5);
			gbo.value() ? nTrue++ : nFalse++;
		}

		// We allow a slight deviation, as the initialization is a random process
		BOOST_REQUIRE(nFalse != 0); // There should be a few false values
		double ratio = double(nTrue) / double(nFalse);
		BOOST_CHECK_MESSAGE(
			ratio > LOWERBND && ratio < UPPERBND, "\n"
			<< "ratio = " << ratio << "\n"
			<< "nTrue = " << nTrue << "\n"
			<< "nFalse = " << nFalse << "\n"
		);
	}

	// --------------------------------------------------------------------------

	{ // Test that random initialization with equal probability for true and false will result in roughly the same amount of corresponding values
		std::shared_ptr <GBooleanObject> p_test = this->clone<GBooleanObject>();

		// Assign a boolean value true
		BOOST_CHECK_NO_THROW(*p_test = true);
		// Cross-check
		BOOST_CHECK(p_test->value());

		// Count the number of true and false values for a number of subsequent initializations
		// with the internal randomInit_ function.
		std::size_t nTrue = 0;
		std::size_t nFalse = 0;
		for (std::size_t i = 0; i < nTests; i++) {
			p_test->randomInit_(activityMode::ALLPARAMETERS, gr);
			p_test->value() ? nTrue++ : nFalse++;
		}

		// We allow a slight deviation, as the initialization is a random process
		BOOST_REQUIRE(nFalse != 0); // There should be a few false values
		double ratio = double(nTrue) / double(nFalse);
		BOOST_CHECK_MESSAGE(
			ratio > 0.8 && ratio < 1.2, "\n"
			<< "ratio = " << ratio << "\n"
			<< "nTrue = " << nTrue << "\n"
			<< "nFalse = " << nFalse << "\n"
		);
	}

	// --------------------------------------------------------------------------

	{ // Test that initialization with a probability of 1 for true will only result in true values
		std::shared_ptr <GBooleanObject> p_test = this->clone<GBooleanObject>();

		// Assign a boolean value true
		BOOST_CHECK_NO_THROW(*p_test = false);
		// Cross-check
		BOOST_CHECK(not p_test->value());

		// Count the number of true and false values for a number of subsequent initializations
		// with the internal randomInit_ function.
		std::size_t nTrue = 0;
		std::size_t nFalse = 0;
		for (std::size_t i = 0; i < nTests; i++) {
			p_test->randomInit_(1., activityMode::ALLPARAMETERS, gr);
			p_test->value() ? nTrue++ : nFalse++;
		}

		// We should have received only true values
		BOOST_CHECK(nTrue == nTests);
	}

	// --------------------------------------------------------------------------

	{ // Test that initialization with a probability of 0 for true will only result in false values
		std::shared_ptr <GBooleanObject> p_test = this->clone<GBooleanObject>();

		// Assign a boolean value true
		BOOST_CHECK_NO_THROW(*p_test = true);
		// Cross-check
		BOOST_CHECK(p_test->value());

		// Count the number of true and false values for a number of subsequent initializations
		// with the internal randomInit_ function.
		std::size_t nTrue = 0;
		std::size_t nFalse = 0;
		for (std::size_t i = 0; i < nTests; i++) {
			p_test->randomInit_(0., activityMode::ALLPARAMETERS, gr);
			p_test->value() ? nTrue++ : nFalse++;
		}

		// We should have received only true values
		BOOST_CHECK(nFalse == nTests);
	}

	//-----------------------------------------------------------------------------

	{ // Test that random initialization with a given probability for true will result in roughly the expected amount of corresponding values
		for (double d = 0.1; d < 0.9; d += 0.1) {
			std::shared_ptr <GBooleanObject> p_test = this->clone<GBooleanObject>();

			// Assign a boolean value true
			BOOST_CHECK_NO_THROW(*p_test = true);
			// Cross-check
			BOOST_CHECK(p_test->value());

			// Randomly initialize, using the internal function, with the current probability
			BOOST_CHECK_NO_THROW(p_test->randomInit_(d, activityMode::ALLPARAMETERS, gr));

			// Count the number of true and false values for a number of subsequent initializations
			// with the internal randomInit_ function.
			std::size_t nTrue = 0;
			std::size_t nFalse = 0;
			for (std::size_t i = 0; i < nTests; i++) {
				p_test->randomInit_(d, activityMode::ALLPARAMETERS, gr);
				p_test->value() ? nTrue++ : nFalse++;
			}

			// We allow a slight deviation, as the initialization is a random process
			double expectedTrueMin = 0.8 * d * nTests;
			double expectedTrueMax = 1.2 * d * nTests;

			BOOST_CHECK_MESSAGE(
				double(nTrue) > expectedTrueMin && double(nTrue) < expectedTrueMax, "\n"
				<< "d = " << d << "\n"
				<< "Allowed window = " <<
				expectedTrueMin << " - " <<
				expectedTrueMax << "\n"
				<< "nTests = " << nTests << "\n"
				<< "nTrue = " << nTrue << "\n"
				<< "nFalse = " << nFalse << "\n"
			);
		}
	}

	// --------------------------------------------------------------------------

	{ // Check that random initialization can be blocked for equal distributions
		std::shared_ptr <GBooleanObject> p_test1 = this->clone<GBooleanObject>();
		std::shared_ptr <GBooleanObject> p_test2 = this->clone<GBooleanObject>();

		// Assign a boolean value true
		BOOST_CHECK_NO_THROW(*p_test1 = true);
		// Cross-check
		BOOST_CHECK(p_test1->value());

		// Block random initialization and cross check
		BOOST_CHECK_NO_THROW(p_test1->blockRandomInitialization());
		BOOST_CHECK(p_test1->randomInitializationBlocked());

		// Load the data into p_test2
		BOOST_CHECK_NO_THROW(p_test2->load(p_test1));

		// Check that both objects are equal
		BOOST_CHECK(*p_test1 == *p_test2);

		// Check that random initialization is also blocked for p_test2
		BOOST_CHECK(p_test2->randomInitializationBlocked());

		// Try to randomly initialize, using the *external* function
		BOOST_CHECK_NO_THROW(p_test1->randomInit(activityMode::ALLPARAMETERS, gr));

		// Check that both objects are still the same
		BOOST_CHECK(*p_test1 == *p_test2);
	}

	// --------------------------------------------------------------------------

	{ // Check that random initialization can be blocked for distributions with a given probability structure
		std::shared_ptr <GBooleanObject> p_test1 = this->clone<GBooleanObject>();
		std::shared_ptr <GBooleanObject> p_test2 = this->clone<GBooleanObject>();

		// Assign a boolean value true
		BOOST_CHECK_NO_THROW(*p_test1 = true);
		// Cross-check
		BOOST_CHECK(p_test1->value()); // Should be true

		// Block random initialization and cross check
		BOOST_CHECK_NO_THROW(p_test1->blockRandomInitialization());
		BOOST_CHECK(p_test1->randomInitializationBlocked());

		// Load the data into p_test2
		BOOST_CHECK_NO_THROW(p_test2->load(p_test1));

		// Check that both objects are equal
		BOOST_CHECK(*p_test1 == *p_test2);

		// Check that random initialization is also blocked for p_test2
		BOOST_CHECK(p_test2->randomInitializationBlocked());

		// Try to randomly initialize, using the *external* function
		BOOST_CHECK_NO_THROW(p_test1->randomInit(0.7, activityMode::ALLPARAMETERS, gr));

		// Check that both objects are still the same
		BOOST_CHECK(*p_test1 == *p_test2);
	}

	// --------------------------------------------------------------------------

	{ // Check that the fp-family of functions doesn't have an effect on this object
		std::shared_ptr <GBooleanObject> p_test1 = this->GObject::clone<GBooleanObject>();
		std::shared_ptr <GBooleanObject> p_test2 = this->GObject::clone<GBooleanObject>();
		std::shared_ptr <GBooleanObject> p_test3 = this->GObject::clone<GBooleanObject>();

		// Assign a boolean value true
		BOOST_CHECK_NO_THROW(*p_test1 = true);
		// Cross-check
		BOOST_CHECK(p_test1->value()); // should be true

		// Load into p_test2 and p_test3 and test equality
		BOOST_CHECK_NO_THROW(p_test2->load(p_test1));
		BOOST_CHECK_NO_THROW(p_test3->load(p_test1));
		BOOST_CHECK(*p_test2 == *p_test1);
		BOOST_CHECK(*p_test3 == *p_test1);
		BOOST_CHECK(*p_test3 == *p_test2);

		// Check that initialization with a fixed floating point value has no effect on this object
		BOOST_CHECK_NO_THROW(p_test2->fixedValueInit<double>(2., activityMode::ALLPARAMETERS));
		BOOST_CHECK(*p_test2 == *p_test1);

		// Check that multiplication with a fixed floating point value has no effect on this object
		BOOST_CHECK_NO_THROW(p_test2->multiplyBy<double>(2., activityMode::ALLPARAMETERS));
		BOOST_CHECK(*p_test2 == *p_test1);

		// Check that a component-wise multiplication with a random fp value in a given range does not have an effect on this object
		BOOST_CHECK_NO_THROW(p_test2->multiplyByRandom<double>(1., 2., activityMode::ALLPARAMETERS, gr));
		BOOST_CHECK(*p_test2 == *p_test1);

		// Check that a component-wise multiplication with a random fp value in the range [0:1[ does not have an effect on this object
		BOOST_CHECK_NO_THROW(p_test2->multiplyByRandom<double>(activityMode::ALLPARAMETERS, gr));
		BOOST_CHECK(*p_test2 == *p_test1);

		// Check that adding p_test1 to p_test3 does not have an effect
		BOOST_CHECK_NO_THROW(p_test3->add<double>(p_test1, activityMode::ALLPARAMETERS));
		BOOST_CHECK(*p_test3 == *p_test2);

		// Check that subtracting p_test1 from p_test3 does not have an effect
		BOOST_CHECK_NO_THROW(p_test3->subtract<double>(p_test1, activityMode::ALLPARAMETERS));
		BOOST_CHECK(*p_test3 == *p_test2);
	}

	// --------------------------------------------------------------------------

	// Restore the adaptor to pristine condition
	this->resetAdaptor();

	// Load the old adaptor, if needed
	if (adaptorStored) {
		this->addAdaptor(storedAdaptor);
	}

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
	Gem::Common::condnotset("GBooleanObject::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GBooleanObject::specificTestsFailuresExpected_GUnitTests_() {
#ifdef GEM_TESTING

	// Make sure we have an appropriate adaptor loaded when performing these tests
	bool adaptorStored = false;
	std::shared_ptr<GAdaptorT<bool>> storedAdaptor;

	if (this->hasAdaptor()) {
		storedAdaptor = this->getAdaptor();
		adaptorStored = true;
	}

	std::shared_ptr<GBooleanAdaptor> gba_ptr(new GBooleanAdaptor(1.0));
	gba_ptr->setAdaptionThreshold(0); // Make sure the adaptor's internal parameters don't change through the adaption
	gba_ptr->setAdaptionMode(adaptionMode::ALWAYS); // Always adapt
	this->addAdaptor(gba_ptr);

	// Call the parent class'es function
	GParameterT<bool>::specificTestsFailuresExpected_GUnitTests_();

	// Restore the adaptor to pristine condition
	this->resetAdaptor();

	// Load the old adaptor, if needed
	if (adaptorStored) {
		this->addAdaptor(storedAdaptor);
	}

	// A random generator
	Gem::Hap::GRandomT<Gem::Hap::RANDFLAVOURS::RANDOMPROXY> gr;

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
	Gem::Common::condnotset("GBooleanObject::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */
