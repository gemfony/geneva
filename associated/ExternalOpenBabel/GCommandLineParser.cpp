/**
 * @file GCommandLineParser.cpp
 */

/*
 * Copyright (C) 2009 Dr. Ruediger Berlich
 *
 * This file is part of the eminim program. eminim is free software:
 * you can redistribute and/or modify this file under the terms of
 * version 2 of the GNU General Public License as published by the
 * Free Software Foundation.
 *
 * eminim is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with the Geneva library.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "GCommandLineParser.hpp"

/************************************************************************************************/
/**
 * A function that parses the command line for all required parameters
 */

bool parseCommandLine(int argc, char **argv,
		bool& init,
		bool& finalize,
		bool& singleEvaluation,
		std::string& paramfile,
		bool& templ,
		bool& randInit,
		bool& result,
		unsigned short& loglevel,
		bool& addhydrogens,
		std::string& forcefield,
		std::string& filename)
{
	// Assemble a std::string in case we need to emit information about the force field
	std::string ffinfo = "The forcefield.\nAvaible options:\n" + OpenBabel::OBPlugin::ListAsString("forcefields", "verbose");

	try{
		// Check the command line options. Uses the Boost program options library.
		po::options_description desc("Usage: eminim2 [options] -n <filename>");
		desc.add_options()
		("help,h", "emit help message")
		("initialize,i", "Perform necessary initial tasks. Other options will be ignored.")
		("finalize,f", "Perform any final actions. Other options will be ignored.")
		("singleEvaluation,s", "Perform just a single evaluation of the molecule data")
		("paramfile,p",po::value<std::string>(&paramfile)->default_value(DEFAULTPARAMFILE),
				"Name of a file with the parameters")
				("template,t", "Write out a template for this population. Requires option \"-p\"")
				("randInit,R","Randomly initialize template variables. Only useful in conjunction with option \"-t\"")
				("result,r", "Write out a result file for a given parameter set. Requires option \"-p\"")
				("loglevel,l",po::value<unsigned short>(&loglevel)->default_value(DEFAULTLOGLEVEL),
						"The dedired log level")
						("addhydrogens,a",po::value<bool>(&addhydrogens)->default_value(DEFAULTADDHYDROGENS),
								"Whether hydrogens should be added before the energy is calculated")
								("forcefield,F",po::value<std::string>(&forcefield)->default_value(DEFAULTFORCEFIELD), ffinfo.c_str())
								("filename,n",po::value<std::string>(&filename)->default_value(DEFAULTFILENAME),
										"Name of a file with the available molecule configurations")
										;

		po::variables_map vm;
		po::store(po::parse_command_line(argc, argv, desc), vm);
		po::notify(vm);

		// Initialize some reference arguments
		init = false;
		finalize = false;
		templ = false;
		randInt = false;
		result = false;

		// Emit a help message, if necessary
		if (vm.count("help") || filename == DEFAULTFILENAME) {
			std::cerr << desc << std::endl;
			return false;
		}

		if(loglevel > 3) {
			std::cerr << "Error: found invalid log level " << loglevel << std::endl;
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

		if(vm.count("filename") == 0) {
			std::cout << "Error: You need to specify the name of the molecular file" << std::endl;
			return false;
		}

		// Check whether we have been asked to calculate the energy of a given molecular configuration
		if(vm.count("singleEvaluation")) {
			singleEvaluation=true;
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
				if(vm.count("randInit")) {
					randInit = true;
				}

				return true;
			}

			if(vm.count("result")) {
				result = true;
				return true;
			}
		}

		// Our duty is to evaluate the content of the parameter file
		return true;
	}
	catch(...){
		std::cout << "Error parsing the command line" << std::endl;
		return false;
	}

	return true;
}

/************************************************************************************************/
