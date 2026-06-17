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

// Target OpenCL 1.2 (broadly available) and silence the deprecation warnings for the stable C API.
#define CL_TARGET_OPENCL_VERSION 120
#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#include <CL/cl.h>

// Geneva headers
#include "common/GErrorStreamer.hpp"
#include "common/GExceptions.hpp"
#include "courtier/gpu/GOpenCLBackend.hpp"

namespace Gem::Courtier::GPU {

namespace {

/**
 * @brief Throws a geneva_exception unless an OpenCL call returned CL_SUCCESS.
 *
 * @param err The status code returned by an OpenCL API call
 * @param what A short label naming the call site, included in the exception message
 */
void clCheck(cl_int err, const char *what) {
    if(err != CL_SUCCESS) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "GOpenCLBackend: OpenCL error in " << what << ": code " << err << '\n');
    }
}

/**
 * @brief Reads an entire file into a string (used to load the OpenCL kernel source).
 *
 * @param path Filesystem path of the file to read
 * @return The file's full contents as a string
 */
std::string readFile(const std::string &path) {
    std::ifstream in(path, std::ios::binary);
    if(not in) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "GOpenCLBackend: could not open kernel file '" << path << "'" << '\n');
    }
    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

} // namespace

/******************************************************************************/

template <typename scalar_type>
struct GOpenCLBackend<scalar_type>::Impl {
    KernelSpec spec;
    cl_context context = nullptr;
    cl_command_queue queue = nullptr;
    cl_program program = nullptr;
    cl_kernel kernel = nullptr;
    cl_device_id device = nullptr;

    cl_mem d_params = nullptr;
    cl_mem d_pconst = nullptr;
    cl_mem d_fitness = nullptr;
    std::size_t cap_params = 0;
    std::size_t cap_pconst = 0;
    std::size_t cap_fitness = 0;

    // To skip re-uploading an unchanged problem-constant blob (e.g. a fixed target image) every batch.
    const void *last_pconst = nullptr;
    std::size_t last_pconst_sz = 0;

    ~Impl() {
        if(d_params) clReleaseMemObject(d_params);
        if(d_pconst) clReleaseMemObject(d_pconst);
        if(d_fitness) clReleaseMemObject(d_fitness);
        if(kernel) clReleaseKernel(kernel);
        if(program) clReleaseProgram(program);
        if(queue) clReleaseCommandQueue(queue);
        if(context) clReleaseContext(context);
    }

    /**
     * @brief Ensures a device buffer is at least @p need bytes, (re)allocating only when it must grow.
     *
     * @param buf The device buffer handle, updated in place if reallocated
     * @param cap The buffer's current capacity in bytes, updated to @p need on reallocation
     * @param need The required size in bytes
     * @param flags The OpenCL memory flags (e.g. CL_MEM_READ_ONLY) used when (re)creating the buffer
     */
    void ensure(cl_mem &buf, std::size_t &cap, std::size_t need, cl_mem_flags flags) {
        if(need <= cap && buf) {
            return;
        }
        if(buf) {
            clReleaseMemObject(buf);
            buf = nullptr;
        }
        cl_int err = CL_SUCCESS;
        buf = clCreateBuffer(context, flags, need, nullptr, &err);
        clCheck(err, "clCreateBuffer");
        cap = need;
    }
};

/******************************************************************************/

/** @brief Default constructor; creates the pimpl. OpenCL context/program are built in initialize().
 *  @tparam scalar_type The host-side flat-buffer element type (double or float) */
template <typename scalar_type>
GOpenCLBackend<scalar_type>::GOpenCLBackend()
    : p_(std::make_unique<Impl>())
{ /* nothing */ }

/** @brief Destructor; the pimpl releases all OpenCL resources (buffers, kernel, program, queue, context).
 *  @tparam scalar_type The host-side flat-buffer element type (double or float) */
template <typename scalar_type>
GOpenCLBackend<scalar_type>::~GOpenCLBackend() = default;

/** @brief A short backend identifier ("opencl"), used for logging and selection.
 *  @tparam scalar_type The host-side flat-buffer element type (double or float)
 *  @return The backend name string */
template <typename scalar_type>
std::string GOpenCLBackend<scalar_type>::name() const { return "opencl"; }

/**
 * @brief Selects a device, builds the OpenCL context/queue, compiles the kernel source and creates the kernel.
 *
 * @tparam scalar_type The host-side flat-buffer element type (double or float)
 * @param spec The kernel specification (source path, entry-point name, launch geometry, device id)
 */
template <typename scalar_type>
void GOpenCLBackend<scalar_type>::initialize(const KernelSpec &spec) {
    p_->spec = spec;

    // Pick a platform and the requested device (default: first GPU, else any).
    cl_uint nPlatforms = 0;
    clCheck(clGetPlatformIDs(0, nullptr, &nPlatforms), "clGetPlatformIDs(count)");
    if(nPlatforms == 0) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "GOpenCLBackend: no OpenCL platforms found" << '\n');
    }
    std::vector<cl_platform_id> platforms(nPlatforms);
    clCheck(clGetPlatformIDs(nPlatforms, platforms.data(), nullptr), "clGetPlatformIDs");

    std::vector<cl_device_id> devices;
    for(cl_platform_id plat : platforms) {
        cl_uint nDev = 0;
        if(clGetDeviceIDs(plat, CL_DEVICE_TYPE_GPU, 0, nullptr, &nDev) == CL_SUCCESS && nDev > 0) {
            devices.resize(nDev);
            clGetDeviceIDs(plat, CL_DEVICE_TYPE_GPU, nDev, devices.data(), nullptr);
            break;
        }
    }
    if(devices.empty()) {
        // Fall back to any device type on the first platform.
        cl_uint nDev = 0;
        clCheck(clGetDeviceIDs(platforms[0], CL_DEVICE_TYPE_ALL, 0, nullptr, &nDev),
                "clGetDeviceIDs(all,count)");
        devices.resize(nDev);
        clCheck(clGetDeviceIDs(platforms[0], CL_DEVICE_TYPE_ALL, nDev, devices.data(), nullptr),
                "clGetDeviceIDs(all)");
    }
    const std::size_t idx = (spec.device_id >= 0 && static_cast<std::size_t>(spec.device_id) < devices.size())
        ? static_cast<std::size_t>(spec.device_id)
        : 0;
    p_->device = devices[idx];

    cl_int err = CL_SUCCESS;
    p_->context = clCreateContext(nullptr, 1, &p_->device, nullptr, nullptr, &err);
    clCheck(err, "clCreateContext");
    p_->queue = clCreateCommandQueue(p_->context, p_->device, 0, &err);
    clCheck(err, "clCreateCommandQueue");

    const std::string src = readFile(spec.path);
    const char *srcPtr = src.c_str();
    const std::size_t srcLen = src.size();
    p_->program = clCreateProgramWithSource(p_->context, 1, &srcPtr, &srcLen, &err);
    clCheck(err, "clCreateProgramWithSource");

    err = clBuildProgram(p_->program, 1, &p_->device, "-cl-std=CL1.2", nullptr, nullptr);
    if(err != CL_SUCCESS) {
        std::size_t logSize = 0;
        clGetProgramBuildInfo(p_->program, p_->device, CL_PROGRAM_BUILD_LOG, 0, nullptr, &logSize);
        std::string log(logSize, '\0');
        clGetProgramBuildInfo(p_->program, p_->device, CL_PROGRAM_BUILD_LOG, logSize, log.data(), nullptr);
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "GOpenCLBackend: clBuildProgram failed (code " << err << "):\n" << log << '\n');
    }

    p_->kernel = clCreateKernel(p_->program, spec.entry.c_str(), &err);
    clCheck(err, "clCreateKernel (entry name correct?)");
}

/**
 * @brief Uploads the batch, launches the kernel (one work-item per item) and reads the fitness back.
 *
 * @tparam scalar_type The host-side flat-buffer element type (double or float)
 * @param params Row-major parameter buffer of n_items * dim scalar_type (params[i*dim + j] = parameter j of item i)
 * @param n_items Number of items in the batch; a non-positive value is a no-op
 * @param dim Number of scalar_type parameters per item
 * @param pconst Opaque problem-constant blob handed to the kernel; re-uploaded only when its pointer/size changes
 * @param pconst_size Size in bytes of the problem-constant blob (0 means none)
 * @param fitness_out Output buffer receiving n_items fitness scalar_type values
 * @note The trailing threads_per_item argument is unused: this backend always uses one work-item per item, lacking portable double atomics
 */
template <typename scalar_type>
void GOpenCLBackend<scalar_type>::evaluate(
    const scalar_type *params, int n_items, int dim,
    const std::byte *pconst, std::size_t pconst_size,
    scalar_type *fitness_out, int /*threads_per_item*/) {
    if(n_items <= 0) {
        return;
    }
    // OpenCL 1.2 has no portable double atomics, so intra-item parallelism is not offered here: this
    // backend always runs one work-item per item (overwrite). The kernel is told so via its
    // threads_per_item argument = 1.
    const int threads_per_item = 1;
    const std::size_t paramBytes = static_cast<std::size_t>(n_items) * static_cast<std::size_t>(dim) * sizeof(scalar_type);
    const std::size_t fitnessBytes = static_cast<std::size_t>(n_items) * sizeof(scalar_type);
    const std::size_t pconstBytes = pconst_size > 0 ? pconst_size : 1;

    p_->ensure(p_->d_params, p_->cap_params, paramBytes, CL_MEM_READ_ONLY);
    p_->ensure(p_->d_pconst, p_->cap_pconst, pconstBytes, CL_MEM_READ_ONLY);
    p_->ensure(p_->d_fitness, p_->cap_fitness, fitnessBytes, CL_MEM_WRITE_ONLY);

    clCheck(clEnqueueWriteBuffer(p_->queue, p_->d_params, CL_TRUE, 0, paramBytes, params, 0, nullptr, nullptr),
            "clEnqueueWriteBuffer(params)");
    // Upload the problem constants only when they actually changed (same pointer + size => cached
    // static blob reused), avoiding e.g. re-sending a multi-MB target image every generation.
    if(pconst_size > 0 && (pconst != p_->last_pconst || pconst_size != p_->last_pconst_sz)) {
        clCheck(clEnqueueWriteBuffer(p_->queue, p_->d_pconst, CL_TRUE, 0, pconst_size, pconst, 0, nullptr, nullptr),
                "clEnqueueWriteBuffer(pconst)");
        p_->last_pconst = pconst;
        p_->last_pconst_sz = pconst_size;
    }

    // Kernel ABI: evaluate(__global const double* params, int n, int dim,
    //                      __global const uchar* pconst, int pconst_size, __global double* fitness,
    //                      int threads_per_item)
    const int pconstSizeArg = static_cast<int>(pconst_size);
    clCheck(clSetKernelArg(p_->kernel, 0, sizeof(cl_mem), &p_->d_params), "clSetKernelArg(0)");
    clCheck(clSetKernelArg(p_->kernel, 1, sizeof(int), &n_items), "clSetKernelArg(1)");
    clCheck(clSetKernelArg(p_->kernel, 2, sizeof(int), &dim), "clSetKernelArg(2)");
    clCheck(clSetKernelArg(p_->kernel, 3, sizeof(cl_mem), &p_->d_pconst), "clSetKernelArg(3)");
    clCheck(clSetKernelArg(p_->kernel, 4, sizeof(int), &pconstSizeArg), "clSetKernelArg(4)");
    clCheck(clSetKernelArg(p_->kernel, 5, sizeof(cl_mem), &p_->d_fitness), "clSetKernelArg(5)");
    clCheck(clSetKernelArg(p_->kernel, 6, sizeof(int), &threads_per_item), "clSetKernelArg(6)");

    const std::size_t local = p_->spec.launch.block_x > 0 ? p_->spec.launch.block_x : 256;
    const std::size_t global = ((static_cast<std::size_t>(n_items) + local - 1) / local) * local;
    clCheck(clEnqueueNDRangeKernel(p_->queue, p_->kernel, 1, nullptr, &global, &local, 0, nullptr, nullptr),
            "clEnqueueNDRangeKernel");
    clCheck(clEnqueueReadBuffer(p_->queue, p_->d_fitness, CL_TRUE, 0, fitnessBytes, fitness_out, 0, nullptr, nullptr),
            "clEnqueueReadBuffer(fitness)");
    clCheck(clFinish(p_->queue), "clFinish");
}

/******************************************************************************/
// Explicit instantiations. OpenCL confined to this .cpp: only the host-side byte sizing differs; the
// runtime-compiled kernel interprets the raw buffer as its own scalar type.
template class GOpenCLBackend<double>;
template class GOpenCLBackend<float>;

/******************************************************************************/

} /* namespace Gem::Courtier::GPU */
