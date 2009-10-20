/**
 * @file GCommandLineParser.cpp
 */

/* Copyright (C) 2004-2008 Dr. Ruediger Berlich
 * Copyright (C) 2008 Forschungszentrum Karlsruhe GmbH
 *
 * This file is part of the Geneva library.
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
 */

#include "GCommandLineParser.hpp"

namespace Gem
{
namespace GenEvA
{

/************************************************************************************************/
/**
 * A function that parses the command line for all required parameters
 */
bool parseCommandLine(int argc, char **argv,
					  std::size_t& parabolaDimension,
					  double& parabolaMin,
					  double& parabolaMax,
					  boost::uint32_t& adaptionThreshold,
					  boost::uint16_t& nProducerThreads,
					  boost::uint16_t& nEvaluationThreads,
					  std::size_t& populationSize,
					  std::size_t& nParents,
					  boost::uint32_t& maxGenerations,
					  long& maxMinutes,
					  boost::uint32_t& reportGeneration,
					  recoScheme& rScheme,
					  boost::uint16_t& parallelizationMode,
					  bool& serverMode,
					  std::string& ip,
					  unsigned short& port,
					  std::size_t& arraySize,
					  bool& productionPlace,
					  bool& useCommonAdaptor,
					  double& sigma,
					  double& sigmaSigma,
					  double& minSigma,
					  double& maxSigma,
					  bool& verbose)
{
	boost::uint16_t recombinationScheme=0;

	try{
		// Check the command line options. Uses the Boost program options library.
		po::options_description desc("Allowed options");
		desc.add_options()
			("help,h", "emit help message")
			("parabolaDimension,d", po::value<std::size_t>(&parabolaDimension)->default_value(DEFAULTPARABOLADIMENSION),
					"number of dimensions in the parabola")
			("parabolaMin,m", po::value<double>(&parabolaMin)->default_value(DEFAULTPARABOLAMIN),
					"Lower boundary for random numbers")
			("adaptionThreshold,a", po::value<boost::uint32_t>(&adaptionThreshold)->default_value(DEFAULTADAPTIONTHRESHOLD),
					"Number of calls to mutate after which mutation parameters should be adapted")
			("parabolaMax,M", po::value<double>(&parabolaMax)->default_value(DEFAULTPARABOLAMAX),
					"Upper boundary for random numbers")
			("nProducerThreads,n",po::value<boost::uint16_t>(&nProducerThreads)->default_value(DEFAULTNPRODUCERTHREADS),
					"The amount of random number producer threads")
			("nEvaluationThreads,N",po::value<boost::uint16_t>(&nEvaluationThreads)->default_value(DEFAULTNEVALUATIONTHREADS),
					"The amount of threads processing individuals simultaneously")
			("populationSize,S",po::value<std::size_t>(&populationSize)->default_value(DEFAULTPOPULATIONSIZE),
					"The size of the super-population")
			("nParents,P",po::value<std::size_t>(&nParents)->default_value(DEFAULTNPARENTS),
					"The number of parents in the population") // Needs to be treated separately
			("maxGenerations,G", po::value<boost::uint32_t>(&maxGenerations)->default_value(DEFAULTMAXGENERATIONS),
					"maximum number of generations in the population")
			("maxMinutes,X", po::value<long>(&maxMinutes)->default_value(DEFAULTMAXMINUTES),
					"The maximum number of minutes the optimization of the population should run")
			("reportGeneration,R",po::value<boost::uint32_t>(&reportGeneration)->default_value(DEFAULTREPORTGENERATION),
					"The number of generations after which information should be emitted in the super-population")
			("rScheme,E",po::value<boost::uint16_t>(&recombinationScheme)->default_value(DEFAULTRSCHEME),
					"The recombination scheme for the super-population")
			("parallelizationMode,p", po::value<boost::uint16_t>(&parallelizationMode)->default_value(DEFAULTPARALLELIZATIONMODE),
			                "Whether or not to run this optimization in serial mode (0), multi-threaded (1) or networked (2) mode")
			("serverMode","Whether to run networked execution in server or client mode. The option only gets evaluated if \"--parallelizationMode=2\"")
			("ip",po::value<std::string>(&ip)->default_value(DEFAULTIP), "The ip of the server")
			("port",po::value<unsigned short>(&port)->default_value(DEFAULTPORT), "The port of the server")
			("arraySize,A", po::value<std::size_t>(&arraySize)->default_value(DEFAULTARRAYSIZE),
					"The size of the buffer with random arrays in the random factory")
		    ("productionPlace,D", po::value<bool>(&productionPlace)->default_value(DEFAULTPRODUCTIONPLACE),
		    		"Whether production of random numbers in individuals should happen locally (0) or in the random number factory (1)")
		    ("useCommonAdaptor,u", po::value<bool>(&useCommonAdaptor)->default_value(DEFAULTUSECOMMONADAPTOR),
		    		"Specifies whether a common adaptor should be used for all GParameterT objects")
		    ("sigma", po::value<double>(&sigma)->default_value(DEFAULTSIGMA),
		    		"The width of the gaussian used for the adaption of double values")
		    ("sigmaSigma", po::value<double>(&sigmaSigma)->default_value(DEFAULTSIGMASIGMA),
		    		"The adaption rate of sigma")
		    ("minSigma", po::value<double>(&minSigma)->default_value(DEFAULTMINSIGMA),
		    		"The minimum allowed value for sigma")
		    ("maxSigma", po::value<double>(&maxSigma)->default_value(DEFAULTMAXSIGMA),
		    		"The maximum allowed value for sigma")
			("verbose,v",po::value<bool>(&verbose)->default_value(DEFAULTVERBOSE),
					"Whether additional information should be emitted")
		;

		po::variables_map vm;
		po::store(po::parse_command_line(argc, argv, desc), vm);
		po::notify(vm);

		// Emit a help message, if necessary
		if (vm.count("help")) {
			 std::cout << desc << std::endl;
			 return false;
		}

		// Check the number of parents in the super-population
		if(2*nParents > populationSize){
			std::cout << "Error: Invalid number of parents inpopulation" << std::endl
				      << "nParents       = " << nParents << std::endl
				      << "populationSize = " << populationSize << std::endl;

			return false;
		}

		// Check the parabolaMin/Max parameters
		if(parabolaMin >= parabolaMax){
			std::cout << "Error: Invalid parabolaMin/Max parameters" << std::endl
				      << "parabolaMin = " << parabolaMin << std::endl
				      << "parabolaMax = " << parabolaMax << std::endl;

			return false;;
		}

		// Workaround for assigment problem with rScheme
		if(recombinationScheme==(boost::uint16_t)VALUERECOMBINE)
			rScheme=VALUERECOMBINE;
		else if(recombinationScheme==(boost::uint16_t)RANDOMRECOMBINE)
			rScheme=RANDOMRECOMBINE;
		else if(recombinationScheme==(boost::uint16_t)DEFAULTRECOMBINE)
			rScheme=DEFAULTRECOMBINE;
		else {
			std::cout << "Error: Invalid recombination scheme in population: " << recombinationScheme << std::endl;
			return false;
		}

		serverMode=false;
		if (vm.count("parallelizationMode")) {
			if(parallelizationMode > 2) {
				std::cout << "Error: the \"-p\" or \"--parallelizationMode\" option may only assume the"<< std::endl
						  << "values 0 (serial), 1 (multi-threaded) or 2 (networked). Leaving ..." << std::endl;
				return false;
			}

			if(parallelizationMode == 2) if(vm.count("serverMode")) serverMode = true;
		}


		if(verbose){
			std::string parModeString;
			switch(parallelizationMode) {
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
				      << "Running with the following options:" << std::endl
					  << "parabolaDimension = " << parabolaDimension << std::endl
					  << "parabolaMin = " << parabolaMin << std::endl
					  << "parabolaMax = " << parabolaMax << std::endl
					  << "adaptionThreshold = " << adaptionThreshold << std::endl
					  << "nProducerThreads = " << (boost::uint16_t)nProducerThreads << std::endl // boost::uint8_t not printable on gcc ???
					  << "populationSize = " << populationSize << std::endl
					  << "nParents = " << nParents << std::endl
					  << "maxGenerations = " << maxGenerations << std::endl
					  << "maxMinutes = " << maxMinutes << std::endl
					  << "reportGeneration = " << reportGeneration << std::endl
					  << "rScheme = " << (boost::uint16_t)rScheme << std::endl
					  << "parallelizationMode = " << parModeString << std::endl
					  << "serverMode = " << (serverMode?"true":"false") << std::endl
					  << "ip = " << ip << std::endl
					  << "port = " << port << std::endl
					  << "arraySize = " << arraySize << std::endl
					  << "productionPlace = " << (productionPlace?"factory":"locally") << std::endl
					  << "useCommonAdaptor = " << (useCommonAdaptor?"joint adaptor":"individual adaptor") << std::endl
					  << "sigma = " << sigma << std::endl
					  << "sigmaSigma " << sigmaSigma << std::endl
					  << "minSigma " << minSigma << std::endl
					  << "maxSigma " << maxSigma << std::endl
					  << std::endl;
		}
	}
	catch(...){
		std::cout << "Error parsing the command line" << std::endl;
		return false;
	}

	return true;
}

/************************************************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */
