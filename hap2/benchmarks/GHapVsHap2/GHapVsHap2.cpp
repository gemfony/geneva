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


/********************************************************************************
 *
 * GHapVsHap2 — side-by-side benchmark of Gem::Hap vs Gem::Hap2.
 *
 * Links BOTH libraries (gemfony-hap and gemfony-hap2) into one binary and runs
 * identical workloads against each, so every engine change in Hap2 can be
 * measured against the Hap baseline.
 *
 *   Cases:
 *     S-1  sustained raw operator()                 (default >=1e9 calls, 10 proxies)
 *     S-2  sustained std::uniform_real_distribution
 *     S-3  sustained std::normal_distribution
 *     B-1  cold-start burst (1000 proxies)          -> first-call latency, hang check
 *     B-2  sustained burst latency                  -> p50/p99/p999
 *
 * Output: TSV to stdout (and to --out <file> if given). Numbers are
 * machine-specific; what matters is Hap vs Hap2 on the same host and the
 * trend across builds.
 *
 * Apache License 2.0. See the NOTICE file in the top-level directory.
 *
 ********************************************************************************/

#include <algorithm>
#include <atomic>
#include <barrier>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <memory>
#include <random>
#include <string>
#include <thread>
#include <vector>

#include "hap/GRandomFactory.hpp"
#include "hap/GRandomT.hpp"
#include "hap2/GRandomFactory.hpp"
#include "hap2/GRandomT.hpp"

namespace {

using clock_type = std::chrono::steady_clock;
using nanos      = std::chrono::nanoseconds;

/******************************************************************************/
// Per-library traits. Identical class/function names in the two namespaces let
// us template the whole harness; only the namespace differs. The `engine`
// string is updated by hand as the Hap2 engine is swapped (phases 3-5).

struct HapTraits {
    using GRandom = Gem::Hap::GRandom;
    static std::shared_ptr<Gem::Hap::GRandomFactory> factory() { return Gem::Hap::randomFactory(); }
    static void reset() { Gem::Hap::resetRandomFactory(); }
    static constexpr const char *lib    = "Hap";
    static const char           *engine() { return "mt19937"; } // Hap is frozen on mt19937
};

struct Hap2Traits {
    using GRandom = Gem::Hap2::GRandom;
    static std::shared_ptr<Gem::Hap2::GRandomFactory> factory() { return Gem::Hap2::randomFactory(); }
    static void reset() { Gem::Hap2::resetRandomFactory(); }
    static constexpr const char *lib    = "Hap2";
    static const char           *engine() { return Gem::Hap2::cpuEngineName(); } // actual compiled engine
};

/******************************************************************************/
// Keep the optimiser from eliding the consumed values.
inline void sink_value(std::uint64_t v) { asm volatile("" : : "r"(v) : "memory"); }

// Per-thread "draw" functors. Each thread holds its own copy (distribution
// state is per-thread). They all return a double so the accumulator is uniform.
struct RawDraw {
    template <typename R> double operator()(R &rng) { return static_cast<double>(rng()); }
};
struct UniformDraw {
    std::uniform_real_distribution<double> d{0., 1.};
    template <typename R> double operator()(R &rng) { return d(rng); }
};
struct NormalDraw {
    std::normal_distribution<double> d;
    template <typename R> double operator()(R &rng) { return d(rng); }
};

/******************************************************************************/
// Sustained throughput. nThreads proxies, each draws perThread = total/nThreads
// values after an untimed warmup. Returns aggregate calls/second.
template <typename Traits, typename Draw>
double sustained(unsigned nThreads, std::uint64_t totalCalls,
                 std::uint64_t warmupPerThread, Draw proto) {
    using GRandom = typename Traits::GRandom;
    const std::uint64_t perThread = totalCalls / nThreads;

    std::atomic<std::uint64_t> outSink{0};
    std::barrier warmDone(static_cast<std::ptrdiff_t>(nThreads) + 1);
    std::barrier startGate(static_cast<std::ptrdiff_t>(nThreads) + 1);

    std::vector<std::thread> threads;
    threads.reserve(nThreads);
    for (unsigned t = 0; t < nThreads; ++t) {
        threads.emplace_back([&, proto]() {
            GRandom rng;
            Draw    draw = proto;
            double  acc  = 0.;
            for (std::uint64_t i = 0; i < warmupPerThread; ++i) acc += draw(rng);
            warmDone.arrive_and_wait();
            startGate.arrive_and_wait();
            for (std::uint64_t i = 0; i < perThread; ++i) acc += draw(rng);
            outSink.fetch_add(static_cast<std::uint64_t>(acc), std::memory_order_relaxed);
        });
    }

    warmDone.arrive_and_wait();          // every proxy warmed up
    auto t0 = clock_type::now();
    startGate.arrive_and_wait();         // release the timed loop
    for (auto &th : threads) th.join();
    auto t1 = clock_type::now();

    sink_value(outSink.load());
    double secs = std::chrono::duration<double>(t1 - t0).count();
    return static_cast<double>(perThread * nThreads) / secs;
}

/******************************************************************************/
// Cold-start burst: factory reset, nProxies threads start simultaneously, each
// times its very first draw then consumes perProxy values. Reports the maximum
// first-call latency (the warm-up penalty) across all proxies and wall time.
struct BurstResult {
    double maxFirstCallNs = 0.;
    double wallMs         = 0.;
};

template <typename Traits>
BurstResult coldStartBurst(unsigned nProxies, std::uint64_t perProxy) {
    using GRandom = typename Traits::GRandom;
    // NOTE: each library's GRandomFactory is a once-only singleton (a
    // multiple-instantiation trap forbids re-creating it), so we cannot force a
    // cold factory mid-process. For a genuine cold start run `--case B-1` in a
    // fresh process (then B-1 is the first factory use). In a full run the
    // factory is already warm from the S-* cases, so this measures warm burst
    // proxy spin-up. (Traits::reset() exists but must NOT be called here.)

    std::atomic<std::uint64_t> maxFirst{0};
    std::atomic<std::uint64_t> outSink{0};
    std::barrier startGate(static_cast<std::ptrdiff_t>(nProxies) + 1);

    std::vector<std::thread> threads;
    threads.reserve(nProxies);
    for (unsigned p = 0; p < nProxies; ++p) {
        threads.emplace_back([&]() {
            startGate.arrive_and_wait();
            GRandom rng;
            auto    a     = clock_type::now();
            auto    first = rng();
            auto    b     = clock_type::now();
            auto    fns   = static_cast<std::uint64_t>(
                std::chrono::duration_cast<nanos>(b - a).count());
            std::uint64_t prev = maxFirst.load(std::memory_order_relaxed);
            while (fns > prev && !maxFirst.compare_exchange_weak(prev, fns)) { }

            std::uint64_t local = static_cast<std::uint64_t>(first);
            for (std::uint64_t i = 1; i < perProxy; ++i) local += static_cast<std::uint64_t>(rng());
            outSink.fetch_add(local, std::memory_order_relaxed);
        });
    }

    auto t0 = clock_type::now();
    startGate.arrive_and_wait();
    for (auto &th : threads) th.join();
    auto t1 = clock_type::now();

    sink_value(outSink.load());
    return {static_cast<double>(maxFirst.load()),
            std::chrono::duration<double, std::milli>(t1 - t0).count()};
}

/******************************************************************************/
// Sustained burst with per-call latency capture -> p50/p99/p999. Note: timing
// every call adds clock overhead on top of the raw draw, so these are
// "instrumented" latencies; valid for Hap-vs-Hap2 comparison, not as absolute
// per-call cost.
struct LatResult {
    double p50 = 0., p99 = 0., p999 = 0.;
    double throughput = 0.;
};

template <typename Traits>
LatResult burstLatency(unsigned nProxies, std::uint64_t perProxy) {
    using GRandom = typename Traits::GRandom;

    std::vector<std::vector<std::uint32_t>> lat(nProxies);
    std::atomic<std::uint64_t>              outSink{0};
    std::barrier startGate(static_cast<std::ptrdiff_t>(nProxies) + 1);

    std::vector<std::thread> threads;
    threads.reserve(nProxies);
    for (unsigned p = 0; p < nProxies; ++p) {
        threads.emplace_back([&, p]() {
            GRandom rng;
            auto   &mine = lat[p];
            mine.resize(perProxy);
            for (int i = 0; i < 256; ++i) sink_value(static_cast<std::uint64_t>(rng())); // small warmup
            startGate.arrive_and_wait();
            std::uint64_t local = 0;
            for (std::uint64_t i = 0; i < perProxy; ++i) {
                auto a = clock_type::now();
                auto v = rng();
                auto b = clock_type::now();
                mine[i] = static_cast<std::uint32_t>(
                    std::chrono::duration_cast<nanos>(b - a).count());
                local += static_cast<std::uint64_t>(v);
            }
            outSink.fetch_add(local, std::memory_order_relaxed);
        });
    }

    auto t0 = clock_type::now();
    startGate.arrive_and_wait();
    for (auto &th : threads) th.join();
    auto t1 = clock_type::now();
    sink_value(outSink.load());

    std::vector<std::uint32_t> all;
    all.reserve(static_cast<std::size_t>(nProxies) * perProxy);
    for (auto &v : lat) all.insert(all.end(), v.begin(), v.end());
    std::sort(all.begin(), all.end());
    auto pct = [&](double q) -> double {
        if (all.empty()) return 0.;
        std::size_t idx = std::min(all.size() - 1, static_cast<std::size_t>(q * all.size()));
        return static_cast<double>(all[idx]);
    };
    double secs = std::chrono::duration<double>(t1 - t0).count();
    return {pct(0.50), pct(0.99), pct(0.999),
            static_cast<double>(static_cast<std::uint64_t>(nProxies) * perProxy) / secs};
}

/******************************************************************************/
// Kolmogorov-Smirnov uniformity sanity check. Feeds the library's proxy through
// std::uniform_real_distribution(0,1) and returns the K-S D statistic against
// the uniform CDF. Asymptotic critical value at alpha=1e-3 is ~1.9495/sqrt(n);
// D < D_crit => the samples are consistent with uniformity.
template <typename Traits>
double ksUniform(std::uint64_t n) {
    using GRandom = typename Traits::GRandom;
    GRandom                                rng;
    std::uniform_real_distribution<double> u(0., 1.);
    std::vector<double>                    v;
    v.reserve(n);
    for (std::uint64_t i = 0; i < n; ++i) v.push_back(u(rng));
    std::sort(v.begin(), v.end());
    double d = 0.;
    for (std::uint64_t i = 0; i < n; ++i) {
        double fnHi = static_cast<double>(i + 1) / static_cast<double>(n);
        double fnLo = static_cast<double>(i) / static_cast<double>(n);
        d = std::max(d, std::max(fnHi - v[i], v[i] - fnLo));
    }
    return d;
}

/******************************************************************************/
// TSV emission

FILE *g_out = nullptr; // optional second sink

void emit(const std::string &line) {
    std::fputs(line.c_str(), stdout);
    std::fputc('\n', stdout);
    if (g_out) { std::fputs(line.c_str(), g_out); std::fputc('\n', g_out); }
}

void emitHeader() {
    emit("case\tlibrary\tengine\trng_max\tthreads\ttotal_calls\t"
         "throughput_calls_per_sec\tp50_ns\tp99_ns\tp999_ns\tfirst_call_ns\twall_ms");
}

template <typename Traits>
std::string row(const char *c, unsigned threads, std::uint64_t total, double tput,
                double p50, double p99, double p999, double firstNs, double wallMs) {
    char buf[512];
    unsigned long long rmax = static_cast<unsigned long long>((Traits::GRandom::max)());
    std::snprintf(buf, sizeof(buf),
        "%s\t%s\t%s\t%llu\t%u\t%llu\t%.0f\t%.0f\t%.0f\t%.0f\t%.0f\t%.2f",
        c, Traits::lib, Traits::engine(), rmax, threads,
        static_cast<unsigned long long>(total), tput, p50, p99, p999, firstNs, wallMs);
    return std::string(buf);
}

/******************************************************************************/
// Run the full matrix for one library.

struct Config {
    unsigned      sustainedProxies   = 10;
    std::uint64_t sustainedTotal     = 1'000'000'000ULL;
    std::uint64_t warmupPerThread    = 1'000'000ULL;
    unsigned      burstProxies       = 1000;
    std::uint64_t burstPerProxy      = 10'000ULL;
    std::string   only;              // run a single case id, empty = all
};

bool want(const Config &cfg, const char *id) { return cfg.only.empty() || cfg.only == id; }

template <typename Traits>
void runLibrary(const Config &cfg) {
    if (want(cfg, "S-1")) {
        double tp = sustained<Traits>(cfg.sustainedProxies, cfg.sustainedTotal,
                                      cfg.warmupPerThread, RawDraw{});
        emit(row<Traits>("S-1", cfg.sustainedProxies, cfg.sustainedTotal, tp, 0, 0, 0, 0,
                         cfg.sustainedTotal / tp * 1000.));
    }
    if (want(cfg, "S-2")) {
        double tp = sustained<Traits>(cfg.sustainedProxies, cfg.sustainedTotal,
                                      cfg.warmupPerThread, UniformDraw{});
        emit(row<Traits>("S-2", cfg.sustainedProxies, cfg.sustainedTotal, tp, 0, 0, 0, 0,
                         cfg.sustainedTotal / tp * 1000.));
    }
    if (want(cfg, "S-3")) {
        double tp = sustained<Traits>(cfg.sustainedProxies, cfg.sustainedTotal,
                                      cfg.warmupPerThread, NormalDraw{});
        emit(row<Traits>("S-3", cfg.sustainedProxies, cfg.sustainedTotal, tp, 0, 0, 0, 0,
                         cfg.sustainedTotal / tp * 1000.));
    }
    if (want(cfg, "B-1")) {
        BurstResult br = coldStartBurst<Traits>(cfg.burstProxies, cfg.burstPerProxy);
        std::uint64_t total = static_cast<std::uint64_t>(cfg.burstProxies) * cfg.burstPerProxy;
        emit(row<Traits>("B-1", cfg.burstProxies, total,
                         static_cast<double>(total) / (br.wallMs / 1000.),
                         0, 0, 0, br.maxFirstCallNs, br.wallMs));
    }
    if (want(cfg, "B-2")) {
        LatResult lr = burstLatency<Traits>(cfg.burstProxies, cfg.burstPerProxy);
        std::uint64_t total = static_cast<std::uint64_t>(cfg.burstProxies) * cfg.burstPerProxy;
        emit(row<Traits>("B-2", cfg.burstProxies, total, lr.throughput,
                         lr.p50, lr.p99, lr.p999, 0, total / lr.throughput * 1000.));
    }
}

} // namespace

/******************************************************************************/

int main(int argc, char **argv) {
    Config cfg;
    std::string outPath;
    bool        ksMode = false;
    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        if (a == "--quick") {
            cfg.sustainedTotal  = 20'000'000ULL;
            cfg.warmupPerThread = 100'000ULL;
            cfg.burstProxies    = 100;
            cfg.burstPerProxy   = 2'000ULL;
        } else if (a == "--case" && i + 1 < argc) {
            cfg.only = argv[++i];
        } else if (a == "--out" && i + 1 < argc) {
            outPath = argv[++i];
        } else if (a == "--proxies" && i + 1 < argc) {
            cfg.sustainedProxies = static_cast<unsigned>(std::stoul(argv[++i]));
        } else if (a == "--total" && i + 1 < argc) {
            cfg.sustainedTotal = std::stoull(argv[++i]);
        } else if (a == "--ks") {
            ksMode = true;
        } else {
            std::fprintf(stderr,
                "usage: %s [--quick] [--case S-1|S-2|S-3|B-1|B-2] [--out file] "
                "[--proxies N] [--total N] [--ks]\n", argv[0]);
            return 2;
        }
    }
    if (ksMode) {
        std::uint64_t n     = 1'000'000;
        double        dcrit = 1.9495 / std::sqrt(static_cast<double>(n));
        double        dHap  = ksUniform<HapTraits>(n);
        double        dHap2 = ksUniform<Hap2Traits>(n);
        std::printf("K-S uniformity (n=%llu, alpha=1e-3, D_crit=%.6f):\n",
                    static_cast<unsigned long long>(n), dcrit);
        std::printf("  Hap  (%-12s): D=%.6f  %s\n", HapTraits::engine(), dHap,
                    dHap < dcrit ? "PASS" : "FAIL");
        std::printf("  Hap2 (%-12s): D=%.6f  %s\n", Hap2Traits::engine(), dHap2,
                    dHap2 < dcrit ? "PASS" : "FAIL");
        return (dHap < dcrit && dHap2 < dcrit) ? 0 : 1;
    }

    if (!outPath.empty()) {
        g_out = std::fopen(outPath.c_str(), "w");
        if (!g_out) { std::fprintf(stderr, "cannot open %s\n", outPath.c_str()); return 2; }
    }

    emitHeader();
    runLibrary<HapTraits>(cfg);
    runLibrary<Hap2Traits>(cfg);

    if (g_out) std::fclose(g_out);
    return 0;
}
