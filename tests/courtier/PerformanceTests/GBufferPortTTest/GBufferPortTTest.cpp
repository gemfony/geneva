/**
 * @file GBufferPortTTest.cpp
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

#include <vector>
#include <mutex>
#include <chrono>
#include <thread>

#include "courtier/GBufferPortT.hpp"
#include "common/GExceptions.hpp"
#include "common/GThreadGroup.hpp"
#include "common/GBarrier.hpp"

#include "../../Misc/GSimpleContainer.hpp"
#include "../../Misc/GRandomNumberContainer.hpp"

#define WORKLOAD GSimpleContainer
// #define WORKLOAD GRandomNumberContainer

#include "GArgumentParser.hpp"

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
using namespace Gem::Courtier::Tests;

/********************************************************************************/
/**
 * A global buffer port, to/from which WORKLOAD objects are written/read. We store
 * smart pointers instead of the objects themselves.
 */
GBufferPortT<std::shared_ptr<WORKLOAD>> bufferport;

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
		if(putTimeout.count() > 0.) {
			while(!bufferport.push_front_orig_bool(p_submit, putTimeout)) {
				if(++putTimeouts >= maxPutTimeouts) {
					raiseException("In producer: Exceeded allowed number \"" << maxPutTimeouts << "\" of put timeouts in iteration " << cycleCounter << std::endl);
				}
			}
			totalPutTimeouts += putTimeouts;
			if(putTimeouts > highestPutTimeouts) highestPutTimeouts = putTimeouts;
			putTimeouts = 0; // Reset the counter -- we have received a valid item
		} else { // putTimeoutMS == 0
			bufferport.push_front_orig(p_submit);
		}

		cycleCounter++;
	}

	// Retrieve the items back. We assume that a single worker is located at the
	// other end so that we retrieve all items back
	std::uint32_t nReceived = 0;
	std::shared_ptr<WORKLOAD> p_receive;
	while(nReceived < nProductionCycles) {
		if(getTimeout.count() > 0.) {
			while(!bufferport.pop_back_processed_bool(p_receive, getTimeout)) {
				if(++getTimeouts >= maxGetTimeouts) {
					raiseException("In producer: Exceeded allowed number \"" << maxGetTimeouts << "\" of get timeouts in iteration " << cycleCounter << std::endl);
				}
			}
			totalGetTimeouts += getTimeouts;
			if(getTimeouts > highestGetTimeouts) highestGetTimeouts = getTimeouts;
			getTimeouts = 0; // Reset the counter -- we have successfully sent the item
		} else {
			bufferport.pop_back_processed(p_receive);
		}

		// Check if we got a valid pointer
		if(p_receive) {
			nReceived++;
		} else {
			raiseException("In producer: Received invalid pointer" << std::endl);
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
			while(!bufferport.pop_back_orig_bool(p, getTimeout)){
				if(++getTimeouts >= maxGetTimeouts) {
					raiseException("In processor: Exceeded allowed number \"" << maxGetTimeouts << "\" of get timeouts in cycle " << cycleCounter << std::endl);
				}
			}
			totalGetTimeouts += getTimeouts;
			if(getTimeouts > highestGetTimeouts) highestGetTimeouts = getTimeouts;
			getTimeouts = 0; // Reset the counter, we have received a valid item
		} else {
			bufferport.pop_back_orig(p);
		}

		// Check that we have received a valid item
		if(p) {
			p->process();
		} else {
			raiseException("In processor: Received invalid pointer" << std::endl);
		}

		// Submit the processed item to the buffer port
		if(putTimeout.count() > 0.) {
			while(!bufferport.push_front_processed_bool(p, putTimeout)) {
				if(++putTimeouts >= maxPutTimeouts) {
					raiseException("In processor: Exceeded allowed number \"" << maxPutTimeouts << "\" of put timeouts in cycle " << cycleCounter << std::endl);
				}
			}
			totalPutTimeouts += putTimeouts;
			if(putTimeouts > highestPutTimeouts) highestPutTimeouts = putTimeouts;
			putTimeouts = 0; // Reset the counter, we have submitted a valid item
		} else {
			bufferport.push_front_processed(p);
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
