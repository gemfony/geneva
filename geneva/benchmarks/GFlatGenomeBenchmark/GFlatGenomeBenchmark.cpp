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
 * A micro-benchmark comparing the tree-based and flat (GFlatParameters) genome
 * representations on the optimization-algorithm hot-path operations: deep clone
 * (every child created each generation is a clone), streamline (parameter
 * extraction for evaluation), and adapt (mutation). The genome is a list of N
 * *separate* constrained-double parameters -- the case the flat representation
 * targets, where the tree pays N object allocations and N virtual dispatches.
 */

#include <chrono>
#include <cstddef>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <memory>
#include <vector>

#include "geneva/par/GConstrainedDoubleObject.hpp"
#include "geneva/par/GFlatParameterSet.hpp"

namespace gpar = Gem::Geneva::Parameters;

namespace {

/** @brief A genome of N separate constrained doubles; flat or tree-backed per the flag. */
class BenchIndividual : public gpar::GFlatParameterSet {
public:
    BenchIndividual(std::size_t n, bool flat) {
        for(std::size_t i = 0; i < n; ++i) {
            this->push_back(std::make_shared<gpar::GConstrainedDoubleObject>(1.0, -5., 5.));
        }
        if(flat) {
            this->compileToFlat();
        }
    }
    BenchIndividual(const BenchIndividual &) = default;

protected:
    double fitnessCalculation() override {
        std::vector<double> v;
        this->streamline<double>(v);
        double s = 0.;
        for(double x : v) {
            s += x * x;
        }
        return s;
    }

private:
    gpar::GParameterSet *clone_() const override {
        return new BenchIndividual(*this);
    }
};

template <typename F>
double timeMs(std::size_t iterations, F &&f) {
    const auto t0 = std::chrono::steady_clock::now();
    for(std::size_t i = 0; i < iterations; ++i) {
        f(i);
    }
    const auto t1 = std::chrono::steady_clock::now();
    return std::chrono::duration<double, std::milli>(t1 - t0).count();
}

} /* anonymous namespace */

int main(int argc, char **argv) {
    const std::size_t n_params = (argc > 1) ? std::strtoul(argv[1], nullptr, 10) : 1000;
    const std::size_t iters = (argc > 2) ? std::strtoul(argv[2], nullptr, 10) : 500;

    BenchIndividual tree(n_params, false);
    BenchIndividual flat(n_params, true);

    // Prevent the optimizer from eliminating the work via a running checksum.
    volatile double sink = 0.;

    // --- clone ---
    const double tree_clone = timeMs(iters, [&](std::size_t) {
        auto c = tree.clone_unique();
        sink += static_cast<double>(reinterpret_cast<std::uintptr_t>(c.get()) & 0xFF);
    });
    const double flat_clone = timeMs(iters, [&](std::size_t) {
        auto c = flat.clone_unique();
        sink += static_cast<double>(reinterpret_cast<std::uintptr_t>(c.get()) & 0xFF);
    });

    // --- streamline ---
    std::vector<double> scratch;
    const double tree_sl = timeMs(iters, [&](std::size_t) {
        scratch.clear();
        tree.streamline<double>(scratch);
        sink += scratch.empty() ? 0. : scratch.front();
    });
    const double flat_sl = timeMs(iters, [&](std::size_t) {
        scratch.clear();
        flat.streamline<double>(scratch);
        sink += scratch.empty() ? 0. : scratch.front();
    });

    // --- adapt ---
    const double tree_ad = timeMs(iters, [&](std::size_t) {
        sink += static_cast<double>(tree.adapt());
    });
    const double flat_ad = timeMs(iters, [&](std::size_t) {
        sink += static_cast<double>(flat.adapt());
    });

    auto row = [](const char *name, double tree_ms, double flat_ms, std::size_t it) {
        std::cout << std::left << std::setw(12) << name << std::right << std::fixed
                  << std::setprecision(4) << std::setw(12) << (tree_ms / static_cast<double>(it))
                  << std::setw(12) << (flat_ms / static_cast<double>(it)) << std::setw(12)
                  << std::setprecision(2) << (tree_ms / flat_ms) << "x" << '\n';
    };

    std::cout << "GFlatGenomeBenchmark: " << n_params << " separate constrained doubles, " << iters
              << " iterations\n\n";
    std::cout << std::left << std::setw(12) << "operation" << std::right << std::setw(12)
              << "tree ms/op" << std::setw(12) << "flat ms/op" << std::setw(13) << "speedup" << '\n';
    std::cout << "-----------------------------------------------------\n";
    row("clone", tree_clone, flat_clone, iters);
    row("streamline", tree_sl, flat_sl, iters);
    row("adapt", tree_ad, flat_ad, iters);

    // Touch the sink so it cannot be optimized away.
    if(sink == 1234567.89) {
        std::cout << "(sink)" << '\n';
    }
    return 0;
}
