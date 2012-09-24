/**
 * @file GOptimizationEnums.cpp
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
#include "geneva/GOptimizationEnums.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * Puts a Gem::Geneva::parMode item into a stream
 *
 * @param o The ostream the item should be added to
 * @param pm the item to be added to the stream
 * @return The std::ostream object used to add the item to
 */
std::ostream& operator<<(std::ostream& o, const Gem::Geneva::parMode& pm) {
	boost::uint16_t tmp = static_cast<boost::uint16_t>(pm);
	o << tmp;
	return o;
}

/******************************************************************************/
/**
 * Reads a Gem::Geneva::parMode item from a stream
 *
 * @param i The stream the item should be read from
 * @param pm The item read from the stream
 * @return The std::istream object used to read the item from
 */
std::istream& operator>>(std::istream& i, Gem::Geneva::parMode& pm) {
	boost::uint16_t tmp;
	i >> tmp;

#ifdef DEBUG
	pm = boost::numeric_cast<Gem::Geneva::parMode>(tmp);
#else
	pm = static_cast<Gem::Geneva::parMode>(tmp);
#endif /* DEBUG */

	return i;
}

/******************************************************************************/
/**
 * Puts a Gem::Geneva::duplicationScheme item into a stream
 *
 * @param o The ostream the item should be added to
 * @param rc the item to be added to the stream
 * @return The std::ostream object used to add the item to
 */
std::ostream& operator<<(std::ostream& o, const Gem::Geneva::duplicationScheme& rc) {
	boost::uint16_t tmp = static_cast<boost::uint16_t>(rc);
	o << tmp;
	return o;
}

/******************************************************************************/
/**
 * Reads a Gem::Geneva::duplicationScheme item from a stream
 *
 * @param i The stream the item should be read from
 * @param rc The item read from the stream
 * @return The std::istream object used to read the item from
 */
std::istream& operator>>(std::istream& i, Gem::Geneva::duplicationScheme& rc) {
	boost::uint16_t tmp;
	i >> tmp;

#ifdef DEBUG
	rc = boost::numeric_cast<Gem::Geneva::duplicationScheme>(tmp);
#else
	rc = static_cast<Gem::Geneva::duplicationScheme>(tmp);
#endif /* DEBUG */

	return i;
}

/******************************************************************************/
/**
 * Puts a Gem::Geneva::infoMode item into a stream
 *
 * @param o The ostream the item should be added to
 * @param im the item to be added to the stream
 * @return The std::ostream object used to add the item to
 */
std::ostream& operator<<(std::ostream& o, const Gem::Geneva::infoMode& im) {
	boost::uint16_t tmp = static_cast<boost::uint16_t>(im);
	o << tmp;
	return o;
}

/******************************************************************************/
/**
 * Reads a Gem::Geneva::infoMode item from a stream
 *
 * @param i The stream the item should be read from
 * @param im The item read from the stream
 * @return The std::istream object used to read the item from
 */
std::istream& operator>>(std::istream& i, Gem::Geneva::infoMode& im) {
	boost::uint16_t tmp;
	i >> tmp;

#ifdef DEBUG
	im = boost::numeric_cast<Gem::Geneva::infoMode>(tmp);
#else
	im = static_cast<Gem::Geneva::infoMode>(tmp);
#endif /* DEBUG */

	return i;
}

/******************************************************************************/
/**
 * Puts a Gem::Geneva::adaptorId item into a stream
 *
 * @param o The ostream the item should be added to
 * @param aid the item to be added to the stream
 * @return The std::ostream object used to add the item to
 */
std::ostream& operator<<(std::ostream& o, const Gem::Geneva::adaptorId& aid) {
	boost::uint16_t tmp = static_cast<boost::uint16_t>(aid);
	o << tmp;
	return o;
}

/******************************************************************************/
/**
 * Reads a Gem::Geneva::adaptorId item from a stream
 *
 * @param i The stream the item should be read from
 * @param aid The item read from the stream
 * @return The std::istream object used to read the item from
 */
std::istream& operator>>(std::istream& i, Gem::Geneva::adaptorId& aid) {
	boost::uint16_t tmp;
	i >> tmp;

#ifdef DEBUG
	aid = boost::numeric_cast<Gem::Geneva::adaptorId>(tmp);
#else
	aid = static_cast<Gem::Geneva::adaptorId>(tmp);
#endif /* DEBUG */

	return i;
}

/******************************************************************************/
/**
 * Puts a Gem::Geneva::sortingMode item into a stream
 *
 * @param o The ostream the item should be added to
 * @param smode the item to be added to the stream
 * @return The std::ostream object used to add the item to
 */
std::ostream& operator<<(std::ostream& o, const Gem::Geneva::sortingMode& smode) {
	boost::uint16_t tmp = static_cast<boost::uint16_t>(smode);
	o << tmp;
	return o;
}

/******************************************************************************/
/**
 * Reads a Gem::Geneva::sortingMode item from a stream
 *
 * @param i The stream the item should be read from
 * @param smode The item read from the stream
 * @return The std::istream object used to read the item from
 */
std::istream& operator>>(std::istream& i, Gem::Geneva::sortingMode& smode) {
	boost::uint16_t tmp;
	i >> tmp;

#ifdef DEBUG
	smode = boost::numeric_cast<Gem::Geneva::sortingMode>(tmp);
#else
	smode = static_cast<Gem::Geneva::sortingMode>(tmp);
#endif /* DEBUG */

	return i;
}

/******************************************************************************/
/**
 * Puts a Gem::Geneva::personality item into a stream
 *
 * @param o The ostream the item should be added to
 * @param pers the item to be added to the stream
 * @return The std::ostream object used to add the item to
 */
std::ostream& operator<<(std::ostream& o, const Gem::Geneva::personality_oa& pers) {
	boost::uint16_t tmp = static_cast<boost::uint16_t>(pers);
	o << tmp;
	return o;
}

/******************************************************************************/
/**
 * Reads a Gem::Geneva::personality item from a stream
 *
 * @param i The stream the item should be read from
 * @param pers The item read from the stream
 * @return The std::istream object used to read the item from
 */
std::istream& operator>>(std::istream& i, Gem::Geneva::personality_oa& pers) {
	boost::uint16_t tmp;
	i >> tmp;

#ifdef DEBUG
	pers = boost::numeric_cast<Gem::Geneva::personality_oa>(tmp);
#else
	pers = static_cast<Gem::Geneva::personality_oa>(tmp);
#endif /* DEBUG */

	return i;
}

/******************************************************************************/
/**
 * Puts a Gem::Geneva::updateRule item into a stream
 *
 * @param o The ostream the item should be added to
 * @param ur the item to be added to the stream
 * @return The std::ostream object used to add the item to
 */
std::ostream& operator<<(std::ostream& o, const Gem::Geneva::updateRule& ur) {
	boost::uint16_t tmp = static_cast<boost::uint16_t>(ur);
	o << tmp;
	return o;
}

/******************************************************************************/
/**
 * Reads a Gem::Geneva::updateRule item from a stream
 *
 * @param i The stream the item should be read from
 * @param ur The item read from the stream
 * @return The std::istream object used to read the item from
 */
std::istream& operator>>(std::istream& i, Gem::Geneva::updateRule& ur) {
	boost::uint16_t tmp;
	i >> tmp;

#ifdef DEBUG
	ur = boost::numeric_cast<Gem::Geneva::updateRule>(tmp);
#else
	ur = static_cast<Gem::Geneva::updateRule>(tmp);
#endif /* DEBUG */

	return i;
}

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */
