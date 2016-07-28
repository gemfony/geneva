/**
 * @file GParallelisationOverhead.cpp
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt)
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) eu
 *
 * This file is part of the Geneva library collection.
 *
 * Geneva was developed with kind support from Karlsruhe Institute of
 * Technology (KIT) and Steinbuch Centre for Computing (SCC). Further
 * information about KIT and SCC can be found at http://www.kit.edu/english
 * and http://scc.kit.edu .
 *
 * Geneva is free software: you can redistribute and/or modify it under
 * the terms of version 3 of the GNU Affero General Public License
 * as published by the Free Software Foundation.
 *
 * Geneva is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with the Geneva library. If not, see <http://www.gnu.org/licenses/>.
 *
 * For further information on Gemfony scientific and Geneva, visit
 * http://www.gemfony.eu .
 */

// Standard header files go here
#include <iostream>
#include <cmath>
#include <string>
#include <fstream>
#include <sstream>

// Boost header files go here

// Geneva header files go here
#include <common/GMathHelperFunctionsT.hpp>
#include <common/GThreadPool.hpp>
#include <geneva/Go2.hpp>

// The individual that should be optimized
#include "GDelayIndividual.hpp"

using namespace Gem::Geneva;
using namespace Gem::Common;
using namespace Gem::Tests;

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
	Go2& go
	, GDelayIndividualFactory& gdif
	, std::tuple<double,double,double,double>& ab
) {
	std::cout << "Starting reference measurement" << std::endl;

	std::vector<std::tuple<double, double>> referenceExecutionTimes;

	//---------------------------------------------------------------------
	// Loop until no valid individuals can be retrieved anymore
	std::uint32_t interMeasurementDelay = 1;
	std::uint32_t nMeasurementsPerIteration = 5;
	std::size_t iter = 0;
	std::shared_ptr<GDelayIndividual> gdi_ptr;
	while((gdi_ptr = gdif.get<GDelayIndividual>())) {
		if(0==iter) { // The first individual must already have been produced in order to access parsed data
			// Determine the amount of seconds the process should sleep in between two measurements
			interMeasurementDelay = gdif.getInterMeasurementDelay();
			// Determine the number of measurements to be made for each delay
			nMeasurementsPerIteration = gdif.getNMeasurements();
		}

		for(std::uint32_t i=0; i<nMeasurementsPerIteration; i++) {
			std::cout << "Serial measurement " << i << " in iteration " << iter << std::endl;

			// Make the individual known to the optimizer
			go.push_back(gdi_ptr);

			// Do the actual optimization and measure the time
			std::chrono::steady_clock::time_point startTime = std::chrono::steady_clock::now();
			go.optimize<GDelayIndividual>();
			std::chrono::steady_clock::time_point endTime = std::chrono::steady_clock::now();
			std::chrono::duration<double> duration = endTime - startTime;

			referenceExecutionTimes.push_back(
				std::tuple<double,double>(
					gdi_ptr->getSleepTime().count()
					, duration.count()
				)
			);

			// Clean up the collection
			go.clear();
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
 	while((gdi_ptr = gdif.get<GDelayIndividual>())) {
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
			std::chrono::steady_clock::time_point startTime = std::chrono::steady_clock::now();
			go.optimize<GDelayIndividual>();
			std::chrono::steady_clock::time_point endTime = std::chrono::steady_clock::now();
			std::chrono::duration<double> duration = endTime - startTime;

			delaySummary.push_back(duration.count());

			// Clean up the collection
			go.clear();
		}

		// Calculate the mean value and standard deviation of all measurements
		std::tuple<double,double> ms = Gem::Common::GStandardDeviation<double>(delaySummary);
		// Output the results
		parallelExecutionTimes.push_back(
			std::tuple<double,double,double,double>(
				gdi_ptr->getSleepTime().count()
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

	// For the serial measurement
	Go2 go_serial("./config/Go2.json");
	go_serial.setParallelizationMode(execMode::EXECMODE_SERIAL);

	// Add default optimization algorithms to the Go2 objects
	go_parallel.registerDefaultAlgorithm("ea");
	go_serial.registerDefaultAlgorithm("ea");

	// Threadpool for two threads
	Gem::Common::GThreadPool tp(2);

	// Start the reference and parallel threads
	tp.async_schedule([&](){ startReferenceMeasurement(go_serial, gdif_ref, ab); });
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
