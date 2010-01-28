/**
 * @file GPODExpectationChecksT.hpp
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

// Standard headers go here
#include <vector>
#include <sstream>
#include <iostream>
#include <string>
#include <cstdlib>
#include <cmath>
#include <typeinfo>

// Includes check for correct Boost version(s)
#include "GGlobalDefines.hpp"

// Boost headers go here
#include <boost/shared_ptr.hpp>
#include <boost/cast.hpp>
#include <boost/logic/tribool.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/type_traits.hpp>
#include <boost/mpl/has_xxx.hpp>
#include <boost/optional.hpp>
#include <boost/lexical_cast.hpp>

#ifndef GPODEXPECTATIONCHECKST_HPP_
#define GPODEXPECTATIONCHECKST_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif

// Our own headers go here
#include "GEnums.hpp"

namespace Gem {
namespace Util {
namespace POD {

/*
 * The functions in this file help to check whether "Plain Old Data" components of Geneva classes
 * meet a given set of expections, such as equality, inequality or floating point similarity.
 *
 * For the sake of simplicity, functions for some non-POD types (such as
 * std::vector<boost::shared_ptr<someGenevaClass> > exist here as well.
 *
 * A separate namespace POD has been chosen in order to better distinguish these functions from the
 * similarly named member functions in the Geneva classes.
 */

/*************************************************************************************************/
/**
 * This function checks whether two "basic" types meet a given expectation. It assumes that x and y
 * understand the == and != operators. If x and y do not fulfill this requirement, you need to provide
 * a specialization of this function. A check for similarity is treated the same as a check for equality.
 * A specialization of this function is provided for floating point values. If "withMessages" is set to
 * true, a descriptive string will be returned in case of deviations from the expected outcome.
 *
 * @param withMessages Specifies whether messages should be emitted in case of failed expectations
 * @param caller The name of the calling class
 * @param x The first parameter to be compared
 * @param y The second parameter to be compared
 * @param x_name The name of the first parameter
 * @param y_name The name of the second parameter
 * @param e The expectation both parameters need to fulfill
 * @param limit The maximum allowed deviation of two floating point values
 * @param dummy Boost::enable_if magic to steer overloaded resolution by the compiler
 * @return A boost::optional<std::string> that optionally contains a description of discrepancies found from the expected outcome
 */
template <typename basic_type>
boost::optional<std::string> checkExpectation (
	    const bool& withMessages
      , const std::string& caller
	  , const basic_type& x
	  , const basic_type& y
	  , const std::string& x_name
	  , const std::string& y_name
	  , const Gem::Util::expectation& e
	  , const double& limit = 0.
	  , typename boost::disable_if<boost::is_floating_point<basic_type> >::type* dummy = 0
)
{
	bool expectationMet = false;
	std::ostringstream message;

	switch(e) {
	case Gem::Util::CE_FP_SIMILARITY:
	case Gem::Util::CE_EQUALITY:
		if(x==y) expectationMet = true;
		break;
	case Gem::Util::CE_INEQUALITY:
		if(x!=y) expectationMet = true;
		break;
	};

	if(withMessages && !expectationMet) {
		message << "In expectation check initiated by \"" << caller << "\" : "
				<< x_name << " and " << y_name << " where not " <<
				((e==Gem::Util::CE_EQUALITY || e==Gem::Util::CE_FP_SIMILARITY)?"equal/similar":"inequal")
				<< " as expected.";
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
 * This function checks whether two vectors of "basic" types meet a given expectation. It assumes that
 * these types understand the == and != operators. If they do not fulfill this requirement, you need to provide
 * a specialization of this function. A check for similarity is treated the same as a check for equality.
 * A  specialization of this function is provided for floating point values.
 *
 * @param withMessages Specifies whether messages should be emitted in case of failed expectations
 * @param caller The name of the calling class
 * @param x The first vector to be compared
 * @param y The second vector to be compared
 * @param x_name The name of the first parameter
 * @param y_name The name of the second parameter
 * @param e The expectation both parameters need to fulfill
 * @param limit The maximum allowed deviation of two floating point values
 * @param dummy Boost::enable_if magic to steer overloaded resolution by the compiler
 * @return A boost::optional<std::string> that optionally contains a description of discrepancies found from the expected outcome
 */
template <typename basic_type>
boost::optional<std::string> checkExpectation (
        const bool& withMessages
	  , const std::string& caller
	  , const std::vector<basic_type>& x
	  , const std::vector<basic_type>& y
	  , const std::string& x_name
	  , const std::string& y_name
	  , const Gem::Util::expectation& e
	  , const double& limit = 0.
	  , typename boost::disable_if<boost::is_floating_point<basic_type> >::type* dummy = 0
)
{
	bool expectationMet = false;
	std::ostringstream message;

	switch(e) {
	case Gem::Util::CE_FP_SIMILARITY:
	case Gem::Util::CE_EQUALITY:
		if(x==y) expectationMet = true;
		break;
	case Gem::Util::CE_INEQUALITY:
		if(x!=y) expectationMet = true;
		break;
	};

	if(withMessages && !expectationMet) {
		switch(e) {
		// Similarity and equality are treated the same for non-floating point types
		case Gem::Util::CE_FP_SIMILARITY:
		case Gem::Util::CE_EQUALITY:
			{
				message << "In expectation check initiated by \"" << caller << "\" : "
						<< "The two vectors " << x_name << " and " << y_name << " differ "
						<< "while equality was expected. Further analysis: ";

				std::size_t x_size = x.size(), y_size = y.size();
				if(x_size != y_size) {
					message << "Different vector-sizes found : "
							<< x_name << ".size() = " << x_size << "; "
							<< y_name << ".size() = " << y_size;
				}
				else {
					// Find out about the first entry that differs
					typename std::vector<basic_type>::const_iterator x_it, y_it;
					std::size_t failedIndex = 0;
					for(x_it=x.begin(), y_it=y.begin(); x_it!=x.end(); ++x_it, ++y_it, ++failedIndex) {
						if(*x_it != *y_it) {
							message << "Found inequality at index " << failedIndex << ": "
									<< x_name << "[" << failedIndex << "] = " << *x_it << "; "
									<< y_name << "[" << failedIndex << "] = " << *y_it;
							break;
						}
					}
				}
			}
			break;

		case Gem::Util::CE_INEQUALITY:
			message << "In expectation check initiated by \"" << caller << "\" : "
			        << "The two vectors " << x_name << " and " << y_name << " are equal "
			        << "even though differences were expected";
			break;
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
 * This function checks whether two floating point types meet a given expectation.
 *
 * @param withMessages Specifies whether messages should be emitted in case of failed expectations
 * @param caller The name of the calling class
 * @param x The first parameter to be compared
 * @param y The second parameter to be compared
 * @param x_name The name of the first parameter
 * @param y_name The name of the second parameter
 * @param e The expectation both parameters need to fulfill
 * @param limit The maximum allowed deviation of two floating point values
 * @param dummy Boost::enable_if magic to steer overloaded resolution by the compiler
 * @return A boost::optional<std::string> that optionally contains a description of discrepancies found from the expected outcome
 */
template <typename fp_type>
boost::optional<std::string> checkExpectation (
        const bool& withMessages
	  , const std::string& caller
	  , const fp_type& x
	  , const fp_type& y
	  , const std::string& x_name
	  , const std::string& y_name
	  , const Gem::Util::expectation& e
	  , const double& limit = exp(-10)
	  , typename boost::enable_if<boost::is_floating_point<fp_type> >::type* dummy = 0
)
{
	bool expectationMet = false;
	std::ostringstream message;

	switch(e) {
	case Gem::Util::CE_FP_SIMILARITY:
		if(fabs(x-y) < boost::numeric_cast<fp_type>(limit)) expectationMet = true;
		break;
	case Gem::Util::CE_EQUALITY:
		if(x==y) expectationMet = true;
		break;
	case Gem::Util::CE_INEQUALITY:
		if(x!=y) expectationMet = true;
		break;
	};

	if(withMessages && !expectationMet) {
		std::streamsize originalPrecision = std::cerr.precision ( );
		std::cerr.precision(15);

		switch(e) {
		case Gem::Util::CE_FP_SIMILARITY:
			message << "In expectation check initiated by \"" << caller << "\" : "
			        << "floating point values " << x_name << " and " << y_name << " where not similar as expected. "
			        << "x = " << x << "; "
			        << "y = " << y << "; "
			        << "limit = " << boost::numeric_cast<fp_type>(limit) << "; "
			        << "deviation = " << fabs(x-y);
			break;

		case Gem::Util::CE_EQUALITY:
			message << "In expectation check initiated by \"" << caller << "\" : "
					<< "floating point values " << x_name << " and " << y_name << " where not equal as expected. "
					<< "x = " << x << "; "
					<< "y = " << y << "; "
					<< "limit = " << boost::numeric_cast<fp_type>(limit) << "; "
					<< "deviation = " << fabs(x-y);
			break;

		case Gem::Util::CE_INEQUALITY:
			message << "In expectation check initiated by \"" << caller << "\" : "
					<< "floating point values " << x_name << " and " << y_name << " where equal contrary to expectation. "
					<< "x = " << x << "; "
					<< "y = " << y;
			break;
		};

		std::cerr.precision(originalPrecision);
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
 * This function checks whether two vectors of floating point types meet a given expectation.
 *
 * @param withMessages Specifies whether messages should be emitted in case of failed expectations
 * @param caller The name of the calling class
 * @param x The first vector to be compared
 * @param y The second vector to be compared
 * @param x_name The name of the first parameter
 * @param y_name The name of the second parameter
 * @param e The expectation both parameters need to fulfill
 * @param limit The maximum allowed deviation of two floating point values
 * @param dummy Boost::enable_if magic to steer overloaded resolution by the compiler
 * @return A boost::optional<std::string> that optionally contains a description of discrepancies found from the expected outcome
 */
template <typename fp_type>
boost::optional<std::string> checkExpectation (
        const bool& withMessages
	  , const std::string& caller
	  , const std::vector<fp_type>& x
	  , const std::vector<fp_type>& y
	  , const std::string& x_name
	  , const std::string& y_name
	  , const Gem::Util::expectation& e
	  , const double& limit = exp(-10)
	  , typename boost::enable_if<boost::is_floating_point<fp_type> >::type* dummy = 0
)
{
	bool expectationMet = false;
	std::ostringstream message;

	switch(e) {
	case Gem::Util::CE_FP_SIMILARITY:
	    {
	    	// Leave switch statement if sizes differ
	    	if(x.size() != y.size()) break;

	    	// Loop over all entries
			typename std::vector<fp_type>::const_iterator x_it, y_it;
			bool foundDeviation = false;

			for(x_it=x.begin(), y_it=y.begin(); x_it!=x.end(); ++x_it, ++y_it) {
				if(fabs(*x_it - *y_it) >= boost::numeric_cast<fp_type>(limit)) {
					foundDeviation = true;
					break;
				}
			}

			if(!foundDeviation) expectationMet = true;
	    }
		break;

	case Gem::Util::CE_EQUALITY:
		if(x==y) expectationMet = true;
		break;

	case Gem::Util::CE_INEQUALITY:
		if(x!=y) expectationMet = true;
		break;
	};

	if(withMessages && !expectationMet) {
		std::streamsize originalPrecision = std::cerr.precision ( );
		std::cerr.precision(15);

		switch(e) {
		case Gem::Util::CE_FP_SIMILARITY:
			{
				message << "In expectation check initiated by \"" << caller << "\" : "
						 << "The two vector<fp_type> objects " << x_name << " and " << y_name << " show deviations "
						 << "while similarity was expected. Further analysis: ";

				std::size_t x_size = x.size(), y_size = y.size();
				if(x_size != y_size) {
					message << "Different vector-sizes found : "
							 << x_name << ".size() = " << x_size << "; "
							 << y_name << ".size() = " << y_size;
				}
				else {
					// Find out about the first entry that differs
					typename std::vector<fp_type>::const_iterator x_it, y_it;
					std::size_t failedIndex = 0;
					for(x_it=x.begin(), y_it=y.begin(); x_it!=x.end(); ++x_it, ++y_it, ++failedIndex) {
						if(fabs(*x_it - *y_it) >= boost::numeric_cast<fp_type>(limit)) {
							message << "Found deviation at index " << failedIndex << ": "
									 << x_name << "[" << failedIndex << "] = " << *x_it << "; "
									 << y_name << "[" << failedIndex << "] = " << *y_it << "; "
									 << "limit = " << boost::numeric_cast<fp_type>(limit) << "; "
									 << "deviation = " << fabs(*x_it - *y_it);
							break;
						}
					}
				}
			}
			break;

		case Gem::Util::CE_EQUALITY:
			{
				message << "In expectation check initiated by \"" << caller << "\" : "
						 << "The two vector<fp_type> objects " << x_name << " and " << y_name << " differ "
						 << "while equality was expected. Further analysis: ";

				std::size_t x_size = x.size(), y_size = y.size();
				if(x_size != y_size) {
					message << "Different vector-sizes found : "
							 << x_name << ".size() = " << x_size << "; "
							 << y_name << ".size() = " << y_size;
				}
				else {
					// Find out about the first entry that differs
					typename std::vector<fp_type>::const_iterator x_it, y_it;
					std::size_t failedIndex = 0;
					for(x_it=x.begin(), y_it=y.begin(); x_it!=x.end(); ++x_it, ++y_it, ++failedIndex) {
						if(*x_it != *y_it) {
							message << "Found inequality at index " << failedIndex << ": "
									 << x_name << "[" << failedIndex << "] = " << *x_it << "; "
									 << y_name << "[" << failedIndex << "] = " << *y_it;
							break;
						}
					}
				}
			}
			break;

		case Gem::Util::CE_INEQUALITY:
			message << "In expectation check initiated by \"" << caller << "\" : "
				     << "The two vectors " << x_name << " and " << y_name << " are equal "
				     << "even though differences were expected";
			break;
		}

		std::cerr.precision(originalPrecision);
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
 * Define a check whether we are dealing with an object which has a given function.
 * See "Beyond the C++ Standard Library" by Bjoern Karlsson, p. 99 for an explanation
 */
BOOST_MPL_HAS_XXX_TRAIT_DEF(checkRelationshipWithFunction)

/*************************************************************************************************/
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
	  , const Gem::Util::expectation& e
	  , const double& limit = 0
	  , typename boost::enable_if<has_checkRelationshipWithFunction<geneva_type> >::type* dummy = 0
)
{
	bool expectationMet = false;
	std::ostringstream message;

	std::string myCaller = "[Gem::GenEvA::checkExpectation(), called by " + caller + "]";

	switch(e) {
	case Gem::Util::CE_FP_SIMILARITY:
	case Gem::Util::CE_EQUALITY:
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
			if(o = x->checkRelationshipWith(*y, e, limit, myCaller, y_name, withMessages)) {
				if(withMessages) {
					message <<  "In expectation check initiated by \"" << caller << "\" : Smart pointers"
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

	case Gem::Util::CE_INEQUALITY:
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
			if(o = x->checkRelationshipWith(*y, e, limit, myCaller, y_name, withMessages)) {
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

/*************************************************************************************************/
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
	  , const Gem::Util::expectation& e
	  , const double& limit = 0
	  , typename boost::enable_if<has_checkRelationshipWithFunction<geneva_type> >::type* dummy = 0
)
{
	bool expectationMet = false;
	std::ostringstream message;

	std::string myCaller = "[Gem::GenEvA::checkExpectation(), called by " + caller + "]";

	bool foundDeviation = false;
	typename std::vector<boost::shared_ptr<geneva_type> >::const_iterator x_it, y_it;

	switch(e) {
	case Gem::Util::CE_FP_SIMILARITY:
	case Gem::Util::CE_EQUALITY:
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

				if(o = checkExpectation(withMessages, myCaller, *x_it, *y_it, first_name, second_name, e, limit)) {
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

	case Gem::Util::CE_INEQUALITY:
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

/*************************************************************************************************/
/**
 * @brief This function checks whether two objects of type boost::logic::tribool meet a given expectation.
 */
boost::optional<std::string> checkExpectation (
        const bool& withMessages
	  , const std::string& caller
	  , const boost::logic::tribool& x
	  , const boost::logic::tribool& y
	  , const std::string& x_name
	  , const std::string& y_name
	  , const Gem::Util::expectation& e
	  , const double& limit = 0
);

/*************************************************************************************************/
/**
 * Helps to evaluate possible discrepancies between expectations in relationship tests
 */
boost::optional<std::string> evaluateDiscrepancies(
		const std::string&,
		const std::string&,
		const std::vector<boost::optional<std::string> >&,
		const Gem::Util::expectation&
);

/*************************************************************************************************/

} /* namespace POD */
} /* namespace Util */
} /* namespace Gem */

#endif /* GPODEXPECTATIONCHECKST_HPP_ */
