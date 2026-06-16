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
#include <memory>
#include <sstream>
#include <tuple>
#include <type_traits>
#include <vector>

// Boost headers go here

// Geneva headers go here
#include "courtier/GCourtierEnums.hpp"
#include "geneva/ind/GOptimizableEntity.hpp"

namespace Gem::Geneva {

/******************************************************************************/
/** @brief Transforms the individual fitness so that the optimization algorithm always "sees" a minimization problem */
double
minOnly_transformed_fitness(const gen::GOptimizableEntity &, std::size_t = 0);

/******************************************************************************/
/** @brief Checks whether the first individual is better than the second */
bool
isBetter(const std::shared_ptr<gen::GOptimizableEntity> &x_ptr, const std::shared_ptr<gen::GOptimizableEntity> &y_ptr);

/******************************************************************************/
/** @brief Checks whether the first individual is worse than the second */
bool
isWorse(const std::shared_ptr<gen::GOptimizableEntity> &x_ptr, const std::shared_ptr<gen::GOptimizableEntity> &y_ptr);

/******************************************************************************/
/** @brief Checks whether the first value is better than the second */
bool isBetter(double x, double y, maxMode m);

/******************************************************************************/
/** @brief Checks whether the first value is worse than the second*/
bool isWorse(double x, double y, maxMode m);

/******************************************************************************/

} /* namespace Gem::Geneva */
