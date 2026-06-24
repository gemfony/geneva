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

#pragma once

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard headers go here
#include <iostream>
#include <string>

// Boost headers go here

// Geneva headers go here
#include "common/GCommonEnums.hpp"

namespace Gem::Hap {
/******************************************************************************/
// For bi_normal_distribution

constexpr double DEF_BINORM_MEAN = 0.;
constexpr double DEF_BINORM_SIGMA1 = 0.1;
constexpr double DEF_BINORM_SIGMA2 = 0.1;
constexpr double DEF_BINORM_DISTANCE = 0.5;

/******************************************************************************/
/**
 * The source strategy a Gem::Hap::GRandomT<source> proxy uses to obtain random numbers.
 *
 * Renamed from the former RANDFLAVOURS as the set of sources grows: QUEUE was RANDOMPROXY,
 * LOCAL was RANDOMLOCAL. The underlying integer values are preserved (QUEUE=0, LOCAL=1) so
 * any streamed/serialized values are unaffected. STAGED=2 is a new source (no legacy value).
 */
enum class randomSource : Gem::Common::ENUMBASETYPE {
    QUEUE = 0, ///< numbers are taken from the central factory (the package queue) -- the default
    LOCAL = 1, ///< numbers are produced locally, from a per-proxy engine seeded by the factory
    STAGED = 2 ///< numbers are claimed in chunks from a shared, bulk-filled staging pool into a per-proxy double buffer
};

/******************************************************************************/

/**
 * @brief Puts a Gem::Hap::randomSource into a stream. Needed for streaming / Gem::Common::fromString<>.
 *
 * @param o The output stream the source is written to
 * @param grts The randomSource value to serialize (written as its underlying integer)
 * @return A reference to the output stream (for chaining)
 */
std::ostream &operator<<(std::ostream &o, const Gem::Hap::randomSource &grts);
/**
 * @brief Reads a Gem::Hap::randomSource item from a stream. Needed for streaming / Gem::Common::fromString<>.
 *
 * @param i The input stream the source is read from
 * @param grts The randomSource variable that receives the parsed value
 * @return A reference to the input stream (for chaining)
 */
std::istream &operator>>(std::istream &i, Gem::Hap::randomSource &grts);

/******************************************************************************/
} /* namespace Gem::Hap */
