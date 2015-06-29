/**
 * @file GTupleIO.hpp
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
#include <iostream>
#include <string>
#include <tuple>
#include <type_traits>

// Boost heders go here
#include <boost/tuple/tuple.hpp>

#ifndef GTUPLEIO_HPP_
#define GTUPLEIO_HPP_

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
	oss << std::get<std::tuple_size<tuple_type>::value-p>(t) << ',';
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

#endif /* GTUPLEIO_HPP_ */
