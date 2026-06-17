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

#include "hap/GHapEnums.hpp"
#include "common/GCommonEnums.hpp"
#include <istream>
#include <ostream>

namespace Gem::Hap {

/******************************************************************************/
/**
 * @brief Puts a Gem::Hap::RANDFLAVOURS item into a stream.
 *
 * The enumerator is first cast to its underlying integral base type
 * (Gem::Common::ENUMBASETYPE) and then written, so the textual form is a
 * plain integer.
 *
 * @param o The std::ostream the item should be added to
 * @param grts The RANDFLAVOURS enumerator to be written to the stream
 * @return The std::ostream object passed in (to allow chaining)
 */
std::ostream &operator<<(std::ostream &o, const Gem::Hap::RANDFLAVOURS &grts) {
    Gem::Common::ENUMBASETYPE tmp = static_cast<Gem::Common::ENUMBASETYPE>(grts);
    o << tmp;
    return o;
}

/******************************************************************************/
/**
 * @brief Reads a Gem::Hap::RANDFLAVOURS item from a stream.
 *
 * An integer is read into the underlying base type
 * (Gem::Common::ENUMBASETYPE) and then cast back into the enumerator.
 *
 * @param i The std::istream the item should be read from
 * @param grts Output reference receiving the RANDFLAVOURS enumerator read from the stream
 * @return The std::istream object passed in (to allow chaining)
 */
std::istream &operator>>(std::istream &i, Gem::Hap::RANDFLAVOURS &grts) {
    Gem::Common::ENUMBASETYPE tmp = 0;
    i >> tmp;
    grts = static_cast<Gem::Hap::RANDFLAVOURS>(tmp);
    return i;
}

/******************************************************************************/

} /* namespace Gem::Hap */
