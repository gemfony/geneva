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
#include "geneva/par/GParameterSet.hpp"
#include "courtier/gpu/GGPUEvaluableI.hpp"
#include "GMonaLisaProblem.hpp"

namespace gpar = Gem::Geneva::Parameters;

namespace Gem::Geneva::MonaLisa {

/******************************************************************************/
/**
 * GGPUEvaluableI marshaller for the experimental Mona-Lisa (example-15) port.
 *
 * flatten()           streamlines each individual's genome (10*NT triangle params + 3 background) into
 *                     a row-major buffer (alpha-sort is disabled on the individual, so the streamline
 *                     order is the canonical render order the kernel expects).
 * problemConstants()  packs, as doubles, [W, H, target(W*H*3)] -- the loaded target image; uploaded
 *                     once per launch. NT and the background come from the per-item parameters.
 * hostEvaluate()      the CPU reference -- calls the SAME score() the individual's fitnessCalculation
 *                     and the device kernel use.
 * scatter()           injects the device-computed fitness via process() (leaves each item PROCESSED).
 *
 * One kernel launch per batch (one thread per individual): minimal kernels, full bulk.
 */
class GMonaLisaGPUMarshaller final
  : public Gem::Courtier::GPU::GGPUEvaluableI<gpar::GParameterSet> {
public:
    void flatten(const std::vector<item_ptr> &items, std::vector<double> &params_out) const override {
        if(items.empty()) {
            params_out.clear();
            return;
        }
        std::vector<double> pv;
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
        std::vector<double> blob;
        blob.reserve(2 + t.rgb.size());
        blob.push_back(static_cast<double>(t.width));
        blob.push_back(static_cast<double>(t.height));
        blob.insert(blob.end(), t.rgb.begin(), t.rgb.end());

        std::vector<std::byte> bytes(blob.size() * sizeof(double));
        std::memcpy(bytes.data(), blob.data(), bytes.size());
        return bytes;
    }

    /** @brief Use intra-item (pixel-stripe) parallelism: 256 cooperating threads per individual, so a
     *  small population still fills the GPU. The CUDA backend launches n_items*256 threads and the
     *  kernel atomic-accumulates each item's fitness; the CPU/OpenCL backends clamp this to 1. */
    [[nodiscard]] int parallelWorkPerItem() const override { return 256; }

    void scatter(const std::vector<item_ptr> &items, const std::vector<double> &fitness) const override {
        for(std::size_t i = 0; i < items.size(); ++i) {
            items[i]->process(std::vector<gpar::parameterset_processing_result>(
                1, gpar::parameterset_processing_result(fitness[i])));
        }
    }

    void hostEvaluate(
        const double *params, int n_items, int dim,
        const std::byte * /*pconst*/, std::size_t /*pconst_size*/,
        double *fitness_out) const override {
        const Target &t = target();
        std::vector<double> scratch;
        for(int i = 0; i < n_items; ++i) {
            fitness_out[i] = score(params + static_cast<std::size_t>(i) * static_cast<std::size_t>(dim),
                                   dim, t.width, t.height, t.rgb.data(), scratch);
        }
    }
};

/******************************************************************************/

} /* namespace Gem::Geneva::MonaLisa */
