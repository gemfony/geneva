/**
 * @file GParser.cpp
 */

/*
 * Copyright (C) 2009 Dr. Ruediger Berlich
 *
 * This file is part of the eminim2 program. eminim2 is free software:
 * you can redistribute and/or modify this file under the terms of
 * version 2 of the GNU General Public License as published by the
 * Free Software Foundation.
 *
 * eminim2 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with the Geneva library.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "GParser.hpp"

/************************************************************************************************/
/**
 * This function parses the command line for all required parameters . The program
 * can be called in the following modes:
 *
 * -i / --initialize : gives the external program the opportunity to do any needed
 *                          preliminary work (e.g. downloading files, setting up directories, ...)
 * -f / -- finilize :   Allows the external program to clean up after work. Compare the -i switch.
 * -p / --paramfile <filename>: The name of the file through which data is exchanged
 *     This switch is needed for the following options:
 *     -t / --template: asks the external program to write a description of the individual
 *                               into paramfile (e.g. program -t -p <filename> ).
 *            -t also allows the additional option -R (randomly initialize parameters)
 *     -r / --result  Asks the external program to emit a result file in a user-defined format.
 *                         It will not be read in by this individual, but allows the user to examine
 *                         the results. The input data needed to create the result file is contained
 *                         in the parameter file. Calls will be done like this: "program -r -p <filename>"
 * If the "-p <filename>" switch is used without any additional switches, the external program
 * is expected to perform a value calculation, based on the data in the parameter file and
 * to emit the result into the same file.
 * The following switch affects the desired transfer mode between external program
 * and this individual.
 * -m / --transferMode=<number>
 *     0 means binary mode (the default, if --transferMode is not used)
 *     1 means text mode
 * -s / --singleEvaluation Used as the only command line argument. Performs a single
 *  calculation of the energy of a given protein. Used independently of the optimization run for
 *  verification purposes only.
 *
 * In other words, the following arguments are possible
 * [Mode  0] -h
 * [Mode  1] -i
 * [Mode  2] -f
 * [Mode  3] -p <filename>
 * [Mode  4] -p <filename>  -t
 * [Mode  5] -p <filename> -t -R
 * [Mode  6] -p <filename> -r
 * [Mode  3] -p <filename> -m <mode>
 * [Mode  4] -p <filename>  -t -m <mode>
 * [Mode  5] -p <filename> -t -R -m <mode>
 * [Mode  6] -p <filename> -r -m <mode>
 * [Mode  7] -s
 */
bool parseCommandLine(int argc, char **argv,
		boost::uint16_t& executionMode,
		std::string& paramfile,
		boost::uint16_t& transferMode,
		std::string& configFile)
{
	try{
		// Check the command line options. Uses the Boost program options library.
		po::options_description desc("Usage: evaluator [options]");
		desc.add_options()
		("help,h", "Emit help message")
		("initialize,i", "Perform necessary initial tasks. Other options will be ignored.")
		("finalize,f", "Perform any final actions. Other options will be ignored.")
		("paramfile,p",po::value<std::string>(&paramfile)->default_value(DEFAULTPARAMFILE),  "Name of a file with the parameters")
		("result,r", "Write out a result file for a given parameter set. Requires option \"-p\"")
		("template,t", "Write out a template for this population. Requires option \"-p\"")
		("random,R", "Asks the program to fill the template with random values. Requires option \"-t\"")
		("transferMode,m",  po::value<boost::uint16_t>(&transferMode)->default_value(DEFAULTTRANSFERMODE),  "The transfer mode between optimizer and this program (binary or text mode, at the moment)")
		("singleEvaluation,s", "Perform just a single evaluation of the molecule data")
		("configFile,F", po::value<std::string>(&configFile)->default_value(DEFAULTCONFIGFILE) ,"The name of the config file used to store additional program options")
		;

		po::variables_map vm;
		po::store(po::parse_command_line(argc, argv, desc), vm);
		po::notify(vm);

		// Initialize some arguments
		executionMode = 0; // no effect

		// Emit a help message, if necessary
		if (vm.count("help")) {
			std::cerr << desc << std::endl;
			return false;
		}

		// Check whether we have been asked to calculate the energy of a given molecular configuration
		// There is no need to do any further parsing of the command line, as all necessary options
		// are already known.
		if(vm.count("singleEvaluation")) {
			executionMode=7;
			return true;
		}

		// Check whether we have been asked to perform initial work.
		// If found, all other options will be ignored
		if(vm.count("initialize")) {
			executionMode = 1;
			return true;
		}

		// Check whether we have been asked to perform final tasks
		// If found, all other options will be ignored
		if(vm.count("finalize")) {
			executionMode = 2;
			return true;
		}

		// All other options require the -p/--paramfile switch
		if(vm.count("paramfile") == 0 || paramfile == DEFAULTPARAMFILE || paramfile.empty()) {
			std::cerr << "Error: You need to specify the name of the parameter file (option \"-p\")." << std::endl
			<< "Make sure it is not \"empty\"" << std::endl;
			if(vm.count("paramfile")==0) std::cout << "Current name is \"" << paramfile << "\"" << std::endl;
			return false;
		}

		// Now we are sure that we have been given a "real" file name

		// Check that the transfer mode has a valid value
		switch(transferMode) {
		case 0: // binary mode, nothing to do
		case 1: // text mode, nothing to do
			break;

		default:
			std::cerr << "Error: An invalid transfer mode has been specified; " << transferMode << std::endl;
			return false;
		};

		if(vm.count("template")) {
			executionMode = 4;
			if(vm.count("random")) executionMode = 5;
			return true;
		}

		if(vm.count("result")) {
			executionMode=6;
			return true;
		}

		// Our duty is to evaluate the content of the parameter file
		executionMode = 3;
		return true;
	}
	catch(std::exception& e){
		std::cout << "Error parsing the command line" << std::endl
			            << e.what() << std::endl;
		return false;
	}

	return true; // make the compiler happy
}

/************************************************************************************************/
/**
 * Parses a configuration file with name "configFile" for additional options, related to
 * the energy calculation for a given molecule description, contained in "proteinDescription".
 */
bool parseConfigFile(const std::string& configFile,
		unsigned short& loglevel,
		bool& addhydrogens,
		std::string& forcefield,
		std::string& proteinDescription)
{
	// Assemble a std::string in case we need to emit information about the force field
	std::string ffinfo = "The forcefield.\nAvaible options:\n" + OpenBabel::OBPlugin::ListAsString("forcefields", "verbose");

	// Check the name of the configuation file
	if(configFile.empty() || configFile == "empty" || configFile == "unknown") {
		std::cerr << "Error: Invalid configuration file name given: \"" << configFile << "\"" << std::endl;
		return false;
	}

	try{
		// Check the command line options. Uses the Boost program options library.
		po::options_description config(("Parser for the configuration file " + configFile).c_str());
		config.add_options()
		("loglevel",po::value<unsigned short>(&loglevel)->default_value(DEFAULTLOGLEVEL), "The dedired log level")
		("addhydrogens",po::value<bool>(&addhydrogens)->default_value(DEFAULTADDHYDROGENS), 	"Whether hydrogens should be added before the energy is calculated")
		("forcefield",po::value<std::string>(&forcefield)->default_value(DEFAULTFORCEFIELD), ffinfo.c_str())
		("proteinDescription",po::value<std::string>(&proteinDescription)->default_value(DEFAULTPROTEINDESCRIPTION),	"Name of a file with the available molecule configurations");

		po::variables_map vm;
		std::ifstream ifs(configFile.c_str());
		if(!ifs.good()) {
			std::cerr << "Error accessing configuration file " << configFile;
			return false;
		}
        store(po::parse_config_file(ifs, config), vm);
        notify(vm);

		if(loglevel > 3) {
			std::cerr << "Error: found invalid log level " << loglevel << std::endl;
			return false;
		}

		if(vm.count("proteinDescription") == 0) {
			std::cout << "Error: You need to specify the name of a molecule description in  " << configFile << std::endl;
			return false;
		}

		// Our duty is to evaluate the content of the parameter file
		return true;
	}
	catch(std::exception& e){
		std::cout << "Error parsing config file \" " << configFile << "\"" << std::endl
			            << e.what() << std::endl;
		return false;
	}

	return true;
}

/************************************************************************************************/
