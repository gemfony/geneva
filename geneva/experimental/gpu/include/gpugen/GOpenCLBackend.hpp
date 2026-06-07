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
#include "gpugen/GGPUDeviceBackendI.hpp"

namespace Gem::Courtier::GPU {

/******************************************************************************/
/**
 * OpenCL device backend. It acquires the kernel at RUN TIME: a `.cl` source file from the config is
 * compiled with clBuildProgram; a prebuilt `.spv` (SPIR-V) is loaded via clCreateProgramWithIL. The
 * whole batch is evaluated in one clEnqueueNDRangeKernel (bulk). All OpenCL types are hidden behind a
 * pimpl so this header pulls in no OpenCL headers.
 *
 * Compiled only when an OpenCL SDK was found at configure time (see the experimental CMakeLists).
 */
class GOpenCLBackend final : public GGPUDeviceBackendI {
public:
    GOpenCLBackend();
    ~GOpenCLBackend() override;

    GOpenCLBackend(const GOpenCLBackend &) = delete;
    GOpenCLBackend &operator=(const GOpenCLBackend &) = delete;

    void initialize(const KernelSpec &spec) override;
    void evaluate(
        const double *params, int n_items, int dim,
        const std::byte *pconst, std::size_t pconst_size,
        double *fitness_out) override;
    [[nodiscard]] std::string name() const override;

private:
    struct Impl;
    std::unique_ptr<Impl> p_;
};

/******************************************************************************/

} /* namespace Gem::Courtier::GPU */
