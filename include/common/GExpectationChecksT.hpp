/**
 * @file GExpectationChecksT.hpp
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

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard headers go here
#include <vector>
#include <deque>
#include <set>
#include <functional>
#include <sstream>
#include <iostream>
#include <string>
#include <cstdlib>
#include <cmath>
#include <typeinfo>
#include <type_traits>
#include <tuple>

// Boost headers go here
#include <boost/cast.hpp>
#include <boost/logic/tribool.hpp>
#include <boost/logic/tribool_io.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/mpl/has_xxx.hpp>
#include <boost/noncopyable.hpp>
#include <boost/optional.hpp>
#include <boost/typeof/typeof.hpp>

#ifndef GEXPECTATIONCHECKST_HPP_
#define GEXPECTATIONCHECKST_HPP_

// Gemfony headers go here
#include "common/GCommonEnums.hpp"
#include "common/GMathHelperFunctions.hpp"
#include "common/GExceptions.hpp"
#include "common/GLogger.hpp"
#include "common/GTypeTraitsT.hpp"

namespace Gem {
namespace Common {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A token to be handed to different comparators, so they can signal the violation
 * of expectations
 */
class GToken
	: private boost::noncopyable
{
public:
	/** @brief The standard constructor -- initialization with class name and expectation */
	G_API_COMMON GToken(const std::string &, const Gem::Common::expectation &);

	/** @brief Increments the test counter */
	G_API_COMMON void incrTestCounter();
	/** @brief Increments the counter of tests that met the expectation */
	G_API_COMMON void incrSuccessCounter();

	/** @brief Allows to retrieve the current state of the success counter */
	G_API_COMMON std::size_t getSuccessCounter() const;
	/** @brief Allows to retrieve the current state of the test counter */
	G_API_COMMON std::size_t getTestCounter() const;

	/** @brief Allows to check whether the xpectation was met */
	G_API_COMMON bool expectationMet() const;
	/** @brief Conversion to a boolean indicating whether the expectation was met */
	G_API_COMMON operator bool() const;

	/** @brief Allows to retrieve the expectation token */
	G_API_COMMON Gem::Common::expectation getExpectation() const;
	/** @brief Allows to retrieve the expectation token as a string */
	G_API_COMMON std::string getExpectationStr() const;
	/** @brief Allows to retrieve the name of the caller */
	G_API_COMMON std::string getCallerName() const;

	/** @brief Allows to register an error message e.g. obtained from a failed check */
	G_API_COMMON void registerErrorMessage(const std::string &);
	/** @brief Allows to register an exception obtained from a failed check */
	G_API_COMMON void registerErrorMessage(const g_expectation_violation &);

	/** @brief Allows to retrieve the currently registered error messages */
	G_API_COMMON std::string getErrorMessages() const;

	/** @brief Conversion to a string indicating success or failure */
	G_API_COMMON std::string toString() const;

	/** @brief Evaluates the information in this object */
	G_API_COMMON void evaluate() const;

private:
	/** @brief The default constructor -- intentionally private and undefined */
	GToken() = delete;

	/** @brief Counts all tests vs. tests that have met the expectation */
	std::tuple<std::size_t, std::size_t> testCounter_;
	/** @brief Error messages obtained from failed checks */
	std::vector<std::string> errorMessages_;

	/** @brief The name of the calling class */
	const std::string caller_;
	/** @brief The expectation to be met */
	const Gem::Common::expectation e_;
};

/******************************************************************************/
/**
 * This function facilitates the output of GToken objects, mostly for debugging purposes.
 */
G_API_COMMON std::ostream &operator<<(std::ostream &s, const GToken &g);

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

#define BASENAME(B) std::string(#B)

/**
 * This struct facilitates transfer of comparable items to comparators
 */
template<typename T>
struct identity {
public:
	/***************************************************************************/
	/**
	 * The standard constructor
	 */
	identity(
		const T &x_var
		, const T &y_var
		, const std::string &x_name_var
		, const std::string &y_name_var
		, const double &l_var
	)
		: x(x_var)
		, y(y_var)
		, x_name(x_name_var)
		, y_name(y_name_var)
		, limit(l_var)
	{ /* nothing */ }

	/***************************************************************************/
	/**
	 * Conversion operator. Needed for compare_base, so we do not need
	 * to use macros for the implicit conversion
	 */
	template<typename B>
	operator identity<B>() const {
		// We use an internal function for the actual conversion
		// so we may check whether B is an actual base of T
		return to<B>();
	}

	/***************************************************************************/
	// The actual data
	const T &x;
	const T &y;
	const std::string x_name;
	const std::string y_name;
	const double limit;

private:
	/***************************************************************************/
	/** @brief The default constructor -- intentionally private and undefined */
	identity() = delete;

	/***************************************************************************/
	/**
	 * Does the actual conversion, including a check that B is indeed a base of T
	 */
	template<typename B>
	identity<B> to(
		typename std::enable_if<std::is_base_of<B, T>::value>::type *dummy = nullptr
	) const {
		const B &x_conv = dynamic_cast<const B &>(x);
		const B &y_conv = dynamic_cast<const B &>(y);

		const std::string x_name_conv = "(" + BASENAME(B) + ")" + x_name;
		const std::string y_name_conv = "(" + BASENAME(B) + ")" + y_name;

		return identity<B>(x_conv, y_conv, x_name_conv, y_name_conv, limit);
	}
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * Easy output of an identity object
 */
template<typename T>
std::ostream &operator<<(std::ostream &s, const identity<T> &i) {
	s
	<< "Identity:" << std::endl
	<< "x_name = " << i.x_name << std::endl
	<< "y_name = " << i.y_name << std::endl;
	return s;
}

/******************************************************************************/
/**
 * Returns an identity object. The function is needed as automatic type
 * deduction does not work for structs / classes. We assume a central default
 * value for the maximum allowed difference for "similar" floating point values.
 */
template<typename T>
identity<T> getIdentity(
	const T &x_var
	, const T &y_var
	, const std::string &x_name_var
	, const std::string &y_name_var
) {
	return identity<T>(x_var, y_var, x_name_var, y_name_var, Gem::Common::CE_DEF_SIMILARITY_DIFFERENCE);
}

/******************************************************************************/
/**
 * This macro facilitates the creation of an identity object including
 * variable names
 */
#define IDENTITY(x, y) Gem::Common::getIdentity((x), (y), std::string(#x), std::string(#y))

/******************************************************************************/
/**
 * Returns an identity object for base types of T
 */
template<typename T, typename B>
identity<B> getBaseIdentity(
	const T &x_var, const T &y_var, const std::string &x_name_var, const std::string &y_name_var
) {
	std::cout << "Creating base identity" << std::endl;

	const B &x_var_base = dynamic_cast<const B &>(x_var);
	const B &y_var_base = dynamic_cast<const B &>(y_var);

	return identity<B>(x_var_base, y_var_base, x_name_var, y_name_var, Gem::Common::CE_DEF_SIMILARITY_DIFFERENCE);
}

/******************************************************************************/
/**
 * This macro helps to cast an object to its parent class before creating the identity object
 */
#define IDENTITY_CAST(t, x, y) \
   Gem::Common::getBaseIdentity< decltype(x), t >((x), (y), std::string("(const " #t "&)" #x), std::string("(const " #t "&)" #y))

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This function checks whether two "basic" types fulfill a given expectation.
 * It assumes that x and y understand the == and != operators and may be streamed.
 * If they do not fulfill this requirement, you need to provide a specialization
 * of these functions. A check for similarity is treated the same as a check for
 * equality. A specialization of this function is provided for floating point values.
 * The function will throw a g_expectation_violation exception if the expectation
 * was violated.
 *
 * @param x The first parameter to be compared
 * @param y The second parameter to be compared
 * @param x_name The name of the first parameter
 * @param y_name The name of the second parameter
 * @param e The expectation both parameters need to fulfill
 * @param limit The maximum allowed deviation of two floating point values
 * @param dummy std::enable_if magic to steer overloaded resolution by the compiler
 */
template<typename basic_type>
void compare(
	const basic_type &x
	, const basic_type &y
	, const std::string &x_name
	, const std::string &y_name
	, const Gem::Common::expectation &e
	, const double &limit = 0.
	, typename std::enable_if<!std::is_floating_point<basic_type>::value>::type *dummy1 = nullptr // Note the negation!
	, typename std::enable_if<!Gem::Common::has_gemfony_common_interface<basic_type>::value>::type *dummy2 = nullptr // Note the negation
) {
	bool expectationMet = false;
	std::string expectation_str;

	switch (e) {
		case Gem::Common::expectation::CE_FP_SIMILARITY:
		case Gem::Common::expectation::CE_EQUALITY:
			expectation_str = "CE_FP_SIMILARITY / CE_EQUALITY";
			if (x == y) {
				expectationMet = true;
			}
			break;

		case Gem::Common::expectation::CE_INEQUALITY:
			expectation_str = "CE_INEQUALITY";
			if (x != y) {
				expectationMet = true;
			}
			break;

		default: {
			glogger
			<< "In compare(/* 1 */): Got invalid expectation " << e << std::endl
			<< GEXCEPTION;
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
/**
 * This function checks whether two floating point types meet a given expectation.
 * The function will throw a g_expectation_violation exception if the expectation
 * was violated.
 *
 * @param x The first parameter to be compared
 * @param y The second parameter to be compared
 * @param x_name The name of the first parameter
 * @param y_name The name of the second parameter
 * @param e The expectation both parameters need to fulfill
 * @param limit The maximum allowed deviation of two floating point values
 * @param dummy std::enable_if magic to steer overloaded resolution by the compiler
 */
template<typename fp_type>
void compare(
	const fp_type &x
	, const fp_type &y
	, const std::string &x_name
	, const std::string &y_name
	, const Gem::Common::expectation &e
	, const double &limit = CE_DEF_SIMILARITY_DIFFERENCE
	, typename std::enable_if<std::is_floating_point<fp_type>::value>::type *dummy = nullptr
) {
	bool expectationMet = false;
	std::string expectation_str;

	switch (e) {
		case Gem::Common::expectation::CE_FP_SIMILARITY:
			expectation_str = "CE_FP_SIMILARITY";
			if (gfabs(x - y) < boost::numeric_cast<fp_type>(limit)) {
				expectationMet = true;
			}
			break;
		case Gem::Common::expectation::CE_EQUALITY:
			expectation_str = "CE_EQUALITY";
			if (x == y) {
				expectationMet = true;
			}
			break;
		case Gem::Common::expectation::CE_INEQUALITY:
			expectation_str = "CE_INEQUALITY";
			if (x != y) {
				expectationMet = true;
			}
			break;

		default: {
			glogger
			<< "In compare(/* 2 */): Got invalid expectation " << e << std::endl
			<< GEXCEPTION;
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
/**
 * This function checks whether two containers of "basic" types meet a given expectation. It assumes that
 * these types understand the == and != operators. If they do not fulfill this requirement, you need to provide
 * a specialization of this function. A check for similarity is treated the same as a check for equality.
 * A specialization of this function is provided for floating point values. The function will throw a
 * g_expectation_violation exception if the expectation was violated.
 *
 * @param x The first container to be compared
 * @param y The second container to be compared
 * @param x_name The name of the first parameter
 * @param y_name The name of the second parameter
 * @param e The expectation both parameters need to fulfill
 * @param limit The maximum allowed deviation of two floating point values
 * @param dummy std::enable_if magic to steer overloaded resolution by the compiler
 */
template<typename base_type, template <typename, typename> class c_type>
void compare(
	const c_type<base_type, std::allocator<base_type>>& x
	, const c_type<base_type, std::allocator<base_type>>& y
	, const std::string &x_name
	, const std::string &y_name
	, const Gem::Common::expectation &e
	, const double &limit = 0.
	, typename std::enable_if<!std::is_floating_point<base_type>::value>::type *dummy = nullptr
) {
	bool expectationMet = false;
	std::string expectation_str;

	switch (e) {
		case Gem::Common::expectation::CE_FP_SIMILARITY:
		case Gem::Common::expectation::CE_EQUALITY:
			expectation_str = "CE_FP_SIMILARITY / CE_EQUALITY";
			if (x == y) {
				expectationMet = true;
			}
			break;

		case Gem::Common::expectation::CE_INEQUALITY:
			expectation_str = "CE_INEQUALITY";
			if (x != y) {
				expectationMet = true;
			}
			break;

		default: {
			glogger
			<< "In compare(/* 3 */): Got invalid expectation " << e << std::endl
			<< GEXCEPTION;
		}
			break;
	};

	if (!expectationMet) {
		std::ostringstream error;
		error
		<< "Expectation of " << expectation_str << " was violated for parameters "
		<< x_name << " and " << y_name << "!" << std::endl;

		if (Gem::Common::expectation::CE_FP_SIMILARITY == e || Gem::Common::expectation::CE_EQUALITY == e) {
			if (x.size() != y.size()) {
				error
				<< "Sizes of containers differ:" << std::endl
				<< x_name << ".size() == " << x.size() << " / " << y_name << ".size() == " << y.size() << std::endl;
			} else { // Some data member differs
				// Find out about the first entry that differs
				typename c_type<base_type, std::allocator<base_type>>::const_iterator x_it, y_it;
				std::size_t failedIndex = 0;
				for (x_it = x.begin(), y_it = y.begin(); x_it != x.end(); ++x_it, ++y_it, ++failedIndex) {
					if (*x_it != *y_it) {
						error
						<< "Found inequality at index " << failedIndex << ": "
						<< x_name << "[" << failedIndex << "] = " << *x_it << "; "
						<< y_name << "[" << failedIndex << "] = " << *y_it;
						break; // break the loop
					}
				}
			}
		} else { // Gem::Common::expectation::CE_INEQUALITY == e
			error
			<< "The two containers " << x_name << " and " << y_name << " are equal "
			<< "even though differences were expected"
			<< std::endl;
		}

		throw g_expectation_violation(error.str());
	}
}

/******************************************************************************/
/**
 * This function checks whether two containers with a std::set-template interface holding "basic" types meet
 * a given expectation. It assumes that these types understand the == and != operators. If they do not fulfill
 * this requirement, you need to provide a specialization of this function. A check for similarity is treated
 * the same as a check for equality. A specialization of this function is provided for floating point values.
 * The function will throw a g_expectation_violation exception if the expectation was violated.
 *
 * @param x The first container to be compared
 * @param y The second container to be compared
 * @param x_name The name of the first parameter
 * @param y_name The name of the second parameter
 * @param e The expectation both parameters need to fulfill
 * @param limit The maximum allowed deviation of two floating point values
 * @param dummy std::enable_if magic to steer overloaded resolution by the compiler
 */
template<typename base_type, template <typename, typename, typename> class s_type>
void compare(
	const s_type<base_type, std::less<base_type>, std::allocator<base_type>>& x
	, const s_type<base_type, std::less<base_type>, std::allocator<base_type>>& y
	, const std::string &x_name
	, const std::string &y_name
	, const Gem::Common::expectation &e
	, const double &limit = 0.
	, typename std::enable_if<!std::is_floating_point<base_type>::value>::type *dummy = nullptr
) {
	bool expectationMet = false;
	std::string expectation_str;

	switch (e) {
		case Gem::Common::expectation::CE_FP_SIMILARITY:
		case Gem::Common::expectation::CE_EQUALITY:
			expectation_str = "CE_FP_SIMILARITY / CE_EQUALITY";
			if (x == y) {
				expectationMet = true;
			}
			break;

		case Gem::Common::expectation::CE_INEQUALITY:
			expectation_str = "CE_INEQUALITY";
			if (x != y) {
				expectationMet = true;
			}
			break;

		default: {
			glogger
				<< "In compare(/* 4 */): Got invalid expectation " << e << std::endl
				<< GEXCEPTION;
		}
			break;
	};

	if (!expectationMet) {
		std::ostringstream error;
		error
			<< "Expectation of " << expectation_str << " was violated for parameters "
			<< x_name << " and " << y_name << "!" << std::endl;

		if (Gem::Common::expectation::CE_FP_SIMILARITY == e || Gem::Common::expectation::CE_EQUALITY == e) {
			if (x.size() != y.size()) {
				error
					<< "Sizes of containers differ:" << std::endl
					<< x_name << ".size() == " << x.size() << " / " << y_name << ".size() == " << y.size() << std::endl;
			} else { // Some data member differs
				// Find out about the first entry that differs
				typename s_type<base_type, std::less<base_type>, std::allocator<base_type>>::const_iterator x_it, y_it;
				std::size_t failedIndex = 0;
				for (x_it = x.begin(), y_it = y.begin(); x_it != x.end(); ++x_it, ++y_it, ++failedIndex) {
					if (*x_it != *y_it) {
						error
							<< "Found inequality at index " << failedIndex << ": "
							<< x_name << "[" << failedIndex << "] = " << *x_it << "; "
							<< y_name << "[" << failedIndex << "] = " << *y_it;
						break; // break the loop
					}
				}
			}
		} else { // Gem::Common::expectation::CE_INEQUALITY == e
			error
				<< "The two containers " << x_name << " and " << y_name << " are equal "
				<< "even though differences were expected"
				<< std::endl;
		}

		throw g_expectation_violation(error.str());
	}
}

/******************************************************************************/
/**
 * This function checks whether two containers of floating point types meet a given expectation.
 *
 * @param x The first vector to be compared
 * @param y The second vector to be compared
 * @param x_name The name of the first parameter
 * @param y_name The name of the second parameter
 * @param e The expectation both parameters need to fulfill
 * @param limit The maximum allowed deviation of two floating point values
 * @param dummy std::enable_if magic to steer overloaded resolution by the compiler
 */
template<typename fp_type, template <typename, typename> class c_type>
void compare(
	const c_type<fp_type, std::allocator<fp_type>> &x
	, const c_type<fp_type, std::allocator<fp_type>> &y
	, const std::string &x_name
	, const std::string &y_name
	, const Gem::Common::expectation &e
	, const double &limit = CE_DEF_SIMILARITY_DIFFERENCE
	, typename std::enable_if<std::is_floating_point<fp_type>::value>::type *dummy = nullptr
) {
	bool expectationMet = false;
	std::string expectation_str;
	std::size_t deviation_pos = 0;
	std::ostringstream error;

	switch (e) {
		case Gem::Common::expectation::CE_FP_SIMILARITY:
		case Gem::Common::expectation::CE_EQUALITY: {
			if (Gem::Common::expectation::CE_FP_SIMILARITY == e) {
				expectation_str = "CE_FP_SIMILARITY";
			} else if (Gem::Common::expectation::CE_EQUALITY == e) {
				expectation_str = "CE_EQUALITY";
			}

			if (x.size() != y.size()) {
				error
				<< "Different vector-sizes found : "
				<< x_name << ".size() = " << x.size() << std::endl
				<< y_name << ".size() = " << y.size() << std::endl;
				break; // expectationMet is false here
			}

			// Do a per-position comparison
			bool foundDeviation = false;
			typename c_type<fp_type, std::allocator<fp_type>>::const_iterator x_it, y_it;
			if (Gem::Common::expectation::CE_FP_SIMILARITY == e) {
				for (x_it = x.begin(), y_it = y.begin(); x_it != x.end(); ++x_it, ++y_it) {
					if (gfabs(*x_it - *y_it) >= boost::numeric_cast<fp_type>(limit)) {
						foundDeviation = true;
						deviation_pos = std::distance(x.begin(), x_it);
						error
						<< "Found deviation between containers:" << std::endl
						<< x_name << "[" << deviation_pos << "] = " << *x_it << "; " << std::endl
						<< y_name << "[" << deviation_pos << "] = " << *y_it << "; " << std::endl
						<< "limit = " << boost::numeric_cast<fp_type>(limit) << "; " << std::endl
						<< "deviation = " << gfabs(*x_it - *y_it) << std::endl;
						break; // break the loop
					}
				}
			} else if (Gem::Common::expectation::CE_EQUALITY == e) {
				for (x_it = x.begin(), y_it = y.begin(); x_it != x.end(); ++x_it, ++y_it) {
					if (*x_it != *y_it) {
						foundDeviation = true;
						deviation_pos = std::distance(x.begin(), x_it);
						error
						<< "Found deviation between containers:" << std::endl
						<< x_name << "[" << deviation_pos << "] = " << *x_it << "; " << std::endl
						<< y_name << "[" << deviation_pos << "] = " << *y_it << "; " << std::endl;
						break; // break the loop
					}
				}
			}

			if (!foundDeviation) {
				expectationMet = true;
			}
		}
			break;

		case Gem::Common::expectation::CE_INEQUALITY:
			expectation_str = "CE_INEQUALITY";
			if (x != y) {
				expectationMet = true;
			} else {
				error
				<< "The containers " << x_name << " and " << y_name << std::endl
				<< "do not differ even though they should" << std::endl;
			}
			break;

		default: {
			glogger
			<< "In compare(/* 5 */): Got invalid expectation " << e << std::endl
			<< GEXCEPTION;
		}
			break;
	};

	if (!expectationMet) {
		throw g_expectation_violation(error.str());
	}
}

/******************************************************************************/
/**
 * This function checks whether two containers with a std::set template interface,
 * holding floating point types meet a given expectation.
 *
 * @param x The first vector to be compared
 * @param y The second vector to be compared
 * @param x_name The name of the first parameter
 * @param y_name The name of the second parameter
 * @param e The expectation both parameters need to fulfill
 * @param limit The maximum allowed deviation of two floating point values
 * @param dummy std::enable_if magic to steer overloaded resolution by the compiler
 */
template<typename fp_type, template <typename, typename, typename> class s_type>
void compare(
	const s_type<fp_type, std::less<fp_type>, std::allocator<fp_type>> &x
	, const s_type<fp_type, std::less<fp_type>, std::allocator<fp_type>> &y
	, const std::string &x_name
	, const std::string &y_name
	, const Gem::Common::expectation &e
	, const double &limit = CE_DEF_SIMILARITY_DIFFERENCE
	, typename std::enable_if<std::is_floating_point<fp_type>::value>::type *dummy = nullptr
) {
	bool expectationMet = false;
	std::string expectation_str;
	std::size_t deviation_pos = 0;
	std::ostringstream error;

	switch (e) {
		case Gem::Common::expectation::CE_FP_SIMILARITY:
		case Gem::Common::expectation::CE_EQUALITY: {
			if (Gem::Common::expectation::CE_FP_SIMILARITY == e) {
				expectation_str = "CE_FP_SIMILARITY";
			} else if (Gem::Common::expectation::CE_EQUALITY == e) {
				expectation_str = "CE_EQUALITY";
			}

			if (x.size() != y.size()) {
				error
					<< "Different vector-sizes found : "
					<< x_name << ".size() = " << x.size() << std::endl
					<< y_name << ".size() = " << y.size() << std::endl;
				break; // expectationMet is false here
			}

			// Do a per-position comparison
			bool foundDeviation = false;
			typename s_type<fp_type, std::less<fp_type>, std::allocator<fp_type>>::const_iterator x_it, y_it;
			if (Gem::Common::expectation::CE_FP_SIMILARITY == e) {
				for (x_it = x.begin(), y_it = y.begin(); x_it != x.end(); ++x_it, ++y_it) {
					if (gfabs(*x_it - *y_it) >= boost::numeric_cast<fp_type>(limit)) {
						foundDeviation = true;
						deviation_pos = std::distance(x.begin(), x_it);
						error
							<< "Found deviation between containers:" << std::endl
							<< x_name << "[" << deviation_pos << "] = " << *x_it << "; " << std::endl
							<< y_name << "[" << deviation_pos << "] = " << *y_it << "; " << std::endl
							<< "limit = " << boost::numeric_cast<fp_type>(limit) << "; " << std::endl
							<< "deviation = " << gfabs(*x_it - *y_it) << std::endl;
						break; // break the loop
					}
				}
			} else if (Gem::Common::expectation::CE_EQUALITY == e) {
				for (x_it = x.begin(), y_it = y.begin(); x_it != x.end(); ++x_it, ++y_it) {
					if (*x_it != *y_it) {
						foundDeviation = true;
						deviation_pos = std::distance(x.begin(), x_it);
						error
							<< "Found deviation between containers:" << std::endl
							<< x_name << "[" << deviation_pos << "] = " << *x_it << "; " << std::endl
							<< y_name << "[" << deviation_pos << "] = " << *y_it << "; " << std::endl;
						break; // break the loop
					}
				}
			}

			if (!foundDeviation) {
				expectationMet = true;
			}
		}
			break;

		case Gem::Common::expectation::CE_INEQUALITY:
			expectation_str = "CE_INEQUALITY";
			if (x != y) {
				expectationMet = true;
			} else {
				error
					<< "The containers " << x_name << " and " << y_name << std::endl
					<< "do not differ even though they should" << std::endl;
			}
			break;

		default: {
			glogger
				<< "In compare(/* 6 */): Got invalid expectation " << e << std::endl
				<< GEXCEPTION;
		}
			break;
	};

	if (!expectationMet) {
		throw g_expectation_violation(error.str());
	}
}


/******************************************************************************/
/**
 * This function checks whether two complex types meet a given expectation. It is assumed that
 * these types have the standard Geneva interface with corresponding "compare" functions.
 *
 * @param x The first parameter to be compared
 * @param y The second parameter to be compared
 * @param x_name The name of the first parameter
 * @param y_name The name of the second parameter
 * @param e The expectation both parameters need to fulfill
 * @param limit The maximum allowed deviation of two floating point values
 * @param dummy std::enable_if magic to steer overloaded resolution by the compiler
 */
template <typename geneva_type>
void compare (
	const geneva_type& x
	, const geneva_type& y
	, const std::string& x_name
	, const std::string& y_name
	, const Gem::Common::expectation& e
	, const double& limit = Gem::Common::CE_DEF_SIMILARITY_DIFFERENCE
	, typename std::enable_if<Gem::Common::has_gemfony_common_interface<geneva_type>::value>::type *dummy = nullptr
) {
	bool expectationMet = false;
	std::string expectation_str;
	std::ostringstream error;

	switch(e) {
		case Gem::Common::expectation::CE_FP_SIMILARITY:
		case Gem::Common::expectation::CE_EQUALITY:
		{
			expectation_str = "CE_FP_SIMILARITY / CE_EQUALITY";

			// If we reach this line, then both pointers have content

			{ // Check whether the content differs
				try {
					x.compare(y,e,limit);
				} catch(g_expectation_violation& g) {
					error
					<< "Content of " << x_name << " and " << y_name << " differ." << std::endl
					<< "Thus the expectation of " << expectation_str << " was violated:" << std::endl
					<< g.what() << std::endl;
					break; // Terminate the switch statement
				}

				// If we reach this line, the expectation was met
				expectationMet = true;
			}
		}
			break;

		case Gem::Common::expectation::CE_INEQUALITY:
		{
			expectation_str = "CE_INEQUALITY";

			// Check whether the content differs
			try {
				x.compare(y,e,limit);
			} catch(g_expectation_violation& g) {
				// If we catch an expectation violation for expectation "inequality",
				// we simply break the switch statement so that expectationMet remains to be false
				error
				<< "Content of " << x_name << " and " << y_name << " are equal/similar." << std::endl
				<< "Thus the expectation of " << expectation_str << " was violated:" << std::endl
				<< g.what() << std::endl;
				break;
			}
			expectationMet = true;
		}
			break;

		default:
		{
			glogger
			<< "In compare(/* 7 */): Got invalid expectation " << e << std::endl
			<< GEXCEPTION;
		}
			break;
	};

	if(!expectationMet) {
		throw g_expectation_violation(error.str());
	}
}

/******************************************************************************/
/**
 * This function checks whether two smart pointers to complex types meet a given expectation.
 * It is assumed that these types have the standard Geneva interface with corresponding "compare"
 * functions.
 *
 * @param x The first parameter to be compared
 * @param y The second parameter to be compared
 * @param x_name The name of the first parameter
 * @param y_name The name of the second parameter
 * @param e The expectation both parameters need to fulfill
 * @param limit The maximum allowed deviation of two floating point values
 * @param dummy std::enable_if magic to steer overloaded resolution by the compiler
 */
template <typename geneva_type>
void compare (
	const std::shared_ptr<geneva_type>& x
	, const std::shared_ptr<geneva_type>& y
	, const std::string& x_name
	, const std::string& y_name
	, const Gem::Common::expectation& e
	, const double& limit = Gem::Common::CE_DEF_SIMILARITY_DIFFERENCE
	, typename std::enable_if<Gem::Common::has_gemfony_common_interface<geneva_type>::value>::type *dummy = nullptr
) {
	bool expectationMet = false;
	std::string expectation_str;
	std::ostringstream error;

	switch(e) {
		case Gem::Common::expectation::CE_FP_SIMILARITY:
		case Gem::Common::expectation::CE_EQUALITY:
		{
			expectation_str = "CE_FP_SIMILARITY / CE_EQUALITY";

			// Check whether the pointers hold content
			if(x && !y) {
				error
				<< "Smart pointer " << x_name << " holds content while " << y_name << " does not." << std::endl
				<< "Thus the expectation of " << expectation_str << " was violated" << std::endl;
				break; //
			} else if(!x && y) {
				error
				<< "Smart pointer " << x_name << " doesn't hold content while " << y_name << " does." << std::endl
				<< "Thus the expectation of " << expectation_str << " was violated" << std::endl;
				break;  // The expectation was clearly not met
			} else if(!x && !y) { // No content to check. Both smart pointers can be considered equal
				expectationMet = true;
				break;
			}

			// If we reach this line, then both pointers have content

			{ // Check whether the content differs
				try {
					x->compare(*y,e,limit);
				} catch(g_expectation_violation& g) {
					error
					<< "Content of " << x_name << " and " << y_name << " differ." << std::endl
					<< "Thus the expectation of " << expectation_str << " was violated:" << std::endl
					<< g.what() << std::endl;
					break; // Terminate the switch statement
				}

				// If we reach this line, the expectation was met
				expectationMet = true;
			}
		}
			break;

		case Gem::Common::expectation::CE_INEQUALITY:
		{
			expectation_str = "CE_INEQUALITY";

			// Check whether the pointers hold content
			if((x && !y) || (!x && y)) {
				expectationMet = true;
				break;
			} else if(!x && !y) { // No content to check. Both smart pointers can be considered equal
				error
				<< "Both smart pointers are empty and are thus considered equal." << std::endl
				<< "Thus the expectation of " << expectation_str << " was violated:" << std::endl;
				break; // The expectation was not met
			}

			// Check whether the content differs
			try {
				x->compare(*y,e,limit);
			} catch(g_expectation_violation& g) {
				// If we catch an expectation violation for expectation "inequality",
				// we simply break the switch statement so that expectationMet remains to be false
				error
				<< "Content of " << x_name << " and " << y_name << " are equal/similar." << std::endl
				<< "Thus the expectation of " << expectation_str << " was violated:" << std::endl
				<< g.what() << std::endl;
				break;
			}
			expectationMet = true;
		}
			break;

		default:
		{
			glogger
			<< "In compare(/* 8 */): Got invalid expectation " << e << std::endl
			<< GEXCEPTION;
		}
			break;
	};

	if(!expectationMet) {
		throw g_expectation_violation(error.str());
	}
}

/******************************************************************************/
/**
 * This function checks whether two containers of smart pointers to complex types meet a given expectation.
 * It is assumed that these types have the standard Geneva interface with corresponding "compare"
 * functions. For an idea of what the template specifyer does, search for "template template" in conjunction
 * with containers.
 *
 * @param x The first vector to be compared
 * @param y The second vector to be compared
 * @param x_name The name of the first parameter
 * @param y_name The name of the second parameter
 * @param e The expectation both parameters need to fulfill
 * @param limit The maximum allowed deviation of two floating point values
 * @param dummy std::enable_if magic to steer overloaded resolution by the compiler
 */
template <typename geneva_type, template <typename, typename> class c_type>
void compare (
	const c_type<std::shared_ptr<geneva_type>, std::allocator<std::shared_ptr<geneva_type>>>& x
	, const c_type<std::shared_ptr<geneva_type>, std::allocator<std::shared_ptr<geneva_type>>>& y
	, const std::string& x_name
	, const std::string& y_name
	, const Gem::Common::expectation& e
	, const double& limit = Gem::Common::CE_DEF_SIMILARITY_DIFFERENCE
	, typename std::enable_if<Gem::Common::has_gemfony_common_interface<geneva_type>::value>::type *dummy = nullptr
) {
	bool expectationMet = false;
	std::string expectation_str;
	std::ostringstream error;

	switch(e) {
	case Gem::Common::expectation::CE_FP_SIMILARITY:
	case Gem::Common::expectation::CE_EQUALITY: {
		expectation_str = "CE_FP_SIMILARITY / CE_EQUALITY";

		// First check sizes
		if(x.size() != y.size()) {
			error
			<< "containers " << x_name << " and " << y_name << " have different sizes " << x.size() << " / " << y.size() << std::endl
			<< "Thus the expectation of " << expectation_str << " was violated:" << std::endl;
			// Terminate the switch statement. expectationMet will be false then
			break;
		}

		// Now loop over all members of the containers
		bool foundDeviation = false;
		typename c_type<std::shared_ptr<geneva_type>, std::allocator<std::shared_ptr<geneva_type>>>::const_iterator x_it, y_it;
		std::size_t index = 0;
		for(x_it=x.begin(), y_it=y.begin(); x_it!=x.end(); ++x_it, ++y_it, ++index) {
			// First check that both pointers have content
			// Check whether the pointers hold content
			if(*x_it && !*y_it) {
				error
				<< "Smart pointer " << x_name << "[" << index << "] holds content while " << y_name << "[" << index << "]  does not." << std::endl
				<< "Thus the expectation of " << expectation_str << " was violated" << std::endl;
				foundDeviation = true;
				break; // terminate the loop
			} else if(!*x_it && *y_it) {
				error
				<< "Smart pointer " << x_name << "[" << index << "] doesn't hold content while " << y_name << "[" << index << "]  does." << std::endl
				<< "Thus the expectation of " << expectation_str << " was violated" << std::endl;
				foundDeviation = true;
				break;  // terminate the loop
			} else if(!*x_it && !*y_it) { // No content to check. Both smart pointers can be considered equal
				continue; // Go on with next iteration in the loop
			}

			// At this point we know that both pointers have content. We can now check the content
			// which is assumed to have the compare() function
			try {
				(*x_it)->compare(**y_it,e,limit);
			} catch(g_expectation_violation& g) {
				error
				<< "Content of " << x_name << "[" << index << "] and " << y_name << "[" << index << "] differs." << std::endl
				<< "Thus the expectation of " << expectation_str << " was violated:" << std::endl
				<< g.what() << std::endl;
				foundDeviation = true;
				break; // Terminate the loop
			}
		}

		if(!foundDeviation) {
			expectationMet = true;
		}
	} break;

	case Gem::Common::expectation::CE_INEQUALITY: {
		expectation_str = "CE_INEQUALITY";

		// First check sizes. The expectation of inequality will be met if they differ
		if(x.size() != y.size()) {
			expectationMet = true;
			break; // Terminate the switch statement
		}

		// Now loop over all members of the containers
		bool foundInequality = false;
		typename c_type<std::shared_ptr<geneva_type>, std::allocator<std::shared_ptr<geneva_type>>>::const_iterator x_it, y_it;
		for(x_it=x.begin(), y_it=y.begin(); x_it!=x.end(); ++x_it, ++y_it) {
			// First check that both pointers have content
			// Check whether the pointers hold content
			if((*x_it && !*y_it) || (!*x_it && *y_it)) {
				foundInequality = true;
				break; // terminate the loop
			} else if(!*x_it && !*y_it) { // No content to check. Both smart pointers can be considered equal
				continue; // Go on with next iteration in the loop - there is nothing to check here
			}

			// At this point we know that both pointers have content. We can now check this content
			// which is assumed to have the compare() function
			try {
				(*x_it)->compare(**y_it,e,limit);
				foundInequality = true;
				break; // terminate the loop
			} catch(g_expectation_violation&) {
				// Go on with the next item in the vector -- the content is equal or similar
				continue;
			}
		}

		if(foundInequality) {
			expectationMet = true;
		} else {
			error
			<< "The two containers " << x_name << " and " << y_name << " are equal." << std::endl
			<< "Thus the expectation of " << expectation_str << " was violated:" << std::endl;
		}
	} break;

	default: {
		glogger
		<< "In compare(/* 9 */): Got invalid expectation " << e << std::endl
		<< GEXCEPTION;
	} break;
	};

	if(!expectationMet) {
		throw g_expectation_violation(error.str());
	}
}

/******************************************************************************/
/** @brief This function checks whether two objects of type boost::logic::tribool meet a given expectation. */
G_API_COMMON
void compare(
	const boost::logic::tribool &
	, const boost::logic::tribool &
	, const std::string &
	, const std::string &
	, const Gem::Common::expectation &
	, const double &limit = CE_DEF_SIMILARITY_DIFFERENCE
);

/******************************************************************************/
/**
 * This function checks whether two types fulfill a given expectation.
 *
 * @param data The identity struct
 * @param token The token holding information about the number of failed tests
 */
template<typename T>
void compare_t(
	const identity<T> &data
	, GToken &token
) {
	try {
		token.incrTestCounter();
		compare(data.x, data.y, data.x_name, data.y_name, token.getExpectation(), data.limit);
		token.incrSuccessCounter();
	} catch (const g_expectation_violation &g) {
		token.registerErrorMessage(g);
	} catch (const std::exception &e) {
		glogger
		<< "Caught std::exception with message " << std::endl
		<< e.what() << std::endl
		<< GEXCEPTION;
	} catch (...) {
		glogger
		<< "Caught unknown exception" << std::endl
		<< GEXCEPTION;
	}
}

/******************************************************************************/
/**
 * This function checks whether two base types fulfill a given expectation.
 *
 * @param data The identity struct
 * @param token The token holding information about the number of failed tests
 */
template<typename B>
void compare_base(
	const identity<B> &data
	, GToken &token
	, typename std::enable_if<Gem::Common::has_gemfony_common_interface<B>::value>::type *dummy = nullptr
) {
	try {
		token.incrTestCounter();
		data.x.B::compare(data.y, token.getExpectation(), data.limit);
		token.incrSuccessCounter();
	} catch (const g_expectation_violation &g) {
		token.registerErrorMessage(g);
	} catch (const std::exception &e) {
		glogger
		<< "Caught std::exception with message " << std::endl
		<< e.what() << std::endl
		<< GEXCEPTION;
	} catch (...) {
		glogger
		<< "Caught unknown exception" << std::endl
		<< GEXCEPTION;
	}
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Common */
} /* namespace Gem */

#endif /* GEXPECTATIONCHECKST_HPP_ */
