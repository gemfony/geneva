/**
 * @file GBrokerOverhead.cpp
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
			std::shared_ptr<Gem::Courtier::GStdThreadConsumerT<GParameterSet>> stc(
				new Gem::Courtier::GStdThreadConsumerT<GParameterSet>(nEvaluationThreads)
			);
			GBROKER(Gem::Geneva::GParameterSet)->enrol_consumer(stc);

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
