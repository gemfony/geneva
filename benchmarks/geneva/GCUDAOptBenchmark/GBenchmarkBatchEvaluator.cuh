/**
 * @file GBenchmarkBatchEvaluator.cuh
 */

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
 ********************************************************************************/

#pragma once

#include <cstddef>
#include <stdexcept>

#include <cuda_runtime.h>

namespace Gem::Geneva::Benchmarks {

/******************************************************************************/
/**
 * @brief Checks a CUDA return code and throws std::runtime_error on failure.
 */
inline void cudaCheck(cudaError_t err, const char* where)
{
    if (err != cudaSuccess) {
        throw std::runtime_error(
            std::string(where) + ": " + cudaGetErrorString(err));
    }
}

/******************************************************************************/
/**
 * @brief Holds pre-allocated device buffers for repeated batch evaluations.
 *
 * Allocating and freeing device memory on every call to batchEvalBenchmarkGPU()
 * adds ~10 µs overhead per call. For benchmark loops that evaluate thousands of
 * generations, this accumulates. GBenchmarkCUDAContext allocates once and resizes
 * only when the batch grows beyond the current capacity.
 *
 * Usage:
 * @code
 *   GBenchmarkCUDAContext ctx;
 *   ctx.eval(h_params, h_results, N, dim, gbm::FUNC_RASTRIGIN);
 *   // ... many generations later ...
 *   ctx.eval(h_params2, h_results2, N2, dim, gbm::FUNC_RASTRIGIN);
 * @endcode
 */
class GBenchmarkCUDAContext {
public:
    GBenchmarkCUDAContext() = default;

    ~GBenchmarkCUDAContext() { freeBuffers(); }

    // Non-copyable, non-movable (owns raw device pointers)
    GBenchmarkCUDAContext(const GBenchmarkCUDAContext&)            = delete;
    GBenchmarkCUDAContext& operator=(const GBenchmarkCUDAContext&) = delete;

    /**
     * @brief Evaluates N individuals of dimension dim on the GPU.
     *
     * h_params is row-major: h_params[i*dim + j] = j-th parameter of individual i.
     * Results written to h_results[i].
     *
     * @param h_params   Host array of parameter values [N × dim]
     * @param h_results  Host array for fitness output [N]
     * @param N          Number of individuals
     * @param dim        Parameter dimension
     * @param funcId     gbm::FUNC_* constant identifying the benchmark function
     */
    void eval(const double* h_params, double* h_results,
              int N, int dim, int funcId);

private:
    void ensureCapacity(std::size_t param_count, std::size_t result_count);
    void freeBuffers();

    double* d_params_  {nullptr};
    double* d_results_ {nullptr};
    std::size_t d_params_cap_  {0};
    std::size_t d_results_cap_ {0};
};

/******************************************************************************/
/**
 * @brief Stateless convenience wrapper: evaluates one batch and returns.
 *
 * Allocates and frees device memory on every call. Suitable for occasional
 * use; for tight loops, use GBenchmarkCUDAContext instead.
 *
 * @param h_params   Host array [N × dim], row-major
 * @param h_results  Host output array [N]
 * @param N          Number of individuals
 * @param dim        Parameter dimension per individual
 * @param funcId     gbm::FUNC_* constant (0=PARABOLA … 14=ZAKHAROV)
 */
void batchEvalBenchmarkGPU(
    const double* h_params,
    double*       h_results,
    int           N,
    int           dim,
    int           funcId);

} /* namespace Gem::Geneva::Benchmarks */
