/**
 * @file
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

#include "common/GCommonEnums.hpp"

namespace Gem {
namespace Common {

/******************************************************************************/
/**
 * Puts a Gem::Common::parameter_source into a stream. Needed also for boost::lexical_cast<>
 */
std::ostream &operator<<(std::ostream &o, Gem::Common::parameter_source const &x) {
	o << static_cast<Gem::Common::ENUMBASETYPE>(x);
	return o;
}

/******************************************************************************/
/**
 * Reads a Gem::Common::parameter_source item from a stream. Needed also for boost::lexical_cast<>
 */
std::istream &operator>>(std::istream &i, Gem::Common::parameter_source &x) {
	Gem::Common::ENUMBASETYPE tmp;
	i >> tmp;

#ifdef DEBUG
	x = boost::numeric_cast<Gem::Common::parameter_source>(tmp);
#else
	x = static_cast<Gem::Common::parameter_source>(tmp);
#endif /* DEBUG */

	return i;
}

/******************************************************************************/
/**
 * Puts a Gem::Common::sortOder into a stream. Needed also for boost::lexical_cast<>
 */
std::ostream &operator<<(std::ostream &o, Gem::Common::sortOrder const &x) {
	o << static_cast<Gem::Common::ENUMBASETYPE>(x);
	return o;
}

/******************************************************************************/
/**
 * Reads a Gem::Common::sortOder item from a stream. Needed also for boost::lexical_cast<>
 */
std::istream &operator>>(std::istream &i, Gem::Common::sortOrder &x) {
	Gem::Common::ENUMBASETYPE tmp;
	i >> tmp;

#ifdef DEBUG
	x = boost::numeric_cast<Gem::Common::sortOrder>(tmp);
#else
	x = static_cast<Gem::Common::sortOrder>(tmp);
#endif /* DEBUG */

	return i;
}

/******************************************************************************/
/**
 * Reads a Gem::Common::dimensions item from a stream. Needed also for boost::lexical_cast<>
 */
std::istream &operator>>(std::istream &i, Gem::Common::dimensions &x) {
	Gem::Common::ENUMBASETYPE tmp;
	i >> tmp;

#ifdef DEBUG
	x = boost::numeric_cast<Gem::Common::dimensions>(tmp);
#else
	x = static_cast<Gem::Common::dimensions>(tmp);
#endif /* DEBUG */

	return i;
}

/******************************************************************************/
/**
 * Puts a Gem::Common::logType into a stream. Needed also for boost::lexical_cast<>
 */
std::ostream &operator<<(std::ostream &o, Gem::Common::logType const &x) {
	o << static_cast<Gem::Common::ENUMBASETYPE>(x);
	return o;
}

/******************************************************************************/
/**
 * Reads a Gem::Common::logType item from a stream. Needed also for boost::lexical_cast<>
 */
std::istream &operator>>(std::istream &i, Gem::Common::logType &x) {
	Gem::Common::ENUMBASETYPE tmp;
	i >> tmp;

#ifdef DEBUG
	x = boost::numeric_cast<Gem::Common::logType>(tmp);
#else
	x = static_cast<Gem::Common::logType>(tmp);
#endif /* DEBUG */

	return i;
}

/******************************************************************************/
/**
 * Puts a Gem::Common::triboolStates into a stream. Needed also for boost::lexical_cast<>
 */
std::ostream &operator<<(std::ostream &o, Gem::Common::triboolStates const &x) {
	o << static_cast<Gem::Common::ENUMBASETYPE>(x);
	return o;
}

/******************************************************************************/
/**
 * Reads a Gem::Common::triboolStates item from a stream. Needed also for boost::lexical_cast<>
 */
std::istream &operator>>(std::istream &i, Gem::Common::triboolStates &x) {
	Gem::Common::ENUMBASETYPE tmp;
	i >> tmp;

#ifdef DEBUG
	x = boost::numeric_cast<Gem::Common::triboolStates>(tmp);
#else
	x = static_cast<Gem::Common::triboolStates>(tmp);
#endif /* DEBUG */

	return i;
}

/******************************************************************************/
/**
 * Puts a Gem::Common::serializationMode into a stream. Needed also for boost::lexical_cast<>
 */
std::ostream &operator<<(std::ostream &o, Gem::Common::serializationMode const &x) {
	o << static_cast<Gem::Common::ENUMBASETYPE>(x);
	return o;
}

/******************************************************************************/
/**
 * Reads a Gem::Common::serializationMode item from a stream. Needed also for boost::lexical_cast<>
 */
std::istream &operator>>(std::istream &i, Gem::Common::serializationMode &x) {
	Gem::Common::ENUMBASETYPE tmp;
	i >> tmp;

#ifdef DEBUG
	x = boost::numeric_cast<Gem::Common::serializationMode>(tmp);
#else
	x = static_cast<Gem::Common::serializationMode>(tmp);
#endif /* DEBUG */

	return i;
}

/******************************************************************************/
/**
 * Converts a serializationMode to a string representation for debugging purposes
 */
std::string serModeToString(Gem::Common::serializationMode serMod) {
	switch(serMod) {
		case Gem::Common::serializationMode::TEXT:
			return "TEXT";
			break;
		case Gem::Common::serializationMode::XML:
			return "XML";
			break;
		case Gem::Common::serializationMode::BINARY:
			return "BINARY";
			break;
	    default:
            return "unkown";
            break;
	}
}

/******************************************************************************/
/**
 * Puts a Gem::Common::expectation into a stream. Needed also for boost::lexical_cast<> *
 */
std::ostream &operator<<(std::ostream &o, Gem::Common::expectation const &x) {
	o << static_cast<Gem::Common::ENUMBASETYPE>(x);
	return o;
}

/******************************************************************************/
/**
 * Reads a Gem::Common::expectation item from a stream. Needed also for boost::lexical_cast<>
 */
std::istream &operator>>(std::istream &i, Gem::Common::expectation &x) {
	Gem::Common::ENUMBASETYPE tmp;
	i >> tmp;

#ifdef DEBUG
	x = boost::numeric_cast<Gem::Common::expectation>(tmp);
#else
	x = static_cast<Gem::Common::expectation>(tmp);
#endif /* DEBUG */

	return i;
}

/******************************************************************************/

} /* namespace Common */
} /* namspace Gem */

