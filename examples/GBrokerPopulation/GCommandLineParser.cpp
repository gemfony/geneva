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
					  std::string& mode,
					  unsigned short port,
					  std::string& ip,
					  boost::uint32_t& maxGenerations)
{
	try{
		// Check the command line options. Uses the Boost.Programoptions library.
		po::options_description desc("Allowed options");
		desc.add_options()
			("help", "emit help message")
			("mode", po::value<std::string>(), "either \"server\" or \"client\" (required)")
			("port", po::value<unsigned short>(), "the server port (required)")
			("ip", po::value<std::string>(), "name or ip of the server (required for the client)")
			("generations", po::value<boost::uint32_t>(), "number of generations")
		;

		po::variables_map vm;
		po::store(po::parse_command_line(argc, argv, desc), vm);
		po::notify(vm);

		// Emit a help message, if necessary
		if (vm.count("help")) {
			 std::cout << desc << std::endl;
			 exit(1);
		}

		// Retrieve the port of the server
		if (vm.count("port") != 1) {
			std::cout << std::endl
					  << "Error: port was not set or was set more than once." << std::endl
					  << std::endl
					  << desc << std::endl;
			exit(1);
		}
		else port = vm["port"].as<unsigned short>();

		// Check whether we're running as a client or as a server
		if (vm.count("mode") != 1) {
			std::cout << std::endl
					  << "Error: mode was not set or was set more than once." << std::endl
					  << std::endl
					  << desc << std::endl;
			exit(1);
		}
		else {
			mode = vm["mode"].as<std::string>();
			if(mode != "client" && mode != "server"){
				std::cout << std::endl
						  << "Error: mode should be either \"client\" or \"server\"" << std::endl
					      << std::endl
					      << desc << std::endl;
				exit(1);
			}
		}

		// If we are a client, extract the server ip
		if (mode == "client"){
			if(vm.count("ip") != 1) {
				std::cout << std::endl
						  << "Error: server ip/name was not set or was set more than once." << std::endl
						  << std::endl
						  << desc << std::endl;
				exit(1);
			}
			else ip=vm["ip"].as<std::string>();
		}

		// Extract the maximum number of generations
		if (!vm.count("generations")) maxGenerations=DEFAULTMAXGENERATIONS;
		else if(vm.count("generations") > 1){
			std::cout << std::endl
				      << "Error: generations parameter was set more than once." << std::endl
					  << std::endl
					  << desc << std::endl;
			exit(1);
		}
		else maxGenerations=vm["generations"].as<boost::uint32_t>();
	}
	catch(...){
		return false;
	}

	return true;
}

/************************************************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */
