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

#include "geneva/GenevaHelperFunctionsT.hpp"
#include "geneva/par/GBooleanAdaptor.hpp"
#include "geneva/par/GDoubleGaussAdaptor.hpp"
#include "geneva/par/GFloatGaussAdaptor.hpp"
#include "geneva/par/GInt32FlipAdaptor.hpp"
#include "geneva/par/GAdaptorT.hpp"
#include <cstdint>
#include <memory>

namespace Gem::Geneva {

// Specializations for double, std::int32_t and bool
/******************************************************************************/
/**
 * A factory function that returns the default adaptor for the base type "double"
 *
 * @return The default adaptor for the base type "double"
 */
template <>
std::shared_ptr<gpar::GAdaptorT<double>> getDefaultAdaptor<double>() {
    return std::make_shared<gpar::GDoubleGaussAdaptor>();
}

/******************************************************************************/
/**
 * A factory function that returns the default adaptor for the base type "float"
 *
 * @return The default adaptor for the base type "float"
 */
template <>
std::shared_ptr<gpar::GAdaptorT<float, float>> getDefaultAdaptor<float>() {
    return std::make_shared<gpar::GFloatGaussAdaptor>();
}

/******************************************************************************/
/**
 * A factory function that returns the default adaptor for the base type "std::int32_t"
 *
 * @return The default adaptor for the base type "std::int32_t"
 */
template <>
std::shared_ptr<gpar::GAdaptorT<std::int32_t>> getDefaultAdaptor<std::int32_t>() {
    return std::make_shared<gpar::GInt32FlipAdaptor>();
}

/******************************************************************************/
/**
 * A factory function that returns the default adaptor for the base type "bool"
 *
 * @return The default adaptor for the base type "bool"
 */
template <>
std::shared_ptr<gpar::GAdaptorT<bool>> getDefaultAdaptor<bool>() {
    return std::make_shared<gpar::GBooleanAdaptor>();
}

/******************************************************************************/

} /* namespace Gem::Geneva */
