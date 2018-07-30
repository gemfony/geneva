/**
 * @file GConstrainedFPTTest.cpp
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

/**
 * This test takes a GConstrainedDoubleObject object:
 * a) It examines the mapping from internal to external representation of its value.
 * b) It tests the "distortion" of a gaussian when going through the mapping from
 *    internal to external value.
 *
 * Additional tests (including error handling) of the GConstrainedDoubleObject class have been
 * implemented as part of the unit tests.
 *
 * In order to see the results of this test, you need the Root toolkit from http://root.cern.ch.
 * Once installed call "root -l mapping.C" .
 */

// Standard header files go here
#include <vector>
#include <iostream>
#include <fstream>
#include <string>

// Boost header files go here
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

// Geneva header files go here
#include "hap/GRandomT.hpp"
#include "courtier/GCommandContainerT.hpp"
#include "geneva-individuals/GFunctionIndividual.hpp"

using namespace Gem::Geneva;
using namespace Gem::Common;
using namespace boost;

const std::uint32_t DEFEXAMPLENTESTS=5000;
const Gem::Common::serializationMode DEFEXAMPLESERMOD = Gem::Common::serializationMode::BINARY;
const Gem::Common::serializationMode PRINTOUTSERMOD = Gem::Common::serializationMode::XML;

int main(int argc, char **argv){
	//-----------------------------------------------------------------------------
	// Declare some local parameters
	std::uint32_t nTests = DEFEXAMPLENTESTS;
	Gem::Common::serializationMode serMod = DEFEXAMPLESERMOD;
	bool printLastWorkItem = false;

	//-----------------------------------------------------------------------------
	// Read in command line parameters

	// Create the parser builder
	Gem::Common::GParserBuilder gpb;

	// Add an option for the number of tests
	gpb.registerCLParameter<std::uint32_t>(
		"nTests,n"
		, nTests
		, DEFEXAMPLENTESTS
		, "The number of tests to run"
	);

	// Add an option for the desired serialization mode
	gpb.registerCLParameter<Gem::Common::serializationMode>(
		"serializationMode,s"
		, serMod
		, DEFEXAMPLESERMOD
		, "The serialization mode: (0) TEXT, (1) XML, (2) BINARY"
	);

	// Add an option to print the last (raw and de-de-serialized) work item as XML
	gpb.registerCLParameter<bool>(
		"printLastWorkItem,p"
		, printLastWorkItem
		, false
		, "Whether the last work item should be printed (as XML, before and after submission / return)"
	);

	// Parse the command line and leave if the help flag was given. The parser
	// will emit an appropriate help message by itself
	if(Gem::Common::GCL_HELP_REQUESTED == gpb.parseCommandLine(argc, argv, true /*verbose*/)) {
		return 0; // Do not continue. This is expected.
	}

	//-----------------------------------------------------------------------------
	// Create a factory for GFunctionIndividual objects and perform
	// any necessary initial work.
	GFunctionIndividualFactory gfi("./config/GFunctionIndividual.json");

	// Loop the specified number of times
	for(std::size_t i=0; i<nTests; i++) {
		if(i%100 == 0) {
			std::cout << "In iteration " << i << std::endl;
		}

		// Retrieve a new work item from the factory
		auto fi_ptr = gfi.get_as<GFunctionIndividual>();

		// Randomly initialize the object
		fi_ptr->randomInit(activityMode::ALLPARAMETERS);

		// Add the object to a new command container
		Gem::Courtier::GCommandContainerT<Gem::Geneva::GParameterSet, Gem::Courtier::networked_consumer_payload_command> gcc1(
			Gem::Courtier::networked_consumer_payload_command::COMPUTE
			, fi_ptr
		);

		// Prepare a command container for de-serialization
		Gem::Courtier::GCommandContainerT<Gem::Geneva::GParameterSet, Gem::Courtier::networked_consumer_payload_command> gcc2(
			Gem::Courtier::networked_consumer_payload_command::NONE
		);

		// Serialize and de-serialize the object
		Gem::Courtier::container_from_string(
			Gem::Courtier::container_to_string(gcc1, serMod)
			, gcc2
			, serMod
		);

		// Check that payloads 1+2 point to different objects
		if(gcc1.get_payload().get() == gcc2.get_payload().get()) {
			std::cout << "Error: payload 1+2 seem to point to the same object" << std::endl;
			return 1; // Indicate an error to the calling process
		}

		// Check that the payloads of gcc1 and gcc2 are identical
		if(*(gcc1.get_payload()) != *(gcc2.get_payload())) {
			std::cout << "Error: Content of payload 1+2 differs" << std::endl;
			return 1; // Indicate an error to the calling process
		}

		// Process payloads 1+2 and compare -- they should now again be identical
		gcc1.process();
		gcc2.process();
		if(gcc1.get_payload().get() == gcc2.get_payload().get()) {
			std::cout << "Error: payload 1+2 seem to point to the same object" << std::endl;
			return 1; // Indicate an error to the calling process
		}

		// Prepare a command container for de-serialization
		Gem::Courtier::GCommandContainerT<Gem::Geneva::GParameterSet, Gem::Courtier::networked_consumer_payload_command> gcc3(
			Gem::Courtier::networked_consumer_payload_command::NONE
		);

		// Serialize and de-serialize the object
		Gem::Courtier::container_from_string(
			Gem::Courtier::container_to_string(gcc2, serMod)
			, gcc3
			, serMod
		);

		// Check that payloads 2+3 point to different objects
		if(gcc2.get_payload().get() == gcc3.get_payload().get()) {
			std::cout << "Error: payload 2+3 seem to point to the same object" << std::endl;
			return 1; // Indicate an error to the calling process
		}

		// Check that the payloads of gcc2 and gcc3 are identical
		if(*(gcc2.get_payload()) != *(gcc3.get_payload())) {
			std::cout << "Error: Content of payload 2+3 differs" << std::endl;
			return 1; // Indicate an error to the calling process
		}

		// Check that the payloads of gcc1 and gcc3 are identical
		if(*(gcc1.get_payload()) != *(gcc3.get_payload())) {
			std::cout << "Error: Content of payload 1+3 differs" << std::endl;
			return 1; // Indicate an error to the calling process
		}

		if(printLastWorkItem && i==(nTests - 1)) {
			std::cout
				<< "==========================================================" << std::endl
			   << Gem::Courtier::container_to_string(gcc1, PRINTOUTSERMOD) << std::endl
				<< "==========================================================" << std::endl
				<< Gem::Courtier::container_to_string(gcc3, PRINTOUTSERMOD) << std::endl
				<< "==========================================================" << std::endl;
		}
	}

	//-----------------------------------------------------------------------------
	// Done
	return 0;
}
