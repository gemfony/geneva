/**
 * @file GCUDAOptBenchmarkMain.cpp
 *
 * Main driver for the GPU-accelerated algorithm comparison benchmark.
 *
 * The whole population of each generation is scored in one bulk, runtime-compiled kernel launch
 * through the unified courtier GPU consumer (Gem::Courtier::GPU::GGPUConsumerT) -- the SAME consumer
 * example 15 uses. There is no build-time CUDA compilation unit any more: the kernel
 * (kernels/benchmark_eval.cu via NVRTC, or .cl via OpenCL) is loaded at run time, and the backend
 * (cpu/cuda/opencl) is selected in config/GGPUConsumer.json.
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

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <boost/program_options.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#include "courtier/GBrokerT.hpp"
#include "courtier/GConsumerRegistry.hpp"
#include "courtier/gpu/GGPUConsumer.hpp"
#include "geneva/GenevaInitializer.hpp"
#include "GAlgorithmBenchmarkRunner.hpp"
#include "GBenchmarkGPUMarshaller.hpp"
#include "GBenchmarkResultWriter.hpp"

namespace po = boost::program_options;
using namespace Gem::Geneva;
using namespace Gem::Geneva::Benchmarks;

/******************************************************************************/
/**
 * @brief Reads BenchmarkConfig from a JSON file using Boost.PropertyTree.
 *
 * Expected structure:
 * @code
 * {
 *   "benchmark_function":  "PARABOLA",
 *   "n_runs":              30,
 *   "n_dimensions":        10,
 *   "individual_config":   "config/GFunctionIndividual.json",
 *   "output_dir":          ".",
 *   "batch_size":          0,
 *   "flush_timeout_ms":     50,
 *   "algorithm_configs": [
 *     { "tag": "ea_default", "mnemonic": "ea",    "config_file": "config/GEvolutionaryAlgorithm.json" },
 *     { "tag": "sa_default", "mnemonic": "sa",    "config_file": "config/GSimulatedAnnealing.json" },
 *     { "tag": "swarm_default", "mnemonic": "swarm", "config_file": "config/GSwarmAlgorithm.json" }
 *   ]
 * }
 * @endcode
 */
static BenchmarkConfig loadConfig(const std::string &configFile) {
    boost::property_tree::ptree pt;
    boost::property_tree::read_json(configFile, pt);

    BenchmarkConfig cfg;
    cfg.functionName       = pt.get<std::string>("benchmark_function", "PARABOLA");
    cfg.nRuns              = pt.get<std::size_t>("n_runs", 30);
    cfg.nDimensions        = pt.get<std::uint32_t>("n_dimensions", 10);
    cfg.individualConfigFile = pt.get<std::string>("individual_config",
                                                    "config/GFunctionIndividual.json");
    cfg.outputDir          = pt.get<std::string>("output_dir", ".");
    cfg.batchSize          = pt.get<std::size_t>("batch_size", 0);
    cfg.flushTimeoutMs     = pt.get<std::uint32_t>("flush_timeout_ms", 50);

    for (const auto &[key, child] : pt.get_child("algorithm_configs")) {
        AlgorithmEntry entry;
        entry.tag        = child.get<std::string>("tag");
        entry.mnemonic   = child.get<std::string>("mnemonic");
        entry.configFile = child.get<std::string>("config_file");
        cfg.algorithms.push_back(entry);
    }

    return cfg;
}

/******************************************************************************/

int main(int argc, char **argv) {
    std::string configFile = "config/GCUDAOptBenchmark.json";

    po::options_description desc("GCUDAOptBenchmark options");
    desc.add_options()
        ("help,h",   "Show help message")
        ("config,c", po::value<std::string>(&configFile),
                     "Path to GCUDAOptBenchmark.json (default: config/GCUDAOptBenchmark.json)");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << desc << std::endl;
        return 0;
    }

    if (!std::filesystem::exists(configFile)) {
        std::cerr << "Config file not found: " << configFile << std::endl;
        return 1;
    }

    BenchmarkConfig cfg;
    try {
        cfg = loadConfig(configFile);
    } catch (const std::exception &e) {
        std::cerr << "Failed to parse config '" << configFile << "': " << e.what() << std::endl;
        return 1;
    }

    if (cfg.algorithms.empty()) {
        std::cerr << "No algorithm entries in config. Nothing to do." << std::endl;
        return 1;
    }

    std::filesystem::create_directories(cfg.outputDir);

    std::cout << "GCUDAOptBenchmark\n"
              << "  function   : " << cfg.functionName  << "\n"
              << "  dimensions : " << cfg.nDimensions   << "\n"
              << "  runs/algo  : " << cfg.nRuns          << "\n"
              << "  algorithms : " << cfg.algorithms.size() << "\n"
              << "  output dir : " << cfg.outputDir      << "\n"
              << std::endl;

    // Initialize Geneva — must outlive the runner and all optimization.
    Gem::Geneva::GenevaInitializer gi;

    // Build the unified GPU consumer (the SAME GGPUConsumerT example 15 uses) and register it as the
    // process consumer. The whole population is scored in one bulk, runtime-compiled kernel launch; the
    // backend (cpu/cuda/opencl) and kernel are selected in config/GGPUConsumer.json. The clone function
    // is the polymorphic GOptimizableEntity clone needed by the clone-on-partial-return policy.
    auto marshaller = std::make_shared<GBenchmarkGPUMarshaller>();
    auto consumer = std::make_shared<Gem::Courtier::GPU::GGPUConsumerT<gen::GOptimizableEntity>>(
        "./config/GGPUConsumer.json", marshaller);
    consumer->setCloneFunction([](const std::unique_ptr<gen::GOptimizableEntity> &p) {
        return p->clone_unique();
    });
    Gem::Courtier::GConsumerRegistryT<gen::GOptimizableEntity>::instance().setConsumer(consumer);

    GAlgorithmBenchmarkRunner runner(cfg);
    const auto results = runner.run();

    for (const auto &r : results) {
        GBenchmarkResultWriter::writeRawCSV(r, cfg.outputDir);
    }
    GBenchmarkResultWriter::writeSummaryCSV(results, cfg.outputDir);
    GBenchmarkResultWriter::printSummary(results);

    return 0;
}
