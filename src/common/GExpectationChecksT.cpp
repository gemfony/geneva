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

#include <common/GExpectationChecksT.hpp>

namespace Gem {
namespace Common {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
// Identifies test counter and success counter
const std::size_t TESTCOUNTER = 0;
const std::size_t SUCCESSCOUNTER = 0;

/******************************************************************************/
/**
 * The standard constructor
 */
g_expectation_violation::g_expectation_violation(const std::string& description) throw()
      : gemfony_error_condition(description)
{ /* nothing */ }

/******************************************************************************/
/**
 * The destructor
 */
g_expectation_violation::~g_expectation_violation() throw()
{ /* nothing */ }

/******************************************************************************/
/**
 * Allows to add further informtion, automatically terminated by a '\n'.
 * The intended use for this feature is catching and throwing exceptions in
 * an easy way, e.g. in order to create a stack trace:
 *
 * try {
 *   // some condition
 * } catch(g_expectation_violation& g) {
 *    throw g()
 * }
 */
g_expectation_violation& g_expectation_violation::operator()(
   const std::string& s
) throw() {
   this->add(s + "\n");
   return *this;
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * The standard constructor -- initialization with class name and expectation
 */
GToken::GToken(
   const std::string& caller
   , const Gem::Common::expectation& e
)
   : testCounter_(boost::tuple<std::size_t, std::size_t>(std::size_t(0), std::size_t(0)))
   , caller_(caller)
   , e_(e)
{ /* nothing */ }

/******************************************************************************/
/**
 * Increments the test counter
 */
void GToken::incrTestCounter() {
   boost::get<TESTCOUNTER>(testCounter_) += 1;
}

/******************************************************************************/
/**
 * Increments the counter of tests that met the expectation
 */
void GToken::incrSuccessCounter() {
   boost::get<SUCCESSCOUNTER>(testCounter_) += 1;
}

/******************************************************************************/
/**
 * Allows to check whether the xpectation was met
 */
bool GToken::expectationMet() const {
   if(boost::get<TESTCOUNTER>(testCounter_) == boost::get<SUCCESSCOUNTER>(testCounter_)) {
      return true;
   } else {
      return false;
   }
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
 * Allows to register an error message e.g. obtained from a failed check
 */
void GToken::registerErrorMessage(const std::string& m) {
   if(!m.empty()) {
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
 * Conversion to a string indicating success or failure
 */
std::string GToken::toString() const {
   std::string result = "Expectation of ";

   switch(e_) {
      case Gem::Common::CE_FP_SIMILARITY:
      {
         result += std::string("CE_FP_SIMILARITY was ");
      }
      break;

      case Gem::Common::CE_EQUALITY:
      {
         result += std::string("CE_EQUALITY was ");
      }
      break;

      case Gem::Common::CE_INEQUALITY:
      {
         result += std::string("CE_INEQUALITY was ");
      }
      break;

      default:
      {
         glogger
         << "In GToken::toString(): Got invalid expectation " << e_ << std::endl
         << GEXCEPTION;
      }
      break;
   }

   if(this->expectationMet()) {
      result += std::string("met in ") + caller_ + std::string("\n");
   } else {
      result += std::string("not met in ") + caller_ + std::string("\n");
      std::vector<std::string>::const_iterator cit;
      for(cit=errorMessages_.begin(); cit!=errorMessages_.end(); ++cit) {
         result += *cit;
      }
   }

   return result;
}

/******************************************************************************/
/**
 * Evaluates the information in this object
 */
G_API_COMMON void GToken::evaluate() const {
   if(!this->expectationMet()) {
      throw(g_expectation_violation(this->toString()));
   }
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
 * @param dummy Boost::enable_if magic to steer overloaded resolution by the compiler
 */
void compare(
   const boost::logic::tribool& x
   , const boost::logic::tribool& y
   , const std::string& x_name
   , const std::string& y_name
   , const Gem::Common::expectation& e
   , const double& limit
) {
   bool expectationMet = false;
   std::string expectation_str;

   switch(e) {
   case Gem::Common::CE_FP_SIMILARITY:
   case Gem::Common::CE_EQUALITY:
      expectation_str = "CE_FP_SIMILARITY / CE_EQUALITY";
      if((x==true && y==true) ||
         (x==false && y==false) ||
         (boost::logic::indeterminate(x) && boost::logic::indeterminate(y))) {
         expectationMet = true;
      }
      break;

   case Gem::Common::CE_INEQUALITY:
      expectation_str = "CE_INEQUALITY";
      if(!(x==true && y==true) &&
         !(x==false && y==false) &&
         !(boost::logic::indeterminate(x) && boost::logic::indeterminate(y))) {
         expectationMet = true;
      }
      break;

   default:
      {
         glogger
         << "In compare(/* 5 */): Got invalid expectation " << e << std::endl
         << GEXCEPTION;
      }
      break;
   };

   if(!expectationMet) {
      std::ostringstream error;
      error
      << "Expectation of " << expectation_str << " was violated for parameters " << std::endl
      << "[" << std::endl
      <<    x_name << " = " << x << std::endl
      <<    y_name << " = " << y << std::endl
      << "]" << std::endl;
      throw g_expectation_violation(error.str());
   }
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This function checks whether two objects of type boost::logic::tribool meet a given expectation.
 *
 * @param withMessages Specifies whether messages should be emitted in case of failed expectations
 * @param caller The name of the calling class
 * @param x The first tribool parameter to be compared
 * @param y The second tribool parameter to be compared
 * @param x_name The name of the first parameter
 * @param y_name The name of the second parameter
 * @param e The expectation both parameters need to fulfill
 * @param limit The maximum allowed deviation of two floating point values
 * @return A boost::optional<std::string> that optionally contains a description of discrepancies found from the expected outcome
 */
boost::optional<std::string> checkExpectation (
  const bool& withMessages
  , const std::string& caller
  , const boost::logic::tribool& x
  , const boost::logic::tribool& y
  , const std::string& x_name
  , const std::string& y_name
  , const Gem::Common::expectation& e
  , const double& limit
) {
	bool expectationMet = false;
	std::ostringstream message;

	switch(e) {
	case Gem::Common::CE_FP_SIMILARITY:
	case Gem::Common::CE_EQUALITY:
		if((x==true && y==true) ||
		   (x==false && y==false) ||
		   (boost::logic::indeterminate(x) && boost::logic::indeterminate(y))) {
			expectationMet = true;
		}
		break;

	case Gem::Common::CE_INEQUALITY:
		if(!checkExpectation(false, caller, x, y, x_name, y_name, Gem::Common::CE_EQUALITY, limit)) {
			expectationMet = true;
		}
		break;
	};

	if(withMessages && !expectationMet) {
		std::string x_string, y_string;

		if(x==true) x_string = "true";
		else if (x==false) x_string = "false";
		else x_string = "indeterminate";

		if(y==true) y_string = "true";
		else if (y==false) y_string = "false";
		else y_string = "indeterminate";

		if(e==Gem::Common::CE_FP_SIMILARITY || e==Gem::Common::CE_EQUALITY) {
			message << "In expectation check initiated by \"" << caller << "\" : "
					<< "The two boost::logic::tribool parameters " << x_name << " " << y_name << " "
					<< "are not equal even though this was expected. "
					<< x_name << " = " << x_string << "; "
				    << y_name << " = " << y_string;
		}
		else {
			std::cerr << "In expectation check initiated by \"" << caller << "\" :" << std::endl
					  << "The two boost::logic::tribool parameters " << x_name << " " << y_name << std::endl
					  << "have the same values even though inequality was expected." << std::endl
					  << x_name << " = " << x_string << std::endl
					  << y_name << " = " << y_string << std::endl;
		}
	}

	if(expectationMet) {
		return boost::optional<std::string>();
	}
	else {
		return boost::optional<std::string>(message.str());
	}
}

/******************************************************************************/
/**
 * Helps to evaluate possible discrepancies between expectations in relationship tests
 *
 * @param caller The name of the calling entity
 * @param v A vector with the results of various discrepancy checks
 * @param e The expectation that needed to be met
 * @return A boost::optional<std::string> object optionally holding a descriptive string of discrepancies
 */
boost::optional<std::string> evaluateDiscrepancies(
		const std::string& className
		, const std::string& caller
		, const std::vector<boost::optional<std::string> >& v
		, const Gem::Common::expectation& e
){
	std::size_t nDiscrepancyChecks = v.size();
	std::size_t nDiscrepanciesFound = 0;
	std::string discrepancies = "";
	std::vector<boost::optional<std::string> >::const_iterator cit;
	bool expectationMet = true;
	boost::optional<std::string> receiver;

	switch(e) {
	case Gem::Common::CE_FP_SIMILARITY:
	case Gem::Common::CE_EQUALITY:
		for(cit=v.begin(); cit!=v.end(); ++cit) {
			receiver = *cit;
			if(receiver) { // A message is held in the boost::optional<std::string> object
				nDiscrepanciesFound++;
				discrepancies += (*receiver + "\n");
			}
		}

		// At least one check didn't meet expectations
		if(nDiscrepanciesFound > 0) {
			discrepancies = "Expectation \"equality/similarity\" was not met in class \"" + className + "\", called by \"" + caller + "\":\n" + discrepancies;
			expectationMet = false;
		}
		break;

	case Gem::Common::CE_INEQUALITY:
		for(cit=v.begin(); cit!=v.end(); ++cit) {
			receiver = *cit;
			if(receiver) { // A message is held in the boost::optional<std::string> object
				nDiscrepanciesFound++;
				discrepancies += (*receiver + "\n");
			}
		}

		// No check met the expectation
		if(nDiscrepanciesFound == nDiscrepancyChecks) {
			discrepancies = "Expectation \"inequality\" was not met in class \"" + className + "\", called by \"" + caller + "\":\n" + discrepancies;
			expectationMet = false;
		}
		break;
	};

	if(!expectationMet) { // discrepancies were found for the entire caller
		return boost::optional<std::string>(discrepancies);
	}
	else
		return boost::optional<std::string>();
}

/******************************************************************************/

} /* namespace Common */
} /* namespace Gem */
