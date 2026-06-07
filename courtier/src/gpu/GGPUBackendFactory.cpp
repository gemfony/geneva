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

std::unique_ptr<GGPUDeviceBackendI> makeBackend(BackendKind kind, const GGPUHostEvalI *hostEval) {
    switch(kind) {
    case BackendKind::CPU:
        return std::make_unique<GCPUBackend>(hostEval);
    case BackendKind::CUDA:
#ifdef GPUGEN_HAVE_CUDA
        return std::make_unique<GCUDABackend>();
#else
        break;
#endif
    case BackendKind::OpenCL:
#ifdef GPUGEN_HAVE_OPENCL
        return std::make_unique<GOpenCLBackend>();
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

} /* namespace Gem::Courtier::GPU */
