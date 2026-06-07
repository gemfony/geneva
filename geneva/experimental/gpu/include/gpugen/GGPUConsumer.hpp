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
#include <string>
#include <vector>

// Geneva headers
#include "common/GErrorStreamer.hpp"
#include "common/GExceptions.hpp"
#include "common/GLogger.hpp"
#include "courtier/GBaseConsumerT.hpp"
#include "gpugen/GCpuBackend.hpp"
#include "gpugen/GGPUBackendFactory.hpp"
#include "gpugen/GGPUConsumerConfig.hpp"
#include "gpugen/GGPUDeviceBackendI.hpp"
#include "gpugen/GGPUEvaluableI.hpp"

namespace Gem::Courtier::GPU {

/******************************************************************************/
/**
 * EXPERIMENTAL: a unified, device-model-agnostic GPU consumer.
 *
 * It is an ordinary courtier LOCAL consumer (it derives from the same Gem::Courtier::GBaseConsumerT
 * that every other consumer -- serial, multi-threaded, asio, beast, mpi -- derives from), so it plugs
 * into the existing span+policy submission path and broker wiring unchanged: register it with a
 * GBrokerT and hand that broker to Go2 via registerBroker(). courtier hands dispatch_() the WHOLE
 * round's batch at once, which this consumer evaluates in a single bulk kernel launch.
 *
 * The two things that used to be hard-wired per CUDA consumer are now decoupled and configurable:
 *   - the device-programming model (CPU / CUDA / OpenCL) -- chosen at run time from the config file,
 *     served by a swappable GGPUDeviceBackendI;
 *   - the kernel code -- a path in the config file, runtime-compiled (NVRTC / clBuildProgram) or
 *     loaded as a prebuilt module.
 * The problem-specific marshalling (how a batch of individuals becomes flat device buffers and how
 * results are written back) is supplied as a GGPUEvaluableI, so one consumer serves any problem.
 *
 * GPU evaluation is deterministic and all-or-nothing, so a batch should be submitted under
 * GSubmissionPolicy::full_success_or_fatal.
 */
template <typename processable_type>
class GGPUConsumerT final : public Gem::Courtier::GBaseConsumerT<processable_type> {
public:
    using item_ptr = typename Gem::Courtier::GBaseConsumerT<processable_type>::item_ptr;

    /** @brief Builds the consumer from @p configFile (backend + kernel selection) and the
     *  problem-specific @p marshaller. The backend is created and the kernel acquired lazily, on the
     *  first dispatch_, so construction is cheap and device errors surface at run time. */
    GGPUConsumerT(const std::string &configFile,
                  std::shared_ptr<GGPUEvaluableI<processable_type>> marshaller)
        : marshaller_(std::move(marshaller)) {
        cfg_.load(configFile);
    }

    ~GGPUConsumerT() override = default;

    GGPUConsumerT(const GGPUConsumerT &) = delete;
    GGPUConsumerT &operator=(const GGPUConsumerT &) = delete;

    /** @brief The backend actually in use (after the first dispatch_), e.g. "cuda" / "cpu". */
    [[nodiscard]] std::string activeBackendName() const {
        return backend_ ? backend_->name() : std::string("(uninitialised)");
    }

protected:
    /***************************************************************************/
    /** @brief Evaluates the whole round's batch in one bulk launch: flatten -> backend -> scatter. */
    void dispatch_(std::vector<item_ptr> &items) override {
        if(items.empty()) {
            return;
        }
        ensureBackend_();

        const int n = static_cast<int>(items.size());
        marshaller_->flatten(items, params_);
        const int dim = (n > 0) ? static_cast<int>(params_.size() / static_cast<std::size_t>(n)) : 0;

        // Build the problem-constant blob ONCE when it is static (e.g. a fixed target image), rather
        // than rebuilding it every generation. The backend likewise skips re-uploading an unchanged
        // blob (it sees the same stable pointer).
        if(not pconst_built_ || not marshaller_->problemConstantsStatic()) {
            pconst_ = marshaller_->problemConstants();
            pconst_built_ = true;
        }
        fitness_.assign(static_cast<std::size_t>(n), 0.0);

        backend_->evaluate(
            params_.data(), n, dim, pconst_.data(), pconst_.size(), fitness_.data(),
            marshaller_->parallelWorkPerItem());

        marshaller_->scatter(items, fitness_);
    }

private:
    /***************************************************************************/
    /** @brief Lazily builds the backend and acquires the kernel. Falls back to the CPU backend (with
     *  a warning) if the configured backend was not compiled into this build. */
    void ensureBackend_() {
        if(backend_) {
            return;
        }
        BackendKind kind = cfg_.backendKind();
        if(not backendAvailable(kind)) {
            glogger << "In Gem::Courtier::GPU::GGPUConsumer: the '" << toString(kind)
                    << "' backend was not compiled into this build; falling back to 'cpu'." << '\n'
                    << GWARNING;
            kind = BackendKind::Cpu;
        }
        backend_ = makeBackend(kind, marshaller_.get());
        backend_->initialize(cfg_.kernelSpec());
        glogger << "Gem::Courtier::GPU::GGPUConsumer using the '" << backend_->name()
                << "' backend (kernel: " << cfg_.kernel_path << ")" << '\n'
                << GLOGGING;
    }

    /***************************************************************************/
    std::shared_ptr<GGPUEvaluableI<processable_type>> marshaller_; ///< Problem-specific flatten/scatter
    GGPUConsumerConfig cfg_;                          ///< backend + kernel selection
    std::unique_ptr<GGPUDeviceBackendI> backend_;    ///< The device backend (lazy)

    std::vector<double> params_;     ///< Reused host parameter buffer (avoids per-round reallocation)
    std::vector<double> fitness_;    ///< Reused host fitness buffer
    std::vector<std::byte> pconst_;  ///< Cached problem-constant blob (built once when static)
    bool pconst_built_ = false;      ///< Whether pconst_ has been built
};

/******************************************************************************/

} /* namespace Gem::Courtier::GPU */
