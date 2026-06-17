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
#include <memory>
#include <string>

// Geneva headers
#include "courtier/gpu/GGPUDeviceBackendI.hpp"

namespace Gem::Courtier::GPU {

/******************************************************************************/
/**
 * OpenCL device backend. It acquires the kernel at RUN TIME: a `.cl` source file from the config is
 * compiled with clBuildProgram; a prebuilt `.spv` (SPIR-V) is loaded via clCreateProgramWithIL. The
 * whole batch is evaluated in one clEnqueueNDRangeKernel (bulk). All OpenCL types are hidden behind a
 * pimpl so this header pulls in no OpenCL headers.
 *
 * Compiled only when an OpenCL SDK was found at configure time (see the courtier GPU CMakeLists).
 *
 * @tparam scalar_type The genome/fitness flat-buffer element type and device ABI type (default double)
 */
template <typename scalar_type = double>
class GOpenCLBackend final : public GGPUDeviceBackendI<scalar_type> {
public:
    /** @brief Constructs the backend (allocates the pimpl; no device is selected until initialize()). */
    GOpenCLBackend();
    /** @brief The destructor. Releases the OpenCL resources held behind the pimpl. */
    ~GOpenCLBackend() override;

    GOpenCLBackend(const GOpenCLBackend &) = delete;
    GOpenCLBackend &operator=(const GOpenCLBackend &) = delete;

    /** @brief Selects the OpenCL device and acquires the kernel (compiles a .cl source or loads a
     *  .spv module). Called once before the first evaluate().
     *
     *  @param spec The kernel specification (device id, kernel path/entry, launch geometry) */
    void initialize(const KernelSpec &spec) override;
    /** @brief Evaluates a whole batch in one clEnqueueNDRangeKernel launch.
     *
     *  @param params Pointer to the flattened parameter buffer (n_items * dim, row-major)
     *  @param n_items Number of work items in the batch
     *  @param dim Number of parameters per item
     *  @param pconst Pointer to the opaque problem-constant blob (may be ignored by a kernel)
     *  @param pconst_size Size in bytes of the problem-constant blob
     *  @param fitness_out Output buffer receiving n_items fitness values
     *  @param threads_per_item Requested intra-item parallelism (default 1 == one thread per item) */
    void evaluate(
        const scalar_type *params, int n_items, int dim,
        const std::byte *pconst, std::size_t pconst_size,
        scalar_type *fitness_out, int threads_per_item = 1) override;
    /** @brief A short human-readable backend name (for logging).
     *  @return The backend's name (e.g. "opencl") */
    [[nodiscard]] std::string name() const override;

private:
    struct Impl;
    std::unique_ptr<Impl> p_;
};

/******************************************************************************/

} /* namespace Gem::Courtier::GPU */
