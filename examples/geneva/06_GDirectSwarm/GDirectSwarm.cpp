/**
 * @file GDirectSwarm.cpp
 */

/********************************************************************************
 *
 * This file is part of the Geneva library collection. The following license
 * applies to this file:
 *
 * ------------------------------------------------------------------------------
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * ------------------------------------------------------------------------------
 *
 * Note that other files in the Geneva library collection may use a different
 * license. Please see the licensing information in each file.
 *
 ********************************************************************************
 *
 * Geneva was started by Dr. Rüdiger Berlich and was later maintained together
 * with Dr. Ariel Garcia under the auspices of Gemfony scientific. For further
 * information on Gemfony scientific, see http://www.gemfomy.eu .
 *
 * The majority of files in Geneva was released under the Apache license v2.0
 * in February 2020.
 *
 * See the NOTICE file in the top-level directory of the Geneva library
 * collection for a list of contributors and copyright information.
 *
 ********************************************************************************/

/*
 * *****************************************************************************
 * NOTE: This file shows how to access some of the optimization algorithms
 * directly (EA in this case), without going through the Go2 class. Usually, Go2
 * is the recommended way and will relieve you from many burdensome tasks you
 * otherwise have to perform. Thus, if you are new to Geneva, we recommend that
 * you start with example 01 first rather than following what is shown in this file.
 * *****************************************************************************
 */

// Standard header files go here
#include <iostream>
#include <cmath>
#include <sstream>
#include <chrono>

// Boost header files go here
#include <boost/lexical_cast.hpp>

// Geneva header files go here
#include "courtier/GAsioConsumerT.hpp"
#include "courtier/GStdThreadConsumerT.hpp"
#include "courtier/GSerialConsumerT.hpp"
#include "courtier/GCourtierEnums.hpp"
#include "common/GParserBuilder.hpp"
#include "geneva/GenevaInitializer.hpp"
#include "geneva/G_OptimizationAlgorithm_SwarmAlgorithm.hpp"

// The individual that should be optimized
#include "geneva-individuals/GFunctionIndividual.hpp"

using namespace Gem::Geneva;
using namespace Gem::Courtier;
using namespace Gem::Hap;
using namespace Gem::Common;

namespace po = boost::program_options;

/******************************************************************************/

const consumerType DEFAULTCONSUMERTYPE=consumerType::MULTITHREADED;
const unsigned short DEFAULTPORT=10000;
const std::string DEFAULTIP="localhost";
const std::uint16_t DEFAULTNPRODUCERTHREADS=10;
const Gem::Common::serializationMode DEFAULTSERMODE=Gem::Common::serializationMode::TEXT;
const bool DEFAULTADDLOCALCONSUMER=false;
const std::uint16_t DEFAULTNEVALUATIONTHREADS=4;
const std::uint32_t DEFAULTMAXITERATIONS=200;
const std::uint32_t DEFAULTREPORTITERATION=1;
const long DEFAULTMAXMINUTES=10;
const std::size_t DEFAULTNNEIGHBORHOODSAP=5;
const std::size_t DEFAULTNNEIGHBORHOODMEMBERSAP=20;
const double DEFAULTCPERSONALAP=2.;
const double DEFAULTCNEIGHBORHOODAP=2.;
const double DEFAULTCGLOBALAP=1.;
const double DEFAULTCVELOCITYAP=0.4;
const bool DEFAULTALLRANDOMINIT=true;
const std::size_t DEFAULTMAXRECONNECTS = 10;

/******************************************************************************/
/**
 * Parses the command line
 */
bool parseCommandLine(
	int argc, char **argv
	, consumerType& cType
	, bool& clientMode
	, std::string& ip
	, unsigned short& port
	, Gem::Common::serializationMode& serMode
	, std::size_t& nNeighborhoods
	, std::size_t& nNeighborhoodMembers
	, double& cPersonal
	, double& cNeighborhood
	, double& cGlobal
	, double& cVelocity
	, updateRule& ur
	, bool& allRandomInit
	, std::uint16_t& nProducerThreads
	, std::uint16_t& nEvaluationThreads
	, std::uint32_t& maxIterations
	, long& maxMinutes
	, std::uint32_t& reportIteration
	, std::size_t& maxReconnects
){
	// Create the parser builder
	Gem::Common::GParserBuilder gpb;

	gpb.registerCLParameter<consumerType>(
		"consumerType"
		, cType
		, DEFAULTCONSUMERTYPE
		, "The type of consumer to use: 0 (serial), 1 (multithreaded) or 2 (networked)"
	);

	gpb.registerCLParameter<bool>(
		"client,c"
		, clientMode
		, false // Use server mode, if no client option is specified
		, "Whether to run networked execution in server or client mode."
		, GCL_IMPLICIT_ALLOWED // Permit implicit values, so that we can say --server instead of --server=true
		, true // Use client mode, of only -c or --client was specified
	);

	gpb.registerCLParameter<std::string>(
		std::string("ip")
		, ip
		, DEFAULTIP
		, "The ip of the server"
	);

	gpb.registerCLParameter<unsigned short>(
		"port"
			, port
			, DEFAULTPORT
			, "The port on the server"
	);

	gpb.registerCLParameter<Gem::Common::serializationMode>(
		"serializationMode"
		, serMode
		, DEFAULTSERMODE
		, "Specifies whether serialization shall be done in TEXTMODE (0), XMLMODE (1) or BINARYMODE (2)"
	);

	gpb.registerCLParameter<std::size_t>(
		"nNeighborhoods"
		, nNeighborhoods
		, DEFAULTNNEIGHBORHOODSAP
		, "The number of neighborhoods in the population"
	);

	gpb.registerCLParameter<std::size_t>(
		"nNeighborhoodMembers"
		, nNeighborhoodMembers
		, DEFAULTNNEIGHBORHOODMEMBERSAP
		, "The default number of members in each neighborhood"
	);

	gpb.registerCLParameter<double>(
		"cPersonal"
		, cPersonal
		, DEFAULTCPERSONALAP
		, "A constant to be multiplied with the personal direction vector"
	);

	gpb.registerCLParameter<double>(
		"cNeighborhood"
		, cNeighborhood
		, DEFAULTCNEIGHBORHOODAP
		, "A constant to be multiplied with the neighborhood direction vector"
	);

	gpb.registerCLParameter<double>(
		"cGlobal"
		, cGlobal
		, DEFAULTCGLOBALAP
		, "A constant to be multiplied with the global direction vector"
	);

	gpb.registerCLParameter<double>(
		"cVelocity"
		, cVelocity
		, DEFAULTCVELOCITYAP
		, "A constant to be multiplied with the old velocity vector"
	);

	gpb.registerCLParameter<updateRule>(
		"updateRule"
		, ur
		, DEFAULTUPDATERULE
		, "Use linear (0) or classical (1) update rule"
	);

	gpb.registerCLParameter<bool>(
		"allRandomInit"
		, allRandomInit
		, DEFAULTALLRANDOMINIT
		, "If set, all individuals will be initialized randomly. If 0, all individuals in one neighborhood will have the same start value"
		, GCL_IMPLICIT_ALLOWED // Permit implicit values, so that we can say --server instead of --server=true
		, true // Always randomly initialize if set
	);

	gpb.registerCLParameter<std::uint16_t>(
		"nProducerThreads"
		, nProducerThreads
		, DEFAULTNPRODUCERTHREADS
		, "The amount of random number producer threads"
	);

	gpb.registerCLParameter<std::uint16_t>(
		"nEvaluationThreads"
		, nEvaluationThreads
		, DEFAULTNEVALUATIONTHREADS
		, "The amount of threads processing individuals simultaneously in multi-threaded mode"
	);

	gpb.registerCLParameter<std::uint32_t>(
		"maxIterations"
		, maxIterations
		, DEFAULTMAXITERATIONS
		, "Maximum number of iterations in the optimization"
	);

	gpb.registerCLParameter<std::uint32_t>(
		"reportIteration"
		, reportIteration
		, DEFAULTREPORTITERATION
		, "The number of iterations after which information should be emitted in the population"
	);

	gpb.registerCLParameter<long>(
		"maxMinutes"
		, maxMinutes
		, DEFAULTMAXMINUTES
		, "The maximum number of minutes the optimization of the population should run"
	);

	gpb.registerCLParameter<std::size_t>(
		"maxReconnects"
		, maxReconnects
		, DEFAULTMAXRECONNECTS
		, "The number of times a client will try to reconnect when it couldn't reach the server"
	);

	// Parse the command line and leave if the help flag was given. The parser
	// will emit an appropriate help message by itself
	if(Gem::Common::GCL_HELP_REQUESTED == gpb.parseCommandLine(argc, argv, true /*verbose*/)) {
		return false; // Do not continue
	}

	return true;
}

/******************************************************************************/
/**
 * The main function.
 */
int main(int argc, char **argv){
	consumerType cType;
	bool clientMode;
	std::string ip;
	unsigned short port;
	std::uint16_t nProducerThreads;
	std::uint16_t nEvaluationThreads;
	std::uint32_t maxIterations;
	long maxMinutes;
	std::uint32_t reportIteration;
	Gem::Common::serializationMode serMode;
	std::size_t nNeighborhoods;
	std::size_t nNeighborhoodMembers;
	double cPersonal;
	double cNeighborhood;
	double cGlobal;
	double cVelocity;
	updateRule ur;
	bool allRandomInit;
	std::size_t maxReconnects;

	/****************************************************************************/
	// Initialization of Geneva
	GenevaInitializer gi;

	/****************************************************************************/
	// Retrieve all necessary configuration data from the command line

	if(!parseCommandLine(
		argc, argv
		, cType
		, clientMode
		, ip
		, port
		, serMode
		, nNeighborhoods
		, nNeighborhoodMembers
		, cPersonal
		, cNeighborhood
		, cGlobal
		, cVelocity
		, ur
		, allRandomInit
		, nProducerThreads
		, nEvaluationThreads
		, maxIterations
		, maxMinutes
		, reportIteration
		, maxReconnects
	)) { exit(1); }

	/****************************************************************************/
	// Random numbers are our most valuable good. Set the number of threads
	GRANDOMFACTORY->setNProducerThreads(nProducerThreads);

	/****************************************************************************/
	// If this is a client in networked mode, we can just start the listener and
	// return when it has finished
	if(clientMode && cType == consumerType::NETWORKED) {
		std::shared_ptr<GAsioConsumerClientT<GParameterSet>> p(
			new GAsioConsumerClientT<GParameterSet>(
				ip
				, port
				, serMode
				, maxReconnects
			)
		);

		// Start the actual processing loop
		p->run();

		return 0;
	}

	/****************************************************************************/
	// We can now start creating populations. We refer to them through the base class

	// This smart pointer will hold the different population types
	std::shared_ptr<GSwarmAlgorithm> pop_ptr(new GSwarmAlgorithm(nNeighborhoods, nNeighborhoodMembers));

	// Create the actual populations
	switch(cType) {
		//---------------------------------------------------------------------------
		case consumerType::SERIAL: // Serial execution
		{
			std::shared_ptr<GSerialConsumerT<GParameterSet>> sc(new GSerialConsumerT<GParameterSet>());
			GBROKER(Gem::Geneva::GParameterSet)->enrol_consumer(sc);
		}
			break;

			//---------------------------------------------------------------------------
		case consumerType::MULTITHREADED: // Multi-threaded execution
		{
			std::shared_ptr<GStdThreadConsumerT<GParameterSet>> gbtc(
				new GStdThreadConsumerT<GParameterSet>(nEvaluationThreads)
			);
			GBROKER(Gem::Geneva::GParameterSet)->enrol_consumer(gbtc);
		}
			break;

			//---------------------------------------------------------------------------
		case consumerType::NETWORKED: // Networked execution (server-side)
		{
			// Create a network consumer and enrol_buffer_port it with the broker
			std::shared_ptr<GAsioConsumerT<GParameterSet>> gatc_ptr(new GAsioConsumerT<GParameterSet>());

			// Set the required options
			gatc_ptr->setServerName(ip);
			gatc_ptr->setPort(port);
			gatc_ptr->setSerializationMode(serMode);
			gatc_ptr->setNThreads(nEvaluationThreads);
			gatc_ptr->setMaxReconnects(maxReconnects);

			// Add the consumer to the broker
			GBROKER(Gem::Geneva::GParameterSet)->enrol_consumer(gatc_ptr);
		}
			break;

			//----------------------------------------------------------------------------
	}

	/****************************************************************************/
	// Create a factory for GFunctionIndividual objects. It performs
	// all necessary initial work related to the individual (i.e. the optimization problem)
	GFunctionIndividualFactory gfi("./config/GFunctionIndividual.json");

	// Create the first set of parent individuals
	std::vector<std::shared_ptr<GFunctionIndividual>> parentIndividuals;

	// Create initial individuals for the population
	if(allRandomInit) { // Random initialization of all individuals in the population
		for(std::size_t p = 0 ; p<pop_ptr->getDefaultPopulationSize(); p++) {
			parentIndividuals.push_back(gfi.get_as<GFunctionIndividual>());
		}
	} else { // Individuals of the same neighborhood start from the same location
		for(std::size_t n=0; n<nNeighborhoods; n++) {
			// Initialize the first individual of the neighborhood
			std::shared_ptr<GFunctionIndividual> functionIndividual_ptr
				= gfi.get_as<GFunctionIndividual>();

			// Now add the required number of clones to the neighborhood
			for(std::size_t p=1; p<nNeighborhoodMembers; p++) {
				parentIndividuals.push_back(functionIndividual_ptr->clone<GFunctionIndividual>());
			}
			parentIndividuals.push_back(functionIndividual_ptr);
		}
	}

	/****************************************************************************/
	// Now we have suitable populations and can fill them with data

	// Add individuals to the population. Many Geneva classes, such as
	// the optimization classes, feature an interface very similar to std::vector.
	for(std::size_t p = 0 ; p<parentIndividuals.size(); p++) {
		pop_ptr->push_back(parentIndividuals[p]);
	}

	// Specify some general population settings
	pop_ptr->setMaxIteration(maxIterations);
	pop_ptr->setMaxTime(std::chrono::minutes(maxMinutes));
	pop_ptr->setReportIteration(reportIteration);
	pop_ptr->setCPersonal(cPersonal);
	pop_ptr->setCGlobal(cGlobal);
	pop_ptr->setCNeighborhood(cNeighborhood);
	pop_ptr->setCVelocity(cVelocity);
	pop_ptr->setUpdateRule(ur);

	// Do the actual optimization
	pop_ptr->optimize();

	/****************************************************************************/
	// Do something with the best individual found
	std::shared_ptr<GFunctionIndividual> p = pop_ptr->getBestGlobalIndividual<GFunctionIndividual>();

	// Here you can do something with the best individual ("p") found.
	// We simply print its content here, by means of an operator<< implemented
	// in the GFunctionIndividual code.
	std::cout
	<< "Best result found:" << std::endl
	<< p << std::endl;

	/****************************************************************************/
	// Terminate
	return 0;
}
