/**
  * @file GCommandLineParser.cpp
  */

/* Copyright (C) 2009 Dr. Ruediger Berlich
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
					  std::string& className,
					  bool& verbose)
{
	boost::uint16_t recombinationScheme=0;

	try{
		// Check the command line options. Uses the Boost program options library.
		po::options_description desc("Allowed options");
		desc.add_options()
			("help,h", "emit help message")
			("className,C",po::value<std::string>(&className)->default_value(DEFAULTCLASSNAME),
					"the name of a file holding the evaluation executable")
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

		if(vm.count("className") == 0) {
			std::cerr << "Warning:" << std::endl
				           << "It is recommended to provide a class name on the command line, using the \"-C\" switch" << std::endl
				           << "Using \"" << DEFAULTCLASSNAME << "\" as class name instead." << std::endl;
		}

		if(verbose){
			std::cout << std::endl
				      << "Running with the following options:" << std::endl
				      << "className = " << className << std::endl
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
