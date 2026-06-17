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
 * See the NOTICE file in the top-level directory of the Geneva library
 * collection for a list of contributors and copyright information.
 *
 ********************************************************************************/
#include "geneva/GOptimizationEnums.hpp"
#include "common/GCommonEnums.hpp"
#include <istream>
#include <ostream>

namespace Gem::Geneva {

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
    Gem::Common::ENUMBASETYPE tmp = 0;
    i >> tmp;
    am = static_cast<Gem::Geneva::maxMode>(tmp);
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
    Gem::Common::ENUMBASETYPE tmp = 0;
    i >> tmp;
    am = static_cast<Gem::Geneva::activityMode>(tmp);
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
    Gem::Common::ENUMBASETYPE tmp = 0;
    i >> tmp;
    vccp = static_cast<Gem::Geneva::validityCheckCombinerPolicy>(tmp);
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
    Gem::Common::ENUMBASETYPE tmp = 0;
    i >> tmp;
    iip = static_cast<Gem::Geneva::evaluationPolicy>(tmp);
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
    Gem::Common::ENUMBASETYPE tmp = 0;
    i >> tmp;
    pm = static_cast<Gem::Geneva::execMode>(tmp);
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
    Gem::Common::ENUMBASETYPE tmp = 0;
    i >> tmp;
    rc = static_cast<Gem::Geneva::duplicationScheme>(tmp);
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
    Gem::Common::ENUMBASETYPE tmp = 0;
    i >> tmp;
    im = static_cast<Gem::Geneva::infoMode>(tmp);
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
    Gem::Common::ENUMBASETYPE tmp = 0;
    i >> tmp;
    aid = static_cast<Gem::Geneva::adaptorId>(tmp);
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
    Gem::Common::ENUMBASETYPE tmp = 0;
    i >> tmp;
    smode = static_cast<Gem::Geneva::sortingMode>(tmp);
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
    Gem::Common::ENUMBASETYPE tmp = 0;
    i >> tmp;
    smode = static_cast<Gem::Geneva::sortingModeMP>(tmp);
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
    Gem::Common::ENUMBASETYPE tmp = 0;
    i >> tmp;
    ur = static_cast<Gem::Geneva::updateRule>(tmp);
    return i;
}

/******************************************************************************/
/**
 * Puts a Gem::Geneva::adaptionMode into a stream. Needed also for boost::lexical_cast<>
 *
 * @param o The ostream the item should be added to
 * @param am The item to be added to the stream
 * @return The std::ostream object used to add the item to
 */
std::ostream &operator<<(std::ostream &o, const Gem::Geneva::adaptionMode &am) {
    Gem::Common::ENUMBASETYPE tmp = static_cast<Gem::Common::ENUMBASETYPE>(am);
    o << tmp;
    return o;
}

/******************************************************************************/
/**
 * Reads a Gem::Geneva::adaptionMode from a stream. Needed also for boost::lexical_cast<>
 *
 * @param i The stream the item should be read from
 * @param am The item read from the stream
 * @return The std::istream object used to read the item from
 */
std::istream &operator>>(std::istream &i, Gem::Geneva::adaptionMode &am) {
    Gem::Common::ENUMBASETYPE tmp = 0;
    i >> tmp;
    am = static_cast<Gem::Geneva::adaptionMode>(tmp);
    return i;
}

/******************************************************************************/

} /* namespace Gem::Geneva */
