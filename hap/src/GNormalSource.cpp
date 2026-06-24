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

#include "hap/GNormalSource.hpp"
#include "hap/GRandomFactory.hpp" // for randomFactory() / getSeed()
#include "GFillBackend.hpp"       // library-private engine selection (GPU/SIMD/scalar)

#include <cstddef>

namespace Gem::Hap {

/******************************************************************************/
/**
 * @brief Bulk-fills dst[0..n) with standard normal N(0,1) deviates (GPU-native when available).
 *
 * Uses a thread_local GFillBackend: the first call on a thread seeds it from the factory (a fresh
 * seed per thread, so the streams are distinct), and on a CUDA build each thread drives its own
 * cuRAND generator -- so concurrent callers neither block nor share state. The CPU path likewise
 * runs per-thread, so it scales.
 *
 * @param dst Destination buffer holding at least n doubles
 * @param n   Number of standard normal deviates to write
 */
void generateStandardNormals(double *dst, std::size_t n) {
    thread_local detail::GFillBackend backend(static_cast<std::uint64_t>(randomFactory()->getSeed()));
    backend.generateNormal(dst, n);
}

/******************************************************************************/

} /* namespace Gem::Hap */
