/**
 * @file GObjectExpectationChecksT.hpp
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt)
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) com
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
 * http://www.gemfony.com .
 */


// Standard headers go here
#include <vector>
#include <sstream>
#include <iostream>
#include <string>
#include <cstdlib>
#include <cmath>
#include <typeinfo>

// Includes check for correct Boost version(s)
#include "common/GGlobalDefines.hpp"

// Boost headers go here
#include <boost/shared_ptr.hpp>
#include <boost/cast.hpp>
#include <boost/logic/tribool.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/type_traits.hpp>
#include <boost/mpl/has_xxx.hpp>
#include <boost/optional.hpp>
#include <boost/lexical_cast.hpp>

#ifndef GOBJECTEXPECTATIONCHECKST_HPP_
#define GOBJECTEXPECTATIONCHECKST_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif

// Geneva headers go here
#include "common/GPODExpectationChecksT.hpp"
#include "GObject.hpp"

namespace Gem {
namespace Common {

/*
 * The functions in this file help to check whether GObject-derivatives (and collections thereof)
 * meet a given set of expectations.
 */

/******************************************************************************/
/**
 * This function checks whether two smart pointers to complex types meet a given expectation.
 * It is assumed that these types have the standard Geneva interface with corresponding "checkRelationshipWith"
 * functions.
 *
 * @param withMessages Specifies whether messages should be emitted in case of failed expectations
 * @param caller The name of the calling class
 * @param x The first smart pointer to be compared
 * @param y The second smart pointer to be compared
 * @param x_name The name of the first pointer
 * @param y_name The name of the second pointer
 * @param e The expectation both pointers need to fulfill
 * @param limit The maximum allowed deviation of two floating point values
 * @param dummy Boost::enable_if magic to steer overloaded resolution by the compiler
 * @return A boost::optional<std::string> that optionally contains a description of discrepancies found from the expected outcome
 */

template <typename geneva_type>
boost::optional<std::string> checkExpectation (
		const bool& withMessages
	  , const std::string& caller
	  , const boost::shared_ptr<geneva_type>& x
	  , const boost::shared_ptr<geneva_type>& y
	  , const std::string& x_name
	  , const std::string& y_name
	  , const Gem::Common::expectation& e
	  , const double& limit = 0
	  , typename boost::enable_if<boost::is_base_of<Gem::Geneva::GObject, geneva_type> >::type* dummy = 0
)
{
	bool expectationMet = false;
	std::ostringstream message;

	std::string myCaller = "[Gem::Common::checkExpectation(), called by " + caller + "]";

	switch(e) {
	case Gem::Common::CE_FP_SIMILARITY:
	case Gem::Common::CE_EQUALITY:
		// Check whether the pointers hold content
		if(x && !y) {
			if(withMessages) {
				message << "In expectation check initiated by \"" << caller << "\" : "
						<< "Smart pointer " << x_name << " holds content while " << y_name << " does not.";
			}
			break; //
		}
		else if(!x && y) {
			if(withMessages) {
				message << "In expectation check initiated by \"" << caller << "\" : "
						<< "Smart pointer " << x_name << " doesn't hold content while " << y_name << " does.";
			}
			break;  // The expectation was clearly not met
		}
		else if(!x && !y) { // No content to check. Both smart pointers can be considered equal
			expectationMet = true;
			break;
		}

		// Check whether the content differs
		{
			boost::optional<std::string> o;
			o = x->checkRelationshipWith(*y, e, limit, myCaller, y_name, withMessages);
			if(o) {
				if(withMessages) {
					message <<  "In expectation check initiated by \"" << caller << "\" : Smart pointers "
							<< x_name << " and " << y_name << " differ. Analysis:\n"
							<< *o;
				}
				break;
			}
			else {
				expectationMet = true;
				break;
			}
		}

		break;

	case Gem::Common::CE_INEQUALITY:
		// Check whether the pointers hold content
		if((x && !y) || (!x && y)) {
			expectationMet = true;
			break;
		}
		else if(!x && !y) { // No content to check. Both smart pointers can be considered equal
			break; // The expectation was not met
		}

		// Check whether the content differs
		{
			boost::optional<std::string> o;
			o = x->checkRelationshipWith(*y, e, limit, myCaller, y_name, withMessages);
			if(o) {
				if(withMessages) {
					message <<  "In expectation check initiated by \"" << caller << "\" : Smart pointers"
							<< x_name << " and " << y_name << " do not differ. Analysis:\n"
							<< *o;
				}
				break;
			}
			else {
				expectationMet = true;
				break;
			}
		}

		break;
	};

	if(expectationMet) {
		return boost::optional<std::string>();
	}
	else {
		return boost::optional<std::string>(message.str());
	}
}

/******************************************************************************/
/**
 * This function checks whether two vectors of smart pointers to complex types meet a given expectation.
 * It is assumed that these types have the standard Geneva interface with corresponding "checkRelationshipWith"
 * functions.
 *
 * @param withMessages Specifies whether messages should be emitted in case of failed expectations
 * @param caller The name of the calling class
 * @param x The first vector to be compared
 * @param y The second vector to be compared
 * @param x_name The name of the first vector
 * @param y_name The name of the second vector
 * @param e The expectation both vectors need to fulfill
 * @param limit The maximum allowed deviation of two floating point values
 * @param dummy Boost::enable_if magic to steer overloaded resolution by the compiler
 * @return A boost::optional<std::string> that optionally contains a description of discrepancies found from the expected outcome
 */

template <typename geneva_type>
boost::optional<std::string> checkExpectation (
        const bool& withMessages
	  , const std::string& caller
	  , const std::vector<boost::shared_ptr<geneva_type> >& x
	  , const std::vector<boost::shared_ptr<geneva_type> >& y
	  , const std::string& x_name
	  , const std::string& y_name
	  , const Gem::Common::expectation& e
	  , const double& limit = 0
	  , typename boost::enable_if<boost::is_base_of<Gem::Geneva::GObject, geneva_type> >::type* dummy = 0
)
{
	bool expectationMet = false;
	std::ostringstream message;

	std::string myCaller = "[Gem::Common::checkExpectation(), called by " + caller + "]";

	bool foundDeviation = false;
	typename std::vector<boost::shared_ptr<geneva_type> >::const_iterator x_it, y_it;

	switch(e) {
	case Gem::Common::CE_FP_SIMILARITY:
	case Gem::Common::CE_EQUALITY:
		// Check whether all items are equal or similar
		{
			std::size_t failedIndex = 0;

			// Check sizes
			if(x.size() != y.size()) {
				if(withMessages) {
					message << "In expectation check initiated by \"" << myCaller << "\" : "
							<< "The two vectors " << x_name << " and " << y_name << " have different sizes "
							<< "even though equality or similarity was expected. "
							<< "Sizes are : "
							<< x_name << ".size() = " << x.size() << "; "
							<< y_name << ".size() = " << y.size();
				}
				break; // Expectation has not been met
			}

			// Check individual entries
			boost::optional<std::string> o;
			std::size_t index = 0;
			for(x_it=x.begin(), y_it=y.begin(); x_it!=x.end(); ++x_it, ++y_it, ++index) {
				std::string first_name = "x[" + boost::lexical_cast<std::string>(index) + "]";
				std::string second_name = "y[" + boost::lexical_cast<std::string>(index) + "]";

				o = checkExpectation(withMessages, myCaller, *x_it, *y_it, first_name, second_name, e, limit);
				if(o) {
					foundDeviation = true;
					failedIndex = index;
					break; // Leave the for-loop, one deviation is enough
				}
			}

			if(!foundDeviation) expectationMet = true;
			else {
				if(withMessages) {
					message << "In expectation check initiated by \"" << myCaller << "\" : "
							<< "The two vectors " << x_name << " and " << y_name << " have deviations "
							<< "even though equality or similarity was expected. "
							<< "First deviating entry is at index " << failedIndex << ". Further analysis "
							<< "of the first deviation:\n"
							<< *o;
				}
			}
		}
		break;

	case Gem::Common::CE_INEQUALITY:
		// Check whether at least one item is inequal
		{
			// Check sizes
			if(x.size() != y.size()) {
				expectationMet = true;
				break;
			}

			// Check individual entries
			std::size_t nDeviationsFound = 0; // deviations from expectation "inequality"
			std::size_t index = 0;
			for(x_it=x.begin(), y_it=y.begin(); x_it!=x.end(); ++x_it, ++y_it, ++index) {
				std::string first_name = "x[" + boost::lexical_cast<std::string>(index) + "]";
				std::string second_name = "y[" + boost::lexical_cast<std::string>(index) + "]";

				if(checkExpectation(withMessages, myCaller, *x_it, *y_it, first_name, second_name, e, limit)) nDeviationsFound++;
			}

			if(nDeviationsFound == x.size()) {
				if(withMessages) {
					message << "In expectation check initiated by \"" << caller << "\" : "
							<< "The two vectors " << x_name << " and " << y_name << " are equal "
							<< "even though inequality was expected.";
				}
			}
			else { // Not all entries were equal
				expectationMet = true;
			}
		}
		break;
	};

	if(expectationMet) {
		return boost::optional<std::string>();
	}
	else {
		return boost::optional<std::string>(message.str());
	}
}

/******************************************************************************/

} /* namespace Common */
} /* namespace Gem */

#endif /* GOBJECTEXPECTATIONCHECKST_HPP_ */
