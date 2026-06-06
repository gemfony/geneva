/**
 * @file GCUDAOptBenchmark.cu
 *
 * Thin CUDA compilation unit for the GCUDAOptBenchmark target.
 *
 * Only responsible for creating the CUDA consumer and enrolling it with the
 * broker.  All C++20 code (GenevaInitializer, main, config loading, result
 * writing) lives in GCUDAOptBenchmarkMain.cpp which is compiled by the host
 * C++20 compiler and therefore not subject to NVCC's C++17 restriction.
 *
 * This file deliberately avoids including GenevaInitializer.hpp (or any
 * header that transitively pulls in GGlobalOptionsT.hpp) because that header
 * uses std::map::contains() which requires C++20.
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

#include <memory>

#include "courtier/GBrokerT.hpp"
#include "geneva/par/GParameterSet.hpp"
#include "GBenchmarkCUDAConsumer.hpp"

/******************************************************************************/
/**
 * @brief Builds a courtier broker holding the GPU batch consumer.
 *
 * Called from GCUDAOptBenchmarkMain.cpp::main() after GenevaInitializer has been constructed there.
 * The returned broker is handed to GAlgorithmBenchmarkRunner, which injects it into each algorithm
 * via setBroker(); the broker (and thus the consumer + its persistent GPU context) lives as
 * long as the runner. The clone function is the polymorphic GParameterSet clone, needed by the
 * clone-on-partial-return policy (used by the population-based algorithms).
 */
std::shared_ptr<Gem::Courtier::GBrokerT<gpar::GParameterSet>> createCUDABroker() {
    auto broker   = std::make_shared<Gem::Courtier::GBrokerT<gpar::GParameterSet>>();
    auto consumer = std::make_shared<Gem::Geneva::Benchmarks::GCUDABatchConsumer>();
    consumer->setCloneFunction([](const std::shared_ptr<gpar::GParameterSet> &p) {
        return p->clone<gpar::GParameterSet>();
    });
    broker->registerConsumer(consumer);
    return broker;
}
