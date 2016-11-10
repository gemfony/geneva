/**
 * @file GDirectEA.cpp
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
#include "geneva/GEvolutionaryAlgorithmFactory.hpp"

// The individual that should be optimized
#include "geneva-individuals/GFunctionIndividual.hpp"

using namespace Gem::Geneva;
using namespace Gem::Courtier;
using namespace Gem::Hap;
using namespace Gem::Common;

using namespace Gem::Common;
namespace po = boost::program_options;

/******************************************************************************/
// Default settings
const execMode DEFAULTPARALLELIZATIONMODEAP=execMode::EXECMODE_MULTITHREADED;
const unsigned short DEFAULTPORT=10000;
const std::string DEFAULTIP="localhost";
const std::uint32_t DEFAULTMAXSTALLS06=0;
const std::uint32_t DEFAULTMAXCONNECTIONATTEMPTS06=100;
const std::uint16_t DEFAULTNPRODUCERTHREADS=10;
const Gem::Common::serializationMode DEFAULTSERMODE=Gem::Common::serializationMode::SERIALIZATIONMODE_TEXT;
const bool DEFAULTADDLOCALCONSUMER=false;
const std::uint16_t DEFAULTNEVALUATIONTHREADS=4;
const std::size_t DEFAULTPOPULATIONSIZE06=100;
const std::size_t DEFAULTNPARENTS=5; // Allow to explore the parameter space from many starting points
const std::uint32_t DEFAULTMAXITERATIONS=200;
const std::uint32_t DEFAULTREPORTITERATION=1;
const long DEFAULTMAXMINUTES=10;
const duplicationScheme DEFAULTRSCHEME=duplicationScheme::VALUEDUPLICATIONSCHEME;
const sortingMode DEFAULTSORTINGSCHEME=sortingMode::MUCOMMANU_SINGLEEVAL;

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
	, std::uint16_t& nProducerThreads
	, std::uint16_t& nEvaluationThreads
	, std::size_t& populationSize
	, std::size_t& nParents
	, std::uint32_t& maxIterations
	, long& maxMinutes
	, std::uint32_t& reportIteration
	, duplicationScheme& rScheme
	, sortingMode& smode
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

	gpb.registerCLParameter<std::size_t>(
		"populationSize"
		, populationSize
		, DEFAULTPOPULATIONSIZE06
		, "The desired size of the population"
	);

	gpb.registerCLParameter<std::size_t>(
		"nParents"
		, nParents
		, DEFAULTNPARENTS
		, "The number of parents in the population"
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

	gpb.registerCLParameter<duplicationScheme>(
		"rScheme"
		, rScheme
		, DEFAULTRSCHEME
		, "The recombination scheme of the evolutionary algorithm"
	);

	gpb.registerCLParameter<sortingMode>(
		"smode"
		, smode
		, DEFAULTSORTINGSCHEME
		, "Determines whether sorting is done in MUPLUSNU_SINGLEEVAL (0), MUCOMMANU_SINGLEEVAL (1) or MUNU1PRETAIN (2) mode"
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
	std::size_t populationSize;
	std::size_t nParents;
	std::uint32_t maxIterations;
	long maxMinutes;
	std::uint32_t reportIteration;
	duplicationScheme rScheme;
	sortingMode smode;
	Gem::Common::serializationMode serMode;
	bool addLocalConsumer;

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
		, nProducerThreads
		, nEvaluationThreads
		, populationSize
		, nParents
		, maxIterations
		, maxMinutes
		, reportIteration
		, rScheme
		, smode
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
	std::shared_ptr<GBaseEA> pop_ptr;

	// Create the actual populations
	switch (parallelizationMode) {
		//----------------------------------------------------------------------------
		case execMode::EXECMODE_SERIAL: // Serial execution
		{
			// Create an empty population
			pop_ptr = std::shared_ptr<GSerialEA>(new GSerialEA());
		}
			break;

			//----------------------------------------------------------------------------
		case execMode::EXECMODE_MULTITHREADED: // Multi-threaded execution
		{
			// Create the multi-threaded population
			std::shared_ptr<GMultiThreadedEA> popPar_ptr(new GMultiThreadedEA());

			// Population-specific settings
			popPar_ptr->setNThreads(nEvaluationThreads);

			// Assignment to the base pointer
			pop_ptr = popPar_ptr;
		}
			break;

			//----------------------------------------------------------------------------
		case execMode::EXECMODE_BROKERAGE: // Execution with networked consumer and possibly a local, multi-threaded consumer
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
			std::shared_ptr<GBrokerEA> popBroker_ptr(new GBrokerEA());

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
	// Create a factory for GFunctionIndividual objects and perform
	// any necessary initial work.
	GFunctionIndividualFactory gfi("./config/GFunctionIndividual.json");

	// Create the first set of parent individuals. Initialization of parameters is done randomly.
	std::vector<std::shared_ptr<GFunctionIndividual>> parentIndividuals;
	for(std::size_t p = 0 ; p<nParents; p++) {
		parentIndividuals.push_back(gfi.get_as<GFunctionIndividual>());
	}

	/****************************************************************************/
	// Now we have suitable populations and can fill them with data

	// Add individuals to the population. Many Geneva classes, such as
	// the optimization classes, feature an interface very similar to std::vector.
	for(std::size_t p = 0 ; p<parentIndividuals.size(); p++) {
		pop_ptr->push_back(parentIndividuals[p]);
	}

	// Specify some general population settings
	pop_ptr->setPopulationSizes(populationSize,nParents);
	pop_ptr->setMaxIteration(maxIterations);
	pop_ptr->setMaxTime(std::chrono::minutes(maxMinutes));
	pop_ptr->setReportIteration(reportIteration);
	pop_ptr->setRecombinationMethod(rScheme);
	pop_ptr->setSortingScheme(smode);

	// Perform the actual optimization
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
	return(0);
}
