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
 * Demo / self-test for the EXPERIMENTAL unified GPU consumer on an IMAGE-fitness problem (the use case
 * example 15 covers). The image individual's CPU fitnessCalculation is identical to the GPU kernel
 * (both call the shared renderScore in GExpImageProblem.hpp), so:
 *
 *   Part 1 -- parity + timing: random individuals are scored by the chosen device backend and by the
 *             host reference; the fitness vectors must agree, and both wall-times are printed so the GPU
 *             speed-up over the CPU can be seen.
 *   Part 2 -- consumer integration: real GExpImageIndividuals are evaluated through a GGPUConsumer +
 *             courtier broker + executor (one bulk launch), confirming every item is left PROCESSED.
 *
 * Run wholly on the CPU with --backend cpu (this exercises the same renderScore the GPU uses, i.e. the
 * cross-check), or on the GPU with --backend cuda / opencl.
 *
 * Usage: ./GGPUImageDemo [--backend cpu|cuda|opencl] [--kernel <path>] [--items N]
 */

#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <random>
#include <string>
#include <vector>

#include "courtier/GBrokerT.hpp"
#include "courtier/GExecutorT.hpp"
#include "courtier/GSubmissionPolicy.hpp"
#include "geneva/par/GParameterSet.hpp"
#include "courtier/gpu/GGPUBackendFactory.hpp"
#include "courtier/gpu/GGPUConsumer.hpp"
#include "courtier/gpu/GGPUConsumerConfig.hpp"
#include "courtier/gpu/GGPUDeviceBackendI.hpp"
#include "GExpImageIndividual.hpp"
#include "GExpImageProblem.hpp"
#include "GImageGPUMarshaller.hpp"

namespace gpu = Gem::Courtier::GPU;
namespace img = Gem::Courtier::GPU::ImageDemo;
namespace c2 = Gem::Courtier;
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
    const std::string kernelPath = argValue(argc, argv, "--kernel", "./kernels/image_eval.cu");
    const int nItems = std::atoi(argValue(argc, argv, "--items", "256").c_str());

    gpu::BackendKind kind = gpu::backendKindFromString(backendStr);
    if(not gpu::backendAvailable(kind)) {
        std::printf("Requested backend '%s' not compiled in; using 'cpu'.\n", gpu::toString(kind));
        kind = gpu::BackendKind::CPU;
    }

    auto marshaller = std::make_shared<img::GImageGPUMarshaller>();

    std::printf("=== GPU consumer IMAGE demo ===\n");
    std::printf("backend=%s  kernel=%s  canvas=%dx%d  triangles=%d  dim=%d  items=%d\n",
                gpu::toString(kind), kernelPath.c_str(), img::IMG_W, img::IMG_H, img::IMG_T,
                img::IMG_DIM, nItems);

    int failures = 0;

    // ---- Part 1: backend vs host-reference parity + CPU/GPU timing ---------------------------
    {
        std::mt19937 rng(2026);
        std::uniform_real_distribution<double> dist(0.0, 1.0);
        std::vector<double> params(static_cast<std::size_t>(nItems) * img::IMG_DIM);
        for(auto &v : params) {
            v = dist(rng);
        }
        const std::vector<std::byte> pconst = marshaller->problemConstants();

        std::vector<double> gpuFit(static_cast<std::size_t>(nItems), 0.0);
        std::vector<double> cpuFit(static_cast<std::size_t>(nItems), 0.0);

        gpu::KernelSpec spec;
        spec.path = kernelPath;
        spec.entry = "evaluate";

        auto backend = gpu::makeBackend(kind, marshaller.get());
        backend->initialize(spec);

        const auto g0 = std::chrono::steady_clock::now();
        backend->evaluate(params.data(), nItems, img::IMG_DIM, pconst.data(), pconst.size(), gpuFit.data());
        const auto g1 = std::chrono::steady_clock::now();

        const auto c0 = std::chrono::steady_clock::now();
        marshaller->hostEvaluate(params.data(), nItems, img::IMG_DIM, pconst.data(), pconst.size(), cpuFit.data());
        const auto c1 = std::chrono::steady_clock::now();

        double maxDiff = 0.0;
        for(int i = 0; i < nItems; ++i) {
            maxDiff = std::max(maxDiff, std::fabs(gpuFit[static_cast<std::size_t>(i)]
                                                  - cpuFit[static_cast<std::size_t>(i)]));
        }
        const double gms = std::chrono::duration<double, std::milli>(g1 - g0).count();
        const double cms = std::chrono::duration<double, std::milli>(c1 - c0).count();
        const bool ok = maxDiff < 1e-6; // image fitness is O(W*H) ~ thousands; 1e-6 abs is tight
        std::printf("[Part 1] backend '%s' vs host: max|diff|=%.3e  | backend %.2f ms  host(CPU) %.2f ms"
                    "  (host/backend = %.1fx)  -> %s\n",
                    backend->name().c_str(), maxDiff, gms, cms, (gms > 0 ? cms / gms : 0.0),
                    ok ? "PASS" : "*** FAIL ***");
        if(not ok) {
            ++failures;
        }
    }

    // ---- Part 2: consumer + broker + executor integration ------------------------------------
    {
        const std::string cfgPath = "./GGPUConsumerImageDemo.json";
        std::remove(cfgPath.c_str());
        {
            gpu::GGPUConsumerConfig seed;
            seed.backend = gpu::toString(kind);
            seed.kernel_path = kernelPath;
            seed.kernel_entry = "evaluate";
            seed.load(cfgPath);
        }

        try {
            auto consumer = std::make_shared<gpu::GGPUConsumerT<gpar::GParameterSet>>(cfgPath, marshaller);
            auto broker = std::make_shared<c2::GBrokerT<gpar::GParameterSet>>();
            broker->registerConsumer(consumer);

            // Build a batch of image individuals with random parameters in [0,1].
            std::mt19937 rng(7);
            std::uniform_real_distribution<double> dist(0.0, 1.0);
            const std::size_t dim = static_cast<std::size_t>(img::IMG_DIM);
            std::vector<std::shared_ptr<gpar::GParameterSet>> batch;
            const int batchN = 100;
            for(int k = 0; k < batchN; ++k) {
                std::vector<double> start(dim);
                for(auto &v : start) {
                    v = dist(rng);
                }
                const std::vector<double> lo(dim, 0.0);
                const std::vector<double> hi(dim, 1.0);
                batch.push_back(std::make_shared<Gem::Geneva::GExpImageIndividual>(
                    dim, start, lo, hi, /*sigma*/ 0.05, /*sigmaSigma*/ 0.2,
                    /*minSigma*/ 0.001, /*maxSigma*/ 0.5, /*adProb*/ 1.0));
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
