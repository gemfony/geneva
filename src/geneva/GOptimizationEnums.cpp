/********************************************************************************
 *
 * This file is part of the Geneva library collection. The following license
 * applies to this file:
 *
 * ------------------------------------------------------------------------------
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * ------------------------------------------------------------------------------
 *
 * Note that other files in the Geneva library collection may use a different
 * license. Please see the licensing information in each file.
 *
 ********************************************************************************
 *
 * Geneva was started by Dr. Rüdiger Berlich and was later maintained together
 * with Dr. Ariel Garcia under the auspices of Gemfony scientific. For further
 * information on Gemfony scientific, see http://www.gemfomy.eu .
 *
 * The majority of files in Geneva was released under the Apache license v2.0
 * in February 2020.
 *
 * See the NOTICE file in the top-level directory of the Geneva library
 * collection for a list of contributors and copyright information.
 *
 ********************************************************************************/
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
