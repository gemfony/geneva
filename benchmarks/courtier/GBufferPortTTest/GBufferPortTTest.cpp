/**
 * @file GBufferPortTTest.cpp
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

#include <vector>
#include <mutex>
#include <chrono>
#include <thread>

#include "courtier/GBufferPortT.hpp"
#include "courtier/GDemoProcessingContainers.hpp"
#include "common/GExceptions.hpp"
#include "common/GThreadGroup.hpp"
#include "common/GBarrier.hpp"
#include "common/GParserBuilder.hpp"

#define WORKLOAD GSimpleContainer
// #define WORKLOAD GRandomNumberContainer

/**
 * Some synchronization primitives
 */
std::size_t producer_counter;
std::mutex producer_mutex;

std::size_t processor_counter;
std::mutex processor_mutex;

std::mutex output_mutex;

/**
 * A barrier on which all threads have to wait
 */
std::shared_ptr<Gem::Common::GBarrier> sync_ptr;

using namespace Gem::Courtier;

/********************************************************************************/
// Default settings
const std::uint32_t DEFAULTNPRODUCTIONCYLCESAP = 500;
const std::size_t DEFAULTNCONTAINERENTRIESAP = 100;
const long DEFAULTPUTTIMEOUTMSAP = 1000;
const long DEFAULTGETTIMEOUTMSAP = 1000;
const std::size_t DEFAULTMAXPUTTIMEOUTS = 100;
const std::size_t DEFAULTMAXGETTIMEOUTS = 100;

/********************************************************************************/
/**
 * A function that parses the command line for all required parameters
 */
bool parseCommandLine(
	int argc, char **argv
	, std::uint32_t &nProductionCycles
	, std::size_t &nContainerEntries
	, long &putTimeoutMS
	, long &getTimeoutMS
	, std::size_t &maxPutTimeouts
	, std::size_t &maxGetTimeouts
) {
	// Create the parser builder
	Gem::Common::GParserBuilder gpb;

	gpb.registerCLParameter<std::uint32_t>(
		"nProductionCycles,n"
		, nProductionCycles
		, DEFAULTNPRODUCTIONCYLCESAP
		, "The number of production cycles in producer and processor"
	);

	gpb.registerCLParameter<std::size_t>(
		"nContainerEntries,c"
		, nContainerEntries
		, DEFAULTNCONTAINERENTRIESAP
		, "The number of items in the random number container"
	);

	gpb.registerCLParameter<long>(
		"putTimeoutMS,p"
		, putTimeoutMS
		, DEFAULTPUTTIMEOUTMSAP
		, "The put timeout"
	);

	gpb.registerCLParameter<long>(
		"getTimeoutMS,g"
		, getTimeoutMS
		, DEFAULTGETTIMEOUTMSAP
		, "The get timeout"
	);

	gpb.registerCLParameter<std::size_t>(
		"maxPutTimeouts,o"
		, maxPutTimeouts
		, DEFAULTMAXPUTTIMEOUTS
		, "The maximum number of put timeouts"
	);

	gpb.registerCLParameter<std::size_t>(
		"maxGetTimeouts,i"
		, maxGetTimeouts
		, DEFAULTMAXPUTTIMEOUTS
		, "The maximum number of getz timeouts"
	);

	// Parse the command line and leave if the help flag was given. The parser
	// will emit an appropriate help message by itself
	if(Gem::Common::GCL_HELP_REQUESTED == gpb.parseCommandLine(argc, argv, true /*verbose*/)) {
		return false; // Do not continue
	}

	return true;
}

/********************************************************************************/
/**
 * A global buffer port, to/from which WORKLOAD objects are written/read. We store
 * smart pointers instead of the objects themselves.
 */
GBufferPortT<WORKLOAD> bufferport;

/********************************************************************************/
/*
 * This function produces a number of work items, submits them to the buffer port
 * and then waits for processed items to return.
 */
void producer(
	std::uint32_t nProductionCycles
	, std::size_t nContainerEntries
	, std::chrono::duration<double> putTimeout
	, std::chrono::duration<double> getTimeout
	, std::size_t maxPutTimeouts
	, std::size_t maxGetTimeouts
) {
	std::size_t id;

	{ // Assign a counter to this producer
		std::unique_lock<std::mutex> lk(producer_mutex);
		id = producer_counter++;
	}

	// Initialize the counters
	std::size_t putTimeouts = 0, totalPutTimeouts = 0, highestPutTimeouts = 0;
	std::size_t getTimeouts = 0, totalGetTimeouts = 0, highestGetTimeouts = 0;
	std::uint32_t cycleCounter = 0;

	sync_ptr->wait(); // Do not start before all threads have reached this wait()

	// Submit all required items
	while(cycleCounter < nProductionCycles) {
		// Submit the WORKLOAD object
		std::shared_ptr<WORKLOAD> p_submit(new WORKLOAD(nContainerEntries));
		p_submit->set_processing_status(Gem::Courtier::processingStatus::DO_PROCESS);

		if(putTimeout.count() > 0.) {
			while(!bufferport.push_raw(
				p_submit
				, putTimeout
			)) {
				if(++putTimeouts >= maxPutTimeouts) {
					throw gemfony_exception(
						g_error_streamer(DO_LOG,  time_and_place)
							<< "In producer: Exceeded allowed number \"" << maxPutTimeouts << "\" of put timeouts in iteration " << cycleCounter << std::endl
					);
				}
			}
			totalPutTimeouts += putTimeouts;
			if(putTimeouts > highestPutTimeouts) highestPutTimeouts = putTimeouts;
			putTimeouts = 0; // Reset the counter -- we have received a valid item
		} else { // putTimeoutMS == 0
			bufferport.push_raw(p_submit);
		}

		cycleCounter++;
	}

	// Retrieve the items back. We assume that a single worker is located at the
	// other end so that we retrieve all items back
	std::uint32_t nReceived = 0;
	std::shared_ptr<WORKLOAD> p_receive;
	while(nReceived < nProductionCycles) {
		if(getTimeout.count() > 0.) {
			while(!bufferport.pop_processed(
				p_receive
				, getTimeout
			)) {
				if(++getTimeouts >= maxGetTimeouts) {
					throw gemfony_exception(
						g_error_streamer(DO_LOG,  time_and_place)
							<< "In producer: Exceeded allowed number \"" << maxGetTimeouts << "\" of get timeouts in iteration " << cycleCounter << std::endl
					);
				}
			}
			totalGetTimeouts += getTimeouts;
			if(getTimeouts > highestGetTimeouts) highestGetTimeouts = getTimeouts;
			getTimeouts = 0; // Reset the counter -- we have successfully sent the item
		} else {
			bufferport.pop_processed(p_receive);
		}

		// Check if we got a valid pointer
		if(p_receive) {
			nReceived++;
		} else {
			throw gemfony_exception(
				g_error_streamer(DO_LOG,  time_and_place)
					<< "In producer: Received invalid pointer" << std::endl
			);
		}

		// Clean the pointer for the next cycle
		p_receive.reset();
	}

	{ // Output the results
		std::unique_lock<std::mutex> lk(output_mutex);

		std::cout << "Producer " << id << " has finished producing";
		if(putTimeouts > 0 || getTimeouts > 0) {
			std::cout << " with " << totalPutTimeouts << " put time-outs (max " << highestPutTimeouts << ") and " << totalGetTimeouts << " get time-outs (max " << highestGetTimeouts << ")";
		}
		std::cout << "." << std::endl;
	}
}

/********************************************************************************/
/**
 * This function processes items it takes out of the GBufferPortT
 */
void processor (
	std::uint32_t nProductionCycles
	, std::size_t nContainerEntries
	, std::chrono::duration<double> putTimeout
	, std::chrono::duration<double> getTimeout
	, std::size_t maxPutTimeouts
	, std::size_t maxGetTimeouts
) {
	std::size_t id;

	{ // Assign a counter to this processor
		std::unique_lock<std::mutex> lk(processor_mutex);
		id = processor_counter++;
	}

	// Initialize the counters
	std::size_t putTimeouts = 0, totalPutTimeouts = 0, highestPutTimeouts = 0;
	std::size_t getTimeouts = 0, totalGetTimeouts = 0, highestGetTimeouts = 0;
	std::uint32_t cycleCounter = 0;

	sync_ptr->wait(); // Do not start before all threads have reached this wait()

	std::shared_ptr<WORKLOAD> p;
	while(cycleCounter < nProductionCycles) {
		// Retrieve an item from the buffer port
		if(getTimeout.count() > 0.) {
			while(!bufferport.pop_raw(
				p
				, getTimeout
			)){
				if(++getTimeouts >= maxGetTimeouts) {
					throw gemfony_exception(
						g_error_streamer(DO_LOG,  time_and_place)
							<< "In processor: Exceeded allowed number \"" << maxGetTimeouts << "\" of get timeouts in cycle " << cycleCounter << std::endl
					);
				}
			}
			totalGetTimeouts += getTimeouts;
			if(getTimeouts > highestGetTimeouts) highestGetTimeouts = getTimeouts;
			getTimeouts = 0; // Reset the counter, we have received a valid item
		} else {
			bufferport.pop_raw(p);
		}

		// Check that we have received a valid item
		if(p) {
			p->process();
		} else {
			throw gemfony_exception(
				g_error_streamer(DO_LOG,  time_and_place)
					<< "In processor: Received invalid pointer" << std::endl
			);
		}

		// Submit the processed item to the buffer port
		if(putTimeout.count() > 0.) {
			while(!bufferport.push_processed(
				p
				, putTimeout
			)) {
				if(++putTimeouts >= maxPutTimeouts) {
					throw gemfony_exception(
						g_error_streamer(DO_LOG,  time_and_place)
							<< "In processor: Exceeded allowed number \"" << maxPutTimeouts << "\" of put timeouts in cycle " << cycleCounter << std::endl
					);
				}
			}
			totalPutTimeouts += putTimeouts;
			if(putTimeouts > highestPutTimeouts) highestPutTimeouts = putTimeouts;
			putTimeouts = 0; // Reset the counter, we have submitted a valid item
		} else {
			bufferport.push_processed(p);
		}

		p.reset(); // Clear the pointer
		cycleCounter++;
	}

	{ // Output the results
		std::unique_lock<std::mutex> lk(output_mutex);

		std::cout << "Processor " << id << " has finished processing";
		if(putTimeouts > 0 || getTimeouts > 0) {
			std::cout << " with " << totalPutTimeouts << " put time-outs (max " << highestPutTimeouts << ") and " << totalGetTimeouts << " get time-outs (max " << highestGetTimeouts << ")";
		}
		std::cout << "." << std::endl;
	}
}

/********************************************************************************/

int main(int argc, char **argv) {
	std::uint32_t nProductionCycles;
	std::size_t nContainerEntries;
	long putTimeoutMS;
	long getTimeoutMS;
	std::size_t maxPutTimeouts;
	std::size_t maxGetTimeouts;

	//--------------------------------------------------------------------------------
	// Find out about our configuration options
	if(!parseCommandLine(
		argc, argv
		, nProductionCycles
		, nContainerEntries
		, putTimeoutMS
		, getTimeoutMS
		, maxPutTimeouts
		, maxGetTimeouts
	))
	{ exit(0); }

	//--------------------------------------------------------------------------------
	// Initialize the global barrier so all threads start at a predefined time
	sync_ptr = std::shared_ptr<Gem::Common::GBarrier>(new Gem::Common::GBarrier(1+1));

	//--------------------------------------------------------------------------------
	// Start the producer and consumer threads
	std::thread producer_thread(
		producer
		, nProductionCycles
		, nContainerEntries
		, std::chrono::microseconds(putTimeoutMS)
		, std::chrono::microseconds(getTimeoutMS)
		, maxPutTimeouts
		, maxGetTimeouts
	);

	std::thread processor_thread(
		processor
		, nProductionCycles
		, nContainerEntries
		, std::chrono::microseconds(putTimeoutMS)
		, std::chrono::microseconds(getTimeoutMS)
		, maxPutTimeouts
		, maxGetTimeouts
	);

	//--------------------------------------------------------------------------------
	// Wait for both threads to terminate
	producer_thread.join();
	processor_thread.join();

	//--------------------------------------------------------------------------------
}
