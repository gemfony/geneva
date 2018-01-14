/**
 * @file GOptimizationEnums.cpp
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
#include "geneva/GOptimizationEnums.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * Puts a Gem::Geneva::maxMode item into a stream
 *
 * @param o The ostream the item should be added to
 * @param am the item to be added to the stream
 * @return The std::ostream object used to add the item to
 */
std::ostream &operator<<(std::ostream &o, const Gem::Geneva::maxMode &am) {
	Gem::Common::ENUMBASETYPE tmp = static_cast<Gem::Common::ENUMBASETYPE>(am);
	o << tmp;
	return o;
}

/******************************************************************************/
/**
 * Reads a Gem::Geneva::maxMode item from a stream
 *
 * @param i The stream the item should be read from
 * @param am The item read from the stream
 * @return The std::istream object used to read the item from
 */
std::istream &operator>>(std::istream &i, Gem::Geneva::maxMode &am) {
	Gem::Common::ENUMBASETYPE tmp;
	i >> tmp;

#ifdef DEBUG
	am = boost::numeric_cast<Gem::Geneva::maxMode>(tmp);
#else
	am = static_cast<Gem::Geneva::maxMode>(tmp);
#endif /* DEBUG */

	return i;
}

/******************************************************************************/
/**
 * Puts a Gem::Geneva::activityMode item into a stream
 *
 * @param o The ostream the item should be added to
 * @param am the item to be added to the stream
 * @return The std::ostream object used to add the item to
 */
std::ostream &operator<<(std::ostream &o, const Gem::Geneva::activityMode &am) {
	Gem::Common::ENUMBASETYPE tmp = static_cast<Gem::Common::ENUMBASETYPE>(am);
	o << tmp;
	return o;
}

/******************************************************************************/
/**
 * Reads a Gem::Geneva::activityMode item from a stream
 *
 * @param i The stream the item should be read from
 * @param am The item read from the stream
 * @return The std::istream object used to read the item from
 */
std::istream &operator>>(std::istream &i, Gem::Geneva::activityMode &am) {
	Gem::Common::ENUMBASETYPE tmp;
	i >> tmp;

#ifdef DEBUG
   am = boost::numeric_cast<Gem::Geneva::activityMode>(tmp);
#else
	am = static_cast<Gem::Geneva::activityMode>(tmp);
#endif /* DEBUG */

	return i;
}

/******************************************************************************/
/**
 * Puts a Gem::Geneva::validityCheckCombinerPolicy item into a stream
 *
 * @param o The ostream the item should be added to
 * @param vccp the item to be added to the stream
 * @return The std::ostream object used to add the item to
 */
std::ostream &operator<<(std::ostream &o, const Gem::Geneva::validityCheckCombinerPolicy &vccp) {
	Gem::Common::ENUMBASETYPE tmp = static_cast<Gem::Common::ENUMBASETYPE>(vccp);
	o << tmp;
	return o;
}

/******************************************************************************/
/**
 * Reads a Gem::Geneva::validityCheckCombinerPolicy item from a stream
 *
 * @param i The stream the item should be read from
 * @param vccp The item read from the stream
 * @return The std::istream object used to read the item from
 */
std::istream &operator>>(std::istream &i, Gem::Geneva::validityCheckCombinerPolicy &vccp) {
	Gem::Common::ENUMBASETYPE tmp;
	i >> tmp;

#ifdef DEBUG
   vccp = boost::numeric_cast<Gem::Geneva::validityCheckCombinerPolicy>(tmp);
#else
	vccp = static_cast<Gem::Geneva::validityCheckCombinerPolicy>(tmp);
#endif /* DEBUG */

	return i;
}

/******************************************************************************/
/**
 * Puts a Gem::Geneva::evaluationPolicy item into a stream
 *
 * @param o The ostream the item should be added to
 * @param iip the item to be added to the stream
 * @return The std::ostream object used to add the item to
 */
std::ostream &operator<<(std::ostream &o, const Gem::Geneva::evaluationPolicy &iip) {
	Gem::Common::ENUMBASETYPE tmp = static_cast<Gem::Common::ENUMBASETYPE>(iip);
	o << tmp;
	return o;
}

/******************************************************************************/
/**
 * Reads a Gem::Geneva::evaluationPolicy item from a stream
 *
 * @param i The stream the item should be read from
 * @param iip The item read from the stream
 * @return The std::istream object used to read the item from
 */
std::istream &operator>>(std::istream &i, Gem::Geneva::evaluationPolicy &iip) {
	Gem::Common::ENUMBASETYPE tmp;
	i >> tmp;

#ifdef DEBUG
   iip = boost::numeric_cast<Gem::Geneva::evaluationPolicy>(tmp);
#else
	iip = static_cast<Gem::Geneva::evaluationPolicy>(tmp);
#endif /* DEBUG */

	return i;
}

/******************************************************************************/
/**
 * Puts a Gem::Geneva::execMode item into a stream
 *
 * @param o The ostream the item should be added to
 * @param pm the item to be added to the stream
 * @return The std::ostream object used to add the item to
 */
std::ostream &operator<<(std::ostream &o, const Gem::Geneva::execMode &pm) {
	Gem::Common::ENUMBASETYPE tmp = static_cast<Gem::Common::ENUMBASETYPE>(pm);
	o << tmp;
	return o;
}

/******************************************************************************/
/**
 * Reads a Gem::Geneva::execMode item from a stream
 *
 * @param i The stream the item should be read from
 * @param pm The item read from the stream
 * @return The std::istream object used to read the item from
 */
std::istream &operator>>(std::istream &i, Gem::Geneva::execMode &pm) {
	Gem::Common::ENUMBASETYPE tmp;
	i >> tmp;

#ifdef DEBUG
	pm = boost::numeric_cast<Gem::Geneva::execMode>(tmp);
#else
	pm = static_cast<Gem::Geneva::execMode>(tmp);
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
std::ostream &operator<<(std::ostream &o, const Gem::Geneva::duplicationScheme &rc) {
	Gem::Common::ENUMBASETYPE tmp = static_cast<Gem::Common::ENUMBASETYPE>(rc);
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
std::istream &operator>>(std::istream &i, Gem::Geneva::duplicationScheme &rc) {
	Gem::Common::ENUMBASETYPE tmp;
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
std::ostream &operator<<(std::ostream &o, const Gem::Geneva::infoMode &im) {
	Gem::Common::ENUMBASETYPE tmp = static_cast<Gem::Common::ENUMBASETYPE>(im);
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
std::istream &operator>>(std::istream &i, Gem::Geneva::infoMode &im) {
	Gem::Common::ENUMBASETYPE tmp;
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
std::ostream &operator<<(std::ostream &o, const Gem::Geneva::adaptorId &aid) {
	Gem::Common::ENUMBASETYPE tmp = static_cast<Gem::Common::ENUMBASETYPE>(aid);
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
std::istream &operator>>(std::istream &i, Gem::Geneva::adaptorId &aid) {
	Gem::Common::ENUMBASETYPE tmp;
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
std::ostream &operator<<(std::ostream &o, const Gem::Geneva::sortingMode &smode) {
	Gem::Common::ENUMBASETYPE tmp = static_cast<Gem::Common::ENUMBASETYPE>(smode);
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
std::istream &operator>>(std::istream &i, Gem::Geneva::sortingMode &smode) {
	Gem::Common::ENUMBASETYPE tmp;
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
 * Puts a Gem::Geneva::sortingModeMP item into a stream
 *
 * @param o The ostream the item should be added to
 * @param smode the item to be added to the stream
 * @return The std::ostream object used to add the item to
 */
std::ostream &operator<<(std::ostream &o, const Gem::Geneva::sortingModeMP &smode) {
	Gem::Common::ENUMBASETYPE tmp = static_cast<Gem::Common::ENUMBASETYPE>(smode);
	o << tmp;
	return o;
}

/******************************************************************************/
/**
 * Reads a Gem::Geneva::sortingModeMP item from a stream
 *
 * @param i The stream the item should be read from
 * @param smode The item read from the stream
 * @return The std::istream object used to read the item from
 */
std::istream &operator>>(std::istream &i, Gem::Geneva::sortingModeMP &smode) {
	Gem::Common::ENUMBASETYPE tmp;
	i >> tmp;

#ifdef DEBUG
   smode = boost::numeric_cast<Gem::Geneva::sortingModeMP>(tmp);
#else
	smode = static_cast<Gem::Geneva::sortingModeMP>(tmp);
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
std::ostream &operator<<(std::ostream &o, const Gem::Geneva::updateRule &ur) {
	Gem::Common::ENUMBASETYPE tmp = static_cast<Gem::Common::ENUMBASETYPE>(ur);
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
std::istream &operator>>(std::istream &i, Gem::Geneva::updateRule &ur) {
	Gem::Common::ENUMBASETYPE tmp;
	i >> tmp;

#ifdef DEBUG
	ur = boost::numeric_cast<Gem::Geneva::updateRule>(tmp);
#else
	ur = static_cast<Gem::Geneva::updateRule>(tmp);
#endif /* DEBUG */

	return i;
}

/******************************************************************************/
/**
 * Puts a Gem::Geneva::adaptionMode into a stream. Needed also for boost::lexical_cast<>
 */
std::ostream& operator<<(std::ostream& o, const Gem::Geneva::adaptionMode& am) {
	Gem::Common::ENUMBASETYPE tmp = static_cast<Gem::Common::ENUMBASETYPE>(am);
	o << tmp;
	return o;
}

/******************************************************************************/
/**
 * Reads a Gem::Geneva::adaptionMode from a stream. Needed also for boost::lexical_cast<>
 */
std::istream& operator>>(std::istream& i, Gem::Geneva::adaptionMode& am) {
	Gem::Common::ENUMBASETYPE tmp;
	i >> tmp;

#ifdef DEBUG
	am = boost::numeric_cast<Gem::Geneva::adaptionMode>(tmp);
#else
	am = static_cast<Gem::Geneva::adaptionMode>(tmp);
#endif /* DEBUG */

	return i;
}

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */
