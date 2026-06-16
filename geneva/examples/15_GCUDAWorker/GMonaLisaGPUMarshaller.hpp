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
#include <cstring>
#include <vector>

// Geneva headers
#include "geneva/ind/GOptimizableEntity.hpp"
#include "courtier/gpu/GGPUEvaluableI.hpp"
#include "GImageScalar.hpp"
#include "GMonaLisaProblem.hpp"

namespace gpar = Gem::Geneva::Parameters;

namespace Gem::Geneva::MonaLisa {

/******************************************************************************/
/**
 * GGPUEvaluableI marshaller for the Mona-Lisa problem of example 15.
 *
 * flatten()           streamlines each individual's genome (10*NT triangle params + 3 background) into
 *                     a row-major scalar buffer (alpha-sort is disabled on the individual, so the
 *                     streamline order is the canonical render order the kernel expects).
 * problemConstants()  packs, as scalars, [W, H, target(W*H*3)] -- the loaded target image; uploaded
 *                     once per launch. NT and the background come from the per-item parameters.
 * hostEvaluate()      the CPU reference -- calls the SAME score() the individual's
 *                     fitnessCalculation and the device kernel use.
 * scatter()           injects the device-computed fitness via process() (leaves each item PROCESSED).
 *
 * One kernel launch per batch: minimal kernels, full bulk.
 *
 * The scalar type (gimage_fp_t, see GImageScalar.hpp) is selected at COMPILE TIME: DOUBLE by default,
 * or FLOAT when the example is built with GIMAGE_USE_FLOAT. The marshaller is that scalar end-to-end:
 * GGPUEvaluableI<gpar::GOptimizableEntity, gimage_fp_t>, so the genome and fitness flat buffers, the device
 * ABI and the CUDA kernel all use it -- no widening/narrowing. The matching device kernel is selected
 * through the default GPU-consumer config.
 */
class GMonaLisaGPUMarshaller final
  : public Gem::Courtier::GPU::GGPUEvaluableI<gpar::GOptimizableEntity, gimage_fp_t> {
public:
    /** @brief The flattened dimension of one image genome: every value is a gimage_fp_t, so the count of
     *  gimage_fp_t parameters is exactly what flatten() streams per item. The consumer uses this to
     *  enforce a uniform geometry across the batch. */
    [[nodiscard]] std::size_t itemDimension(const item_ptr &item) const override {
        return item->countParameters<gimage_fp_t>();
    }

    void flatten(const std::vector<item_ptr> &items, std::vector<gimage_fp_t> &params_out) const override {
        if(items.empty()) {
            params_out.clear();
            return;
        }
        std::vector<gimage_fp_t> pv;
        // streamline<T> is a storage-agnostic method on GOptimizableEntity, so the flat GImageIndividual
        // genome streamlines through the base pointer directly (no genome-model downcast).
        items.front()->streamline(pv);
        const std::size_t dim = pv.size();
        params_out.resize(items.size() * dim);
        for(std::size_t i = 0; i < items.size(); ++i) {
            items[i]->streamline(pv);
            std::copy(pv.begin(), pv.end(),
                      params_out.begin() + static_cast<std::ptrdiff_t>(i * dim));
        }
    }

    [[nodiscard]] std::vector<std::byte> problemConstants() const override {
        const Target &t = target();
        std::vector<gimage_fp_t> blob;
        blob.reserve(2 + t.rgb.size());
        blob.push_back(static_cast<gimage_fp_t>(t.width));
        blob.push_back(static_cast<gimage_fp_t>(t.height));
        // The target image already uses the selected scalar -- pack it natively.
        blob.insert(blob.end(), t.rgb.begin(), t.rgb.end());

        std::vector<std::byte> bytes(blob.size() * sizeof(gimage_fp_t));
        std::memcpy(bytes.data(), blob.data(), bytes.size());
        return bytes;
    }

    /** @brief Use intra-item (pixel-stripe) parallelism: 256 cooperating threads per individual, so a
     *  small population still fills the GPU. The CUDA backend launches n_items*256 threads and the
     *  kernel atomic-accumulates each item's fitness; the CPU/OpenCL backends clamp this to 1. */
    [[nodiscard]] int parallelWorkPerItem() const override { return 256; }

    void scatter(const std::vector<item_ptr> &items, const std::vector<gimage_fp_t> &fitness) const override {
        for(std::size_t i = 0; i < items.size(); ++i) {
            items[i]->process(std::vector<gpar::individual_processing_result>(
                1, gpar::individual_processing_result(fitness[i])));
        }
    }

    void hostEvaluate(
        const gimage_fp_t *params, int n_items, int dim,
        const std::byte * /*pconst*/, std::size_t /*pconst_size*/,
        gimage_fp_t *fitness_out) const override {
        const Target &t = target();
        std::vector<gimage_fp_t> scratch;
        for(int i = 0; i < n_items; ++i) {
            // Native scalar: the params buffer already uses gimage_fp_t, so feed it straight to score().
            const gimage_fp_t *src = params + static_cast<std::size_t>(i) * static_cast<std::size_t>(dim);
            fitness_out[i] = score(src, dim, t.width, t.height, t.rgb.data(), scratch);
        }
    }
};

/******************************************************************************/

} /* namespace Gem::Geneva::MonaLisa */
