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

#include "geneva/GParameterBase.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * Loads the data of another GObject
 *
 * @param cp A copy of another GParameterBase object, camouflaged as a GObject
 */
void GParameterBase::load_(const GObject *cp) {
	// Check that we are dealing with a GParameterBase reference independent of this object and convert the pointer
	const GParameterBase *p_load = Gem::Common::g_convert_and_compare<GObject, GParameterBase>(cp, this);

	// Load the parent class'es data
	GObject::load_(cp);

	// Load local data
	m_adaptionsActive = p_load->m_adaptionsActive;
	m_randomInitializationBlocked = p_load->m_randomInitializationBlocked;
	m_parameterName = p_load->m_parameterName;
}

/******************************************************************************/
/**
 * Calls the function that does the actual adaption (which is in turn implemented
 * by derived classes. Will omit adaption if the adaptionsActive_ parameter is set.
 *
 * @return The number of adaptions that were performed
 */
std::size_t GParameterBase::adapt(Gem::Hap::GRandomBase& gr) {
	if (m_adaptionsActive) {
		return adapt_(gr); // Will determine whether a modification was made
	} else {
		return 0;
	}
}

/******************************************************************************/
/**
 * Allows to update the way adaptors work depending on the number of iterations
 * without impprovements.
 *
 * @param nStalls The number of iterations without improvements
 * @return A boolean indicating whether adaptors were changed
 */
bool GParameterBase::updateAdaptorsOnStall(std::size_t nStalls) {
    return updateAdaptorsOnStall_(nStalls);
}

/******************************************************************************/
/**
 * Retrieves information from an adaptor on a given property
 */
void GParameterBase::queryAdaptor(
	const std::string &adaptorName
	, const std::string &property
	, std::vector<boost::any> &data
) const {
	queryAdaptor_(adaptorName, property, data);
}

/******************************************************************************/
/**
 * Switches on adaptions for this object
 */
bool GParameterBase::setAdaptionsActive() {
	bool previous = m_adaptionsActive;
	m_adaptionsActive = true;
	return previous;
}

/******************************************************************************/
/**
 * Disables adaptions for this object
 */
bool GParameterBase::setAdaptionsInactive() {
	bool previous = m_adaptionsActive;
	m_adaptionsActive = false;
	return previous;
}

/* -----------------------------------------------------------------------------
 * Tested in GParameterBase::specificTestsNoFailuresExpected_GUnitTests()
 * -----------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Determines whether adaptions are performed for this object
 *
 * @return A boolean indicating whether adaptions are performed for this object
 */
bool GParameterBase::adaptionsActive() const {
	return m_adaptionsActive;
}

/* -----------------------------------------------------------------------------
 * Tested in GParameterBase::specificTestsNoFailuresExpected_GUnitTests()
 * -----------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Determines whether adaptions are inactive for this object
 *
 * @return A boolean indicating whether adaptions are inactive for this object
 */
bool GParameterBase::adaptionsInactive() const {
	return not m_adaptionsActive;
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
void GParameterBase::compare_(
	const GObject &cp, const Gem::Common::expectation &e, const double &limit
) const {
	using namespace Gem::Common;

	// Check that we are dealing with a GParameterBase reference independent of this object and convert the pointer
	const GParameterBase *p_load = Gem::Common::g_convert_and_compare<GObject, GParameterBase>(cp, this);

	GToken token("GParameterBase", e);

	// Compare our parent data ...
	Gem::Common::compare_base_t<GObject>(*this, *p_load, token);

	// ... and then the local data
	compare_t(IDENTITY(m_adaptionsActive, p_load->m_adaptionsActive), token);
	compare_t(IDENTITY(m_randomInitializationBlocked, p_load->m_randomInitializationBlocked), token);
	compare_t(IDENTITY(m_parameterName, p_load->m_parameterName), token);

	// React on deviations from the expectation
	token.evaluate();
}

/***********************************************************************************/
/**
 * Emits a name for this class / object
 */
std::string GParameterBase::name_() const {
	return std::string("GParameterBase");
}

/***********************************************************************************/
/**
 * Allows to assign a name to this parameter
 */
void GParameterBase::setParameterName(const std::string &pn) {
	m_parameterName = pn;
}

/***********************************************************************************/
/** @brief Allows to retrieve the name of this parameter */
std::string GParameterBase::getParameterName() const {
	return m_parameterName;
}

/***********************************************************************************/
/**
 * Checks whether this object matches a given activity mode. This helper function
 * saves us from having to repeat this switch statement in all GParmaterBase-derivatives.
 *
 * @param am The desired activity mode (ACTIVEONLY, ALLPARAMETERS or INACTIVEONLY)
 * @return A boolean indicating whether a given GParameterBase-derivative matches the activity mode
 */
bool GParameterBase::amMatch(const activityMode &am) const {
	switch (am) {
		case activityMode::ACTIVEONLY: {
			if (this->adaptionsActive()) {
				return true;
			} else {
				return false;
			}
		}
			break;

		case activityMode::ALLPARAMETERS: {
			return true;
		}
			break;

		case activityMode::INACTIVEONLY: {
			if (this->adaptionsInactive()) {
				return true;
			} else {
				return false;
			}
		}
			break;
	}

	throw gemfony_exception(
		g_error_streamer(DO_LOG,  time_and_place)
			<< "In GParameterBase::amMatch(const activityMode& am): Error!" << std::endl
			<< "This line should never be reached" << std::endl
	);

	// Make the compiler happy
	return false;
}

/***********************************************************************************/
/**
 * Returns true on the case of an activity mode mismatch
 */
bool GParameterBase::amMismatch(const activityMode &am) const {
	return not amMatch(am);
}

/******************************************************************************/
/**
 * Checks whether this object matches a given activity mode and is modifiable
 */
bool GParameterBase::modifiableAmMatchOrHandover(const activityMode &am) const {
	return ((this->isLeaf() && this->amMatch(am)) || not this->isLeaf());
}

/***********************************************************************************/
/**
 * Lets the audience know whether this is a leaf or a branch object
 */
bool GParameterBase::isLeaf() const {
	return false;
}

/******************************************************************************/
/**
 * Convenience function so we do not need to always cast  derived classes.
 * See GParameterBaseWithAdaptors::hasAdaptors() for the "real"
 * function.
 */
bool GParameterBase::hasAdaptor() const {
	return false;
}

/******************************************************************************/
/**
 * Triggers random initialization of the parameter(-collection). This is the public
 * version of this function, which only acts if initialization has not been blocked.
 */
bool GParameterBase::randomInit(
	const activityMode &am
	, Gem::Hap::GRandomBase& gr
) {
	if (not m_randomInitializationBlocked && this->modifiableAmMatchOrHandover(am)) {
		return randomInit_(am, gr);
	} else {
		return false;
	}
}

/******************************************************************************/
/**
 * Allows to identify whether we are dealing with a collection or an individual parameter.
 * This function needs to be overloaded for parameter collections so that it returns the
 * correct value.
 *
 * @return A boolean indicating whether the GParameterBase-derivative is an individual parameter
 */
bool GParameterBase::isIndividualParameter() const {
	return isIndividualParameter_();
}

/******************************************************************************/
/**
 * Allows to identify whether we are dealing with a collection or an individual parameter.
 * This function needs to be overloaded for parameter collections so that it returns the
 * correct value.
 *
 * @return A boolean indicating whether the GParameterBase-derivative is an individual parameter
 */
bool GParameterBase::isIndividualParameter_() const {
	return true;
}

/******************************************************************************/
/**
 * Allows to identify whether we are dealing with a collection or an individual parameter.
 * As GParameterBase derivates can be either individual parameters or parameter collections,
 * this function just returns the inverse of isIndividualParameter() .
 *
 * @return A boolean indicating whether the GParameterBase-derivative is a collection
 */
bool GParameterBase::isParameterCollection() const {
	return not isIndividualParameter();
}

/******************************************************************************/
/**
 * Attach parameters of type double to the vector. This function does nothing by
 * default. Parameter types based on doubles need to overload this function and do
 * the actual work.
 */
void GParameterBase::floatStreamline(
	std::vector<float> &parVec, const activityMode &
) const {
	/* do nothing by default */
}

/* -----------------------------------------------------------------------------
 * So far untested
 * -----------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Attach parameters of type double to the vector. This function does nothing by
 * default. Parameter types based on doubles need to overload this function and do
 * the actual work.
 */
void GParameterBase::doubleStreamline(
	std::vector<double> &parVec, const activityMode &
) const {
	/* do nothing by default */
}

/* -----------------------------------------------------------------------------
 * So far untested
 * -----------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Attach parameters of type std::int32_t to the vector. This function does nothing by
 * default. Parameter types based on std::int32_t need to overload this function and do
 * the actual work.
 */
void GParameterBase::int32Streamline(
	std::vector<std::int32_t> &parVec, const activityMode &
) const {
	/* do nothing by default */
}

/* -----------------------------------------------------------------------------
 * So far untested
 * -----------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Attach parameters of type bool to the vector. This function does nothing by
 * default. Parameter types based on bool need to overload this function and do
 * the actual work.
 */
void GParameterBase::booleanStreamline(
	std::vector<bool> &parVec, const activityMode &
) const {
	/* do nothing by default */
}

/* -----------------------------------------------------------------------------
 * So far untested
 * -----------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Attach parameters of type double to the map. This function does nothing by
 * default. Parameter types based on doubles need to overload this function and do
 * the actual work.
 */
void GParameterBase::floatStreamline(
	std::map<std::string, std::vector<float>> &parVec, const activityMode &
) const {
	/* do nothing by default */
}

/* -----------------------------------------------------------------------------
 * So far untested
 * -----------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Attach parameters of type double to the map. This function does nothing by
 * default. Parameter types based on doubles need to overload this function and do
 * the actual work.
 */
void GParameterBase::doubleStreamline(
	std::map<std::string, std::vector<double>> &parVec, const activityMode &
) const {
	/* do nothing by default */
}

/* -----------------------------------------------------------------------------
 * So far untested
 * -----------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Attach parameters of type std::int32_t to the map. This function does nothing by
 * default. Parameter types based on std::int32_t need to overload this function and do
 * the actual work.
 */
void GParameterBase::int32Streamline(
	std::map<std::string, std::vector<std::int32_t>> &parVec, const activityMode &
) const {
	/* do nothing by default */
}

/* -----------------------------------------------------------------------------
 * So far untested
 * -----------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Attach parameters of type bool to the map. This function does nothing by
 * default. Parameter types based on bool need to overload this function and do
 * the actual work.
 */
void GParameterBase::booleanStreamline(
	std::map<std::string, std::vector<bool>> &parVec, const activityMode &
) const {
	/* do nothing by default */
}

/* -----------------------------------------------------------------------------
 * So far untested
 * -----------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Attach boundaries of type float to the vectors
 *
 * @param lBndVec A vector of lower float parameter boundaries
 * @param uBndVec A vector of upper float parameter boundaries
 */
void GParameterBase::floatBoundaries(
	std::vector<float> &lBndVec, std::vector<float> &uBndVec, const activityMode &
) const {
	/* do nothing by default */
}

/******************************************************************************/
/**
 * Attach boundaries of type double to the vectors
 *
 * @param lBndVec A vector of lower double parameter boundaries
 * @param uBndVec A vector of upper double parameter boundaries
 */
void GParameterBase::doubleBoundaries(
	std::vector<double> &lBndVec, std::vector<double> &uBndVec, const activityMode &
) const {
	/* do nothing by default */
}

/******************************************************************************/
/**
 * Attach boundaries of type std::int32_t to the vectors
 *
 * @param lBndVec A vector of lower std::int32_t parameter boundaries
 * @param uBndVec A vector of upper std::int32_t parameter boundaries
 */
void GParameterBase::int32Boundaries(
	std::vector<std::int32_t> &lBndVec, std::vector<std::int32_t> &uBndVec, const activityMode &
) const {
	/* do nothing by default */
}

/******************************************************************************/
/**
 * Attach boundaries of type bool to the vectors
 *
 * @param lBndVec A vector of lower bool parameter boundaries
 * @param uBndVec A vector of upper bool parameter boundaries
 */
void GParameterBase::booleanBoundaries(
	std::vector<bool> &lBndVec, std::vector<bool> &uBndVec, const activityMode &
) const {
	/* do nothing by default */
}

/******************************************************************************/
/**
 * Count the number of float parameters.
 *
 * @param am An enum indicating whether only information about active, inactive or all parameters of this type should be extracted
 * @return The number of float parameters in this object
 */
std::size_t GParameterBase::countFloatParameters(
	const activityMode &am
) const {
	return 0;
}

/* -----------------------------------------------------------------------------
 * So far untested
 * -----------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Count the number of double parameters. The actual work needs to be done by
 * derived classes, if they possess double parameters.
 *
 * @param am An enum indicating whether only information about active, inactive or all parameters of this type should be extracted
 * @return The number of double parameters in this object
 */
std::size_t GParameterBase::countDoubleParameters(
	const activityMode &am
) const {
	return 0;
}

/* -----------------------------------------------------------------------------
 * So far untested
 * -----------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Count the number of std::int32_t parameters. The actual work needs to be done by
 * derived classes, if they possess std::int32_t parameters.
 *
 * @param am An enum indicating whether only information about active, inactive or all parameters of this type should be extracted
 * @return The number of std::int32_t parameters in this object
 */
std::size_t GParameterBase::countInt32Parameters(
	const activityMode &am
) const {
	return 0;
}

/* -----------------------------------------------------------------------------
 * So far untested
 * -----------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Count the number of bool parameters. The actual work needs to be done by
 * derived classes, if they possess bool parameters.
 *
 * @param am An enum indicating whether only information about active, inactive or all parameters of this type should be extracted
 * @return The number of bool parameters in this object
 */
std::size_t GParameterBase::countBoolParameters(
	const activityMode &am
) const {
	return 0;
}

/* -----------------------------------------------------------------------------
 * So far untested
 * -----------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Assigns part of a value vector to the parameter
 */
void GParameterBase::assignFloatValueVector(
	const std::vector<float> &parVec, std::size_t &pos, const activityMode &
) {
	/* Do nothing by default */
}

/* -----------------------------------------------------------------------------
 * So far untested
 * -----------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Assigns part of a value vector to the parameter
 */
void GParameterBase::assignDoubleValueVector(
	const std::vector<double> &parVec, std::size_t &pos, const activityMode &
) {
	/* Do nothing by default */
}

/* -----------------------------------------------------------------------------
 * So far untested
 * -----------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Assigns part of a value vector to the parameter
 */
void GParameterBase::assignInt32ValueVector(
	const std::vector<std::int32_t> &parVec, std::size_t &pos, const activityMode &
) {
	/* Do nothing by default */
}

/* -----------------------------------------------------------------------------
 * So far untested
 * -----------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Assigns part of a value vector to the parameter
 */
void GParameterBase::assignBooleanValueVector(
	const std::vector<bool> &parVec, std::size_t &pos, const activityMode &
) {
	/* Do nothing by default */
}

/* -----------------------------------------------------------------------------
 * So far untested
 * -----------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Assigns part of a value vector to the parameter
 */
void GParameterBase::assignFloatValueVectors(
	const std::map<std::string, std::vector<float>> &parMap, const activityMode &
) {
	/* Do nothing by default */
}

/* -----------------------------------------------------------------------------
 * So far untested
 * -----------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Assigns part of a value vector to the parameter
 */
void GParameterBase::assignDoubleValueVectors(
	const std::map<std::string, std::vector<double>> &parMap, const activityMode &
) {
	/* Do nothing by default */
}

/* -----------------------------------------------------------------------------
 * So far untested
 * -----------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Assigns part of a value vector to the parameter
 */
void GParameterBase::assignInt32ValueVectors(
	const std::map<std::string, std::vector<std::int32_t>> &parMap, const activityMode &
) {
	/* Do nothing by default */
}

/* -----------------------------------------------------------------------------
 * So far untested
 * -----------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Assigns part of a value vector to the parameter
 */
void GParameterBase::assignBooleanValueVectors(
	const std::map<std::string, std::vector<bool>> &parMap, const activityMode &
) {
	/* Do nothing by default */
}

/* -----------------------------------------------------------------------------
 * So far untested
 * -----------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Multiplication with a random value in a given range
 */
void GParameterBase::floatMultiplyByRandom(
	const float &min
	, const float &max
	, const activityMode &am
	, Gem::Hap::GRandomBase& gr
) {
	/* Do nothing by default */
}

/******************************************************************************/
/**
 * Multiplication with a random value in a given range
 */
void GParameterBase::doubleMultiplyByRandom(
	const double &min
	, const double &max
	, const activityMode &am
	, Gem::Hap::GRandomBase& gr
) {
	/* Do nothing by default */
}

/******************************************************************************/
/**
 * Multiplication with a random value in a given range
 */
void GParameterBase::int32MultiplyByRandom(
	const std::int32_t &min
	, const std::int32_t &max
	, const activityMode &am
	, Gem::Hap::GRandomBase& gr
) {
	/* Do nothing by default */
}

/******************************************************************************/
/**
 * Multiplication with a random value in a given range
 */
void GParameterBase::booleanMultiplyByRandom(
	const bool &min
	, const bool &max
	, const activityMode &am
	, Gem::Hap::GRandomBase& gr
) {
	// Complain: This function should not be called for boolean values
	throw gemfony_exception(
		g_error_streamer(DO_LOG,  time_and_place)
			<< "In GParameterBase::booleanMultiplyByRandom(min,max): Error!" << std::endl
			<< "This function should not be called for boolean parameters" << std::endl
	);
}

/******************************************************************************/
/**
 * Multiplication with a random value in the range [0,1[
 */
void GParameterBase::floatMultiplyByRandom(
	const activityMode &am
	, Gem::Hap::GRandomBase& gr
) {
	/* Do nothing by default */
}

/******************************************************************************/
/**
 * Multiplication with a random value in the range [0,1[
 */
void GParameterBase::doubleMultiplyByRandom(
	const activityMode &am
	, Gem::Hap::GRandomBase& gr
) {
	/* Do nothing by default */
}

/******************************************************************************/
/**
 * Multiplication with a random value in the range [0,1[
 */
void GParameterBase::int32MultiplyByRandom(
	const activityMode &am
	, Gem::Hap::GRandomBase& gr
) {
	/* Do nothing by default */
}

/******************************************************************************/
/**
 * Multiplication with a random value in the range [0,1[
 */
void GParameterBase::booleanMultiplyByRandom(
	const activityMode &am
	, Gem::Hap::GRandomBase& gr
) {
	// Complain: This function should not be called for boolean values
	throw gemfony_exception(
		g_error_streamer(DO_LOG,  time_and_place)
			<< "In GParameterBase::booleanMultiplyByRandom(): Error!" << std::endl
			<< "This function should not be called for boolean parameters" << std::endl
	);
}

/******************************************************************************/
/**
 * Multiplication with a constant value
 */
void GParameterBase::floatMultiplyBy(
	const float &value
	, const activityMode &am
) {
	/* Do nothing by default */
}

/******************************************************************************/
/**
 * Multiplication with a constant value
 */
void GParameterBase::doubleMultiplyBy(
	const double &value
	, const activityMode &am
) {
	/* Do nothing by default */
}

/******************************************************************************/
/**
 * Multiplication with a constant value
 */
void GParameterBase::int32MultiplyBy(
	const std::int32_t &value
	, const activityMode &am
) {
	/* Do nothing by default */
}

/******************************************************************************/
/**
 * Multiplication with a random value in the range [0,1[
 */
void GParameterBase::booleanMultiplyBy(
	const bool &value
	, const activityMode &am
) {
	// Complain: This function should not be called for boolean values
	throw gemfony_exception(
		g_error_streamer(DO_LOG,  time_and_place)
			<< "In GParameterBase::booleanMultiplyBy(): Error!" << std::endl
			<< "This function should not be called for boolean parameters" << std::endl
	);
}

/******************************************************************************/
/**
 * Initialization with a constant value
 */
void GParameterBase::floatFixedValueInit(
	const float &value
	, const activityMode &am
) {
	/* Do nothing by default */
}

/******************************************************************************/
/**
 * Initialization with a constant value
 */
void GParameterBase::doubleFixedValueInit(
	const double &value, const activityMode &am
) {
	/* Do nothing by default */
}

/******************************************************************************/
/**
 * Initialization with a constant value
 */
void GParameterBase::int32FixedValueInit(
	const std::int32_t &value
	, const activityMode &am
) {
	/* Do nothing by default */
}

/******************************************************************************/
/**
 * Initialization with a constant value
 */
void GParameterBase::booleanFixedValueInit(
	const bool &value
	, const activityMode &am
) {
	/* Do nothing by default */
}

/******************************************************************************/
/**
 * Adds the "same-type" parameters of another GParameterBase object to this one
 */
void GParameterBase::floatAdd(
	std::shared_ptr < GParameterBase >
	, const activityMode &am
) {
/* Do nothing by default */
}

/******************************************************************************/
/**
 * Adds the "same-type" parameters of another GParameterBase object to this one
 */
void GParameterBase::doubleAdd(
	std::shared_ptr < GParameterBase >
	, const activityMode &am
) {
/* Do nothing by default */
}

/******************************************************************************/
/**
 * Adds the "same-type" parameters of another GParameterBase object to this one
 */
void GParameterBase::int32Add(
	std::shared_ptr < GParameterBase >
	, const activityMode &am
) {
/* Do nothing by default */
}

/******************************************************************************/
/**
 * Adds the "same-type" parameters of another GParameterBase object to this one
 */
void GParameterBase::booleanAdd(
	std::shared_ptr < GParameterBase >
	, const activityMode &am
) {
// Complain: This function should not be called for boolean values
	throw gemfony_exception(
		g_error_streamer(DO_LOG,  time_and_place)
			<< "In GParameterBase::booleanAdd(): Error!" << std::endl
			<< "This function should not be called for boolean parameters" << std::endl
	);
}

/******************************************************************************/
/**
 * Subtracts the "same-type" parameters of another GParameterBase object from this one
 */
void GParameterBase::floatSubtract(
	std::shared_ptr < GParameterBase >
	, const activityMode &am
) {
/* Do nothing by default */
}

/******************************************************************************/
/**
 * Subtracts the "same-type" parameters of another GParameterBase object from this one
 */
void GParameterBase::doubleSubtract(
	std::shared_ptr < GParameterBase >
	, const activityMode &am
) {
/* Do nothing by default */
}

/******************************************************************************/
/**
 * Subtracts the "same-type" parameters of another GParameterBase object from this one
 */
void GParameterBase::int32Subtract(
	std::shared_ptr < GParameterBase >
	, const activityMode &am
) {
/* Do nothing by default */
}

/******************************************************************************/
/**
 * Subtracts the "same-type" parameters of another GParameterBase object from this one
 */
void GParameterBase::booleanSubtract(
	std::shared_ptr < GParameterBase >
	, const activityMode &am
) {
	// Complain: This function should not be called for boolean values
	throw gemfony_exception(
		g_error_streamer(DO_LOG,  time_and_place)
			<< "In GParameterBase::booleanSubtract(): Error!" << std::endl
			<< "This function should not be called for boolean parameters" << std::endl
	);
}

/******************************************************************************/
/**
 * Specifies that no random initialization should occur anymore
 */
void GParameterBase::blockRandomInitialization() {
	m_randomInitializationBlocked = true;
}

/* -----------------------------------------------------------------------------
 * Tested in GParameterBase::specificTestsNoFailuresExpected_GUnitTests()
 * -----------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Specifies that no random initialization should occur anymore
 */
void GParameterBase::allowRandomInitialization() {
	m_randomInitializationBlocked = false;
}

/* -----------------------------------------------------------------------------
 * Tested in GParameterBase::specificTestsNoFailuresExpected_GUnitTests()
 * -----------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Checks whether initialization has been blocked
 */
bool GParameterBase::randomInitializationBlocked() const {
	return m_randomInitializationBlocked;
}

/* -----------------------------------------------------------------------------
 * Tested in GParameterBase::specificTestsNoFailuresExpected_GUnitTests()
 * -----------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GParameterBase::modify_GUnitTests_() {
#ifdef GEM_TESTING
	bool result = false;

	// Call the parent class'es function
	if (GObject::modify_GUnitTests_()) result = true;

	return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
	Gem::Common::condnotset("GParameterBase::modify_GUnitTests", "GEM_TESTING");
   return false;
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GParameterBase::specificTestsNoFailureExpected_GUnitTests_() {
#ifdef GEM_TESTING
	// Call the parent class'es function
	GObject::specificTestsNoFailureExpected_GUnitTests_();

	// A random generator
	Gem::Hap::GRandomT<Gem::Hap::RANDFLAVOURS::RANDOMPROXY> gr;

	//---------------------------------------------------------------------

	{ // Test blocking and unblocking of random initialization
		using namespace Gem::Common;

		// Create two GParameterBase objects as clone of this object for further usage
		std::shared_ptr <GParameterBase> p_test1 = this->clone<GParameterBase>();
		std::shared_ptr <GParameterBase> p_test2 = this->clone<GParameterBase>();

		BOOST_CHECK_NO_THROW(p_test1->blockRandomInitialization());
		BOOST_CHECK_NO_THROW(p_test2->blockRandomInitialization());
		BOOST_CHECK(p_test1->randomInitializationBlocked() == true);
		BOOST_CHECK(p_test2->randomInitializationBlocked() == true);

		// Random initialization should leave the object unchanged
		BOOST_CHECK_NO_THROW(p_test1->randomInit(activityMode::ALLPARAMETERS, gr));
		BOOST_CHECK(*p_test1 == *p_test2);

		// Unblock random initialization
		BOOST_CHECK_NO_THROW(p_test1->allowRandomInitialization());
		BOOST_CHECK_NO_THROW(p_test2->allowRandomInitialization());
		BOOST_CHECK(p_test1->randomInitializationBlocked() == false);
		BOOST_CHECK(p_test2->randomInitializationBlocked() == false);

		// Random initialization should change p_test1. We run the test
		// multiple times, as random initialization particularly of
		// boolean parameters may turn out to be the same as before
		if (p_test1->isIndividualParameter()) {
			bool valueChanged = false;
			p_test2->load(p_test1);
			for (int i = 0; i < 100; i++) {
				if (p_test1->randomInit(activityMode::ALLPARAMETERS, gr)) {
					if (*p_test1 != *p_test2) {
						valueChanged = true;
						break;
					}
				}
			}
			BOOST_CHECK(valueChanged);
		}
	}

	//---------------------------------------------------------------------

	{ // Check activating and de-activating adaptions. Note that the
		// effects of these flags can only be tested if an adaptor or
		// multiple adaptors (in the case of object collections) have
		// been loaded.

		// Create some clones of this object
		std::shared_ptr <GParameterBase> p_test = this->clone<GParameterBase>();
		std::shared_ptr <GParameterBase> p_test_1 = this->clone<GParameterBase>();
		std::shared_ptr <GParameterBase> p_test_2 = this->clone<GParameterBase>();

		// activate adaptions
		BOOST_CHECK_NO_THROW(p_test_1->setAdaptionsActive());
		BOOST_CHECK(p_test_1->adaptionsActive() == true);

		// de-activate adaptions
		BOOST_CHECK_NO_THROW(p_test_2->setAdaptionsInactive());
		BOOST_CHECK(p_test_2->adaptionsActive() == false);

		if (p_test_1->hasAdaptor() && p_test_2->hasAdaptor()) {
			bool adapted = false;
			BOOST_CHECK_NO_THROW(
				adapted = p_test_1->adapt(gr)); // Should change, unless we are dealing with an "empty" container type
			BOOST_CHECK_NO_THROW(p_test->setAdaptionsActive()); // Make sure differences do not stem from this flag
			if (adapted) {
				BOOST_CHECK(*p_test_1 != *p_test);
			}

			BOOST_CHECK_NO_THROW(p_test_2->adapt(gr)); // Should stay unchanged
			BOOST_CHECK_NO_THROW(p_test->setAdaptionsInactive()); // Make sure differences do not stem from this flag
			BOOST_CHECK(*p_test_2 == *p_test);
		}
	}

	//---------------------------------------------------------------------

	{ // Check adapt() and (de-)activation of adaptions
		// Create two local clones
		std::shared_ptr <GParameterBase> p_test1 = this->clone<GParameterBase>();
		std::shared_ptr <GParameterBase> p_test2 = this->clone<GParameterBase>();

		// Always adapt
		BOOST_CHECK_NO_THROW(p_test1->setAdaptionsActive());
		BOOST_CHECK_NO_THROW(p_test2->setAdaptionsActive());

		// Check that adaptions are indeed active in both objects
		BOOST_CHECK(p_test1->adaptionsActive() == true);
		BOOST_CHECK(p_test2->adaptionsActive() == true);

		// Check that we have adaption powers when using the random number generator
		if (p_test1->hasAdaptor()) {
			for (std::size_t i = 0; i < 100; i++) {
				bool adapted = false;
				BOOST_CHECK_NO_THROW(adapted = p_test1->adapt(gr));
				if (adapted) {
					BOOST_CHECK(*p_test1 != *p_test2);
				}
				BOOST_CHECK_NO_THROW(p_test1->load(p_test2));
				BOOST_CHECK(*p_test1 == *p_test2);
			}
		}

		// De-activate adaptions in both objects
		BOOST_CHECK_NO_THROW(p_test1->setAdaptionsInactive());
		BOOST_CHECK_NO_THROW(p_test2->setAdaptionsInactive());

		// Check that adaptions are indeed inactive in both objects
		BOOST_CHECK(p_test1->adaptionsActive() == false);
		BOOST_CHECK(p_test2->adaptionsActive() == false);

		// Check that adaptions do not occur anymore in p_test1
		if (p_test1->hasAdaptor()) {
			for (std::size_t i = 0; i < 100; i++) {
				BOOST_CHECK_NO_THROW(p_test1->adapt(gr));
				BOOST_CHECK(*p_test1 == *p_test2);
			}
		}
	}

	//---------------------------------------------------------------------------

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
	Gem::Common::condnotset("GParameterBase::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GParameterBase::specificTestsFailuresExpected_GUnitTests_() {
#ifdef GEM_TESTING
	// Call the parent class'es function
	GObject::specificTestsFailuresExpected_GUnitTests_();

	//---------------------------------------------------------------------------

	{ // Some test

	}

	//---------------------------------------------------------------------------

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
	Gem::Common::condnotset("GParameterBase::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */
