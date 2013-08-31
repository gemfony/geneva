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

// Boost headers go here
#include <boost/cstdint.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>

// Geneva headers go here
#include "common/GThreadPool.hpp"

using namespace Gem::Common;

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
	testTask()	:operatorCalled_(0)
	{ /* nothing */ }

	/********************************************************************/
	/**
	 * The function to be executed in each thread
	 */
	void increment() {
		operatorCalled_++;
	}

	/********************************************************************/
	/**
	 * Allows to check how often increment() has been called
	 */
	boost::uint32_t getNCalls() const {
		return operatorCalled_;
	}

private:
	/********************************************************************/
	boost::uint32_t operatorCalled_; ///< This counter will be incremented whenever operator() is called
};

/************************************************************************/

const std::size_t NJOBS = 100;
const std::size_t NSUBMISSIONS = 5;

int main(int argc, char** argv) {
	Gem::Common::GThreadPool gtp;

	// Create a number of test tasks
	std::vector<boost::shared_ptr<testTask> > tasks(NJOBS);
	for(std::size_t i=0; i<NJOBS; i++) {
		tasks.at(i).reset(new testTask);
	}

	// Submit each task to the pool a number of times
	for(std::size_t n = 0; n<NSUBMISSIONS; n++) {
		// Submission number n
		for(std::size_t i=0; i<NJOBS; i++) {
			gtp.schedule(boost::function<void()>(boost::bind(&testTask::increment, tasks.at(i))));
		}

		// Wait for all tasks to complete and check for errors
		if(!gtp.wait()) {
			std::cout
			<< "Errors occurred during the execution" << std::endl;
		}
	}

	// Check that each task has been called exactly NSUBMISSIONS times
	for(std::size_t i=0; i<NJOBS; i++) {
		if(NSUBMISSIONS != (tasks.at(i))->getNCalls()) {
			std::cout
			<< "In task " << i << ":" << std::endl
			<< "Got wrong number of calls: " << (tasks.at(i))->getNCalls() << "." << std::endl;
		}
	}
}
