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

// Geneva headers
#include "common/GErrorStreamer.hpp"
#include "common/GExceptions.hpp"
#include "courtier/gpu/GCPUBackend.hpp"
#include "courtier/gpu/GGPUBackendFactory.hpp"

#ifdef GPUGEN_HAVE_CUDA
#include "courtier/gpu/GCUDABackend.hpp"
#endif
#ifdef GPUGEN_HAVE_OPENCL
#include "courtier/gpu/GOpenCLBackend.hpp"
#endif

namespace Gem::Courtier::GPU {

/******************************************************************************/
/**
 * @brief Reports whether a given backend was compiled into this build.
 *
 * The CPU backend is always available; CUDA and OpenCL are only available when their toolkit was
 * found at configure time (guarded by GPUGEN_HAVE_CUDA / GPUGEN_HAVE_OPENCL).
 *
 * @param kind The backend to query
 * @return true if the backend is available in this build, false otherwise
 */
bool backendAvailable(BackendKind kind) {
    switch(kind) {
    case BackendKind::CPU:
        return true;
    case BackendKind::CUDA:
#ifdef GPUGEN_HAVE_CUDA
        return true;
#else
        return false;
#endif
    case BackendKind::OpenCL:
#ifdef GPUGEN_HAVE_OPENCL
        return true;
#else
        return false;
#endif
    }
    return false;
}

/******************************************************************************/
/**
 * @brief Constructs the requested device backend for the given scalar type.
 *
 * The CPU backend always delegates to the supplied host-evaluation interface; the CUDA and OpenCL
 * backends are only constructible when their toolkit was compiled in.
 *
 * @tparam scalar_type The floating-point scalar the backend operates on (double or float)
 * @param kind The backend to construct
 * @param hostEval The host-evaluation interface used by the CPU backend (the CUDA/OpenCL backends ignore it)
 * @return An owning pointer to the constructed backend
 * @throws geneva_exception if the requested backend was not compiled into this build
 */
template <typename scalar_type>
std::unique_ptr<GGPUDeviceBackendI<scalar_type>> makeBackend(BackendKind kind,
                                                            const GGPUHostEvalI<scalar_type> *hostEval) {
    switch(kind) {
    case BackendKind::CPU:
        return std::make_unique<GCPUBackend<scalar_type>>(hostEval);
    case BackendKind::CUDA:
#ifdef GPUGEN_HAVE_CUDA
        return std::make_unique<GCUDABackend<scalar_type>>();
#else
        break;
#endif
    case BackendKind::OpenCL:
#ifdef GPUGEN_HAVE_OPENCL
        return std::make_unique<GOpenCLBackend<scalar_type>>();
#else
        break;
#endif
    }
    throw geneva_exception(
        g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
        << "Gem::Courtier::GPU::makeBackend(): backend '" << toString(kind)
        << "' was not compiled into this build (its toolkit was not found at configure time)." << '\n');
}

/******************************************************************************/
// Explicit instantiations: this .cpp is where the CUDA/OpenCL backend headers and their toolkit
// guards are available, so the templated backends are materialised here for the supported scalars.
template std::unique_ptr<GGPUDeviceBackendI<double>>
makeBackend<double>(BackendKind, const GGPUHostEvalI<double> *);
template std::unique_ptr<GGPUDeviceBackendI<float>>
makeBackend<float>(BackendKind, const GGPUHostEvalI<float> *);

/******************************************************************************/

} /* namespace Gem::Courtier::GPU */
