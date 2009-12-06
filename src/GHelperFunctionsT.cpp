/**
 * @file GHelperFunctionsT.cpp
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

#include "GHelperFunctionsT.hpp"

namespace Gem {
namespace Util {

/**************************************************************************************************/
/**
 * Specialization for typeof(basic_type) == typeof(double).
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
template <>
bool checkForDissimilarity<double>(const std::string& className,
															                       const double& x,
															                       const double& y,
															                       const double& limit,
															                       const std::string& x_name,
															                       const std::string& y_name,
															                       const boost::logic::tribool& expected)
{
	bool raiseAlarm = false;
	bool result = false;
	std::ostringstream error;

	if(fabs(x-y) <= fabs(limit)) { // deviation is within the allowed limits
		if(expected == true) {
			raiseAlarm = true; // expected the result to be "dissimilar"

			error << "//-----------------------------------------------------------------" << std::endl
				      << "Found the following value(s) in dissimilarity check of object of type \"" << className << "\":" << std::endl
				      << x_name << " (type double) = " << x << std::endl
				      << y_name << " (type double) = " << y << std::endl
				      << "limit = " << limit << std::endl
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
				      << "limit = " << limit << std::endl
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
 * Specialization for typeof(basic_type) == typeof(std::map<std::string, std::string>).
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
template <>
bool checkForDissimilarity<std::map<std::string, std::string> >(const std::string& className,
																																  const std::map<std::string, std::string>& x,
																																  const std::map<std::string, std::string>& y,
																																  const double& limit,
																																  const std::string& x_name,
																																  const std::string& y_name,
																																  const boost::logic::tribool& expected)
 {
	bool raiseAlarm = false;
	bool result = false;
	std::ostringstream error;

	if(x==y) {
		if(expected == true) {  // expected the result to be "dissimilar"
			raiseAlarm = true;

			error << "//-----------------------------------------------------------------" << std::endl
				     << "Found no dissimilarity  in object of type \"" << className << "\" with" << std::endl
			         << x_name << " (type std::map<std::string, std::string>): Size = " << x.size() << std::endl
			         << y_name << " (type std::map<std::string, std::string>): Size = " << y.size() << std::endl
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
				          << x_name << " (type std::map<std::string, std::string>): Size = " << x.size() << std::endl
				          << y_name << " (type std::map<std::string, std::string>): Size = " << y.size() << std::endl
					      << "when no dissimilarity  was expected" << std::endl;
			}
			else { // something happened with one of the data entries
				// Loop over all data items
			   std::map<std::string, std::string>::const_iterator cit_x, cit_y;
				for(cit_x=x.begin(), cit_y=y.begin(); cit_x!=x.end(), cit_y!=y.end(); ++cit_x, ++cit_y) {
					if(*cit_x != *cit_y) {
						raiseAlarm = true;
						error << "//-----------------------------------------------------------------" << std::endl
								  << "Found dissimilarity in object of type \"" << className << "\":" << std::endl
								  << x_name << " (type std::map<std::string, std::string>): Key = " << cit_x->first << " value = " << cit_x->second << std::endl
								  << y_name << " (type std::map<std::string, std::string>): Key = " << cit_y->first << " value = " << cit_y->second << std::endl
								  << "when no dissimilarity was expected." << std::endl;

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
 * Specialization for typeof(basic_type) == typeof(std::map<std::string, std::string>).
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
template <>
bool checkForInequality<std::map<std::string, std::string> >(const std::string& className,
																															const std::map<std::string, std::string>& x,
																															const std::map<std::string, std::string>& y,
																															const std::string& x_name,
																															const std::string& y_name,
																															const boost::logic::tribool& expected)
{
	bool raiseAlarm = false;
	bool result = false;
	std::ostringstream error;

	if(x==y) {
		if(expected == true) {  // expected the result to be "dissimilar"
			raiseAlarm = true;

			error << "//-----------------------------------------------------------------" << std::endl
				     << "Found equality  in object of type \"" << className << "\" with" << std::endl
			         << x_name << " (type std::map<std::string, std::string>): Size = " << x.size() << std::endl
			         << y_name << " (type std::map<std::string, std::string>): Size = " << y.size() << std::endl
			         << "when inequality  was expected" << std::endl;
		}

		result = false;
	}
	else {
		if(expected==false && !boost::logic::indeterminate(expected)) {
			raiseAlarm=true;

			if(x.size() != y.size()) {
				error << "//-----------------------------------------------------------------" << std::endl
				          << "Found no equality in object of type \"" << className << "\":" << std::endl
				          << x_name << " (type std::map<std::string, std::string>): Size = " << x.size() << std::endl
				          << y_name << " (type std::map<std::string, std::string>): Size = " << y.size() << std::endl
					      << "when equality  was expected" << std::endl;
			}
			else { // something happened with one of the data entries
				// Loop over all data items
			   std::map<std::string, std::string>::const_iterator cit_x, cit_y;
				for(cit_x=x.begin(), cit_y=y.begin(); cit_x!=x.end(), cit_y!=y.end(); ++cit_x, ++cit_y) {
					if(*cit_x != *cit_y) {
						raiseAlarm = true;
						error << "//-----------------------------------------------------------------" << std::endl
								  << "Found no equality in object of type \"" << className << "\":" << std::endl
								  << x_name << " (type std::map<std::string, std::string>): Key = " << cit_x->first << " value = " << cit_x->second << std::endl
								  << y_name << " (type std::map<std::string, std::string>): Key = " << cit_y->first << " value = " << cit_y->second << std::endl
								  << "when equality was expected." << std::endl;

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
 * Specialization for typeof(basic_type) == typeof(std::map<std::string, boost::variant<std::string, boost::int32_t, double, bool> >).
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
template <>
bool checkForDissimilarity<std::map<std::string, boost::variant<std::string, boost::int32_t, double, bool> > >
       (const std::string& className,
		const std::map<std::string, boost::variant<std::string, boost::int32_t, double, bool> >& x,
		const std::map<std::string, boost::variant<std::string, boost::int32_t, double, bool> >& y,
		const double& limit,
		const std::string& x_name,
		const std::string& y_name,
		const boost::logic::tribool& expected)
 {
	// This is BAD. We currently have no way to find out whether a given
	// boost::variant holds a double, hence we cannot check for similarity.
	return checkForInequality(className, x, y, x_name, y_name, expected);
 }

/**************************************************************************************************/
/**
 * Specialization for typeof(basic_type) == typeof(std::map<std::string, boost::variant<std::string, boost::int32_t, double, bool> >).
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
template <>
bool checkForInequality<std::map<std::string, boost::variant<std::string, boost::int32_t, double, bool> > >
	   (const std::string& className,
		const std::map<std::string, boost::variant<std::string, boost::int32_t, double, bool> >& x,
		const std::map<std::string, boost::variant<std::string, boost::int32_t, double, bool> >& y,
		const std::string& x_name,
		const std::string& y_name,
		const boost::logic::tribool& expected)
{
	bool raiseAlarm = false;
	bool result = false;
	std::ostringstream error;

	if(x==y) {
		if(expected == true) {  // expected the result to be "dissimilar"
			raiseAlarm = true;

			error << "//-----------------------------------------------------------------" << std::endl
				     << "Found equality  in object of type \"" << className << "\" with" << std::endl
			         << x_name << " (type std::map<std::string, boost::variant<> >): Size = " << x.size() << std::endl
			         << y_name << " (type std::map<std::string, boost::variant<> >): Size = " << y.size() << std::endl
			         << "when inequality  was expected" << std::endl;
		}

		result = false;
	}
	else {
		if(expected==false && !boost::logic::indeterminate(expected)) {
			raiseAlarm=true;

			if(x.size() != y.size()) {
				error << "//-----------------------------------------------------------------" << std::endl
				          << "Found no equality in object of type \"" << className << "\":" << std::endl
				          << x_name << " (type std::map<std::string, boost::variant<> >): Size = " << x.size() << std::endl
				          << y_name << " (type std::map<std::string, boost::variant<> >): Size = " << y.size() << std::endl
					      << "when equality  was expected" << std::endl;
			}
			else { // something happened with one of the data entries
				// Loop over all data items
				std::map<std::string, boost::variant<std::string, boost::int32_t, double, bool> >::const_iterator cit_x, cit_y;
				for(cit_x=x.begin(), cit_y=y.begin(); cit_x!=x.end(), cit_y!=y.end(); ++cit_x, ++cit_y) {
					if(*cit_x != *cit_y) {
						raiseAlarm = true;
						error << "//-----------------------------------------------------------------" << std::endl
								  << "Found no equality in object of type \"" << className << "\":" << std::endl
								  << x_name << " (type std::map<std::string, boost::variant<> >): Key = " << cit_x->first << " value = " << cit_x->second << std::endl
								  << y_name << " (type std::map<std::string, boost::variant<> >): Key = " << cit_y->first << " value = " << cit_y->second << std::endl
								  << "when equality was expected." << std::endl;

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
 * Checks for dissimilarity of the two arguments, which are assumed to be vectors of doubles.
 * This is needed by the isSimilarTo function, so we have a standardized way of emitting information
 * on deviations.
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
template <>
bool checkForDissimilarity<double>(const std::string& className,
									           const std::vector<double>& x,
									           const std::vector<double>& y,
									           const double& limit,
									           const std::string& x_name,
									           const std::string& y_name,
									           const boost::logic::tribool& expected)
{
	bool raiseAlarm = false;
	bool result = false;
	std::ostringstream error;

	if(x==y) {
		if(expected == true) {  // expected the result to be "inequal"
			raiseAlarm = true;

			error << "//-----------------------------------------------------------------" << std::endl
				     << "Found no dissimilarity  in object of type \"" << className << "\" with" << std::endl
			            << x_name << " (type std::vector<double>): Size = " << x.size() << std::endl
			            << y_name << " (type std::vector<double>): Size = " << y.size() << std::endl
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
				            << x_name << " (type std::vector<double>): Size = " << x.size() << std::endl
				            << y_name << " (type std::vector<double>): Size = " << y.size() << std::endl
					      << "when no dissimilarity  was expected" << std::endl;
			}
			else { // something happened with one of the data entries
				// Loop over all data items
				for(std::size_t i=0; i<x.size(); i++) {
					if(fabs(x.at(i) - y.at(i)) > fabs(limit)) {
						error << "//-----------------------------------------------------------------" << std::endl
							      << "Found dissimilarity in object of type \"" << className << "\":" << std::endl
						          << x_name << "[" << i << "] (type std::vector<double>) " << x.at(i) << std::endl
						          << y_name << "[" << i << "] (type std::vector<double>) " << y.at(i) << std::endl
						          << "limit = " << limit << std::endl
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

} /* namespace Util */
} /* namespace Gem */
