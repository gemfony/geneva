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

namespace Gem {
namespace GenEvA {

/************************************************************************************************/
/**
 * A function that parses the command line for all required parameters
 */

bool parseCommandLine(int argc, char **argv,
		double& sigma,
		double& sigmaSigma,
		double& minSigma,
		double& maxSigma,
		boost::uint32_t& adaptionThreshold,
		std::string& resultFile,
		boost::uint32_t& maxIter,
		bool& verbose)
{
	try {
		// Check the command line options. Uses the Boost program options library.
		po::options_description desc("Allowed options");
		desc.add_options()
		("help,h", "emit help message")
		("sigma,S",po::value<double>(&sigma)->default_value(CMD_DEFAULTSIGMA),
				"Width of the gaussian")
		("sigmaSigma,s",po::value<double>(&sigmaSigma)->default_value(CMD_DEFAULTSIGMASIGMA),
				"Width of the gaussian used to adapt sigma")
		("minSigma,m",po::value<double>(&minSigma)->default_value(CMD_DEFAULTMINSIGMA),
				"Minimal allowed value of sigma")
		("maxSigma,M",po::value<double>(&maxSigma)->default_value(CMD_DEFAULTMAXSIGMA),
				"Maximum allowed value of sigma")
		("adaptionThreshold,a",po::value<boost::uint32_t>(&adaptionThreshold)->default_value(CMD_DEFAULTADAPTIONTHRESHOLD),
				"Number of calls to mutate after which the mutation should be adapted")
		("resultFile,F",po::value<std::string>(&resultFile)->default_value(CMD_DEFAULTRESULTFILE),
				"The file to write the result to")
		("maxIter,I",po::value<boost::uint32_t>(&maxIter)->default_value(CMD_DEFAULTMAXITER),
				"The maximum number of test cycles")
		("verbose,v",po::value<bool>(&verbose)->default_value(CMD_DEFAULTVERBOSE),
				"Whether to emit status information")
		;

		po::variables_map vm;
		po::store(po::parse_command_line(argc, argv, desc), vm);
		po::notify(vm);

		// Emit a help message, if necessary
		if (vm.count("help")) {
			std::cout << desc << std::endl;
			return false;
		}

		if(verbose) {
			std::cout << std::endl
			<< "Running with the following options:" << std::endl
			<< "sigma = " << sigma << std::endl
			<< "sigmaSigma = " << sigmaSigma << std::endl
			<< "minSigma = " << minSigma << std::endl
			<< "maxSigma = " << maxSigma << std::endl
			<< "adaptionThreshold = " << adaptionThreshold << std::endl
			<< "resultFile = " << resultFile << std::endl
			<< "maxIter = " << maxIter << std::endl
			<< std::endl;
		}
	}
	catch(...) {
		std::cout << "Error parsing the command line" << std::endl;
		return false;
	}

	return true;
}

/************************************************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */
