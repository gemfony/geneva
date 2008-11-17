/**
 * @file commandLineParser.cpp
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

#include "commandLineParser.hpp"

/************************************************************************************************/
/**
 * A function that parses the command line for all required parameters
 */
bool parseCommandLine(int argc, char **argv,
		      std::string& paramfile,
		      bool& writeTemplate,
		      bool& verbose)
{
  try{
    // Check the command line options. Uses the Boost program options library.
    po::options_description desc("Usage: eminim [options]");
    desc.add_options()
      ("help,h", "emit help message")
      ("paramfile,p",po::value<std::string>(&paramfile)->default_value(DEFAULTPARAMFILE),
       "Name of a file with the parameters")
      ("writeTemplate,t",po::value<bool>(&writeTemplate)->default_value(DEFAULTWRITETEMPLATE),
       "Writes out a template parameter file. Requires option \"-p\"")
      ("writeResult,r",po::value<bool>(&writeResult)->default_value(DEFAULTWRITERESULT),
       "Writes out the current parameter set in a user-defined way. Requires option \"-p\"")
      ("verbose,v",po::value<bool>(&verbose)->default_value(DEFAULTVERBOSE),
       "Whether to emit the command line options")
      ;

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    // Emit a help message, if necessary
    if (vm.count("help")) {
      std::cerr << desc << std::endl;
      return false;
    }

    // We can only write a template if we have received a pdbfile
    if(paramfile == DEFAULTPARAMFILE) {
      std::cout << "Error: You need to specify the name of the parameter file (option \"-p\")" << std::endl;
      return false;
    }

    if(verbose) {
      std::cout << "paramfile = " << paramfile << std::endl
	            << "writeTemplate = " << writeTemplate << std::endl
	            << "writeResult = " << writeResult << std::endl;
    }
  }
  catch(...){
    std::cout << "Error parsing the command line" << std::endl;
    return false;
  }

  return true;
}

/************************************************************************************************/
