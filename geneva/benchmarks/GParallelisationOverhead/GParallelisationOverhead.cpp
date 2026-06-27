/**
 * @file GParallelisationOverhead.cpp
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

// Standard header files go here
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

// Boost header files go here

// Geneva header files go here
#include "common/GCommonMathHelperFunctionsT.hpp"
#include "dietrich/GPlotDesigner.hpp"
#include "common/concurrency/GThreadPool.hpp"
#include "geneva/oa/GAdaption.hpp"
#include "geneva/oa/GEvolutionaryAlgorithmFactory.hpp"
#include "geneva/Go2.hpp"

// The individual that should be optimized
#include "geneva/individuals/GDelayIndividual.hpp"

using namespace Gem::Geneva;
using namespace Gem::Common;
using namespace Gem::Common::Concurrency;
using namespace Gem::Dietrich; // plotting types live here now

/******************************************************************************/
/**
 * Starts a series of reference measurements to be compared with the parallel
 * measurements. This will usually mean serial execution. The execution mode
 * is however determined by the caller.
 *
 * @param delay_config The parsed delay-individual configuration
 * @param ab The parameters a and b of the line best describing all measurements, so that f(x)=a+b*x
 */
void startReferenceMeasurement(
    const gind::GDelayIndividual::Config &delay_config,
    std::tuple<double, double, double, double> &ab
) {
    std::cout << "Starting reference measurement" << std::endl;

    std::vector<std::tuple<double, double>> referenceExecutionTimes;

    // Create a factory for serial EA algorithms
    oa::GEvolutionaryAlgorithmFactory ea_serial_factory("./config/GEvolutionaryAlgorithm.json");

    // Create an evolutionary algorithm
    auto ea_alg_master = ea_serial_factory.get<oa::GEvolutionaryAlgorithm>();

    // The algorithm submits through the process-wide consumer (a default local thread-pool consumer
    // is lazily built if none has been registered).

    //---------------------------------------------------------------------
    // Cycle through the configured delays, building a fresh delay individual for each one.
    const std::uint32_t interMeasurementDelay = delay_config.inter_measurement_delay;
    const std::uint32_t nMeasurementsPerIteration = delay_config.n_measurements;
    const auto sleep_times = gind::GDelayIndividual::parseSleepTimes(delay_config);
    std::size_t iter = 0;
    for(const auto &sleep_time_tuple : sleep_times) {
        auto gdi_ptr = gind::GDelayIndividual::create(
            delay_config,
            gind::GDelayIndividual::tupleToTime(sleep_time_tuple)
        );

        for(std::uint32_t i = 0; i < nMeasurementsPerIteration; i++) {
            std::cout << "Serial measurement " << i << " in iteration " << iter << std::endl;

            // Create a clone of the evolutionary algorithm
            auto ea_alg = ea_alg_master->clone<oa::GEvolutionaryAlgorithm>();

            // Make the individual known to the optimizer
            ea_alg->push_back(gdi_ptr->clone_unique());

            // GDelay's genome is transport ballast (its values are irrelevant to the timing benchmark),
            // but the EA still mutates + re-evaluates it every generation -- that cycle IS the measured
            // workload. Author a real Gauss adaptor on its OA-owned config and hand it to the EA.
            {
                auto cfg = oa::makeAdaptionConfig<oa::GAdaptionConfigBase>(*gdi_ptr);
                for(std::size_t i = 0; i < cfg->doubleGroups().size(); i++) {
                    cfg->groupDouble(i).gauss(0.025, 0.1, 0., 1., 1.);
                }
                ea_alg->setAdaptionConfig(cfg);
            }

            // Do the actual optimization and measure the time
            std::chrono::system_clock::time_point startTime = std::chrono::system_clock::now();
            ea_alg->optimize();
            std::chrono::system_clock::time_point endTime = std::chrono::system_clock::now();
            std::chrono::duration<double> duration = endTime - startTime;

            referenceExecutionTimes.push_back(
                std::tuple<double, double>(gdi_ptr->getFixedSleepTime().count(), duration.count())
            );
        }

        // Wait for late arrivals
        std::this_thread::sleep_for(std::chrono::seconds(interMeasurementDelay));

        // Increment the iteration counter
        iter++;
    }

    // Calculate the regression parameters a and b, including errors
    ab = getRegressionParameters(referenceExecutionTimes);

    std::cout << "End of reference measurement" << std::endl;
}

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
 * @param delay_config The parsed delay-individual configuration
 * @param parallelExecutionTimes A vector of parallel execution times, together with errors
 */
void startParallelMeasurement(
    Go2 &go,
    const gind::GDelayIndividual::Config &delay_config,
    std::vector<std::tuple<double, double, double, double>> &parallelExecutionTimes
) {
    std::cout << "Starting parallel measurement" << std::endl;

    //---------------------------------------------------------------------
    // Make sure the output vector is empty
    parallelExecutionTimes.clear();

    //---------------------------------------------------------------------
    // Cycle through the configured delays, building a fresh delay individual for each one.
    const std::uint32_t interMeasurementDelay = delay_config.inter_measurement_delay;
    const std::uint32_t nMeasurementsPerIteration = delay_config.n_measurements;
    const auto sleep_times = gind::GDelayIndividual::parseSleepTimes(delay_config);
    std::size_t iter = 0;
    for(const auto &sleep_time_tuple : sleep_times) {
        auto gdi_ptr = gind::GDelayIndividual::create(
            delay_config,
            gind::GDelayIndividual::tupleToTime(sleep_time_tuple)
        );

        std::vector<double> delaySummary;
        for(std::uint32_t i = 0; i < nMeasurementsPerIteration; i++) {
            std::cout << "Parallel measurement " << i << " in iteration " << iter << std::endl;

            // Make the individual known to the optimizer
            go.push_back(gdi_ptr);

            // Do the actual optimization and measure the time. Each run starts at
            // the accumulated iteration (passed as the offset) so the absolute
            // max-iteration halt criterion does not fire immediately on the second
            // and later measurements.
            std::chrono::system_clock::time_point startTime = std::chrono::system_clock::now();
            go.optimize(go.getIteration());
            std::chrono::system_clock::time_point endTime = std::chrono::system_clock::now();
            std::chrono::duration<double> duration = endTime - startTime;

            delaySummary.push_back(duration.count());

            // Clean up the collection
            go.clear();
        }

        // Calculate the mean value and standard deviation of all measurements
        std::tuple<double, double> ms = Gem::Common::GStandardDeviation<double>(delaySummary);
        // Output the results
        parallelExecutionTimes.push_back(
            std::tuple<double, double, double, double>(
                gdi_ptr->getFixedSleepTime().count(),
                0. // No error on the sleep time
                ,
                std::get<0>(ms) // mean
                ,
                std::get<1>(ms) // standard deviation
            )
        );

        // Clean up the delay vector
        delaySummary.clear();

        // Wait for late arrivals
        std::this_thread::sleep_for(std::chrono::seconds(interMeasurementDelay));

        // Increment the iteration counter
        iter++;
    }

    std::cout << "End of parallel measurement" << std::endl;
}

/******************************************************************************/
/**
 * Calculate suitable timings including errors for the reference measurement
 */
std::vector<std::tuple<double, double, double, double>> getReferenceTimes(
    const std::tuple<double, double, double, double> &ab,
    const std::vector<std::tuple<double, double, double, double>> &measurementTemplate
) {
    std::vector<std::tuple<double, double, double, double>> referenceExecutionTimes =
        measurementTemplate;

    std::vector<std::tuple<double, double, double, double>>::iterator it;
    for(it = referenceExecutionTimes.begin(); it != referenceExecutionTimes.end(); ++it) {
        double sleepTime = std::get<0>(*it); // Left unmodified, taken from measurementTemplate

        double a = std::get<0>(ab);
        double a_err = std::get<1>(ab);
        double b = std::get<2>(ab);
        double b_err = std::get<3>(ab);

        std::get<1>(*it) = 0.;                // No error on the sleep time
        std::get<2>(*it) = a + b * sleepTime; // a line
        std::get<3>(*it) = sqrt(std::pow(a_err, 2.) + std::pow(sleepTime * b_err, 2.));
    }

    return referenceExecutionTimes;
}

/******************************************************************************/

int main(int argc, char **argv) {
    std::vector<std::tuple<double, double, double, double>> parallelExecutionTimes,
        referenceExecutionTimes;
    std::tuple<double, double, double, double> ab;

    // For the parallel measurement
    Go2 go_parallel(argc, argv, "./config/Go2.json");

    //---------------------------------------------------------------------
    // Client mode
    if(go_parallel.clientMode()) {
        return go_parallel.clientRun();
    }

    //---------------------------------------------------------------------
    // Read the delay-individual configuration for reference measurements ...
    auto delay_config_ref =
        gind::GDelayIndividual::readConfig("./config/GDelayIndividual-reference.json");
    // ... and for parallel measurements
    auto delay_config_par = gind::GDelayIndividual::readConfig("./config/GDelayIndividual.json");

    // Add default optimization algorithms to the parallel Go2 object
    go_parallel.registerDefaultAlgorithm("ea");

    // Threadpool for two threads
    Gem::Common::Concurrency::GThreadPool tp(2);

    // Start the reference and parallel threads
    tp.async_schedule([&]() { startReferenceMeasurement(delay_config_ref, ab); });
    tp.async_schedule([&]() {
        startParallelMeasurement(go_parallel, delay_config_par, parallelExecutionTimes);
    });
    std::cout << "Waiting for threads to return" << std::endl;
    // And wait for their return
    tp.wait();

    // Calculate reference times from the line parameters
    referenceExecutionTimes = getReferenceTimes(ab, parallelExecutionTimes);

    // Calculate the errors
    std::vector<std::tuple<double, double, double, double>> ratioWithErrors =
        getRatioErrors(referenceExecutionTimes, parallelExecutionTimes);

    //---------------------------------------------------------------------
    // Will hold all plot information
    std::shared_ptr<GGraph2ED> greference_ptr(new GGraph2ED());
    greference_ptr->setPlotLabel("Serial execution times and errors");

    std::shared_ptr<GGraph2ED> gparallel_ptr(new GGraph2ED());
    gparallel_ptr->setPlotLabel("Parallel execution times and errors");

    std::shared_ptr<GGraph2ED> gratio_ptr(new GGraph2ED());
    gratio_ptr->setPlotLabel("Speedup: serial/parallel execution times and errors");

    (*greference_ptr) & referenceExecutionTimes;
    (*gparallel_ptr) & parallelExecutionTimes;
    (*gratio_ptr) & ratioWithErrors;

    GPlotDesigner gpd("Processing times and speed-up as a function of evaluation time", 1, 3);

    gpd.registerPlotter(greference_ptr);
    gpd.registerPlotter(gparallel_ptr);
    gpd.registerPlotter(gratio_ptr);

    gpd.setCanvasDimensions(800, 1200);
    gpd.writeToFile(delay_config_par.result_file);

    return 0;
}
