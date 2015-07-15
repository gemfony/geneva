/**
 * @file GCommonEnums.hpp
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
#include <string>
#include <istream>
#include <ostream>
#include <cstdio>
#include <cstdlib>
#include <csignal>
#include <cmath>

// Boost headers go here

#include <boost/cast.hpp>

#ifndef GCOMMONENUMS_HPP_
#define GCOMMONENUMS_HPP_

// Geneva headers go here

/******************************************************************************/
/**
 * We need local signals, so we can act both on Windows and POSIX-OSs
 */
#if defined(_MSC_VER)  && (_MSC_VER >= 1020)
#define G_SIGHUP CTRL_CLOSE_EVENT
#else
#define G_SIGHUP SIGHUP
#endif
/******************************************************************************/

namespace Gem {
namespace Common {

/******************************************************************************/
/**
 * This function allows to put generic strongly typed enums from the Gem::Common
 * namespace into an output stream. Note the redundancy here -- we want to avoid
 * clashes with user-defined operators, so we leave this facility inside of the
 * Gem::Common namespace.
 */
template <typename enum_type, typename stream_type>
typename std::enable_if<std::is_enum<enum_type>::value, stream_type&>::type
operator<<(
	stream_type& ops
	, const enum_type& t
) {
	std::uint16_t tmp = static_cast<std::uint16_t>(t);
	ops << tmp;
	return ops;
};


/******************************************************************************/
/**
 * This function allows to read generic strongly typed enums from the Gem::Common
 * namespace from an input stream. Note the redundancy here -- we want to avoid
 * clashes with user-defined operators, so we leave this facility inside of the
 * Gem::Common namespace.
 */
template <typename enum_type, typename stream_type>
typename std::enable_if<std::is_enum<enum_type>::value, stream_type&>::type
operator>>(
	stream_type& ips
	, enum_type& t
) {
	std::uint16_t tmp;
	ips >> tmp;

#ifdef DEBUG
	t = boost::numeric_cast<enum_type>(tmp);
#else
	t = static_cast<enum_type>(tmp);
#endif /* DEBUG */

	return ips;
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This enum denotes different dimensions (used particularly by GMarkerCollection
 */
enum class dimensions {
	x = 0
	, y = 1
	, z = 2
	, w = 3
};

/******************************************************************************/
/** @brief Puts a Gem::Common::dimensions into a stream. Needed also for boost::lexical_cast<> */
G_API_COMMON std::ostream &operator<<(std::ostream &, const Gem::Common::dimensions &);
/** @brief Reads a Gem::Common::dimensions from a stream. Needed also for boost::lexical_cast<> */
G_API_COMMON std::istream &operator>>(std::istream &, Gem::Common::dimensions &);

/******************************************************************************/
/**
 * The limit for similarity checks of floating point numbers
 */
const double CE_DEF_SIMILARITY_DIFFERENCE = pow(10., -5);

/******************************************************************************/
/**
 * Indicates whether higher or lower values are considered better. Needed e.g.
 * in conjunction with the sorting in priority queues.
 */
const bool HIGHERISBETTER = true;
const bool LOWERISBETTER = false;

/******************************************************************************/
/**
 * Different log and exception types
 */
enum class logType {
	EXCEPTION = 0
	, TERMINATION = 1
	, WARNING = 2
	, LOGGING = 3
	, FILE = 4
	, STDOUT = 5
	, STDERR = 6
};

/******************************************************************************/
/** @brief Puts a Gem::Common::logType into a stream. Needed also for boost::lexical_cast<> */
G_API_COMMON std::ostream &operator<<(std::ostream &, const Gem::Common::logType &);
/** @brief Reads a Gem::Common::logType from a stream. Needed also for boost::lexical_cast<> */
G_API_COMMON std::istream &operator>>(std::istream &, Gem::Common::logType &);

/******************************************************************************/
/**
 * The default number of bins in histograms, used in GPlotDesigner
 */
const std::size_t DEFAULTNBINSGPD = 100;

/******************************************************************************/
/**
 * Used in parameter definitions (GParserBuilder)
 */
const bool VAR_IS_ESSENTIAL = true;
const bool VAR_IS_SECONDARY = false;

/******************************************************************************/
/**
 * Needed for the serialization of boost::logic::tribool
 */
enum class triboolStates {
	TBS_FALSE
	, TBS_INDETERMINATE
	, TBS_TRUE
};

/******************************************************************************/
/** @brief Puts a Gem::Common::triboolStates into a stream. Needed also for boost::lexical_cast<> */
G_API_COMMON  std::ostream &operator<<(std::ostream &, const Gem::Common::triboolStates &);
/** @brief Reads a Gem::Common::triboolStates from a stream. Needed also for boost::lexical_cast<> */
G_API_COMMON std::istream &operator>>(std::istream &, Gem::Common::triboolStates &);

/******************************************************************************/
/**
 * The serialization modes that are currently allowed
 */
enum class serializationMode {
	SERIALIZATIONMODE_TEXT = 0
	, SERIALIZATIONMODE_XML = 1
	, SERIALIZATIONMODE_BINARY = 2
};

/******************************************************************************/

/** @brief Puts a Gem::Common::serializationMode into a stream. Needed also for boost::lexical_cast<> */
G_API_COMMON std::ostream &operator<<(std::ostream &, const Gem::Common::serializationMode &);
/** @brief Reads a Gem::Common::serializationMode item from a stream. Needed also for boost::lexical_cast<> */
G_API_COMMON std::istream &operator>>(std::istream &, Gem::Common::serializationMode &);

/******************************************************************************/
/**
 * Specification of whether checkExpectation should emit messages
 */
const bool CE_SILENT = false;
const bool CE_WITH_MESSAGES = true;

/******************************************************************************/
/**
 * Needed to express expectations in testing framework. CE stands for "Check expectation".
 */
enum class expectation {
	CE_EQUALITY // bitwise equality of all checked components
	, CE_FP_SIMILARITY // equality for non-floating point components, similarity for floating point
	, CE_INEQUALITY // at least one checked component differs
};

/******************************************************************************/

/** @brief Puts a Gem::Common::expectation into a stream. Needed also for boost::lexical_cast<> */
G_API_COMMON std::ostream &operator<<(std::ostream &, const Gem::Common::expectation &);
/** @brief Reads a Gem::Common::expectation item from a stream. Needed also for boost::lexical_cast<> */
G_API_COMMON std::istream &operator>>(std::istream &, Gem::Common::expectation &);

/******************************************************************************/
/**
 * Specification of the default number of threads, when no information about hardware
 * concurrency can be determined.
 */
const unsigned int DEFAULTNHARDWARETHREADS = 2;

/******************************************************************************/

} /* namespace Common */
} /* namespace Gem */

#endif /* GCOMMONENUMS_HPP_ */
