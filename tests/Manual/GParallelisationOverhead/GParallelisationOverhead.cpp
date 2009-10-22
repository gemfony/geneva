/**
 * @file GParallelisationOverhead.cpp
 */

/* Copyright (C) 2009 Dr. Ruediger Berlich
 *
 * This file is part of Geneva, Gemfony scientific's optimization library.
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
 */

// Standard header files go here
#include <iostream>
#include <cmath>
#include <string>
#include <fstream>
#include <sstream>


// Boost header files go here
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/date_time.hpp>
#include "boost/date_time/posix_time/posix_time.hpp"

// GenEvA header files go here
#include "GRandom.hpp"
#include "GBasePopulation.hpp"
#include "GBoostThreadPopulation.hpp"
#include "GBrokerPopulation.hpp"
#include "GIndividualBroker.hpp"
#include "GAsioTCPConsumer.hpp"
#include "GAsioTCPClient.hpp"
#include "GAsioHelperFunctions.hpp"
#include "GBoundedDoubleCollection.hpp"

// The individual that should be optimized
#include "GDelayIndividual.hpp"

// Declares a function to parse the command line
#include "GArgumentParser.hpp"

using namespace Gem::GenEvA;
using namespace Gem::Util;

/************************************************************************************************/
/**
 * The main function. We try to measure the overhead incurred through the parallelisation.
 */
int main(int argc, char **argv){
	bool firstConsumer = true; // Controls the start of consumers

	std::string configFile;
	boost::uint16_t parallelizationMode;
	bool serverMode;
	std::string ip;
	unsigned short port;
	boost::uint16_t nProducerThreads;
	boost::uint16_t nEvaluationThreads;
	std::size_t nBoostThreadConsumerThreads;
	std::size_t populationSize;
	std::size_t nParents;
	boost::uint32_t maxGenerations;
	boost::uint32_t startGeneration;
	boost::uint32_t processingCycles;
	boost::uint32_t waitFactor;
	std::size_t nVariables;

	if(!parseCommandLine(argc, argv,
			configFile,
			parallelizationMode,
			serverMode,
			ip,
			port,
			startGeneration)
			||
		!parseConfigFile(configFile,
			nProducerThreads,
			nEvaluationThreads,
			populationSize,
			nParents,
			maxGenerations,
			processingCycles,
			nBoostThreadConsumerThreads,
			waitFactor,
			nVariables))
	{ exit(1); }

	// Random numbers are our most valuable good. Set the number of threads
	GRANDOMFACTORY->setNProducerThreads(nProducerThreads);

	// If this is a client in networked mode, we can just start the listener
	if(parallelizationMode==2 && !serverMode) {
		boost::shared_ptr<GAsioTCPClient> p(new GAsioTCPClient(ip, boost::lexical_cast<std::string>(port)));

		p->setMaxStalls(0); // Loop indefinitely when receiving no work items
		p->setMaxConnectionAttempts(200); // Try up to 200 times to connect to the server before terminating

		// Return results even if no better results were found
		p->returnResultIfUnsuccessful(true);

		// Start the client loop
		p->run();

		return 0;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Set up the sleeping times
	std::vector<long> sleepSeconds, sleepMilliSeconds;

	// 0.01 s
	sleepSeconds.push_back(0);
	sleepMilliSeconds.push_back(10);
	// 0.1 s
	sleepSeconds.push_back(0);
	sleepMilliSeconds.push_back(100);
	// 0.5 s
	sleepSeconds.push_back(0);
	sleepMilliSeconds.push_back(500);
	// 1 s
	sleepSeconds.push_back(1);
	sleepMilliSeconds.push_back(0);
	// 2 s
	sleepSeconds.push_back(2);
	sleepMilliSeconds.push_back(0);
	// 3 s
	sleepSeconds.push_back(3);
	sleepMilliSeconds.push_back(0);
	// 4 s
	sleepSeconds.push_back(4);
	sleepMilliSeconds.push_back(0);
	// 5 s
	sleepSeconds.push_back(5);
	sleepMilliSeconds.push_back(0);
	// 6 s
	sleepSeconds.push_back(6);
	sleepMilliSeconds.push_back(0);
	// 7 s
	sleepSeconds.push_back(7);
	sleepMilliSeconds.push_back(0);
	// 8 s
	sleepSeconds.push_back(8);
	sleepMilliSeconds.push_back(0);
	// 9 s
	sleepSeconds.push_back(9);
	sleepMilliSeconds.push_back(0);
	// 10 s
	sleepSeconds.push_back(10);
	sleepMilliSeconds.push_back(0);
	// 15 s
	sleepSeconds.push_back(15);
	sleepMilliSeconds.push_back(0);
	// 20 s
	sleepSeconds.push_back(20);
	sleepMilliSeconds.push_back(0);
	// 25 s
	sleepSeconds.push_back(25);
	sleepMilliSeconds.push_back(0);
	// 30 s
	sleepSeconds.push_back(30);
	sleepMilliSeconds.push_back(0);
	// 40 s
	sleepSeconds.push_back(40);
	sleepMilliSeconds.push_back(0);
	// 50 s
	sleepSeconds.push_back(50);
	sleepMilliSeconds.push_back(0);
	// 60 s
	sleepSeconds.push_back(60);
	sleepMilliSeconds.push_back(0);
	// 80 s
	sleepSeconds.push_back(80);
	sleepMilliSeconds.push_back(0);
	// 100 s
	sleepSeconds.push_back(100);
	sleepMilliSeconds.push_back(0);
	// 120 s
	sleepSeconds.push_back(120);
	sleepMilliSeconds.push_back(0);
	// 240 s
	sleepSeconds.push_back(240);
	sleepMilliSeconds.push_back(0);

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Prepare the output file used to record the measurements
	std::string resultFile;
	switch (parallelizationMode) {
	case 0:
		resultFile = "resultSerial.C";
		break;
	case 1:
		resultFile = "resultThread.C";
		break;
	case 2:
		resultFile = "resultNetwork.C";
		break;
	}
	std::ofstream result(resultFile.c_str());

	result
	<< "{" << std::endl
	<< "  gStyle->SetOptTitle(0);" << std::endl
	<< "  TCanvas *cc = new TCanvas(\"cc\",\"cc\",0,0,800,600);" << std::endl
	<< std::endl
	<< "  std::vector<double> sleepTime; // The amount of time each individual sleeps" << std::endl
	<< "  std::vector<double> averageProcessingTime; // The average processing time per generation" << std::endl
	<< std::endl;

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// We need to measure consecutively for a number of times specified by the sleepSeconds vector.
	std::size_t nMeasurements = sleepSeconds.size();

	for(std::size_t iter=0; iter<nMeasurements; iter++) {
		///////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Create a population, depending on the parallelization mode. We refer to it through the base class.

		// This smart pointer will hold the different population types
		boost::shared_ptr<GBasePopulation> pop_ptr;

		// Create the actual populations
		switch (parallelizationMode) {
		//-----------------------------------------------------------------------------------------------------
		case 0: // Serial execution
			// Create an empty population
			pop_ptr = boost::shared_ptr<GBasePopulation>(new GBasePopulation());
			break;

			//-----------------------------------------------------------------------------------------------------
		case 1: // Multi-threaded execution
		{
			// Create the multi-threaded population
			boost::shared_ptr<GBoostThreadPopulation> popPar_ptr(new GBoostThreadPopulation());

			// Population-specific settings
			popPar_ptr->setNThreads(nEvaluationThreads);

			// Assignment to the base pointer
			pop_ptr = popPar_ptr;
		}
		break;

		//-----------------------------------------------------------------------------------------------------
		case 2: // Networked execution (server-side)
		{
			if(firstConsumer) {
				// Create a network consumer and enrol it with the broker
				boost::shared_ptr<GAsioTCPConsumer> gatc(new GAsioTCPConsumer(port));
				GINDIVIDUALBROKER->enrol(gatc);

				firstConsumer = false;
			}

			// Create the actual broker population
			boost::shared_ptr<GBrokerPopulation> popBroker_ptr(new GBrokerPopulation());
			popBroker_ptr->setWaitFactor(waitFactor);

			// Assignment to the base pointer
			pop_ptr = popBroker_ptr;
		}
		break;
		}

		///////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Now we have a suitable population and can fill it with data

		boost::posix_time::time_duration sleepTime =
				boost::posix_time::seconds(sleepSeconds.at(iter)) +
				boost::posix_time::milliseconds(sleepMilliSeconds.at(iter));

		// Create the first set of parent individuals.
		std::vector<boost::shared_ptr<GDelayIndividual> > parentIndividuals;
		for(std::size_t p = 0 ; p<nParents; p++) {
			boost::shared_ptr<GDelayIndividual> gdi_ptr(new GDelayIndividual(sleepTime));
			gdi_ptr->setProcessingCycles(processingCycles);

			// Set up a GBoundedDoubleCollection, including adaptor
			boost::shared_ptr<GBoundedDoubleCollection> gbdc_ptr(new GBoundedDoubleCollection());
			boost::shared_ptr<GDoubleGaussAdaptor> gdga_ptr(new GDoubleGaussAdaptor(0.1, 0.5, 0., 1.));
			gdga_ptr->setAdaptionThreshold(1);
			gdga_ptr->setRnrGenerationMode(Gem::Util::RNRLOCAL);
			gbdc_ptr->addAdaptor(gdga_ptr); // We use a common adaptor for all objects in the collection

			// Set up nVariables GBoundedDouble objects in the desired value range,
			// and register them with the double collection
			for(std::size_t var=0; var<nVariables; var++) {
				boost::shared_ptr<GBoundedDouble> gbd_ptr(new GBoundedDouble(0.,1.)); // range [0,1], random initialization

				// Make the GBoundedDouble known to the collection
				gbdc_ptr->push_back(gbd_ptr);
			}

			// Make the GBoundedDoubleCollection known to the individual
			gdi_ptr->push_back(gbdc_ptr);

			parentIndividuals.push_back(gdi_ptr);
		}

		///////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Add individuals to the population
		for(std::size_t p = 0 ; p<nParents; p++) {
			pop_ptr->push_back(parentIndividuals[p]);
		}

		///////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Specify some general population settings
		pop_ptr->setPopulationSize(populationSize,nParents);
		pop_ptr->setMaxGeneration(maxGenerations);
		pop_ptr->setMaxTime(boost::posix_time::minutes(0));
		pop_ptr->setReportGeneration(1); // Emit information during every generation
		pop_ptr->setRecombinationMethod(Gem::GenEvA::DEFAULTRECOMBINE);
		pop_ptr->setSortingScheme(Gem::GenEvA::MUCOMMANU);
		pop_ptr->setRnrGenerationMode(Gem::Util::RNRLOCAL);

		///////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Do the actual optimization and measure the time
		boost::posix_time::ptime startTime = boost::posix_time::microsec_clock::local_time();
		pop_ptr->optimize(startGeneration);
		boost::posix_time::ptime endTime = boost::posix_time::microsec_clock::local_time();

		boost::posix_time::time_duration duration = endTime - startTime;

		// Output the results
		result << "  // Iteration " << iter << ":" << std::endl
			   << "  sleepTime.push_back(" << sleepTime.total_milliseconds() << "/1000.);" << std::endl
			   << "  averageProcessingTime.push_back(" << double(duration.total_milliseconds())/double(maxGenerations+1) <<"/1000.);" << std::endl;

		//--------------------------------------------------------------------------------------------
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Clean up

	// Output the footer of the result file
	result << std::endl
		   << "  // Transfer of vectors into arrays" << std::endl
		   << "  double sleepTimeArr[" << nMeasurements << "];" << std::endl
		   << "  double averageProcessingTimeArr[" << nMeasurements << "];" << std::endl
		   << std::endl
		   << "  for(int i=0; i< " << nMeasurements << "; i++) {" << std::endl
		   << "    sleepTimeArr[i] = sleepTime.at(i);" << std::endl
		   << "    averageProcessingTimeArr[i] = averageProcessingTime.at(i);" << std::endl
		   << "  }" << std::endl
	       << std::endl
	       << "  // Creation of TGraph objects and data transfer into the objects" << std::endl
	       << "  TGraph *evGraph = new TGraph(" << nMeasurements << ", sleepTimeArr, averageProcessingTimeArr);" << std::endl
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
