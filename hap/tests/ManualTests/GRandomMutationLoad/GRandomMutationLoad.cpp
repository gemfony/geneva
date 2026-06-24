/**
 * @file GRandomMutationLoad.cpp
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

// A distribution-bound, Geneva-representative RNG benchmark that ranks the random
// SOURCES for the actual Geneva use-case. Two design points matter:
//
//  1. BURSTY demand, not a stream. Random numbers in an EA are consumed in bursts:
//     each generation adapts the whole population (a burst of Gaussian-mutation
//     draws), then the algorithm evaluates fitness / selects -- a GAP in which it
//     draws (almost) nothing. The entire point of the random-number factory is to
//     use that gap to pre-produce numbers so the next burst finds them ready. A
//     continuous-stream loop would unfairly starve the QUEUE source (its producers
//     never get the gap to run ahead), so this benchmark models burst + gap and
//     measures only the IN-BURST time -- how fast a source serves a burst given it
//     had the realistic gap to prepare.
//
//  2. The draw mix mirrors geneva/.../GAdaptionKernels.hpp adaptGaussGroup: a
//     g_bernoulli gate per value plus a g_normal value step when it fires, with two
//     g_normal self-adaption draws per generation.
//
// All four sources run in one binary (templated on GRandomT<source>), under a
// thread count that adapts to the host (a factor x hardware_concurrency), like a
// parallel EA.

// Standard header files go here
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

// Geneva header files go here
#include "common/GParserBuilder.hpp"
#include "hap/GDistributionCache.hpp"
#include "hap/GRandomDistributionsT.hpp"
#include "hap/GRandomT.hpp"

namespace {

/******************************************************************************/
/**
 * @brief Runs the bursty EA-mutation draw mix for one source and returns in-burst value-steps/s.
 *
 * Each thread owns its own proxy (as a per-individual engine would) and runs
 * nGenerations bursts. A burst adapts nParams parameters (mirroring
 * adaptGaussGroup: two self-adaption normals, then per parameter a bernoulli gate
 * and -- when it fires -- one N(0, sigma) value step); only the burst is timed.
 * Between bursts the thread sleeps gapMicros microseconds (the fitness-evaluation
 * gap) WITHOUT drawing, giving the factory / background producers time to refill --
 * exactly the regime the factory is designed for. The reported rate is the sum of
 * the per-thread in-burst rates (system throughput while a burst is in flight).
 *
 * When usePrefetch is true, the normal value step is drawn through a GNormalCacheT
 * that is filled in the GAP (untimed) via prefetch(); the burst then only pops a
 * pre-produced standard normal and scales it -- so the sqrt/log of the transform
 * leaves the timed burst. When false, the same cache is used but never prefetched,
 * so operator() always produces inline (the transform stays in the burst) -- a clean
 * A/B that isolates exactly the gap-transform effect.
 *
 * @tparam S The random source the proxies draw from
 * @return In-burst throughput in value-steps per second
 */
template <Gem::Hap::randomSource S>
double run_mutation_load(std::uint32_t nParams, std::uint32_t nGenerations, unsigned nThreads,
                         std::uint32_t gapMicros, bool usePrefetch) {
    std::vector<std::thread> threads;
    threads.reserve(nThreads);
    std::vector<double>        rates(nThreads, 0.);
    std::vector<std::uint64_t> sinks(nThreads, 0);

    for(unsigned t = 0; t < nThreads; ++t) {
        threads.emplace_back([&, t]() {
            Gem::Hap::GRandomT<S>              gr;
            Gem::Hap::g_bernoulli_distribution bernoulli;
            using b_param = Gem::Hap::g_bernoulli_distribution::param_type;
            // The cache holds standard normals; +2 covers the two self-adaption draws per burst.
            Gem::Hap::GNormalCacheT<double> normal(gr, nParams + 2);

            double sigma      = 0.1;
            double ad_prob    = 1.0;
            double local      = 0.;
            double burstNanos = 0.;
            for(std::uint32_t g = 0; g < nGenerations; ++g) {
                // --- GAP work (untimed): pre-produce the burst's standard normals ---
                if(usePrefetch) {
                    normal.prefetch(nParams + 2);
                }

                const std::chrono::steady_clock::time_point b0 = std::chrono::steady_clock::now();

                // --- BURST: adapt the whole (per-thread) parameter set ---
                // normal(mean, stddev) pops a prefetched standard normal and scales it (prefetch
                // mode), or transforms inline (non-prefetch mode) -- same numbers, different timing.
                ad_prob *= std::exp(normal(0., 0.1));
                ad_prob = std::clamp(ad_prob, 1.0e-3, 1.0);
                sigma *= std::exp(normal(0., 0.8));
                sigma = std::clamp(sigma, 1.0e-9, 1.0);
                for(std::uint32_t p = 0; p < nParams; ++p) {
                    if(bernoulli(gr, b_param(ad_prob))) {
                        local += normal(0., sigma);
                    }
                }

                const std::chrono::steady_clock::time_point b1 = std::chrono::steady_clock::now();
                burstNanos += std::chrono::duration<double, std::nano>(b1 - b0).count();

                // --- GAP: fitness evaluation / selection -- no draws, producers refill ---
                if(gapMicros > 0) {
                    std::this_thread::sleep_for(std::chrono::microseconds(gapMicros));
                }
            }
            sinks[t] = static_cast<std::uint64_t>(std::abs(local)) & 0xffU;
            const double steps = static_cast<double>(nGenerations) * static_cast<double>(nParams);
            rates[t] = (burstNanos > 0.) ? (steps / (burstNanos * 1.0e-9)) : 0.;
        });
    }
    for(auto &th : threads) {
        th.join();
    }

    // Consume the sinks so the work cannot be elided.
    std::uint64_t guard = 0;
    for(auto s : sinks) {
        guard += s;
    }
    if(guard == 0xffffffffffffffffULL) {
        std::cerr << ""; // unreachable; keeps `guard` live
    }

    // System in-burst throughput = sum of the per-thread in-burst rates.
    double total = 0.;
    for(double r : rates) {
        total += r;
    }
    return total;
}

/******************************************************************************/

} // namespace

int main(int argc, char **argv) {
    std::uint32_t nParams = 2000;
    std::uint32_t nGenerations = 200;
    std::uint16_t nThreads = 0;     // 0 -> threadFactor * hardware_concurrency
    std::uint16_t threadFactor = 2; // worker threads per core (the user's "2 x n-cores")
    std::uint32_t gapMicros = 250;  // fitness-evaluation gap between bursts (microseconds)
    std::uint16_t nProducerThreads = 4;

    Gem::Common::GParserBuilder gpb;
    gpb.registerCLParameter<std::uint32_t>(
        "nParams,d", nParams, nParams, "Parameters adapted per burst (per thread)");
    gpb.registerCLParameter<std::uint32_t>(
        "generations,g", nGenerations, nGenerations, "Number of bursts (generations) per thread");
    gpb.registerCLParameter<std::uint16_t>(
        "threads,t", nThreads, nThreads, "Worker threads (0 = threadFactor x cores)");
    gpb.registerCLParameter<std::uint16_t>(
        "threadFactor,f", threadFactor, threadFactor,
        "Worker threads per core when --threads is 0 (e.g. 2 = 2x n-cores)");
    gpb.registerCLParameter<std::uint32_t>(
        "gapMicros,u", gapMicros, gapMicros,
        "Per-burst evaluation gap in microseconds (0 = continuous, no refill gap)");
    gpb.registerCLParameter<std::uint16_t>(
        "nProducerThreads,n", nProducerThreads, nProducerThreads,
        "Producer threads for the QUEUE factory");

    if(Gem::Common::GCL_HELP_REQUESTED == gpb.parseCommandLine(argc, argv, true /*verbose*/)) {
        return 0;
    }

    const unsigned cores = std::max(1u, std::thread::hardware_concurrency());
    const unsigned workers =
        (nThreads == 0) ? std::max(1u, threadFactor * cores) : nThreads;
    Gem::Hap::randomFactory()->setNProducerThreads(nProducerThreads);

    std::cout << "Bursty EA-mutation RNG load\n"
              << "  cores=" << cores << " threads=" << workers
              << (nThreads == 0 ? (" (=" + std::to_string(threadFactor) + "x cores)") : " (fixed)")
              << " bursts=" << nGenerations << " params/burst=" << nParams
              << " gap=" << gapMicros << "us"
              << " producers=" << nProducerThreads << "\n"
              << "  per param: bernoulli gate + N(0,sigma) step; IN-BURST timing only.\n"
              << "  'inline' = transform in the burst; 'prefetch' = standard normals produced in\n"
              << "  the gap (GNormalCacheT), burst only pops + scales. speedup = prefetch / inline.\n\n";

    struct Row {
        std::string name;
        double      inlineRate;
        double      prefetchRate;
    };
    std::vector<Row> rows;
    rows.push_back({"queue",
                    run_mutation_load<Gem::Hap::randomSource::QUEUE>(nParams, nGenerations, workers, gapMicros, false),
                    run_mutation_load<Gem::Hap::randomSource::QUEUE>(nParams, nGenerations, workers, gapMicros, true)});
    rows.push_back({"local",
                    run_mutation_load<Gem::Hap::randomSource::LOCAL>(nParams, nGenerations, workers, gapMicros, false),
                    run_mutation_load<Gem::Hap::randomSource::LOCAL>(nParams, nGenerations, workers, gapMicros, true)});
    rows.push_back({"staged",
                    run_mutation_load<Gem::Hap::randomSource::STAGED>(nParams, nGenerations, workers, gapMicros, false),
                    run_mutation_load<Gem::Hap::randomSource::STAGED>(nParams, nGenerations, workers, gapMicros, true)});
    rows.push_back({"quarantine",
                    run_mutation_load<Gem::Hap::randomSource::QUARANTINE>(nParams, nGenerations, workers, gapMicros, false),
                    run_mutation_load<Gem::Hap::randomSource::QUARANTINE>(nParams, nGenerations, workers, gapMicros, true)});

    std::sort(rows.begin(), rows.end(),
              [](const Row &a, const Row &b) { return a.prefetchRate > b.prefetchRate; });

    std::cout << std::left << std::setw(14) << "source" << std::right << std::setw(18)
              << "inline steps/s" << std::setw(20) << "prefetch steps/s" << std::setw(12)
              << "speedup" << '\n';
    std::cout << "----------------------------------------------------------------\n";
    for(const auto &r : rows) {
        const double speedup = (r.inlineRate > 0.) ? (r.prefetchRate / r.inlineRate) : 0.;
        std::cout << std::left << std::setw(14) << r.name << std::right << std::setw(18)
                  << std::fixed << std::setprecision(0) << r.inlineRate << std::setw(20)
                  << r.prefetchRate << std::setw(11) << std::setprecision(3) << speedup << "x"
                  << '\n';
    }
    return 0;
}
