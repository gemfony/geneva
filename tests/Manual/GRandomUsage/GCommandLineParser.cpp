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
					  std::size_t& nEntries,
					  boost::uint16_t& nProducerThreads,
					  boost::uint16_t& rnrProductionMode,
					  bool& verbose)
{
	try{
		// Check the command line options. Uses the Boost program options library.
		po::options_description desc("Allowed options");
		desc.add_options()
			("help,h", "emit help message")
			("nEntries,n",po::value<std::size_t>(&nEntries)->default_value(DEFAULTNENTRIES),
					"Number of random numbers to generate for each distribution")
			("nProducerThreads,t",po::value<boost::uint16_t>(&nProducerThreads)->default_value(DEFAULTNPRODUCERTHREADS),
					"The amount of random number producer threads")
			("rnrProductionMode,r",po::value<boost::uint16_t>(&rnrProductionMode)->default_value(DEFAULTRNRPRODUCTIONMODE),
					"FACTORY(0), PROXY(1) or LOCAL(2)")
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

		if(rnrProductionMode > 2) {
			std::cerr << "Got invalid random number production mode: " << rnrProductionMode << std::endl;
			return false;
		}

		if(verbose){
			std::string rnrProductionModeString;
			if(rnrProductionMode == 0) rnrProductionModeString = "Factory";
			else if(rnrProductionMode == 1) rnrProductionModeString = "Proxy";
			else if(rnrProductionMode == 2) rnrProductionModeString = "Local";

			std::cout << std::endl
				      << "Running with the following options:" << std::endl
				      << "nEntries = " << nEntries << std::endl
					  << "nProducerThreads = " << nProducerThreads << std::endl
					  << "rnrProductionMode = " << rnrProductionModeString << std::endl
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
