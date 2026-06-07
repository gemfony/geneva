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

// Standard headers
#include <cstddef>
#include <string>

// Geneva headers
#include "courtier/gpu/GGPUKernelSpec.hpp"

namespace Gem::Courtier::GPU {

/******************************************************************************/
/**
 * The GPU consumer framework -- the device-programming-model abstraction.
 *
 * This is the "different technical content" layer: one interface, several implementations
 * (GCPUBackend always; GCUDABackend / GOpenCLBackend when their toolkit is present). The consumer
 * marshals a whole batch into flat host buffers (via GGPUEvaluableI) and hands them to a backend,
 * which uploads, launches the kernel ONCE over the whole batch (bulk submission), and downloads the
 * per-item fitness. All backends share this API; they differ only in how they acquire and run the
 * kernel.
 */
class GGPUDeviceBackendI {
public:
    virtual ~GGPUDeviceBackendI() = default;

    /** @brief Selects the device and acquires the kernel described by @p spec (runtime-compiles a
     *  source file, or loads a prebuilt module). Called once before the first evaluate(). */
    virtual void initialize(const KernelSpec &spec) = 0;

    /** @brief Evaluates a whole batch in one launch. @p params is n_items * dim row-major doubles;
     *  @p pconst is an opaque problem-constant blob; writes n_items doubles into @p fitness_out.
     *  @p threads_per_item requests intra-item (e.g. pixel-level) parallelism: a backend that supports
     *  it launches n_items * threads_per_item threads and the kernel accumulates each item's fitness
     *  (the backend zeroes fitness_out first); the default 1 is one thread per item (overwrite). */
    virtual void evaluate(
        const double *params, int n_items, int dim,
        const std::byte *pconst, std::size_t pconst_size,
        double *fitness_out, int threads_per_item = 1) = 0;

    /** @brief A short human-readable backend name (for logging). */
    [[nodiscard]] virtual std::string name() const = 0;
};

/******************************************************************************/

} /* namespace Gem::Courtier::GPU */
