/**
 * @file GAlgorithmBenchmarkRunner.hpp
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
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

// Geneva headers
#include "common/GCommonEnums.hpp"
#include "common/GLogger.hpp"
#include "courtier/GBrokerT.hpp"
#include "geneva/GenevaInitializer.hpp"
#include "geneva/ind/GFlatGenome.hpp"
#include "geneva/oa/GOptimizationAlgorithmBase.hpp"
#include "geneva/oa/GEvolutionaryAlgorithmFactory.hpp"
#include "geneva/oa/GConjugateGradientDescentFactory.hpp"
#include "geneva/oa/GSimulatedAnnealingFactory.hpp"
#include "geneva/oa/GSwarmAlgorithmFactory.hpp"
#include "geneva/individuals/GFunctionIndividual.hpp"

// Local headers
#include "GBenchmarkRunResult.hpp"

namespace Gem::Geneva::Benchmarks {

/******************************************************************************/
/**
 * @brief Configuration for one algorithm entry in the benchmark.
 */
struct AlgorithmEntry {
    std::string tag;        ///< Free label, e.g. "ea_pop100"
    std::string mnemonic;   ///< Algorithm type: "ea", "sa", "swarm", or "gd"
    std::string configFile; ///< Path to algorithm-specific JSON config
};

/******************************************************************************/
/**
 * @brief Top-level configuration for GAlgorithmBenchmarkRunner.
 */
struct BenchmarkConfig {
    std::string functionName     {"PARABOLA"};  ///< GFunctionIndividual demo function name
    std::uint32_t nDimensions    {10};          ///< Parameter dimension
    std::size_t   nRuns          {30};          ///< Runs per algorithm entry
    std::string individualConfigFile {"config/GFunctionIndividual.json"};
    std::size_t   batchSize      {0};           ///< 0 = flush by timeout
    std::uint32_t flushTimeoutMs {50};          ///< Flush timeout in ms (batchSize==0)
    std::string outputDir        {"."};         ///< Directory for output CSV files
    std::vector<AlgorithmEntry> algorithms;     ///< Algorithm entries to benchmark
};

/******************************************************************************/
/**
 * @brief Minimal pluggable monitor that captures termination reason and result.
 *
 * Registered per optimization run via algorithm->registerPluggableOM().
 * At INFOEND, extracts the algorithm's final state into public members.
 */
class GBenchmarkTerminationMonitor final : public oa::GBasePluggableOM {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;
        ar &BOOST_SERIALIZATION_BASE_OBJECT_NVP(oa::GBasePluggableOM);
    }
    ///////////////////////////////////////////////////////////////////////

public:
    GBenchmarkTerminationMonitor() = default;
    GBenchmarkTerminationMonitor(const GBenchmarkTerminationMonitor &) = default;
    ~GBenchmarkTerminationMonitor() override = default;

    // Results populated at INFOEND
    double        finalFitness   {0.0};
    std::uint32_t finalIteration {0};
    TerminationReason terminationReason {TerminationReason::Unknown};
    bool targetReached {false};

protected:
    void load_(const oa::GBasePluggableOM *cp) override {
        const auto *p = dynamic_cast<const GBenchmarkTerminationMonitor *>(cp);
        if (!p) throw std::bad_cast{};
        oa::GBasePluggableOM::load_(cp);
        finalFitness      = p->finalFitness;
        finalIteration    = p->finalIteration;
        terminationReason = p->terminationReason;
        targetReached     = p->targetReached;
    }

private:
    oa::GBasePluggableOM *clone_() const override {
        return new GBenchmarkTerminationMonitor(*this);
    }

    void informationFunction_(
        infoMode mode,
        oa::GOptimizationAlgorithmBase const *const goa
    ) override {
        if (mode != infoMode::INFOEND) return;

        finalIteration = goa->getIteration();
        finalFitness   = std::get<0>(goa->getBestKnownPrimaryFitness());

        // stallHalt() and stallHaltSet() are private; replicate with public API:
        // max_stall_iteration_ != 0 guards whether the criterion is active
        const bool stallExceeded =
            (goa->getMaxStallIteration() != 0u) &&
            (goa->getStallCounter() > goa->getMaxStallIteration());

        if (stallExceeded) {
            terminationReason = TerminationReason::StallThreshold;
        } else if (goa->hasQualityThreshold() &&
                   goa->getIteration() < goa->getMaxIteration()) {
            // Ended early with quality threshold active → target was reached
            terminationReason = TerminationReason::QualityThreshold;
            targetReached     = true;
        } else {
            terminationReason = TerminationReason::MaxIterations;
        }
    }
};

/******************************************************************************/
/**
 * @brief Runs sequential multi-algorithm benchmarks using a pre-enrolled consumer.
 *
 * The caller is responsible for:
 *   1. Creating a GenevaInitializer (must outlive this runner).
 *   2. Enrolling a consumer with broker<GOptimizableEntity>() before calling run().
 *
 * The GPU consumer (Gem::Courtier::GPU::GGPUConsumerT, the same one example 15 uses) is created by
 * the caller and passed to the runner via the broker; the runner itself is pure C++ and free of any
 * CUDA build-time dependency (the kernel is runtime-compiled by the consumer's backend).
 *
 * For each algorithm entry and each run, the algorithm is created fresh via the
 * appropriate factory, a GFunctionIndividual is added, and optimize() is called.
 * Results are captured by GBenchmarkTerminationMonitor.
 */
class GAlgorithmBenchmarkRunner {
public:
    /** @brief @p cudaBroker holds the courtier GPU consumer; it is injected into every algorithm
     *  via setBroker() so each optimization submits its population to the GPU. */
    GAlgorithmBenchmarkRunner(
        BenchmarkConfig cfg,
        std::shared_ptr<Gem::Courtier::GBrokerT<gpar::GOptimizableEntity>> cudaBroker);

    /**
     * @brief Runs the full benchmark and returns aggregated results per tag.
     * Prints per-run progress to stdout.
     */
    std::vector<GAlgorithmBenchmarkResult> run();

private:
    GBenchmarkRunResult runOne(
        const AlgorithmEntry &entry,
        std::uint32_t runIdx,
        const gind::GFunctionIndividual::Config &indCfg
    );

    std::shared_ptr<oa::GOptimizationAlgorithmBase>
    makeAlgorithm(const AlgorithmEntry &entry) const;

    static GAlgorithmBenchmarkResult aggregate(
        const std::string &tag,
        const std::string &funcName,
        std::uint32_t nDims,
        const std::vector<GBenchmarkRunResult> &runs
    );

    BenchmarkConfig cfg_;
    std::shared_ptr<Gem::Courtier::GBrokerT<gpar::GOptimizableEntity>> cudaBroker_;
};

/******************************************************************************/

} /* namespace Gem::Geneva::Benchmarks */
