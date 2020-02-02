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

#include "geneva/GConstrainedDoubleObject.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GConstrainedDoubleObject)

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * Initialization with boundaries only. The value is set randomly.
 *
 * @param lowerBoundary The lower boundary of the value range
 * @param upperBoundary The upper boundary of the value range
 */
GConstrainedDoubleObject::GConstrainedDoubleObject(
	const double &lowerBoundary
	, const double &upperBoundary
)
	: GConstrainedFPT<double>(lowerBoundary, upperBoundary)
{ /* nothing */ }

/******************************************************************************/
/**
 * Initialization with value and boundaries
 *
 * @param val Initialization value
 * @param lowerBoundary The lower boundary of the value range
 * @param upperBoundary The upper boundary of the value range
 */
GConstrainedDoubleObject::GConstrainedDoubleObject(
	const double &val
	, const double &lowerBoundary
	, const double &upperBoundary
)
	: GConstrainedFPT<double>(val, lowerBoundary, upperBoundary)
{ /* nothing */ }

/******************************************************************************/
/**
 * Initialization by contained value
 *
 * @param val A value used for the initialization
 */
GConstrainedDoubleObject::GConstrainedDoubleObject(const double &val)
	: GConstrainedFPT<double>(val)
{ /* nothing */ }

/******************************************************************************/
/**
 * An assignment operator for the contained value type
 *
 * @param val The value to be assigned to this object
 * @return The value that was just assigned to this object
 */
GConstrainedDoubleObject& GConstrainedDoubleObject::operator=(const double &val) {
	GConstrainedFPT<double>::operator=(val);
	return *this;
}

/******************************************************************************/
/**
 * Creates a deep clone of this object.
 *
 * @return A copy of this object, camouflaged as a GObject
 */
GObject *GConstrainedDoubleObject::clone_() const {
	return new GConstrainedDoubleObject(*this);
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
void GConstrainedDoubleObject::compare_(
	const GObject &cp, const Gem::Common::expectation &e, const double &limit
) const {
	using namespace Gem::Common;

	// Check that we are dealing with a GConstrainedDoubleObject reference independent of this object and convert the pointer
	const GConstrainedDoubleObject *p_load = Gem::Common::g_convert_and_compare<GObject, GConstrainedDoubleObject>(cp, this);

	GToken token("GConstrainedDoubleObject", e);

	// Compare our parent data ...
	Gem::Common::compare_base_t<GConstrainedFPT<double>>(*this, *p_load, token);

	// .... no local data

	// React on deviations from the expectation
	token.evaluate();
}

/***********************************************************************************/
/**
 * Emits a name for this class / object
 */
std::string GConstrainedDoubleObject::name_() const {
	return std::string("GConstrainedDoubleObject");
}

/******************************************************************************/
/**
 * Attach our local value to the vector. This is used to collect all parameters of this type
 * in the sequence in which they were registered.
 *
 * @param parVec The vector to which the local value should be attached
 */
void GConstrainedDoubleObject::doubleStreamline(
	std::vector<double> &parVec, const activityMode &am
) const {
	// Note: application of the transfer function happens in GConstrainedNumT inside value()
	parVec.push_back(this->value());
}

/******************************************************************************/
/**
 * Attach our local value to the map.
 *
 * @param parVec The map to which the local value should be attached
 */
void GConstrainedDoubleObject::doubleStreamline(
	std::map<std::string, std::vector<double>> &parVec, const activityMode &am
) const {
	std::vector<double> parameters;
	// Note: application of the transfer function happens in GConstrainedNumT inside value()
	parameters.push_back(this->value());
	parVec[this->getParameterName()] = parameters;
}

/******************************************************************************/
/**
 * Attach boundaries of type double to the vectors.
 *
 * @param lBndVec A vector of lower double parameter boundaries
 * @param uBndVec A vector of upper double parameter boundaries
 */
void GConstrainedDoubleObject::doubleBoundaries(
	std::vector<double> &lBndVec, std::vector<double> &uBndVec, const activityMode &am
) const {
	lBndVec.push_back(this->getLowerBoundary());
	uBndVec.push_back(this->getUpperBoundary());
}

/******************************************************************************/
/**
 * Tell the audience that we own a double value
 *
 * @param am An enum indicating whether only information about active, inactive or all parameters of this type should be extracted
 * @return The number 1, as we own a single double parameter
 */
std::size_t GConstrainedDoubleObject::countDoubleParameters(
	const activityMode &am
) const {
	return 1;
}

/******************************************************************************/
/**
 * Assigns part of a value vector to the parameter. Note that we apply a transformation to
 * the parameter value, so that it lies inside of the allowed value range.
 */
void GConstrainedDoubleObject::assignDoubleValueVector(
	const std::vector<double> &parVec, std::size_t &pos, const activityMode &am
) {
#ifdef DEBUG
	// Do we have a valid position ?
	if(pos >= parVec.size()) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GConstrainedDoubleObject::assignDoubleValueVector(const std::vector<double>&, std::size_t&):" << std::endl
				<< "Tried to access position beyond end of vector: " << parVec.size() << "/" << pos << std::endl
		);
	}
#endif

	this->setValue(this->transfer(parVec[pos]));
	pos++;
}

/******************************************************************************/
/**
 * Assigns part of a value map to the parameter
 */
void GConstrainedDoubleObject::assignDoubleValueVectors(
	const std::map<std::string, std::vector<double>> &parMap, const activityMode &am
) {
	this->setValue(
		this->transfer(
			Gem::Common::getMapItem(
				parMap
				, this->getParameterName()
			).at(0)
		)
	);
}

/******************************************************************************/
/**
 * Multiplication with a random value in a given range
 */
void GConstrainedDoubleObject::doubleMultiplyByRandom(
	const double &min
	, const double &max
	, const activityMode &am
	, Gem::Hap::GRandomBase& gr
) {
	std::uniform_real_distribution<double> uniform_real_distribution(min, max);
	GParameterT<double>::setValue(transfer(GParameterT<double>::value() * uniform_real_distribution(gr)));
}

/******************************************************************************/
/**
 * Multiplication with a random value in the range [0,1[
 */
void GConstrainedDoubleObject::doubleMultiplyByRandom(
	const activityMode &am
	, Gem::Hap::GRandomBase& gr
) {
	std::uniform_real_distribution<double> uniform_real_distribution(0., 1.);
	GParameterT<double>::setValue(transfer(GParameterT<double>::value() * uniform_real_distribution(gr)));
}

/******************************************************************************/
/**
 * Multiplication with a constant value
 */
void GConstrainedDoubleObject::doubleMultiplyBy(
	const double &val
	, const activityMode &am
) {
	GParameterT<double>::setValue(transfer(val * GParameterT<double>::value()));
}

/******************************************************************************/
/**
 * Initialization with a constant value
 */
void GConstrainedDoubleObject::doubleFixedValueInit(
	const double &val
	, const activityMode &am
) {
	GParameterT<double>::setValue(transfer(val));
}

/******************************************************************************/
/**
 * Adds the "same-type" parameters of another GParameterBase object to this one
 */
void GConstrainedDoubleObject::doubleAdd(
	std::shared_ptr<GParameterBase> p_base
	, const activityMode &am
) {
	// We first need to convert p_base into the local type
	std::shared_ptr<GConstrainedDoubleObject> p = GParameterBase::parameterbase_cast<GConstrainedDoubleObject>(p_base);
	GParameterT<double>::setValue(transfer(this->value() + p->value()));
}

/******************************************************************************/
/**
 * Adds the "same-type" parameters of another GParameterBase object to this one
 */
void GConstrainedDoubleObject::doubleSubtract(
	std::shared_ptr<GParameterBase > p_base
	, const activityMode &am
) {
	// We first need to convert p_base into the local type
	std::shared_ptr<GConstrainedDoubleObject> p = GParameterBase::parameterbase_cast<GConstrainedDoubleObject>(p_base);
	GParameterT<double>::setValue(transfer(this->value() - p->value()));
}


/******************************************************************************/
/**
 * Loads the data of another GObject
 *
 * @param cp A copy of another GConstrainedDoubleObject object, camouflaged as a GObject
 */
void GConstrainedDoubleObject::load_(const GObject *cp) {
	// Convert the pointer to our target type and check for self-assignment
	const GConstrainedDoubleObject * p_load = Gem::Common::g_convert_and_compare<GObject, GConstrainedDoubleObject>(cp, this);

	// Load our parent class'es data ...
	GConstrainedFPT<double>::load_(cp);

	// ... no local data
}

/******************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GConstrainedDoubleObject::modify_GUnitTests_() {
#ifdef GEM_TESTING
	// A random generator
	Gem::Hap::GRandomT<Gem::Hap::RANDFLAVOURS::RANDOMPROXY> gr;

	bool result = false;

	// Call the parent class'es function
	if (GConstrainedFPT<double>::modify_GUnitTests_()) result = true;

	this->randomInit(activityMode::ALLPARAMETERS, gr);
	result = true;

	return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
	Gem::Common::condnotset("GConstrainedDoubleObject::modify_GUnitTests", "GEM_TESTING");
   return false;
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GConstrainedDoubleObject::specificTestsNoFailureExpected_GUnitTests_() {
#ifdef GEM_TESTING
	// Some general settings
	const double testVal = 42.;
	const double testVal2 = 17.;
	double testVal3 = 0.;
	const double lowerBoundary = 0.;
	const double upperBoundary = 100.;
	const std::size_t NTESTS = 100;

	// Make sure we have an appropriate adaptor loaded when performing these tests
	bool adaptorStored = false;
	std::shared_ptr <GAdaptorT<double>> storedAdaptor;

	if (this->hasAdaptor()) {
		storedAdaptor = this->getAdaptor();
		adaptorStored = true;
	}

	std::shared_ptr <GDoubleGaussAdaptor> gdga_ptr(new GDoubleGaussAdaptor(0.025, 0.1, 0., 1., 1.0));
	gdga_ptr->setAdaptionThreshold(0); // Make sure the adaptor's internal parameters don't change through the adaption
	gdga_ptr->setAdaptionMode(adaptionMode::ALWAYS); // Always adapt
	this->addAdaptor(gdga_ptr);

	// Call the parent class'es function
	GConstrainedFPT<double>::specificTestsNoFailureExpected_GUnitTests_();

	// --------------------------------------------------------------------------

	{ // Check that assignment of a value with operator= works both for set and unset boundaries
		std::shared_ptr <GConstrainedDoubleObject> p_test = this->GObject::clone<GConstrainedDoubleObject>();

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

	// --------------------------------------------------------------------------

	{ // Check construction with two boundaries plus initialization with a random value and extraction of that value
		std::shared_ptr <GConstrainedDoubleObject> p_test(new GConstrainedDoubleObject(0.3, 0.6));
		BOOST_CHECK_NO_THROW(testVal3 = p_test->value());
	}

	// --------------------------------------------------------------------------

	{ // Check construction with two boundaries and a value and extraction of that value
		const double TESTVAL = 0.4;
		std::shared_ptr <GConstrainedDoubleObject> p_test(new GConstrainedDoubleObject(0.4, 0.3, 0.6));
		BOOST_CHECK_NO_THROW(testVal3 = p_test->value());
		BOOST_CHECK(testVal3 == TESTVAL);
	}

	// --------------------------------------------------------------------------

	{ // Check that repeated retrieval of the value always yields the same value
		const double TESTVAL = 0.4;
		std::shared_ptr <GConstrainedDoubleObject> p_test(new GConstrainedDoubleObject(0.4, 0.3, 0.6));
		for (std::size_t i = 0; i < NTESTS; i++) {
			BOOST_CHECK_NO_THROW(testVal3 = p_test->value());
			BOOST_CHECK_MESSAGE(
				testVal3 == TESTVAL, "The value has changed: " << testVal3 << " / " << TESTVAL
			);
		}
	}

	// --------------------------------------------------------------------------

	// Remove the test adaptor
	this->resetAdaptor();

	// Load the old adaptor, if needed
	if (adaptorStored) {
		this->addAdaptor(storedAdaptor);
	}

	// --------------------------------------------------------------------------

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
	Gem::Common::condnotset("GConstrainedDoubleObject::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GConstrainedDoubleObject::specificTestsFailuresExpected_GUnitTests_() {
#ifdef GEM_TESTING
	// Make sure we have an appropriate adaptor loaded when performing these tests
	bool adaptorStored = false;
	std::shared_ptr <GAdaptorT<double>> storedAdaptor;

	if (this->hasAdaptor()) {
		storedAdaptor = this->getAdaptor();
		adaptorStored = true;
	}

	std::shared_ptr <GDoubleGaussAdaptor> gdga_ptr(new GDoubleGaussAdaptor(0.025, 0.1, 0., 1., 1.0));
	gdga_ptr->setAdaptionThreshold(0); // Make sure the adaptor's internal parameters don't change through the adaption
	gdga_ptr->setAdaptionMode(adaptionMode::ALWAYS); // Always adapt
	this->addAdaptor(gdga_ptr);

	// Call the parent class'es function
	GConstrainedFPT<double>::specificTestsFailuresExpected_GUnitTests_();

	// Remove the test adaptor
	this->resetAdaptor();

	// Load the old adaptor, if needed
	if (adaptorStored) {
		this->addAdaptor(storedAdaptor);
	}

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
	Gem::Common::condnotset("GConstrainedDoubleObject::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */
