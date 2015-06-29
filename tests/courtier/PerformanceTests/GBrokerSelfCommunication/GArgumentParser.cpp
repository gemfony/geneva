/**
 * @file GArgumentParser.cpp
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

#include <common/GLogger.hpp>
#include "GArgumentParser.hpp"

namespace Gem {
namespace Courtier {
namespace Tests {

/************************************************************************************************/
/**
 * A function that parses the command line for all required parameters
 */
bool parseCommandLine(
	int argc, char **argv, std::string &configFile, GBSCModes &executionMode, bool &serverMode, std::string &ip,
	unsigned short &port, Gem::Common::serializationMode &serMode, submissionReturnMode &srm,
	bool &useDirectBrokerConnection
) {
	try {
		// Check the command line options. Uses the Boost program options library.
		po::options_description desc("Usage: evaluator [options]");
		desc.add_options()
			("help,h", "emit help message")
			("configFile,c", po::value<std::string>(&configFile)->default_value(DEFAULTCONFIGFILEAP),
			 "The name of the configuration file holding further configuration options")
			("executionMode,e", po::value<GBSCModes>(&executionMode)->default_value(DEFAULTEXECUTIONMODEAP),
			 "Whether to run this program with a serial consumer (0), with internal networking (1), networking (2), multi-threaded (3), multithreaded and internal networking (4) or multithreaded and networked mode (5)")
			("serverMode,s",
			 "Whether to run networked execution in server or client mode. The option only gets evaluated if \"--parallelizationMode=2\" or \"--parallelizationMode=5\"")
			("ip", po::value<std::string>(&ip)->default_value(DEFAULTIPAP), "The ip of the server")
			("port", po::value < unsigned short > (&port)->default_value(DEFAULTPORTAP), "The port of the server")
			("serMode", po::value<Gem::Common::serializationMode>(&serMode)->default_value(DEFAULTSERMODEAP),
			 "Specifies whether serialization shall be done in TEXTMODE (0), XMLMODE (1) or BINARYMODE (2)")
			("srm,f", po::value<submissionReturnMode>(&srm)->default_value(DEFAULTSRMAP),
			 "Whether items from older iterations may return and an incomplete return is acceptable (0), the "
				 "same but without the permission for older items to return (0) ir whether a complete return of"
				 "a given submission\'s items is required")
			("useDirectBrokerConnection,u",
			 po::value<bool>(&useDirectBrokerConnection)->default_value(DEFAULTUSEDIRECTBROKERCONNECTIONAP),
			 "Indicates whether producers should connect directly to the broker or through the broker connector object");

		po::variables_map vm;
		po::store(po::parse_command_line(argc, argv, desc), vm);
		po::notify(vm);

		// Emit a help message, if necessary
		if (vm.count("help")) {
			glogger
			<< desc << std::endl
			<< GSTDERR;
		}

		serverMode = false;
		if (vm.count("executionMode")) {
			if (executionMode > Gem::Courtier::Tests::MAXGBSCMODES) {
				glogger
				<< "Error: Received invalid GBSC mode: " << executionMode << std::endl
				<< GWARNING;
				return false;
			}

			if (executionMode == Gem::Courtier::Tests::NETWORKING ||
				 executionMode == Gem::Courtier::Tests::THREAEDANDNETWORKING) {
				if (vm.count("serverMode")) serverMode = true;
			}
		}

		std::string parModeString;
		switch (executionMode) {
			case Gem::Courtier::Tests::SERIAL:
				parModeString = "serial consumer";
				break;

			case Gem::Courtier::Tests::INTERNALNETWORKING:
				parModeString = "internal networking";
				break;

			case Gem::Courtier::Tests::NETWORKING:
				parModeString = "networking";
				break;

			case Gem::Courtier::Tests::MULTITHREADING:
				parModeString = "multi-threaded";
				break;

			case Gem::Courtier::Tests::THREADANDINTERNALNETWORKING:
				parModeString = "multi-threaded and internal networking";
				break;

			case Gem::Courtier::Tests::THREAEDANDNETWORKING:
				parModeString = "multi-threaded and networked";
				break;

			default: {
				glogger
				<< "Error: Invalid execution mode requested: " << executionMode << std::endl
				<< GWARNING;
				return false;
			}
				break;
		};

		std::cout
		<< std::endl
		<< "Running with the following command line options:" << std::endl
		<< "configFile = " << configFile << std::endl
		<< "executionMode = " << parModeString << std::endl
		<< "serverMode = " << (serverMode ? "true" : "false") << std::endl
		<< "ip = " << ip << std::endl
		<< "port = " << port << std::endl
		<< "serMode = " << serMode << std::endl
		<< "srm = " << srm << std::endl
		<< "useDirectBrokerConnection = " << useDirectBrokerConnection << std::endl
		<< std::endl;
	}
	catch (...) {
		std::cout << "Error parsing the command line" << std::endl;
		return false;
	}

	return true;
}

/************************************************************************************************/

bool parseConfigFile(
	const std::string &configFile, std::uint32_t &nProducers, std::uint32_t &nProductionCycles,
	std::uint32_t &nContainerObjects, std::size_t &nContainerEntries, std::size_t &maxResubmissions,
	std::uint32_t &nWorkers
) {
	// Check the name of the configuation file
	if (configFile.empty() || configFile == "empty" || configFile == "unknown") {
		glogger
		<< "Error: Invalid configuration file name given: \"" << configFile << "\"" << std::endl
		<< GWARNING;
		return false;
	}

	try {
		// Check the configuration file line options. Uses the Boost program options library.
		po::options_description config("Allowed options");
		config.add_options()
			("nProducers", po::value<std::uint32_t>(&nProducers)->default_value(DEFAULTNPRODUCERSAP))
			("nProductionCycles",
			 po::value<std::uint32_t>(&nProductionCycles)->default_value(DEFAULTNPRODUCTIONCYLCESAP))
			("nContainerObjects",
			 po::value<std::uint32_t>(&nContainerObjects)->default_value(DEFAULTNCONTAINEROBJECTSAP))
			("nContainerEntries", po::value<std::size_t>(&nContainerEntries)->default_value(DEFAULTNCONTAINERENTRIESAP))
			("maxResubmissions", po::value<std::size_t>(&maxResubmissions)->default_value(DEFAULTMAXRESUBMISSIONSAP))
			("nWorkers", po::value<std::uint32_t>(&nWorkers)->default_value(DEFAULTNWORKERSAP));

		po::variables_map vm;
		boost::filesystem::ifstream ifs(configFile);
		if (!ifs) {
			glogger
			<< "Error accessing configuration file " << configFile
			<< GWARNING;
			return false;
		}

		store(po::parse_config_file(ifs, config), vm);
		notify(vm);

		std::cout
		<< std::endl
		<< "Running with the following options from " << configFile << ":" << std::endl
		<< "nProducers = " << nProducers << std::endl
		<< "nProductionCycles = " << nProductionCycles << std::endl
		<< "maxResubmissions = " << maxResubmissions << std::endl
		<< "nContainerObjects = " << nContainerObjects << std::endl
		<< "nContainerEntries = " << nContainerEntries << std::endl
		<< "nWorkers = " << nWorkers << std::endl
		<< std::endl;
	} catch (...) {
		glogger
		<< "Error parsing the configuration file " << configFile << std::endl
		<< GWARNING;
		return false;
	}

	return true;
}

/************************************************************************************************/

} /* namespace Tests */
} /* namespace Courtier */
} /* namespace Gem */
