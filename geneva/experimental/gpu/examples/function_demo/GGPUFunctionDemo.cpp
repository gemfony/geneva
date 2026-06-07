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

/**
 * @file
 * Demo / self-test for the EXPERIMENTAL unified GPU consumer framework, exercising it on a
 * mathematical-function fitness (the use case the GCUDAOptBenchmark consumer covers). It runs two
 * checks against the backend selected on the command line (cpu / cuda / opencl):
 *
 *   Part 1 -- backend parity: random parameters are evaluated by the chosen device backend and by
 *             the marshaller's host reference; the two fitness vectors must agree. This validates the
 *             runtime-compiled kernel against the CPU reference.
 *   Part 2 -- consumer integration: real GFunctionIndividuals (from the factory) are evaluated through
 *             a GGPUConsumer wired to a courtier broker + executor (bulk dispatch), confirming every
 *             item is left PROCESSED.
 *
 * Usage:
 *   ./GGPUFunctionDemo [--backend cpu|cuda|opencl] [--kernel <path>] [--func 0|1] [--items N] [--dim D]
 *
 * It exits non-zero on any failure, so it doubles as a smoke test. The default backend is cpu, so it
 * runs anywhere; pass --backend cuda / opencl on a machine with that toolkit.
 */

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstdio>
#include <memory>
#include <random>
#include <string>
#include <vector>

#include "courtier/GBrokerT.hpp"
#include "courtier/GExecutorT.hpp"
#include "courtier/GSubmissionPolicy.hpp"
#include "geneva/individuals/GFunctionIndividual.hpp"
#include "geneva/par/GParameterSet.hpp"
#include "gpugen/GGPUBackendFactory.hpp"
#include "gpugen/GGPUConsumer.hpp"
#include "gpugen/GGPUConsumerConfig.hpp"
#include "gpugen/GGPUDeviceBackendI.hpp"
#include "GFunctionIndividualGPUMarshaller.hpp"

namespace gpu = Gem::Courtier::GPU;
namespace c2 = Gem::Courtier;
namespace gind = Gem::Geneva::Individuals;
namespace gpar = Gem::Geneva::Parameters;

namespace {

std::string argValue(int argc, char **argv, const std::string &key, const std::string &def) {
    for(int i = 1; i + 1 < argc; ++i) {
        if(key == argv[i]) {
            return argv[i + 1];
        }
    }
    return def;
}

} // namespace

int main(int argc, char **argv) {
    const std::string backendStr = argValue(argc, argv, "--backend", "cpu");
    const std::string kernelPath = argValue(argc, argv, "--kernel", "./kernels/function_eval.cu");
    const int funcId = std::atoi(argValue(argc, argv, "--func", "0").c_str());
    const int nItems = std::atoi(argValue(argc, argv, "--items", "256").c_str());
    const int dim = std::atoi(argValue(argc, argv, "--dim", "8").c_str());

    const gpu::BackendKind requested = gpu::backendKindFromString(backendStr);
    gpu::BackendKind kind = requested;
    if(not gpu::backendAvailable(kind)) {
        std::printf("Requested backend '%s' is not compiled into this build; using 'cpu'.\n",
                    gpu::toString(requested));
        kind = gpu::BackendKind::Cpu;
    }

    auto marshaller = std::make_shared<gpu::Demo::GFunctionIndividualGPUMarshaller>(funcId);

    std::printf("=== GPU consumer function demo ===\n");
    std::printf("backend=%s  kernel=%s  func=%d  items=%d  dim=%d\n",
                gpu::toString(kind), kernelPath.c_str(), funcId, nItems, dim);

    int failures = 0;

    // ---- Part 1: backend vs. host-reference parity -------------------------------------------
    {
        std::mt19937 rng(12345);
        std::uniform_real_distribution<double> dist(-2.0, 2.0);
        std::vector<double> params(static_cast<std::size_t>(nItems) * dim);
        for(auto &v : params) {
            v = dist(rng);
        }
        const std::vector<std::byte> pconst = marshaller->problemConstants();

        std::vector<double> gpuFit(static_cast<std::size_t>(nItems), 0.0);
        std::vector<double> cpuFit(static_cast<std::size_t>(nItems), 0.0);

        gpu::KernelSpec spec;
        spec.path = kernelPath;
        spec.entry = "evaluate";
        spec.device_id = 0;

        auto backend = gpu::makeBackend(kind, marshaller.get());
        backend->initialize(spec);
        backend->evaluate(params.data(), nItems, dim, pconst.data(), pconst.size(), gpuFit.data());
        marshaller->hostEvaluate(params.data(), nItems, dim, pconst.data(), pconst.size(), cpuFit.data());

        double maxDiff = 0.0;
        for(int i = 0; i < nItems; ++i) {
            maxDiff = std::max(maxDiff, std::fabs(gpuFit[static_cast<std::size_t>(i)]
                                                  - cpuFit[static_cast<std::size_t>(i)]));
        }
        const bool ok = maxDiff < 1e-9;
        std::printf("[Part 1] backend '%s' vs host reference: max|diff|=%.3e  sample: gpu=%.6f cpu=%.6f  -> %s\n",
                    backend->name().c_str(), maxDiff, gpuFit[0], cpuFit[0], ok ? "PASS" : "*** FAIL ***");
        if(not ok) {
            ++failures;
        }
    }

    // ---- Part 2: full consumer + broker + executor integration -------------------------------
    {
        // Seed a config file reflecting the chosen backend/kernel, then drive a GGPUConsumer with it.
        const std::string cfgPath = "./GGPUConsumerFunctionDemo.json";
        std::remove(cfgPath.c_str()); // ensure our seeded values are written, not stale ones
        {
            gpu::GGPUConsumerConfig seed;
            seed.backend = gpu::toString(kind);
            seed.kernel_path = kernelPath;
            seed.kernel_entry = "evaluate";
            seed.load(cfgPath); // writes the file with these values, then reads them back
        }

        try {
            auto consumer = std::make_shared<gpu::GGPUConsumerT<gpar::GParameterSet>>(cfgPath, marshaller);
            auto broker = std::make_shared<c2::GBrokerT<gpar::GParameterSet>>();
            broker->registerConsumer(consumer);

            // Build a batch of GFunctionIndividuals via the factory (random initial parameters).
            gind::GFunctionIndividualFactory factory("./config/GFunctionIndividual.json");
            std::vector<std::shared_ptr<gpar::GParameterSet>> batch;
            const int batchN = 100;
            for(int i = 0; i < batchN; ++i) {
                batch.push_back(factory.get_as<gind::GFunctionIndividual>());
            }

            c2::GExecutorT<gpar::GParameterSet> executor(broker);
            executor.workOn(batch, c2::GSubmissionPolicy::full_success_or_fatal());

            std::size_t processed = 0;
            for(const auto &it : batch) {
                if(it && it->is_processed()) {
                    ++processed;
                }
            }
            const bool ok = (processed == batch.size());
            std::printf("[Part 2] consumer integration (backend '%s'): %zu/%zu items processed  -> %s\n",
                        consumer->activeBackendName().c_str(), processed, batch.size(),
                        ok ? "PASS" : "*** FAIL ***");
            if(not ok) {
                ++failures;
            }
        } catch(const std::exception &e) {
            std::printf("[Part 2] *** FAIL *** exception: %s\n", e.what());
            ++failures;
        }
    }

    std::printf("=== %s ===\n", failures == 0 ? "ALL CHECKS PASSED" : "SOME CHECKS FAILED");
    return failures == 0 ? 0 : 1;
}
