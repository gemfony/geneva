/**
 * @file GLoggerTest.cpp
 */

/********************************************************************************
 *
 * This file is part of the Geneva library collection. The following license
 * applies to this file:
 *
 * ------------------------------------------------------------------------------
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * ------------------------------------------------------------------------------
 *
 * Note that other files in the Geneva library collection may use a different
 * license. Please see the licensing information in each file.
 *
 ********************************************************************************
 *
 * Geneva was started by Dr. Rüdiger Berlich and was later maintained together
 * with Dr. Ariel Garcia under the auspices of Gemfony scientific. For further
 * information on Gemfony scientific, see http://www.gemfomy.eu .
 *
 * The majority of files in Geneva was released under the Apache license v2.0
 * in February 2020.
 *
 * See the NOTICE file in the top-level directory of the Geneva library
 * collection for a list of contributors and copyright information.
 *
 ********************************************************************************/

// Standard headers go here
#include <cmath>
#include <iostream>

// Boost headers go here
#include <boost/filesystem.hpp>

// Geneva headers go here
#include "common/GLogger.hpp"
#include "common/GParserBuilder.hpp"

using namespace Gem::Common;

int main(int argc, char** argv) {
	bool do_crash = false;

	// Create the parser builder
	GParserBuilder gpb;

	gpb.registerCLParameter<bool>(
		"crash,c"
		, do_crash
		, false // do not crash by default
		, "Whether an uncaught exception should be raised"
		, GCL_IMPLICIT_ALLOWED // Permit implicit values, so that we can say --crash instead of --crash=true
		, true // Crash only, if --crash or -c was specified on the command line
	);

	// Do the actual command line parsing
	if(GCL_HELP_REQUESTED == gpb.parseCommandLine(argc, argv, false /* not verbose */)) {
		return 0; // Do not continue
	}

	std::shared_ptr<GBaseLogTarget> gcl_ptr(new GConsoleLogger());
	std::shared_ptr<GBaseLogTarget> gfl_ptr(new GFileLogger("./somePathToLogFile.txt"));

	glogger.addLogTarget(gcl_ptr);
	glogger.addLogTarget(gfl_ptr);

	// Emission of a leading std::endl
	glogger << std::endl << "This comment starts in the next line!" << std::endl << GLOGGING;

	// Normal output to all logging targets
	glogger << "Some information " << 1 << " " << 2 << std::endl << GLOGGING;

	// Warning emitted to all targets
	glogger << "Some information " << 3 << " " << 4 << std::endl << GWARNING;

	// Raising an exception. Note that the data will also be written to
	// a file named GENEVA-EXCEPTION.log
	try {
		glogger << "Some information " << 5 << " " << 6 << std::endl << GEXCEPTION;
	} catch(gemfony_exception& e) {
		std::cout
		<< "Caught exception with message" << std::endl
		<< e << std::endl;
	}

	// Output to a specific file
	glogger(boost::filesystem::path("anotherFile")) << "Some other information " << 7 << " " << 8 << std::endl << GFILE;

	// Output to registered logging targets with a given extension
	glogger(std::string("extension")) << "And yet another information " << 9 << " " << 10 << std::endl << GLOGGING;

	// Output to stdout
	glogger << "std::out-information" << std::endl << GSTDOUT;

	// Output to stderr
	glogger << "std::err information" << std::endl << GSTDERR;

	// Crash the applicaton if requested
	if(do_crash) {
		glogger << "A crash was requested. Crashing ..." << GEXCEPTION;
	}
}
