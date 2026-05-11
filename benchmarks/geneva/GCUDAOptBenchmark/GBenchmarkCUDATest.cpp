/**
 * @file GBenchmarkCUDATest.cpp
 *
 * @brief Validates GPU batch evaluation against CPU reference for all 15
 *        GFunctionIndividual benchmark functions.
 *
 * For each function the test:
 *   1. Fills N individuals with random parameter vectors.
 *   2. Evaluates them on the CPU via BM::eval() (reference).
 *   3. Evaluates the same parameter vectors on the GPU via
 *      batchEvalBenchmarkGPU().
 *   4. Reports the maximum and mean absolute difference.
 *
 * A difference <= 1e-9 is considered a pass; larger differences are flagged
 * as failures and the program exits with a non-zero code.
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
 ********************************************************************************/

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <random>
#include <string>
#include <vector>

#include "GBenchmarkBatchEvaluator.cuh"
#include "geneva-individuals/GBenchmarkFunctions.hpp"

namespace BM = Gem::Geneva::BM;

/******************************************************************************/

namespace {

// Per-function evaluation domains — keep consistent with GFunctionIndividual defaults.
struct FuncInfo {
    int id;
    std::string name;
    double lo; ///< lower bound for random parameter generation
    double hi; ///< upper bound for random parameter generation
};

constexpr int N_INDIVIDUALS = 512; ///< batch size
constexpr int DIM = 16;            ///< parameter dimension
// GPU uses FMA (fused multiply-add) which can give slightly different results
// than CPU sequential ops.  We therefore use a mixed tolerance:
//   |cpu - gpu| / max(|cpu|, 1.0) < REL_TOL
// This is tight for small values and scales with magnitude for functions whose
// output can be large (e.g. Zakharov where s2^4 ~ 10^11 at dim=16).
constexpr double REL_TOL = 1e-10;

const FuncInfo FUNCTIONS[] = {
    {BM::FUNC_PARABOLA, "Parabola", -10.0, 10.0},
    {BM::FUNC_NOISYPARABOLA, "NoisyParabola", -10.0, 10.0},
    {BM::FUNC_ROSENBROCK, "Rosenbrock", -10.0, 10.0},
    {BM::FUNC_ACKLEY, "Ackley (pairwise)", -10.0, 10.0},
    {BM::FUNC_RASTRIGIN, "Rastrigin", -10.0, 10.0},
    {BM::FUNC_SCHWEFEL, "Schwefel", -500.0, 500.0},
    {BM::FUNC_SALOMON, "Salomon", -10.0, 10.0},
    {BM::FUNC_NEGPARABOLA, "NegParabola", -10.0, 10.0},
    {BM::FUNC_ACKLEY_CANONICAL, "Ackley (canonical)", -32.768, 32.768},
    {BM::FUNC_GRIEWANK, "Griewank", -600.0, 600.0},
    {BM::FUNC_LEVY, "Levy", -10.0, 10.0},
    {BM::FUNC_STYBLINSKI_TANG, "StyblinskiTang", -5.0, 5.0},
    {BM::FUNC_ELLIPSOID, "Ellipsoid", -5.0, 5.0},
    {BM::FUNC_MICHALEWICZ, "Michalewicz", 0.0, 3.14159265358979},
    {BM::FUNC_ZAKHAROV, "Zakharov", -5.0, 10.0},
};

constexpr int N_FUNCTIONS = static_cast<int>(sizeof(FUNCTIONS) / sizeof(FUNCTIONS[0]));

} // anonymous namespace

/******************************************************************************/

int main() {
    std::mt19937_64 rng(42);

    bool all_passed = true;

    std::cout << std::left << std::setw(22) << "Function" << std::right << std::setw(14)
              << "MaxRelErr" << std::setw(14) << "MeanRelErr"
              << "  Status\n"
              << std::string(56, '-') << "\n";

    for(int fi = 0; fi < N_FUNCTIONS; ++fi) {
        const FuncInfo &info = FUNCTIONS[fi];

        std::uniform_real_distribution<double> dist(info.lo, info.hi);

        // Build flat row-major parameter buffer.
        const std::size_t p_count = static_cast<std::size_t>(N_INDIVIDUALS) * DIM;
        std::vector<double> h_params(p_count);
        for(auto &v : h_params)
            v = dist(rng);

        // CPU reference.
        std::vector<double> cpu_results(static_cast<std::size_t>(N_INDIVIDUALS));
        for(int i = 0; i < N_INDIVIDUALS; ++i) {
            cpu_results[static_cast<std::size_t>(i)] =
                BM::eval(info.id, h_params.data() + static_cast<std::ptrdiff_t>(i * DIM), DIM);
        }

        // GPU evaluation.
        std::vector<double> gpu_results(static_cast<std::size_t>(N_INDIVIDUALS));
        try {
            Gem::Geneva::batchEvalBenchmarkGPU(
                h_params.data(),
                gpu_results.data(),
                N_INDIVIDUALS,
                DIM,
                info.id
            );
        }
        catch(const std::exception &e) {
            std::cerr << "CUDA error for " << info.name << ": " << e.what() << "\n";
            all_passed = false;
            continue;
        }

        // Mixed absolute/relative error: |cpu-gpu| / max(|cpu|, 1.0).
        // This gives the same result as absolute error near zero and scales
        // correctly for functions whose output can be very large (Zakharov, Rosenbrock).
        std::vector<double> rel_errs(static_cast<std::size_t>(N_INDIVIDUALS));
        for(std::size_t i = 0; i < static_cast<std::size_t>(N_INDIVIDUALS); ++i) {
            const double denom = std::max(std::abs(cpu_results[i]), 1.0);
            rel_errs[i] = std::abs(cpu_results[i] - gpu_results[i]) / denom;
        }

        const double max_rel = *std::max_element(rel_errs.begin(), rel_errs.end());
        const double mean_rel = std::accumulate(rel_errs.begin(), rel_errs.end(), 0.0) /
                                static_cast<double>(N_INDIVIDUALS);

        const bool passed = (max_rel <= REL_TOL);
        if(!passed)
            all_passed = false;

        std::cout << std::left << std::setw(22) << info.name << std::right << std::setw(14)
                  << std::scientific << std::setprecision(3) << max_rel << std::setw(14) << mean_rel
                  << "  " << (passed ? "PASS" : "FAIL") << "\n";
    }

    std::cout << std::string(56, '-') << "\n"
              << (all_passed ? "All tests PASSED.\n" : "One or more tests FAILED.\n");

    return all_passed ? EXIT_SUCCESS : EXIT_FAILURE;
}
