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
#include <cstddef>

namespace Gem::Hap {

/******************************************************************************/
/**
 * @brief Bulk-fills dst[0..n) with standard normal @f$N(0,1)@f$ deviates.
 *
 * The deviates are produced in bulk by the same engine GFillBackend selects for raw words: on a
 * CUDA build with a device present, natively on the GPU (curandGenerateNormalDouble -- the
 * Box-Muller transform runs on the device); otherwise on the CPU (Marsaglia polar). This is the
 * intended fill path for GNormalCacheT's bulk mode: one call per prefetch fills a whole chunk, so
 * the (otherwise per-value) sqrt/log transform is both batched and, on a GPU, offloaded.
 *
 * Backed by a @c thread_local generator, so concurrent callers neither block nor share state (on
 * the GPU each thread drives its own cuRAND generator/stream). Scale to @f$N(\mu,\sigma)@f$ at the
 * point of use via @f$x=\mu+\sigma z@f$.
 *
 * @param dst Destination buffer holding at least n doubles
 * @param n   Number of standard normal deviates to write
 */
void generateStandardNormals(double *dst, std::size_t n);

/******************************************************************************/

} /* namespace Gem::Hap */
