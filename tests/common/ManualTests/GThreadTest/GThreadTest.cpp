/**
 * @file GThreadTest.cpp
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

// Standard headers go here
#include <cmath>
#include <iostream>
#include <random>
#include <thread>
#include <chrono>

// Boost headers go here

// Geneva headers go here
#include "common/GLogger.hpp"
#include "common/GParserBuilder.hpp"
#include "common/GThread.hpp"
#include "hap/GRandomT.hpp"
#include "hap/GRandomDistributionsT.hpp"

using namespace Gem::Common;

const std::size_t NTHREADS = 4;
const std::size_t NSECONDS = 10;

/************************************************************************/
/**
 * A simple test task that sets a flag when the operator() has been called
 */
class testTask {
public:
	/********************************************************************/
	/**
	 * The default constructor
	 */
	testTask()
		: m_counterValue(0)
		, m_operatorCalled(0)
	{ /* nothing */ }

	/********************************************************************/
	/**
	 * Allows to check how often increment() has been called
	 */
	std::int32_t getCounterValue() const {
		return m_counterValue;
	}

	/********************************************************************/
	/**
	 * Retrieves the number of operator calls
	 */
	std::uint32_t getOperatorCalledValue() const {
		return m_operatorCalled;
	}

	/********************************************************************/
	/**
	 * Performs work on this object. This is the function to be executed
	 * inside of the threads
	 */
	void process(bool simulateCrash) {
		// Loop until we are interrupted
		while(true) {
			// Check whether we have been interrupted. This function will
			// throw an exception which will be caught by the surrounding
			// Gem::Common::thread implementation.
			Gem::Common::thread::interruption_point();

			if (m_uniform_bool(m_gr)) {
				this->increment();
			} else {
				this->decrement();
			}

			std::this_thread::sleep_for(
				std::chrono::milliseconds(
					this->m_uniform_int(10, 20)
				)
			);

			if (true == simulateCrash) {
				glogger
					<< "In testTask::process(): Error!" << std::endl
					<< "SHF-Exception (Some Horrible Failure)" << std::endl
					<< "occurred, as requested ..." << std::endl
					<< GEXCEPTION;
			}
		}
	}

private:
	/********************************************************************/
	/**
	 * Increments the local counter
	 */
	void increment() {
		m_counterValue++;
		m_operatorCalled++;
	}

	/********************************************************************/
	/**
	 * Decrements the local counter
	 */
	void decrement() {
		m_counterValue--;
		m_operatorCalled++;
	}

	/********************************************************************/
	std::int32_t m_counterValue; ///< The internal value to be decremented or incremented
	std::uint32_t m_operatorCalled; ///< This counter will be incremented whenever process() is called

	Gem::Hap::GRandom m_gr; // Instantiates a random number generator
   Gem::Hap::g_uniform_int<long> m_uniform_int;

 	std::bernoulli_distribution m_uniform_bool; // probability of 0.5 is the default
};

/************************************************************************/
/**
 * This test tries to ascertain that GThreadPool works as expected. It submits
 * a given number of jobs to the pool, waits for their execution and submits them again
 * a user-definable number of times. The user may ask the application to resize
 * the thread pool in random intervals and
 */
int main(int argc, char** argv) {
	Gem::Hap::GRandom gr; // Instantiates a random number generator
	Gem::Hap::g_uniform_int<unsigned int> m_uniform_int;

	//----------------------------------------------------------------
	// Local variables
	bool simulateThreadCrash = false;
	std::size_t nThreads = NTHREADS; // The number of threads to be started
	std::size_t nSeconds = NSECONDS; // The number of seconds after which threads should be terminated
	bool showCLOptions = false; // When set to true, will show a summary of command line options

	//----------------------------------------------------------------
	// Create the parser builder -- needed for command line parsing
	Gem::Common::GParserBuilder gpb;

	// Register some command line options
	gpb.registerCLParameter<std::size_t>(
		"nThreads,n"
		, nThreads
		, NTHREADS
	) << "The number of threads to be started";

	gpb.registerCLParameter<std::size_t>(
		"nSeconds,i"
		, nSeconds
		, NSECONDS
	) << "The number of test iterations";

	gpb.registerCLParameter<bool>(
		"simulateThreadCrash,s"
		, simulateThreadCrash
		, false // the default value
		, "When set to true, simulates the crash of a single thread"
		, true // implicit allowed (i.e. "-s" without argument)
		, true // the implicit value
	);

	gpb.registerCLParameter<bool>(
		"showCLOptions,o"
		, showCLOptions
		, false // the default value
		, "When set to true, shows a summary of command line options"
		, true // implicit allowed (i.e. "-o" without argument)
		, true // the implicit value
	);

	// Parse the command line and leave if the help flag was given
	if(Gem::Common::GCL_HELP_REQUESTED == gpb.parseCommandLine(argc, argv, showCLOptions)) {
		return 0;
	}

	//----------------------------------------------------------------
	// Create a number of test tasks and threads
	std::vector<std::shared_ptr<testTask>> tasks(nThreads);
	std::vector<Gem::Common::thread> threads(nThreads);
	for(std::size_t i=0; i<nThreads; i++) {
		tasks.at(i).reset(new testTask);
		threads.at(i) = Gem::Common::thread(
			[tasks,i,simulateThreadCrash](){ (tasks.at(i))->process(simulateThreadCrash); }
		);
	}

	// Sleep for the predefined number of seconds
	std::cout << "Main thread sleeps for " << nSeconds << " seconds" << std::endl;
	std::this_thread::sleep_for(std::chrono::seconds(nSeconds));

	// Interrupt all threads and wait for their return
	for(auto& t: threads) {
		t.interrupt();
		t.join();
	}

	// Retrieve some information on the tasks that were executed
	for(auto task_ptr: tasks) {
		std::cout << task_ptr->getCounterValue() << " / " << task_ptr->getOperatorCalledValue() << std::endl;
	}
}
