/**
 * @file GBrokerPopulation.cpp
 */

/* Copyright (C) 2004-2008 Dr. Ruediger Berlich
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
BOOST_CLASS_EXPORT(Gem::GenEvA::GBrokerPopulation)

namespace Gem
{
namespace GenEvA
{

/******************************************************************************/
/**
 * The default constructor
 */
GBrokerPopulation::GBrokerPopulation()
	:GBasePopulation(),
     waitFactor_(DEFAULTWAITFACTOR),
     firstTimeOut_(boost::posix_time::duration_from_string(DEFAULTFIRSTTIMEOUT)),
     loopTime_(boost::posix_time::milliseconds(DEFAULTLOOPMSEC))
{ /* nothing */ }

/******************************************************************************/
/**
 * The standard copy constructor
 *
 * @param cp A copy of another GBrokerPopulation object
 */
GBrokerPopulation::GBrokerPopulation(const GBrokerPopulation& cp)
	:GBasePopulation(cp),
	 waitFactor_(cp.waitFactor_),
	firstTimeOut_(cp.firstTimeOut_),
	loopTime_(cp.loopTime_)
{ /* nothing */ }

/******************************************************************************/
/**
 * The standard destructor. We have no object-wide dynamically allocated data, hence
 * this function is empty.
 */
GBrokerPopulation::~GBrokerPopulation()
{ /* nothing */}

/******************************************************************************/
/**
 * A standard assignment operator for GBrokerPopulation objects,
 *
 * @param cp A copy of another GBrokerPopulation object
 * @return A constant reference to this object
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
 * @param cp A pointer to another GBrokerPopulation object, camouflaged as a GObject
 */
void GBrokerPopulation::load(const GObject * cp) {
	const GBrokerPopulation *gbp_load = this->checkedConversion(cp, this);

	// Load the parent class'es data ...
	GBasePopulation::load(cp);

	// ... and then our own
	waitFactor_=gbp_load->waitFactor_;
	firstTimeOut_=gbp_load->firstTimeOut_;
	loopTime_=gbp_load->loopTime_;
}

/******************************************************************************/
/**
 * Creates a deep copy of this object
 *
 * @return A deep copy of this object
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
 * @param waitFactor The desired new value for waitFactor_ .
 */
void GBrokerPopulation::setWaitFactor(boost::uint32_t waitFactor) throw() {
	waitFactor_ = waitFactor;
}

/******************************************************************************/
/**
 * Retrieves the waitFactor_ variable.
 *
 * @return The value of the waitFactor_ variable
 */
boost::uint32_t GBrokerPopulation::getWaitFactor() const throw() {
	return waitFactor_;
}

/******************************************************************************/
/**
 * Sets the maximum turn-around time for the first individual. When this time
 * has passed, an exception will be raised. Set the time out value to 0 if you
 * do not want the first individual to time out.
 *
 * @param firstTimeOut The maximum allowed time until the first individual returns
 */
void GBrokerPopulation::setFirstTimeOut(const boost::posix_time::time_duration& firstTimeOut) {
	firstTimeOut_ = firstTimeOut;
}

/******************************************************************************/
/**
 * Retrieves the value of the firstTimeOut_ variable.
 *
 * @return The value of firstTimeOut_ variable
 */
boost::posix_time::time_duration GBrokerPopulation::getFirstTimeOut() const {
	return firstTimeOut_;
}

/******************************************************************************/
/**
 * When retrieving items from the GBoundedBufferT queue (which in turn is accessed through
 * the GBroker interface), a time-out factor can be set with this function. The
 * default values is DEFAULTLOOPMSEC. A minimum value of 1 micro second is required.
 *
 * @param loopTime Timeout until an item was retrieved from the GBoundedBufferT
 */
void GBrokerPopulation::setLoopTime(const boost::posix_time::time_duration& loopTime) {
	// Only allow "real" values
	if(loopTime.is_special() || loopTime.is_negative() || loopTime.total_microseconds()==0) {
		std::ostringstream error;
		error << "In GBrokerPopulation::setLoopTime() : Error!" << std::endl
			  << "loopTime is set to 0" << std::endl;

		LOGGER.log(error.str(), Gem::GLogFramework::CRITICAL);
		throw geneva_invalid_loop_time()  << error_string(error.str());
	}

	loopTime_ = loopTime;
}

/******************************************************************************/
/**
 * Retrieves the value of the loopTime_ variable
 *
 * @return The value of the loopTime_ variable
 */
boost::posix_time::time_duration GBrokerPopulation::getLoopTime() const {
	return loopTime_;
}

/******************************************************************************/
/**
 * We provide the broker with a new GBufferPort object. Next the standard optimization cycle
 * of the parent population is started. When it is finished, we reset the shared_ptr<GBufferPort>.
 * The corresponding object is then deleted, and the GBoundedBufferT objects  owned by the broker are
 * orphaned. They will then be removed during the next enrollment.
 */
void GBrokerPopulation::optimize() {
	CurrentBufferPort_ = GBufferPort_ptr(new Gem::Util::GBufferPort<boost::shared_ptr<Gem::GenEvA::GIndividual> >());
	GINDIVIDUALBROKER.enrol(CurrentBufferPort_);

	// The main optimization cycle
	GBasePopulation::optimize();

	// Remove the GBufferPort object
	CurrentBufferPort_.reset();
}

/******************************************************************************/
/**
 * Starting from the end of the children's list, we submit individuals  to the
 * broker. In the first generation, in the case of the MUPLUSNU sorting strategy,
 * also the fitness of the parents is calculated. The type of command intended to
 * be executed on the individuals is stored in their attributes. The function
 * then waits for the first individual to come back. The timeframe for all other
 * individuals to come back is a multiple of this timeframe.
 */
void GBrokerPopulation::mutateChildren() {
	using namespace boost::posix_time;

	std::vector<boost::shared_ptr<GIndividual> >::reverse_iterator rit;
	std::size_t np = getNParents(), nc=data.size()-np;
	boost::uint32_t generation=this->getGeneration();

	//--------------------------------------------------------------------------------
	// First we send all individuals abroad

	// Start with the children from the back of the population
	// This is the same for MUPLUSNU and MUCOMMANU mode
	for(rit=data.rbegin(); rit!=data.rbegin()+nc; ++rit) {
		(*rit)->setAttribute("command","mutate");
		CurrentBufferPort_->push_front_orig(*rit);
	}

	// We can remove children, so only parents remain in the population
	data.resize(np);

	// Make sure we also evaluate the parents in the first generation, if needed.
	// This is only applicable to the MUPLUSNU mode.
	if(generation==0 && this->getSortingScheme()==MUPLUSNU) {
		// Note that we only have parents left in this generation
		for(rit=data.rbegin(); rit!=data.rend(); ++rit) {
			(*rit)->setAttribute("command","evaluate");
			CurrentBufferPort_->push_front_orig(*rit);
		}

		// Next we clear the population. We do not loose anything,
		// as at least one shared_ptr to our individuals remains
		data.clear();
	}

	//--------------------------------------------------------------------------------
	// If we are running in MUPLUSNU mode, we now have an empty population, as parents
	// have been sent away for evaluation. If this is the MUCOMMANU mode, parents do not
	// participate in the sorting and can be ignored. We can now wait for individuals
	// to return from their journey.

	// First we wait for the first individual from the current generation to arrive
	// or until a timeout has been reached. Individuals from older generations will
	// also be accepted in this loop, unless they are parents. If firstTimeOut_ is set
	// to 0, we do not timeout.

	// start to measure time. Uses the Boost.date_time library
	ptime startTime = boost::posix_time::microsec_clock::local_time();

	std::size_t nReceivedParent = 0, nReceivedChildCurrent=0, nReceivedChildOlder = 0;

	while(true)
	{
		try {
			boost::shared_ptr<GIndividual> p;
			CurrentBufferPort_->pop_back_processed(&p,loopTime_);

			// If it is from the current generation, break the loop.
			// Count the number of items received.
			if(p->getParentPopGeneration() == generation){
				// Add the individual to our list.
				data.push_back(p);

				// Update the counter.
				if(p->isParent()) nReceivedParent++;
				else nReceivedChildCurrent++;

				break;
			}
			else {
				if(!p->isParent()){ // We do not accept parents from older populations
					// Add the individual to our list.
					data.push_back(p);

					// Update the counter
					nReceivedChildOlder++;
				}
				else p.reset();
			}
		}
		catch(Gem::Util::gem_util_condition_time_out&) {
			// Find out whether we have exceeded a threshold
			if(firstTimeOut_.total_microseconds() && ((microsec_clock::local_time()-startTime) > firstTimeOut_)){
				std::ostringstream error;
				error << "In GBrokerPopulation::mutateChildren() : Error!" << std::endl
					  << "Timeout for first individual reached." << std::endl;

				LOGGER.log(error.str(), Gem::GLogFramework::CRITICAL);
				throw geneva_first_individual_timeout() << error_string(error.str());
			}
		}
	}

	// At this point we have received the first individual of the current generation back.
	// Record the time
	time_duration totalElapsedFirst = microsec_clock::local_time()-startTime;
	time_duration totalElapsed = totalElapsedFirst;

	// Wait for further arrivals
	bool complete=false;
	while(true){
		try {
			boost::shared_ptr<GIndividual> p;
			CurrentBufferPort_->pop_back_processed(&p,loopTime_);

			// Count the number of items received.
			if(p->getParentPopGeneration() == generation) {
				// Add the individual to our list.
				data.push_back(p);

				// Update the counter
				if(p->isParent()) nReceivedParent++;
				else nReceivedChildCurrent++;
			}
			else {
				if(!p->isParent()){  // We do not accept parents from older populations
					// Add the individual to our list.
					data.push_back(p);

					// Update the counter
					nReceivedChildOlder++;
				}
			}
		}
		catch(Gem::Util::gem_util_condition_time_out&) {
			// Break if we have reached the timeout
			totalElapsed = microsec_clock::local_time()-startTime;
			if(waitFactor_ && (totalElapsed > totalElapsedFirst*waitFactor_)) break;
		}

		// Break if all children (and parents in generation 0 / MUPLUSNU) of the
		// current generation have returned. Older individuals have another chance
		// to return in the next generation, unless they are parents.
		if(generation == 0 && this->getSortingScheme()==MUPLUSNU) {
			if(nReceivedParent+nReceivedChildCurrent==np+this->getDefaultNChildren()) {
				complete=true;
				break;
			}
		}
		else {
			if(nReceivedChildCurrent==this->getDefaultNChildren()) {
				complete=true;
				break;
			}
		}
	}

	if(generation==0 && this->getSortingScheme()==MUPLUSNU){
		// Have any individuals returned at all ??
		if(data.size()==0) { // No way out ...
			std::ostringstream error;
			error << "In GBrokerPopulation::mutateChildren() : Error!" << std::endl
				  << "Population is empty when it shouldn't be." << std::endl;

			LOGGER.log(error.str(), Gem::GLogFramework::CRITICAL);
			throw geneva_population_empty() << error_string(error.str());
		}

		// Sort according to parent/child tag. We do not know in whar order individuals have returned.
		// Hence we need to sort them.
		sort(data.begin(), data.end(),
			 boost::bind(&GIndividual::isParent, _1) > boost::bind(&GIndividual::isParent, _2));
	}

	// We are done, if all individuals from this generation have returned.
	// The population size is at least at nominal values.
	if(complete) return;

	//--------------------------------------------------------------------------------
	// O.k., we are missing individuals from the current population. Do some fixing.

#ifdef DEBUG
	std::ostringstream information;
	information << "Note that in GBrokerPopulation::mutateChildren()" << std::endl
				<< "some individuals of the current population did not return" << std::endl
				<< "in generation " << generation << "." << std::endl;

	if(generation==0 && this->getSortingScheme()==MUPLUSNU){
		information << "We have received " << nReceivedParent << " parents." << std::endl
			        << "where " << np << " parents are required." << std::endl;
	}

	information << nReceivedChildCurrent << " children of the current population returned" << std::endl
			    << "plus " << nReceivedChildOlder << " older children," << std::endl
			    << "where the default number of children is " << this->getDefaultNChildren() << std::endl;
#endif /* DEBUG*/


	if(data.size() < np+this->getDefaultNChildren()){
		std::size_t fixSize=np+this->getDefaultNChildren() - data.size();

#ifdef DEBUG
		information << fixSize << "individuals thus need to be added to the population." << std::endl
					<< "Note that children may still return in later generations." << std::endl;
#endif /* DEBUG */

		for(std::size_t i=0; i<fixSize; i++){
			GIndividual *gi = dynamic_cast<GIndividual *>((data.back())->clone());

			if(!gi){ // Cross check that the conversion worked
				std::ostringstream error;
				error << "In GBrokerPopulation::mutateChildren() : Conversion Error!" << std::endl;

				LOGGER.log(error.str(), Gem::GLogFramework::CRITICAL);

				throw geneva_dynamic_cast_conversion_error() << error_string(error.str());
			}

			data.push_back(shared_ptr<GIndividual>(gi));
		}
	}

#ifdef DEBUG
	LOGGER.log(information.str(), Gem::GLogFramework::INFORMATIONAL);
#endif /* DEBUG */

	// We care for too many returned individuals in the select() function. Older
	// individuals might nevertheless have a better quality. We do not want to loose them.

	// We might theoretically at this point have a population that (in generation 0 / MUPLUSNU)
	// consists of only parents. This is not a problem, as the entire population will get sorted
	// in this case, and new parents and children will be tagged after the select function.
}

/******************************************************************************/
/**
 * We will at this point have a population with at least the default number
 * of individuals. More individuals are allowed. the population will be
 * resized to nominal values at the end of this function.
 */
void GBrokerPopulation::select() {
	////////////////////////////////////////////////////////////
	// Great - we are at least at the default level and are
	// ready to call the actual select() function. This will
	// automatically take care of MUPLUSNU and MUCOMMANU mode.
	GBasePopulation::select();

	////////////////////////////////////////////////////////////
	// At this point we have a sorted list of individuals and can take care of
	// too many members, so the next generation finds a "standard" population. This
	// function will remove the last items.
	data.resize(this->getNParents() + this->getDefaultNChildren());

	// Everything should be back to normal ...
}

/******************************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */
