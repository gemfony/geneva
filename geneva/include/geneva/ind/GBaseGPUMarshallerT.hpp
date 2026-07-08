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

// Standard header files go here
#include <cstddef>
#include <ranges>
#include <span>
#include <vector>

// Geneva headers go here
#include "common/GCommonHelperFunctions.hpp"
#include "common/GErrorStreamer.hpp"
#include "common/GExceptions.hpp"
#include "common/GLogger.hpp"
#include "courtier/gpu/GGPUEvaluableI.hpp"
#include "geneva/ind/GFlatGenome.hpp"
#include "geneva/ind/GOptimizableEntity.hpp"

namespace Gem::Geneva {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * @brief Base scaffolding for a GPU marshaller of flat-genome individuals.
 *
 * The marshaller is the accelerator ADAPTER: it prepares a batch of individuals for the device (flatten)
 * and reconciles the device results back (scatter); it never runs the evaluation itself (the backend/kernel
 * does). This base supplies the parts of that adapter that are identical for every flat-genome problem, so a
 * concrete problem marshaller adds only the genuinely device-specific pieces:
 *
 *  - @c problemConstants() / @c problemConstantsStatic() -- the opaque blob the kernel also needs (default:
 *    none, inherited);
 *  - @c parallelWorkPerItem() -- intra-item device parallelism (default: 1, inherited);
 *  - @c hostEvaluate() -- the CPU reference mirroring the device kernel (still pure-virtual here).
 *
 * The generic plumbing provided here:
 *  - @c itemDimension() -- the flattened scalar count of one individual (its parameter count);
 *  - @c flatten() -- streamlines each individual's EXTERNAL (range-folded) values row-major straight into
 *    the device buffer. Because the buffer element is @c scalar_type, the individual's @c long @c double /
 *    @c double genome is converted to @c scalar_type here (the FP conversion), so the author's @c evaluate()
 *    stays in full precision while the device runs at @c scalar_type;
 *  - @c scatter() -- injects each device-computed fitness through the individual's own @c process() results
 *    channel (which leaves the item PROCESSED for the courtier reconciliation).
 *
 * This sits in @c geneva (not @c courtier) because the generic flatten/scatter necessarily know the geneva
 * individual model (@c GFlatGenome, @c individual_processing_result); it derives from courtier's
 * genome-agnostic @c GGPUEvaluableI, which stays free of the geneva layer.
 *
 * @tparam scalar_type The device ABI / flat-buffer element type (double for full parity, float for FP32 speed)
 */
template <typename scalar_type = double>
class GBaseGPUMarshallerT
  : public Gem::Courtier::GPU::GGPUEvaluableI<Gem::Geneva::Genome::GOptimizableEntity, scalar_type> {
    using base_type = Gem::Courtier::GPU::GGPUEvaluableI<Gem::Geneva::Genome::GOptimizableEntity, scalar_type>;

public:
    using item_ptr = typename base_type::item_ptr;

    /***************************************************************************/
    /**
     * @brief The flattened scalar count of one individual (its parameter count in @c scalar_type).
     * @param item The individual whose flattened dimension is queried
     * @return The number of @c scalar_type values @c flatten() writes for this item
     */
    [[nodiscard]] std::size_t itemDimension(const item_ptr &item) const override {
        return item->template countParameters<scalar_type>();
    }

    /***************************************************************************/
    /**
     * @brief Streamlines every item's external values row-major into @p params_out (@c scalar_type each).
     *
     * @param items The batch to flatten (every item must be a GFlatGenome of uniform dimension)
     * @param params_out Filled with @c items.size() * itemDimension() scalars (parameter j of item i at i*dim + j)
     */
    void flatten(std::span<const item_ptr> items, std::vector<scalar_type> &params_out) const override {
        if(items.empty()) {
            params_out.clear();
            return;
        }
        const std::size_t dim = this->itemDimension(items.front());
        params_out.resize(items.size() * dim);
        for(std::size_t i = 0; i < items.size(); ++i) {
            const auto *flat = dynamic_cast<const Gem::Geneva::Genome::GFlatGenome *>(items[i].get());
            if(flat == nullptr) {
                throw geneva_exception(
                    g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                    << "In GBaseGPUMarshallerT::flatten(): Error!" << '\n'
                    << "A work item is not a GFlatGenome; a GPU marshaller requires flat-genome"
                    << " individuals." << '\n'
                );
            }
            // streamlineInto() writes the external (range-folded) values as scalar_type -- no per-item
            // temporary, and the double-to-scalar_type conversion happens here.
            flat->streamlineInto(params_out.data() + i * dim);
        }
    }

    /***************************************************************************/
    /**
     * @brief Injects each device-computed fitness back into its item via the item's process() results channel.
     * @param items The batch the fitness values belong to (in order)
     * @param fitness One fitness scalar per item, produced by the device/host evaluation
     */
    void scatter(std::span<const item_ptr> items, const std::vector<scalar_type> &fitness) const override {
        for(auto const &[item, fit] : std::views::zip(items, fitness)) {
            item->process(std::vector<Gem::Geneva::Genome::individual_processing_result>(
                1, Gem::Geneva::Genome::individual_processing_result(fit)));
        }
    }
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Gem::Geneva */
