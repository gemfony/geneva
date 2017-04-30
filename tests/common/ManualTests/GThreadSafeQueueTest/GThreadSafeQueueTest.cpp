/**
 * @file GThreadSafeQueueTest.cpp
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
#include "common/GThreadSafeQueueT.hpp"
#include "hap/GRandomT.hpp"
#include "hap/GRandomDistributionsT.hpp"

using namespace Gem::Common;

Gem::Common::GThreadPool gtp; ///< The global threadpool

const std::size_t NRESIZEEVENTS = 0;
const std::size_t NJOBS = 100;
const std::size_t NITERATIONS = 5;



/************************************************************************/

int main(int argc, char** argv) {
	Gem::Common::GThreadSafeQueueT<int,500> gtsq;

	for(int i=0; i<5000000; i++){
		// Submit the item to the queu
		if(!gtsq.push_and_wait(i, std::chrono::milliseconds(50))) {
			glogger
			<< "I GThreadSafeQueueTest: Submission of item " << i << " failed" << std::endl
			<< GEXCEPTION;
		}

		// Retrieve the same item again
		int retrieved_value = 0;
		if(!gtsq.pop_and_wait(retrieved_value, std::chrono::milliseconds(50))) {
			glogger
			<< "I GThreadSafeQueueTest: Retrieval of item " << i << " failed" << std::endl
			<< GEXCEPTION;
		} else {
			if(i != retrieved_value) {
				glogger
				<< "I GThreadSafeQueueTest: i (" << i << ") != retrieved_value (" << retrieved_value << ")" << std::endl
				<< GEXCEPTION;
			}
		}
	}
}
