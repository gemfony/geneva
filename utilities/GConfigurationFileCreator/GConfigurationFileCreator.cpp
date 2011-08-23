/**
 * @file GConfigurationFileCreator.cpp
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt)
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) com
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
 * http://www.gemfony.com .
 */

// Standard header files go here
#include <string>

// Boost header files go here
#include <boost/filesystem.hpp>

// Geneva header files go here
#include "common/GExceptions.hpp"
#include "geneva/GEvolutionaryAlgorithmFactory.hpp"

/**************************************************************************************/
/**
 *
 * @param argc The number of command line arguments
 * @param argv An array with the arguments
 */
void parseCommandLine(
		int argc
		, char **argv
		, unsigned int& target
		, std::string& directory
) {
    namespace po = boost::program_options;

	try {
		std::string usageString = std::string("Usage: ") + argv[0] + " [options]";
		po::options_description desc(usageString.c_str());
		desc.add_options()
				("help,h", "emit help message")
				("target,t", po::value<unsigned int>(&target)->default_value(0),
				"The id of the Geneva object to be created: (0) Evolutionary Algorithms (1) Swarm Algorithms (2)"
				"Gradient Descents")
				("directory,d", po::value<std::string>(&directory)->default_value(std::string("./config/")),
				"The name of the directory to which results should be written")

		;

		po::variables_map vm;
		po::store(po::parse_command_line(argc, argv, desc), vm);
		po::notify(vm);

		// Emit a help message, if necessary
		if (vm.count("help")) {
			std::cout << desc << std::endl;
			exit(0);
		}
	}
	catch(const po::error& e) {
		std::cerr << "Error parsing the command line:" << std::endl
				  << e.what() << std::endl;
		exit(1);
	}
	catch(...) {
		std::cerr << "Unknown error while parsing the command line" << std::endl;
		exit(1);
	}
}


/**************************************************************************************/
/**
 * This application creates a set of configuration files on demand
 */
int main(int argc, char **argv) {
	using namespace Gem::Geneva;

	unsigned int target;
	std::string directory;

	// Parse the command line
	parseCommandLine(
		argc
		, argv
		, target
		, directory
	);

	// Check that the target directory exists. If not, create it
	{
		using namespace boost::filesystem;

		if(exists(directory) && !is_directory(directory)){
			raiseException(
				"In GConfigurationFileCreator: Error!" << std::endl
				<< directory << " exists, but is no directory." << std::endl
			);
		}

		if(!exists(directory)) {
			create_directory(directory);
		}
	}

	// Emit the configuration file
	switch(target) {
	case 0:
		{
			std::string header;
			header += "Configuration file for evolutionary algorithms;";
			header += "Created using the GEvolutionaryAlgorithmFactory;";
			GEvolutionaryAlgorithmFactory gevaf(directory+"/GEvolutionaryAlgorithm.cfg", PARMODE_SERIAL);
			gevaf.writeConfigFile(header);
		}
		break;
	}
}
