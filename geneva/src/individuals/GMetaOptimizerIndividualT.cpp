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

#include "geneva/individuals/GMetaOptimizerIndividualT.hpp"
#include "common/GCommonEnums.hpp"
#include "common/GCommonMathHelperFunctionsT.hpp"
#include "geneva/individuals/GFunctionIndividual.hpp"
#include <istream>
#include <ostream>

BOOST_CLASS_EXPORT_IMPLEMENT(
    Gem::Geneva::Individuals::GMetaOptimizerIndividualT<Gem::Geneva::Individuals::GFunctionIndividual>
) // NOLINT

BOOST_CLASS_EXPORT_IMPLEMENT(
    Gem::Geneva::Individuals::GOptOptMonitorT<Gem::Geneva::Individuals::GFunctionIndividual>
) // NOLINT
namespace Gem::Geneva::Individuals {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * @brief Puts a Gem::Geneva::Individuals::metaOptimizationTarget item into a stream.
 *
 * @param o The ostream the item should be added to
 * @param mot The metaOptimizationTarget item to be added to the stream
 * @return The std::ostream object used to add the item to
 */
std::ostream &operator<<(std::ostream &o, const Gem::Geneva::Individuals::metaOptimizationTarget &mot) {
    Gem::Common::ENUMBASETYPE tmp = static_cast<Gem::Common::ENUMBASETYPE>(mot);
    o << tmp;
    return o;
}

/******************************************************************************/
/**
 * @brief Reads a Gem::Geneva::Individuals::metaOptimizationTarget item from a stream.
 *
 * @param i The istream the item should be read from
 * @param mot The metaOptimizationTarget item read from the stream (output parameter)
 * @return The std::istream object used to read the item from
 */
std::istream &operator>>(std::istream &i, Gem::Geneva::Individuals::metaOptimizationTarget &mot) {
    Gem::Common::ENUMBASETYPE tmp = 0;
    i >> tmp;

#ifdef DEBUG
    mot = Gem::Common::narrow<Gem::Geneva::Individuals::metaOptimizationTarget>(tmp);
#else
    mot = static_cast<Gem::Geneva::Individuals::metaOptimizationTarget>(tmp);
#endif /* DEBUG */

    return i;
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Gem::Geneva::Individuals */
