/**
 * @file GArgumentParser.cpp
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

#include "GArgumentParser.hpp"

namespace Gem {
namespace Geneva {
/************************************************************************************************/
/**
 * A function that parses the command line for all required parameters
 */
bool parseCommandLine(
		int argc, char **argv
		, std::string& configFile
		, boost::uint16_t& parallelizationMode
		, bool& serverMode
		, std::string& ip
		, unsigned short& port
		, Gem::Common::serializationMode& serMode
) {
	try {
		// Check the command line options. Uses the Boost program options library.
		po::options_description desc("Usage: evaluator [options]");
		desc.add_options()("help,h", "emit help message")("configFile,c",
				po::value<std::string>(&configFile)->default_value(
						DEFAULTCONFIGFILE),
				"The name of the configuration file holding further configuration options")(
				"parallelizationMode,p",
				po::value<boost::uint16_t>(&parallelizationMode)->default_value(
						DEFAULTPARALLELIZATIONMODE),
				"Whether or not to run this optimization in serial mode (0), multi-threaded (1) or networked (2) mode")(
				"serverMode,s",
				"Whether to run networked execution in server or client mode. The option only gets evaluated if \"--parallelizationMode=2\"")(
				"ip", po::value<std::string>(&ip)->default_value(DEFAULTIP),
				"The ip of the server")("port",
				po::value<unsigned short>(&port)->default_value(DEFAULTPORT),
				"The port of the server")(
				"serMode",
				po::value<Gem::Common::serializationMode>(&serMode)->default_value(
						DEFAULTSERMODE),
				"Specifies whether serialization shall be done in TEXTMODE (0), XMLMODE (1) or BINARYMODE (2)");

		po::variables_map vm;
		po::store(po::parse_command_line(argc, argv, desc), vm);
		po::notify(vm);

		// Emit a help message, if necessary
		if (vm.count("help")) {
			std::cerr << desc << std::endl;
			return false;
		}

		serverMode = false;
		if (vm.count("parallelizationMode")) {
			if (parallelizationMode > 2) {
				std::cout
						<< "Error: the \"-p\" or \"--parallelizationMode\" option may only assume the"
						<< std::endl
						<< "values 0 (serial), 1 (multi-threaded) or 2 (networked). Leaving ..."
						<< std::endl;
				return false;
			}

			if (parallelizationMode == 2)
				if (vm.count("serverMode"))
					serverMode = true;
		}

		if (parallelizationMode != DEFAULTPARALLELIZATIONMODE || ip
				!= DEFAULTIP || port != DEFAULTPORT) {
			std::string parModeString;
			switch (parallelizationMode) {
			case 0:
				parModeString = "serial";
				break;
			case 1:
				parModeString = "multi-threaded";
				break;
			case 2:
				parModeString = "networked";
				break;
			};

			std::cout << std::endl
					<< "Running with the following command line options:"
					<< std::endl << "configFile = " << configFile << std::endl
					<< "parallelizationMode = " << parModeString << std::endl
					<< "serverMode = " << (serverMode ? "true" : "false")
					<< std::endl << "ip = " << ip << std::endl << "port = "
					<< port << std::endl << "serMode = " << serMode
					<< std::endl << std::endl;
		}
	} catch (...) {
		std::cout << "Error parsing the command line" << std::endl;
		return false;
	}

	return true;
}

/************************************************************************************************/
/**
 * A function that parses a config file for further parameters
 */
bool parseConfigFile(
		const std::string& configFile
		, boost::uint16_t& nProducerThreads
		, boost::uint16_t& nEvaluationThreads
		, std::size_t& populationSize
		, std::size_t& nParents
		, boost::uint32_t& maxIterations
		, long& maxMinutes
		, boost::uint32_t& reportIteration
		, recoScheme& rScheme
		, sortingMode& smode
		, std::size_t& arraySize
		, boost::uint32_t& processingCycles
		, bool& returnRegardless
		, boost::uint32_t& waitFactor
		, double& adProb
		, boost::uint32_t& adaptionThreshold
		, double& sigma
		, double& sigmaSigma
		, double& minSigma
		, double& maxSigma
		, std::size_t& parDim
		, double& minVar
		, double& maxVar
		, demoFunction& df
		, boost::uint16_t& xDim
		, boost::uint16_t& yDim
		, bool& followProgress
) {
	boost::uint16_t recombinationScheme = 0;
	boost::uint16_t sortingScheme = 0;
	boost::uint16_t evalFunction = 0;
	bool verbose = true;

	// Check the name of the configuation file
	if (configFile.empty() || configFile == "empty" || configFile == "unknown") {
		std::cerr << "Error: Invalid configuration file name given: \""
				<< configFile << "\"" << std::endl;
		return false;
	}

	try {
		// Check the configuration file line options. Uses the Boost program options library.
		po::options_description config("Allowed options");
		config.add_options()
			("nProducerThreads", po::value<boost::uint16_t>(&nProducerThreads)->default_value(DEFAULTNPRODUCERTHREADS),
			"The amount of random number producer threads")
			("nEvaluationThreads", po::value<boost::uint16_t>(&nEvaluationThreads)->default_value(DEFAULTNEVALUATIONTHREADS),
			"The amount of threads processing individuals simultaneously")
			("populationSize", po::value<std::size_t>(&populationSize)->default_value(DEFAULTPOPULATIONSIZE),
			"The size of the super-population")
			("nParents", po::value<std::size_t>(&nParents)->default_value(DEFAULTNPARENTS),
			"The number of parents in the population") // Needs to be treated separately
			("maxIterations", po::value<boost::uint32_t>(&maxIterations)->default_value(DEFAULTMAXITERATIONS),
			"Maximum number of iterations in the population")
			("maxMinutes",	po::value<long>(&maxMinutes)->default_value(DEFAULTMAXMINUTES),
			"The maximum number of minutes the optimization of the population should run")
			("reportIteration",	po::value<boost::uint32_t>(&reportIteration)->default_value(DEFAULTREPORTITERATION),
			"The number of iterations after which information should be emitted in the super-population")
			("rScheme",	po::value<boost::uint16_t>(&recombinationScheme)->default_value(DEFAULTRSCHEME),
			"The recombination scheme for the super-population")
			("sortingScheme,o",	po::value<boost::uint16_t>(&sortingScheme)->default_value(DEFAULTSORTINGSCHEME),
			"Determines whether sorting is done in MUPLUSNU (0), MUCOMMANU (1)  or MUNU1PRETAIN (2) mode")
			("arraySize", po::value<std::size_t>(&arraySize)->default_value(DEFAULTARRAYSIZE),
			"The size of the buffer with random arrays in the random factory")
			("verbose", po::value<bool>(&verbose)->default_value(DEFAULTVERBOSE),
			"Whether additional information should be emitted")
			("processingCycles", po::value<boost::uint32_t>(&processingCycles)->default_value(DEFAULTPROCESSINGCYCLES),
			"The maximum number of cycles a client should perform adaptions before it returns without success")
			("returnRegardless", po::value<bool>(&returnRegardless)->default_value(DEFAULTRETURNREGARDLESS),
			"Specifies whether results should be returned even if they are not better than before")
			("waitFactor", po::value<boost::uint32_t>(&waitFactor)->default_value(DEFAULTGBTCWAITFACTOR),
			"Influences the maximum waiting time of the GBrokerEA after the arrival of the first evaluated individuum")
			("adProb", po::value<double>(&adProb)->default_value(DEFAULTGDAADPROB),
			"Specifies the likelihood for adaptions to be actually carried out")
			("adaptionThreshold", po::value<boost::uint32_t>(&adaptionThreshold)->default_value(DEFAULTADAPTIONTHRESHOLD),
			"Number of calls to adapt() after which adaption parameters should be modified")
			("sigma", po::value<double>(&sigma)->default_value(DEFAULTSIGMA),
			"The width of the gaussian used for the adaption of double values")
			("sigmaSigma", po::value<double>(&sigmaSigma)->default_value(DEFAULTSIGMASIGMA),
			"The adaption rate of sigma")
			("minSigma", po::value<double>(&minSigma)->default_value(DEFAULTMINSIGMA),
			"The minimum allowed value for sigma")
			("maxSigma", po::value<double>(&maxSigma)->default_value(DEFAULTMAXSIGMA),
			"The maximum allowed value for sigma")
			("parDim", po::value<std::size_t>(&parDim)->default_value(DEFAULTPARDIM),
			"The amount of variables in the parabola")
			("minVar", po::value<double>(&minVar)->default_value(DEFAULTMINVAR),
			"The lower boundary for all variables")
			("maxVar", po::value<double>(&maxVar)->default_value(DEFAULTMAXVAR),
			"The upper boundary for all variables")
			("evalFunction",po::value<boost::uint16_t>(&evalFunction),
			"The id of the evaluation function.")
	       ("xDim", po::value<boost::uint16_t>(&xDim)->default_value(DEFAULTXDIMAP),
	        "The x-dimension of the canvas for the result print(s)")
	       ("yDim", po::value<boost::uint16_t>(&yDim)->default_value(DEFAULTYDIMAP),
	        "The y-dimension of the canvas for the result print(s)")
	       ("followProgress", po::value<bool>(&followProgress)->default_value(DEFAULTFOLLOWPROGRESS),
	    	"Specifies whether snapshots should be taken in regular intervals");

		po::variables_map vm;
		std::ifstream ifs(configFile.c_str());
		if (!ifs.good()) {
			std::cerr << "Error accessing configuration file " << configFile;
			return false;
		}

		store(po::parse_config_file(ifs, config), vm);
		notify(vm);

		// Emit a help message, if necessary
		if (vm.count("help")) {
			std::cout << config << std::endl;
			return false;
		}

		// Check the number of parents in the super-population
		if (2 * nParents > populationSize) {
			std::cout << "Error: Invalid number of parents inpopulation"
					<< std::endl << "nParents       = " << nParents
					<< std::endl << "populationSize = " << populationSize
					<< std::endl;

			return false;
		}

		// Workaround for assigment problem with rScheme
		if (recombinationScheme == (boost::uint16_t) VALUERECOMBINE)        rScheme = VALUERECOMBINE;
		else if (recombinationScheme == (boost::uint16_t) RANDOMRECOMBINE)	rScheme = RANDOMRECOMBINE;
		else if (recombinationScheme == (boost::uint16_t) DEFAULTRECOMBINE)	rScheme = DEFAULTRECOMBINE;
		else {
			std::cout << "Error: Invalid recombination scheme in population: " << recombinationScheme << std::endl;
			return false;
		}

		// Convert sorting scheme to desired target type
		if (sortingScheme == (boost::uint16_t) MUPLUSNU)
			smode = MUPLUSNU;
		else if (sortingScheme == (boost::uint16_t) MUCOMMANU)
			smode = MUCOMMANU;
		else if (sortingScheme == (boost::uint16_t) MUNU1PRETAIN)
			smode = MUNU1PRETAIN;
		else {
			std::cout << "Error: Invalid sorting scheme in population: " << sortingScheme << std::endl;
			return false;
		}

		// Assign the demo function
		if(evalFunction > (boost::uint16_t)MAXDEMOFUNCTION) {
			std::cout << "Error: Invalid evaluation function: " << evalFunction
					<< std::endl;
			return false;
		}
		df=(demoFunction)evalFunction;

		if (verbose) {
			std::cout << std::endl
					<< "Running with the following options from " << configFile
					<< ":" << std::endl << "nProducerThreads = "
					<< (boost::uint16_t) nProducerThreads << std::endl // boost::uint8_t not printable on gcc ???
					<< "populationSize = " << populationSize << std::endl
					<< "nParents = " << nParents << std::endl
					<< "maxIterations = " << maxIterations << std::endl
					<< "maxMinutes = " << maxMinutes << std::endl
					<< "reportIteration = " << reportIteration << std::endl
					<< "rScheme = " << (boost::uint16_t) rScheme << std::endl
					<< "sortingScheme = " << smode << std::endl
					<< "arraySize = " << arraySize << std::endl
					<< "processingCycles = " << processingCycles << std::endl
					<< "waitFactor = " << waitFactor << std::endl
					<< "adProb = " << adProb << std::endl
					<< "adaptionThreshold = " << adaptionThreshold << std::endl
					<< "sigma = " << sigma << std::endl << "sigmaSigma "
					<< sigmaSigma << std::endl << "minSigma " << minSigma
					<< std::endl << "maxSigma " << maxSigma << std::endl
					<< "parDim = " << parDim << std::endl << "minVar = "
					<< minVar << std::endl << "maxVar = " << maxVar
					<< std::endl << "evalFunction = " << GFunctionIndividual<>::getStringRepresentation(df) << std::endl
					<< "xDim = " << xDim << std::endl
					<< "yDim = " << yDim << std::endl
					<< "followProgress = " << (followProgress?"true":"false") << std::endl
					<< std::endl;
		}
	} catch (...) {
		std::cout << "Error parsing the configuration file " << configFile
				<< std::endl;
		return false;
	}

	return true;
}

/************************************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */
