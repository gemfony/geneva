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

// Standard headers
#include <cstddef>
#include <string>

// Geneva headers
#include "courtier/gpu/GGPUDeviceBackendI.hpp"
#include "courtier/gpu/GGPUEvaluableI.hpp"

namespace Gem::Courtier::GPU {

/******************************************************************************/
/**
 * The always-available reference backend: it ignores the kernel file and evaluates the batch on the
 * host by delegating to the marshaller's hostEvaluate(). This lets the whole framework build and run
 * (and be unit-tested) on machines with no GPU toolkit, and provides the CPU reference a GPU run can
 * be compared against. It is single-threaded by design (a parallel CPU path is the existing
 * GStdThreadConsumerT's job); here it exists for correctness and parity, not speed.
 */
class GCPUBackend final : public GGPUDeviceBackendI {
public:
    explicit GCPUBackend(const GGPUHostEvalI *hostEval)
        : hostEval_(hostEval)
    { /* nothing */ }

    void initialize(const KernelSpec & /*spec*/) override {
        // The CPU backend needs no kernel: it runs the marshaller's host reference.
    }

    void evaluate(
        const double *params, int n_items, int dim,
        const std::byte *pconst, std::size_t pconst_size,
        double *fitness_out, int /*threads_per_item*/ = 1) override {
        // The host reference is inherently per-item; intra-item parallelism does not apply.
        hostEval_->hostEvaluate(params, n_items, dim, pconst, pconst_size, fitness_out);
    }

    [[nodiscard]] std::string name() const override { return "cpu"; }

private:
    const GGPUHostEvalI *hostEval_; ///< Non-owning; outlives the backend (the marshaller, held by the consumer)
};

/******************************************************************************/

} /* namespace Gem::Courtier::GPU */
