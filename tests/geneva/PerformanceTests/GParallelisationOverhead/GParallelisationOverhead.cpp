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
#include <unistd.h>

// Boost header files go here

// Geneva header files go here
#include <common/GMathHelperFunctionsT.hpp>
#include <geneva/Go2.hpp>

// The individual that should be optimized
#include "GDelayIndividual.hpp"

using namespace Gem::Geneva;
using namespace Gem::Common;
using namespace Gem::Tests;

int main(int argc, char **argv) {
   Go2 go(argc, argv, "./config/Go2.json");

   //---------------------------------------------------------------------
   // Will hold all plot information
   boost::shared_ptr<GGraph2ED> gdelay_ptr(new GGraph2ED());

   //---------------------------------------------------------------------
   // Client mode
   if(go.clientMode()) {
      return go.clientRun();
   }

	//---------------------------------------------------------------------
	// Server mode, serial or multi-threaded execution

	// Create a factory for GFunctionIndividual objects and perform
	// any necessary initial work.
	GDelayIndividualFactory gdif("./config/GDelayIndividual.json");

	//---------------------------------------------------------------------
	// Loop until no valid individuals can be retrieved anymore
	std::ofstream shortResult;
	boost::uint32_t interMeasurementDelay = 1;
	boost::uint32_t nMeasurements = 5;
	std::size_t iter = 0;
	boost::shared_ptr<GDelayIndividual> gdi_ptr;
	while((gdi_ptr = gdif.get<GDelayIndividual>())) {
	   if(0==iter) { // The first individual must already have been produced in order to access parsed data
	      // Prepare the output files used to record the measurements
	      shortResult.open(gdif.getShortResultFileName().c_str());

	      // Determine the amount of seconds the process should sleep in between two measurements
	      interMeasurementDelay = gdif.getInterMeasurementDelay();

	      // Determine the number of measurements to be made for each delay
	      nMeasurements = gdif.getNMeasurements();
	   }

		std::vector<double> delaySummary;
		std::cout << "Starting " << nMeasurements << " measurements" << std::endl;
		for(boost::uint32_t i=0; i<nMeasurements; i++) {
			// Make the individual known to the optimizer
			go.push_back(gdi_ptr);

			// Do the actual optimization and measure the time
			boost::posix_time::ptime startTime = boost::posix_time::microsec_clock::local_time();
			go.optimize<GDelayIndividual>();
			boost::posix_time::ptime endTime = boost::posix_time::microsec_clock::local_time();
			boost::posix_time::time_duration duration = endTime - startTime;

			delaySummary.push_back(double(duration.total_milliseconds())/1000.);

			// Clean up the collection
			go.clear();
		}

      // Calculate the mean value and standard deviation of all measurements
		boost::tuple<double,double> ms = Gem::Common::GStandardDeviation<double>(delaySummary);
		// Output the results
		gdelay_ptr->add(
		      boost::tuple<double,double,double,double>(
		            double(gdi_ptr->getSleepTime().total_milliseconds())/1000.
		            , 0. // No error on the sleep time
		            , boost::get<0>(ms) // mean
		            , boost::get<1>(ms) // standard deviation
            )
      );

		shortResult
		<< double(gdi_ptr->getSleepTime().total_milliseconds())/1000. << " / " << boost::get<0>(ms) << " / " << boost::get<1>(ms) << std::endl;

		// Clean up the delay vector
		delaySummary.clear();

		// Wait for late arrivals
		sleep(interMeasurementDelay);

		// Increment the iteration counter
		iter++;
	}

	//---------------------------------------------------------------------
	// Finalize the output
	GPlotDesigner gpd("Average total processing time as a function of individual evaluation time", 1,1);
   gpd.setCanvasDimensions(1200,800);
   gpd.registerPlotter(gdelay_ptr);
   gpd.writeToFile(gdif.getResultFileName());

	shortResult.close();

	return 0;
}
