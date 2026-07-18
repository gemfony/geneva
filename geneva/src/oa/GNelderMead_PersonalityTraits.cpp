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
#include "geneva/oa/GNelderMead_PersonalityTraits.hpp"
#include <string>

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::OptimizationAlgorithms::GNelderMead_PersonalityTraits) // NOLINT
namespace Gem::Geneva::OptimizationAlgorithms {

/******************************************************************************/
/** A short identifier suitable for storage in a std::map */
const std::string GNelderMead_PersonalityTraits::nickname = "nm";

/******************************************************************************/
/**
 * @brief Retrieves the mnemonic of the optimization algorithm.
 *
 * @return The short identifier ("nm") of the Nelder-Mead algorithm
 */
std::string GNelderMead_PersonalityTraits::getMnemonic() const {
    return GNelderMead_PersonalityTraits::nickname;
}

/******************************************************************************/
/**
 * @brief Emits a name for this class / object.
 *
 * @return The string "GNelderMead_PersonalityTraits"
 */
std::string GNelderMead_PersonalityTraits::name_() const {
    return std::string("GNelderMead_PersonalityTraits");
}

/******************************************************************************/
/**
 * @brief Creates a deep clone of this object.
 *
 * @return A pointer to a newly allocated, deep copy of this object (caller takes ownership)
 */
GPersonalityTraits *GNelderMead_PersonalityTraits::clone_() const {
    return new GNelderMead_PersonalityTraits(*this);
}

/******************************************************************************/

} /* namespace Gem::Geneva::OptimizationAlgorithms */
