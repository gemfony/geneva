/**
 * @file GBrokerSelfCommunication.cpp
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

// Geneva header files go here
#include "common/GCommonEnums.hpp"
#include "common/GThreadGroup.hpp"
#include "hap/GRandomT.hpp"
#include "courtier/GAsioTCPConsumerT.hpp"
#include "courtier/GAsioTCPClientT.hpp"
#include "geneva/GEvolutionaryAlgorithm.hpp"
#include "geneva/GDoubleCollection.hpp"
#include "geneva/GParameterSet.hpp"
#include "geneva/GDoubleGaussAdaptor.hpp"
#include "geneva/GBrokerEA.hpp"
#include "geneva/GIndividual.hpp"

// The individual that should be optimized.
// Represents the projection of an m-dimensional
// data set to an n-dimensional data set.
#include "GFunctionIndividual.hpp"

// Parses the command line for all required options
#include "GCommandLineParser.hpp"

using namespace Gem::Tests;
using namespace Gem::Geneva;
using namespace Gem::Courtier;
using namespace Gem::Hap;

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
	Gem::Common::serializationMode serMode;

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Command-line parsing
	if(!parseCommandLine(argc, argv,
				         nClients,
				         nProducerThreads,
				         populationSize,
				         nParents,
				         maxGenerations,
				         maxMinutes,
				         reportGeneration,
				         rScheme,
				         serMode,
				         verbose))
		{ exit(1); }

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Set-up of local resources

	// Random numbers are our most valuable good. Set the number of threads
	GRANDOMFACTORY->setNProducerThreads(nProducerThreads);

	// We create a thread group of nClients threads + the server thread.
	Gem::Common::GThreadGroup gtg;

	// Global settings
	ip="localhost";
	port=10000;

	std::size_t dimension=1000;
	double randMin = -10., randMax = 10.;

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Start of server

	// Create a consumer and enrol it with the broker
	boost::shared_ptr<GAsioTCPConsumerT<GIndividual> > gatc(new GAsioTCPConsumerT<GIndividual>(port));
	gatc->setSerializationMode(serMode);
	GINDIVIDUALBROKER->enrol(gatc);

	// Set up a single function individual
	boost::shared_ptr<GFunctionIndividual<PARABOLA> > functionIndividual(new GFunctionIndividual<PARABOLA>());

	// Set up a GDoubleCollection with dimension values, each initialized
	// with a random number in the range [min,max[
	boost::shared_ptr<GDoubleCollection> gdc_ptr(new GDoubleCollection(dimension,randMin,randMax));

	// Set up and register an adaptor for the collection, so it
	// knows how to be adapted.
	boost::shared_ptr<GDoubleGaussAdaptor> gdga_ptr(new GDoubleGaussAdaptor(2.,0.8,0.000001,2));
	gdga_ptr->setAdaptionThreshold(1);
	gdga_ptr->setAdaptionProbability(0.05);
	gdc_ptr->addAdaptor(gdga_ptr);

	// Make the parameter collection known to this individual
	functionIndividual->push_back(gdc_ptr);

	// Create the actual population
	boost::shared_ptr<GBrokerEA> pop(new GBrokerEA());

	// Make the individual known to the population
	pop->push_back(functionIndividual);

	// Specify some population settings
	pop->setDefaultPopulationSize(populationSize, nParents);
	pop->setMaxIteration(maxGenerations);
	pop->setMaxTime(boost::posix_time::minutes(maxMinutes));
	pop->setReportIteration(reportGeneration);
	pop->setRecombinationMethod(rScheme);

	// Start the actual optimization
	gtg.create_thread(boost::bind(&GBrokerEA::optimize, pop, 0));

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Start of clients
	for(std::size_t i=0; i<nClients; i++){
		boost::shared_ptr<GAsioTCPClientT<GIndividual> > p(new GAsioTCPClientT<GIndividual>(ip,boost::lexical_cast<std::string>(port)));
		gtg.create_thread(boost::bind(&GAsioTCPClientT<GIndividual>::run,p));
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Wait for all threads to finish
	gtg.join_all();

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Done ...
	std::cout << "Done ..." << std::endl;

	return 0;
}
