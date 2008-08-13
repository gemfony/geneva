/**
 * @file
 */

/* GBrokerPopulation.cpp
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

#include "GBrokerPopulation.hpp"

/**
 * Included here so no conflicts occur. See explanation at
 * http://www.boost.org/libs/serialization/doc/special.html#derivedpointers
 */
#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT(GenEvA::GBrokerPopulation)

namespace Gem
{
namespace GenEvA
{

/******************************************************************************/
/**
 * The default constructor. A global GMemberBroker object is available
 * through the BROKER define (which in turn calls gbroker.instance() ,
 * referring to a singleton).
 */
GBrokerPopulation::GBrokerPopulation() :
	GBasePopulation() {
	waitFactor_ = DEFAULTWAITFACTOR;
	firstTimeOut_
			= boost::posix_time::duration_from_string(DEFAULTFIRSTTIMEOUT);
	setLoopTime(DEFAULTLOOPSEC, DEFAULTLOOPMSEC);

	// Note that _2 comes after _1 , as false < true and parents should
	// go to the beginning of the array.
	f_isparent_ = boost::bind(std::less<bool>(), boost::bind(&GMember::isParent,
			_2), boost::bind(&GMember::isParent, _1));
}

/******************************************************************************/
/**
 * The standard copy constructor
 *
 * \param cp A copy of another GBrokerPopulation object
 */
GBrokerPopulation::GBrokerPopulation(const GBrokerPopulation& cp) :
	GBasePopulation(cp) {
	waitFactor_ = cp.waitFactor_;
	firstTimeOut_ = cp.firstTimeOut_;
	loopMSec_ = cp.loopMSec_;
	loopSec_ = cp.loopSec_;
	f_isparent_ = cp.f_isparent_;
}

/******************************************************************************/
/**
 * The standard destructor. We have no local, dynamically allocated data, hence
 * this function is empty.
 */
GBrokerPopulation::~GBrokerPopulation()
{ /* nothing */}

/******************************************************************************/
/**
 * A standard assignment operator for GBrokerPopulation objects,
 *
 * \param cp A copy of another GBrokerPopulation object
 * \return A constant reference to this object
 */
const GBrokerPopulation& GBrokerPopulation::operator=(const GBrokerPopulation& cp) {
	GBrokerPopulation::load(&cp);
	return *this;
}

/******************************************************************************/
/**
 * Loads the data of another GBrokerPopulation object, camouflaged as a
 * pointer to a GObject
 *
 * \param cp A pointer to another GBrokerPopulation object, camouflaged as a GObject
 */
void GBrokerPopulation::load(const GObject * cp) {
	// Check that this object is not accidently assigned to itself.
	// As we do not actually do any calls with this pointer, we
	// can use the faster static_cast<>
	if(static_cast<const GBrokerPopulation *>(cp) == this) {
		GException ge;
		ge << "In GBrokerPopulation::load(): Error!" << endl
		<< "Tried to assign an object to itself." << endl
		<< raiseException;
	}

	GBasePopulation::load(cp);

	const GBrokerPopulation * gtp = dynamic_cast<const GBrokerPopulation *>(cp);
	if(!gtp) {
		GException ge;
		ge << "In GBrokerPopulation::load() : Conversion Error!" << endl
		<< raiseException;
	}

	waitFactor_=gtp->waitFactor_;
	firstTimeOut_=gtp->firstTimeOut_;

	setLoopTime(gtp->loopSec_, gtp->loopMSec_);

	f_isparent_ = gtp->f_isparent_;
}

/******************************************************************************/
/**
 * Creates a deep copy of this object
 *
 * \return A deep copy of this object
 */
GObject *GBrokerPopulation::clone() {
	return new GBrokerPopulation(*this);
}

/******************************************************************************/
/**
 * Sets the waitFactor_ variable. This population measures the time until the
 * first individual has returned. This time times the waitFactor_ variable is
 * then used to check whether a timeout was reached for other individuals.
 * waitFactor_ is by default set to DEFAULTWAITFACTOR. You can disable this
 * timeout by setting waitFactor_ to 0.
 *
 * \param waitFactor The desired new value for waitFactor_ .
 */
void GBrokerPopulation::setWaitFactor(boost::uint16_t waitFactor) {
	waitFactor_ = waitFactor;
}

/******************************************************************************/
/**
 * Retrieves the waitFactor_ variable.
 *
 * \return The value of the waitFactor_ variable
 */
boost::uint16_t GBrokerPopulation::getWaitFactor() const {
	return waitFactor_;
}

/******************************************************************************/
/**
 * Sets the maximum turn-around time for the first individual. When this time
 * has passed, an exception will be raised. If set to 0, this timeout is disabled.
 * The default value is DEFAULTFIRSTTIMEOUT (likely 0.).
 *
 * \param d The maximum turn-around time in days for the first individual in generation 0
 * \param h The maximum turn-around time in hours for the first individual in generation 0
 * \param m The maximum turn-around time in minutes for the first individual in generation 0
 * \param s The maximum turn-around time in seconds for the first individual in generation 0
 */
void GBrokerPopulation::setFirstTimeOut(boost::int32_t d, boost::int32_t h, boost::int32_t m, boost::int32_t s) {
	using namespace boost::posix_time;
	firstTimeOut_ = hours(d*24+h) + minutes(m) + seconds(s);
}

/******************************************************************************/
/**
 * Retrieves the number of seconds allowed for the return of the first individual
 *
 * \return The value of firstTimeOut_ in seconds
 */
boost::int32_t GBrokerPopulation::getFirstTimeOut() const {
	return firstTimeOut_.total_seconds();
}

/******************************************************************************/
/**
 * When retrieving items from the GBoundedBuffer queue (which in turn is accessed through
 * the GBroker interface), a time-out factor can be set with this function. The
 * default values are DEFAULTLOOPSEC and DEFAULTLOOPMSEC. Values are in seconds
 * and milli seconds respectively.
 *
 * Check what needs to be done with the broker in case of an exception ...
 * BROKER.shutdown() ???
 *
 * \param loopSec  Timeout in seconds
 * \param loopMSec Timeout in milli seconds
 */
void GBrokerPopulation::setLoopTime(boost::uint16_t loopSec, boost::uint16_t loopMSec) {
	// Only allow "real" values
	if(loopSec==0 && loopMSec==0) {
		GException ge;
		ge << "In GBrokerPopulation::setLoopTime() : Error!" << endl
		<< "Both seconds and milli-seconds are set to 0" << endl
		<< raiseException;
	}

	loopSec_ = loopSec;
	loopMSec_ = loopMSec;
}

/******************************************************************************/
/**
 * Retrieves the second part of the GBoundedBuffer timeout factor
 *
 * \return seconds before GBoundedBuffer::get() times out
 */
boost::uint16_t GBrokerPopulation::getLoopSec() const {
	return loopSec_;
}

/******************************************************************************/
/**
 * Retrieves the milli second part of the GBoundedBuffer timeout factor
 *
 * \return milli seconds before GBoundedBuffer::get() times out
 */
boost::uint16_t GBrokerPopulation::getLoopMSec() const {
	return loopMSec_;
}

/******************************************************************************/
/**
 * We make ourselves known to the broker. Next the standard optimization cycle
 * of the parent population is started. When it has finished, the broker is informed
 * that it can remove the population from its lists. Note that we explicitly disallow
 * lazy evaluation in all contexts. This means that, whenever mutate() is called
 * for a GMember object, the value of that object is updated instantly.
 */
void GBrokerPopulation::optimize() {
	std::string id = GBasePopulation::getId();

	BROKER.enrol(id);
	CurrentGBiBufferPtr_ = BROKER.at(id);
	// Check that a GBiBuffer with this key actually exists ...
	if(!CurrentGBiBufferPtr_) {
		GException ge;
		ge << "In GBrokerPopulation::optimize() : Error!" << endl
		<< "Could not retrieve GBiBuffer" << endl
		<< raiseException;
	}

	boost::uint8_t originalEvaluationPermission = GManagedMemberCollection::setEvaluationPermission(PREVENTEVALUATION);
	GBasePopulation::optimize();
	GManagedMemberCollection::setEvaluationPermission(originalEvaluationPermission);

	BROKER.signoff(id);
	CurrentGBiBufferPtr_.reset();
}

/******************************************************************************/
/**
 * Starting from the end of the children's list, we wrap the GMember objects
 * into GMemberCarrier objects and tell them the kind of command we wish to have
 * executed. We use a unique identifier to put the GMemberCarriers into the GMemberBroker
 * for further processing. Once this has been done for all children, we wait for
 * the first child to come back and measure the time that has passed. We then wait
 * a predefined amount of time (by default up to three times the time of the first
 * child) for further children to arrive. It is expected that some individuals do not
 * return (due to network problems, crashed clients, ...). This is remedied in
 * the GBrokerPopulation::select() function.
 */
void GBrokerPopulation::mutateChildren() {
	boost::uint16_t np = getNParents(), nc = getNChildren();
	boost::uint32_t generation = GBasePopulation::getGeneration();
	std::vector<boost::shared_ptr<GMember> >::reverse_iterator rit;
	std::vector<boost::shared_ptr<GMember> >::iterator it;
	std::string id = GBasePopulation::getId();
	boost::uint16_t nParentsSentAway = 0;

	// First we send all individuals abroad

	// Start with the children from the back of the population
	// This is the same for MUPLUSNU and MUCOMMANU mode
	for(rit=this->rbegin(); rit!=this->rbegin()+nc; ++rit) {
		GMemberCarrier *gmc = new GMemberCarrier(*rit,"mutate", id, generation, false);
		shared_ptr<GMemberCarrier> p(gmc);
		CurrentGBiBufferPtr_->push_front_orig(p); // let's get rid of it
	}

	// We can remove children, so only parents remain in the population
	this->resize(np);

	// Make sure we also evaluate the parents in the first generation, if needed.
	// This is only applicable to the MUPLUSNU mode.
	if(generation==0 && getSortingScheme()==MUPLUSNU) {
		// We might need to temporary store some parent objects
		std::vector<boost::shared_ptr<GMember> > tempParents;

		// Note that we only have parents left in this generation
		for(rit=this->rbegin(); rit!=this->rend(); ++rit) {
			if((*rit)->isDirty()) {
				// Last "true" in the GMemberCarrier constructor
				// means: This is a parent individual!
				GMemberCarrier *gmc = new GMemberCarrier(*rit,"evaluate", id, generation, true);
				shared_ptr<GMemberCarrier> p(gmc);
				CurrentGBiBufferPtr_->push_front_orig(p); // let's get rid of it
				nParentsSentAway++; // We need to count how many parents were sent away
			}
			else { // clean, let's store it
				tempParents.push_back(*rit);
			}
		}

		// Next we clear the population. Some members are now stored in
		// GMemberCarrier objects and the clean members are in the tempParents
		// vector, so we do not loose anything (unless of course there are any
		// failures, such as stalled connections, etc.).
		this->clear();

		// Now we move the items in the tempParents vector back
		for(it=tempParents.begin(); it!=tempParents.end(); ++it) this->push_back(*it);
		// Nothing is lost as we are dealing with shared_ptr objects.
		tempParents.clear();
	}

	// If we are running in MUPLUSNU mode, we now have a clean set of parents in the
	// population. "Dirty" parents have been sent away for evaluation. If this is the
	// MUCOMMANU mode, parents do not participate in the sorting and can be ignored.
	// We can now wait for individuals to return from their journey.

	// First we wait for the first individual from the current generation to arrive
	// or until a timeout has been reached. Individuals from older generations will
	// alsobe accepted in this loop. If firstTimeOut_ is set to 0, we do not timeout.
	shared_ptr<GMemberCarrier> p;
	bool ok = false;
	boost::uint16_t nReceivedCurrent = 0;
	boost::uint16_t nReceivedOlder = 0;

	// start to measure time. Uses the Boost.date_time library
	boost::posix_time::ptime startTime = boost::posix_time::microsec_clock::local_time();
	boost::posix_time::time_duration totalElapsedFirst;
	bool foundFirst=false;
	while(true) {
		try {
			CurrentGBiBufferPtr_->pop_back_processed(&p,getLoopSec(), getLoopMSec());
			ok = true;
		}
		catch(time_out& t) { /* nothing */ }

		if(ok) { // p now contains a GMemberCarrier
			// is this a member of the current generation ?
			if(p->getGeneration() == generation) {
				nReceivedCurrent++;
				foundFirst=true;
			}
			else {
				nReceivedOlder++;
			}

			// store first individual in the list
			this->push_back(p->payload());

			// Is this an individual from the current generation ? Then we do not
			// need to continue with the loop. Otherwise we want to continue until
			// the first individual from the current generation has been received.
			if(foundFirst) {
				// note the time it took until the first individual has returned
				boost::posix_time::ptime firstTime = boost::posix_time::microsec_clock::local_time();
				totalElapsedFirst = (firstTime - startTime);

				// make sure the duration is at least one second
				if(totalElapsedFirst.total_seconds() == 0)
				totalElapsedFirst += boost::posix_time::seconds(1);

				break; // we can stop the loop
			}
		}

		// No individual from current generation received.
		// Find out whether we have exceeded the timeout
		boost::posix_time::ptime firstTimeMissed = boost::posix_time::microsec_clock::local_time();

		// Check what can be done with the broker in case of an error. BROKER.shutdown() ???
		// Is a maximum allowed time set ? If so: have we exceeded this allowed time ?
		if(getFirstTimeOut() && ((firstTimeMissed-startTime)> firstTimeOut_)) {
			GException ge;
			ge << "In GBrokerPopulation::mutateChildren() : Error!" << endl
			<< "Timeout for first individual reached." << endl
			<< raiseException;
		}
	}

	// Great. Now we can wait for further arrivals, until waitFactor_*totalFirstElapsed has been reached
	// or until all members of this generation have returned. Please note that we do allow late arrivals
	// from earlier generations.
	while(true) {
		try {
			CurrentGBiBufferPtr_->pop_back_processed(&p,getLoopSec(), getLoopMSec());
			ok = true;
		}
		catch(time_out& t) { /* nothing */ }

		if(ok) {
			// is this a member of the current generation ?
			if(p->getGeneration() == generation) nReceivedCurrent++;
			else nReceivedOlder++;

			// Add individual to the list
			this->push_back(p->payload());

			// Have all members of the current generation returned ? If so, we can stop.
			// FALSCH: Hier nehmen wir an, dass alle Eltern weggeschickt wurden. Gilt auch nur
			// fuer MUPLUSNU ...
			boost::uint16_t defaultChildren = getDefaultChildren();
			if(((generation==0) && (getSortingScheme()==MUPLUSNU))?
					nReceivedCurrent==(defaultChildren+nParentsSentAway):
					nReceivedCurrent==defaultChildren)
			break;
		}

		// Have we reached the timeout ?
		boost::posix_time::ptime subsequentTimeMissed = boost::posix_time::microsec_clock::local_time();
		boost::posix_time::time_duration totalElapsed = (subsequentTimeMissed-startTime);
		if(getWaitFactor() && (totalElapsed> totalElapsedFirst*getWaitFactor())) {
			GLogStreamer gls;
			gls << "In GBrokerPopulation::mutateChildren():" << this << " : " << endl
			<< "In generation " << generation << ":" << endl
			<< "Timeout reached after " << totalElapsed.total_seconds() << " seconds" << endl
			<< "with nReceivedCurrent = " << nReceivedCurrent << " and" << endl
			<< "nReceivedOlder =" << nReceivedOlder << " where " << endl
			<< "nParents = " << np << " and" << endl
			<< "nChildren = " << nc << endl
			<< logLevel(UNCRITICAL);
			break;
		}
	}
}

/******************************************************************************/
/**
 * We are at this point not sure what the population looks like and whether
 * we have to do some fixing. Open questions include: In generation 0: Have we
 * received any parents ? Have we received enough parents ? In all generations:
 * Have we received enough children ? Have we received more children than expected
 * (e.g. from older generations) ?
 */
void GBrokerPopulation::select() {
	boost::uint16_t sz = this->size();
	boost::uint32_t generation = this->getGeneration();
	boost::uint16_t npar, nParents=this->getNParents();
	bool sortingScheme = this->getSortingScheme();

	std::vector<boost::shared_ptr<GMember> >::iterator it;

	// Do we have any individuals at all in the population ?
	if(sz==0) {
		// No - this should only be possible in generation 0 in MUPLUSNU mode, where we
		// also send away the parents for evaluation. In any case we cannot cope ...
		GException ge;
		ge << "In GBrokerPopulation::select() : Error in population " << this << endl
		<< "Population is empty in generation " << generation << endl
		<< "We cannot cope with this." << endl
		<< raiseException;
	}

	// If this is generation 0 and MUPLUSNU mode, we first need to check parents.
	// We want to emit a warning if fewer parents are available than expected.
	// Note that in this case the quality of the population can actually decrease
	// (unlike the normal situation in MUPLUSNU, where the quality always stays at
	// least constant). The user should know. This is usually not critical, as we
	// can replace parents with children and the quality will increase in furture
	// generations.
	if(generation==0 && sortingScheme==MUPLUSNU) {
		// Let's first sort the individuals according to their parent status
		sort(this->begin(),this->end(),f_isparent_);

		// Find out how many parents we have received.
		npar=0;
		for(it=this->begin(); it!=this->end(); ++it) if((*it)->isParent()) npar++;

		if(npar < nParents) {
			// No parents at all received ? Emit a warning
			if(npar==0) {
				gls << "In GBrokerPopulation::select(): Warning in population " << this << endl
				<< "No parents received in generation " << generation << " with a" << endl
				<< "population size of " << sz << endl
				<< logLevel(UNCRITICAL);
			}

			// We fill up the population to the expected level. We do this with
			// copies of the first element.
			boost::uint16_t missing = nParents - npar;

			// Let the user know
			gls << "In GBrokerPopulation::select(): Adding " << missing << " missing" << endl
			<< "parents to the population " << this << endl
			<< logLevel(UNCRITICAL);

			// Do the actual fill-up
			for(boost::uint16_t i=0; i<missing; i++) {
				GMember *newParent = dynamic_cast<GMember *>((*(this->begin()))->clone());
				if(!newParent) {
					GException ge;
					ge << "In GBrokerPopulation::select(): Conversion error "
					<< "in population " << this << endl
					<< raiseException;
				}
				// Make sure the parent knows about his role
				newParent->setIsParent(true);
				shared_ptr<GMember> p(newParent);
				this->insert(this->begin(), p);
			}
		}
	}

	// Next we fill up with children to the default level
	boost::uint16_t currentChildren = this->size() - nParents;
	boost::uint16_t defaultChildren = getDefaultChildren();

	if(currentChildren < defaultChildren) {
		boost::uint16_t missingChildren = defaultChildren - currentChildren;

		// Let the user know
		gls << "In GBrokerPopulation::select(): Adding " << missingChildren << " missing" << endl
		<< "children to the population " << this << endl
		<< logLevel(UNCRITICAL);

		// Add copies of last available member
		for(boost::uint16_t i = 0; i < missingChildren; i++) {
			// Doing this with an iterator would be more difficult
			GMember *gm = dynamic_cast<GMember *>(this->back()->clone());
			if(!gm) {
				GException ge;
				ge << "In GBrokerPopulation::select(): Conversion error (2)!" << endl
				<< raiseException;
			}
			gm->setIsParent(false); // Make sure they know their role
			shared_ptr<GMember> p(gm);
			this->push_back(p);
		}
	}

	////////////////////////////////////////////////////////////
	// Great - we are at least at the default level and are
	// ready to call the actual select() function. This will
	// automatically take care of MUPLUSNU and MUCOMMANU mode.
	GBasePopulation::select();
	////////////////////////////////////////////////////////////

	// At this point we have a sorted list of individuals and can take care of
	// too many members, so the next generation finds an intact population. This
	// function will remove the last items.
	this->resize(nParents + defaultChildren);

	// Everything should be back to normal ...
}

/******************************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */
