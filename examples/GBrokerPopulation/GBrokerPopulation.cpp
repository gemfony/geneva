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

// Boost header files go here
#include <boost/asio.hpp>
#include <boost/function.hpp>

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

// The individual that should be optimized
// This is a simple parabola
#include "GParabolaIndividual.hpp"

// Parses the command line for all required options
#include "GCommandLineParser.hpp"

using namespace Gem::GenEvA;
using namespace Gem::Util;

/************************************************************************************************/
/**
 * The main function. We search for the minimum of a parabola, with the help of multiple clients,
 * possibly running on different machines.
 */
int main(int argc, char **argv){
	std::string mode, ip;
	unsigned short port=10000;
	std::size_t parabolaDimension, populationSize, nParents;
	double parabolaMin, parabolaMax;
	boost::uint16_t nProducerThreads;
	boost::uint32_t maxGenerations, reportGeneration;
	boost::uint32_t adaptionThreshold;
	long maxMinutes;
	bool verbose;
	recoScheme rScheme;
	Gem::GenEvA::sortingMode smode;

	// Retrieve command line options
	if(!parseCommandLine(argc, argv,
						 ip,
						 mode,
						 port,
				         parabolaDimension,
				         parabolaMin,
				         parabolaMax,
				         adaptionThreshold,
				         nProducerThreads,
				         populationSize,
				         nParents,
				         maxGenerations,
				         maxMinutes,
				         reportGeneration,
				         rScheme,
				         smode,
				         verbose))
		{ exit(1); }

	// Random numbers are our most valuable good. Set the number of threads
	GRANDOMFACTORY->setNProducerThreads(nProducerThreads);

	if(mode == "server"){
		// Create a consumer and enrol it with the broker
		boost::shared_ptr<GAsioTCPConsumer> gatc(new GAsioTCPConsumer(port));
		// gatc->setSerializationMode(BINARYSERIALIZATION);
		GINDIVIDUALBROKER->enrol(gatc);

		// Set up a single parabola individual
		boost::shared_ptr<GParabolaIndividual> parabolaIndividual(new GParabolaIndividual(parabolaDimension, parabolaMin,parabolaMax,adaptionThreshold));

		// Create the actual population
		GBrokerPopulation pop;

		// Make the individual known to the population
		pop.push_back(parabolaIndividual);

		// Specify some population settings
		pop.setPopulationSize(populationSize,nParents);
		pop.setMaxIteration(maxGenerations);
		pop.setMaxTime(boost::posix_time::minutes(maxMinutes));
		pop.setReportIteration(reportGeneration);
		pop.setRecombinationMethod(rScheme);
		pop.setSortingScheme(smode);

		// Do the actual optimization
		pop.optimize();
	}
	else if(mode == "client"){
		// Just start the client with the required parameters
	    GAsioTCPClient gasiotcpclient(ip,boost::lexical_cast<std::string>(port));
	    // gasiotcpclient.setSerializationMode(BINARYSERIALIZATION);
	    gasiotcpclient.run();
	}

	std::cout << "Done ..." << std::endl;

	return 0;
}
