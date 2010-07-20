/**
 * @file GPODExpectationChecksT.cpp
 */

/* Copyright (C) Dr. Ruediger Berlich and Karlsruhe Institute of Technology
 * (University of the State of Baden-Wuerttemberg and National Laboratory
 * of the Helmholtz Association)
 *
 * Contact: info [at] gemfony (dot) com
 *
 * This file is part of Geneva, Gemfony scientific's optimization library.
 *
 * This file is free software: you can redistribute it and/or modify
 * it under the terms of version 3 of the GNU Affero General Public License,
 * or, at your option, under the terms of version 2 of the GNU General Public
 * License, as published by the Free Software Foundation.
 *
 * NOTE THAT THIS FORM OF DUAL-LICENSING DOES NOT APPLY TO ANY OTHER FILES
 * OF THE GENEVA LIBRARY, UNLESS THIS IS EXPLICITLY STATED IN THE CORRESPONDING FILE.
 *
 * Geneva is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of version 2 of the GNU General Public License
 * and of version 3 of the GNU Affero General Public License along with the Geneva
 * library. If not, see <http://www.gnu.org/licenses/>.
 *
 * For further information on Gemfony scientific and Geneva, visit
 * http://www.gemfony.com .
 */

#include "common/GPODExpectationChecksT.hpp"

namespace Gem {
namespace Common {

/*************************************************************************************************/
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
)
{
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

/*************************************************************************************************/
/**
 * Helps to evaluate possible discrepancies between expectations in relationship tests
 *
 * @param caller The name of the calling entity
 * @param v A vector with the results of various discrepancy checks
 * @param e The expectation that needed to be met
 * @return A boost::optional<std::string> object optionally holding a descriptive string of discrepancies
 */
boost::optional<std::string> evaluateDiscrepancies(const std::string& className,
												   const std::string& caller,
		                                           const std::vector<boost::optional<std::string> >& v,
		                                           const Gem::Common::expectation& e)
{
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
			if(receiver = *cit) { // A message is held in the boost::optional<std::string> object
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
			if(receiver = *cit) { // A message is held in the boost::optional<std::string> object
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

/*************************************************************************************************/

} /* namespace Common */
} /* namespace Gem */
