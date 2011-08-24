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
#include "geneva/GGradientDescentFactory.hpp"
#include "geneva/GSwarmAlgorithmFactory.hpp"
#include "geneva-individuals/GFunctionIndividual.hpp"

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
		, bool& check
) {
    namespace po = boost::program_options;

	try {
		std::string usageString = std::string("Usage: ") + argv[0] + " [options]";
		po::options_description desc(usageString.c_str());
		desc.add_options()
				("help,h", "emit help message")
				("target,t", po::value<unsigned int>(&target)->default_value(0),
				"The id of the Geneva object to be created: (0) Evolutionary Algorithms (1) Swarm Algorithms (2)"
				"Gradient Descents, (3) GFunctionIndividual")
				("directory,d", po::value<std::string>(&directory)->default_value(std::string("./config/")),
				"The name of the directory to which results should be written")
				("check,c", "Check whether generated config can be read")
		;

		po::variables_map vm;
		po::store(po::parse_command_line(argc, argv, desc), vm);
		po::notify(vm);

		// Emit a help message, if necessary
		if (vm.count("help")) {
			std::cout << desc << std::endl;
			exit(0);
		}

		if(vm.count("check")) {
			check = true;
		} else {
			check = false;
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
	bool check;

	// Parse the command line
	parseCommandLine(
		argc
		, argv
		, target
		, directory
		, check
	);

	// Check that the target directory exists. If not, check it
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
			GEvolutionaryAlgorithmFactory gevaf(directory+"/GEvolutionaryAlgorithm.cfg", PARMODE_SERIAL);

			if(check) {
				boost::shared_ptr<GBaseEA> p = gevaf();
				if(!p) {
					raiseException(
						"In GConfigurationFileCreator: Error!" << std::endl
						<< "Could not check GBaseEA object." << std::endl
					);
				}
			} else {
				std::string header;
				header += "Configuration file for evolutionary algorithms;";
				header += "created using the GEvolutionaryAlgorithmFactory;";
				gevaf.writeConfigFile(header);
			}
		}
		break;

	case 1:
		{
			GSwarmAlgorithmFactory gsaf(directory+"/GSwarmAlgorithm.cfg", PARMODE_SERIAL);

			if(check) {
				boost::shared_ptr<GBaseSwarm> p = gsaf();
				if(!p) {
					raiseException(
						"In GConfigurationFileCreator: Error!" << std::endl
						<< "Could not check GBaseSwarm object." << std::endl
					);
				}
			} else {
				std::string header;
				header += "Configuration file for swarm algorithms;";
				header += "created using the GSwarmAlgorithmFactory;";
				gsaf.writeConfigFile(header);
			}
		}
		break;

	case 2:
		{
			GGradientDescentFactory ggdf(directory+"/GGradientDescent.cfg", PARMODE_SERIAL);

			if(check) {
				boost::shared_ptr<GBaseGD> p = ggdf();
				if(!p) {
					raiseException(
						"In GConfigurationFileCreator: Error!" << std::endl
						<< "Could not check GBaseGD object." << std::endl
					);
				}
			} else {
				std::string header;
				header += "Configuration file for gradient descents;";
				header += "created using the GGradientDescentFactory;";
				ggdf.writeConfigFile(header);
			}
		}
		break;

	case 3:
		{
			GFunctionIndividualFactory gfif(directory+"/GFunctionIndividual.cfg");

			if(check) {
				boost::shared_ptr<GFunctionIndividual> p = gfif();
				if(!p) {
					raiseException(
						"In GConfigurationFileCreator: Error!" << std::endl
						<< "Could not check GFunctionIndividual object." << std::endl
					);
				}
			} else {
				std::string header;
				header += "Configuration file for GFunctionIndividual objects;";
				header += "created using the GFunctionIndividualFactory;";
				gfif.writeConfigFile(header);
			}
		}
		break;
	};
}
