/**
 * @file GParallelisationOverhead.cpp
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt)
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) com
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
 * http://www.gemfony.com .
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
#include <geneva/Go.hpp>

// The individual that should be optimized
#include "GDelayIndividual.hpp"

using namespace Gem::Geneva;
using namespace Gem::Tests;

int main(int argc, char **argv) {
	Go go(argc, argv, "GParallelisationOverhead.cfg");

	//---------------------------------------------------------------------
	// Client mode
	if(go.clientMode()) {
		go.clientRun();
		return 0;
	}

	//---------------------------------------------------------------------
	// Server mode, serial or multi-threaded execution

	// Create a factory for GFunctionIndividual objects and perform
	// any necessary initial work.
	GDelayIndividualFactory gdif("./GDelayIndividual.cfg");
	gdif.init();

	//---------------------------------------------------------------------
	// Prepare the output files used to record the measurements
	std::ofstream result(gdif.getResultFileName().c_str());
	result
	<< "{" << std::endl
	<< "  gStyle->SetOptTitle(0);" << std::endl
	<< "  TCanvas *cc = new TCanvas(\"cc\",\"cc\",0,0,800,600);" << std::endl
	<< std::endl
	<< "  std::vector<double> sleepTime; // The amount of time each individual sleeps" << std::endl
	<< "  std::vector<double> totalProcessingTime; // The total processing time for a given optimization cycle" << std::endl
	<< std::endl;

	std::ofstream shortResult(gdif.getShortResultFileName().c_str());

	// Determine the amount of seconds the process should sleep in between two measurements
	boost::uint32_t interMeasurementDelay = gdif.getInterMeasurementDelay();

	// Loop until no valid individuals can be retrieved anymore
	std::size_t iter = 0;
	while(boost::shared_ptr<GDelayIndividual> gdi_ptr = gdif()) {
		// Make the individual known to the optimizer
		go.push_back(gdi_ptr);

		// Do the actual optimization and measure the time
		boost::posix_time::ptime startTime = boost::posix_time::microsec_clock::local_time();
		go.optimize<GDelayIndividual>();
		boost::posix_time::ptime endTime = boost::posix_time::microsec_clock::local_time();
		boost::posix_time::time_duration duration = endTime - startTime;

		// Output the results
		result
		<< std::endl
		<< "  // =========================================================" << std::endl
		<< "  // Sleep time = " << gdi_ptr->getSleepTime().total_milliseconds() << " milliseconds:" << std::endl
		<< "  sleepTime.push_back(" << gdi_ptr->getSleepTime().total_milliseconds()  << "/1000.);" << std::endl
		<< "  totalProcessingTime.push_back(" << double(duration.total_milliseconds()) << "/1000.);" << std::endl
		<< std::endl;

		shortResult
		<< gdi_ptr->getSleepTime().total_milliseconds() << "/" << duration.total_milliseconds() << std::endl;

		// Clean up the collection
		go.clear();

		// Wait for late arrivals
		sleep(interMeasurementDelay);

		// Increment the iteration counter
		iter++;
	}

	//---------------------------------------------------------------------
	// Tell the evaluation program to perform any necessary final work
	gdif.finalize();

	//---------------------------------------------------------------------
	// Output the footer of the result file
	result << std::endl
		   << "  // Transfer of vectors into arrays" << std::endl
		   << "  double sleepTimeArr[" << gdif.getNMeasurements() << "];" << std::endl
		   << "  double totalProcessingTimeArr[" << gdif.getNMeasurements() << "];" << std::endl
		   << std::endl
		   << "  for(int i=0; i< " << gdif.getNMeasurements() << "; i++) {" << std::endl
		   << "    sleepTimeArr[i] = sleepTime.at(i);" << std::endl
		   << "    totalProcessingTimeArr[i] = totalProcessingTime.at(i);" << std::endl
		   << "  }" << std::endl
	       << std::endl
	       << "  // Creation of TGraph objects and data transfer into the objects" << std::endl
	       << "  TGraph *evGraph = new TGraph(" << gdif.getNMeasurements() << ", sleepTimeArr, totalProcessingTimeArr);" << std::endl
	       << std::endl
	       << "  evGraph->SetMarkerStyle(2);" << std::endl
	       << "  evGraph->SetMarkerSize(1.0);" << std::endl
	       << "  evGraph->Draw(\"ACP\");" << std::endl
	       << "  evGraph->GetXaxis()->SetTitle(\"Evaluation time/individual [s]\");" << std::endl
	       << "  evGraph->GetYaxis()->SetTitle(\"Total processing time/ [s]\");" << std::endl
	       << "}" << std::endl;

	 // Close the result files
	result.close();
	shortResult.close();

	std::cout << "Done ..." << std::endl;
	return 0;
}
