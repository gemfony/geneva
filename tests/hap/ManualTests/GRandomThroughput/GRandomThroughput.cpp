/**
 * @file GRandomThroughput.cpp
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
	Gem::Hap::GRandomT<Gem::Hap::RANDFLAVOURS::PROXY> gr;

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
