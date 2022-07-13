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

// Boost header files go here

// Geneva header files go here
#include "common/GCommonMathHelperFunctionsT.hpp"
#include "geneva/Go2.hpp"
#include "geneva/G_OptimizationAlgorithm_EvolutionaryAlgorithm_Factory.hpp"

// The individual that should be optimized
#include "geneva-individuals/GDelayIndividual.hpp"

using namespace Gem::Geneva;
using namespace Gem::Common;

const std::string executionTimesFileName{"executionTimesVector.ser"};

/******************************************************************************/
/**
 * Starts a series of (usually parallel) measurements. The tuples in the return-
 * argument has the following structure:
 * - The sleep-time
 * - The error on the sleep-time (always 0)
 * - The mean value of all measurements of an iteration
 * - The sigma / error of the mean value
 *
 * @param go A reference to the optimization wrapper
 * @param delayIndividualFactory A factory for delay-individual objects
 * @param parallelExecutionTimes A vector of parallel execution times, together with errors
 */
void measureExecutionTimes(
        Go2 &go, GDelayIndividualFactory &delayIndividualFactory,
        std::vector<std::tuple<double, double, double, double>> &parallelExecutionTimes,
        const std::uint32_t &optAlgIterations
) {
    std::cout << "Starting measurement" << std::endl;

    //---------------------------------------------------------------------
    // Make sure the output vector is empty
    parallelExecutionTimes.clear();

    //---------------------------------------------------------------------
    // Loop until no valid individuals can be retrieved anymore
    std::uint32_t interMeasurementDelay = 1;
    std::uint32_t nMeasurementsPerIteration = 5;
    std::uint32_t nBenchmarkIterations = 5;
    std::size_t iter = 0;
    std::shared_ptr<GDelayIndividual> gdi_ptr;
    while ((gdi_ptr = delayIndividualFactory.get_as<GDelayIndividual>())) {
        if (0 == iter) { // The first individual must already have been produced in order to access parsed data
            // Determine the amount of seconds the process should sleep in between two measurements
            interMeasurementDelay = delayIndividualFactory.getInterMeasurementDelay();
            // Determine the number of measurements to be made for each delay
            nMeasurementsPerIteration = delayIndividualFactory.getNMeasurements();
            // determine the number of iterations
            nBenchmarkIterations = delayIndividualFactory.getNDelays();
        }

        std::vector<double> delaySummary;
        for (std::uint32_t i = 0; i < nMeasurementsPerIteration; i++) {
            std::cout << "Measurement " << i + 1 << "/" << nMeasurementsPerIteration
                      << " in iteration " << iter + 1 << "/" << nBenchmarkIterations << std::endl;

            // Make the individual known to the optimizer
            go.push_back(gdi_ptr);

            // Do the actual optimization and measure the time
            std::chrono::system_clock::time_point startTime = std::chrono::system_clock::now();
            go.optimize();
            std::chrono::system_clock::time_point endTime = std::chrono::system_clock::now();
            std::chrono::duration<double> duration = endTime - startTime;

            delaySummary.push_back(duration.count());

            // Clean up the collection
            go.clear();
            std::uint32_t currentIteration{go.getIteration()};
            // continue counting at the next iteration
            go.setOffset(currentIteration);
            // set the iteration to stop to a new number
            go.getRegisteredAlgorithms()[0]->setMaxIteration(currentIteration + optAlgIterations);
            // we want min=max to stop exactly at max. But that is forbidden, so we set min = max - 1
            go.getRegisteredAlgorithms()[0]->setMinIteration(currentIteration + optAlgIterations - 1);
        }

        // Calculate the mean value and standard deviation of all measurements
        std::tuple<double, double> ms = Gem::Common::GStandardDeviation<double>(delaySummary);
        // Output the results
        parallelExecutionTimes.push_back(
                std::tuple<double, double, double, double>(
                        gdi_ptr->getFixedSleepTime().count(), 0. // No error on the sleep time
                        , std::get<0>(ms) // mean
                        , std::get<1>(ms) // standard deviation
                )
        );

        // Clean up the delay vector
        delaySummary.clear();

        // Wait for late arrivals
        std::this_thread::sleep_for(std::chrono::seconds(interMeasurementDelay));

        // Increment the iteration counter
        iter++;
    }

    std::cout << "End of measurement" << std::endl;
}

/**
 * Serialize the execution times and write them to a file
 */
void executionTimesToFile(const std::vector<std::tuple<double, double, double, double>> &executionTimes) {
    std::ofstream ofs(executionTimesFileName);
    boost::archive::text_oarchive oa(ofs);
    oa & executionTimes;
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

    GDelayIndividualFactory delayIndividualFactory("./config/GDelayIndividual.json");

    // Use evolutionary algorithm for this benchmark
    GEvolutionaryAlgorithmFactory ea("./config/GEvolutionaryAlgorithm.json");
    std::shared_ptr<GEvolutionaryAlgorithm> eaPtr = ea.get<GEvolutionaryAlgorithm>();

    // In this benchmark we want to run exactly the number of maxIterations
    const std::uint32_t optAlgIterations = eaPtr->getMaxIteration();
    // set minimum to maximum -1
    // unfortunately we cannot set min=max, which would be best because then we would get exactly that amount of iterations
    eaPtr->setMinIteration(optAlgIterations - 1);

    // add algorithm to the optimizer
    go & eaPtr;

    measureExecutionTimes(go, delayIndividualFactory, executionTimes, optAlgIterations);

    executionTimesToFile(executionTimes);

    std::shared_ptr<GGraph2ED> graph(new GGraph2ED());
    graph->setPlotLabel("Execution times");

    (*graph) & executionTimes;

    GPlotDesigner gpd("Processing times for different evaluation times of individuals ", 1, 1);

    gpd.registerPlotter(graph);

    gpd.setCanvasDimensions(800, 1200);
    gpd.writeToFile(delayIndividualFactory.getResultFileName());


    return 0;
}
