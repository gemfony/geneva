/**
 * @file
 */

/* GBoostThreadConsumer.cpp
 *
 * Copyright (C) 2004-2008 Dr. Ruediger Berlich
 * Copyright (C) 2007-2008 Forschungszentrum Karlsruhe GmbH
 *
 * This file is part of Geneva, Gemfony scientific's optimization library.
 *
 * Geneva is free software: you can redistribute it and/or modify
 * it under the terms of version 3 of the GNU Affero General Public License
 * as published by the Free Software Foundation.
 *
 * Geneva is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with the Geneva library.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "GBoostThreadConsumer.hpp"

namespace Gem {
namespace GenEvA {

/***************************************************************/
/**
 * The default constructor. Nothing special here.
 */
GBoostThreadConsumer::GBoostThreadConsumer()
	:GConsumer<GIndividual>(GINDIVIDUALBROKER),
	 maxThreads_(DEFAULTGBTCMAXTHREADS),
	 tp_(maxThreads_)
{ /* nothing */ }

/***************************************************************/
/**
 * Standard destructor. Nothing - our thread is interrupted by the
 * broker, and GConsumer<GIndividual>::process() catches the exception.
 */
GBoostThreadConsumer::~GBoostThreadConsumer()
{ /* nothing */ }

/***************************************************************/
/**
 * The actual business logic. We retrieve work items from the
 * GMemberBroker, process them with the help of a thread pool
 * and put processed items back into the broker. BROKER.get() or
 * BROKER.put() will throw an exception if our thread has been
 * interrupted. This terminates the while() loop. The exception
 * is caught in GConsumer::process(). GBoostThreadConsumer::finally()
 * then clears the remaining tasks in our queue and waits for the
 * termination of the remaining threads.
 */
void GBoostThreadConsumer::customProcess() {


	boost::uint16_t counter = 0;

	while(true){

		counter=0;

		// Start a predefined number of threads
		while (counter++ < this->getMaxThreads()) {
			boost::shared_ptr<GIndividual> p;
			boost::uint32_t id = GINDIVIDUALBROKER.get(p);
			p->setAttribute("key",boost::lexical_cast<string>(id));
			tp_.schedule(boost::bind(&GIndividual::checkedFitness, p));
		}

		// Wait for the threads to finish
		tp_.wait();

	}

	typedef multimap<string, shared_ptr<GMemberCarrier> , less<string> >
			storeMultiMap;
	bool timeout = false;
	bool success = false;
	storeMultiMap store;
	storeMultiMap::iterator it;
	shared_ptr<GMemberCarrier> p; // temporary carrier storage
	uint16_t counter = 0;

	while (!stopConditionReached()) {
		counter = 0;

		while (counter++ < this->getMaxThreads()) {
			success = BROKER.get(p, timeout);

			if (!success || timeout)
				break; // no items retrieved or timeout
			store.insert(storeMultiMap::value_type(p->getId(), p));
			tp_.schedule(boost::bind(&GMemberCarrier::process, p.get()));
		}

		// wait for threads to finish
		tp_.wait();

		// Put the processed GMemberCarrier objects back into the broker
		for (it = store.begin(); it != store.end(); ++it) {
			if (!BROKER.put(it->second)) {

				// tried to put an unknown key back - needs to be logged ...
				GLogStreamer gls;
				gls
						<< "In GBoostThreadConsumer::operator()(GConsumer * const) : Warning!"
						<< endl << "Could not submit item." << endl << logLevel(UNCRITICAL);
			}
		}

		// Empty the store
		store.clear();
	}
}

/***************************************************************/
/**
 * The function that gets new items from the broker, processes them
 * and returns them when finished.
 */
void GBoostThreadConsumer::processItems(){

}

/***************************************************************/
/**
 * Finalization code. Removes all pending tasks and waits for the
 * termination of the remaining threads.
 */
void GBoostThreadConsumer::finally()
{

}

/***************************************************************/
/**
 * Sets the maximum number of threads and adjusts the thread
 * pool accordingly.
 *
 * @param maxThreads The maximum number of allowed threads
 */
void GBoostThreadConsumer::setMaxThreads(boost::uint16_t maxThreads) {
	maxThreads_ = maxThreads;
	tp_.size_controller().resize(maxThreads);
}

/***************************************************************/
/**
 * Retrieves the maximum number of allowed threads
 *
 * @return The maximum number of allowed threads
 */
uint16_t GBoostThreadConsumer::getMaxThreads(void) const throw() {
	return maxThreads_;
}

/***************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */
