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
#include <memory>

// Geneva headers
#include "gpugen/GGPUKernelSpec.hpp"

namespace Gem::Geneva::GPU {

class GGPUDeviceBackendI;
class GGPUEvaluableI;

/******************************************************************************/
/**
 * Builds the device backend for @p kind. The CPU backend uses @p marshaller for its host reference
 * evaluation; the CUDA/OpenCL backends ignore it (they run the kernel) but take it for a uniform
 * signature. Throws a geneva_exception if the requested backend was not compiled in (its toolkit was
 * absent at configure time) -- the caller can fall back to BackendKind::Cpu.
 */
std::unique_ptr<GGPUDeviceBackendI> makeBackend(BackendKind kind, const GGPUEvaluableI *marshaller);

/** @brief Whether @p kind was compiled into this build (cpu is always true). */
[[nodiscard]] bool backendAvailable(BackendKind kind);

/******************************************************************************/

} /* namespace Gem::Geneva::GPU */
