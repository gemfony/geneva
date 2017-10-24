/**
 * @file GBrokerOverhead.cpp
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

// Standard header files go here
#include <iostream>
#include <cmath>
#include <sstream>
#include <chrono>

// Boost header files go here
#include <boost/lexical_cast.hpp>

// Geneva header files go here
#include "courtier/GCourtierHelperFunctions.hpp"
#include "courtier/GCourtierEnums.hpp"
#include "courtier/GExecutorT.hpp"
#include "common/GCommonEnums.hpp"
#include "common/GSerializationHelperFunctionsT.hpp"
#include "geneva/Go2.hpp"
#include "geneva/GOptimizationEnums.hpp"

// The individual that should be optimized
#include "geneva-individuals/GFunctionIndividual.hpp"

using namespace Gem::Geneva;
using namespace Gem::Common;
using namespace Gem::Courtier;

/************************************************************************************************/

// Default settings
const std::uint16_t DEFAULTNPRODUCERTHREADS=10;
const std::uint16_t DEFAULTNEVALUATIONTHREADS=4;
const std::size_t DEFAULTNPARENTS=5; // Allow to explore the parameter space from many starting points
const std::uint32_t DEFAULTMAXITERATIONS=2000;
const long DEFAULTMAXMINUTES=10;
const std::uint32_t DEFAULTREPORTITERATION=1;
const duplicationScheme DEFAULTRSCHEME=duplicationScheme::VALUEDUPLICATIONSCHEME;
const bool DEFAULTVERBOSE=true;
const execMode DEFAULTPARALLELIZATIONMODE=execMode::MULTITHREADED;
const bool DEFAULTUSECOMMONADAPTOR=false; // whether to use a common adaptor for all GParameterT objects
const unsigned short DEFAULTPORT=10000;
const std::string DEFAULTIP="localhost";
const std::string DEFAULTCONFIGFILE="./GBrokerOverhead.cfg";
const sortingMode DEFAULTSORTINGSCHEME=sortingMode::MUPLUSNU_SINGLEEVAL;
const std::uint32_t DEFAULTSTARTITERATION=0;
const std::size_t DEFAULTNBTCONSUMERTHREADS=2;
const std::uint32_t DEFAULTGBTCNPROCUNITS=1;
const std::size_t DEFAULTPARDIM=100;
const double DEFAULTMINVAR=-10.;
const double DEFAULTMAXVAR=10.;
const std::uint16_t DEFAULTEVALFUNCTION=0;
const double DEFAULTGDAADPROB=1.0;

/************************************************************************************************/
/**
	* A function that parses the command line for all required parameters
	*/
bool parseCommandLine(
	int argc, char **argv
	, execMode &parallelizationMode
	, std::uint16_t &nProducerThreads
	, std::uint16_t &nEvaluationThreads
	, std::size_t &populationSize
	, std::size_t &nParents
	, std::uint32_t &maxIterations
	, long &maxMinutes
	, std::uint32_t &reportIteration
	, duplicationScheme &rScheme
	, sortingMode &smode
	, std::uint32_t &nProcessingUnits
	, double &adProb
	, std::uint32_t &adaptionThreshold
	, double &sigma
	, double &sigmaSigma
	, double &minSigma
	, double &maxSigma
	, std::size_t &parDim
	, double &minVar
	, double &maxVar
	, solverFunction &df
) {
	std::uint16_t evalFunction = 0;

	// Create the parser builder
	Gem::Common::GParserBuilder gpb;

	gpb.registerCLParameter<execMode>(
		"parallelizationMode,p"
		, parallelizationMode
		, DEFAULTPARALLELIZATIONMODE
		, "Whether to run this optimization in serial mode (0), multi-threaded (1) or mt-consumer (2) mode"
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
		, "The amount of threads processing individuals simultaneously"
	);

	gpb.registerCLParameter<std::size_t>(
		"populationSize"
		, populationSize
		, DEFAULTPOPULATIONSIZE
		, "The size of the super-population"
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
		, "Maximum number of iterations in the population"
	);

	gpb.registerCLParameter<long>(
		"maxMinutes"
		, maxMinutes
		, DEFAULTMAXMINUTES
		, "The maximum number of minutes the optimization of the population should run"
	);

	gpb.registerCLParameter<std::uint32_t>(
		"reportIteration"
		, reportIteration
		, DEFAULTREPORTITERATION
		, "The number of iterations after which information should be emitted in the super-population"
	);

	gpb.registerCLParameter<duplicationScheme>(
		"rScheme"
		, rScheme
		, DEFAULTRSCHEME
		, "The recombination scheme for the super-population"
	);

	gpb.registerCLParameter<sortingMode>(
		"sortingScheme,o"
		, smode
		, DEFAULTSORTINGSCHEME
		, "Determines whether sorting is done in MUCOMMANU_SINGLEEVAL (0), MUPLUSNU_SINGLEEVAL (1) or MUNU1PRETAIN (2) mode"
	);

	gpb.registerCLParameter<std::uint32_t>(
		"nProcessingUnits"
		, nProcessingUnits
		, DEFAULTGBTCNPROCUNITS
		, "Specifies how many processing units are available in networked mode"
	);

	gpb.registerCLParameter<double>(
		"adProb"
		, adProb
		, DEFAULTGDAADPROB
		, "Specifies the likelihood for adaptions to be actually carried out"
	);

	gpb.registerCLParameter<std::uint32_t>(
		"adaptionThreshold"
		, adaptionThreshold
		, DEFAULTADAPTIONTHRESHOLD
		, "Number of calls to adapt() after which adaption parameters should be modified"
	);

	gpb.registerCLParameter<double>(
		"sigma"
		, sigma
		, DEFAULTSIGMA
		, "The width of the gaussian used for the adaption of double values"
	);

	gpb.registerCLParameter<double>(
		"sigmaSigma"
		, sigmaSigma
		, DEFAULTSIGMASIGMA
		, "The adaption rate of sigma"
	);

	gpb.registerCLParameter<double>(
		"minSigma"
		, minSigma
		, DEFAULTMINSIGMA
		, "The minimum allowed value for sigma"
	);

	gpb.registerCLParameter<double>(
		"maxSigma"
		, maxSigma
		, DEFAULTMAXSIGMA
		, "The maximum allowed value for sigma"
	);

	gpb.registerCLParameter<std::size_t>(
		"parDim"
		, parDim
		, DEFAULTPARDIM
		, "The amount of variables in the parabola"
	);

	gpb.registerCLParameter<double>(
		"minVar"
		, minVar
		, DEFAULTMINVAR
		, "The lower boundary for all variables"
	);

	gpb.registerCLParameter<double>(
		"maxVar"
		, maxVar
		, DEFAULTMAXVAR
		, "The upper boundary for all variables"
	);

	gpb.registerCLParameter<std::uint16_t>(
		"evalFunction"
		, evalFunction
		, 0
		, "The id of the evaluation function"
	);


	// Some post-processing:

	// Check the number of parents in the super-population
	if (2 * nParents > populationSize) {
		glogger
			<< "Error: Invalid number of parents inpopulation" << std::endl
			<< "nParents       = " << nParents << std::endl
			<< "populationSize = " << populationSize << std::endl
			<< GWARNING;

		return false;
	}

	// Assign the demo function
	if (evalFunction > (std::uint16_t) MAXDEMOFUNCTION) {
		std::cout << "Error: Invalid evaluation function: " << evalFunction
					 << std::endl;
		return false;
	}
	df = (solverFunction) evalFunction;


	// Parse the command line and leave if the help flag was given. The parser
	// will emit an appropriate help message by itself
	if(Gem::Common::GCL_HELP_REQUESTED == gpb.parseCommandLine(argc, argv, true /*verbose*/)) {
		return false; // Do not continue
	}

	return true;
}

/************************************************************************************************/
/**
 * The main function.
 */
int main(int argc, char **argv){
	std::string configFile;
	execMode parallelizationMode;
	std::string ip;
	std::uint16_t nProducerThreads;
	std::uint16_t nEvaluationThreads;
	std::size_t populationSize;
	std::size_t nParents;
	std::uint32_t maxIterations;
	long maxMinutes;
	std::uint32_t reportIteration;
	duplicationScheme rScheme;
	std::size_t parDim;
	double minVar;
	double maxVar;
	sortingMode smode;
	std::uint32_t nProcessingUnits;
	solverFunction df;
	std::uint32_t adaptionThreshold;
	double sigma;
	double sigmaSigma;
	double minSigma;
	double maxSigma;
	double adProb;

	// Parse the command line
	if(!parseCommandLine(
		argc, argv
		, parallelizationMode
		, nProducerThreads
		, nEvaluationThreads
		, populationSize
		, nParents
		, maxIterations
		, maxMinutes
		, reportIteration
		, rScheme
		, smode
		, nProcessingUnits
		, adProb
		, adaptionThreshold
		, sigma
		, sigmaSigma
		, minSigma
		, maxSigma
		, parDim
		, minVar
		, maxVar
		, df
	)) { exit(0); }

	// Random numbers are our most valuable good. Set the number of threads
	GRANDOMFACTORY->setNProducerThreads(nProducerThreads);

	//***************************************************************************
	// Create a factory for GFunctionIndividual objects and perform
	// any necessary initial work.
	GFunctionIndividualFactory gfi("./config/GFunctionIndividual.json");

	// Create the first set of parent individuals. Initialization of parameters is done randomly.
	std::vector<std::shared_ptr<GParameterSet>> parentIndividuals;
	for(std::size_t p = 0 ; p<nParents; p++) {
		std::shared_ptr<GParameterSet> functionIndividual_ptr = gfi();

		// Set up a GDoubleCollection with dimension values, each initialized
		// with a random number in the range [min,max[
		std::shared_ptr<GDoubleCollection> gdc_ptr(new GDoubleCollection(parDim,minVar,maxVar));

		// Set up and register an adaptor for the collection, so it
		// knows how to be adapted.
		std::shared_ptr<GDoubleGaussAdaptor> gdga_ptr(new GDoubleGaussAdaptor(sigma,sigmaSigma,minSigma,maxSigma));
		gdga_ptr->setAdaptionThreshold(adaptionThreshold);
		gdga_ptr->setAdaptionProbability(adProb);
		gdc_ptr->addAdaptor(gdga_ptr);

		// Make the parameter collection known to this individual
		functionIndividual_ptr->push_back(gdc_ptr);

		parentIndividuals.push_back(functionIndividual_ptr);
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// We can now start creating populations. Executors are registered for different execution modes

	std::shared_ptr<GEvolutionaryAlgorithm> pop_ptr(new GEvolutionaryAlgorithm());

	switch (parallelizationMode) {
		case execMode::SERIAL: // Serial execution
			std::cout << "Using serial execution." << std::endl;
			pop_ptr->registerExecutor(execMode::SERIAL, "./config/GSerialExecutor.json");
			break;

		case execMode::MULTITHREADED: // Multi-threaded execution
			std::cout << "Using plain multi-threaded execution." << std::endl;
			pop_ptr->registerExecutor(execMode::MULTITHREADED, "./config/GMTExecutor.json");

			// Set the number of threads used in the executor
			pop_ptr->getExecutor<Gem::Courtier::GMTExecutorT<GParameterSet>>()->setNThreads(nEvaluationThreads);

			break;

		case execMode::BROKER: // Execution with multi-threaded consumer. Note that we use BROKER here, even though no networked execution takes place
		{
			// Create a consumer and make it known to the global broker
			std::shared_ptr<Gem::Courtier::GStdThreadConsumerT<GParameterSet>> stc(new Gem::Courtier::GStdThreadConsumerT<GParameterSet>());
			stc->setNThreadsPerWorker(nEvaluationThreads);
			GBROKER(Gem::Geneva::GParameterSet)->enrol(stc);

			std::cout << "Using the GStdThreadConsumerT consumer." << std::endl;
			pop_ptr->registerExecutor(
				execMode::BROKER
				, "./config/GBrokerExecutor.json"
			);
		}
			break;
	}

	// Add individuals to the population
	for (std::size_t p = 0; p < nParents; p++) {
		pop_ptr->push_back(parentIndividuals[p]);
	}

	// Specify some general population settings
	pop_ptr->setPopulationSizes(
		populationSize
		, nParents
	);
	pop_ptr->setMaxIteration(maxIterations);
	pop_ptr->setMaxTime(std::chrono::minutes(maxMinutes));
	pop_ptr->setReportIteration(reportIteration);
	pop_ptr->setRecombinationMethod(rScheme);
	pop_ptr->setSortingScheme(smode);

	// Do the actual optimization
	pop_ptr->optimize();

	// Terminate
	return 0;
}
