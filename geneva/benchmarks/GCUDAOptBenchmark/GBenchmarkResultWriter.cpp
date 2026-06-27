/**
 * @file GBenchmarkResultWriter.cpp
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

#include "GBenchmarkResultWriter.hpp"

#include <algorithm>
#include <chrono>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace Gem::Geneva::Benchmarks {

/******************************************************************************/

std::string GBenchmarkResultWriter::datestamp() {
    const auto now  = std::chrono::system_clock::now();
    const std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm tm{};
    localtime_r(&t, &tm);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y%m%d_%H%M%S");
    return oss.str();
}

std::string GBenchmarkResultWriter::sanitize(const std::string &s) {
    std::string out = s;
    std::replace_if(out.begin(), out.end(),
        [](char c){ return c == '/' || c == '\\' || c == ' '; }, '_');
    return out;
}

/******************************************************************************/

void GBenchmarkResultWriter::writeRawCSV(
    const GAlgorithmBenchmarkResult &result,
    const std::string &outputDir
) {
    const std::string filename =
        outputDir + "/raw_"
        + sanitize(result.algorithmTag) + "_"
        + sanitize(result.functionName) + "_"
        + datestamp() + ".csv";

    std::ofstream ofs(filename);
    if (!ofs) {
        throw std::runtime_error("GBenchmarkResultWriter: cannot open '" + filename + "'");
    }

    ofs << "run_index,algorithm_tag,function,nDims,final_fitness,"
           "iterations,wall_time_s,termination_reason,target_reached\n";

    for (const auto &r : result.rawRuns) {
        ofs << r.runIndex            << ','
            << r.algorithmTag        << ','
            << r.functionName        << ','
            << r.nDimensions         << ','
            << std::scientific << std::setprecision(12) << r.finalFitness << ','
            << r.iterationsConsumed  << ','
            << std::fixed << std::setprecision(6) << r.wallTimeSeconds << ','
            << terminationReasonToString(r.terminationReason) << ','
            << (r.targetReached ? "1" : "0")
            << '\n';
    }

    std::cout << "Wrote raw CSV: " << filename << std::endl;
}

/******************************************************************************/

void GBenchmarkResultWriter::writeSummaryCSV(
    const std::vector<GAlgorithmBenchmarkResult> &results,
    const std::string &outputDir
) {
    const std::string filename =
        outputDir + "/summary_" + datestamp() + ".csv";

    std::ofstream ofs(filename);
    if (!ofs) {
        throw std::runtime_error("GBenchmarkResultWriter: cannot open '" + filename + "'");
    }

    ofs << "algorithm_tag,function,nDims,nRuns,"
           "mean_fitness,sigma_fitness,"
           "mean_iterations,sigma_iterations,"
           "mean_wall_time_s,sigma_wall_time_s,"
           "success_rate\n";

    for (const auto &r : results) {
        ofs << r.algorithmTag  << ','
            << r.functionName  << ','
            << r.nDimensions   << ','
            << r.nRuns         << ','
            << std::scientific << std::setprecision(12)
            << r.meanFinalFitness  << ',' << r.sigmaFinalFitness  << ','
            << std::fixed << std::setprecision(3)
            << r.meanIterations   << ',' << r.sigmaIterations   << ','
            << std::setprecision(6)
            << r.meanWallTime     << ',' << r.sigmaWallTime     << ','
            << std::setprecision(4) << r.successRate
            << '\n';
    }

    std::cout << "Wrote summary CSV: " << filename << std::endl;
}

/******************************************************************************/

void GBenchmarkResultWriter::printSummary(
    const std::vector<GAlgorithmBenchmarkResult> &results
) {
    std::cout << "\n========== Benchmark Summary ==========\n"
              << std::left
              << std::setw(20) << "Tag"
              << std::setw(14) << "Function"
              << std::setw(7)  << "Dims"
              << std::setw(7)  << "Runs"
              << std::setw(18) << "Fitness (mean±σ)"
              << std::setw(18) << "Iter (mean±σ)"
              << std::setw(18) << "Time s (mean±σ)"
              << "Success%"
              << '\n'
              << std::string(110, '-') << '\n';

    for (const auto &r : results) {
        std::ostringstream fitness, iter, wall;
        fitness << std::scientific << std::setprecision(3)
                << r.meanFinalFitness << "±" << r.sigmaFinalFitness;
        iter    << std::fixed << std::setprecision(1)
                << r.meanIterations << "±" << r.sigmaIterations;
        wall    << std::fixed << std::setprecision(2)
                << r.meanWallTime << "±" << r.sigmaWallTime;

        std::cout << std::left
                  << std::setw(20) << r.algorithmTag
                  << std::setw(14) << r.functionName
                  << std::setw(7)  << r.nDimensions
                  << std::setw(7)  << r.nRuns
                  << std::setw(18) << fitness.str()
                  << std::setw(18) << iter.str()
                  << std::setw(18) << wall.str()
                  << std::setprecision(1) << (r.successRate * 100.0) << "%"
                  << '\n';
    }
    std::cout << std::string(110, '=') << '\n';
}

/******************************************************************************/

} /* namespace Gem::Geneva::Benchmarks */
