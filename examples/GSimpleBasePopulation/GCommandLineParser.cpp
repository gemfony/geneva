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
					  std::size_t& populationSize,
					  std::size_t& nParents,
					  boost::uint32_t& maxGenerations,
					  boost::uint32_t& maxStallGenerations,
					  double &qualityThreshold,
					  long& maxMinutes,
					  boost::uint32_t& reportGeneration,
					  recoScheme& rScheme,
					  sortingMode& smode,
					  bool& parallel,
					  bool& maximize,
					  std::size_t& arraySize,
					  bool& productionPlace,
					  double& mutProb,
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
			("populationSize,S",po::value<std::size_t>(&populationSize)->default_value(DEFAULTPOPULATIONSIZE),
					"The size of the super-population")
			("nParents,P",po::value<std::size_t>(&nParents)->default_value(DEFAULTNPARENTS),
					"The number of parents in the population") // Needs to be treated separately
			("maxGenerations,G", po::value<boost::uint32_t>(&maxGenerations)->default_value(DEFAULTMAXGENERATIONS),
					"maximum number of generations in the population")
			("maxStallGenerations,Y", po::value<boost::uint32_t>(&maxStallGenerations)->default_value(DEFAULTMAXSTALLGENERATIONS),
					"maximum number of generations without improvement in the population")
			("qualityThreshold,q", po::value<double>(&qualityThreshold)->default_value(DEFAULTQTHRESHOLD),
					"A threshold beyond which optimization is supposed to stop")
			("maxMinutes,X", po::value<long>(&maxMinutes)->default_value(DEFAULTMAXMINUTES),
					"The maximum number of minutes the optimization of the population should run")
			("reportGeneration,R",po::value<boost::uint32_t>(&reportGeneration)->default_value(DEFAULTREPORTGENERATION),
					"The number of generations after which information should be emitted in the super-population")
			("rScheme,E",po::value<boost::uint16_t>(&recombinationScheme)->default_value(DEFAULTRSCHEME),
					"The recombination scheme for the super-population")
			("sortingMode,o", po::value<sortingMode>(&smode)->default_value(DEFAULTSORTINGSCHEME),
					"Determines the sorting scheme being used")
			("parallel,p", po::value<bool>(&parallel)->default_value(DEFAULTPARALLEL),
			        "Whether or not to run this optimization in multi-threaded mode")
			("maximize,z", po::value<bool>(&maximize)->default_value(DEFAULTMAXIMIZE),
					"Whether or not to run this optimization in multi-threaded mode")
			("arraySize,A", po::value<std::size_t>(&arraySize)->default_value(DEFAULTARRAYSIZE),
					"The size of the buffer with random arrays in the random factory")
            ("productionPlace,D", po::value<bool>(&productionPlace)->default_value(DEFAULTPRODUCTIONPLACE),
            		"Whether production of random numbers in individuals should happen locally (0) or in the random number factory (1)")
            ("mutProb", po::value<double>(&mutProb)->default_value(DEFAULTGDAMUTPROB),
            		"Specifies the likelihood for mutations to be actually carried out")
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

		if(verbose){
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
					  << "maxStallGenerations = " << maxStallGenerations << std::endl
					  << "maxMinutes = " << maxMinutes << std::endl
					  << "reportGeneration = " << reportGeneration << std::endl
					  << "rScheme = " << (boost::uint16_t)rScheme << std::endl
					  << "sortingMode = " << smode << std::endl
					  << "maximize = " << maximize << std::endl
					  << "arraySize = " << arraySize << std::endl
					  << "productionPlace = " << (productionPlace?"factory":"locally") << std::endl
					  << "mutProb = " << mutProb << std::endl
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
