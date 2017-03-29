/**
 * @file GExpectationChecksT.cpp
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
	const std::string &caller, const Gem::Common::expectation &e
)
	: testCounter_(std::tuple<std::size_t, std::size_t>(std::size_t(0), std::size_t(0))), caller_(caller),
	  e_(e) { /* nothing */ }

/******************************************************************************/
/**
 * Increments the test counter
 */
void GToken::incrTestCounter() {
	std::get<TESTCOUNTER>(testCounter_) += 1;
}

/******************************************************************************/
/**
 * Increments the counter of tests that met the expectation
 */
void GToken::incrSuccessCounter() {
	std::get<SUCCESSCOUNTER>(testCounter_) += 1;
}

/******************************************************************************/
/**
 * Allows to retrieve the current state of the success counter
 */
std::size_t GToken::getSuccessCounter() const {
	return std::get<SUCCESSCOUNTER>(testCounter_);
}

/******************************************************************************/
/** @brief Allows to retrieve the current state of the test counter */
std::size_t GToken::getTestCounter() const {
	return std::get<TESTCOUNTER>(testCounter_);
}

/******************************************************************************/
/**
 * Allows to check whether the expectation was met
 */
bool GToken::expectationMet() const {
	switch (e_) {
		case Gem::Common::expectation::CE_FP_SIMILARITY:
		case Gem::Common::expectation::CE_EQUALITY:
			if (std::get<TESTCOUNTER>(testCounter_) == std::get<SUCCESSCOUNTER>(testCounter_)) {
				return true;
			}
			break;

		case Gem::Common::expectation::CE_INEQUALITY:
			if (std::get<SUCCESSCOUNTER>(testCounter_) > 0) {
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
	return e_;
}

/******************************************************************************/
/**
 * Allows to retrieve the expectation token as a string
 */
std::string GToken::getExpectationStr() const {
	switch (e_) {
		case Gem::Common::expectation::CE_FP_SIMILARITY:
			return std::string("CE_FP_SIMILARITY");
			break;

		case Gem::Common::expectation::CE_EQUALITY:
			return std::string("CE_EQUALITY");
			break;

		case Gem::Common::expectation::CE_INEQUALITY:
			return std::string("CE_INEQUALITY");
			break;
	}
}

/******************************************************************************/
/**
 * Allows to retrieve the name of the caller
 */
std::string GToken::getCallerName() const {
	return caller_;
}

/******************************************************************************/
/**
 * Allows to register an error message e.g. obtained from a failed check
 */
void GToken::registerErrorMessage(const std::string &m) {
	if (!m.empty()) {
		errorMessages_.push_back(m);
	} else {
		glogger
		<< "In GToken::registerErrorMessage(): Error" << std::endl
		<< "Tried to register empty error message" << std::endl
		<< GEXCEPTION;
	}
}

/******************************************************************************/
/**
 * Allows to register an exception obtained from a failed check
 */
void GToken::registerErrorMessage(const g_expectation_violation &g) {
	if (!g.empty()) {
		errorMessages_.push_back(std::string(g.what()));
	} else {
		glogger
		<< "In GToken::registerErrorMessage(): Error" << std::endl
		<< "Tried to register empty exception" << std::endl
		<< GEXCEPTION;
	}
}

/******************************************************************************/
/**
 * Allows to retrieve the currently registered error messages
 */
std::string GToken::getErrorMessages() const {
	std::string result;
	result = "Registered errors:\n";
	std::vector<std::string>::const_iterator cit;
	for (cit = errorMessages_.begin(); cit != errorMessages_.end(); ++cit) {
		result += *cit;
	}
	return result;
}

/******************************************************************************/
/**
 * Conversion to a string indicating success or failure
 */
std::string GToken::toString() const {
	std::string result = "Expectation of ";

	switch (e_) {
		case Gem::Common::expectation::CE_FP_SIMILARITY: {
			result += std::string("CE_FP_SIMILARITY was ");
		}
			break;

		case Gem::Common::expectation::CE_EQUALITY: {
			result += std::string("CE_EQUALITY was ");
		}
			break;

		case Gem::Common::expectation::CE_INEQUALITY: {
			result += std::string("CE_INEQUALITY was ");
		}
			break;
	}

	if (this->expectationMet()) {
		result += std::string("met in ") + caller_ + std::string("\n");
	} else {
		result += std::string("not met in ") + caller_ + std::string("\n");
		// We only add specific information about failed checks for the expectation
		// "inequality", so we do not swamp the user with useless information.
		// For the inequality information, just one out of many checks for data-
		// inequality must be met, so the "equal" comparisons are of no concern.
		// Only the information "everything is equal while inequality was expected"
		// is important. If equality or similarity were expected, every single
		// deviation from equality is of interest.
		if (Gem::Common::expectation::CE_INEQUALITY != e_) {
			std::vector<std::string>::const_iterator cit;
			for (cit = errorMessages_.begin(); cit != errorMessages_.end(); ++cit) {
				result += *cit;
			}
		}
	}

	return result;
}

/******************************************************************************/
/**
 * Evaluates the information in this object
 */
G_API_COMMON void GToken::evaluate() const {
	if (!this->expectationMet()) {
		throw(g_expectation_violation(this->toString()));
	}
}

/******************************************************************************/
/**
 * Easy output of GToken objects
 */
std::ostream &operator<<(std::ostream &s, const GToken &g) {
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
	const boost::logic::tribool & x
	, const boost::logic::tribool &y
	, const std::string &x_name
	, const std::string &y_name
	, const Gem::Common::expectation &e
	, const double &limit
) {
	bool expectationMet = false;
	std::string expectation_str;

	switch (e) {
		case Gem::Common::expectation::CE_FP_SIMILARITY:
		case Gem::Common::expectation::CE_EQUALITY:
			expectation_str = "CE_FP_SIMILARITY / CE_EQUALITY";
			if ((x == true && y == true) ||
				 (x == false && y == false) ||
				 (boost::logic::indeterminate(x) && boost::logic::indeterminate(y))) {
				expectationMet = true;
			}
			break;

		case Gem::Common::expectation::CE_INEQUALITY:
			expectation_str = "CE_INEQUALITY";
			if (!(x == true && y == true) &&
				 !(x == false && y == false) &&
				 !(boost::logic::indeterminate(x) && boost::logic::indeterminate(y))) {
				expectationMet = true;
			}
			break;
	};

	if (!expectationMet) {
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
