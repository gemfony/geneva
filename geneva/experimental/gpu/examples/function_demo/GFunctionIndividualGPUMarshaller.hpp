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
#include "geneva/individuals/GFunctionIndividual.hpp"
#include "geneva/par/GParameterSet.hpp"
#include "gpugen/GGPUEvaluableI.hpp"

namespace gind = Gem::Geneva::Individuals;
namespace gpar = Gem::Geneva::Parameters;

namespace Gem::Geneva::GPU::Demo {

/******************************************************************************/
/**
 * A GGPUEvaluableI marshaller for GFunctionIndividual batches. It flattens every individual's
 * parameters via streamline(), passes a small "function id" as the opaque problem constant, and
 * injects the device-computed fitness back via process() (which leaves each item PROCESSED, exactly
 * what the courtier reconciliation reads -- the pattern the existing CUDA benchmark consumer uses).
 *
 * The host reference (hostEvaluate) mirrors the device kernel (function_eval.cu / .cl): function id
 * 0 = parabola (sum of squares), 1 = Rosenbrock. Because both the kernel and the host reference are
 * defined here, a CUDA/OpenCL run and a CPU run produce identical fitness -- the demo asserts that.
 */
class GFunctionIndividualGPUMarshaller final : public GGPUEvaluableI {
public:
    explicit GFunctionIndividualGPUMarshaller(int funcId = 0) : funcId_(funcId) {}

    void flatten(const std::vector<item_ptr> &items, std::vector<double> &params_out) const override {
        if(items.empty()) {
            params_out.clear();
            return;
        }
        std::vector<double> pv;
        auto first = std::dynamic_pointer_cast<gind::GFunctionIndividual>(items.front());
        first->streamline(pv);
        const std::size_t dim = pv.size();
        params_out.resize(items.size() * dim);
        for(std::size_t i = 0; i < items.size(); ++i) {
            auto fi = std::dynamic_pointer_cast<gind::GFunctionIndividual>(items[i]);
            fi->streamline(pv);
            std::copy(pv.begin(), pv.end(),
                      params_out.begin() + static_cast<std::ptrdiff_t>(i * dim));
        }
    }

    [[nodiscard]] std::vector<std::byte> problemConstants() const override {
        std::vector<std::byte> b(sizeof(int));
        std::memcpy(b.data(), &funcId_, sizeof(int));
        return b;
    }

    void scatter(const std::vector<item_ptr> &items, const std::vector<double> &fitness) const override {
        for(std::size_t i = 0; i < items.size(); ++i) {
            items[i]->process(std::vector<gpar::parameterset_processing_result>(
                1, gpar::parameterset_processing_result(fitness[i])));
        }
    }

    void hostEvaluate(
        const double *params, int n_items, int dim,
        const std::byte *pconst, std::size_t pconst_size,
        double *fitness_out) const override {
        int fid = 0;
        if(pconst_size >= sizeof(int)) {
            std::memcpy(&fid, pconst, sizeof(int));
        }
        for(int i = 0; i < n_items; ++i) {
            const double *x = params + static_cast<std::size_t>(i) * static_cast<std::size_t>(dim);
            double f = 0.0;
            if(fid == 1) { // Rosenbrock
                for(int j = 0; j < dim - 1; ++j) {
                    const double a = x[j + 1] - x[j] * x[j];
                    const double b = 1.0 - x[j];
                    f += 100.0 * a * a + b * b;
                }
            } else { // parabola (sum of squares)
                for(int j = 0; j < dim; ++j) {
                    f += x[j] * x[j];
                }
            }
            fitness_out[i] = f;
        }
    }

private:
    int funcId_ = 0;
};

/******************************************************************************/

} /* namespace Gem::Geneva::GPU::Demo */
