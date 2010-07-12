/**
 * @file GEnums.cpp
 */

/* Copyright (C) Dr. Ruediger Berlich and Karlsruhe Institute of Technology
 * (University of the State of Baden-Wuerttemberg and National Laboratory
 * of the Helmholtz Association)
 *
 * Contact: info [at] gemfony (dot) com
 *
 * This file is part of the Geneva library, Gemfony scientific's optimization
 * library.
 *
 * Geneva is free software: you can redistribute it and/or modify
 * it under the terms of version 3 of the GNU Affero General Public License
 * as published by the Free Software Foundation.
 *
 * Geneva is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with the Geneva library.  If not, see <http://www.gnu.org/licenses/>.
 *
 * For further information on Gemfony scientific and Geneva, visit
 * http://www.gemfony.com .
 */

#include "GOptimizationEnums.hpp"

namespace Gem {
namespace GenEvA {

/*********************************************************************/
/**
 * Puts a Gem::GenEvA::recoScheme item into a stream
 *
 * @param o The ostream the item should be added to
 * @param rc the item to be added to the stream
 * @return The std::ostream object used to add the item to
 */
std::ostream& operator<<(std::ostream& o, const Gem::GenEvA::recoScheme& rc) {
	boost::uint16_t tmp = static_cast<boost::uint16_t>(rc);
	o << tmp;
	return o;
}

/*********************************************************************/
/**
 * Reads a Gem::GenEvA::recoScheme item from a stream
 *
 * @param i The stream the item should be read from
 * @param rc The item read from the stream
 * @return The std::istream object used to read the item from
 */
std::istream& operator>>(std::istream& i, Gem::GenEvA::recoScheme& rc) {
	boost::uint16_t tmp;
	i >> tmp;

#ifdef DEBUG
	rc = boost::numeric_cast<Gem::GenEvA::recoScheme>(tmp);
#else
	rc = static_cast<Gem::GenEvA::recoScheme>(tmp);
#endif /* DEBUG */

	return i;
}

/*********************************************************************/
/**
 * Puts a Gem::GenEvA::infoMode item into a stream
 *
 * @param o The ostream the item should be added to
 * @param im the item to be added to the stream
 * @return The std::ostream object used to add the item to
 */
std::ostream& operator<<(std::ostream& o, const Gem::GenEvA::infoMode& im) {
	boost::uint16_t tmp = static_cast<boost::uint16_t>(im);
	o << tmp;
	return o;
}

/*********************************************************************/
/**
 * Reads a Gem::GenEvA::infoMode item from a stream
 *
 * @param i The stream the item should be read from
 * @param im The item read from the stream
 * @return The std::istream object used to read the item from
 */
std::istream& operator>>(std::istream& i, Gem::GenEvA::infoMode& im) {
	boost::uint16_t tmp;
	i >> tmp;

#ifdef DEBUG
	im = boost::numeric_cast<Gem::GenEvA::infoMode>(tmp);
#else
	im = static_cast<Gem::GenEvA::infoMode>(tmp);
#endif /* DEBUG */

	return i;
}

/*********************************************************************/
/**
 * Puts a Gem::GenEvA::adaptorId item into a stream
 *
 * @param o The ostream the item should be added to
 * @param aid the item to be added to the stream
 * @return The std::ostream object used to add the item to
 */
std::ostream& operator<<(std::ostream& o, const Gem::GenEvA::adaptorId& aid) {
	boost::uint16_t tmp = static_cast<boost::uint16_t>(aid);
	o << tmp;
	return o;
}

/*********************************************************************/
/**
 * Reads a Gem::GenEvA::adaptorId item from a stream
 *
 * @param i The stream the item should be read from
 * @param aid The item read from the stream
 * @return The std::istream object used to read the item from
 */
std::istream& operator>>(std::istream& i, Gem::GenEvA::adaptorId& aid) {
	boost::uint16_t tmp;
	i >> tmp;

#ifdef DEBUG
	aid = boost::numeric_cast<Gem::GenEvA::adaptorId>(tmp);
#else
	aid = static_cast<Gem::GenEvA::adaptorId>(tmp);
#endif /* DEBUG */

	return i;
}

/*********************************************************************/
/**
 * Puts a Gem::GenEvA::sortingMode item into a stream
 *
 * @param o The ostream the item should be added to
 * @param smode the item to be added to the stream
 * @return The std::ostream object used to add the item to
 */
std::ostream& operator<<(std::ostream& o, const Gem::GenEvA::sortingMode& smode) {
	boost::uint16_t tmp = static_cast<boost::uint16_t>(smode);
	o << tmp;
	return o;
}

/*********************************************************************/
/**
 * Reads a Gem::GenEvA::sortingMode item from a stream
 *
 * @param i The stream the item should be read from
 * @param smode The item read from the stream
 * @return The std::istream object used to read the item from
 */
std::istream& operator>>(std::istream& i, Gem::GenEvA::sortingMode& smode) {
	boost::uint16_t tmp;
	i >> tmp;

#ifdef DEBUG
	smode = boost::numeric_cast<Gem::GenEvA::sortingMode>(tmp);
#else
	smode = static_cast<Gem::GenEvA::sortingMode>(tmp);
#endif /* DEBUG */

	return i;
}

/*********************************************************************/
/**
 * Puts a Gem::GenEvA::personality item into a stream
 *
 * @param o The ostream the item should be added to
 * @param pers the item to be added to the stream
 * @return The std::ostream object used to add the item to
 */
std::ostream& operator<<(std::ostream& o, const Gem::GenEvA::personality& pers) {
	boost::uint16_t tmp = static_cast<boost::uint16_t>(pers);
	o << tmp;
	return o;
}

/*********************************************************************/
/**
 * Reads a Gem::GenEvA::personality item from a stream
 *
 * @param i The stream the item should be read from
 * @param pers The item read from the stream
 * @return The std::istream object used to read the item from
 */
std::istream& operator>>(std::istream& i, Gem::GenEvA::personality& pers) {
	boost::uint16_t tmp;
	i >> tmp;

#ifdef DEBUG
	pers = boost::numeric_cast<Gem::GenEvA::personality>(tmp);
#else
	pers = static_cast<Gem::GenEvA::personality>(tmp);
#endif /* DEBUG */

	return i;
}

/*********************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */
