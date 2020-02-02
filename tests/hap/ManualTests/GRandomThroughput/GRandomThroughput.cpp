/**
 * @file GRandomThroughput.cpp
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

// Standard header files go here
#include <iostream>
#include <random>
#include <thread>
#include <mutex>
#include <vector>
#include <algorithm>
#include <chrono>

// Boost header files go here

// Geneva header files go here
#include "hap/GRandomT.hpp"
#include "hap/GRandomDistributionsT.hpp"
#include "common/GParserBuilder.hpp"


int main(int argc, char **argv) {
	std::uint16_t nProducerThreads = 4;
	std::uint32_t packageSize = 10000;
	std::uint32_t nCycles = 1000;
	double lowerBoundary = 0.;
	double upperBoundary = 1.;

	//----------------------------------------------------------------
	// Create the parser builder
	Gem::Common::GParserBuilder gpb;

	// Specify command line options
	gpb.registerCLParameter<std::uint16_t>(
		"nProducerThreads,n"
		, nProducerThreads // the variable to be filled
		, nProducerThreads // the default
		, "The number of threads for the production of random numbers"
	);

	gpb.registerCLParameter<std::uint32_t>(
		"packageSize,p"
		, packageSize // the variable to be filled
		, packageSize // the default
	   , "The amount of random numbers to be read in one go"
	);

	gpb.registerCLParameter<std::uint32_t>(
		"cycles,c"
		, nCycles // the variable to be filled
		, nCycles // the default
		, "The number of times that a vector should be filled with random numbers"
	);

	gpb.registerCLParameter<double>(
		"lowerBoundary,l"
		, lowerBoundary // the variable to be filled
		, lowerBoundary // the default
		, "The lower boundary for the production of random numbers"
	);

	gpb.registerCLParameter<double>(
		"upperBoundary,u"
		, upperBoundary // the variable to be filled
		, upperBoundary // the default
		, "The upper boundary for the production of random numbers"
	);

	// Parse the command line and leave if the help flag was given
	if(Gem::Common::GCL_HELP_REQUESTED == gpb.parseCommandLine(argc, argv, true /*verbose*/)) {
		return 0;
	}

	//----------------------------------------------------------------

	// Configure the random number factory
	GRANDOMFACTORY->setNProducerThreads(nProducerThreads);

	// Retrieve a random number proxy
	Gem::Hap::GRandomT<Gem::Hap::RANDFLAVOURS::RANDOMPROXY> gr;

	// Storage and production of random numbers
	std::vector<double> payload(packageSize);
	std::uniform_real_distribution<double> uniform_real(lowerBoundary, upperBoundary);

	// Run the measurement loop
	std::chrono::system_clock::time_point startTime = std::chrono::system_clock::now();
	for(std::uint32_t c=0; c<nCycles; c++) {
		for(auto& p: payload) {
			p = uniform_real(gr);
		}
		std::sort(payload.begin(), payload.end());
		// assert(is_sorted(payload.begin(), payload.end()));
	}
	std::chrono::system_clock::time_point endTime = std::chrono::system_clock::now();
	std::chrono::duration<double> duration = endTime - startTime;

	// Let the audience know
	double throughput=double(nCycles*packageSize)/duration.count();
	double megabytes=8.*throughput/(1024*1024);
	std::cout << "Achieved a throughput of " << throughput << " double random numbers/s (equivalent to " << megabytes << " MB/s)" << std::endl;
}
