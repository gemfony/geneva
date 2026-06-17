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
     * Allowed specializations of Gem::Hap::GRandomT<T>
     */
enum class RANDFLAVOURS : Gem::Common::ENUMBASETYPE {
    RANDOMPROXY = 0 // random numbers are taken from the factory
        ,
    RANDOMLOCAL = 1
    // random numbers are produced locally, using a seed taken from the seed manager or provided to the constructor
};

/******************************************************************************/

/**
 * @brief Puts a Gem::Hap::RANDFLAVOURS into a stream. Needed for streaming / Gem::Common::fromString<>.
 *
 * @param os The output stream the flavour is written to
 * @param rf The RANDFLAVOURS value to serialize (written as its underlying integer)
 * @return A reference to the output stream (for chaining)
 */
std::ostream &operator<<(std::ostream &, const Gem::Hap::RANDFLAVOURS &);
/**
 * @brief Reads a Gem::Hap::RANDFLAVOURS item from a stream. Needed for streaming / Gem::Common::fromString<>.
 *
 * @param is The input stream the flavour is read from
 * @param rf The RANDFLAVOURS variable that receives the parsed value
 * @return A reference to the input stream (for chaining)
 */
std::istream &operator>>(std::istream &, Gem::Hap::RANDFLAVOURS &);

/******************************************************************************/
} /* namespace Gem::Hap */
