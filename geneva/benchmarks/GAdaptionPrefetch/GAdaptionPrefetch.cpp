/**
 * @file GAdaptionPrefetch.cpp
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

// Phase C: drive the REAL EA Gauss-mutation kernel (geneva/ind/GAdaptionKernels.hpp
// adaptGaussGroup) over many bursts, comparing the inline kernel against the cache-aware
// overload (the per-value N(0,sigma) step popped from a prefetched standard-normal cache,
// scaled by sigma). The prefetch runs in the (untimed) evaluation gap; only the adaption
// burst is timed -- so this measures the end-to-end in-adaption win of moving the value-step
// transform off the burst, on the actual production kernel.

// Standard headers
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <optional>
#include <span>
#include <string>
#include <thread>
#include <vector>

// Geneva headers
#include "common/GParserBuilder.hpp"
#include "geneva/ind/GAdaptionKernels.hpp"
#include "hap/GDistributionCache.hpp"
#include "hap/GRandomT.hpp"

namespace {

enum class Mode { Inline, CacheCpu, CacheGpu };

/******************************************************************************/
/**
 * @brief Runs the real adaptGaussGroup over nGenerations bursts and returns in-adaption values/s.
 *
 * Mode::Inline uses the gr-only kernel overload; Mode::CacheCpu / Mode::CacheGpu use the
 * cache-aware overload, with the standard normals produced on the CPU per-value or in bulk
 * (GPU-native when present). The cache is refilled in the untimed gap; only the adaption call is
 * timed. adaption_threshold is set high so the per-value gaussian step (the cached draw) dominates
 * the normal load.
 *
 * @return Summed in-adaption throughput in adapted-values per second
 */
double run_adaption(std::uint32_t nParams, std::uint32_t nGenerations, unsigned nThreads,
                    std::uint32_t gapMicros, std::uint32_t threshold, Mode mode) {
    using Gem::Geneva::Genome::GaussConfig;
    using Gem::Geneva::Genome::GaussState;

    std::vector<std::thread> threads;
    threads.reserve(nThreads);
    std::vector<double> rates(nThreads, 0.);
    std::vector<double> sinks(nThreads, 0.);

    for(unsigned t = 0; t < nThreads; ++t) {
        threads.emplace_back([&, t]() {
            Gem::Hap::GRandomT<Gem::Hap::randomSource::QUEUE> gr;
            std::vector<double>                              values(nParams, 0.0);

            GaussConfig<double> cfg;
            cfg.mode               = Gem::Geneva::adaptionMode::ALWAYS; // every value adapts
            cfg.adaption_threshold = threshold; // sigma self-adapts every Nth -> value step dominates
            GaussState<double> st;
            st.sigma   = 0.1;
            st.ad_prob = 1.0;

            const std::size_t cap = nParams + 8;
            std::optional<Gem::Hap::GNormalCacheT<double>> cpuCache;
            std::optional<Gem::Hap::GNormalCacheT<double>> gpuCache;
            if(mode == Mode::CacheCpu) {
                cpuCache.emplace(gr, cap);
            }
            else if(mode == Mode::CacheGpu) {
                gpuCache.emplace(cap); // bulk (no-proxy) constructor
            }

            double burstNanos = 0.;
            for(std::uint32_t g = 0; g < nGenerations; ++g) {
                // --- GAP (untimed): pre-produce the value-step standard normals ---
                if(mode == Mode::CacheCpu) {
                    cpuCache->prefetch(cap);
                }
                else if(mode == Mode::CacheGpu) {
                    gpuCache->prefetch(cap);
                }

                const std::chrono::steady_clock::time_point b0 = std::chrono::steady_clock::now();
                // --- BURST (timed): the real Gauss-mutation kernel ---
                if(mode == Mode::Inline) {
                    Gem::Geneva::Genome::adaptGaussGroup<double>(cfg, st, std::span<double>(values), gr);
                }
                else {
                    Gem::Hap::GNormalCacheT<double> &nc = (mode == Mode::CacheCpu) ? *cpuCache : *gpuCache;
                    Gem::Geneva::Genome::adaptGaussGroup<double>(cfg, st, std::span<double>(values), gr, nc);
                }
                const std::chrono::steady_clock::time_point b1 = std::chrono::steady_clock::now();
                burstNanos += std::chrono::duration<double, std::nano>(b1 - b0).count();

                if(gapMicros > 0) {
                    std::this_thread::sleep_for(std::chrono::microseconds(gapMicros));
                }
            }
            sinks[t] = values.empty() ? 0.0 : values[(nGenerations + t) % nParams];
            rates[t] = (burstNanos > 0.)
                         ? (static_cast<double>(nGenerations) * nParams / (burstNanos * 1.0e-9))
                         : 0.;
        });
    }
    for(auto &th : threads) {
        th.join();
    }
    double guard = 0.;
    for(double s : sinks) {
        guard += s;
    }
    if(guard == 1.0e300) {
        std::cerr << ""; // keep sinks live
    }
    double total = 0.;
    for(double r : rates) {
        total += r;
    }
    return total;
}

/******************************************************************************/

} // namespace

int main(int argc, char **argv) {
    std::uint32_t nParams = 4000;
    std::uint32_t nGenerations = 150;
    std::uint16_t nThreads = 0;
    std::uint16_t threadFactor = 2;
    std::uint32_t gapMicros = 1000;
    std::uint32_t threshold = 100;

    Gem::Common::GParserBuilder gpb;
    gpb.registerCLParameter<std::uint32_t>("nParams,d", nParams, nParams, "Parameters per group (per thread)");
    gpb.registerCLParameter<std::uint32_t>("generations,g", nGenerations, nGenerations, "Bursts per thread");
    gpb.registerCLParameter<std::uint16_t>("threads,t", nThreads, nThreads, "Worker threads (0 = factor x cores)");
    gpb.registerCLParameter<std::uint16_t>("threadFactor,f", threadFactor, threadFactor, "Threads per core when --threads is 0");
    gpb.registerCLParameter<std::uint32_t>("gapMicros,u", gapMicros, gapMicros, "Per-burst evaluation gap (microseconds)");
    gpb.registerCLParameter<std::uint32_t>("threshold,k", threshold, threshold, "sigma self-adaption threshold (higher = value step dominates)");
    if(Gem::Common::GCL_HELP_REQUESTED == gpb.parseCommandLine(argc, argv, true /*verbose*/)) {
        return 0;
    }

    const unsigned cores   = std::max(1u, std::thread::hardware_concurrency());
    const unsigned workers = (nThreads == 0) ? std::max(1u, threadFactor * cores) : nThreads;

    std::cout << "Real adaptGaussGroup kernel: " << workers << " threads x " << nGenerations
              << " bursts x " << nParams << " params, gap=" << gapMicros << "us, threshold="
              << threshold << "\n  IN-ADAPTION timing only; cache prefetched in the gap.\n\n";

    const double inl = run_adaption(nParams, nGenerations, workers, gapMicros, threshold, Mode::Inline);
    const double cpu = run_adaption(nParams, nGenerations, workers, gapMicros, threshold, Mode::CacheCpu);
    const double gpu = run_adaption(nParams, nGenerations, workers, gapMicros, threshold, Mode::CacheGpu);

    std::cout << std::left << std::setw(34) << "mode" << std::right << std::setw(20)
              << "values/s" << std::setw(12) << "speedup" << '\n'
              << "------------------------------------------------------------------\n"
              << std::left << std::setw(34) << "inline (gr-only kernel)" << std::right
              << std::setw(20) << std::fixed << std::setprecision(0) << inl << std::setw(11)
              << std::setprecision(3) << 1.0 << "x\n"
              << std::left << std::setw(34) << "cache, CPU per-value normals" << std::right
              << std::setw(20) << cpu << std::setw(11) << (inl > 0. ? cpu / inl : 0.) << "x\n"
              << std::left << std::setw(34) << "cache, bulk (GPU-native) normals" << std::right
              << std::setw(20) << gpu << std::setw(11) << (inl > 0. ? gpu / inl : 0.) << "x\n";
    return 0;
}
