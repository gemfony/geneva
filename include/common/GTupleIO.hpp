/*
 * This file is part of the Geneva library collection.
 *
 * See the NOTICE file in the top-level directory of the Geneva library
 * collection for a list of contributors and copyright information.
 *
 * The following license applies to the code IN THIS FILE:
 *
 * ***************************************************************************
 *
 * Boost Software License - Version 1.0 - August 17th, 2003
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * ***************************************************************************
 *
 * NOTE THAT THE BOOST-LICENSE DOES NOT APPLY TO ANY OTHER FILES OF THE
 * GENEVA LIBRARY, UNLESS THIS IS EXPLICITLY STATED IN THE CORRESPONDING FILE!
 */

#pragma once

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard headers go here
#include <iostream>
#include <string>
#include <tuple>
#include <type_traits>

// Boost heders go here
#include <boost/tuple/tuple.hpp>

// Gemfony headers go here

namespace Gem {
namespace Common {

/******************************************************************************/
// This set of functions allows to output tuples of any size, starting at 0.
// Boost boost::tuple and std::tuple are supported.

/******************************************************************************/
/**
 * Defines a sequence. Its only purpose is to serve as a stop criterion, so that
 * when tuple_output_seq<1> is reached, recursion stops.
 */
template<std::size_t>
struct tuple_output_seq
{ /* nothing */ };

/******************************************************************************/
/**
 * Recursively breaks down output of a tuple. Note: this function assumes that
 * the corresponding tuple element may indeed be streamed!
 */
template <class tuple_type, size_t p>
std::string g_to_string(
	const tuple_type& t
	, const std::string& s
	, tuple_output_seq<p> sq// This will determine the current value of p
){
	std::ostringstream oss;
	oss << ", " << std::get<std::tuple_size<tuple_type>::value-p>(t);
	return (g_to_string(t, oss.str(), tuple_output_seq<p-1>()) + s);
}

/******************************************************************************/
/**
 * Specialization for the case of a single element in the tuple. This overload
 * makes sure that no attempt is made to continue recursion.
 */
template <class tuple_type>
std::string g_to_string(
	const tuple_type& t
	, const std::string& s
	, tuple_output_seq<1> sq
){
	std::ostringstream oss;
	oss << std::get<0>(t);
  	return oss.str() + s;
}

/******************************************************************************/
/**
 * This command is supposed to add nothing to the string, as a tuple<>()
 * has no data. It is meant to cover cases like "std::cout << std::make_tuple();".
 */
template <class tuple_type>
std::string g_to_string(
	const tuple_type& t
	, const std::string& s
	, tuple_output_seq<0> sq
) {
	return std::string() + s;
}

/******************************************************************************/
/**
 * The actual output function for std::tuple. This implementation uses
 * information taken from http://www.cplusplus.com/articles/EhvU7k9E/ to determine
 * the size of the tuple. It relies on the services of a "g_to_string" function defined
 * for std::tuple and boost::tuple above.
 */
template <class... args>
std::string g_to_string(
	const std::tuple<args...>& t
){
	static const unsigned short int sz = sizeof...(args); // The actual tuple size
	std::string empty;
	return std::string("(") + g_to_string(t, empty, tuple_output_seq<sz>()) + std::string(")");
}

/******************************************************************************/
/**
 * The actual output function for boost::tuple. This implementation uses
 * information taken from http://www.cplusplus.com/articles/EhvU7k9E/ to determine
 * the size of the tuple. It relies on the services of a "g_to_string" function defined
 * for std::tuple and boost::tuple above.
 */
template <class... args>
std::string g_to_string(
	const boost::tuple<args...>& t
){
	static const unsigned short int sz = sizeof...(args); // The actual tuple size
	std::string empty;
	return std::string("(") + g_to_string(t, empty, tuple_output_seq<sz>()) + std::string(")");
}
/******************************************************************************/
/**
 * Output for all other streamable types
 */
template <typename T>
std::string g_to_string(const T& t){
	std::ostringstream oss;
	oss << t;
	return oss.str();
}

/******************************************************************************/
/**
 * Streaming operator for std::tuple
 */
template <class... args>
std::ostream& operator<<(
	std::ostream& o
	, const std::tuple<args...>& t
) {
	o << g_to_string(t);
	return o;
}

/******************************************************************************/
/**
 * Streaming operator for boost::tuple
 */
template <class... args>
std::ostream& operator<<(
	std::ostream& o
	, const boost::tuple<args...>& t
) {
	o << g_to_string(t);
	return o;
}

/******************************************************************************/

} /* namespace Common */
} /* namespace Gem */
