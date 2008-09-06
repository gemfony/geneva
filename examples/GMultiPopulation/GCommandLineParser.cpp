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
					  std::size_t& nConsumerThreads,
					  std::size_t& nSuperThreads,
					  std::size_t& superPopulationSize,
					  std::size_t& superNParents,
					  std::size_t& subPopulationSize,
					  std::size_t& subNParents,
					  boost::uint32_t& superMaxGenerations,
					  boost::uint32_t& subMaxGenerations,
					  long& superMaxMinutes,
					  long& subMaxMinutes,
					  boost::uint32_t& superReportGeneration,
					  boost::uint32_t& subReportGeneration,
					  recoScheme& superRScheme,
					  recoScheme& subRScheme,
					  bool& verbose)
{
	boost::uint16_t superRecombinationScheme=0, subRecombinationScheme=0;

	try{
		// Check the command line options. Uses the Boost program options library.
		po::options_description desc("Allowed options");
		desc.add_options()
			("help,h", "emit help message")
			("parabolaDimension,d", po::value<std::size_t>(&parabolaDimension)->default_value(DEFAULTPARABOLADIMENSION),
					"number of dimensions in the parabola")
			("parabolaMin,m", po::value<double>(&parabolaMin)->default_value(DEFAULTPARABOLAMIN),
					"Lower boundary for random numbers")
			("parabolaMax,M", po::value<double>(&parabolaMax)->default_value(DEFAULTPARABOLAMAX),
					"Upper boundary for random numbers")
			("adaptionThreshold,a", po::value<boost::uint32_t>(&adaptionThreshold)->default_value(DEFAULTADAPTIONTHRESHOLD),
					"Number of calls to mutate after which mutation parameters should be adapted")
			("nProducerThreads,n",po::value<boost::uint16_t>(&nProducerThreads)->default_value(DEFAULTNPRODUCERTHREADS),
					"The amount of random number producer threads")
			("nConsumerThreads,t",po::value<std::size_t>(&nConsumerThreads)->default_value(DEFAULTCONSUMERTHREADS),
					"The amount of consumer threads")
			("nSuperThreads,T",po::value<std::size_t>(&nSuperThreads)->default_value(DEFAULTNSUPERTHREADS),
					"The amount of threads in the super population")
			("superPopulationSize,S",po::value<std::size_t>(&superPopulationSize)->default_value(DEFAULTSUPERPOPULATIONSIZE),
					"The size of the super-population")
			("subPopulationSize,s",po::value<std::size_t>(&subPopulationSize)->default_value(DEFAULTSUBPOPULATIONSIZE),
					"The size of the sub-population")
			("superNParents,P",po::value<std::size_t>(&superNParents)->default_value(DEFAULTSUPERNPARENTS),
					"The number of parents in the super-population") // Needs to be treated separately
			("subNParents,p",po::value<std::size_t>(&subNParents)->default_value(DEFAULTSUBNPARENTS),
					"The number of parents in the sub-population") // Needs to be treated separately
			("superMaxGenerations,G", po::value<boost::uint32_t>(&superMaxGenerations)->default_value(DEFAULTSUPERMAXGENERATIONS),
					"maximum number of generations in the super-population")
			("subMaxGenerations,g", po::value<boost::uint32_t>(&subMaxGenerations)->default_value(DEFAULTSUBMAXGENERATIONS),
					"maximum number of generations in the sub-population")
			("superMaxMinutes,X", po::value<long>(&superMaxMinutes)->default_value(DEFAULTSUPERMAXMINUTES),
					"The maximum number of minutes the optimization of the super-population should run")
			("subMaxMinutes,x", po::value<long>(&subMaxMinutes)->default_value(DEFAULTSUBMAXMINUTES),
					"The maximum number of minutes the optimization of the sub-population should run")
			("superReportGeneration,R",po::value<boost::uint32_t>(&superReportGeneration)->default_value(DEFAULTSUPERREPORTGENERATION),
					"The number of generations after which information should be emitted in the super-population")
			("subReportGeneration,r",po::value<boost::uint32_t>(&subReportGeneration)->default_value(DEFAULTSUBREPORTGENERATION),
					"The number of generations after which information should be emitted in the sub-population")
			("superRScheme,E",po::value<boost::uint16_t>(&superRecombinationScheme)->default_value(DEFAULTSUPERRSCHEME),
					"The recombination scheme for the super-population")
			("subRScheme,e",po::value<boost::uint16_t>(&subRecombinationScheme)->default_value(DEFAULTSUBRSCHEME),
					"The recombination scheme for the sub-population")
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
		if(2*superNParents > superPopulationSize){
			std::cout << "Error: Invalid number of parents in super-population" << std::endl
				      << "superNParents       = " << superNParents << std::endl
				      << "superPopulationSize = " << superPopulationSize << std::endl;

			return false;
		}

		// Check the number of parents in the sub-population
		if(2*subNParents > subPopulationSize){
			std::cout << "Error: Invalid number of parents in sub-population" << std::endl
				      << "subNParents       = " << subNParents << std::endl
				      << "subPopulationSize = " << subPopulationSize << std::endl;

			return false;
		}

		// Check the parabolaMin/Max parameters
		if(parabolaMin >= parabolaMax){
			std::cout << "Error: Invalid parabolaMin/Max parameters" << std::endl
				      << "parabolaMin = " << parabolaMin << std::endl
				      << "parabolaMax = " << parabolaMax << std::endl;

			return false;;
		}

		// Workaround for assigment problem with superRScheme
		if(superRecombinationScheme==(boost::uint16_t)VALUERECOMBINE)
			superRScheme=VALUERECOMBINE;
		else if(superRecombinationScheme==(boost::uint16_t)RANDOMRECOMBINE)
			superRScheme=RANDOMRECOMBINE;
		else if(superRecombinationScheme==(boost::uint16_t)DEFAULTRECOMBINE)
			superRScheme=DEFAULTRECOMBINE;
		else {
			std::cout << "Error: Invalid recombination scheme in super population: " << superRecombinationScheme << std::endl;
			return false;
		}

		// Workaround for assigment problem with subRScheme
		if(subRecombinationScheme==(boost::uint16_t)VALUERECOMBINE)
			subRScheme=VALUERECOMBINE;
		else if(subRecombinationScheme==(boost::uint16_t)RANDOMRECOMBINE)
			subRScheme=RANDOMRECOMBINE;
		else if(subRecombinationScheme==(boost::uint16_t)DEFAULTRECOMBINE)
			subRScheme=DEFAULTRECOMBINE;
		else {
			std::cout << "Error: Invalid recombination scheme in sub population: " << subRecombinationScheme << std::endl;
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
					  << "nConsumerThreads = " << nConsumerThreads << std::endl
					  << "superPopulationSize = " << superPopulationSize << std::endl
					  << "subPopulationSize = " << subPopulationSize << std::endl
					  << "superNParents = " << superNParents << std::endl
					  << "subNParents = " << subNParents << std::endl
					  << "superMaxGenerations = " << superMaxGenerations << std::endl
					  << "subMaxGenerations = " << subMaxGenerations << std::endl
					  << "superMaxMinutes = " << superMaxMinutes << std::endl
					  << "subMaxMinutes = " << subMaxMinutes << std::endl
					  << "superReportGeneration = " << superReportGeneration << std::endl
					  << "subReportGeneration = " << subReportGeneration << std::endl
					  << "superRScheme = " << (boost::uint16_t)superRScheme << std::endl
					  << "subRScheme = " << (boost::uint16_t)subRScheme << std::endl
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
