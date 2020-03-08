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

#include "common/GExpectationChecksT.hpp"

namespace Gem {
namespace Common {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
// Identifies test counter and success counter
const std::size_t TESTCOUNTER = 0;
const std::size_t SUCCESSCOUNTER = 1;

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * The standard constructor -- initialization with class name and expectation
 */
GToken::GToken(
	std::string caller
	, Gem::Common::expectation e
)
	: m_test_counter(std::make_tuple(std::size_t(0), std::size_t(0)))
	, m_caller(std::move(caller))
	, m_e(e)
{ /* nothing */ }

/******************************************************************************/
/**
 * Increments the test counter
 */
void GToken::incrTestCounter() {
	std::get<TESTCOUNTER>(m_test_counter) += 1;
}

/******************************************************************************/
/**
 * Increments the counter of tests that met the expectation
 */
void GToken::incrSuccessCounter() {
	std::get<SUCCESSCOUNTER>(m_test_counter) += 1;
}

/******************************************************************************/
/**
 * Allows to retrieve the current state of the success counter
 */
std::size_t GToken::getSuccessCounter() const {
	return std::get<SUCCESSCOUNTER>(m_test_counter);
}

/******************************************************************************/
/** @brief Allows to retrieve the current state of the test counter */
std::size_t GToken::getTestCounter() const {
	return std::get<TESTCOUNTER>(m_test_counter);
}

/******************************************************************************/
/**
 * Allows to check whether the expectation was met
 */
bool GToken::expectationMet() const {
	switch (m_e) {
		case Gem::Common::expectation::FP_SIMILARITY:
		case Gem::Common::expectation::EQUALITY:
			if (std::get<TESTCOUNTER>(m_test_counter) == std::get<SUCCESSCOUNTER>(m_test_counter)) {
				return true;
			}
			break;

		case Gem::Common::expectation::INEQUALITY:
			if (std::get<SUCCESSCOUNTER>(m_test_counter) > 0) {
				return true;
			}
			break;
	}

	return false;
}

/******************************************************************************/
/**
 * Conversion to a boolean indicating whether the expectation was met
 */
GToken::operator bool() const {
	return this->expectationMet();
}

/******************************************************************************/
/**
 * Allows to retrieve the expectation token
 */
Gem::Common::expectation GToken::getExpectation() const {
	return m_e;
}

/******************************************************************************/
/**
 * Allows to retrieve the expectation token as a string
 */
std::string GToken::getExpectationStr() const {
	switch (m_e) {
		case Gem::Common::expectation::FP_SIMILARITY:
			return "FP_SIMILARITY";
			break;

		case Gem::Common::expectation::EQUALITY:
			return "EQUALITY";
			break;

		case Gem::Common::expectation::INEQUALITY:
			return "INEQUALITY";
			break;
		default:
			return "unknown";
			break;
	}
}

/******************************************************************************/
/**
 * Allows to retrieve the name of the caller
 */
std::string GToken::getCallerName() const {
	return m_caller;
}

/******************************************************************************/
/**
 * Allows to register an error message e.g. obtained from a failed check
 */
void GToken::registerErrorMessage(std::string const &m) {
	if (not m.empty()) {
		m_error_messages.push_back(m);
	} else {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GToken::registerErrorMessage(): Error" << std::endl
				<< "Tried to register empty error message" << std::endl
		);
	}
}

/******************************************************************************/
/**
 * Allows to register an exception obtained from a failed check
 */
void GToken::registerErrorMessage(g_expectation_violation const & g) {
	m_error_messages.emplace_back(g.what());
}

/******************************************************************************/
/**
 * Allows to retrieve the currently registered error messages
 */
std::string GToken::getErrorMessages() const {
	std::string result;
	result = "Registered errors:\n";
	for(auto const& error: m_error_messages) {
		result += error;
	}
	return result;
}

/******************************************************************************/
/**
 * Conversion to a string indicating success or failure
 */
std::string GToken::toString() const {
	std::string result = "Expectation of ";

	switch (m_e) {
		case Gem::Common::expectation::FP_SIMILARITY: {
			result += std::string("FP_SIMILARITY was ");
		}
			break;

		case Gem::Common::expectation::EQUALITY: {
			result += std::string("EQUALITY was ");
		}
			break;

		case Gem::Common::expectation::INEQUALITY: {
			result += std::string("INEQUALITY was ");
		}
			break;
	}

	if (this->expectationMet()) {
		result += std::string("met in ") + m_caller + std::string("\n");
	} else {
		result += std::string("not met in ") + m_caller + std::string("\n");
		// We only add specific information about failed checks for the expectation
		// "inequality", so we do not swamp the user with useless information.
		// For the inequality information, just one out of many checks for data-
		// inequality must be met, so the "equal" comparisons are of no concern.
		// Only the information "everything is equal while inequality was expected"
		// is important. If equality or similarity were expected, every single
		// deviation from equality is of interest.
		if (Gem::Common::expectation::INEQUALITY != m_e) {
			for(auto const& error: m_error_messages) {
				result += error;
			}
		}
	}

	return result;
}

/******************************************************************************/
/**
 * Evaluates the information in this object
 */
void GToken::evaluate() const {
	if (not this->expectationMet()) {
		throw(g_expectation_violation(this->toString()));
	}
}

/******************************************************************************/
/**
 * Easy output of GToken objects
 */
std::ostream &operator<<(std::ostream &s, GToken const & g) {
	s
		<< "GToken for caller " << g.getCallerName() << " with expectation  " << g.getExpectationStr() << ":" << std::endl
		<< "Test counter:     " << g.getTestCounter() << std::endl
		<< "Success counter:  " << g.getSuccessCounter() << std::endl
		<< g.getErrorMessages() << std::endl;
	return s;
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This function checks whether two objects of type boost::logic::tribool meet a given expectation.
 *
 * @param x The first vector to be compared
 * @param y The second vector to be compared
 * @param x_name The name of the first parameter
 * @param y_name The name of the second parameter
 * @param e The expectation both parameters need to fulfill
 * @param limit The maximum allowed deviation of two floating point values
 */
void compare(
	boost::logic::tribool const & x
	, boost::logic::tribool const & y
	, std::string const & x_name
	, std::string const & y_name
	, Gem::Common::expectation e
	, double
) {
	bool expectationMet = false;
	std::string expectation_str;

	switch (e) {
		case Gem::Common::expectation::FP_SIMILARITY:
		case Gem::Common::expectation::EQUALITY:
			expectation_str = "FP_SIMILARITY / EQUALITY";
			if ((x == true && y == true) ||
				 (x == false && y == false) ||
				 (boost::logic::indeterminate(x) && boost::logic::indeterminate(y))) {
				expectationMet = true;
			}
			break;

		case Gem::Common::expectation::INEQUALITY:
			expectation_str = "INEQUALITY";
			if (not (x == true && y == true) &&
				 not (x == false && y == false) &&
				 not (boost::logic::indeterminate(x) && boost::logic::indeterminate(y))) {
				expectationMet = true;
			}
			break;
	};

	if (not expectationMet) {
		std::ostringstream error;
		error
			<< "Expectation of " << expectation_str << " was violated for parameters " << std::endl
			<< "[" << std::endl
			<< x_name << " = " << x << std::endl
			<< y_name << " = " << y << std::endl
			<< "]" << std::endl;
		throw g_expectation_violation(error.str());
	}
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Common */
} /* namespace Gem */
