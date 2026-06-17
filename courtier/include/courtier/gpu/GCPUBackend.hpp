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
 *
 * @tparam scalar_type The floating-point scalar the backend operates on (defaults to double).
 */
template <typename scalar_type = double>
class GCPUBackend final : public GGPUDeviceBackendI<scalar_type> {
public:
    /**
     * @brief Constructs the CPU backend.
     * @param hostEval The host-evaluation interface to delegate to (non-owning; must outlive the backend)
     */
    explicit GCPUBackend(const GGPUHostEvalI<scalar_type> *hostEval)
        : hostEval_(hostEval)
    { /* nothing */ }

    /**
     * @brief Initializes the backend. A no-op for the CPU backend, which needs no kernel.
     * @param spec The kernel specification (ignored: the CPU backend runs the marshaller's host reference)
     */
    void initialize(const KernelSpec & /*spec*/) override {
        // The CPU backend needs no kernel: it runs the marshaller's host reference.
    }

    /**
     * @brief Evaluates a batch of items on the host by delegating to the host-evaluation interface.
     *
     * @param params Flat array of parameters for all items (n_items * dim values)
     * @param n_items The number of items in the batch
     * @param dim The number of parameters per item
     * @param pconst Pointer to the per-batch constant data (problem constants), or nullptr if none
     * @param pconst_size Size in bytes of the constant data pointed to by pconst
     * @param fitness_out Output array receiving one fitness value per item (n_items values)
     * @param threads_per_item Requested intra-item parallelism (ignored: the host reference is per-item)
     */
    void evaluate(
        const scalar_type *params, int n_items, int dim,
        const std::byte *pconst, std::size_t pconst_size,
        scalar_type *fitness_out, int /*threads_per_item*/ = 1) override {
        // The host reference is inherently per-item; intra-item parallelism does not apply.
        hostEval_->hostEvaluate(params, n_items, dim, pconst, pconst_size, fitness_out);
    }

    /**
     * @brief The human-readable name of this backend.
     * @return The string "cpu".
     */
    [[nodiscard]] std::string name() const override { return "cpu"; }

private:
    const GGPUHostEvalI<scalar_type> *hostEval_; ///< Non-owning; outlives the backend (the marshaller, held by the consumer)
};

/******************************************************************************/

} /* namespace Gem::Courtier::GPU */
