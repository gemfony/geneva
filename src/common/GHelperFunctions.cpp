/**
 * @file GHelperFunctions.cpp
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt)
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) com
 *
 * This file is part of the Geneva library collection.
 *
 * Geneva was developed with kind support from Karlsruhe Institute of
 * Technology (KIT) and Steinbuch Centre for Computing (SCC). Further
 * information about KIT and SCC can be found at http://www.kit.edu/english
 * and http://scc.kit.edu .
 *
 * Geneva is free software: you can redistribute and/or modify it under
 * the terms of version 3 of the GNU Affero General Public License,
 * or, at your option, under the terms of version 2 of the GNU General
 * Public License, as published by the Free Software Foundation.
 *
 * NOTE THAT THIS DUAL-LICENSING OPTION DOES NOT APPLY TO ANY OTHER FILES OF THE
 * GENEVA LIBRARY, UNLESS THIS IS EXPLICITLY STATED IN THE CORRESPONDING FILE.
 *
 * Geneva is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * version 3 and of version 2 of the GNU General Public License
 * along with the Geneva library. If not, see <http://www.gnu.org/licenses/>.
 *
 * For further information on Gemfony scientific and Geneva, visit
 * http://www.gemfony.com .
 */

#include "common/GHelperFunctions.hpp"

namespace Gem {
namespace Common {

/**************************************************************************************************/
/**
 * This function retrieves the number of CPU cores on the system, which is regarded as a suitable
 * default number for the amount of threads of a specific kind.
 *
 * @param defaultNThreads The default number of threads to be returned if hardware concurrency cannot be determined
 * @return A guess at a suitable number of threads for this architecture
 */
unsigned int getNHardwareThreads(const unsigned int& defaultNThreads) {
	unsigned int nHardwareThreads = boost::thread::hardware_concurrency();
	if(nHardwareThreads > 0) {
		return nHardwareThreads;
	}
	else {
#ifdef DEBUG
		std::cout << "Could not get information regarding suitable number of threads." << std::endl
				  << "Using the default value " << defaultNThreads << " instead." << std::endl;
#endif

		return defaultNThreads;
	}
}

/**************************************************************************************************/
/**
 * This function loads textual (ASCII) data from an external file. Note that this function is
 * not currently meant to be used with binary data, although it should work with that as well
 * (at least in theory -- this is untested). There is no check whether the data loaded represents
 * a text.
 *
 * @param fileName The name of the file to be loaded
 * @return The data contained in the file
 */
std::string loadTextDataFromFile(const std::string& fileName) {
   std::ifstream sourceFileStream(fileName.c_str());
   std::string sourceFile (
            std::istreambuf_iterator<char>(sourceFileStream)
            , (std::istreambuf_iterator<char>())
            );
   return sourceFile;
}

/**************************************************************************************************/
/**
 * Execute an external command, reacting to possible errors.
 *
 * @param command The command to be executed
 */
void runExternalCommand(const std::string& command) {
#ifdef GEM_COMMON_PRINT_COMMANDLINE
		std::cout << "Executing external command \"" << commandLine << "\" ...";
#endif /* GEM_COMMON_PRINT_COMMANDLINE */

	int errorCode = system(command.c_str());

#ifdef GEM_COMMON_PRINT_COMMANDLINE
		std::cout << "... done." << std::endl;
#endif /* GEM_COMMON_PRINT_COMMANDLINE */

	if(errorCode) {
		raiseException(
				"In runExternalCommand(): Error" << std::endl
				<< "Command: " << command << std::endl
				<< "Error code: " << errorCode
		);
	}
}

/**************************************************************************************************/
/**
 * Returns a string for a given serialization mode
 *
 * @param s The serialization mode which should be translated to a string
 * @return A string for a given serialization mode
 */
std::string serializationModeToString(const serializationMode& s) {
	switch(s) {
	case SERIALIZATIONMODE_TEXT:
		return std::string("text mode");
		break;
	case SERIALIZATIONMODE_XML:
		return std::string("XML mode");
		break;
	case SERIALIZATIONMODE_BINARY:
		return std::string("binary mode");
		break;
	default:
	{
		raiseException(
				"In serializationModeToString(): Error!" << std::endl
				<< "Incorrect serialization mode requested: " << s << std::endl
		);
		break;
	}
	}

	// Make the compiler happy
	return std::string("");
}

/**************************************************************************************************/

} /* namespace Common */
} /* namespace Gem */
