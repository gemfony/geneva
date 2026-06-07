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

// Standard headers
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

// CUDA driver API + NVRTC (runtime kernel compilation). No CUDA runtime (cudart) is needed: the
// whole flow -- context, memory, module load and launch -- uses the driver API in one context.
#include <cuda.h>
#include <nvrtc.h>

// Geneva headers
#include "common/GErrorStreamer.hpp"
#include "common/GExceptions.hpp"
#include "courtier/gpu/GCUDABackend.hpp"

namespace Gem::Courtier::GPU {

namespace {

/** @brief Throws a geneva_exception if a CUDA driver call failed. */
void cuCheck(CUresult r, const char *what) {
    if(r != CUDA_SUCCESS) {
        const char *name = nullptr;
        const char *desc = nullptr;
        cuGetErrorName(r, &name);
        cuGetErrorString(r, &desc);
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "GCUDABackend: CUDA driver error in " << what << ": "
            << (name ? name : "?") << " -- " << (desc ? desc : "?") << '\n');
    }
}

/** @brief Throws a geneva_exception if an NVRTC call failed (optionally appending the build log). */
void nvrtcCheck(nvrtcResult r, const char *what, const std::string &log = {}) {
    if(r != NVRTC_SUCCESS) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "GCUDABackend: NVRTC error in " << what << ": " << nvrtcGetErrorString(r) << '\n'
            << (log.empty() ? std::string{} : ("NVRTC build log:\n" + log + "\n")));
    }
}

std::string readFile(const std::string &path) {
    std::ifstream in(path, std::ios::binary);
    if(not in) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "GCUDABackend: could not open kernel file '" << path << "'" << '\n');
    }
    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

bool endsWith(const std::string &s, const char *suffix) {
    const std::string suf(suffix);
    return s.size() >= suf.size() && s.compare(s.size() - suf.size(), suf.size(), suf) == 0;
}

} // namespace

/******************************************************************************/

struct GCUDABackend::Impl {
    KernelSpec spec;
    CUdevice device = 0;
    CUcontext context = nullptr;
    CUmodule module = nullptr;
    CUfunction kernel = nullptr;

    // Device buffers, grown lazily to fit the largest batch seen.
    CUdeviceptr d_params = 0;
    CUdeviceptr d_pconst = 0;
    CUdeviceptr d_fitness = 0;
    std::size_t cap_params = 0;
    std::size_t cap_pconst = 0;
    std::size_t cap_fitness = 0;

    // To skip re-uploading an unchanged problem-constant blob (e.g. a fixed target image) every batch.
    const void *last_pconst = nullptr;
    std::size_t last_pconst_sz = 0;

    ~Impl() {
        if(context) {
            cuCtxPushCurrent(context);
            if(d_params) cuMemFree(d_params);
            if(d_pconst) cuMemFree(d_pconst);
            if(d_fitness) cuMemFree(d_fitness);
            if(module) cuModuleUnload(module);
            CUcontext popped = nullptr;
            cuCtxPopCurrent(&popped);
            cuDevicePrimaryCtxRelease(device);
        }
    }

    void ensure(CUdeviceptr &buf, std::size_t &cap, std::size_t need) {
        if(need <= cap) {
            return;
        }
        if(buf) {
            cuCheck(cuMemFree(buf), "cuMemFree(grow)");
            buf = 0;
        }
        cuCheck(cuMemAlloc(&buf, need), "cuMemAlloc");
        cap = need;
    }
};

/******************************************************************************/

GCUDABackend::GCUDABackend()
    : p_(std::make_unique<Impl>())
{ /* nothing */ }

GCUDABackend::~GCUDABackend() = default;

std::string GCUDABackend::name() const { return "cuda"; }

void GCUDABackend::initialize(const KernelSpec &spec) {
    p_->spec = spec;

    cuCheck(cuInit(0), "cuInit");
    int n_devices = 0;
    cuCheck(cuDeviceGetCount(&n_devices), "cuDeviceGetCount");
    if(n_devices <= 0) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "GCUDABackend: no CUDA devices found" << '\n');
    }
    if(spec.device_id < 0 || spec.device_id >= n_devices) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "GCUDABackend: device_id " << spec.device_id << " out of range [0," << n_devices << ")"
            << '\n');
    }
    cuCheck(cuDeviceGet(&p_->device, spec.device_id), "cuDeviceGet");
    // Use the primary context so behaviour matches the usual CUDA runtime model.
    cuCheck(cuDevicePrimaryCtxRetain(&p_->context, p_->device), "cuDevicePrimaryCtxRetain");
    cuCheck(cuCtxPushCurrent(p_->context), "cuCtxPushCurrent");

    // Acquire the module: either load a prebuilt PTX/cubin, or NVRTC-compile a .cu source.
    std::string ptx;
    const std::string &path = spec.path;
    if(endsWith(path, ".ptx") || endsWith(path, ".cubin")) {
        ptx = readFile(path);
        cuCheck(cuModuleLoadData(&p_->module, ptx.c_str()), "cuModuleLoadData(prebuilt)");
    } else {
        const std::string src = readFile(path);

        int major = 0;
        int minor = 0;
        cuCheck(cuDeviceGetAttribute(&major, CU_DEVICE_ATTRIBUTE_COMPUTE_CAPABILITY_MAJOR, p_->device),
                "cuDeviceGetAttribute(major)");
        cuCheck(cuDeviceGetAttribute(&minor, CU_DEVICE_ATTRIBUTE_COMPUTE_CAPABILITY_MINOR, p_->device),
                "cuDeviceGetAttribute(minor)");
        const std::string arch = "--gpu-architecture=compute_" + std::to_string(major) + std::to_string(minor);

        nvrtcProgram prog = nullptr;
        nvrtcCheck(nvrtcCreateProgram(&prog, src.c_str(), path.c_str(), 0, nullptr, nullptr),
                   "nvrtcCreateProgram");
        const char *opts[] = {arch.c_str(), "--std=c++17"};
        const nvrtcResult cr = nvrtcCompileProgram(prog, 2, opts);
        std::size_t logSize = 0;
        nvrtcGetProgramLogSize(prog, &logSize);
        std::string log(logSize, '\0');
        if(logSize > 1) {
            nvrtcGetProgramLog(prog, log.data());
        }
        if(cr != NVRTC_SUCCESS) {
            nvrtcDestroyProgram(&prog);
            nvrtcCheck(cr, "nvrtcCompileProgram", log);
        }
        std::size_t ptxSize = 0;
        nvrtcCheck(nvrtcGetPTXSize(prog, &ptxSize), "nvrtcGetPTXSize");
        ptx.resize(ptxSize);
        nvrtcCheck(nvrtcGetPTX(prog, ptx.data()), "nvrtcGetPTX");
        nvrtcDestroyProgram(&prog);
        cuCheck(cuModuleLoadData(&p_->module, ptx.c_str()), "cuModuleLoadData(nvrtc)");
    }

    cuCheck(cuModuleGetFunction(&p_->kernel, p_->module, spec.entry.c_str()),
            "cuModuleGetFunction (is the entry 'extern \"C\"' and named correctly?)");

    CUcontext popped = nullptr;
    cuCheck(cuCtxPopCurrent(&popped), "cuCtxPopCurrent");
}

void GCUDABackend::evaluate(
    const double *params, int n_items, int dim,
    const std::byte *pconst, std::size_t pconst_size,
    double *fitness_out, int threads_per_item) {
    if(n_items <= 0) {
        return;
    }
    if(threads_per_item < 1) {
        threads_per_item = 1;
    }
    cuCheck(cuCtxPushCurrent(p_->context), "cuCtxPushCurrent(evaluate)");

    const std::size_t paramBytes = static_cast<std::size_t>(n_items) * static_cast<std::size_t>(dim) * sizeof(double);
    const std::size_t fitnessBytes = static_cast<std::size_t>(n_items) * sizeof(double);
    const std::size_t pconstBytes = pconst_size > 0 ? pconst_size : 1; // avoid a 0-byte allocation

    p_->ensure(p_->d_params, p_->cap_params, paramBytes);
    p_->ensure(p_->d_fitness, p_->cap_fitness, fitnessBytes);
    p_->ensure(p_->d_pconst, p_->cap_pconst, pconstBytes);

    cuCheck(cuMemcpyHtoD(p_->d_params, params, paramBytes), "cuMemcpyHtoD(params)");
    // Upload the problem constants only when they actually changed (same pointer + size => the
    // consumer reused its cached, static blob, so the device copy is still valid). This avoids
    // re-sending e.g. a multi-MB target image every generation.
    if(pconst_size > 0 && (pconst != p_->last_pconst || pconst_size != p_->last_pconst_sz)) {
        cuCheck(cuMemcpyHtoD(p_->d_pconst, pconst, pconst_size), "cuMemcpyHtoD(pconst)");
        p_->last_pconst = pconst;
        p_->last_pconst_sz = pconst_size;
    }
    // Always zero the fitness buffer: a kernel that accumulates (atomicAdd, for intra-item
    // parallelism) needs it, and a kernel that overwrites is unaffected. This keeps an accumulating
    // kernel correct at ANY threads_per_item (including 1).
    cuCheck(cuMemsetD8(p_->d_fitness, 0, fitnessBytes), "cuMemsetD8(fitness)");

    // Kernel ABI: evaluate(const double* params, int n, int dim,
    //                      const unsigned char* pconst, int pconst_size, double* fitness,
    //                      int threads_per_item)
    int pconstSizeArg = static_cast<int>(pconst_size);
    void *args[] = {
        &p_->d_params, &n_items, &dim, &p_->d_pconst, &pconstSizeArg, &p_->d_fitness,
        &threads_per_item};

    // Total threads = n_items * threads_per_item (one per (item, work-unit)).
    const unsigned int bx = p_->spec.launch.block_x > 0 ? p_->spec.launch.block_x : 256;
    const unsigned long long totalThreads =
        static_cast<unsigned long long>(n_items) * static_cast<unsigned long long>(threads_per_item);
    const unsigned int gx = static_cast<unsigned int>((totalThreads + bx - 1) / bx);

    cuCheck(
        cuLaunchKernel(p_->kernel, gx, 1, 1, bx, 1, 1, 0, nullptr, args, nullptr),
        "cuLaunchKernel");
    cuCheck(cuCtxSynchronize(), "cuCtxSynchronize");

    cuCheck(cuMemcpyDtoH(fitness_out, p_->d_fitness, fitnessBytes), "cuMemcpyDtoH(fitness)");

    CUcontext popped = nullptr;
    cuCheck(cuCtxPopCurrent(&popped), "cuCtxPopCurrent(evaluate)");
}

/******************************************************************************/

} /* namespace Gem::Courtier::GPU */
