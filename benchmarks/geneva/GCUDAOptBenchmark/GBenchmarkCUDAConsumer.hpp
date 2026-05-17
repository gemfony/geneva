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

#include <chrono>
#include <memory>
#include <thread>
#include <vector>

// Boost headers (required by GBaseConsumerT)
#include <boost/program_options.hpp>

// Geneva headers
#include "common/GGlobalDefines.hpp"
#include "common/GLogger.hpp"
#include "courtier/consumers/GBaseConsumerT.hpp"
#include "courtier/GBrokerT.hpp"
#include "geneva/par/GParameterSet.hpp"
#include "geneva-individuals/GFunctionIndividual.hpp"

// Local CUDA evaluator
#include "GBenchmarkBatchEvaluator.cuh"

namespace Gem::Geneva {

/******************************************************************************/
/**
 * @brief Broker-integrated GPU batch consumer for GFunctionIndividual.
 *
 * Registered with GBROKER(GParameterSet) — the same broker type Go2 uses.
 * Work items arrive as GParameterSet; they are cast to GFunctionIndividual
 * internally to extract demoFunction and parameters.
 *
 * Batch flushing policy:
 *   - batchSize_ > 0: flush when exactly batchSize_ items have arrived.
 *   - batchSize_ == 0: flush flushTimeout_ ms after the first item arrives,
 *     with whatever has accumulated by then.
 *
 * The persistent GBenchmarkCUDAContext avoids repeated cudaMalloc across
 * generations.
 *
 * The pre-computed GPU fitness is injected via
 *   individual->process(vector<parameterset_processing_result>{fitness})
 * following the pattern from examples/geneva/15_GCUDAWorker.
 * This bypasses fitnessCalculation() while performing all Geneva bookkeeping.
 * GFunctionIndividual itself is not modified and remains CPU-runnable.
 */
class GCUDABatchConsumer
    : public cons::GBaseConsumerT<gpar::GParameterSet>
{
    using individual_t = gpar::GParameterSet;
    using base_t       = cons::GBaseConsumerT<individual_t>;

public:
    //--------------------------------------------------------------------------

    GCUDABatchConsumer() = default;
    ~GCUDABatchConsumer() override = default;

    GCUDABatchConsumer(const GCUDABatchConsumer &) = delete;
    GCUDABatchConsumer &operator=(const GCUDABatchConsumer &) = delete;

    //--------------------------------------------------------------------------
    /**
     * @brief Sets the target batch size.
     *
     * 0 (default): flush by timeout — the consumer flushes flushTimeout_ ms
     *   after the first item of a new batch arrives.
     * >0: flush when exactly that many items have accumulated, regardless of time.
     *   Recommended value: population size of the optimizer being benchmarked.
     */
    void setBatchSize(std::size_t sz) { batchSize_ = sz; }
    std::size_t getBatchSize() const  { return batchSize_; }

    //--------------------------------------------------------------------------
    /**
     * @brief Sets the flush timeout.
     *
     * When batchSize_ == 0, the consumer flushes after this duration from the
     * first item's arrival. Also used as the per-item get-timeout in the inner
     * accumulation loop. Default: 50 ms.
     */
    void setFlushTimeout(std::chrono::milliseconds t) { flushTimeout_ = t; }

    //--------------------------------------------------------------------------
    /**
     * @brief Convenience factory: creates and enrolls a consumer with the broker.
     *
     * Must be called BEFORE constructing Go2 so that Go2's hasConsumers() check
     * finds the consumer already registered and skips its own enrollment.
     */
    static std::shared_ptr<GCUDABatchConsumer> setup() {
        auto consumer_ptr = std::make_shared<GCUDABatchConsumer>();
        GBROKER(gpar::GParameterSet)->enrol_consumer(consumer_ptr);
        return consumer_ptr;
    }

protected:
    //--------------------------------------------------------------------------
    void shutdown_() override {
        base_t::shutdown_();
        if (processingThread_.joinable()) {
            processingThread_.join();
        }
    }

private:
    //--------------------------------------------------------------------------
    // GBaseConsumerT pure-virtual interface

    void addCLOptions_(
        boost::program_options::options_description & /*visible*/,
        boost::program_options::options_description & /*hidden*/
    ) override { /* no command-line options */ }

    void actOnCLOptions_(
        const boost::program_options::variables_map & /*vm*/
    ) override { /* nothing */ }

    std::string getConsumerName_() const override { return "GCUDABatchConsumer"; }
    std::string getMnemonic_()     const override { return "cuda"; }

    bool capableOfFullReturn_() const override { return true; }

    std::size_t getNProcessingUnitsEstimate_(bool &exact) const override {
        exact = true;
        return std::size_t{1}; // one GPU
    }

    //--------------------------------------------------------------------------
    void async_startProcessing_() override {
        glogger << "GCUDABatchConsumer: starting GPU batch worker thread" << std::endl
                << GLOGGING;
        processingThread_ = std::thread([this]() { workerLoop(); });
    }

    //--------------------------------------------------------------------------
    void workerLoop() {
        while (!this->stopped()) {
            auto batch = collectBatch();
            if (batch.empty()) continue;
            evaluateBatchOnGPU(batch);
            returnBatchToBroker(batch);
        }
    }

    //--------------------------------------------------------------------------
    /**
     * @brief Collects a batch of individuals from the broker.
     *
     * Blocks on the first item (checking stopped() every flushTimeout_ ms).
     * Once the first item arrives the flush deadline is set. Further items are
     * collected with a short 10 ms inner timeout until the deadline expires
     * (batchSize_ == 0) or batchSize_ items have arrived (batchSize_ > 0).
     */
    std::vector<std::shared_ptr<individual_t>> collectBatch() {
        std::vector<std::shared_ptr<individual_t>> batch;

        // Wait for first item
        std::shared_ptr<individual_t> first;
        while (!this->stopped() && !first) {
            broker_ptr_->get(first, flushTimeout_);
        }
        if (!first) return batch; // stopped() fired before any item arrived

        batch.push_back(std::move(first));

        const auto deadline = std::chrono::steady_clock::now() + flushTimeout_;
        constexpr auto innerTimeout = std::chrono::milliseconds{10};

        while (!this->stopped()) {
            if (batchSize_ > 0 && batch.size() >= batchSize_) break;
            if (batchSize_ == 0 && std::chrono::steady_clock::now() >= deadline) break;

            std::shared_ptr<individual_t> p;
            broker_ptr_->get(p, innerTimeout);
            if (p) batch.push_back(std::move(p));
        }

        return batch;
    }

    //--------------------------------------------------------------------------
    /**
     * @brief GPU-evaluates all individuals in the batch in one kernel launch.
     *
     * Items arrive as GParameterSet; we cast to GFunctionIndividual to extract
     * demoFunction (for the kernel funcId) and double parameters.
     * Results are injected back via process(vector<parameterset_processing_result>{fitness})
     * on the base GParameterSet pointer — pattern from examples/geneva/15_GCUDAWorker.
     */
    void evaluateBatchOnGPU(std::vector<std::shared_ptr<individual_t>> &batch) {
        const int N = static_cast<int>(batch.size());

        // Cast first item to GFunctionIndividual to get demoFunction and dimension.
        // All items in one batch share the same function and dimension.
        auto front_fi = std::dynamic_pointer_cast<Gem::Geneva::GFunctionIndividual>(batch.front());
        if (!front_fi) {
            throw std::runtime_error("GCUDABatchConsumer: batch item is not a GFunctionIndividual");
        }
        // solverFunction enum values equal BM::FUNC_* integer constants (both 0–14)
        const int funcId = static_cast<int>(front_fi->getDemoFunction());

        std::vector<double> probe;
        front_fi->streamline(probe);
        const int dim = static_cast<int>(probe.size());

        // Build flat row-major parameter buffer: h_params[i*dim + j] = param j of individual i
        std::vector<double> h_params(static_cast<std::size_t>(N * dim));
        for (int i = 0; i < N; ++i) {
            auto fi = std::dynamic_pointer_cast<Gem::Geneva::GFunctionIndividual>(
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
    void returnBatchToBroker(std::vector<std::shared_ptr<individual_t>> &batch) {
        for (auto &ind : batch) {
            broker_ptr_->put(ind, flushTimeout_);
        }
    }

    //--------------------------------------------------------------------------
    // Data members

    std::thread               processingThread_;
    GBenchmarkCUDAContext     cudaCtx_;

    std::size_t               batchSize_   {0};
    std::chrono::milliseconds flushTimeout_{50};

    std::shared_ptr<Gem::Courtier::GBrokerT<individual_t>> broker_ptr_
        = GBROKER(gpar::GParameterSet);
};

/******************************************************************************/

} /* namespace Gem::Geneva */
