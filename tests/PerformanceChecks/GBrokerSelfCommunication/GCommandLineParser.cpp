/**
 * @file GCommandLineParser.cpp
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
					  std::size_t& nClients,
					  boost::uint16_t& nProducerThreads,
					  std::size_t& populationSize,
					  std::size_t& nParents,
					  boost::uint32_t& maxGenerations,
					  long& maxMinutes,
					  boost::uint32_t& reportGeneration,
					  recoScheme& rScheme,
					  Gem::Common::serializationMode& serMode,
					  bool& verbose)
{
	boost::uint16_t recombinationScheme;

	try{
		// Check the command line options. Uses the Boost program options library.
		po::options_description desc("Allowed options");
		desc.add_options()
			("help,h", "emit help message")
			("nClients,c",po::value<std::size_t>(&nClients)->default_value(DEFAULTNCLIENTS),
					"The number of clients trying to connect to the server")
			("nProducerThreads,p",po::value<boost::uint16_t>(&nProducerThreads)->default_value(DEFAULTNPRODUCERTHREADS),
					"The amount of random number producer threads")
			("populationSize,s",po::value<std::size_t>(&populationSize)->default_value(DEFAULTPOPULATIONSIZE),
					"The size of the population")
			("nParents,P",po::value<std::size_t>(&nParents)->default_value(DEFAULTNPARENTS),
					"The number of parents in the population") // Needs to be treated separately
			("maxGenerations,g", po::value<boost::uint32_t>(&maxGenerations)->default_value(DEFAULTMAXGENERATIONS),
					"maximum number of generations")
			("maxMinutes,x", po::value<long>(&maxMinutes)->default_value(DEFAULTMAXMINUTES),
					"The maximum number of minutes the optimization should run")
			("reportGeneration,G",po::value<boost::uint32_t>(&reportGeneration)->default_value(DEFAULTREPORTGENERATION),
					"The number of generations after which information should be emitted")
			("rScheme,r",po::value<boost::uint16_t>(&recombinationScheme)->default_value(DEFAULTRSCHEME),
					"The recombination scheme")
			("serMode", po::value<Gem::Common::serializationMode>(&serMode)->default_value(DEFAULTSERMODE),
					"Specifies whether serialization shall be done in TEXTMODE (0), XMLMODE (1) or BINARYMODE (2)")
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

		// Check the number of parents
		if(2*nParents > populationSize){
			std::cout << "Error: Invalid number of parents" << std::endl
				      << "nParents       = " << nParents << std::endl
				      << "populationSize = " << populationSize << std::endl;

			return false;
		}

		// Workaround for assigment problem with recoScheme
		if(recombinationScheme==(boost::uint16_t)VALUERECOMBINE)
			rScheme=VALUERECOMBINE;
		else if(recombinationScheme==(boost::uint16_t)RANDOMRECOMBINE)
			rScheme=RANDOMRECOMBINE;
		else if(recombinationScheme==(boost::uint16_t)DEFAULTRECOMBINE)
			rScheme=DEFAULTRECOMBINE;
		else {
			std::cout << "Error: Invalid recombination scheme: " << recombinationScheme << std::endl;
			return false;
		}

		if(verbose){
			std::cout << std::endl
				      << "Running with the following options:" << std::endl
					  << "nClients = " << nClients << std::endl
					  << "nProducerThreads = " << (boost::uint16_t)nProducerThreads << std::endl // boost::uint8_t not printable on gcc ???
					  << "populationSize = " << populationSize << std::endl
					  << "nParents = " << nParents << std::endl
					  << "maxGenerations = " << maxGenerations << std::endl
					  << "maxMinutes = " << maxMinutes << std::endl
					  << "reportGeneration = " << reportGeneration << std::endl
					  << "rScheme = " << (boost::uint16_t)rScheme << std::endl
					  << "serMode = " << serMode << std::endl
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
