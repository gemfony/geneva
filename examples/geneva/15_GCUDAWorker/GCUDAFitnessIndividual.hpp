/**
* @file GCUDAFitnessIndividual.hpp
 */

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

// Includes check for correct Boost version(s)
#include "common/GGlobalDefines.hpp"

// Standard header files go here
#include <algorithm> // for std::sort
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <iterator>
#include <memory>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility> // For std::pair
#include <vector>

// Geneva header files go here
#include "geneva-individuals/GFunctionIndividual.hpp"

namespace Gem::Geneva {

/******************************************************************************/
/**
 * This individual searches for the optima of a number of different
 * multidimensional mathematical functions defined in its parent class.
 * Instead of performing the evaluation on the CPU, the individual uses
 * a CUDA-capable GPU for the evaluation.
 */
class GCUDAFitnessIndividual : public GFunctionIndividual {
public:
private:
};

/******************************************************************************/

} /* namespace Gem::Geneva */
