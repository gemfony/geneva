/**
 * @file GThreadPoolTest.cpp
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
#include "common/GThreadPool.hpp"
#include "common/GParserBuilder.hpp"
#include "hap/GRandomT.hpp"
#include "hap/GRandomDistributionsT.hpp"

using namespace Gem::Common;

Gem::Common::GThreadPool gtp; ///< The global threadpool

const std::size_t NRESIZEEVENTS = 0;
const std::size_t NJOBS = 100;
const std::size_t NITERATIONS = 5;
const unsigned int MINTHREADS = 1;
const unsigned int MAXTHREADS = 20;

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
		if(m_uniform_bool(m_gr)) {
			this->increment();
		} else {
			this->decrement();
		}

		std::this_thread::sleep_for(
			std::chrono::milliseconds(
				this->m_uniform_int(m_gr, std::uniform_int_distribution<long>::param_type(10, 20))
			)
		);

		if(true==simulateCrash) {
			throw gemfony_exception(
				g_error_streamer(DO_LOG,  time_and_place)
					<< "In testTask::process(): Error!" << std::endl
					<< "SHF-Exception (Some Horrible Failure)" << std::endl
					<< "occurred, as requested ..." << std::endl
			);
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
 	std::uniform_int_distribution<long> m_uniform_int;
 	std::bernoulli_distribution m_uniform_bool; // probability of 0.5 is the default
};

/************************************************************************/
/**
 * This test tries to ascertain that GThreadPool works as expected. It submits
 * a given number of jobs to the pool, waits for their execution and submits them again
 * a user-definable number of times.
 *
 * TODO: Extract futures and check for errors
 */
int main(int argc, char** argv) {
	Gem::Hap::GRandom gr; // Instantiates a random number generator
	std::uniform_int_distribution<unsigned int> m_uniform_int;

	//----------------------------------------------------------------
	// Local variables
	bool simulateThreadCrash = false;
	std::size_t nResizeEvents = NRESIZEEVENTS;
	std::size_t nJobs = NJOBS; // The number of tasks in each iteration
	std::size_t nIterations = NITERATIONS; // The default number of iterations
	bool showCLOptions = false; // When set to true, will show a summary of command line options

	//----------------------------------------------------------------
	// Create the parser builder -- needed for command line parsing
	Gem::Common::GParserBuilder gpb;

	// Register some command line options
	gpb.registerCLParameter<std::size_t>(
		"nJobs,j"
		, nJobs
		, NJOBS
	) << "The number of testTask objects on which work is performed";

	gpb.registerCLParameter<std::size_t>(
		"nIterations,i"
		, nIterations
		, NITERATIONS
	) << "The number of test iteration";

	gpb.registerCLParameter<std::size_t>(
		"nResizeEvents,r"
		, nResizeEvents
		, NRESIZEEVENTS
	)
	<< "Tests random resizing of the thread pool \"nResizeEvents\" times";

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
	// Start measurements

	// Create a number of test tasks
	std::vector<std::shared_ptr<testTask>> tasks(nJobs);
	for(std::size_t i=0; i<nJobs; i++) {
		tasks.at(i).reset(new testTask);
	}

	// Submit each task to the pool a number of times
	double resizeLikelihood=(std::min)(double(nResizeEvents)/double(nIterations),1.);
	std::bernoulli_distribution weighted_bool(resizeLikelihood);

	for(std::size_t n = 0; n<nIterations; n++) {
		// Submission number n
		for(std::size_t i=0; i<nJobs; i++) {
			bool stc = false;
			if(i==nJobs-1 && n==nIterations-1 && true==simulateThreadCrash) {
				stc = true;
			}

			gtp.async_schedule(
				[&tasks,i,stc](){ (tasks.at(i))->process(stc); }
			);
		}

		if(nResizeEvents > 0 && weighted_bool(gr)) {
			unsigned int nt = m_uniform_int(gr, std::uniform_int_distribution<unsigned int>::param_type(MINTHREADS,MAXTHREADS));
			gtp.setNThreads(nt);

			glogger
			<< "Resized thread pool to size " << nt << std::endl
			<< GLOGGING;
		}

		// Wait for all tasks to complete and check for errors
		gtp.wait();
	}

	// Check that each task has been called exactly nIterations times
	for(std::size_t i=0; i<nJobs; i++) {
		if(nIterations != (tasks.at(i))->getOperatorCalledValue()) {
			glogger
			<< "In task " << i << ":" << std::endl
			<< "Got wrong number of calls: " << (tasks.at(i))->getOperatorCalledValue() << "." << std::endl
			<< GLOGGING;
		}
	}
}
