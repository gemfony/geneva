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
 * EXPERIMENTAL port of example 15 (the Mona-Lisa triangle-superimposition problem) onto the unified
 * GPU consumer framework. The candidate's fitness is the deviation of its alpha-blended circle-triangle
 * rendering from a real target image (loaded from PNG), summed per pixel/channel through a rational
 * saturation function -- the same metric and genome as example 15. The CPU fitnessCalculation and the
 * CUDA kernel share the same render math, so the GPU is cross-checked against the CPU.
 *
 *   Part 1 -- parity + timing: random genomes scored by the device backend and by the host reference;
 *             fitness vectors must agree, and both wall-times are printed (GPU speed-up).
 *   Part 2 -- consumer integration: real GImageIndividuals (from the factory) evaluated through a
 *             GGPUConsumerT + broker + executor (one bulk launch), confirming every item is PROCESSED.
 *
 * Usage: ./GGPUMonaLisaDemo [--backend cpu|cuda] [--kernel <path>] [--target <png>] [--items N]
 * Concentrates on CUDA; --backend cpu runs the whole problem on the CPU for the cross-check.
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
#include "GImageIndividual.hpp"
#include "GMonaLisaGPUMarshaller.hpp"
#include "GMonaLisaProblem.hpp"

namespace gpu = Gem::Courtier::GPU;
namespace ml = Gem::Geneva::MonaLisa;
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
    const std::string backendStr = argValue(argc, argv, "--backend", "cuda");
    const std::string kernelPath = argValue(argc, argv, "--kernel", "./kernels/monalisa_eval.cu");
    const std::string targetPath = argValue(argc, argv, "--target", "./pictures/ml-small.png");
    const int nItems = std::atoi(argValue(argc, argv, "--items", "16").c_str());

    gpu::BackendKind kind = gpu::backendKindFromString(backendStr);
    if(not gpu::backendAvailable(kind)) {
        std::printf("Requested backend '%s' not compiled in; using 'cpu'.\n", gpu::toString(kind));
        kind = gpu::BackendKind::CPU;
    }

    try {
        ml::loadTarget(targetPath);
    } catch(const std::exception &e) {
        std::printf("*** FAIL *** %s\n", e.what());
        return 1;
    }
    const ml::Target &tgt = ml::target();
    std::printf("=== GPU consumer MONA-LISA demo (example-15 port) ===\n");
    std::printf("backend=%s  kernel=%s  target=%s (%dx%d)  items=%d\n",
                gpu::toString(kind), kernelPath.c_str(), targetPath.c_str(), tgt.width, tgt.height, nItems);

    auto marshaller = std::make_shared<ml::GMonaLisaGPUMarshaller>();

    constexpr int NT = 64;
    const int dim = 10 * NT + 3;
    int failures = 0;

    // ---- Part 1: backend vs host-reference parity + timing -----------------------------------
    {
        std::mt19937 rng(2026);
        std::uniform_real_distribution<double> dist(0.0, 1.0);
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

        auto backend = gpu::makeBackend(kind, marshaller.get());
        backend->initialize(spec);

        const auto g0 = std::chrono::steady_clock::now();
        backend->evaluate(params.data(), nItems, dim, pconst.data(), pconst.size(), gpuFit.data(),
                          marshaller->parallelWorkPerItem());
        const auto g1 = std::chrono::steady_clock::now();
        const auto c0 = std::chrono::steady_clock::now();
        marshaller->hostEvaluate(params.data(), nItems, dim, pconst.data(), pconst.size(), cpuFit.data());
        const auto c1 = std::chrono::steady_clock::now();

        double maxRel = 0.0;
        for(int i = 0; i < nItems; ++i) {
            const double gv = gpuFit[static_cast<std::size_t>(i)];
            const double cv = cpuFit[static_cast<std::size_t>(i)];
            const double denom = std::max(1.0, std::fabs(cv));
            maxRel = std::max(maxRel, std::fabs(gv - cv) / denom);
        }
        const double gms = std::chrono::duration<double, std::milli>(g1 - g0).count();
        const double cms = std::chrono::duration<double, std::milli>(c1 - c0).count();
        const bool ok = maxRel < 1e-6;
        std::printf("[Part 1] backend '%s' vs host: max rel diff=%.3e  | backend %.1f ms  host(CPU) %.1f ms"
                    "  (host/backend = %.1fx)  sample gpu=%.3f cpu=%.3f  -> %s\n",
                    backend->name().c_str(), maxRel, gms, cms, (gms > 0 ? cms / gms : 0.0),
                    gpuFit[0], cpuFit[0], ok ? "PASS" : "*** FAIL ***");
        if(not ok) {
            ++failures;
        }
    }

    // ---- Part 2: consumer + broker + executor integration with real individuals --------------
    {
        const std::string cfgPath = "./GGPUConsumerMonaLisaDemo.json";
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

            Gem::Geneva::GImageIndividualFactory factory("./config/GImageIndividual.json");
            std::vector<std::shared_ptr<gpar::GParameterSet>> batch;
            for(int k = 0; k < nItems; ++k) {
                batch.push_back(factory.get_as<Gem::Geneva::GImageIndividual>());
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
