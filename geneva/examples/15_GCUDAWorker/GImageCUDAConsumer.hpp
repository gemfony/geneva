/**
 * @file GImageCUDAConsumer.hpp
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

#include "common/GGlobalDefines.hpp"

// Standard headers
#include <memory>
#include <string>
#include <tuple>
#include <vector>

// Geneva headers
#include "common/GParserBuilder.hpp"
#include "courtier/GBaseConsumerT.hpp"
#include "geneva/par/GParameterSet.hpp"

// Local headers for the image individual and evaluator
#include "GImageIndividual.hpp"
#include "GImageIndividualEvaluator.hpp"

namespace Gem::Geneva {

/******************************************************************************/
// Default settings (mirroring the former GImageCUDAWorker)
constexpr unsigned int GIC_DEF_BS_X{16};
constexpr unsigned int GIC_DEF_BS_Y{16};
constexpr unsigned int GIC_DEF_GS_X{0}; // 0 means: determine automatically
constexpr unsigned int GIC_DEF_GS_Y{0};
const std::string GIC_DEF_IMAGEFILE{"./pictures/ml.png"};
constexpr bool GIC_DEF_USEGPU{true};

/******************************************************************************/
/**
 * A courtier2 local consumer that evaluates GImageIndividuals on the GPU (the courtier2 replacement
 * for the former GImageCUDAWorker + legacy GStdThreadConsumerT). courtier2 hands dispatch_() the whole
 * round's batch at once; a single persistent GImageIndividualEvaluator (created lazily from the first
 * individual, which fixes the image dimensions) is reused across all items and generations, holding
 * its GPU memory + CUDA stream for the consumer's lifetime. evaluate() injects the fitness via the
 * individual's process(result) call, which also leaves the item PROCESSED -- exactly what courtier2's
 * reconciliation reads -- so dispatch_ needs to do nothing else.
 *
 * Wiring (see GImageBuilder.cpp): the example registers this consumer with a courtier2 GBrokerT and
 * hands that broker to Go2 via Go2::registerBroker(), rather than enrolling a worker with the
 * old broker. The evaluation is sequential on one GPU; the former multi-worker model is not needed for
 * a single device (a thread-pool of per-thread evaluators could be reintroduced later if it pays off).
 */
class GImageCUDAConsumer final : public Gem::Courtier::GBaseConsumerT<gpar::GParameterSet> {
public:
    using item_ptr = typename Gem::Courtier::GBaseConsumerT<gpar::GParameterSet>::item_ptr;

    /***************************************************************************/
    /** @brief Initialization with the name of a configuration file (the same keys the former
     *  GImageCUDAWorker read: use_gpu, block_x/y, grid_x/y, image_file). */
    explicit GImageCUDAConsumer(const std::string &configFile) {
        Gem::Common::GParserBuilder gpb;

        gpb.registerFileParameter<bool>(
            "use_gpu", useGPU_, GIC_DEF_USEGPU, Gem::Common::VAR_IS_ESSENTIAL,
            "Indicates whether evaluation should run on the GPU (1); or the CPU (0)");

        gpb.registerFileParameter<unsigned int, unsigned int>(
            "block_x", "block_y", GIC_DEF_BS_X, GIC_DEF_BS_Y,
            [this](const unsigned int &x, const unsigned int &y) { blockSize_x_ = x; blockSize_y_ = y; },
            "CUDA block size", Gem::Common::VAR_IS_ESSENTIAL,
            "The block size (x-component);", "The block size (y-component);");

        gpb.registerFileParameter<unsigned int, unsigned int>(
            "grid_x", "grid_y", GIC_DEF_GS_X, GIC_DEF_GS_Y,
            [this](const unsigned int &x, const unsigned int &y) { gridSize_x_ = x; gridSize_y_ = y; },
            "CUDA grid size", Gem::Common::VAR_IS_ESSENTIAL,
            "The grid size (x-component); Will be calculated automatically if set to 0",
            "The grid size (y-component); Will be calculated automatically if set to 0");

        gpb.registerFileParameter<std::string>(
            "image_file", targetImageFileName_, GIC_DEF_IMAGEFILE, Gem::Common::VAR_IS_ESSENTIAL,
            "The name of the file holding the target image;");

        gpb.parseConfigFile(configFile);
    }

    /***************************************************************************/
    /** @brief Releases the evaluator's GPU resources. */
    ~GImageCUDAConsumer() override {
        if(evaluator_ptr_) {
            evaluator_ptr_->finalize();
        }
    }

    GImageCUDAConsumer(const GImageCUDAConsumer &) = delete;
    GImageCUDAConsumer &operator=(const GImageCUDAConsumer &) = delete;

    //--------------------------------------------------------------------------
    // Accessors used by the example's pluggable monitor (see GImageBuilder.cpp).

    std::string getTargetImageFileName() const { return targetImageFileName_; }
    bool useGPU() const { return useGPU_; }
    std::tuple<int, int> getBlockSize() const {
        return std::make_tuple(static_cast<int>(blockSize_x_), static_cast<int>(blockSize_y_));
    }
    std::tuple<int, int> getGridSize() const {
        return std::make_tuple(static_cast<int>(gridSize_x_), static_cast<int>(gridSize_y_));
    }

protected:
    /***************************************************************************/
    /** @brief Evaluates the whole round's batch on the GPU, one item at a time, through the persistent
     *  evaluator. evaluate() sets each item's fitness and PROCESSED status via process(result). */
    void dispatch_(std::vector<item_ptr> &items) override {
        if(items.empty()) {
            return;
        }
        if(not evaluator_ptr_) {
            // Lazy one-time setup: the first individual fixes the image dimensions / GPU allocations.
            auto first = std::static_pointer_cast<GImageIndividual>(items.front());
            evaluator_ptr_ = std::make_shared<GImageIndividualEvaluator>(
                targetImageFileName_, useGPU_, false /* getGPUCandidateImage */,
                static_cast<int>(blockSize_x_), static_cast<int>(blockSize_y_),
                static_cast<int>(gridSize_x_), static_cast<int>(gridSize_y_));
            evaluator_ptr_->init(first);
        }
        for(auto &it : items) {
            auto img = std::static_pointer_cast<GImageIndividual>(it);
            evaluator_ptr_->evaluate(img); // sets fitness + leaves the item PROCESSED
        }
    }

private:
    //--------------------------------------------------------------------------
    bool useGPU_{GIC_DEF_USEGPU};
    unsigned int blockSize_x_{GIC_DEF_BS_X};
    unsigned int blockSize_y_{GIC_DEF_BS_Y};
    unsigned int gridSize_x_{GIC_DEF_GS_X};
    unsigned int gridSize_y_{GIC_DEF_GS_Y};
    std::string targetImageFileName_{GIC_DEF_IMAGEFILE};

    std::shared_ptr<GImageIndividualEvaluator> evaluator_ptr_; ///< Persistent GPU evaluator (lazy init)
};

/******************************************************************************/

} /* namespace Gem::Geneva */
