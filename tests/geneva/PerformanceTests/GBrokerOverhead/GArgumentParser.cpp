/**
 * @file GArgumentParser.cpp
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


#include "GArgumentParser.hpp"

using namespace Gem::Geneva;

namespace Gem {
namespace Tests {
/************************************************************************************************/
/**
	* A function that parses the command line for all required parameters
	*/
bool parseCommandLine(int argc, char **argv,
							 std::string &configFile,
							 std::uint16_t &parallelizationMode) {
	try {
		// Check the command line options. Uses the Boost program options library.
		po::options_description desc("Usage: evaluator [options]");
		desc.add_options()
			("help,h", "emit help message")
			("configFile,c", po::value<std::string>(&configFile)->default_value(DEFAULTCONFIGFILE),
			 "The name of the configuration file holding further configuration options")
			("parallelizationMode,p",
			 po::value<std::uint16_t>(&parallelizationMode)->default_value(DEFAULTPARALLELIZATIONMODE),
			 "Whether or not to run this optimization in serial mode (0), multi-threaded (1) or mt-consumer (2) mode");

		po::variables_map vm;
		po::store(po::parse_command_line(argc, argv, desc), vm);
		po::notify(vm);

		// Emit a help message, if necessary
		if (vm.count("help")) {
			glogger
			<< desc << std::endl
			<< GSTDERR;
			return false;
		}

		if (vm.count("parallelizationMode")) {
			if (parallelizationMode > 2) {
				glogger
				<< "Error: the \"-p\" or \"--parallelizationMode\" option may only assume the" << std::endl
				<< "values 0 (serial), 1 (multi-threaded) or 2 (mt-consumer). Leaving ..." << std::endl
				<< GWARNING;
				return false;
			}
		}

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

			default:
				glogger
				<< "Unkown parallelization mode " << parallelizationMode << std::endl
				<< GEXCEPTION;
				break;
		};

		std::cout
		<< std::endl
		<< "Running with the following command line options:" << std::endl
		<< "configFile = " << configFile << std::endl
		<< "parallelizationMode = " << parModeString << std::endl
		<< std::endl;
	}
	catch (...) {
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
	const std::string &configFile, std::uint16_t &nProducerThreads, std::uint16_t &nEvaluationThreads,
	std::size_t &populationSize, std::size_t &nParents, std::uint32_t &maxIterations, long &maxMinutes,
	std::uint32_t &reportIteration, duplicationScheme &rScheme, sortingMode &smode, std::uint32_t &nProcessingUnits,
	double &adProb, std::uint32_t &adaptionThreshold, double &sigma, double &sigmaSigma, double &minSigma,
	double &maxSigma, std::size_t &parDim, double &minVar, double &maxVar, solverFunction &df
) {
	std::uint16_t recombinationScheme = 0;
	std::uint16_t evalFunction = 0;
	bool verbose = true;

	// Check the name of the configuation file
	if (configFile.empty() || configFile == "empty" || configFile == "unknown") {
		glogger
		<< "Error: Invalid configuration file name given: \"" << configFile << "\"" << std::endl
		<< GWARNING;
		return false;
	}

	try {
		// Check the configuration file line options. Uses the Boost program options library.
		po::options_description config("Allowed options");
		config.add_options()
			("nProducerThreads", po::value<std::uint16_t>(&nProducerThreads)->default_value(DEFAULTNPRODUCERTHREADS),
			 "The amount of random number producer threads")
			("nEvaluationThreads",
			 po::value<std::uint16_t>(&nEvaluationThreads)->default_value(DEFAULTNEVALUATIONTHREADS),
			 "The amount of threads processing individuals simultaneously")
			("populationSize", po::value<std::size_t>(&populationSize)->default_value(DEFAULTPOPULATIONSIZE),
			 "The size of the super-population")
			("nParents", po::value<std::size_t>(&nParents)->default_value(DEFAULTNPARENTS),
			 "The number of parents in the population") // Needs to be treated separately
			("maxIterations", po::value<std::uint32_t>(&maxIterations)->default_value(DEFAULTMAXITERATIONS),
			 "Maximum number of iterations in the population")
			("maxMinutes", po::value<long>(&maxMinutes)->default_value(DEFAULTMAXMINUTES),
			 "The maximum number of minutes the optimization of the population should run")
			("reportIteration", po::value<std::uint32_t>(&reportIteration)->default_value(DEFAULTREPORTITERATION),
			 "The number of iterations after which information should be emitted in the super-population")
			("rScheme", po::value<std::uint16_t>(&recombinationScheme)->default_value(DEFAULTRSCHEME),
			 "The recombination scheme for the super-population")
			("sortingScheme,o", po::value<sortingMode>(&smode)->default_value(DEFAULTSORTINGSCHEME),
			 "Determines whether sorting is done in MUCOMMANU_SINGLEEVAL (0), MUPLUSNU_SINGLEEVAL (1) or MUNU1PRETAIN (2) mode")
			("verbose", po::value<bool>(&verbose)->default_value(DEFAULTVERBOSE),
			 "Whether additional information should be emitted")
			("nProcessingUnits", po::value<std::uint32_t>(&nProcessingUnits)->default_value(DEFAULTGBTCNPROCUNITS),
			 "Specifies how many processing units are available in networked mode")
			("adProb", po::value<double>(&adProb)->default_value(DEFAULTGDAADPROB),
			 "Specifies the likelihood for adaptions to be actually carried out")
			("adaptionThreshold", po::value<std::uint32_t>(&adaptionThreshold)->default_value(DEFAULTADAPTIONTHRESHOLD),
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
			("evalFunction", po::value<std::uint16_t>(&evalFunction),
			 "The id of the evaluation function");

		po::variables_map vm;
		boost::filesystem::ifstream ifs(configFile);
		if (!ifs) {
			glogger
			<< "Error accessing configuration file " << configFile
			<< GWARNING;
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
			glogger
			<< "Error: Invalid number of parents inpopulation" << std::endl
			<< "nParents       = " << nParents << std::endl
			<< "populationSize = " << populationSize << std::endl
			<< GWARNING;

			return false;
		}

		// Workaround for assigment problem with rScheme
		if (recombinationScheme == (std::uint16_t) duplicationScheme::VALUEDUPLICATIONSCHEME)
			rScheme = duplicationScheme::VALUEDUPLICATIONSCHEME;
		else if (recombinationScheme == (std::uint16_t) duplicationScheme::RANDOMDUPLICATIONSCHEME)
			rScheme = duplicationScheme::RANDOMDUPLICATIONSCHEME;
		else if (recombinationScheme == (std::uint16_t) duplicationScheme::DEFAULTDUPLICATIONSCHEME)
			rScheme = duplicationScheme::DEFAULTDUPLICATIONSCHEME;
		else {
			glogger
			<< "Error: Invalid recombination scheme in population: " << recombinationScheme << std::endl
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

		if (verbose) {
			std::cout << std::endl
			<< "Running with the following options from " << configFile << ":" << std::endl
			<< "nProducerThreads = " << (std::uint16_t) nProducerThreads <<
			std::endl // std::uint8_t not printable on gcc ???
			<< "populationSize = " << populationSize << std::endl
			<< "nParents = " << nParents << std::endl
			<< "maxIterations = " << maxIterations << std::endl
			<< "maxMinutes = " << maxMinutes << std::endl
			<< "reportIteration = " << reportIteration << std::endl
			<< "rScheme = " << (std::uint16_t) rScheme << std::endl
			<< "sortingScheme = " << smode << std::endl
			<< "nProcessingUnits = " << nProcessingUnits << std::endl
			<< "adProb = " << adProb << std::endl
			<< "adaptionThreshold = " << adaptionThreshold << std::endl
			<< "sigma = " << sigma << std::endl
			<< "sigmaSigma " << sigmaSigma << std::endl
			<< "minSigma " << minSigma << std::endl
			<< "maxSigma " << maxSigma << std::endl
			<< "parDim = " << parDim << std::endl
			<< "minVar = " << minVar << std::endl
			<< "maxVar = " << maxVar << std::endl
			<< "evalFunction = " << df << std::endl
			<< std::endl;
		}
	}
	catch (...) {
		std::cout << "Error parsing the configuration file " << configFile << std::endl;
		return false;
	}

	return true;
}

/************************************************************************************************/

} /* namespace Tests */
} /* namespace Gem */
