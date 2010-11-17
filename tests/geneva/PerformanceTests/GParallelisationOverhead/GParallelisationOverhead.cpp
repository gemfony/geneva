/**
 * @file GParallelisationOverhead.cpp
 */

/*
 * Copyright (C) Authors of the Geneva library collection and Karlsruhe
 * Institute of Technology (University of the State of Baden-Wuerttemberg
 * and National Laboratory of the Helmholtz Association).
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: info [at] gemfony (dot) com
 *
 * This file is part of the Geneva library collection
 *
 * Geneva is free software: you can redistribute it and/or modify
 * it under the terms of version 3 of the GNU Affero General Public License
 * as published by the Free Software Foundation.
 *
 * Geneva is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with the Geneva library.  If not, see <http://www.gnu.org/licenses/>.
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

// Boost header files go here

// Geneva header files go here
#include <geneva/Go.hpp>

// The individual that should be optimized
#include "geneva-individuals/GDelayIndividual.hpp"

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
	// Prepare the output file used to record the measurements
	std::ofstream result(gdif.getResultFileName().c_str());
	result
	<< "{" << std::endl
	<< "  gStyle->SetOptTitle(0);" << std::endl
	<< "  TCanvas *cc = new TCanvas(\"cc\",\"cc\",0,0,800,600);" << std::endl
	<< std::endl
	<< "  std::vector<double> sleepTime; // The amount of time each individual sleeps" << std::endl
	<< "  std::vector<double> averageProcessingTime; // The average processing time per generation" << std::endl
	<< std::endl;

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
		<< "  // Sleep time = " << gdi_ptr->getSleepTime().total_milliseconds() << " milliseconds) :" << std::endl
		<< "  sleepTime.push_back(" << gdi_ptr->getSleepTime().total_milliseconds()  << "/1000.);" << std::endl
		<< "  averageProcessingTime.push_back(" << double(duration.total_milliseconds())/double(go.getMaxIterations()+1) <<"/1000.);" << std::endl
		<< std::endl;

		// Clean up the collection
		go.clear();

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
		   << "  double averageProcessingTimeArr[" << gdif.getNMeasurements() << "];" << std::endl
		   << std::endl
		   << "  for(int i=0; i< " << gdif.getNMeasurements() << "; i++) {" << std::endl
		   << "    sleepTimeArr[i] = sleepTime.at(i);" << std::endl
		   << "    averageProcessingTimeArr[i] = averageProcessingTime.at(i);" << std::endl
		   << "  }" << std::endl
	       << std::endl
	       << "  // Creation of TGraph objects and data transfer into the objects" << std::endl
	       << "  TGraph *evGraph = new TGraph(" << gdif.getNMeasurements() << ", sleepTimeArr, averageProcessingTimeArr);" << std::endl
	       << std::endl
	       << "  evGraph->SetMarkerStyle(2);" << std::endl
	       << "  evGraph->SetMarkerSize(1.0);" << std::endl
	       << "  evGraph->Draw(\"ACP\");" << std::endl
	       << "  evGraph->GetXaxis()->SetTitle(\"Evaluation time/individual [s]\");" << std::endl
	       << "  evGraph->GetYaxis()->SetTitle(\"Average processing time/generation [s]\");" << std::endl
	       << "}" << std::endl;

	 // Close the result file
	result.close();

	std::cout << "Done ..." << std::endl;
	return 0;
}
