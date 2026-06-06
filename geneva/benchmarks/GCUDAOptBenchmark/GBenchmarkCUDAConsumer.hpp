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
#include <vector>

// Geneva headers
#include "common/GGlobalDefines.hpp"
#include "courtier/GBaseConsumerT.hpp"
#include "geneva/par/GParameterSet.hpp"
#include "geneva/individuals/GFunctionIndividual.hpp"

// Local CUDA evaluator
#include "GBenchmarkBatchEvaluator.cuh"

namespace Gem::Geneva::Benchmarks {

/******************************************************************************/
/**
 * @brief GPU batch consumer for GFunctionIndividual (courtier2).
 *
 * A courtier2 local consumer: courtier2 hands dispatch_() the WHOLE round's batch (a vector of work
 * items) at once, so the GPU evaluates an entire population in one kernel launch -- the manual
 * batch-accumulation machinery (the broker get/put stream, batchSize_/flushTimeout_, the worker
 * thread) the old broker-stream consumer needed simply disappears. The persistent
 * GBenchmarkCUDAContext avoids repeated cudaMalloc across generations.
 *
 * Work items arrive as GParameterSet; they are cast to GFunctionIndividual internally to extract
 * demoFunction (the kernel funcId) and the double parameters. The pre-computed GPU fitness is
 * injected via individual->process(vector<parameterset_processing_result>{fitness}) -- the pattern
 * from examples/geneva/15_GCUDAWorker -- which performs all Geneva bookkeeping (leaving the item
 * PROCESSED, which is what courtier2's reconciliation reads) without triggering fitnessCalculation().
 * GFunctionIndividual itself is not modified and remains CPU-runnable.
 *
 * Wiring (see GCUDAOptBenchmark.cu / GAlgorithmBenchmarkRunner): the benchmark registers this consumer
 * with a courtier2 GBrokerT and injects that broker into each algorithm via setBroker(),
 * rather than enrolling with the old singleton broker.
 */
class GCUDABatchConsumer
    : public Gem::Courtier::GBaseConsumerT<gpar::GParameterSet>
{
    using individual_t = gpar::GParameterSet;

public:
    using item_ptr = typename Gem::Courtier::GBaseConsumerT<gpar::GParameterSet>::item_ptr;

    GCUDABatchConsumer() = default;
    ~GCUDABatchConsumer() override = default;

    GCUDABatchConsumer(const GCUDABatchConsumer &) = delete;
    GCUDABatchConsumer &operator=(const GCUDABatchConsumer &) = delete;

protected:
    //--------------------------------------------------------------------------
    /**
     * @brief Evaluates the whole round's batch on the GPU in one launch. A GPU/cast failure throws
     * (a hard benchmark error); on success every item is left PROCESSED for the policy reconciliation.
     */
    void dispatch_(std::vector<item_ptr> &items) override {
        if (items.empty()) return;
        evaluateBatchOnGPU(items);
    }

private:
    //--------------------------------------------------------------------------
    /**
     * @brief GPU-evaluates all individuals in the batch in one kernel launch.
     *
     * Items arrive as GParameterSet; we cast to GFunctionIndividual to extract
     * demoFunction (for the kernel funcId) and double parameters.
     * Results are injected back via process(vector<parameterset_processing_result>{fitness})
     * on the base GParameterSet pointer — pattern from examples/geneva/15_GCUDAWorker.
     */
    void evaluateBatchOnGPU(std::vector<item_ptr> &batch) {
        const int N = static_cast<int>(batch.size());

        // Cast first item to GFunctionIndividual to get demoFunction and dimension.
        // All items in one batch share the same function and dimension.
        auto front_fi = std::dynamic_pointer_cast<gind::GFunctionIndividual>(batch.front());
        if (!front_fi) {
            throw std::runtime_error("GCUDABatchConsumer: batch item is not a GFunctionIndividual");
        }
        // solverFunction enum values equal gbm::FUNC_* integer constants (both 0–14)
        const int funcId = static_cast<int>(front_fi->getDemoFunction());

        std::vector<double> probe;
        front_fi->streamline(probe);
        const int dim = static_cast<int>(probe.size());

        // Build flat row-major parameter buffer: h_params[i*dim + j] = param j of individual i
        std::vector<double> h_params(static_cast<std::size_t>(N * dim));
        for (int i = 0; i < N; ++i) {
            auto fi = std::dynamic_pointer_cast<gind::GFunctionIndividual>(
                batch[static_cast<std::size_t>(i)]);
            std::vector<double> pv;
            fi->streamline(pv);
            std::copy(pv.begin(), pv.end(),
                      h_params.begin() + static_cast<std::ptrdiff_t>(i * dim));
        }

        std::vector<double> h_results(static_cast<std::size_t>(N));
        cudaCtx_.eval(h_params.data(), h_results.data(), N, dim, funcId);

        // Inject GPU-computed fitness values.
        // process(results_vec) sets fitness and performs all Geneva bookkeeping
        // without triggering fitnessCalculation() — pattern from example 15.
        for (int i = 0; i < N; ++i) {
            batch[static_cast<std::size_t>(i)]->process(
                std::vector<gpar::parameterset_processing_result>(
                    1, gpar::parameterset_processing_result(h_results[static_cast<std::size_t>(i)])));
        }
    }

    //--------------------------------------------------------------------------
    // Data members

    GBenchmarkCUDAContext cudaCtx_; ///< Persistent GPU context (avoids per-generation cudaMalloc)
};

/******************************************************************************/

} /* namespace Gem::Geneva::Benchmarks */
