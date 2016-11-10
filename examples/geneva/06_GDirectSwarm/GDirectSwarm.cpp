/**
 * @file GDirectSwarm.cpp
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
#include "courtier/GAsioTCPConsumerT.hpp"
#include "courtier/GBoostThreadConsumerT.hpp"
#include "courtier/GSerialConsumerT.hpp"
#include "common/GParserBuilder.hpp"
#include "geneva/GenevaInitializer.hpp"
#include "geneva/GSwarmAlgorithmFactory.hpp"

// The individual that should be optimized
#include "geneva-individuals/GFunctionIndividual.hpp"

using namespace Gem::Geneva;
using namespace Gem::Courtier;
using namespace Gem::Hap;
using namespace Gem::Common;

namespace po = boost::program_options;

/******************************************************************************/

const execMode DEFAULTPARALLELIZATIONMODEAP=execMode::EXECMODE_MULTITHREADED;
const unsigned short DEFAULTPORT=10000;
const std::string DEFAULTIP="localhost";
const std::uint32_t DEFAULTMAXSTALLS06=0;
const std::uint32_t DEFAULTMAXCONNECTIONATTEMPTS06=100;
const std::uint16_t DEFAULTNPRODUCERTHREADS=10;
const Gem::Common::serializationMode DEFAULTSERMODE=Gem::Common::serializationMode::SERIALIZATIONMODE_TEXT;
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

/******************************************************************************/
/**
 * Parses the command line
 */
bool parseCommandLine(
	int argc, char **argv
	, execMode& parallelizationMode
	, bool& serverMode
	, std::string& ip
	, unsigned short& port
	, std::uint32_t& maxStalls
	, std::uint32_t& maxConnectionAttempts
	, Gem::Common::serializationMode& serMode
	, bool& addLocalConsumer
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
){
	// Create the parser builder
	Gem::Common::GParserBuilder gpb;

	gpb.registerCLParameter<execMode>(
		"parallelizationMode,p"
		, parallelizationMode
		, DEFAULTPARALLELIZATIONMODEAP
		, "Whether to run the optimization in serial (0), multi-threaded (1) or networked (2) mode"
	);

	gpb.registerCLParameter<bool>(
		"serverMode,s"
		, serverMode
		, false // Use client mode, if no server option is specified
		, "Whether to run networked execution in server or client mode. The option only has an effect if \"--parallelizationMode=2\". You can either say \"--server=true\" or just \"--server\"."
		, GCL_IMPLICIT_ALLOWED // Permit implicit values, so that we can say --server instead of --server=true
		, true // Use server mode, of only -s or --server was specified
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

	gpb.registerCLParameter<std::uint32_t>(
		"maxStalls"
		, maxStalls
		, DEFAULTMAXSTALLS06
		, "The number of stalled data transfers (i.e. transfers without a useful work item returned) before the client terminates in networked mode"
	);

	gpb.registerCLParameter<std::uint32_t>(
		"maxConnectionAttempts"
		, maxConnectionAttempts
		, DEFAULTMAXCONNECTIONATTEMPTS06
		, "The number of connection attempts from client to server, before the client terminates in networked mode"
	);

	gpb.registerCLParameter<Gem::Common::serializationMode>(
		"serializationMode"
		, serMode
		, DEFAULTSERMODE
		, "Specifies whether serialization shall be done in TEXTMODE (0), XMLMODE (1) or BINARYMODE (2)"
	);

	gpb.registerCLParameter<bool>(
		"addLocalConsumer"
		, addLocalConsumer
		, DEFAULTADDLOCALCONSUMER // Use client mode, if no server option is specified
		, "Whether or not a local consumer should be added to networked execution. You can use this option with or without arguments."
		, GCL_IMPLICIT_ALLOWED // Permit implicit values, so that we can say --server instead of --server=true
		, true // Use a local consumer if the option --addLocalConsumer was given without arguments
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
	execMode parallelizationMode;
	bool serverMode;
	std::string ip;
	unsigned short port;
	std::uint32_t maxStalls;
	std::uint32_t maxConnectionAttempts;
	std::uint16_t nProducerThreads;
	std::uint16_t nEvaluationThreads;
	std::uint32_t maxIterations;
	long maxMinutes;
	std::uint32_t reportIteration;
	Gem::Common::serializationMode serMode;
	bool addLocalConsumer;
	std::size_t nNeighborhoods;
	std::size_t nNeighborhoodMembers;
	double cPersonal;
	double cNeighborhood;
	double cGlobal;
	double cVelocity;
	updateRule ur;
	bool allRandomInit;

	/****************************************************************************/
	// Initialization of Geneva
	GenevaInitializer gi;

	/****************************************************************************/
	// Retrieve all necessary configuration data from the command line

	if(!parseCommandLine(
		argc, argv
		, parallelizationMode
		, serverMode
		, ip
		, port
		, maxStalls
		, maxConnectionAttempts
		, serMode
		, addLocalConsumer
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
	)) { exit(1); }

	/****************************************************************************/
	// Random numbers are our most valuable good. Set the number of threads
	GRANDOMFACTORY->setNProducerThreads(nProducerThreads);

	/****************************************************************************/
	// If this is a client in networked mode, we can just start the listener and
	// return when it has finished
	if(execMode::EXECMODE_BROKERAGE==parallelizationMode && !serverMode) {
		std::shared_ptr<GAsioTCPClientT<GParameterSet>>
			p(new GAsioTCPClientT<GParameterSet>(ip, boost::lexical_cast<std::string>(port)));

		p->setMaxStalls(maxStalls); // 0 would mean an infinite number of stalled data retrievals
		p->setMaxConnectionAttempts(maxConnectionAttempts);

		// Start the actual processing loop
		p->run();

		return 0;
	}

	/****************************************************************************/
	// We can now start creating populations. We refer to them through the base class

	// This smart pointer will hold the different population types
	std::shared_ptr<GBaseSwarm> pop_ptr;

	// Create the actual populations
	switch (parallelizationMode) {
		//---------------------------------------------------------------------------
		case execMode::EXECMODE_SERIAL: // Serial execution
			// Create an empty population
			pop_ptr
				= std::shared_ptr<GSerialSwarm>(new GSerialSwarm(nNeighborhoods, nNeighborhoodMembers));
			break;

			//---------------------------------------------------------------------------
		case execMode::EXECMODE_MULTITHREADED: // Multi-threaded execution
		{
			// Create the multi-threaded population
			std::shared_ptr<GMultiThreadedSwarm>
				popPar_ptr(new GMultiThreadedSwarm(nNeighborhoods, nNeighborhoodMembers));

			// Population-specific settings
			popPar_ptr->setNThreads(nEvaluationThreads);

			// Assignment to the base pointer
			pop_ptr = popPar_ptr;
		}
			break;

			//---------------------------------------------------------------------------
		case execMode::EXECMODE_BROKERAGE: // Networked execution (server-side)
		{
			// Create a network consumer and enrol it with the broker
			std::shared_ptr<GAsioTCPConsumerT<GParameterSet>>
				gatc(new GAsioTCPConsumerT<GParameterSet>(port, 0, serMode));
			GBROKER(Gem::Geneva::GParameterSet)->enrol(gatc);

			if(addLocalConsumer) { // This is mainly for testing and benchmarking
				std::shared_ptr<GBoostThreadConsumerT<GParameterSet>>
					gbtc(new GBoostThreadConsumerT<GParameterSet>());
				gbtc->setNThreadsPerWorker(nEvaluationThreads);
				GBROKER(Gem::Geneva::GParameterSet)->enrol(gbtc);
			}

			// Create the actual broker population
			std::shared_ptr<GBrokerSwarm>
				popBroker_ptr(new GBrokerSwarm(nNeighborhoods, nNeighborhoodMembers));

			// Assignment to the base pointer
			pop_ptr = popBroker_ptr;
		}
			break;

			//----------------------------------------------------------------------------
		default:
		{
			glogger
			<< "In main(): Received invalid parallelization mode " << parallelizationMode << std::endl
			<< GEXCEPTION;
		}
			break;
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
