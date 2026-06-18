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

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard headers
#include <cstddef>
#include <memory>

// Boost headers
#include <boost/serialization/export.hpp>
#include <boost/serialization/nvp.hpp>

// Geneva headers
#include "common/GThreadPool.hpp"
#include "courtier/GExecutorStatusT.hpp"
#include "geneva/oa/GEvolutionaryAlgorithm.hpp"

namespace Gem::Geneva::OptimizationAlgorithms {

/******************************************************************************/
/**
 * The meta-optimization evolutionary algorithm: a standard evolutionary algorithm whose population
 * members are "umbrella" individuals, each wrapping and running a sub-optimization (see
 * GMetaOptimizerIndividualT). It is the SINGLE facility for meta-optimization in Geneva.
 *
 * The only thing it changes about GEvolutionaryAlgorithm is WHERE its population is evaluated: a meta-EA
 * evaluates its umbrella-individuals on its OWN orchestration thread pool, NOT through the process-wide
 * work consumer. Each umbrella-individual's evaluation in turn runs a sub-optimization whose fine-grained
 * work items DO go to the one work consumer. So the meta level is just an orchestration pool -- never a
 * second consumer -- which keeps the "one consumer per process" rule intact AND avoids deadlock: the
 * orchestration pool and the work consumer are distinct pools, so an umbrella-individual blocking on its
 * sub-optimization's results (handled by the work consumer) never starves the pool it is running on.
 *
 * It derives from GEvolutionaryAlgorithm to reuse all of the evolutionary machinery (selection, adaption,
 * sorting, personality traits -- its individuals are evolved exactly like an EA's, hence PERSONALITY_EA);
 * it overrides only the per-generation evaluation seam (evaluatePopulationRange_) and clone_().
 */
class GMetaEvolutionaryAlgorithm : public GEvolutionaryAlgorithm {
private:
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        ar &boost::serialization::make_nvp(
            "GEvolutionaryAlgorithm",
            boost::serialization::base_object<GEvolutionaryAlgorithm>(*this));
        // orchestration_pool_ / n_orchestration_threads_ are transient run state: not serialized,
        // not compared (a clone or a resumed algorithm re-establishes the pool on first use).
    }
    ///////////////////////////////////////////////////////////////////////

public:
    // Identifiers consumed by the GOptimizationAlgorithmT scaffold. The personality type stays
    // PERSONALITY_EA: the umbrella-individuals are evolved exactly like an EA's and carry EA traits.
    static constexpr std::string_view oa_class_name = "GMetaEvolutionaryAlgorithm";
    static constexpr std::string_view oa_algorithm_name = "Meta Evolutionary Algorithm";
    static constexpr std::string_view oa_personality_type = "PERSONALITY_EA";

    /** @brief The default constructor */
    GMetaEvolutionaryAlgorithm() = default;
    /** @brief The copy constructor. The orchestration pool is transient run state, so a copy starts
     *  without one (it is re-established on first use); only the evolved EA state is copied.
     *  @param cp The meta-EA to copy */
    GMetaEvolutionaryAlgorithm(const GMetaEvolutionaryAlgorithm &cp)
      : GEvolutionaryAlgorithm(cp)
      , n_orchestration_threads_(cp.n_orchestration_threads_)
    { /* orchestration_pool_ deliberately left null -- rebuilt on first use */ }
    /** @brief The standard destructor */
    ~GMetaEvolutionaryAlgorithm() override = default;

    /** @brief Sets the number of umbrella-individuals evaluated in parallel on the orchestration pool.
     *  @param n The orchestration-pool size; 0 means hardware concurrency */
    void setNOrchestrationThreads(unsigned int n) { n_orchestration_threads_ = n; }
    /** @brief Retrieves the configured orchestration-pool size (0 == hardware concurrency).
     *  @return The configured number of orchestration threads */
    [[nodiscard]] unsigned int getNOrchestrationThreads() const { return n_orchestration_threads_; }

protected:
    /***************************************************************************/
    /**
     * @brief Evaluates the population's [start, end) range on this meta-EA's OWN orchestration thread
     * pool instead of the process-wide work consumer. Each item's process() runs an umbrella-individual
     * (which spawns a sub-optimization that uses the work consumer). A local evaluation never goes
     * MISSING -- process() either succeeds (PROCESSED) or records a caught exception -- so the returned
     * status is always "complete", with the error flag set if any item failed.
     *
     * @param start First population index to evaluate (inclusive)
     * @param end One past the last population index to evaluate
     * @return The executor status: always complete; error flag set if any umbrella-individual failed
     */
    Gem::Courtier::executor_status_t
    evaluatePopulationRange_(std::size_t start, std::size_t end) override;

private:
    /***************************************************************************/
    /** @brief Creates a deep clone of this object. Overridden so the GOptimizationAlgorithmT scaffold's
     *  clone (which would build a plain GEvolutionaryAlgorithm and slice the meta-EA) is not used.
     *  @return A heap-allocated deep copy of this meta-EA, as a GOptimizationAlgorithmBase pointer */
    GOptimizationAlgorithmBase *clone_() const override {
        return new GMetaEvolutionaryAlgorithm(*this);
    }

    /***************************************************************************/
    std::unique_ptr<Gem::Common::GThreadPool> orchestration_pool_; ///< Transient: built on first use
    unsigned int n_orchestration_threads_ = 0; ///< Orchestration-pool size (0 == hardware concurrency)
};

/******************************************************************************/

} /* namespace Gem::Geneva::OptimizationAlgorithms */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::OptimizationAlgorithms::GMetaEvolutionaryAlgorithm) // NOLINT
