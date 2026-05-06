/**
 * @file GBenchmarkCUDAConsumer.hpp
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

#include <memory>
#include <stdexcept>
#include <vector>

#include "GBenchmarkBatchEvaluator.cuh"
#include "geneva-individuals/GFunctionIndividual.hpp"

namespace Gem::Geneva {

/******************************************************************************/
/**
 * @brief Batch-evaluates a population of GFunctionIndividual objects on the GPU.
 *
 * Extracts all parameter vectors into a flat row-major host buffer, dispatches
 * a single kernel launch via GBenchmarkCUDAContext, then writes the returned
 * fitness values back into each individual.
 *
 * All individuals must share the same demoFunction and parameter dimension.
 * The function does not mark individuals as processed — callers are responsible
 * for any Geneva bookkeeping (e.g. calling setFitness / mark_as_processed).
 *
 * @param population   Non-empty vector of shared_ptr<GFunctionIndividual>.
 * @param funcId       BM::FUNC_* constant (must match the individuals' demoFunction).
 * @param ctx          Persistent CUDA context; reuse across generations to avoid
 *                     repeated cudaMalloc overhead.
 *
 * @throws std::invalid_argument if population is empty.
 * @throws std::runtime_error    on CUDA errors (from GBenchmarkCUDAContext::eval).
 */
inline void batchEvalPopulationGPU(
    const std::vector<std::shared_ptr<GFunctionIndividual>> &population,
    int funcId,
    GBenchmarkCUDAContext &ctx
) {
    if(population.empty()) {
        throw std::invalid_argument("batchEvalPopulationGPU: population must not be empty");
    }

    // Extract parameter dimension from the first individual.
    std::vector<double> probe;
    population.front()->streamline(probe);
    const int dim = static_cast<int>(probe.size());
    const int N = static_cast<int>(population.size());

    // Fill flat row-major parameter buffer.
    std::vector<double> h_params(static_cast<std::size_t>(N) * dim);
    for(int i = 0; i < N; ++i) {
        std::vector<double> pv;
        population[static_cast<std::size_t>(i)]->streamline(pv);
        std::copy(pv.begin(), pv.end(), h_params.begin() + static_cast<std::ptrdiff_t>(i * dim));
    }

    std::vector<double> h_results(static_cast<std::size_t>(N));
    ctx.eval(h_params.data(), h_results.data(), N, dim, funcId);

    // Write fitness values back into the individuals.
    for(int i = 0; i < N; ++i) {
        population[static_cast<std::size_t>(i)]->setFitness(
            0,
            h_results[static_cast<std::size_t>(i)]
        );
    }
}

/******************************************************************************/
/**
 * @brief Stateless overload: allocates a temporary GBenchmarkCUDAContext.
 *
 * Suitable for single shots; for repeated calls (e.g., across generations)
 * prefer the overload that accepts an explicit context to amortise cudaMalloc.
 */
inline void batchEvalPopulationGPU(
    const std::vector<std::shared_ptr<GFunctionIndividual>> &population,
    int funcId
) {
    GBenchmarkCUDAContext ctx;
    batchEvalPopulationGPU(population, funcId, ctx);
}

/******************************************************************************/

} /* namespace Gem::Geneva */
