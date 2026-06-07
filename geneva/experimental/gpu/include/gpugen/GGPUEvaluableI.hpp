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
#include <memory>
#include <vector>

namespace Gem::Courtier::GPU {

/******************************************************************************/
/**
 * EXPERIMENTAL GPU consumer framework. Logically a Gem::Courtier consumer (it derives from
 * GBaseConsumerT), so it lives in Gem::Courtier::GPU and is generic in the processable type, with NO
 * dependency on the Gem::Geneva layer (which sits above courtier). The concrete, GParameterSet-aware
 * marshallers live with the problems (the demos), not here.
 *
 * GGPUHostEvalI is the NON-templated host-reference part of a marshaller: it evaluates a batch in flat
 * doubles only (no individuals). The CPU backend and the backend factory depend only on this, so they
 * stay non-templated and free of the processable type.
 */
class GGPUHostEvalI {
public:
    virtual ~GGPUHostEvalI() = default;

    /** @brief CPU reference evaluation mirroring the device kernel. params is n_items * dim row-major;
     *  writes n_items fitness values. Used by the CPU backend and for GPU/CPU parity checks. */
    virtual void hostEvaluate(
        const double *params, int n_items, int dim,
        const std::byte *pconst, std::size_t pconst_size,
        double *fitness_out) const = 0;
};

/******************************************************************************/
/**
 * The problem-specific marshalling interface, generic in the processable type. The GPU consumer owns
 * the device plumbing; the problem owns how a batch of items becomes flat device buffers and how
 * device results are written back.
 *
 * ABI: parameters are row-major, one row of `dim` doubles per item (params[i*dim + j] = parameter j of
 * item i); the kernel writes one fitness double per item. problemConstants() is an opaque byte blob
 * the kernel also receives (function id, target image, ...), uploaded once per launch. hostEvaluate()
 * (inherited) must compute the same fitness as the device kernel.
 */
template <typename processable_type>
class GGPUEvaluableI : public GGPUHostEvalI {
public:
    using item_ptr = std::shared_ptr<processable_type>;

    /** @brief Flattens every item's parameters into a row-major buffer of n_items * dim doubles. The
     *  dimension is inferred by the consumer as params_out.size() / items.size(). */
    virtual void flatten(const std::vector<item_ptr> &items, std::vector<double> &params_out) const = 0;

    /** @brief Optional opaque constants the kernel needs. Default: none. */
    [[nodiscard]] virtual std::vector<std::byte> problemConstants() const { return {}; }

    /** @brief Writes the per-item fitness back into each item (typically via item->process(result),
     *  which also leaves the item PROCESSED for the courtier reconciliation). */
    virtual void scatter(const std::vector<item_ptr> &items, const std::vector<double> &fitness) const = 0;
};

/******************************************************************************/

} /* namespace Gem::Courtier::GPU */
