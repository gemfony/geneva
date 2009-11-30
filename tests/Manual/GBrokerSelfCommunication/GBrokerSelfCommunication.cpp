/**
 * @file GBrokerPopulation.cpp
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
#include <iterator>
#include <cmath>
#include <sstream>
#include <string>
#include <vector>

// Boost header files go here
#include <boost/asio.hpp>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>

// GenEvA header files go here
#include "GRandom.hpp"
#include "GBasePopulation.hpp"
#include "GDoubleCollection.hpp"
#include "GParameterSet.hpp"
#include "GDoubleGaussAdaptor.hpp"
#include "GBrokerPopulation.hpp"
#include "GIndividualBroker.hpp"
#include "GAsioTCPConsumer.hpp"
#include "GAsioTCPClient.hpp"
#include "GEnums.hpp"
#include "GThreadGroup.hpp"

// The individual that should be optimized.
// Represents the projection of an m-dimensional
// data set to an n-dimensional data set.
#include "GProjectionIndividual.hpp"

// Parses the command line for all required options
#include "GCommandLineParser.hpp"

#include "GOptimizationMonitor.hpp"

using namespace Gem::GenEvA;
using namespace Gem::Util;

/************************************************************************************************/
/**
 * The main function. We search for the minimum of a parabola, with the help of multiple clients,
 * possibly running on different machines.
 */
int main(int argc, char **argv){
	std::string ip;
	std::size_t nData=10000, nDimOrig=5, nDimTarget=2;
	std::size_t nClients=4;
	double radius=1.;
	unsigned short port=10000;
	std::size_t populationSize=100, nParents=5;
	boost::uint16_t nProducerThreads=8;
	boost::uint32_t maxGenerations=2000, reportGeneration=1;
	long maxMinutes=10;
	bool verbose=true;
	recoScheme rScheme=VALUERECOMBINE;

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Command-line parsing
	if(!parseCommandLine(argc, argv,
						 nData,
				         nDimOrig,
				         nDimTarget,
				         radius,
				         nClients,
				         nProducerThreads,
				         populationSize,
				         nParents,
				         maxGenerations,
				         maxMinutes,
				         reportGeneration,
				         rScheme,
				         verbose))
		{ exit(1); }

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Creation of an input file for this example
	GProjectionIndividual::createSphereFile("sphere.xml",
											nData,
											nDimOrig,
											nDimTarget,
											radius);

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Set-up of local resources

	// Random numbers are our most valuable good. Set the number of threads
	GRANDOMFACTORY->setNProducerThreads(nProducerThreads);

	// We create a thread group of nClients threads + the server thread.
	Gem::Util::GThreadGroup gtg;

	// Global settings
	ip="localhost";
	port=10000;

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Start of server

	// Create a consumer and enrol it with the broker
	boost::shared_ptr<GAsioTCPConsumer> gatc(new GAsioTCPConsumer(port));
	// gatc->setSerializationMode(BINARYSERIALIZATION);
	GINDIVIDUALBROKER->enrol(gatc);

	// Set up a single projection individual
	boost::shared_ptr<GProjectionIndividual>
		projectionIndividual(new GProjectionIndividual("sphere.xml",-radius, radius));

	// Create the optimizationMonitor
	boost::shared_ptr<optimizationMonitor> om(new optimizationMonitor("optimization.xml"));

	// Create the actual population
	boost::shared_ptr<GBrokerPopulation> pop(new GBrokerPopulation());

	// Make the individual known to the population
	pop->push_back(projectionIndividual);

	// Register the monitor with the population. boost::bind knows how to handle a shared_ptr.
	pop->registerInfoFunction(boost::bind(&optimizationMonitor::informationFunction, om, _1, _2));

	// Specify some population settings
	pop->setPopulationSize(populationSize,nParents);
	pop->setMaxGeneration(maxGenerations);
	pop->setMaxTime(boost::posix_time::minutes(maxMinutes));
	pop->setReportGeneration(reportGeneration);
	pop->setRecombinationMethod(rScheme);

	// Start the actual optimization
	gtg.create_thread(boost::bind(&GBrokerPopulation::optimize, pop, 0));

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Start of clients
	for(std::size_t i=0; i<nClients; i++){
		boost::shared_ptr<GAsioTCPClient> p(new GAsioTCPClient(ip,boost::lexical_cast<std::string>(port)));
		gtg.create_thread(boost::bind(&GAsioTCPClient::run,p));
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Wait for all threads to finish
	gtg.join_all();

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Done ...
	std::cout << "Done ..." << std::endl;

	return 0;
}
