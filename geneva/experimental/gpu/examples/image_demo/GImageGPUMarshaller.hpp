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
#include <memory>
#include <vector>

// Geneva headers
#include "geneva/par/GParameterSet.hpp"
#include "courtier/gpu/GGPUEvaluableI.hpp"
#include "GExpImageProblem.hpp"

namespace gpar = Gem::Geneva::Parameters;

namespace Gem::Courtier::GPU::ImageDemo {

/******************************************************************************/
/**
 * GGPUEvaluableI marshaller for the experimental image-fitness problem.
 *
 * flatten()           gathers every individual's IMG_DIM triangle parameters into a row-major buffer.
 * problemConstants()  packs, as doubles, [W, H, T, bgR, bgG, bgB, target(W*H*3)] -- everything the
 *                     kernel needs besides the per-item parameters; uploaded once per launch.
 * scatter()           injects the device-computed fitness via process() (leaves each item PROCESSED).
 * hostEvaluate()      the CPU reference -- it calls the SAME renderScore() the individual's
 *                     fitnessCalculation() and the device kernel use, so CPU and GPU agree.
 *
 * The whole batch is evaluated in ONE kernel launch (one thread per individual), the minimal-kernel /
 * maximal-bulk design.
 */
class GImageGPUMarshaller final : public Gem::Courtier::GPU::GGPUEvaluableI<gpar::GParameterSet> {
public:
    void flatten(const std::vector<item_ptr> &items, std::vector<double> &params_out) const override {
        params_out.resize(items.size() * static_cast<std::size_t>(IMG_DIM));
        std::vector<double> pv;
        for(std::size_t i = 0; i < items.size(); ++i) {
            items[i]->streamline(pv);
            // Copy the first IMG_DIM values (the triangle parameters).
            for(int j = 0; j < IMG_DIM; ++j) {
                params_out[i * static_cast<std::size_t>(IMG_DIM) + static_cast<std::size_t>(j)] =
                    pv[static_cast<std::size_t>(j)];
            }
        }
    }

    [[nodiscard]] std::vector<std::byte> problemConstants() const override {
        const std::vector<double> &target = targetImage();
        std::vector<double> blob;
        blob.reserve(6 + target.size());
        blob.push_back(static_cast<double>(IMG_W));
        blob.push_back(static_cast<double>(IMG_H));
        blob.push_back(static_cast<double>(IMG_T));
        blob.push_back(BG_R);
        blob.push_back(BG_G);
        blob.push_back(BG_B);
        blob.insert(blob.end(), target.begin(), target.end());

        std::vector<std::byte> bytes(blob.size() * sizeof(double));
        std::memcpy(bytes.data(), blob.data(), bytes.size());
        return bytes;
    }

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
        const double *target = targetImage().data();
        for(int i = 0; i < n_items; ++i) {
            fitness_out[i] = renderScore(params + static_cast<std::size_t>(i) * static_cast<std::size_t>(dim), target);
        }
    }
};

/******************************************************************************/

} /* namespace Gem::Courtier::GPU::ImageDemo */
