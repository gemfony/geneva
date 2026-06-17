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
#include <cstdint>
#include <string>

namespace Gem::Courtier::GPU {

/******************************************************************************/
/**
 * The GPU consumer framework -- common, device-agnostic descriptors.
 *
 * Which device-programming-model backend evaluates a batch. The CPU backend is always available
 * (it runs the marshaller's host reference evaluation); the Cuda / OpenCL backends are compiled in
 * only when their toolkit was found at configure time.
 */
enum class BackendKind {
    CPU,
    CUDA,
    OpenCL
};

/** @brief Parse a backend mnemonic ("cpu" | "cuda" | "opencl"); falls back to Cpu on anything else.
 *  @param s The backend mnemonic string to parse
 *  @return The matching BackendKind, or BackendKind::CPU for any unrecognized string */
inline BackendKind backendKindFromString(const std::string &s) {
    if(s == "cuda") {
        return BackendKind::CUDA;
    }
    if(s == "opencl") {
        return BackendKind::OpenCL;
    }
    return BackendKind::CPU;
}

/** @brief Human-readable name of a backend kind.
 *  @param k The backend kind to name
 *  @return Its mnemonic ("cuda" | "opencl" | "cpu") */
inline const char *toString(BackendKind k) {
    switch(k) {
    case BackendKind::CUDA:   return "cuda";
    case BackendKind::OpenCL: return "opencl";
    case BackendKind::CPU:    return "cpu";
    }
    return "cpu";
}

/******************************************************************************/
/**
 * The launch geometry for one bulk kernel launch. The framework computes a 1D grid over the batch
 * (one work-item / thread per population member) from block_x by default; the other dimensions are
 * available for kernels that want a 2D/3D decomposition. A grid extent of 0 means "compute it from
 * the item count and the block size".
 */
struct LaunchConfig {
    unsigned int block_x = 256;
    unsigned int block_y = 1;
    unsigned int block_z = 1;
    unsigned int grid_x = 0; // 0 == auto (ceil(n_items / block_x))
    unsigned int grid_y = 0;
    unsigned int grid_z = 0;
};

/******************************************************************************/
/**
 * Everything a backend needs to acquire and launch the user's kernel, all sourced from the config
 * file. `path` points at the device code: a source file (.cu for CUDA / NVRTC, .cl for OpenCL) that
 * is compiled at run time, or a prebuilt module (.ptx / .cubin for CUDA, .spv for OpenCL/SPIR-V)
 * that is loaded directly. `entry` is the kernel's entry-point name (it must be declared so its
 * symbol is un-mangled, e.g. `extern "C"` for CUDA).
 */
struct KernelSpec {
    std::string path;             ///< path to the kernel source or prebuilt module
    std::string entry = "evaluate"; ///< kernel entry-point name
    int device_id = 0;            ///< which GPU/device to use
    LaunchConfig launch;          ///< launch geometry
};

/******************************************************************************/

} /* namespace Gem::Courtier::GPU */
