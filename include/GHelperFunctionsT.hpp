/**
 * @file GHelperFunctionsT.hpp
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
#include <map>
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
#include <boost/logic/tribool.hpp>
#include <boost/date_time.hpp>

#ifndef GHELPERFUNCTIONST_HPP_
#define GHELPERFUNCTIONST_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif

// Our own headers go here

namespace Gem
{
namespace Util
{


/**************************************************************************************************/
/**
 * This function takes two vectors of boost::shared_ptr smart pointers and copies
 * one into the other. As we want to make a deep copy of the smart pointers' contents
 * this can be quite complicated. Note that we assume here that the objects pointed to
 * can be copied using an operator=(). The function also assumes the existence of
 * a valid copy constructor.
 *
 * @param from The vector used as the source of the copying
 * @param to The vector used as the target of thec copying
 */
template <typename T>
void copySmartPointerVector(const std::vector<boost::shared_ptr<T> >& from,
		                            std::vector<boost::shared_ptr<T> >& to) {
	typename std::vector<boost::shared_ptr<T> >::const_iterator it_from;
	typename std::vector<boost::shared_ptr<T> >::iterator it_to;

	std::size_t size_from = from.size();
	std::size_t size_to = to.size();

	if(size_from==size_to) { // The most likely case
		for(it_from=from.begin(), it_to=to.begin();
		     it_from!=from.end(), it_to!=to.end();
		     ++it_from, ++it_to) {
			**it_to=**it_from; // Uses T::operator=()
		}
	}
	else if(size_from > size_to) {
		// First copy the data of the first size_to items
		for(it_from=from.begin(), it_to=to.begin();
		     it_to!=to.end(); ++it_from, ++it_to) {
			**it_to=**it_from;
		}

		// Then attach copies of the remaining items
		for(it_from=from.begin()+size_to; it_from!=from.end(); ++it_from) {
			boost::shared_ptr<T> p(new T(**it_from));
			to.push_back(p);
		}
	}
	else if(size_from < size_to) {
		// First copy the initial size_foreight items over
		for(it_from=from.begin(), it_to=to.begin();
		     it_from!=from.end(); ++it_from, ++it_to) {
			**it_to=**it_from;
		}

		// Then resize the local vector. Surplus items will vanish
		to.resize(size_from);
	}
}

/**************************************************************************************************/
/**
 * Checks for inequality of the two arguments, which are assumed to be basic types. This is
 * needed by the isEqualTo function, so we have a standardized way of emitting information
 * on deviations. Note: If you want specific behavior for a particular type then you can always
 * create a specialization of this function.
 *
 * expected == true means: An inequality is expected, emit a message if nevertheless an equality was found
 * expected == boost::logic::indeterminate means: do not emit any messages
 * expected == false means: Equality is expected, emit a message, if nevertheless an inequality was found
 *
 * @param className The name of the calling class
 * @param x The first parameter to be compared
 * @param y The second parameter to be compared
 * @param x_name The name of the first parameter
 * @param y_name The name of the second parameter
 * @param expected Indicates whether we expect the condition to be true or false, or whether any reporting should be suppressed ("ignore")
 * @return A boolean indicating whether any differences were found
 */
template <typename basic_type>
bool checkForInequality(const std::string& className,
									          const basic_type& x,
									          const basic_type& y,
									          const std::string& x_name,
									          const std::string& y_name,
									          const boost::logic::tribool& expected = boost::logic::indeterminate)
{
	bool raiseAlarm = false;
	bool result = false;
	std::ostringstream error;

	if(x==y) {
		if(expected == true) {
			raiseAlarm = true; // expected the result to be "inequal"

			error << "//-----------------------------------------------------------------" << std::endl
				      << "Found the following value(s) in inequality check of object of type \"" << className << "\":" << std::endl
				      << x_name << " (type " << typeid(x).name() << ") = " << x << std::endl
				      << y_name << " (type " << typeid(y).name() << ") = " << y << std::endl
				      << "when inequality was expected" << std::endl;
		}

		result = false;
	}
	else {
		if(expected==false) {
			raiseAlarm = true; // expected the result to be "equal"

			error << "//-----------------------------------------------------------------" << std::endl
				      << "Found the following value(s) in inequality check of object of type \"" << className << "\":" << std::endl
				      << x_name << " (type " << typeid(x).name() << ") = " << x << std::endl
				      << y_name << " (type " << typeid(y).name() << ") = " << y << std::endl
				      << "when equality was expected" << std::endl;
		}
		result = true;
	}

	if(raiseAlarm) {
		std::cerr << error.str();
	}

	return result;
}

/**************************************************************************************************/
/**
 * Checks for inequality of the two arguments, which are assumed to be vectors of basic types.
 * This is needed by the isEqualTo function, so we have a standardized way of emitting information
 * on deviations. Note: If you want specific behavior for a particular type then you can always
 * create a specialization of this function.
 *
 * expected == true means: An inequality is expected, emit a message if nevertheless an equality was found
 * expected == boost::logic::indeterminate means: do not emit any messages
 * expected == false means: No inequality is expected, emit a message, if nevertheless an inequality was found
 *
 * @param className The name of the calling class
 * @param x The first parameter to be compared
 * @param y The second parameter to be compared
 * @param x_name The name of the first parameter
 * @param y_name The name of the second parameter
 * @param expected Indicates whether we expect the condition to be true or false, or whether any reporting should be suppressed ("ignore")
 * @return A boolean indicating whether any differences were found
 */
template <typename basic_type>
bool checkForInequality(const std::string& className,
									           const std::vector<basic_type>& x,
									           const std::vector<basic_type>& y,
									           const std::string& x_name,
									           const std::string& y_name,
									           const boost::logic::tribool& expected = boost::logic::indeterminate)
{
	bool raiseAlarm = false;
	bool result = false;
	std::ostringstream error;

	if(x==y) {
		if(expected == true) {  // expected the result to be "inequal"
			raiseAlarm = true;

			error << "//-----------------------------------------------------------------" << std::endl
				     << "Found equality  in object of type \"" << className << "\" with" << std::endl
			         << x_name << " (type std::vector<" << typeid(basic_type).name() <<">): Size = " << x.size() << std::endl
			         << y_name << " (type std::vector<" << typeid(basic_type).name() <<">): Size = " << y.size() << std::endl
			         << "when inequality was expected" << std::endl;
		}

		result = false;
	}
	else {
		if(expected==false && !boost::logic::indeterminate(expected)) {
			raiseAlarm=true;

			if(x.size() != y.size()) {
				error << "//-----------------------------------------------------------------" << std::endl
				          << "Found inequality in object of type \"" << className << "\":" << std::endl
					      << x_name << " (type std::vector<" << typeid(basic_type).name() <<">): Size = " << x.size() << std::endl
					      << y_name << " (type std::vector<" << typeid(basic_type).name() <<">): Size = " << y.size() << std::endl
					      << "when equality was expected" << std::endl;
			}
			else { // something happened with one of the data entries
				// Loop over all data items
				for(std::size_t i=0; i<x.size(); i++) {
					if(x.at(i) != y.at(i)) {
						error << "//-----------------------------------------------------------------" << std::endl
							      << "Found inequality in object of type \"" << className << "\":" << std::endl
							      << x_name << "[" << i << "] (type std::vector<" << typeid(basic_type).name() <<">) " << x.at(i) << std::endl
							      << y_name << "[" << i << "] (type std::vector<" << typeid(basic_type).name() <<">) " << y.at(i) << std::endl
							      << "when equality was expected. Checks do not proceed after this index." << std::endl;

						break; // Leave the loop
					}
				}
			}
		}

		result=true;
	}

	if(raiseAlarm) {
		std::cerr << error.str();
	}

	return result;
}

/**************************************************************************************************/
/**
 * Checks for inequality of the two arguments, which are assumed to be vectors of boost::share_ptr
 * objects of complex. This is needed by the isEqualTo function, so we have a standardized way of
 * emitting information on deviations. Note: If you want specific behavior for a particular type then you
 * can always create a specialization of this function.
 * expected == true means: An inequality is expected, emit a message if nevertheless an equality was found
 * expected == boost::logic::indeterminate means: do not emit any messages
 * expected == false means: No inequality is expected, emit a message, if nevertheless an inequality was found
 *
 * @param className The name of the calling class
 * @param x The first parameter to be compared
 * @param y The second parameter to be compared
 * @param x_name The name of the first parameter
 * @param y_name The name of the second parameter
 * @param expected Indicates whether we expect the condition to be true or false, or whether any reporting should be suppressed ("ignore")
 * @return A boolean indicating whether any differences were found
 */
template <typename complex_type>
bool checkForInequality(const std::string& className,
									          const std::vector<boost::shared_ptr<complex_type> >& x,
									          const std::vector<boost::shared_ptr<complex_type> >& y,
									          const std::string& x_name,
									          const std::string& y_name,
									          const boost::logic::tribool& expected = boost::logic::indeterminate)
{
	bool raiseAlarm = false;
	bool result = false;
	std::ostringstream error;

	// Are there any inequalities in the vector's sizes ?
	if(x.size() != y.size()) {
		if(expected == false) {
			raiseAlarm = true;

			error << "//-----------------------------------------------------------------" << std::endl
			          << "Found inequality in object of type \"" << className << "\":" << std::endl
					  << x_name << " (type std::vector<boost::shared_ptr<" << typeid(complex_type).name() <<"> >): Size = " << x.size() << std::endl
					  << y_name << " (type std::vector<boost::shared_ptr<" << typeid(complex_type).name() <<"> >): Size = " << y.size() << std::endl
				      << "when equality was expected" << std::endl;
		}

		result = true;
	}
	else { // O.k., can we at least find inequalities in the data sets ?
		// Loop over all entries and find out which is wrong
		typename std::vector<boost::shared_ptr<complex_type> >::const_iterator cit_x;
		typename std::vector<boost::shared_ptr<complex_type> >::const_iterator cit_y;
	    std::size_t i=0;
		for(cit_x=x.begin(), cit_y=y.begin(); cit_x != x.end(), cit_y != y.end(); ++cit_x, ++cit_y) {
			if((*cit_x)->isNotEqualTo(**cit_y, expected)) {
				if(expected == false) {
					raiseAlarm = true;

					error << "//-----------------------------------------------------------------" << std::endl
							 << "Found inequality in object of type \"" << className << "\":" << std::endl
							 << x_name << "[" << i << "] (type std::vector<boost::shared_ptr<" << typeid(complex_type).name() <<"> >) " << std::endl
							 << y_name << "[" << i << "] (type std::vector<boost:.shared_ptr<" << typeid(complex_type).name() <<"> >) " << std::endl
							 << "when equality was expected. Checks do not proceed after this index." << std::endl;
				}

				result = true;
				break;
			}

			i++;
		}
	}

	// Is the result still false ? Then no inequality has been found. Did we expect an inequality ?
	if(result == false && expected==true) {
		raiseAlarm = true;

		error << "//-----------------------------------------------------------------" << std::endl
			     << "Found equality  in object of type \"" << className << "\" with" << std::endl
				 << x_name << " (type std::vector<boost::shared_ptr<" << typeid(complex_type).name() <<"> >): Size = " << x.size() << std::endl
				 << y_name << " (type std::vector<boost::shared_ptr<" << typeid(complex_type).name() <<"> >): Size = " << y.size() << std::endl
		         << "when inequality was expected" << std::endl;
	}

	if(raiseAlarm) {
		std::cerr << error.str();
	}

	return result;
}

/**************************************************************************************************/
/**
 * Checks for similarity of the two arguments, which are assumed to be basic types. This is
 * needed by the isSimilarTo function, so we have a standardized way of emitting information on
 * deviations.  By default all types are just checked for equality. A specialization exists for
 * typeof(basic_type) == typeof(double). Note that: If you want specific behaviour for any other
 * type then you can always create a specialization of this function.
 *
 * expected == true means: An inequality is expected, emit a message if nevertheless an equality was found
 * expected == boost::logic::indeterminate means: do not emit any messages
 * expected == false means: No inequality is expected, emit a message, if nevertheless an inequality was found
 *
 * @param className The name of the calling class
 * @param x The first parameter to be compared
 * @param y The second parameter to be compared
 * @param limit The acceptable deviation of x and y
 * @param x_name The name of the first parameter
 * @param y_name The name of the second parameter
 * @param expected Indicates whether we expect the condition to be true or false, or whether any reporting should be suppressed ("ignore")
 * @return A boolean indicating whether any differences were found
 */
template <typename basic_type>
bool checkForDissimilarity(const std::string& className,
												  const basic_type& x,
												  const basic_type& y,
												  const double& limit,
												  const std::string& x_name,
												  const std::string& y_name,
												  const boost::logic::tribool& expected = boost::logic::indeterminate)
{
	bool raiseAlarm = false;
	bool result = false;
	std::ostringstream error;

	if(x==y) {
		if(expected == true) {
			raiseAlarm = true; // expected the result to be "inequal"

			error << "//-----------------------------------------------------------------" << std::endl
				      << "Found the following value(s) in dissimilarity check of object of type \"" << className << "\":" << std::endl
				      << x_name << " (type " << typeid(x).name() << ") = " << x << std::endl
				      << y_name << " (type " << typeid(y).name() << ") = " << y << std::endl
				      << "when dissimilarity was expected" << std::endl;
		}

		result = false;
	}
	else {
		if(expected==false) {
			raiseAlarm = true; // expected the result to be "equal"

			error << "//-----------------------------------------------------------------" << std::endl
				      << "Found the following value(s) in dissimilarity check of object of type \"" << className << "\":" << std::endl
				      << x_name << " (type " << typeid(x).name() << ") = " << x << std::endl
				      << y_name << " (type " << typeid(y).name() << ") = " << y << std::endl
				      << "when no dissimilarity was expected" << std::endl;
		}
		result = true;
	}

	if(raiseAlarm) {
		std::cerr << error.str();
	}

	return result;
}

/**************************************************************************************************/
/**
 * Checks for dissimilarity of the two arguments, which are assumed to be vectors of basic types.
 * This is needed by the isSimilarTo function, so we have a standardized way of emitting information
 * on deviations. Note: If you want specific behavior for a particular type then you can always
 * create a specialization of this function. One such specialization exists for
 * typef(basic_type) == typeof(double).
 *
 * expected == true means: An inequality is expected, emit a message if nevertheless an equality was found
 * expected == boost::logic::indeterminate means: do not emit any messages
 * expected == false means: No inequality is expected, emit a message, if nevertheless an inequality was found
 *
 * @param className The name of the calling class
 * @param x The first parameter to be compared
 * @param y The second parameter to be compared
 * @param x_name The name of the first parameter
 * @param y_name The name of the second parameter
 * @param expected Indicates whether we expect the condition to be true or false, or whether any reporting should be suppressed ("ignore")
 * @return A boolean indicating whether any differences were found
 */
template <typename basic_type>
bool checkForDissimilarity(const std::string& className,
									           const std::vector<basic_type>& x,
									           const std::vector<basic_type>& y,
									           const double& limit,
									           const std::string& x_name,
									           const std::string& y_name,
									           const boost::logic::tribool& expected = boost::logic::indeterminate)
{
	bool raiseAlarm = false;
	bool result = false;
	std::ostringstream error;

	if(x==y) {
		if(expected == true) {  // expected the result to be "inequal"
			raiseAlarm = true;

			error << "//-----------------------------------------------------------------" << std::endl
				     << "Found no dissimilarity  in object of type \"" << className << "\" with" << std::endl
			         << x_name << " (type std::vector<" << typeid(basic_type).name() <<">): Size = " << x.size() << std::endl
			         << y_name << " (type std::vector<" << typeid(basic_type).name() <<">): Size = " << y.size() << std::endl
			         << "when dissimilarity was expected" << std::endl;
		}

		result = false;
	}
	else {
		if(expected==false && !boost::logic::indeterminate(expected)) {
			raiseAlarm=true;

			if(x.size() != y.size()) {
				error << "//-----------------------------------------------------------------" << std::endl
				          << "Found dissimilarity in object of type \"" << className << "\":" << std::endl
					      << x_name << " (type std::vector<" << typeid(basic_type).name() <<">): Size = " << x.size() << std::endl
					      << y_name << " (type std::vector<" << typeid(basic_type).name() <<">): Size = " << y.size() << std::endl
					      << "when no dissimilarity  was expected" << std::endl;
			}
			else { // something happened with one of the data entries
				// Loop over all data items
				for(std::size_t i=0; i<x.size(); i++) {
					if(x.at(i) != y.at(i)) {
						error << "//-----------------------------------------------------------------" << std::endl
							      << "Found dissimilarity in object of type \"" << className << "\":" << std::endl
							      << x_name << "[" << i << "] (type std::vector<" << typeid(basic_type).name() <<">) " << x.at(i) << std::endl
							      << y_name << "[" << i << "] (type std::vector<" << typeid(basic_type).name() <<">) " << y.at(i) << std::endl
							      << "when no dissimilarity was expected. Checks do not proceed after this index." << std::endl;

						break; // Leave the loop
					}
				}
			}
		}

		result=true;
	}

	if(raiseAlarm) {
		std::cerr << error.str();
	}

	return result;
}

/**************************************************************************************************/
/**
 * Checks for dissimilarity of the two arguments, which are assumed to be vectors of boost::share_ptr
 * objects of complex. This is needed by the isEqualTo function, so we have a standardized way of
 * emitting information on deviations. Note: If you want specific behavior for a particular type then you
 * can always create a specialization of this function.
 *
 * expected == true means: An inequality is expected, emit a message if nevertheless an equality was found
 * expected == boost::logic::indeterminate means: do not emit any messages
 * expected == false means: No inequality is expected, emit a message, if nevertheless an inequality was found
 *
 * @param className The name of the calling class
 * @param x The first parameter to be compared
 * @param y The second parameter to be compared
 * @param limit Acceptable limits for deviation of the two parameters
 * @param x_name The name of the first parameter
 * @param y_name The name of the second parameter
 * @param expected Indicates whether we expect the condition to be true or false, or whether any reporting should be suppressed ("ignore")
 * @return A boolean indicating whether any differences were found
 */
template <typename complex_type>
bool checkForDissimilarity(const std::string& className,
									           const std::vector<boost::shared_ptr<complex_type> >& x,
									           const std::vector<boost::shared_ptr<complex_type> >& y,
									           const double& limit,
									           const std::string& x_name,
									           const std::string& y_name,
									           const boost::logic::tribool& expected = boost::logic::indeterminate)
{
	bool raiseAlarm = false;
	bool result = false;
	std::ostringstream error;

	// Are there any inequalities in the vector's sizes ?
	if(x.size() != y.size()) {
		if(expected == false) {
			raiseAlarm = true;

			error << "//-----------------------------------------------------------------" << std::endl
			          << "Found dissimilarity in object of type \"" << className << "\":" << std::endl
					  << x_name << " (type std::vector<boost::shared_ptr<" << typeid(complex_type).name() <<"> >): Size = " << x.size() << std::endl
					  << y_name << " (type std::vector<boost::shared_ptr<" << typeid(complex_type).name() <<"> >): Size = " << y.size() << std::endl
				      << "when no dissimilarity was expected" << std::endl;
		}

		result = true;
	}
	else { // O.k., can we at least find inequalities in the data sets ?
		// Loop over all entries and find out which is wrong
		typename std::vector<boost::shared_ptr<complex_type> >::const_iterator cit_x;
		typename std::vector<boost::shared_ptr<complex_type> >::const_iterator cit_y;
	    std::size_t i=0;
		for(cit_x=x.begin(), cit_y=y.begin(); cit_x != x.end(), cit_y != y.end(); ++cit_x, ++cit_y) {
			if((*cit_x)->isNotSimilarTo(**cit_y, limit, expected)) {
				if(expected == false) {
					raiseAlarm = true;

					error << "//-----------------------------------------------------------------" << std::endl
							 << "Found dissimilarity in object of type \"" << className << "\":" << std::endl
							 << x_name << "[" << i << "] (type std::vector<boost::shared_ptr<" << typeid(complex_type).name() <<"> >) " << std::endl
							 << y_name << "[" << i << "] (type std::vector<boost:.shared_ptr<" << typeid(complex_type).name() <<"> >) " << std::endl
							 << "when no dissimilarity was expected. Checks do not proceed after this index." << std::endl;
				}

				result = true;
				break;
			}

			i++;
		}
	}

	// Is the result still false ? Then no inequality has been found. Did we expect an inequality ?
	if(result == false && expected==true) {
		raiseAlarm = true;

		error << "//-----------------------------------------------------------------" << std::endl
			     << "Found no dissimilarity  in object of type \"" << className << "\" with" << std::endl
				 << x_name << " (type std::vector<boost::shared_ptr<" << typeid(complex_type).name() <<"> >): Size = " << x.size() << std::endl
				 << y_name << " (type std::vector<boost::shared_ptr<" << typeid(complex_type).name() <<"> >): Size = " << y.size() << std::endl
		         << "when dissimilarity was expected" << std::endl;
	}

	if(raiseAlarm) {
		std::cerr << error.str();
	}

	return result;
}

/**************************************************************************************************/
/*
 * Specializations of some template functions
 */
template <> bool checkForInequality<std::map<std::string, std::string> >(const std::string&, const std::map<std::string, std::string>&,	const std::map<std::string, std::string>&, const std::string&, const std::string&, const boost::logic::tribool& expected);
template <> bool checkForDissimilarity<std::map<std::string, std::string> >(const std::string&, const std::map<std::string, std::string>&, const std::map<std::string, std::string>&, const double&, const std::string&, const std::string&, const boost::logic::tribool& expected);
template <> bool checkForDissimilarity<double>(const std::string&, const double&,	const double&, const double&, const std::string&, const std::string&, const boost::logic::tribool& expected);
template <> bool checkForDissimilarity<double>(const std::string&,  const std::vector<double>&,  const std::vector<double>&,  const double&,  const std::string&, const std::string&, const boost::logic::tribool& expected);

} /* namespace Util */
} /* namespace Gem */

#endif /* GHELPERFUNCTIONST_HPP_ */
