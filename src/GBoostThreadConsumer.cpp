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
	:GConsumer<GIndividual>(),
	 maxThreads_(DEFAULTGBTCMAXTHREADS),
	 tp_(maxThreads_) { /* nothing */
}

/***************************************************************/
/**
 * Standard destructor. Nothing - our thread is interrupted by the
 * broker, and GConsumer<GIndividual>::process() catches the exception.
 */
GBoostThreadConsumer::~GBoostThreadConsumer()
{ /* nothing */ }

/***************************************************************/
/**
 * Initialization code. As we only start threads locally, unlike
 * e.g. an MPI consumer we do not have to do any work here.
 */
void GBoostThreadConsumer::init()
{ /* nothing */ }

/***************************************************************/
/**
 * The actual business logic. We retrieve work items from the
 * GMemberBroker, process them with the help of a thread pool
 * and put processed items back into the broker.
 */
void GBoostThreadConsumer::customProcess() {
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
 * Finalization code. As we operate with threads in this class,
 * there is no work to be done here.
 */
void GBoostThreadConsumer::finally()
{ /* nothing */ }

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
