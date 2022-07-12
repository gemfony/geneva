/**
 * @file GNetworkedConsumerBenchmarkSubProgram.cpp
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
 * Geneva was started by Dr. Rüdiger Berlich and was later maintained together
 * with Dr. Ariel Garcia under the auspices of Gemfony scientific. For further
 * information on Gemfony scientific, see http://www.gemfomy.eu .
 *
 * The majority of files in Geneva was released under the Apache license v2.0
 * in February 2020.
 *
 * See the NOTICE file in the top-level directory of the Geneva library
 * collection for a list of contributors and copyright information.
 *
 ********************************************************************************/

// Standard header files go here
#include <iostream>
#include <cmath>
#include <string>
#include <fstream>
#include <sstream>

// Boost header files go here

// Geneva header files go here
#include "common/GCommonMathHelperFunctionsT.hpp"
#include "common/GThreadPool.hpp"
#include "geneva/Go2.hpp"
#include "geneva/G_OptimizationAlgorithm_EvolutionaryAlgorithm_Factory.hpp"

// The individual that should be optimized
#include "geneva-individuals/GDelayIndividual.hpp"

using namespace Gem::Geneva;
using namespace Gem::Common;

const std::string executionTimesFileName{"executionTimesVector.ser"};

/******************************************************************************/
/**
 *
 * Optimizes in an endless loop using the first individual returned by the factory
 *
 * @param go A reference to the optimization wrapper
 * @param delayIndividualFactory A factory for delay-individual objects
 */
[[noreturn]] void runTest(
        Go2 &go,
        GDelayIndividualFactory &delayIndividualFactory
) {
    //---------------------------------------------------------------------
    // Loop until no valid individuals can be retrieved anymore
    std::uint32_t interMeasurementDelay = 1;
    std::uint32_t nMeasurementsPerIteration = 5;
    std::uint32_t nBenchmarkIterations = 5;
    std::size_t iter = 0;

    auto gdi_ptr = delayIndividualFactory.get_as<GDelayIndividual>();

    // optimize in an endless loop and let the caller kill this process
    while (true) {
        // add individual to optimizer
        go.push_back(gdi_ptr);

        // optimizer
        go.optimize();

        // reset optimizer
        go.clear();
        std::uint32_t currentIteration{go.getIteration()};
        // continue counting at the next iteration
        go.setOffset(currentIteration);
    }
}

int main(int argc, char **argv) {
    std::vector<std::tuple<double, double, double, double>> executionTimes;
    std::tuple<double, double, double, double> ab;

    // For the parallel measurement
    Go2 go(argc, argv, "./config/Go2.json");


    //---------------------------------------------------------------------
    // Client mode
    if (go.clientMode()) {
        return go.clientRun();
    }

    // TODO: remove this
    std::this_thread::sleep_for(std::chrono::seconds(45)); // sleep to generate connection issues on client side

    GDelayIndividualFactory delayIndividualFactory("./config/GDelayIndividual.json");

    // Use evolutionary algorithm for this benchmark
    GEvolutionaryAlgorithmFactory ea("./config/GEvolutionaryAlgorithm.json");
    std::shared_ptr<GEvolutionaryAlgorithm> eaPtr = ea.get<GEvolutionaryAlgorithm>();

    // do not stop at any number of iterations
    // the executable is killed by the calling process after the preset duration has been elapsed
    eaPtr->setMaxIteration(0);

    // add algorithm to the optimizer
    go & eaPtr;

    std::cout << "Starting GNetworkedConsumerStabilitySubProgram" << std::endl;

    runTest(go, delayIndividualFactory);
}
