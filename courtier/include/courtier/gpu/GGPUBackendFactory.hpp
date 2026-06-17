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
#include "courtier/gpu/GGPUKernelSpec.hpp"

namespace Gem::Courtier::GPU {

template <typename scalar_type>
class GGPUDeviceBackendI;
template <typename scalar_type>
class GGPUHostEvalI;

/******************************************************************************/
/**
 * Builds the device backend for @p kind. The CPU backend uses @p hostEval for its host reference
 * evaluation (a marshaller is-a GGPUHostEvalI); the CUDA/OpenCL backends ignore it (they run the
 * kernel) but take it for a uniform signature. Throws a geneva_exception if the requested backend was
 * not compiled in (its toolkit was absent at configure time) -- the caller can fall back to
 * BackendKind::CPU.
 *
 * Templated on scalar_type (default double). The CUDA/OpenCL backends whose definitions live in
 * GGPUBackendFactory.cpp are explicitly instantiated there for double and float; other scalar types
 * would need an additional explicit instantiation.
 *
 * @tparam scalar_type The host-side scalar element type (double or float) of the param/fitness buffers
 * @param kind The requested backend kind (CPU, CUDA or OpenCL)
 * @param hostEval Host-evaluation interface used by the CPU backend; ignored by CUDA/OpenCL backends
 * @return An owning pointer to the constructed device backend
 */
template <typename scalar_type = double>
std::unique_ptr<GGPUDeviceBackendI<scalar_type>> makeBackend(BackendKind kind,
                                                             const GGPUHostEvalI<scalar_type> *hostEval);

/**
 * @brief Whether @p kind was compiled into this build (cpu is always true).
 * @param kind The backend kind to query
 * @return true if the backend was compiled into this build, otherwise false
 */
[[nodiscard]] bool backendAvailable(BackendKind kind);

/******************************************************************************/

} /* namespace Gem::Courtier::GPU */
