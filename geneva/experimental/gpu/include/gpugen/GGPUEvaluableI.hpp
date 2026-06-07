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

// Geneva headers
#include "geneva/par/GParameterSet.hpp"

namespace Gem::Geneva::GPU {

/******************************************************************************/
/**
 * EXPERIMENTAL GPU consumer framework -- the problem-specific marshalling interface.
 *
 * The GPU consumer (GGPUConsumer) owns the device plumbing; the *problem* owns how a batch of
 * individuals is turned into flat device buffers and how device results are written back. An image
 * problem and a mathematical-function problem flatten and interpret very differently, so a small
 * marshaller object implementing this interface is supplied per problem. The consumer never needs to
 * know the concrete individual type.
 *
 * Contract / ABI: parameters are laid out row-major, one row of `dim` doubles per item
 * (params[i*dim + j] = parameter j of item i); the kernel writes one double of fitness per item
 * (fitness[i]). `problemConstants()` returns an opaque byte blob the kernel also receives (e.g. a
 * function id, a target image), uploaded once per launch. `hostEvaluate()` is a CPU reference that
 * MUST compute the same fitness as the device kernel -- the CPU backend uses it, so a CUDA/OpenCL run
 * and a CPU run of the same problem can be compared for parity.
 */
class GGPUEvaluableI {
public:
    using item_ptr = std::shared_ptr<Gem::Geneva::Parameters::GParameterSet>;

    virtual ~GGPUEvaluableI() = default;

    /** @brief Flattens every item's parameters into a row-major buffer of n_items * dim doubles.
     *  The dimension is inferred by the consumer as params_out.size() / items.size(), so all items in
     *  a batch must share the same dimension. */
    virtual void flatten(const std::vector<item_ptr> &items, std::vector<double> &params_out) const = 0;

    /** @brief Optional opaque constants the kernel needs (function id, target image, ...). Default: none. */
    [[nodiscard]] virtual std::vector<std::byte> problemConstants() const { return {}; }

    /** @brief Writes the per-item fitness back into each item (typically via item->process(result),
     *  which also leaves the item PROCESSED for the courtier reconciliation). */
    virtual void scatter(const std::vector<item_ptr> &items, const std::vector<double> &fitness) const = 0;

    /** @brief CPU reference evaluation mirroring the device kernel. Used by the CPU backend and for
     *  GPU/CPU parity checks. params is n_items * dim row-major; writes n_items fitness values. */
    virtual void hostEvaluate(
        const double *params, int n_items, int dim,
        const std::byte *pconst, std::size_t pconst_size,
        double *fitness_out) const = 0;
};

/******************************************************************************/

} /* namespace Gem::Geneva::GPU */
