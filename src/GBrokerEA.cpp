/**
 * @file GBrokerEA.cpp
 */

/* Copyright (C) Dr. Ruediger Berlich and Karlsruhe Institute of Technology
 * (University of the State of Baden-Wuerttemberg and National Laboratory
 * of the Helmholtz Association)
 *
 * Contact: info [at] gemfony (dot) com
 *
 * This file is part of the Geneva library, Gemfony scientific's optimization
 * library.
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
 *
 * For further information on Gemfony scientific and Geneva, visit
 * http://www.gemfony.com .
 */

#include "GBrokerEA.hpp"

/**
 * Included here so no conflicts occur. See explanation at
 * http://www.boost.org/libs/serialization/doc/special.html#derivedpointers
 */
#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT(Gem::GenEvA::GBrokerEA)

namespace Gem
{
namespace GenEvA
{

/************************************************************************************************************/
/**
 * The default constructor
 */
GBrokerEA::GBrokerEA()
	: GEvolutionaryAlgorithm()
    , waitFactor_(DEFAULTWAITFACTOR)
    , maxWaitFactor_(DEFAULTMAXWAITFACTOR)
    , firstTimeOut_(boost::posix_time::duration_from_string(DEFAULTFIRSTTIMEOUT))
    , loopTime_(boost::posix_time::milliseconds(DEFAULTLOOPMSEC))
    , doLogging_(false)
{ /* nothing */ }

/************************************************************************************************************/
/**
 * The standard copy constructor
 *
 * @param cp A copy of another GBrokerEA object
 */
GBrokerEA::GBrokerEA(const GBrokerEA& cp)
	: GEvolutionaryAlgorithm(cp)
	, waitFactor_(cp.waitFactor_)
	, maxWaitFactor_(cp.maxWaitFactor_)
	, firstTimeOut_(cp.firstTimeOut_)
	, loopTime_(cp.loopTime_)
	, doLogging_(cp.doLogging_)
{ /* nothing */ }

/************************************************************************************************************/
/**
 * The standard destructor. We have no object-wide dynamically allocated data, hence
 * this function is empty.
 */
GBrokerEA::~GBrokerEA()
{ /* nothing */}

/************************************************************************************************************/
/**
 * A standard assignment operator for GBrokerEA objects,
 *
 * @param cp A copy of another GBrokerEA object
 * @return A constant reference to this object
 */
const GBrokerEA& GBrokerEA::operator=(const GBrokerEA& cp) {
	GBrokerEA::load_(&cp);
	return *this;
}

/************************************************************************************************************/
/**
 * Loads the data of another GBrokerEA object, camouflaged as a
 * pointer to a GObject
 *
 * @param cp A pointer to another GBrokerEA object, camouflaged as a GObject
 */
void GBrokerEA::load_(const GObject * cp) {
	const GBrokerEA *p_load = conversion_cast<GBrokerEA>(cp);

	// Load the parent class'es data ...
	GEvolutionaryAlgorithm::load_(cp);

	// ... and then our own
	waitFactor_=p_load->waitFactor_;
	maxWaitFactor_=p_load->maxWaitFactor_;
	firstTimeOut_=p_load->firstTimeOut_;
	loopTime_=p_load->loopTime_;
	doLogging_ = p_load->doLogging_;
}

/************************************************************************************************************/
/**
 * Creates a deep copy of this object
 *
 * @return A deep copy of this object
 */
GObject *GBrokerEA::clone_() const {
	return new GBrokerEA(*this);
}

/************************************************************************************************************/
/**
 * Checks for equality with another GBrokerEA object
 *
 * @param  cp A constant reference to another GBrokerEA object
 * @return A boolean indicating whether both objects are equal
 */
bool GBrokerEA::operator==(const GBrokerEA& cp) const {
	using namespace Gem::Util;
	// Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GBrokerEA::operator==","cp", CE_SILENT);
}

/************************************************************************************************************/
/**
 * Checks for inequality with another GBrokerEA object
 *
 * @param  cp A constant reference to another GBrokerEA object
 * @return A boolean indicating whether both objects are inequal
 */
bool GBrokerEA::operator!=(const GBrokerEA& cp) const {
	using namespace Gem::Util;
	// Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GBrokerEA::operator!=","cp", CE_SILENT);
}

/************************************************************************************************************/
/**
 * Checks whether a given expectation for the relationship between this object and another object
 * is fulfilled.
 *
 * @param cp A constant reference to another object, camouflaged as a GObject
 * @param e The expected outcome of the comparison
 * @param limit The maximum deviation for floating point values (important for similarity checks)
 * @param caller An identifier for the calling entity
 * @param y_name An identifier for the object that should be compared to this one
 * @param withMessages Whether or not information should be emitted in case of deviations from the expected outcome
 * @return A boost::optional<std::string> object that holds a descriptive string if expectations were not met
 */
boost::optional<std::string> GBrokerEA::checkRelationshipWith(const GObject& cp,
		const Gem::Util::expectation& e,
		const double& limit,
		const std::string& caller,
		const std::string& y_name,
		const bool& withMessages) const
{
    using namespace Gem::Util;
    using namespace Gem::Util::POD;

	// Check that we are indeed dealing with a GParamterBase reference
	const GBrokerEA *p_load = GObject::conversion_cast<GBrokerEA>(&cp);

	// Will hold possible deviations from the expectation, including explanations
    std::vector<boost::optional<std::string> > deviations;

	// Check our parent class'es data ...
	deviations.push_back(GEvolutionaryAlgorithm::checkRelationshipWith(cp, e, limit, "GBrokerEA", y_name, withMessages));

	// ... and then our local data
	deviations.push_back(checkExpectation(withMessages, "GBrokerEA", waitFactor_, p_load->waitFactor_, "waitFactor_", "p_load->waitFactor_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GBrokerEA", maxWaitFactor_, p_load->maxWaitFactor_, "maxWaitFactor_", "p_load->maxWaitFactor_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GBrokerEA", firstTimeOut_, p_load->firstTimeOut_, "firstTimeOut_", "p_load->firstTimeOut_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GBrokerEA", loopTime_, p_load->loopTime_, "loopTime_", "p_load->loopTime_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GBrokerEA", doLogging_, p_load->doLogging_, "doLogging_", "p_load->doLogging_", e , limit));

	return evaluateDiscrepancies("GBrokerEA", caller, deviations, e);
}

/************************************************************************************************************/
/**
 * Sets the waitFactor_ variable. This population measures the time until the
 * first individual has returned. This time times the waitFactor_ variable is
 * then used to check whether a timeout was reached for other individuals.
 * waitFactor_ is by default set to DEFAULTWAITFACTOR. You can disable this
 * timeout by setting waitFactor_ to 0.
 *
 * @param waitFactor The desired new value for waitFactor_ .
 */
void GBrokerEA::setWaitFactor(const boost::uint32_t& waitFactor)  {
	waitFactor_ = waitFactor;
}

/************************************************************************************************************/
/**
 * Sets the waitFactor_ and the maximumWaitFactor_ variable. If the latter is
 * != 0, the waitFactor_ will be automatically adapted, based on the number of
 * older individuals received.
 *
 * @param waitFactor The desired new value for waitFactor_ .
 */
void GBrokerEA::setWaitFactor(const boost::uint32_t& waitFactor, const boost::uint32_t& maxWaitFactor)  {
	// Do error checks
	if(maxWaitFactor && maxWaitFactor < waitFactor) {
		std::ostringstream error;
		error << "In GBrokerEA::setWaitFactor(uint32_t, uint32_t) : Error!" << std::endl
			  << "invalid maximum wait factor: " << maxWaitFactor << " / " << waitFactor << std::endl;
		throw geneva_error_condition(error.str());
	}

	waitFactor_ = waitFactor;
	maxWaitFactor_ = maxWaitFactor;
}

/************************************************************************************************************/
/**
 * Retrieves the waitFactor_ variable.
 *
 * @return The value of the waitFactor_ variable
 */
boost::uint32_t GBrokerEA::getWaitFactor() const  {
	return waitFactor_;
}

/************************************************************************************************************/
/**
 * Retrieves the maxWaitFactor_ variable.
 *
 * @return The value of the maxWaitFactor_ variable
 */
boost::uint32_t GBrokerEA::getMaxWaitFactor() const  {
	return maxWaitFactor_;
}

/************************************************************************************************************/
/**
 * Sets the maximum turn-around time for the first individual. When this time
 * has passed, an exception will be raised. Set the time out value to 0 if you
 * do not want the first individual to time out.
 *
 * @param firstTimeOut The maximum allowed time until the first individual returns
 */
void GBrokerEA::setFirstTimeOut(const boost::posix_time::time_duration& firstTimeOut) {
	firstTimeOut_ = firstTimeOut;
}

/************************************************************************************************************/
/**
 * Retrieves the value of the firstTimeOut_ variable.
 *
 * @return The value of firstTimeOut_ variable
 */
boost::posix_time::time_duration GBrokerEA::getFirstTimeOut() const {
	return firstTimeOut_;
}

/************************************************************************************************************/
/**
 * When retrieving items from the GBoundedBufferT queue (which in turn is accessed through
 * the GBroker interface), a time-out factor can be set with this function. The
 * default values is DEFAULTLOOPMSEC. A minimum value of 1 micro second is required.
 *
 * @param loopTime Timeout until an item was retrieved from the GBoundedBufferT
 */
void GBrokerEA::setLoopTime(const boost::posix_time::time_duration& loopTime) {
	// Only allow "real" values
	if(loopTime.is_special() || loopTime.is_negative() || loopTime.total_microseconds()==0) {
		std::ostringstream error;
		error << "In GBrokerEA::setLoopTime() : Error!" << std::endl
			  << "loopTime is set to 0" << std::endl;

		throw geneva_error_condition(error.str());
	}

	loopTime_ = loopTime;
}

/************************************************************************************************************/
/**
 * Retrieves the value of the loopTime_ variable
 *
 * @return The value of the loopTime_ variable
 */
boost::posix_time::time_duration GBrokerEA::getLoopTime() const {
	return loopTime_;
}

/************************************************************************************************************/
/**
 * Allows to specify whether logging of arrival times of individuals should be done. Note that only
 * arrival times of individuals of the current generation are logged. This also allows to find out
 * how many individuals did not return before the deadline.
 *
 * @param dl A boolean which allows to specify whether logging of arrival times of individuals should be done
 */
void GBrokerEA::doLogging(const bool& dl) {
	doLogging_ = dl;
}

/************************************************************************************************************/
/**
 * Allows to determine whether logging of arrival times has been activated.
 *
 * @return A boolean indicating whether logging of arrival times has been activated
 */
bool GBrokerEA::loggingActivated() const {
	return doLogging_;
}

/************************************************************************************************************/
/**
 * Allows to retrieve the logging results
 *
 * @return A vector containing the logging results
 */
std::vector<std::vector<boost::uint32_t> > GBrokerEA::getLoggingResults() const {
	return arrivalTimes_;
}

/************************************************************************************************************/
/**
 * Performs any necessary initialization work before the start of the optimization cycle
 */
void GBrokerEA::init() {
	// Prevent usage of this population inside another broker population - check type of first individual
	{
		std::vector<boost::shared_ptr<GIndividual> >::iterator it;
		for(it=this->begin(); it!=this->end(); ++it) {
			boost::shared_ptr<GBrokerEA> p = boost::dynamic_pointer_cast<GBrokerEA>(*it);

			if(p) {
				// Conversion was successful - this should not be, as there are not to supposed to be
				// any GBrokerEA objects inside itself.
				std::ostringstream error;
				error << "In GBrokerEA::optimize(): Error" << std::endl
				      << "GBrokerEA stored as an individual inside of" << std::endl
				      << "a population of the same type" << std::endl;
				throw(Gem::GenEvA::geneva_error_condition(error.str()));
			}
		}
	}

	// GEvolutionaryAlgorithm sees exactly the environment it would when called from its own class
	GEvolutionaryAlgorithm::init();

	CurrentBufferPort_ = GBufferPortT_ptr(new Gem::Util::GBufferPortT<boost::shared_ptr<Gem::GenEvA::GIndividual> >());
	GINDIVIDUALBROKER->enrol(CurrentBufferPort_);
}

/************************************************************************************************************/
/**
 * Performs any necessary finalization work after the end of the optimization cycle
 */
void GBrokerEA::finalize() {
	// Remove the GBufferPortT object. The broker only holds shared_ptr's to the
	// two objects contained therein, which are not invalidated, but become unique.
	// This is a selection criterion which lets the broker remove surplus buffer
	// twins.
	CurrentBufferPort_.reset();

	// GEvolutionaryAlgorithm sees exactly the environment it would when called from its own class
	GEvolutionaryAlgorithm::finalize();
}

/************************************************************************************************************/
/**
 * Starting from the end of the children's list, we submit individuals  to the
 * broker. In the first generation, in the case of the MUPLUSNU sorting strategy,
 * also the fitness of the parents is calculated. The type of command intended to
 * be executed on the individuals is stored in their attributes. The function
 * then waits for the first individual to come back. The time frame for all other
 * individuals to come back is a multiple of this time frame.
 */
void GBrokerEA::adaptChildren() {
	using namespace boost::posix_time;

	std::vector<boost::shared_ptr<GIndividual> >::reverse_iterator rit;
	std::size_t np = getNParents(), nc=data.size()-np;
	boost::uint32_t generation=getIteration();

	//--------------------------------------------------------------------------------
	// First we send all individuals abroad

	// Start with the children from the back of the population
	// This is the same for all sorting modes
	for(rit=data.rbegin(); rit!=data.rbegin()+nc; ++rit) {
		(*rit)->getPersonalityTraits()->setCommand("adapt");
		CurrentBufferPort_->push_front_orig(*rit);
	}

	// We can remove children, so only parents remain in the population
	data.resize(np);

	// Make sure we also evaluate the parents in the first generation, if needed.
	// This is only applicable to the MUPLUSNU and MUNU1PRETAIN modes.
	if(generation==0) {
		switch(getSortingScheme()) {
		//--------------------------------------------------------------
		case MUPLUSNU:
		case MUNU1PRETAIN: // same procedure. We do not know which parent is best
			// Note that we only have parents left in this generation
			for(rit=data.rbegin(); rit!=data.rend(); ++rit) {
				(*rit)->getPersonalityTraits()->setCommand("adapt");
				CurrentBufferPort_->push_front_orig(*rit);
			}

			// Next we clear the population. We do not loose anything,
			// as at least one shared_ptr to our individuals remains
			data.clear();
			break;

		case MUCOMMANU:
			break; // nothing
		}
		//--------------------------------------------------------------
		// If we are running in MUPLUSNU or MUNU1PRETAIN mode, we now have an empty population,
		// as parents have been sent away for evaluation. If this is the MUCOMMANU mode, parents
		// do not participate in the sorting and can be ignored.
		//
	}

	//--------------------------------------------------------------------------------
	// If logging is enabled, add a std::vector<boost::uint32_t> for the current generation
	// to arrivalTimes_
	if(doLogging_) arrivalTimes_.push_back(std::vector<boost::uint32_t>());

	//--------------------------------------------------------------------------------
	// We can now wait for the individuals to return from their journey.

	// First we wait for the first individual from the current generation to arrive,
	// or until a timeout has been reached. Individuals from older generations will
	// also be accepted in this loop, unless they are parents. If firstTimeOut_ is set
	// to 0, we do not timeout. Note that we can have a situation where less genuine
	// parents are in the population than have originally been sent away.

	// start to measure time. Uses the Boost.date_time library
	ptime startTime = boost::posix_time::microsec_clock::local_time();

	std::size_t nReceivedParent = 0, nReceivedChildCurrent=0, nReceivedChildOlder = 0;

	while(true)
	{
		try {
			boost::shared_ptr<GIndividual> p;

			// This function will throw a time-out condition if we
			// have exceeded the allowed time. Hence, when we reach
			// the next code line, we can assume to have a valid object
			CurrentBufferPort_->pop_back_processed(&p,loopTime_);

			// If it is from the current generation, break the loop.
			// Update the number of items received.
			if(p->getParentAlgIteration() == generation){
				// Add the individual to our list.
				this->push_back(p);

				// Log arrival times if requested by the user
				if(doLogging_) {
#ifdef DEBUG
					if(arrivalTimes_.size() != generation+1) {
						std::ostringstream error;
						error << "In GBrokerEA::adaptChildren() : Error!" << std::endl
							  << "The arrivalTimes_ vector has the incorrect size " << arrivalTimes_.size() << std::endl
							  << "Expected the size to be " << generation+1 << std::endl;
						throw geneva_error_condition(error.str());
					}
#endif /* DEBUG */
					arrivalTimes_[(std::size_t)generation].push_back((microsec_clock::local_time()-startTime).total_milliseconds());
				}

				// Update the counter.
				if(p->getEAPersonalityTraits()->isParent()) nReceivedParent++;
				else nReceivedChildCurrent++;

				break;
			}
			else {
				if(!p->getEAPersonalityTraits()->isParent()){ // We do not accept parents from older populations
					// Make it known to the individual that it is now part of a new generation
					p->setParentAlgIteration(generation);

					// Add the individual to our list.
					this->push_back(p);

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
				error << "In GBrokerEA::adaptChildren() : Error!" << std::endl
					  << "Timeout for first individual reached." << std::endl
					  << "Current timeout setting in microseconds is " << firstTimeOut_.total_microseconds() << std::endl
					  << "You can change this value with the setFirstTimeOut() function." << std::endl;

				throw geneva_error_condition(error.str());
			}
			// The loop will continue if no exception was thrown here
			else {
				continue;
			}
		}
	}

	// At this point we have received the first individual of the current generation back.
	// Record the elapsed time.
	time_duration totalElapsedFirst = microsec_clock::local_time()-startTime;
	time_duration maxAllowedElapsed = totalElapsedFirst * waitFactor_;
	time_duration totalElapsed = totalElapsedFirst;

	// Wait for further arrivals until the population is complete or
	// a timeout has been reached.
	bool complete=false;
	while(true){
		try {
			boost::shared_ptr<GIndividual> p;
			CurrentBufferPort_->pop_back_processed(&p,loopTime_);

			// Count the number of items received.
			if(p->getParentAlgIteration() == generation) {
				// Add the individual to our list.
				this->push_back(p);

				// Log arrival times if requested by the user
				if(doLogging_) {
#ifdef DEBUG
					if(arrivalTimes_.size() != generation+1) {
						std::ostringstream error;
						error << "In GBrokerEA::adaptChildren() : Error!" << std::endl
							  << "The arrivalTimes_ vector has the incorrect size " << arrivalTimes_.size() << std::endl
							  << "Expected the size to be " << generation+1 << std::endl;
						throw geneva_error_condition(error.str());
					}
#endif /* DEBUG */
					arrivalTimes_[(std::size_t)generation].push_back((microsec_clock::local_time()-startTime).total_milliseconds());
				}

				// Update the counter
				if(p->getEAPersonalityTraits()->isParent()) nReceivedParent++;
				else nReceivedChildCurrent++;
			}
			else {
				if(!p->getEAPersonalityTraits()->isParent()){  // Parents from older populations will be ignored, as there is no else clause
					// Make it known to the individual that it is now part of a new generation
					p->setParentAlgIteration(generation);

					// Add the individual to our list.
					this->push_back(p);

					// Update the counter
					nReceivedChildOlder++;
				}
			}
		}
		catch(Gem::Util::gem_util_condition_time_out&) {
			// Break if we have reached the timeout
			totalElapsed = microsec_clock::local_time()-startTime;
			if(waitFactor_ && (totalElapsed > maxAllowedElapsed)) break;
			else continue; // Nothing to do. Continue to wait for arrivals
		}

		// Break if a full set of children (and parents in generation 0 / MUPLUSNU / MUNU1PRETAIN)
		// has returned. Older individuals may return in the next iterations (i.e. generations), unless they are parents
		if(generation == 0 && (getSortingScheme()==MUPLUSNU || getSortingScheme()==MUNU1PRETAIN)) {
			if(nReceivedParent+nReceivedChildCurrent==np+getDefaultNChildren()) {
				complete=true;
				break;
			}
		}
		else {
			if(nReceivedChildCurrent>=getDefaultNChildren()) { // Note the >=
				complete=true;
				break;
			}
		}
	}

	if(generation==0 && (getSortingScheme()==MUPLUSNU || getSortingScheme()==MUNU1PRETAIN)){
		// Have any individuals returned at all ?
		if(data.size()==0) { // No way out ...
			std::ostringstream error;
			error << "In GBrokerEA::adaptChildren() : Error!" << std::endl
				  << "Population is empty when it shouldn't be." << std::endl;

			throw geneva_error_condition(error.str());
		}

		// Sort according to parent/child tag. We do not know in what order individuals have returned.
		// Hence we need to sort them.
		sort(data.begin(), data.end(), indParentComp());
	}

	//--------------------------------------------------------------------------------
	// We are done, if a full set of individuals has returned.
	// The population size is at least at nominal values.
	if(complete) {
		// Check if we have used significantly less time than allowed by the waitFactor_ variable
		// If so, and we do use automatic adaption of that variable, decrease the factor by one.
		if(waitFactor_ && maxWaitFactor_) { // Have we been asked to adapt the waitFactor_ variable ?
			totalElapsed = microsec_clock::local_time()-startTime;
			if(totalElapsed < maxAllowedElapsed &&
					(maxAllowedElapsed.total_microseconds() - totalElapsedFirst.total_microseconds()) >
					 long(double(maxAllowedElapsed.total_microseconds())*0.1)) {
				if(waitFactor_ > 1) waitFactor_ -= 1;
#ifdef DEBUG
				std::cout << "Adapted the waitFactor_ variable to " << waitFactor_ << std::endl;
#endif /* DEBUG */
			}
		}

		return;
	}

	//--------------------------------------------------------------------------------
	// O.k., so we are missing individuals from the current population. Do some fixing.

#ifdef DEBUG
	std::ostringstream information;
	information << "Note that in GBrokerEA::adaptChildren()" << std::endl
				<< "some individuals of the current population did not return" << std::endl
				<< "in generation " << generation << "." << std::endl;

	if(generation==0 && (getSortingScheme()==MUPLUSNU || getSortingScheme()==MUNU1PRETAIN)){
		information << "We have received " << nReceivedParent << " parents." << std::endl
			        << "where " << np << " parents were expected." << std::endl;
	}

	information << nReceivedChildCurrent << " children of the current population returned" << std::endl
			    << "plus " << nReceivedChildOlder << " older children," << std::endl
			    << "where the default number of children is " << getDefaultNChildren() << std::endl;
#endif /* DEBUG*/


	if(data.size() < np+getDefaultNChildren()){
		std::size_t fixSize=np+getDefaultNChildren() - data.size();

#ifdef DEBUG
		information << fixSize << "individuals thus need to be added to the population." << std::endl
					<< "Note that children may still return in later generations." << std::endl;
#endif /* DEBUG */

		for(std::size_t i=0; i<fixSize; i++){
			// This function will create a clone of its argument
			this->push_back_clone(data.back());
		}
	}

#ifdef DEBUG
	std::cout << information.str();
#endif /* DEBUG */

	// Adapt the waitFactor_ variable, if necessary
	// Check if we have used significantly less time than allowed by the waitFactor_ variable
	// If so, and we do use automatic adaption of that variable, decrease the factor by one.
	if(waitFactor_ && maxWaitFactor_) { // Have we been asked to adapt the waitFactor_ variable ?
		if(generation>0 && double(nReceivedChildOlder) > 0.1*double(nc)) {
			if(waitFactor_ < maxWaitFactor_) {
				waitFactor_ += 1;

#ifdef DEBUG
				std::cout << "Adapted the waitFactor_ variable to " << waitFactor_ << std::endl;
#endif /* DEBUG */
			}
		}
	}

	// Mark the first nParents_ individuals as parents, if they aren't parents yet. We want
	// to have a "sane" population.
	if(generation==0 && (getSortingScheme()==MUPLUSNU || getSortingScheme()==MUNU1PRETAIN)){
		GEvolutionaryAlgorithm::iterator it;
		for(it=this->begin(); it!=this->begin() + getNParents(); ++it) {
			if(!(*it)->getEAPersonalityTraits()->isParent()) {
				(*it)->getEAPersonalityTraits()->setIsParent();
			}
		}
	}

	// We care for too many returned individuals in the select() function. Older
	// individuals might nevertheless have a better quality. We do not want to loose them.

	// We might theoretically at this point have a population that (in generation 0 / MUPLUSNU / MUNU1PRETAIN)
	// consists of only parents. This is not a problem, as the entire population will get sorted
	// in this case, and new parents and children will be tagged after the select function.
}

/************************************************************************************************************/
/**
 * We will at this point have a population with at least the default number
 * of individuals. More individuals are allowed. the population will be
 * resized to nominal values at the end of this function.
 */
void GBrokerEA::select() {
	////////////////////////////////////////////////////////////
	// Great - we are at least at the default level and are
	// ready to call the actual select() function. This will
	// automatically take care of the selection modes.
	GEvolutionaryAlgorithm::select();

	////////////////////////////////////////////////////////////
	// At this point we have a sorted list of individuals and can take care of
	// too many members, so the next generation finds a "standard" population. This
	// function will remove the last items.
	data.resize(getNParents() + getDefaultNChildren());

	// Everything should be back to normal ...
}

/************************************************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GBrokerEA::modify_GUnitTests() {
	bool result = false;

	// Call the parent class'es function
	if(GEvolutionaryAlgorithm::modify_GUnitTests()) result = true;

	return result;
}

/************************************************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GBrokerEA::specificTestsNoFailureExpected_GUnitTests() {
	// Call the parent class'es function
	GEvolutionaryAlgorithm::specificTestsNoFailureExpected_GUnitTests();
}

/************************************************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GBrokerEA::specificTestsFailuresExpected_GUnitTests() {
	// Call the parent class'es function
	GEvolutionaryAlgorithm::specificTestsFailuresExpected_GUnitTests();
}

/************************************************************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */
