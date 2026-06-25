/**
 * @file GBenchmarkGPUMarshaller.hpp
 */

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
#include "courtier/gpu/GGPUEvaluableI.hpp"
#include "geneva/individuals/GBenchmarkFunctions.hpp"
#include "geneva/individuals/GFunctionIndividual.hpp"
#include "geneva/ind/GFlatGenome.hpp"
#include "geneva/ind/GIndividualSlot.hpp" // the courtier work item wraps the genome in a slot

namespace gind = Gem::Geneva::Individuals;
namespace gen = Gem::Geneva::Genome;

namespace Gem::Geneva::Benchmarks {

/******************************************************************************/
/**
 * A GGPUEvaluableI marshaller for the CUDA optimisation benchmark. It turns a batch of
 * GFunctionIndividuals into flat device buffers for the unified courtier GPU consumer
 * (Gem::Courtier::GPU::GGPUConsumerT): the whole population is scored in ONE bulk, runtime-compiled
 * kernel launch (kernels/benchmark_eval.cu via NVRTC), instead of the previous
 * build-time-compiled .cu consumer. This is what unifies the benchmark with example 15 onto the same
 * backend.
 *
 * The benchmark function is the same for every individual in a run; it is read from the first item's
 * demoFunction() in flatten() and passed to the kernel as the opaque problem-constant (a single int
 * funcId). The host reference (hostEvaluate) reuses the EXACT same function math the CPU path uses
 * (Gem::Geneva::Benchmarks::eval in GBenchmarkFunctions.hpp), so a CPU run and a GPU run agree and the
 * GPU can be cross-checked.
 */
class GBenchmarkGPUMarshaller final
  : public Gem::Courtier::GPU::GGPUEvaluableI<gen::GIndividualSlot> {
public:
    /** @brief The flattened dimension of one benchmark genome (its count of double parameters), used by
     *  the consumer to enforce a uniform geometry across the batch. */
    [[nodiscard]] std::size_t itemDimension(const item_ptr &item) const override {
        return item->individual().countParameters<double>();
    }

    void flatten(const std::vector<item_ptr> &items, std::vector<double> &params_out) const override {
        if(items.empty()) {
            params_out.clear();
            return;
        }
        // All items in a benchmark run share the same function and dimension.
        auto *first = dynamic_cast<gind::GFunctionIndividual *>(&items.front()->individual());
        funcId_ = static_cast<int>(first->getDemoFunction());

        // Bulk flatten via GFlatGenome::streamlineInto(): each item's external (range-folded) values are
        // written straight into the output buffer -- no per-item temporary vector and no second copy.
        const std::size_t dim = this->itemDimension(items.front());
        params_out.resize(items.size() * dim);
        for(std::size_t i = 0; i < items.size(); ++i) {
            const auto *flat = dynamic_cast<const gen::GFlatGenome *>(&items[i]->individual());
            flat->streamlineInto(params_out.data() + i * dim);
        }
    }

    /** @brief The opaque problem constant is a single int: the benchmark function id. It is read from
     *  the batch in flatten(), which the consumer always calls first, so funcId_ is current here. */
    [[nodiscard]] std::vector<std::byte> problemConstants() const override {
        std::vector<std::byte> b(sizeof(int));
        std::memcpy(b.data(), &funcId_, sizeof(int));
        return b;
    }

    void scatter(const std::vector<item_ptr> &items, const std::vector<double> &fitness) const override {
        for(std::size_t i = 0; i < items.size(); ++i) {
            items[i]->process(std::vector<gen::individual_processing_result>(
                1, gen::individual_processing_result(fitness[i])));
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
            fitness_out[i] = eval(fid, x, dim); // the shared CPU/GPU function math
        }
    }

private:
    mutable int funcId_ = 0; ///< Benchmark function id, refreshed from the batch in flatten()
};

/******************************************************************************/

} /* namespace Gem::Geneva::Benchmarks */
