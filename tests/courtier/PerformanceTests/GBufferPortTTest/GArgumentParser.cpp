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

#include "GArgumentParser.hpp"

namespace Gem {
namespace Courtier {
namespace Tests {

/************************************************************************************************/
/**
 * A function that parses the command line for all required parameters
 */
bool parseCommandLine(
	int argc, char **argv, boost::uint32_t &nProductionCycles, std::size_t &nContainerEntries, long &putTimeoutMS,
	long &getTimeoutMS, std::size_t &maxPutTimeouts, std::size_t &maxGetTimeouts
) {
	try {
		// Check the command line options. Uses the Boost program options library.
		po::options_description desc("Usage: program [options]");
		desc.add_options()
			("help,h", "emit help message")
			("nProductionCycles,n",
			 po::value<boost::uint32_t>(&nProductionCycles)->default_value(DEFAULTNPRODUCTIONCYLCESAP),
			 "The number of production cycles in producer and processor")
			("nContainerEntries,c", po::value<std::size_t>(&nContainerEntries)->default_value(DEFAULTNCONTAINERENTRIESAP),
			 "The number of items in the random number container")
			("putTimeoutMS,p", po::value<long>(&putTimeoutMS)->default_value(DEFAULTPUTTIMEOUTMSAP),
			 "The put timeout")
			("getTimeoutMS,g", po::value<long>(&getTimeoutMS)->default_value(DEFAULTGETTIMEOUTMSAP),
			 "The get timeout")
			("maxPutTimeouts,o", po::value<std::size_t>(&maxPutTimeouts)->default_value(DEFAULTMAXPUTTIMEOUTS),
			 "The maximum number of put timeouts")
			("maxGetTimeouts,i", po::value<std::size_t>(&maxGetTimeouts)->default_value(DEFAULTMAXGETTIMEOUTS),
			 "The maximum number of get timeouts");

		po::variables_map vm;
		po::store(po::parse_command_line(argc, argv, desc), vm);
		po::notify(vm);

		// Emit a help message, if necessary
		if (vm.count("help")) {
			glogger
			<< desc << std::endl
			<< GSTDERR;
			return false;
		}

		std::cout << std::endl
		<< "Running with the following command line options:" << std::endl
		<< "nProductionCycles = " << nProductionCycles << std::endl
		<< "nContainerEntries = " << nContainerEntries << std::endl
		<< "putTimeoutMS = " << putTimeoutMS << std::endl
		<< "getTimeoutMS = " << getTimeoutMS << std::endl
		<< "maxPutTimeouts = " << maxPutTimeouts << std::endl
		<< "maxGetTimeouts = " << maxGetTimeouts << std::endl
		<< std::endl;
	}
	catch (...) {
		std::cout << "Error parsing the command line" << std::endl;
		return false;
	}

	return true;
}

/************************************************************************************************/

} /* namespace Tests */
} /* namespace Courtier */
} /* namespace Gem */
