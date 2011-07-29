/**
 * @file GBoundedBufferPerformance.hpp
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt)
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) com
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
 * http://www.gemfony.com .
 */

// Standard headers go here

// Boost headers go here
#include <boost/cstdint.hpp>
#include <boost/date_time.hpp>

// Geneva headers go here
#include "common/GBoundedBufferT.hpp"
#include "hap/GRandomT.hpp"

// Local headers
#include "GArgumentParser.hpp"

using namespace Gem::Common;
using namespace Gem::Common::Testing;

/*****************************************************************/
/**
 * The global thread-safe queue
 */
GBoundedBufferT<double> buffer;

/**
 * A counter for dropped items
 */
std::vector<std::size_t> droppedCounter;

/**
 * An id to be assigned to each producer
 */
std::size_t idCounter;

/**
 * A mutex that protects "everything producer"
 */
boost::mutex producerMutex;

/*****************************************************************/
/**
 * This function produces double random numbers and adds them to
 * a global queue
 *
 * @param nItems The amount of random numbers to be produced
 */
void producer(
		const std::size_t& nItems
		, const boost::date_time& timeout
		, const boost::date_time& maxRandomDelay
) {
	std::size_t id = 0;
	std::size_t nDropped = 0;

	// A random number generator
	Gem::Hap::GRandomT<Gem::Hap::RANDOMPROXY> gr;

	// Find out about the maximum random delay in microseconds
	long maxRandomDelayMS = maxRandomDelay.total_microseconds();

	// Find out about the id of this producer
	{
		boost::mutex::scoped_lock lk(producerMutex);
		id = idCounter++;
	}

	if(maxRandomDelay.total_microseconds() == 0) {
		if(timeout.total_microseconds() == 0) {
			for(std::size_t i=0; i<nItems; i++) {
				buffer.push_front(gr.uniform_01<double>());
			}
		} else { // We use a timeout for push_fronts
			for(std::size_t i=0; i<nItems; i++) {
				if(!push_front_bool(gr.uniform_01<double>(), timeout)) {
					nDropped++;
				}
			}
		}
	} else { // We use a random delay in between submissions
		if(timeout.total_microseconds() == 0) {
			for(std::size_t i=0; i<nItems; i++) {
				boost::this_thread::sleep(gr.uniform_int(long(0),maxRandomDelayMS));
				buffer.push_front(gr.uniform_01<double>());
			}
		} else {
			for(std::size_t i=0; i<nItems; i++) {
				boost::this_thread::sleep(gr.uniform_int(long(0),maxRandomDelayMS));
				if(!buffer.push_front_bool(gr.uniform_01<double>(), timeout)) {
					nDropped++;
				}
			}
		}
	}

	// Update the drop counter
	{ // Explicit scope
		boost::mutex::scoped_lock lk(producerMutex);
		droppedCounter.at(id) = nDropped;
	}
}

/*****************************************************************/


int main(int argc, char**argv) {
	// Read the program options

	// Initialize the counters
	idCounter = 0;
	droppedCounter.resize(nProducers);

	// Initialize the counter with 0s
	for(std::size_t i=0; i<nProducers; i++) {
		droppedCounter[i] = 0;
	}
}
