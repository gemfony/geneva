/**
 * @file GBoundedParabola.cpp
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
#include <sstream>

// Boost header files go here

// GenEvA header files go here
#include "GRandom.hpp"
#include "GEvolutionaryAlgorithm.hpp"
#include "GMultiThreadedEA.hpp"
#include "GBrokerEA.hpp"
#include "GIndividualBroker.hpp"
#include "GAsioTCPConsumer.hpp"
#include "GAsioTCPClient.hpp"

// The individual that should be optimized
// This is a simple parabola
#include "GBoundedParabolaIndividual.hpp"

// Declares a function to parse the command line
#include "GCommandLineParser.hpp"

using namespace Gem::GenEvA;
using namespace Gem::Util;

/************************************************************************************************/
/**
 * The main function. We search for the minimum of a parabola. This example demonstrates the use
 * of the GEvolutionaryAlgorithm class or (at your choice) the GMultiThreadedEA or GBrokerPopilation
 * class. Note that a number of command line options are available. Call the executable with the "-h"
 * switch to get an overview.
 */

int main(int argc, char **argv){
	// Variables for the command line parsing
	std::size_t parabolaDimension;
	std::size_t populationSize, nParents;
	double parabolaMin, parabolaMax;
	boost::uint16_t nProducerThreads;
	boost::uint16_t nEvaluationThreads;
	boost::uint32_t maxGenerations, reportGeneration;
	boost::uint32_t adaptionThreshold;
	long maxMinutes;
	bool verbose;
	recoScheme rScheme;
	std::size_t arraySize;
	bool productionPlace;
	bool useCommonAdaptor;
	double sigma;
	double sigmaSigma;
	double minSigma;
	double maxSigma;
	boost::uint16_t parallelizationMode;
	bool serverMode;
	std::string ip;
	unsigned short port;

	// Parse the command line
	if(!parseCommandLine(argc, argv,
		  			     parabolaDimension,
						 parabolaMin,
						 parabolaMax,
						 adaptionThreshold,
						 nProducerThreads,
						 nEvaluationThreads,
						 populationSize,
						 nParents,
						 maxGenerations,
						 maxMinutes,
						 reportGeneration,
						 rScheme,
						 parallelizationMode,
						 serverMode,
						 ip,
						 port,
						 arraySize,
						 productionPlace,
						 useCommonAdaptor,
						 sigma,
						 sigmaSigma,
						 minSigma,
						 maxSigma,
						 verbose))
	{ exit(1); }

	// Random numbers are our most valuable good. Set the number of threads
	GRANDOMFACTORY->setNProducerThreads(nProducerThreads);
	GRANDOMFACTORY->setArraySize(arraySize);

	switch (parallelizationMode) {
	//-----------------------------------------------------------------------------------------------------
	case 0: // Serial execution
		{
			// Set up a single parabola individual
			boost::shared_ptr<GBoundedParabolaIndividual> parabolaIndividual(new GBoundedParabolaIndividual());

			// Set up a GBoundedDoubleCollection
			boost::shared_ptr<GBoundedDoubleCollection> gbdc_ptr(new GBoundedDoubleCollection());

			// Set up an adaptor.
			boost::shared_ptr<GDoubleGaussAdaptor> gdga_ptr(new GDoubleGaussAdaptor(sigma, sigmaSigma, minSigma, maxSigma));
			gdga_ptr->setAdaptionThreshold(adaptionThreshold);

			// Check whether random numbers should be produced locally or in the factory
			if(productionPlace) // Factory means "true"
				gdga_ptr->setRnrGenerationMode(Gem::Util::RNRFACTORY);
			else
				gdga_ptr->setRnrGenerationMode(Gem::Util::RNRLOCAL);

			if(useCommonAdaptor) {
				// Adding the adaptor to the collection instead of individual GBoundedDouble objects will
				// result in a joint adaptor for all GBoundedDoubles
				gbdc_ptr->addAdaptor(gdga_ptr);

				// Set up parabolaDimension GBoundedDouble objects in the desired value range,
				// and register them with the double collection
				for(std::size_t i=0; i<parabolaDimension; i++) {
					// GBoundedDouble will start with value "max"
					boost::shared_ptr<GBoundedDouble> gbd_ptr(new GBoundedDouble(parabolaMax, parabolaMin, parabolaMax));

					// Make the GBoundedDouble known to the collection
					gbdc_ptr->push_back(gbd_ptr);
				}
			}
			else {
				// Set up parabolaDimension GBoundedDouble objects in the desired value range,
				// and register them with the double collection
				for(std::size_t i=0; i<parabolaDimension; i++) {
					// GBoundedDouble will start with value "max"
					boost::shared_ptr<GBoundedDouble> gbd_ptr(new GBoundedDouble(parabolaMax, parabolaMin, parabolaMax));

					// Registering the adaptor with each GBoundedDouble results in individual adaptors for each object
					gbd_ptr->addAdaptor(gdga_ptr);

					// Make the GBoundedDouble known to the collection
					gbdc_ptr->push_back(gbd_ptr);
				}
			}

			// Make the GBoundedDouble collection known to the individual
			parabolaIndividual->push_back(gbdc_ptr);

			// Now we've got our first individual and can create a simple population with serial execution.
			GEvolutionaryAlgorithm pop_ser;

			pop_ser.push_back(parabolaIndividual);

			// Specify some population settings
			pop_ser.setPopulationSize(populationSize,nParents);
			pop_ser.setMaxIteration(maxGenerations);
			pop_ser.setMaxTime(boost::posix_time::minutes(maxMinutes)); // Calculation should be finished after 5 minutes
			pop_ser.setReportIteration(reportGeneration); // Emit information during every generation
			pop_ser.setRecombinationMethod(rScheme); // The best parents have higher chances of survival

			// Check whether random numbers should be produced locally or in the factory
			if(productionPlace) // Factory means "true"
			  pop_ser.setRnrGenerationMode(Gem::Util::RNRFACTORY);
			else
			  pop_ser.setRnrGenerationMode(Gem::Util::RNRLOCAL);

			// Do the actual optimization
			pop_ser.optimize();
		}
		break;

		//-----------------------------------------------------------------------------------------------------
	case 1: // Multi-threaded execution
		{
			// Set up a single parabola individual
			boost::shared_ptr<GBoundedParabolaIndividual> parabolaIndividual(new GBoundedParabolaIndividual());

			// Set up a GBoundedDoubleCollection
			boost::shared_ptr<GBoundedDoubleCollection> gbdc_ptr(new GBoundedDoubleCollection());

			// Set up an adaptor.
			boost::shared_ptr<GDoubleGaussAdaptor> gdga_ptr(new GDoubleGaussAdaptor(sigma, sigmaSigma, minSigma, maxSigma));
			gdga_ptr->setAdaptionThreshold(adaptionThreshold);

			// Check whether random numbers should be produced locally or in the factory
			if(productionPlace) // Factory means "true"
				gdga_ptr->setRnrGenerationMode(Gem::Util::RNRFACTORY);
			else
				gdga_ptr->setRnrGenerationMode(Gem::Util::RNRLOCAL);

			if(useCommonAdaptor) {
				// Adding the adaptor to the collection instead of individual GBoundedDouble objects will
				// result in a joint adaptor for all GBoundedDoubles
				gbdc_ptr->addAdaptor(gdga_ptr);

				// Set up parabolaDimension GBoundedDouble objects in the desired value range,
				// and register them with the double collection
				for(std::size_t i=0; i<parabolaDimension; i++) {
					// GBoundedDouble will start with value "max"
					boost::shared_ptr<GBoundedDouble> gbd_ptr(new GBoundedDouble(parabolaMax, parabolaMin, parabolaMax));

					// Make the GBoundedDouble known to the collection
					gbdc_ptr->push_back(gbd_ptr);
				}
			}
			else {
				// Set up parabolaDimension GBoundedDouble objects in the desired value range,
				// and register them with the double collection
				for(std::size_t i=0; i<parabolaDimension; i++) {
					// GBoundedDouble will start with value "max"
					boost::shared_ptr<GBoundedDouble> gbd_ptr(new GBoundedDouble(parabolaMax, parabolaMin, parabolaMax));

					// Registering the adaptor with each GBoundedDouble results in individual adaptors for each object
					gbd_ptr->addAdaptor(gdga_ptr);

					// Make the GBoundedDouble known to the collection
					gbdc_ptr->push_back(gbd_ptr);
				}
			}

			// Make the GBoundedDouble collection known to the individual
			parabolaIndividual->push_back(gbdc_ptr);

			// Now we've got our first individual and can create a simple population with parallel execution.
			GMultiThreadedEA pop_par;
			pop_par.setNThreads(nEvaluationThreads);

			pop_par.push_back(parabolaIndividual);

			// Specify some population settings
			pop_par.setPopulationSize(populationSize,nParents);
			pop_par.setMaxIteration(maxGenerations);
			pop_par.setMaxTime(boost::posix_time::minutes(maxMinutes)); // Calculation should be finished after 5 minutes
			pop_par.setReportIteration(reportGeneration); // Emit information during every generation
			pop_par.setRecombinationMethod(rScheme); // The best parents have higher chances of survival

			// Check whether random numbers should be produced locally or in the factory
			if(productionPlace) // Factory means "true"
			  pop_par.setRnrGenerationMode(Gem::Util::RNRFACTORY);
			else
			  pop_par.setRnrGenerationMode(Gem::Util::RNRLOCAL);

			// Do the actual optimization
			pop_par.optimize();
		}
		break;

		//-----------------------------------------------------------------------------------------------------
	case 2: // Networked execution
		{
			// Check if we are running as a (networked) client or server.
			// The client should be executed before the setup of individuals,
			// as it sets the seed of the random number factory, using a
			// seed supplied by the server
			if (serverMode) {
				// Set up a single parabola individual
				boost::shared_ptr<GBoundedParabolaIndividual> parabolaIndividual(new GBoundedParabolaIndividual());

				// Set up a GBoundedDoubleCollection
				boost::shared_ptr<GBoundedDoubleCollection> gbdc_ptr(new GBoundedDoubleCollection());

				// Set up an adaptor.
				boost::shared_ptr<GDoubleGaussAdaptor> gdga_ptr(new GDoubleGaussAdaptor(sigma, sigmaSigma, minSigma, maxSigma));
				gdga_ptr->setAdaptionThreshold(adaptionThreshold);

				// Check whether random numbers should be produced locally or in the factory
				if(productionPlace) // Factory means "true"
					gdga_ptr->setRnrGenerationMode(Gem::Util::RNRFACTORY);
				else
					gdga_ptr->setRnrGenerationMode(Gem::Util::RNRLOCAL);

				if(useCommonAdaptor) {
					// Adding the adaptor to the collection instead of individual GBoundedDouble objects will
					// result in a joint adaptor for all GBoundedDoubles
					gbdc_ptr->addAdaptor(gdga_ptr);

					// Set up parabolaDimension GBoundedDouble objects in the desired value range,
					// and register them with the double collection
					for(std::size_t i=0; i<parabolaDimension; i++) {
						// GBoundedDouble will start with value "max"
						boost::shared_ptr<GBoundedDouble> gbd_ptr(new GBoundedDouble(parabolaMax, parabolaMin, parabolaMax));

						// Make the GBoundedDouble known to the collection
						gbdc_ptr->push_back(gbd_ptr);
					}
				}
				else {
					// Set up parabolaDimension GBoundedDouble objects in the desired value range,
					// and register them with the double collection
					for(std::size_t i=0; i<parabolaDimension; i++) {
						// GBoundedDouble will start with value "max"
						boost::shared_ptr<GBoundedDouble> gbd_ptr(new GBoundedDouble(parabolaMax, parabolaMin, parabolaMax));

						// Registering the adaptor with each GBoundedDouble results in individual adaptors for each object
						gbd_ptr->addAdaptor(gdga_ptr);

						// Make the GBoundedDouble known to the collection
						gbdc_ptr->push_back(gbd_ptr);
					}
				}

				// Make the GBoundedDouble collection known to the individual
				parabolaIndividual->push_back(gbdc_ptr);

				// Create a consumer and enrol it with the broker
				boost::shared_ptr<GAsioTCPConsumer> gatc(new GAsioTCPConsumer(port));
				// gatc->setSerializationMode(BINARYSERIALIZATION);
				GINDIVIDUALBROKER->enrol(gatc);

				// Create the actual population
				GBrokerEA pop_broker;

				// Make the individual known to the population
				pop_broker.push_back(parabolaIndividual);

				// Specify some population settings
				pop_broker.setPopulationSize(populationSize,nParents);
				pop_broker.setMaxIteration(maxGenerations);
				pop_broker.setMaxTime(boost::posix_time::minutes(maxMinutes));
				pop_broker.setReportIteration(reportGeneration);
				pop_broker.setRecombinationMethod(rScheme);

				// Check whether random numbers should be produced locally or in the factory
				if(productionPlace) // Factory means "true"
					pop_broker.setRnrGenerationMode(Gem::Util::RNRFACTORY);
				else
					pop_broker.setRnrGenerationMode(Gem::Util::RNRLOCAL);


				// Do the actual optimization
				pop_broker.optimize();
			}
			else { // Client
				// Just start the client with the required parameters
				GAsioTCPClient gasiotcpclient(ip,boost::lexical_cast<std::string>(port));
				gasiotcpclient.run();
				return 0;
			}
		}
		break;

		//-----------------------------------------------------------------------------------------------------
	};

	std::cout << "Done ..." << std::endl;
	return 0;
}
