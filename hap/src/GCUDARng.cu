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

inline bool checkCuda(cudaError_t st, char const *what) {
    if (st == cudaSuccess) return true;
    if (st == cudaErrorCudartUnloading) { g_cudaShuttingDown.store(true, std::memory_order_relaxed); return false; }
    if (g_cudaShuttingDown.load(std::memory_order_relaxed)) return false;
    std::fprintf(stderr, "GCudaRNG: CUDA call failed (%s): %s\n", what, cudaGetErrorString(st));
    return false;
}
inline bool checkCurand(curandStatus_t st, char const *what) {
    if (st == CURAND_STATUS_SUCCESS) return true;
    if (g_cudaShuttingDown.load(std::memory_order_relaxed)) return false;
    std::fprintf(stderr, "GCudaRNG: cuRAND call failed (%s), status %d\n", what, static_cast<int>(st));
    return false;
}

bool cudaShuttingDown() noexcept { return g_cudaShuttingDown.load(std::memory_order_relaxed); }

} // namespace

bool GCudaRNG::deviceAvailable() noexcept {
    int count = 0;
    // On a host without a driver/device cudaGetDeviceCount returns an error,
    // which correctly yields "not available" -> CPU fallback.
    return cudaGetDeviceCount(&count) == cudaSuccess && count > 0;
}

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

GCudaRNG::~GCudaRNG() {
    if (gen_ != nullptr) curandDestroyGenerator(static_cast<curandGenerator_t>(gen_));
    if (d_buf_ != nullptr) cudaFree(d_buf_);
    if (stream_ != nullptr) cudaStreamDestroy(static_cast<cudaStream_t>(stream_));
}

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
