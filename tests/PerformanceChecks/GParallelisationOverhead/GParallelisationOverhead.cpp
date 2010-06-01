/**
 * @file GParallelisationOverhead.cpp
 */

/* Copyright (C) Dr. Ruediger Berlich and Karlsruhe Institute of Technology
 * (University of the State of Baden-Wuerttemberg and National Laboratory
 * of the Helmholtz Association)
 *
 * Contact: info [at] gemfony (dot) com
 *
 * This file is part of the Geneva library, Gemfony scientific's optimization
 * library.
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
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/date_time.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

// GenEvA header files go here
#include "GRandom.hpp"
#include "GEvolutionaryAlgorithm.hpp"
#include "GMultiThreadedEA.hpp"
#include "GBrokerEA.hpp"
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
 * The main function. We try to measure the overhead incurred through the parallelization.
 */
int main(int argc, char **argv){
	std::string configFile;
	boost::uint16_t parallelizationMode;
	bool serverMode;
	std::string ip;
	unsigned short port;
	boost::uint16_t nProducerThreads;
	boost::uint16_t nEvaluationThreads;
	std::size_t populationSize;
	std::size_t nParents;
	boost::uint32_t maxGenerations;
	boost::uint32_t processingCycles;
	boost::uint32_t waitFactor;
	std::size_t nVariables;
	serializationMode serMode;
	boost::uint32_t maxStalls;
	boost::uint32_t maxConnAttempts;
	std::vector<long> sleepSeconds, sleepMilliSeconds;
	std::string resultFile;

	if(!parseCommandLine(argc, argv,
			configFile,
			parallelizationMode,
			serverMode,
			ip,
			port,
			serMode)
			||
		!parseConfigFile(configFile,
			nProducerThreads,
			nEvaluationThreads,
			populationSize,
			nParents,
			maxGenerations,
			processingCycles,
			waitFactor,
			maxStalls,
			maxConnAttempts,
			nVariables,
			resultFile,
			sleepSeconds,
			sleepMilliSeconds))
	{ exit(1); }

	// Random numbers are our most valuable good. Set the number of threads
	GRANDOMFACTORY->setNProducerThreads(nProducerThreads);

	// If this is a client in networked mode, we can just start the listener
	if(parallelizationMode==2 && !serverMode) {
		boost::shared_ptr<GAsioTCPClient> p(new GAsioTCPClient(ip, boost::lexical_cast<std::string>(port)));

		p->setMaxStalls(maxStalls); // Loop maxStalls when receiving no work items before terminating. 0 means "loop indefinitely"
		p->setMaxConnectionAttempts(maxConnAttempts); // Try up to maxConnAttempts times to connect to the server before terminating

		// Return results even if no better results were found
		p->returnResultIfUnsuccessful(true);

		// Start the client loop
		p->run();

		return 0;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Start the network consumer, if necessary
	if(parallelizationMode == 2) {
		// Create a network consumer and enrol it with the broker
		boost::shared_ptr<GAsioTCPConsumer> gatc(new GAsioTCPConsumer(port));
		gatc->setSerializationMode(serMode);
		GINDIVIDUALBROKER->enrol(gatc);
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Prepare the output file used to record the measurements
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
		// Calculate the current sleep time
		boost::posix_time::time_duration sleepTime =
				boost::posix_time::seconds(sleepSeconds.at(iter)) +
				boost::posix_time::milliseconds(sleepMilliSeconds.at(iter));

		std::cout << "Starting measurement with sleep time = " << sleepTime.total_milliseconds() << std::endl;

		///////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Create a population, depending on the parallelization mode. We refer to it through the base class.

		// This smart pointer will hold the different population types
		boost::shared_ptr<GEvolutionaryAlgorithm> pop_ptr;
		boost::shared_ptr<GBrokerEA> popBroker_ptr; // We need access to the GBrokerEA population at a later time

		// Create the actual populations
		switch (parallelizationMode) {
		//-----------------------------------------------------------------------------------------------------
		case 0: // Serial execution
			// Create an empty population
			pop_ptr = boost::shared_ptr<GEvolutionaryAlgorithm>(new GEvolutionaryAlgorithm());
			break;

			//-----------------------------------------------------------------------------------------------------
		case 1: // Multi-threaded execution
		{
			// Create the multi-threaded population
			boost::shared_ptr<GMultiThreadedEA> popPar_ptr(new GMultiThreadedEA());

			// Population-specific settings
			popPar_ptr->setNThreads(nEvaluationThreads);

			// Assignment to the base pointer
			pop_ptr = popPar_ptr;
		}
		break;

		//-----------------------------------------------------------------------------------------------------
		case 2: // Networked execution (server-side)
		{
			// Create the actual broker population
			popBroker_ptr = boost::shared_ptr<GBrokerEA>(new GBrokerEA());
			popBroker_ptr->setWaitFactor(waitFactor);
			popBroker_ptr->doLogging(); // Activate logging of arrival times of individuals

			// Assignment to the base pointer
			pop_ptr = popBroker_ptr;
		}
		break;
		}

		///////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Now we have a suitable population and can fill it with data

		// Create the first set of parent individuals.
		std::vector<boost::shared_ptr<GDelayIndividual> > parentIndividuals;
		for(std::size_t p = 0 ; p<nParents; p++) {
			boost::shared_ptr<GDelayIndividual> gdi_ptr(new GDelayIndividual(sleepTime));
			gdi_ptr->setProcessingCycles(processingCycles);

			// Set up a GBoundedDoubleCollection
			boost::shared_ptr<GBoundedDoubleCollection> gbdc_ptr(new GBoundedDoubleCollection());

			// Set up nVariables GBoundedDouble objects in the desired value range,
			// and register them with the double collection
			for(std::size_t var=0; var<nVariables; var++) {
				boost::shared_ptr<GBoundedDouble> gbd_ptr(new GBoundedDouble(0.,1.)); // range [0,1], random initialization

				boost::shared_ptr<GDoubleGaussAdaptor> gdga_ptr(new GDoubleGaussAdaptor(0.1, 0.5, 0., 1.));
				gdga_ptr->setAdaptionThreshold(1);
				gdga_ptr->setRnrGenerationMode(Gem::Util::RNRLOCAL);
				gbd_ptr->addAdaptor(gdga_ptr); // We use a common adaptor for all objects in the collection

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
		pop_ptr->setDefaultPopulationSize(populationSize,nParents);
		pop_ptr->setMaxIteration(maxGenerations);
		pop_ptr->setMaxTime(boost::posix_time::minutes(0));
		pop_ptr->setReportIteration(1); // Emit information during every generation
		pop_ptr->setRecombinationMethod(Gem::GenEvA::DEFAULTRECOMBINE);
		pop_ptr->setSortingScheme(Gem::GenEvA::MUCOMMANU);
		pop_ptr->setRnrGenerationMode(Gem::Util::RNRLOCAL);

		///////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Do the actual optimization and measure the time
		boost::posix_time::ptime startTime = boost::posix_time::microsec_clock::local_time();
		pop_ptr->optimize();
		boost::posix_time::ptime endTime = boost::posix_time::microsec_clock::local_time();
		boost::posix_time::time_duration duration = endTime - startTime;

		///////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Output the results
		result << std::endl
			   << "  // =========================================================" << std::endl
			   << "  // Iteration " << iter << " (" << sleepTime.total_milliseconds() << " milliseconds) :" << std::endl
			   << std::endl;

		// Log arrival times of individuals if this is networked mode
		if(parallelizationMode==2) {
			result << "  TH1F *arrivalTimes" << iter << " = new TH1F(\"arrivalTimes" << iter << "\", \"arrivalTimes" << iter << "\", 500, 0., 5.);" << std::endl
				   << "  TH1I *nReturned" << iter << " = new TH1I(\"nReturned" << iter << "\", \"nReturned" << iter << "\", " << maxGenerations+1 << ", 0, " << maxGenerations << ");" << std::endl
				   << std::endl;

			std::vector<std::vector<boost::uint32_t> > arrivalTimes = popBroker_ptr->getLoggingResults();

			for(std::size_t gen=0; gen<arrivalTimes.size(); gen++) {
				for(std::size_t ind=0; ind<arrivalTimes[gen].size(); ind++) {
					result << "  arrivalTimes" << iter << ".Fill(" << (double(arrivalTimes[gen][ind]) - sleepTime.total_milliseconds())/1000. << "); // ind = " << ind << ", gen = " << gen << std::endl;
				}
				result << "  nReturned" << iter << ".Fill(" << gen << ", " << arrivalTimes[gen].size() <<  ");" << std::endl
				       << std::endl;
			}
		}

		result << "  sleepTime.push_back(" << sleepTime.total_milliseconds() << "/1000.);" << std::endl
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
