/**
 * @file GBenchmarkBatchEvaluator.cu
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

#include "GBenchmarkBatchEvaluator.cuh"
#include "geneva/individuals/GBenchmarkFunctions.hpp"

namespace Gem::Geneva {

/******************************************************************************/
/**
 * @brief CUDA kernel: one thread evaluates one individual.
 *
 * Thread tid evaluates the individual whose parameter vector starts at
 * params[tid * dim]. The result is written to results[tid].
 *
 * Grid and block dimensions are chosen by the caller (see batchEvalBenchmarkGPU).
 * Threads with tid >= N are guard-exited immediately.
 *
 * Memory access pattern: each thread reads a contiguous slice of the params
 * array (row-major layout). Adjacent threads in a warp access adjacent rows,
 * so cache lines are not shared across threads — this is effectively strided
 * access. For functions where dim >> warp_size, consider a column-major
 * (transposed) layout to enable coalesced reads; for typical benchmark
 * dimensions (dim ≤ 512) the current layout is adequate.
 */
__global__ static void
evalBenchmarkKernel(const double* __restrict__ params,
                    double* __restrict__       results,
                    int N, int dim, int funcId)
{
    const int tid = blockIdx.x * blockDim.x + threadIdx.x;
    if (tid >= N) return;
    results[tid] = BM::eval(funcId, params + tid * dim, dim);
}

/******************************************************************************/
// GBenchmarkCUDAContext implementation

void GBenchmarkCUDAContext::ensureCapacity(std::size_t param_count,
                                           std::size_t result_count)
{
    if (param_count > d_params_cap_) {
        if (d_params_) cudaFree(d_params_);
        cudaCheck(cudaMalloc(&d_params_, param_count * sizeof(double)),
                  "cudaMalloc d_params_");
        d_params_cap_ = param_count;
    }
    if (result_count > d_results_cap_) {
        if (d_results_) cudaFree(d_results_);
        cudaCheck(cudaMalloc(&d_results_, result_count * sizeof(double)),
                  "cudaMalloc d_results_");
        d_results_cap_ = result_count;
    }
}

void GBenchmarkCUDAContext::freeBuffers()
{
    if (d_params_)  { cudaFree(d_params_);  d_params_  = nullptr; }
    if (d_results_) { cudaFree(d_results_); d_results_ = nullptr; }
    d_params_cap_  = 0;
    d_results_cap_ = 0;
}

void GBenchmarkCUDAContext::eval(const double* h_params, double* h_results,
                                 int N, int dim, int funcId)
{
    const std::size_t p_count = static_cast<std::size_t>(N) * dim;
    const std::size_t r_count = static_cast<std::size_t>(N);

    ensureCapacity(p_count, r_count);

    cudaCheck(cudaMemcpy(d_params_, h_params, p_count * sizeof(double),
                         cudaMemcpyHostToDevice),
              "cudaMemcpy H->D params");

    constexpr int BLOCK = 256;
    const int grid = (N + BLOCK - 1) / BLOCK;
    evalBenchmarkKernel<<<grid, BLOCK>>>(d_params_, d_results_, N, dim, funcId);
    cudaCheck(cudaGetLastError(), "evalBenchmarkKernel launch");
    cudaCheck(cudaDeviceSynchronize(), "cudaDeviceSynchronize");

    cudaCheck(cudaMemcpy(h_results, d_results_, r_count * sizeof(double),
                         cudaMemcpyDeviceToHost),
              "cudaMemcpy D->H results");
}

/******************************************************************************/
// Stateless convenience wrapper

void batchEvalBenchmarkGPU(const double* h_params, double* h_results,
                            int N, int dim, int funcId)
{
    GBenchmarkCUDAContext ctx;
    ctx.eval(h_params, h_results, N, dim, funcId);
}

} /* namespace Gem::Geneva */
