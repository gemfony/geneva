/**
 * @file GDirectEA.cpp
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
#include "common/GParserBuilder.hpp"
#include "geneva/GenevaInitializer.hpp"
#include "geneva/G_OptimizationAlgorithm_SimulatedAnnealing.hpp"

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
const execMode DEFAULTPARALLELIZATIONMODEAP=execMode::MULTITHREADED;
const unsigned short DEFAULTPORT=10000;
const std::string DEFAULTIP="localhost";
const std::uint16_t DEFAULTNPRODUCERTHREADS=10;
const Gem::Common::serializationMode DEFAULTSERMODE=Gem::Common::serializationMode::TEXT;
const bool DEFAULTADDLOCALCONSUMER=false;
const std::uint16_t DEFAULTNEVALUATIONTHREADS=4;
const std::size_t DEFAULTPOPULATIONSIZE06=100;
const std::size_t DEFAULTNPARENTS=5; // Allow to explore the parameter space from many starting points
const std::uint32_t DEFAULTMAXITERATIONS=200;
const std::uint32_t DEFAULTREPORTITERATION=1;
const long DEFAULTMAXMINUTES=10;
const duplicationScheme DEFAULTRSCHEME=duplicationScheme::VALUEDUPLICATIONSCHEME;
const std::size_t DEFAULTMAXRECONNECTS = 10;

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
	, std::size_t& maxReconnects
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
	execMode parallelizationMode;
	bool serverMode;
	std::string ip;
	unsigned short port;
	std::uint16_t nProducerThreads;
	std::uint16_t nEvaluationThreads;
	std::size_t populationSize;
	std::size_t nParents;
	std::uint32_t maxIterations;
	long maxMinutes;
	std::uint32_t reportIteration;
	duplicationScheme rScheme;
	Gem::Common::serializationMode serMode;
	bool addLocalConsumer;
	std::size_t maxReconnects;

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
		, maxReconnects
	)) { exit(1); }

	/****************************************************************************/
	// Random numbers are our most valuable good. Set the number of threads
	GRANDOMFACTORY->setNProducerThreads(nProducerThreads);

	/****************************************************************************/
	// If this is a client in networked mode, we can just start the listener and
	// return when it has finished
	if(execMode::BROKER==parallelizationMode && !serverMode) {
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

	// Create a factory for GFunctionIndividual objects and perform
	// any necessary initial work.
	GFunctionIndividualFactory gfi("./config/GFunctionIndividual.json");

	// Create the first set of parent individuals. Initialization of parameters is done randomly.
	std::vector<std::shared_ptr<GFunctionIndividual>> parentIndividuals;
	for(std::size_t p = 0 ; p<nParents; p++) {
		parentIndividuals.push_back(gfi.get_as<GFunctionIndividual>());
	}

	/****************************************************************************/

	// Create an empty population
	std::shared_ptr<GSimulatedAnnealing> pop_ptr(new GSimulatedAnnealing());

	// General settings
	pop_ptr->setPopulationSizes(populationSize,nParents);
	pop_ptr->setMaxIteration(maxIterations);
	pop_ptr->setMaxTime(std::chrono::minutes(maxMinutes));
	pop_ptr->setReportIteration(reportIteration);
	pop_ptr->setRecombinationMethod(rScheme);

	// Add individuals to the population.
	for(auto ind: parentIndividuals) {
		pop_ptr->push_back(ind);
	}

	// Register executors, depending on the parallelisation mode, possibly adding
	// a local consumer in broker-mode
	switch (parallelizationMode) {
		//----------------------------------------------------------------------------
		case execMode::SERIAL: // Serial execution
			pop_ptr->registerExecutor(execMode::SERIAL, "./config/GSerialExecutor.json");
			break;

			//----------------------------------------------------------------------------
		case execMode::MULTITHREADED: // Multi-threaded execution
			pop_ptr->registerExecutor(execMode::MULTITHREADED, "./config/GMTExecutor.json");

			// Configure the number of threads
			pop_ptr->getExecutor<Gem::Courtier::GMTExecutorT<GParameterSet>>()->setNThreads(nEvaluationThreads);
			break;

			//----------------------------------------------------------------------------
		case execMode::BROKER: // Execution with networked consumer and possibly a local, multi-threaded consumer
			if(addLocalConsumer) {
				// Create a multi-threaded consumer. This
				// is mainly for testing and benchmarking
				std::shared_ptr<GStdThreadConsumerT<GParameterSet>> gbtc(
					new GStdThreadConsumerT<GParameterSet>(nEvaluationThreads)
				);
				GBROKER(Gem::Geneva::GParameterSet)->enrol_consumer(gbtc);
			} else {
				// Create a network consumer and enrol_buffer_port it with the broker
				std::shared_ptr<GAsioConsumerT<GParameterSet>> gatc_ptr(new GAsioConsumerT<GParameterSet>());

				// Set the required options
				gatc_ptr->setServerName(ip);
				gatc_ptr->setPort(port);
				gatc_ptr->setSerializationMode(serMode);
				gatc_ptr->setNThreads(nEvaluationThreads);
                gatc_ptr->setMaxClientReconnects( maxReconnects );

				// Add the consumer to the broker
				GBROKER(Gem::Geneva::GParameterSet)->enrol_consumer(gatc_ptr);
			}

			pop_ptr->registerExecutor(execMode::BROKER, "./config/GBrokerExecutor.json");
			break;

			//----------------------------------------------------------------------------
	}

	/****************************************************************************/
	// Perform the actual optimization
	pop_ptr->optimize();

	// Retrieve the best individual found
	auto p = pop_ptr->getBestGlobalIndividual<GFunctionIndividual>();

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
