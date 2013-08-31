/**
 * @file GCommandLineParser.cpp
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt)
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) eu
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
 * http://www.gemfony.eu .
 */

#include "GCommandLineParser.hpp"

namespace Gem {
namespace Tests {

/************************************************************************************************/
/**
 * A function that parses the command line for all required parameters
 */

bool parseCommandLine (
		int argc, char **argv
		, float& sigma1
		, float& sigmaSigma1
		, float& minSigma1
		, float& maxSigma1
		, float& sigma2
		, float& sigmaSigma2
		, float& minSigma2
		, float& maxSigma2
		, float& delta
		, float& sigmaDelta
		, float& minDelta
		, float& maxDelta
		, boost::uint32_t& adaptionThreshold
		, std::string& resultFile
		, boost::uint32_t& maxIter
		, bool& verbose
){
	try {
		// Check the command line options. Uses the Boost program options library.
		po::options_description desc("Allowed options");
		desc.add_options()
		("help,h", "emit help message")
		("sigma1",po::value<float>(&sigma1)->default_value(CMD_DEFAULTSIGMA1),
				"Width of the first gaussian")
		("sigmaSigma1",po::value<float>(&sigmaSigma1)->default_value(CMD_DEFAULTSIGMASIGMA1),
				"Width of the gaussian used to adapt sigma1")
		("minSigma1",po::value<float>(&minSigma1)->default_value(CMD_DEFAULTMINSIGMA1),
				"Minimal allowed value of sigma1")
		("maxSigma1",po::value<float>(&maxSigma1)->default_value(CMD_DEFAULTMAXSIGMA1),
				"Maximum allowed value of sigma1")
		("sigma2",po::value<float>(&sigma2)->default_value(CMD_DEFAULTSIGMA2),
				"Width of the second gaussian")
		("sigmaSigma2",po::value<float>(&sigmaSigma2)->default_value(CMD_DEFAULTSIGMASIGMA2),
				"Width of the gaussian used to adapt sigma2")
		("minSigma2",po::value<float>(&minSigma2)->default_value(CMD_DEFAULTMINSIGMA2),
				"Minimal allowed value of sigma2")
		("maxSigma2",po::value<float>(&maxSigma2)->default_value(CMD_DEFAULTMAXSIGMA2),
				"Maximum allowed value of sigma2")
		("delta",po::value<float>(&delta)->default_value(CMD_DEFAULTDELTA),
				"Distance between both gaussians")
		("sigmaDelta",po::value<float>(&sigmaDelta)->default_value(CMD_DEFAULTSIGMADELTA),
				"Width of the gaussian used to adapt delta")
		("minDelta",po::value<float>(&minDelta)->default_value(CMD_DEFAULTMINDELTA),
				"Minimal allowed value for delta")
		("maxDelta",po::value<float>(&maxDelta)->default_value(CMD_DEFAULTMAXDELTA),
				"Maximum allowed value for delta")
		("adaptionThreshold,a",po::value<boost::uint32_t>(&adaptionThreshold)->default_value(CMD_DEFAULTADAPTIONTHRESHOLD),
				"Number of calls to adapt() after which the adaption parameters should be modified")
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
			<< "sigma1 = " << sigma1 << std::endl
			<< "sigmaSigma1 = " << sigmaSigma1 << std::endl
			<< "minSigma1 = " << minSigma1 << std::endl
			<< "maxSigma1 = " << maxSigma1 << std::endl
			<< "sigma2 = " << sigma1 << std::endl
			<< "sigmaSigma2 = " << sigmaSigma1 << std::endl
			<< "minSigma2 = " << minSigma1 << std::endl
			<< "maxSigma2 = " << maxSigma1 << std::endl
			<< "delta = " << delta << std::endl
			<< "sigmaDelta = " << sigmaDelta << std::endl
			<< "minDelta = " << minDelta << std::endl
			<< "maxDelta = " << maxDelta << std::endl
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

} /* namespace Tests */
} /* namespace Gem */
