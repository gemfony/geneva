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

#include "common/GCommonEnums.hpp"
#include <istream>
#include <ostream>
#include <string>
#include <utility>

namespace Gem::Common {

/******************************************************************************/
/**
 * Puts a Gem::Common::parameter_source into a stream. Needed for streaming / Gem::Common::fromString<>
 *
 * @param o The output stream to write to
 * @param x The parameter_source value to write
 * @return The output stream, to allow chaining
 */
std::ostream &operator<<(std::ostream &o, Gem::Common::parameter_source const &x) {
    o << std::to_underlying(x);
    return o;
}

/******************************************************************************/
/**
 * Reads a Gem::Common::parameter_source item from a stream. Needed for streaming / Gem::Common::fromString<>
 *
 * @param i The input stream to read from
 * @param x Output parameter: receives the parameter_source value read from the stream
 * @return The input stream, to allow chaining
 */
std::istream &operator>>(std::istream &i, Gem::Common::parameter_source &x) {
    Gem::Common::ENUMBASETYPE tmp = 0;
    i >> tmp;
    x = static_cast<Gem::Common::parameter_source>(tmp);
    return i;
}

/******************************************************************************/
/**
 * Puts a Gem::Common::sortOrder into a stream. Needed for streaming / Gem::Common::fromString<>
 *
 * @param o The output stream to write to
 * @param x The sortOrder value to write
 * @return The output stream, to allow chaining
 */
std::ostream &operator<<(std::ostream &o, Gem::Common::sortOrder const &x) {
    o << std::to_underlying(x);
    return o;
}

/******************************************************************************/
/**
 * Reads a Gem::Common::sortOrder item from a stream. Needed for streaming / Gem::Common::fromString<>
 *
 * @param i The input stream to read from
 * @param x Output parameter: receives the sortOrder value read from the stream
 * @return The input stream, to allow chaining
 */
std::istream &operator>>(std::istream &i, Gem::Common::sortOrder &x) {
    Gem::Common::ENUMBASETYPE tmp = 0;
    i >> tmp;
    x = static_cast<Gem::Common::sortOrder>(tmp);
    return i;
}

/******************************************************************************/
/**
 * Puts a Gem::Common::dimensions into a stream. Needed for streaming / Gem::Common::fromString<>
 *
 * @param o The output stream to write to
 * @param x The dimensions value to write
 * @return The output stream, to allow chaining
 */
std::ostream &operator<<(std::ostream &o, Gem::Common::dimensions const &x) {
    o << std::to_underlying(x);
    return o;
}

/******************************************************************************/
/**
 * Reads a Gem::Common::dimensions item from a stream. Needed for streaming / Gem::Common::fromString<>
 *
 * @param i The input stream to read from
 * @param x Output parameter: receives the dimensions value read from the stream
 * @return The input stream, to allow chaining
 */
std::istream &operator>>(std::istream &i, Gem::Common::dimensions &x) {
    Gem::Common::ENUMBASETYPE tmp = 0;
    i >> tmp;
    x = static_cast<Gem::Common::dimensions>(tmp);
    return i;
}

/******************************************************************************/
/**
 * Puts a Gem::Common::logType into a stream. Needed for streaming / Gem::Common::fromString<>
 *
 * @param o The output stream to write to
 * @param x The logType value to write
 * @return The output stream, to allow chaining
 */
std::ostream &operator<<(std::ostream &o, Gem::Common::logType const &x) {
    o << std::to_underlying(x);
    return o;
}

/******************************************************************************/
/**
 * Reads a Gem::Common::logType item from a stream. Needed for streaming / Gem::Common::fromString<>
 *
 * @param i The input stream to read from
 * @param x Output parameter: receives the logType value read from the stream
 * @return The input stream, to allow chaining
 */
std::istream &operator>>(std::istream &i, Gem::Common::logType &x) {
    Gem::Common::ENUMBASETYPE tmp = 0;
    i >> tmp;
    x = static_cast<Gem::Common::logType>(tmp);
    return i;
}

/******************************************************************************/
/**
 * Puts a Gem::Common::tribool into a stream
 *
 * @param o The output stream to write to
 * @param x The tribool value to write (True / False / Indeterminate)
 * @return The output stream, to allow chaining
 */
std::ostream &operator<<(std::ostream &o, Gem::Common::tribool const &x) {
    switch(x) {
    case Gem::Common::tribool::True:          o << "True";          break;
    case Gem::Common::tribool::False:         o << "False";         break;
    case Gem::Common::tribool::Indeterminate: o << "Indeterminate"; break;
    default:
        // Guard against corrupted-stream deserialisation (>> casts any
        // ENUMBASETYPE to tribool without validation). Without this arm
        // the stream would be left untouched, silently producing empty
        // output instead of a diagnostic.
        o << "tribool::?(" << std::to_underlying(x) << ")";
        break;
    }
    return o;
}

/******************************************************************************/
/**
 * Puts a Gem::Common::triboolStates into a stream. Needed for streaming / Gem::Common::fromString<>
 *
 * @param o The output stream to write to
 * @param x The triboolStates value to write
 * @return The output stream, to allow chaining
 */
std::ostream &operator<<(std::ostream &o, Gem::Common::triboolStates const &x) {
    o << std::to_underlying(x);
    return o;
}

/******************************************************************************/
/**
 * Reads a Gem::Common::triboolStates item from a stream. Needed for streaming / Gem::Common::fromString<>
 *
 * @param i The input stream to read from
 * @param x Output parameter: receives the triboolStates value read from the stream
 * @return The input stream, to allow chaining
 */
std::istream &operator>>(std::istream &i, Gem::Common::triboolStates &x) {
    Gem::Common::ENUMBASETYPE tmp = 0;
    i >> tmp;
    x = static_cast<Gem::Common::triboolStates>(tmp);
    return i;
}

/******************************************************************************/
/**
 * Puts a Gem::Common::serializationMode into a stream. Needed for streaming / Gem::Common::fromString<>
 *
 * @param o The output stream to write to
 * @param x The serializationMode value to write
 * @return The output stream, to allow chaining
 */
std::ostream &operator<<(std::ostream &o, Gem::Common::serializationMode const &x) {
    o << std::to_underlying(x);
    return o;
}

/******************************************************************************/
/**
 * Reads a Gem::Common::serializationMode item from a stream. Needed for streaming / Gem::Common::fromString<>
 *
 * @param i The input stream to read from
 * @param x Output parameter: receives the serializationMode value read from the stream
 * @return The input stream, to allow chaining
 */
std::istream &operator>>(std::istream &i, Gem::Common::serializationMode &x) {
    Gem::Common::ENUMBASETYPE tmp = 0;
    i >> tmp;
    x = static_cast<Gem::Common::serializationMode>(tmp);
    return i;
}

/******************************************************************************/
/**
 * Converts a serializationMode to a string representation for debugging purposes
 *
 * @param ser_mod The serialization mode to convert
 * @return A string representation of the mode ("TEXT", "XML", "BINARY", or "unknown")
 */
std::string serModeToString(Gem::Common::serializationMode ser_mod) {
    switch(ser_mod) {
    case Gem::Common::serializationMode::TEXT:
        return "TEXT";
    case Gem::Common::serializationMode::XML:
        return "XML";
    case Gem::Common::serializationMode::BINARY:
        return "BINARY";
    default:
        return "unknown";
    }
}

/******************************************************************************/
/**
 * Puts a Gem::Common::expectation into a stream. Needed for streaming / Gem::Common::fromString<>
 *
 * @param o The output stream to write to
 * @param x The expectation value to write
 * @return The output stream, to allow chaining
 */
std::ostream &operator<<(std::ostream &o, Gem::Common::expectation const &x) {
    o << std::to_underlying(x);
    return o;
}

/******************************************************************************/
/**
 * Reads a Gem::Common::expectation item from a stream. Needed for streaming / Gem::Common::fromString<>
 *
 * @param i The input stream to read from
 * @param x Output parameter: receives the expectation value read from the stream
 * @return The input stream, to allow chaining
 */
std::istream &operator>>(std::istream &i, Gem::Common::expectation &x) {
    Gem::Common::ENUMBASETYPE tmp = 0;
    i >> tmp;
    x = static_cast<Gem::Common::expectation>(tmp);
    return i;
}

/******************************************************************************/

} /* namespace Gem::Common */
