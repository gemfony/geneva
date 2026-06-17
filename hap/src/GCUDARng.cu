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

#include "hap/GCUDARng.hpp"

#include <cuda_runtime.h>
#include <curand.h>

#include <atomic>
#include <cstdint>
#include <cstdio>

namespace Gem::Hap {

namespace {

// At process exit the CUDA runtime unloads (atexit) while producer threads may
// still be issuing work, so calls then fail with cudaErrorCudartUnloading
// ("driver shutting down") or a downstream LAUNCH_FAILURE. That is expected and
// benign — the numbers are no longer needed. Once any thread observes it we set
// this flag so every backend stops issuing CUDA work quietly. Genuine errors
// (not during shutdown) are reported once to stderr — never via the Geneva
// logger, whose singleton may already be gone during static destruction.
std::atomic<bool> g_cudaShuttingDown{false};

/**
 * @brief Reports a failed CUDA runtime call (unless during shutdown) and signals success.
 *
 * Treats cudaErrorCudartUnloading as benign process-shutdown noise (setting the
 * global shutdown flag). Genuine errors are printed once to stderr -- never via the
 * Geneva logger, whose singleton may already be gone during static destruction.
 *
 * @param st The status returned by the CUDA runtime call
 * @param what A short label identifying the call, for the error message
 * @return true if the call succeeded, false on any error or during shutdown
 */
inline bool checkCuda(cudaError_t st, char const *what) {
    if (st == cudaSuccess) return true;
    if (st == cudaErrorCudartUnloading) { g_cudaShuttingDown.store(true, std::memory_order_relaxed); return false; }
    if (g_cudaShuttingDown.load(std::memory_order_relaxed)) return false;
    std::fprintf(stderr, "GCudaRNG: CUDA call failed (%s): %s\n", what, cudaGetErrorString(st));
    return false;
}
/**
 * @brief Reports a failed cuRAND call (unless during shutdown) and signals success.
 *
 * @param st The status returned by the cuRAND call
 * @param what A short label identifying the call, for the error message
 * @return true if the call succeeded, false on any error or during shutdown
 */
inline bool checkCurand(curandStatus_t st, char const *what) {
    if (st == CURAND_STATUS_SUCCESS) return true;
    if (g_cudaShuttingDown.load(std::memory_order_relaxed)) return false;
    std::fprintf(stderr, "GCudaRNG: cuRAND call failed (%s), status %d\n", what, static_cast<int>(st));
    return false;
}

/**
 * @brief Reports whether the CUDA runtime has begun shutting down.
 *
 * @return true once any backend has observed a shutdown-related CUDA error
 */
bool cudaShuttingDown() noexcept { return g_cudaShuttingDown.load(std::memory_order_relaxed); }

} // namespace

/**
 * @brief True iff at least one usable CUDA device is present at runtime.
 *
 * On a host without a driver or device, cudaGetDeviceCount returns an error,
 * which correctly yields "not available" and triggers the CPU fallback.
 *
 * @return true if a CUDA device is available, false otherwise
 */
bool GCudaRNG::deviceAvailable() noexcept {
    int count = 0;
    // On a host without a driver/device cudaGetDeviceCount returns an error,
    // which correctly yields "not available" -> CPU fallback.
    return cudaGetDeviceCount(&count) == cudaSuccess && count > 0;
}

/**
 * @brief Creates a CUDA stream and a Philox4-32-10 cuRAND generator seeded from the given value.
 *
 * @param seed The seed value passed to the cuRAND pseudo-random generator
 */
GCudaRNG::GCudaRNG(std::uint64_t seed) {
    cudaStream_t stream{};
    checkCuda(cudaStreamCreate(&stream), "cudaStreamCreate");
    stream_ = stream;

    curandGenerator_t gen{};
    // Philox4-32-10: fast, high-quality, no per-thread state setup needed.
    checkCurand(curandCreateGenerator(&gen, CURAND_RNG_PSEUDO_PHILOX4_32_10),
                "curandCreateGenerator");
    checkCurand(curandSetPseudoRandomGeneratorSeed(gen, static_cast<unsigned long long>(seed)),
                "curandSetPseudoRandomGeneratorSeed");
    checkCurand(curandSetStream(gen, stream), "curandSetStream");
    gen_ = gen;
}

/**
 * @brief Destroys the cuRAND generator, frees the device buffer and destroys the CUDA stream.
 */
GCudaRNG::~GCudaRNG() {
    if (gen_ != nullptr) curandDestroyGenerator(static_cast<curandGenerator_t>(gen_));
    if (d_buf_ != nullptr) cudaFree(d_buf_);
    if (stream_ != nullptr) cudaStreamDestroy(static_cast<cudaStream_t>(stream_));
}

/**
 * @brief Fills dst[0..n) with n 64-bit random values via a single device generate plus host copy.
 *
 * Requests 2*n 32-bit words from cuRAND (two words form one 64-bit result_type),
 * growing the device buffer as needed, then copies them to the host. Does nothing
 * if n is zero or the CUDA runtime is shutting down.
 *
 * @param dst Destination host buffer that receives n 64-bit values; must hold at least n entries
 * @param n The number of 64-bit values to generate
 */
void GCudaRNG::generate(result_type *dst, std::size_t n) {
    if (n == 0 || cudaShuttingDown()) return;

    // cuRAND's curandGenerate produces 32-bit words; two of them make one
    // 64-bit result_type, so we request 2*n words of raw uniform bits.
    const std::size_t words = 2 * n;

    if (words > d_words_) {
        if (d_buf_ != nullptr) cudaFree(d_buf_);
        if (!checkCuda(cudaMalloc(&d_buf_, words * sizeof(std::uint32_t)), "cudaMalloc")) {
            d_buf_ = nullptr;
            d_words_ = 0;
            return;
        }
        d_words_ = words;
    }

    auto stream = static_cast<cudaStream_t>(stream_);
    if (!checkCurand(curandGenerate(static_cast<curandGenerator_t>(gen_),
                                    static_cast<unsigned int *>(d_buf_), words),
                     "curandGenerate")) return;
    if (!checkCuda(cudaMemcpyAsync(dst, d_buf_, words * sizeof(std::uint32_t),
                                   cudaMemcpyDeviceToHost, stream),
                   "cudaMemcpyAsync")) return;
    checkCuda(cudaStreamSynchronize(stream), "cudaStreamSynchronize");
}

} /* namespace Gem::Hap */
