/**
 * @file commandLineParser.cpp
 */

/* Copyright (C) 2009 Dr. Ruediger Berlich
 * Copyright (C) 2009 Forschungszentrum Karlsruhe GmbH
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
			  bool& init,
			  bool& finalize,
			  std::string& paramfile,
		      bool& templ,
		      bool& result)
{
  try{
    // Check the command line options. Uses the Boost program options library.
    po::options_description desc("Usage: evaluator [options]");
    desc.add_options()
      ("help,h", "Emit help message")
      ("initialize,i", "Perform necessary initial tasks. Other options will be ignored.")
      ("finalize,f", "Perform any final actions. Other options will be ignored.")
      ("paramfile,p",po::value<std::string>(&paramfile)->default_value(DEFAULTPARAMFILE),
    		  "Name of a file with the parameters")
      ("template,t", "Write out a template for this population. Requires option \"-p\"")
      ("result,r", "Write out a result file for a given parameter set. Requires option \"-p\"")
      ;

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    // Initialize the reference arguments
	init = false;
	finalize = false;
	templ = false;
	result = false;

    // Emit a help message, if necessary
    if (vm.count("help")) {
      std::cerr << desc << std::endl;

      return false;
    }

    // Check whether we have been asked to perform initial work.
    // If found, all other options will be ignored
    if(vm.count("initialize")) {
    	init = true;
    	return true;
    }

    // Check whether we have been asked to perform final tasks
    // If found, all other options will be ignored
    if(vm.count("finalize")) {
    	finalize = true;
    	return true;
    }

    if(vm.count("paramfile")) {
    	if(paramfile == DEFAULTPARAMFILE) {
    	      std::cout << "Error: You need to specify the name of the parameter file (option \"-p\")." << std::endl
					    << "Make sure it is not \"unknown\"" << std::endl;
    	      return false;
		}

    	if(vm.count("template")) { // option result will be ignored
    		templ = true;
    		return true;
    	}

    	if(vm.count("result")) {
    		result = true;
    		return true;
    	}

		// Our duty is to evaluate the content of the parameter file
    	return true;
    }


  }
  catch(...){
    std::cout << "Error parsing the command line" << std::endl;
    return false;
  }

  return true; // make the compiler happy
}

/************************************************************************************************/
