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

#include "geneva/oa/GMetaEvolutionaryAlgorithm.hpp"

// Standard headers
#include <algorithm>
#include <thread>

#include "common/GExpectationChecksT.hpp"
#include "geneva/ind/GOptimizableEntity.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::OptimizationAlgorithms::GMetaEvolutionaryAlgorithm) // NOLINT

namespace Gem::Geneva::OptimizationAlgorithms {

/******************************************************************************/

Gem::Courtier::submission_status_t
GMetaEvolutionaryAlgorithm::evaluatePopulationRange_(std::size_t start, std::size_t end) {
    // Build the orchestration pool on first use (transient run state, sized to hardware concurrency by
    // default). It is this meta-EA's OWN pool -- separate from the process-wide work consumer the
    // umbrella-individuals' sub-optimizations submit to -- which is what keeps the two tiers deadlock-free.
    if(not orchestration_pool_) {
        const unsigned int n = n_orchestration_threads_ != 0
                                   ? n_orchestration_threads_
                                   : std::max(1u, std::thread::hardware_concurrency());
        orchestration_pool_ = std::make_unique<Gem::Common::Concurrency::GThreadPool>(n);
    }

    // Run each umbrella-individual's process() on the orchestration pool. process() funnels any thrown
    // exception into the item's status (EXCEPTION_CAUGHT) rather than letting it escape the worker.
    for(std::size_t i = start; i < end; ++i) {
        auto *ind = &((*this->at(i)));
        orchestration_pool_->post([ind]() { ind->process(); });
    }
    orchestration_pool_->wait();

    // A local evaluation never goes MISSING: every slot is PROCESSED or carries a caught error. Report
    // "complete", flagging errors so the base evaluatePopulation_ removes any failed umbrella-individual.
    bool has_errors = false;
    for(std::size_t i = start; i < end; ++i) {
        if(this->at(i)->has_errors()) {
            has_errors = true;
            break;
        }
    }
    return Gem::Courtier::submission_status_t{.is_complete=true, .has_errors=has_errors};
}

/******************************************************************************/
/**
 * @brief Loads the data of another GMetaEvolutionaryAlgorithm, camouflaged as a
 * GOptimizationAlgorithmBase. The local config members travel through localMembers_() -- the same
 * single source serialize()/compare_()/the copy constructor use; the transient orchestration pool is
 * not copied (it is re-established on first use).
 *
 * @param cp A pointer to another GMetaEvolutionaryAlgorithm, camouflaged as a GOptimizationAlgorithmBase
 */
void GMetaEvolutionaryAlgorithm::load_(const GOptimizationAlgorithmBase *cp) {
    const GMetaEvolutionaryAlgorithm *p_load = Gem::Common::g_convert_and_compare<
        GOptimizationAlgorithmBase, GMetaEvolutionaryAlgorithm>(cp, this);

    GEvolutionaryAlgorithm::load_(cp);
    Gem::Common::g_load_members(this->localMembers_(), p_load->localMembers_());
}

/******************************************************************************/
/**
 * @brief Searches for compliance with expectations with respect to another object of the same type.
 *
 * @param cp A constant reference to another object, camouflaged as a GOptimizationAlgorithmBase
 * @param e The expected outcome of the comparison (equality, inequality, ...)
 * @param limit The maximum acceptable deviation for floating-point comparisons (unused here)
 */
void GMetaEvolutionaryAlgorithm::compare_(
    const GOptimizationAlgorithmBase &cp,
    const Gem::Common::expectation &e,
    const double & /*limit*/
) const {
    using namespace Gem::Common;

    const GMetaEvolutionaryAlgorithm *p_load = Gem::Common::g_convert_and_compare<
        GOptimizationAlgorithmBase, GMetaEvolutionaryAlgorithm>(cp, this);

    GToken token("GMetaEvolutionaryAlgorithm", e);

    Gem::Common::compare_base_t<GEvolutionaryAlgorithm>(*this, *p_load, token);
    g_compare_members(this->localMembers_(), p_load->localMembers_(), token);

    token.evaluate();
}

/******************************************************************************/

} /* namespace Gem::Geneva::OptimizationAlgorithms */
