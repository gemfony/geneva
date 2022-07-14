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

/******************************************************************************/
/**
 * Starts a series of reference measurements to be compared with the parallel
 * measurements. This will usually mean serial execution. The execution mode
 * is however determined by the caller.
 *
 * @param go A reference to the optimization wrapper
 * @param gdif A factory for delay-individual objects
 * @param ab The parameters a and b of the line best describing all measurements, so that f(x)=a+b*x
 */
void startReferenceMeasurement(
	GDelayIndividualFactory& gdif
	, std::tuple<double,double,double,double>& ab
) {
	std::cout << "Starting reference measurement" << std::endl;

	std::vector<std::tuple<double, double>> referenceExecutionTimes;

	// Create a factory for serial EA algorithms
	GEvolutionaryAlgorithmFactory ea_serial_factory("./config/GEvolutionaryAlgorithm.json");

	// Create an evolutionary algorithm
	auto ea_alg_master = ea_serial_factory.get<GEvolutionaryAlgorithm>();

	// Register an appropriate executor
	ea_alg_master->registerExecutor(execMode::SERIAL, "./config/GSerialExecutor.json");

	//---------------------------------------------------------------------
	// Loop until no valid individuals can be retrieved anymore
	std::uint32_t interMeasurementDelay = 1;
	std::uint32_t nMeasurementsPerIteration = 5;
	std::size_t iter = 0;
	std::shared_ptr<GDelayIndividual> gdi_ptr;
	while((gdi_ptr = gdif.get_as<GDelayIndividual>())) {
		if(0==iter) { // The first individual must already have been produced in order to access parsed data
			// Determine the amount of seconds the process should sleep in between two measurements
			interMeasurementDelay = gdif.getInterMeasurementDelay();
			// Determine the number of measurements to be made for each delay
			nMeasurementsPerIteration = gdif.getNMeasurements();
		}

		for(std::uint32_t i=0; i<nMeasurementsPerIteration; i++) {
			std::cout << "Serial measurement " << i << " in iteration " << iter << std::endl;

			// Create a clone of the evolutionary algorithm
			auto ea_alg = ea_alg_master->clone<GEvolutionaryAlgorithm>();

			// Make the individual known to the optimizer
			ea_alg->push_back(gdi_ptr);

			// Do the actual optimization and measure the time
			std::chrono::system_clock::time_point startTime = std::chrono::system_clock::now();
			ea_alg->optimize();
			std::chrono::system_clock::time_point endTime = std::chrono::system_clock::now();
			std::chrono::duration<double> duration = endTime - startTime;

			referenceExecutionTimes.push_back(
				std::tuple<double,double>(
					gdi_ptr->getFixedSleepTime().count()
					, duration.count()
				)
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
 * @param gdif A factory for delay-individual objects
 * @param parallelExecutionTimes A vector of parallel execution times, together with errors
 */
void startParallelMeasurement(
	Go2& go
	, GDelayIndividualFactory& gdif
	, std::vector<std::tuple<double,double,double,double>>& parallelExecutionTimes
) {
	std::cout << "Starting parallel measurement" << std::endl;

	//---------------------------------------------------------------------
	// Make sure the output vector is empty
	parallelExecutionTimes.clear();

	//---------------------------------------------------------------------
	// Loop until no valid individuals can be retrieved anymore
	std::uint32_t interMeasurementDelay = 1;
	std::uint32_t nMeasurementsPerIteration = 5;
	std::size_t iter = 0;
	std::shared_ptr<GDelayIndividual> gdi_ptr;
 	while((gdi_ptr = gdif.get_as<GDelayIndividual>())) {
		if(0==iter) { // The first individual must already have been produced in order to access parsed data
			// Determine the amount of seconds the process should sleep in between two measurements
			interMeasurementDelay = gdif.getInterMeasurementDelay();
			// Determine the number of measurements to be made for each delay
			nMeasurementsPerIteration = gdif.getNMeasurements();
		}

		std::vector<double> delaySummary;
		for(std::uint32_t i=0; i<nMeasurementsPerIteration; i++) {
			std::cout << "Parallel measurement " << i << " in iteration " << iter << std::endl;

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
            go.setOffset(go.getIteration());
		}

		// Calculate the mean value and standard deviation of all measurements
		std::tuple<double,double> ms = Gem::Common::GStandardDeviation<double>(delaySummary);
		// Output the results
		parallelExecutionTimes.push_back(
			std::tuple<double,double,double,double>(
				gdi_ptr->getFixedSleepTime().count()
				, 0. // No error on the sleep time
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

	std::cout << "End of parallel measurement" << std::endl;
}

/******************************************************************************/
/**
 * Calculate suitable timings including errors for the reference measurement
 */
std::vector<std::tuple<double,double,double,double>> getReferenceTimes(
	const std::tuple<double,double,double,double>& ab
	, const std::vector<std::tuple<double,double,double,double>>& measurementTemplate
) {
	std::vector<std::tuple<double,double,double,double>> referenceExecutionTimes = measurementTemplate;

	std::vector<std::tuple<double,double,double,double>>::iterator it;
	for(it=referenceExecutionTimes.begin(); it!=referenceExecutionTimes.end(); ++it) {
		double sleepTime = std::get<0>(*it); // Left unmodified, taken from measurementTemplate

		double a     = std::get<0>(ab);
		double a_err = std::get<1>(ab);
		double b     = std::get<2>(ab);
		double b_err = std::get<3>(ab);

		std::get<1>(*it) = 0.; // No error on the sleep time
		std::get<2>(*it) = a + b*sleepTime; // a line
		std::get<3>(*it) = sqrt(gpow(a_err, 2.) + gpow(sleepTime*b_err, 2.));
	}

	return referenceExecutionTimes;
}

/******************************************************************************/

int main(int argc, char **argv) {
	std::vector<std::tuple<double,double,double,double>> parallelExecutionTimes, referenceExecutionTimes;
	std::tuple<double,double,double,double> ab;

	// For the parallel measurement
	Go2 go_parallel(argc, argv, "./config/Go2.json");

	//---------------------------------------------------------------------
	// Client mode
	if(go_parallel.clientMode()) {
		return go_parallel.clientRun();
	}

	//---------------------------------------------------------------------
	// Create a factory for GDelayIndividualFactory objects for reference measurements
	GDelayIndividualFactory gdif_ref("./config/GDelayIndividual-reference.json");
	// ... and for parallel measurements
	GDelayIndividualFactory gdif_par("./config/GDelayIndividual.json");

	// Add default optimization algorithms to the parallel Go2 object
	go_parallel.registerDefaultAlgorithm("ea");

	// Threadpool for two threads
	Gem::Common::GThreadPool tp(2);

	// Start the reference and parallel threads
	tp.async_schedule([&](){ startReferenceMeasurement(gdif_ref, ab); });
	tp.async_schedule([&](){ startParallelMeasurement(go_parallel, gdif_par, parallelExecutionTimes); });
	std::cout << "Waiting for threads to return" << std::endl;
	// And wait for their return
	tp.wait();

	// Calculate reference times from the line parameters
	referenceExecutionTimes = getReferenceTimes(ab, parallelExecutionTimes);

	// Calculate the errors
	std::vector<std::tuple<double, double, double, double>> ratioWithErrors = getRatioErrors(
		referenceExecutionTimes
		, parallelExecutionTimes
	);

	//---------------------------------------------------------------------
	// Will hold all plot information
	std::shared_ptr<GGraph2ED> greference_ptr(new GGraph2ED());
	greference_ptr->setPlotLabel("Serial execution times and errors");

	std::shared_ptr<GGraph2ED> gparallel_ptr(new GGraph2ED());
	gparallel_ptr->setPlotLabel("Parallel execution times and errors");

	std::shared_ptr<GGraph2ED> gratio_ptr(new GGraph2ED());
	gratio_ptr->setPlotLabel("Speedup: serial/parallel execution times and errors");

	(*greference_ptr) & referenceExecutionTimes;
	(*gparallel_ptr)  & parallelExecutionTimes;
	(*gratio_ptr)     & ratioWithErrors;

	GPlotDesigner gpd("Processing times and speed-up as a function of evaluation time", 1,3);

	gpd.registerPlotter(greference_ptr);
	gpd.registerPlotter(gparallel_ptr);
	gpd.registerPlotter(gratio_ptr);

	gpd.setCanvasDimensions(800,1200);
	gpd.writeToFile(gdif_par.getResultFileName());

	return 0;
}
