/**
 * @file GBrokerConnector.cpp
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

#include "geneva/GBrokerConnector.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GBrokerConnector)

namespace Gem
{
namespace Geneva
{

/************************************************************************************************************/
/**
 * The default constructor
 */
GBrokerConnector::GBrokerConnector()
    : waitFactor_(DEFAULTBROKERWAITFACTOR)
    , nProcessingUnits_(DEFAULTNPROCESSINGUNITS)
    , nProcessableItems_(1)
    , firstTimeOut_(boost::posix_time::duration_from_string(DEFAULTBROKERFIRSTTIMEOUT))
    , doLogging_(false)
{ /* nothing */ }

/************************************************************************************************************/
/**
 * The standard copy constructor
 *
 * @param cp A copy of another GBrokerConnector object
 */
GBrokerConnector::GBrokerConnector(const GBrokerConnector& cp)
	: waitFactor_(cp.waitFactor_)
	, nProcessingUnits_(cp.nProcessingUnits_)
	, nProcessableItems_(cp.nProcessableItems_)
	, firstTimeOut_(cp.firstTimeOut_)
	, doLogging_(cp.doLogging_)
{ /* nothing */ }

/************************************************************************************************************/
/**
 * The standard destructor. We have no object-wide dynamically allocated data, hence
 * this function is empty.
 */
GBrokerConnector::~GBrokerConnector()
{ /* nothing */}

/************************************************************************************************************/
/**
 * A standard assignment operator for GBrokerConnector objects,
 *
 * @param cp A copy of another GBrokerConnector object
 * @return A constant reference to this object
 */
const GBrokerConnector& GBrokerConnector::operator=(const GBrokerConnector& cp) {
	GBrokerConnector::load(&cp);
	return *this;
}

/************************************************************************************************************/
/**
 * Loads the data of another GBrokerConnector object
 *
 * @param cp A constant pointer to another GBrokerConnector object
 */
void GBrokerConnector::load(GBrokerConnector const * const cp) {
	waitFactor_ = cp->waitFactor_;
	nProcessingUnits_ = cp->nProcessingUnits_;
	nProcessableItems_ = cp->nProcessableItems_;
	firstTimeOut_ = cp->firstTimeOut_;
	doLogging_ = cp->doLogging_;
}

/************************************************************************************************************/
/**
 * Checks whether a given expectation for the relationship between this object and another object
 * is fulfilled.
 *
 * @param cp A constant reference to another GBrokerConnector
 * @param e The expected outcome of the comparison
 * @param limit The maximum deviation for floating point values (important for similarity checks)
 * @param caller An identifier for the calling entity
 * @param y_name An identifier for the object that should be compared to this one
 * @param withMessages Whether or not information should be emitted in case of deviations from the expected outcome
 * @return A boost::optional<std::string> object that holds a descriptive string if expectations were not met
 */
boost::optional<std::string> GBrokerConnector::checkRelationshipWith(
		const GBrokerConnector& cp
	  , const Gem::Common::expectation& e
	  , const double& limit
	  , const std::string& caller
	  , const std::string& y_name
	  , const bool& withMessages
) const {
    using namespace Gem::Common;

	// Will hold possible deviations from the expectation, including explanations
    std::vector<boost::optional<std::string> > deviations;

	// Check the local local data
	deviations.push_back(checkExpectation(withMessages, "GBrokerConnector", waitFactor_, cp.waitFactor_, "waitFactor_", "cp.waitFactor_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GBrokerConnector", nProcessingUnits_, cp.nProcessingUnits_, "nProcessingUnits_", "cp.nProcessingUnits_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GBrokerConnector", nProcessableItems_, cp.nProcessableItems_, "nProcessableItems_", "cp.nProcessableItems_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GBrokerConnector", firstTimeOut_, cp.firstTimeOut_, "firstTimeOut_", "cp.firstTimeOut_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GBrokerConnector", doLogging_, cp.doLogging_, "doLogging_", "cp.doLogging_", e , limit));

	return evaluateDiscrepancies("GBrokerConnector", caller, deviations, e);
}

/************************************************************************************************************/
/**
 * Checks for equality with another GBrokerConnector object
 *
 * @param  cp A constant reference to another GBrokerConnector object
 * @return A boolean indicating whether both objects are equal
 */
bool GBrokerConnector::operator==(const GBrokerConnector& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GBrokerConnector::operator==","cp", CE_SILENT);
}

/************************************************************************************************************/
/**
 * Checks for inequality with another GBrokerConnector object
 *
 * @param  cp A constant reference to another GBrokerConnector object
 * @return A boolean indicating whether both objects are inequal
 */
bool GBrokerConnector::operator!=(const GBrokerConnector& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GBrokerConnector::operator!=","cp", CE_SILENT);
}

/************************************************************************************************************/
/**
 * Sets the waitFactor_ variable. This allows to measure the time until the
 * first individual has returned. This time times the waitFactor_ variable is
 * then used to check whether a timeout was reached for other individuals.
 * waitFactor_ is by default set to DEFAULTBROKERWAITFACTOR. You can disable this
 * timeout by setting waitFactor_ to 0.
 *
 * @param waitFactor The desired new value for waitFactor_ .
 */
void GBrokerConnector::setWaitFactor(const boost::uint32_t& waitFactor)  {
	waitFactor_ = waitFactor;
}

/************************************************************************************************************/
/**
 * Retrieves the waitFactor_ variable.
 *
 * @return The value of the waitFactor_ variable
 */
boost::uint32_t GBrokerConnector::getWaitFactor() const  {
	return waitFactor_;
}

/************************************************************************************************************/
/**
 * Sets the desired number of processing units available for this population
 *
 * @param nProcessingUnits The desired number of processing units available for this population
 */
void GBrokerConnector::setNProcessingUnits(const boost::uint32_t& nProcessingUnits) {
	nProcessingUnits_ = nProcessingUnits;
}

/************************************************************************************************************/
/**
 * Allows to retrieve the assumed number of processing units available for this population
 *
 * @return The assumed number of processing units available for this population
 */
boost::uint32_t GBrokerConnector::getNProcessingUnits() const {
	return nProcessingUnits_;
}

/************************************************************************************************************/
/**
 * Sets the number of processable items in a given iteration
 *
 * @param nProcessableItems The desired number of processable items in a given iteration
 */
void GBrokerConnector::setBCNProcessableItems(const boost::uint32_t& nProcessableItems) {
#ifdef DEBUG
	if(nProcessableItems == 0) {
		raiseException (
				"In GBrokerConnector::setNProcessableItems(...): Error!" << std::endl
				<< "Received invalid number of processable items: " << nProcessableItems << std::endl
		);
	}
#endif

	nProcessableItems_ = nProcessableItems;
}

/************************************************************************************************************/
/**
 * Allows to retrieve the number of processable items in a given iteration
 *
 * @return The number of processable items in a given iteration
 */
boost::uint32_t GBrokerConnector::getBCNProcessableItems() const {
	return nProcessableItems_;
}

/************************************************************************************************************/
/**
 * Sets the maximum turn-around time for the first individual. When this time
 * has passed, an exception should be thrown. Set the time out value to 0 if you
 * do not want the first individual to time out.
 *
 * @param firstTimeOut The maximum allowed time until the first individual returns
 */
void GBrokerConnector::setFirstTimeOut(const boost::posix_time::time_duration& firstTimeOut) {
	firstTimeOut_ = firstTimeOut;
}

/************************************************************************************************************/
/**
 * Retrieves the value of the firstTimeOut_ variable.
 *
 * @return The value of firstTimeOut_ variable
 */
boost::posix_time::time_duration GBrokerConnector::getFirstTimeOut() const {
	return firstTimeOut_;
}

/************************************************************************************************************/
/**
 * Allows to specify whether logging of arrival times of individuals should be done. Note that only
 * arrival times of individuals of the current generation are logged. This also allows to find out
 * how many individuals did not return before the deadline.
 *
 * @param dl A boolean which allows to specify whether logging of arrival times of individuals should be done
 */
void GBrokerConnector::doLogging(const bool& dl) {
	doLogging_ = dl;
}

/************************************************************************************************************/
/**
 * Allows to determine whether logging of arrival times has been activated.
 *
 * @return A boolean indicating whether logging of arrival times has been activated
 */
bool GBrokerConnector::loggingActivated() const {
	return doLogging_;
}

/************************************************************************************************************/
/**
 * Instructs GBrokerConnector to performing logging activities
 */
void GBrokerConnector::log() {
	// Log arrival times if requested by the user
	if(doLogging_) {
		arrivalTimes_.back().push_back((boost::posix_time::microsec_clock::local_time()-iterationStartTime_).total_milliseconds());
	}
}

/************************************************************************************************************/
/**
 * Allows to retrieve the logging results
 *
 * @return A vector containing the logging results
 */
std::vector<std::vector<boost::uint32_t> > GBrokerConnector::getLoggingResults() const {
	return arrivalTimes_;
}

/************************************************************************************************************/
/**
 * Performs necessary initialization work after an optimization run. In particular it creates a buffer port
 * and registers it with the broker, so communication with external consumers can occur.
 */
void GBrokerConnector::init() {
	CurrentBufferPort_ = GBufferPortT_ptr(new Gem::Courtier::GBufferPortT<boost::shared_ptr<Gem::Geneva::GIndividual> >());
	GINDIVIDUALBROKER->enrol(CurrentBufferPort_);
}

/************************************************************************************************************/
/**
 * Performs necessary finalization work after an optimization run
 */
void GBrokerConnector::finalize() {
	// Remove the GBufferPortT object. The broker only holds shared_ptr's to the
	// two objects contained therein, which are not invalidated, but become unique.
	// This is a selection criterion which lets the broker remove surplus buffer
	// twins.
	CurrentBufferPort_.reset();
}

/************************************************************************************************************/
/**
 * Allows to perform any work necessary to be repeated in each new iteration.
 */
void GBrokerConnector::markNewIteration() {
	// If logging is enabled, add a std::vector<boost::uint32_t> for the current iteration
	// to arrivalTimes_
	if(doLogging_) arrivalTimes_.push_back(std::vector<boost::uint32_t>());

	// Recalculate the wait factor, based on the current number of processing units and the number of processable items
	// If e.g. nProcessableItems_ is 10 and nProcessingUnits_ is 3. we want the result to be 4. The calculation will also
	// result in a minimum wait factor of 1
	waitFactor_ = boost::numeric_cast<boost::uint32_t>(std::ceil(double(std::max(nProcessableItems_, boost::uint32_t(1)))/double(std::max(nProcessingUnits_, boost::uint32_t(1)))));

	// Set the start time of the new iteration so we calculate a correct
	// Return time for the first individual, regardless of whether older
	// individuals have returned first.
	iterationStartTime_ = boost::posix_time::microsec_clock::local_time();
}

/************************************************************************************************************/
/**
 * Resets the iteration's start time to the current time. This is useful when there
 * is a need for re-submission of individuals, such as in gradient descents.
 */
void GBrokerConnector::resetIterationStartTime() {
	iterationStartTime_ = boost::posix_time::microsec_clock::local_time();
}

/************************************************************************************************************/
/**
 * Allows to submit GIndividual-derivatives.
 *
 * @param gi A boost::shared_ptr to a GIndividual-derivative
 */
void GBrokerConnector::submit(boost::shared_ptr<GIndividual> gi) {
	CurrentBufferPort_->push_front_orig(gi);
}

/************************************************************************************************************/
/**
 * Retrieval of the first item of an iteration. This is a specialization for ind_type == GIndividual
 */
template <>
boost::shared_ptr<GIndividual> GBrokerConnector::retrieveFirstItem<GIndividual>() {
	// Holds the retrieved item
	boost::shared_ptr<GIndividual> p;

	if(firstTimeOut_.total_microseconds()) { // Wait for a given maximum amount of time
		// This function will throw if we have reached the timeout.
		// We cannot continue in this case. It is recommended to set this
		// variable to a rather high value or to set it to 0, in which case
		// we will wait indefinitely for the first item
		if(!CurrentBufferPort_->pop_back_processed_bool(&p, firstTimeOut_)) {
			raiseException(
					"In GBrokerConnector::retrieveFirstItem():" << std::endl
					<< "Timeout for first item reached." << std::endl
					<< "Current timeout setting in microseconds is " << firstTimeOut_.total_microseconds() << std::endl
					<< "You can change this value with the setFirstTimeOut() function."
			);
		}
	}
	else { // Wait indefinitely for the first item to return
		CurrentBufferPort_->pop_back_processed(&p);
	}

	// At this point we have received the first individual of the current generation back.

	// Record the elapsed time
	totalElapsedFirst_ = boost::posix_time::microsec_clock::local_time()-iterationStartTime_;

	// The total time that may have elapsed since the start of the retrieval procedure. A
	// waitFactor_ of 1 thus means that, after we have received the first individual, we
	// wait the same amount of time for further individuals to return from their journey.
	maxAllowedElapsed_ = totalElapsedFirst_ * (waitFactor_ + 1);

	return p;
}

/************************************************************************************************************/
/**
 * Retrieval of individuals and conversion to target type. This function will
 * return individuals as long as the elapsed time hasn't surpassed the allotted
 * time-frame. Once this has happened, it will return an empty pointer.
 *
 * @return An individual from the processed queue, converted to the desired target type
 */
template <>
boost::shared_ptr<GIndividual> GBrokerConnector::retrieveItem<GIndividual>() {
	// Will hold retrieved items or stay empty, if none could be retrieved
	boost::shared_ptr<GIndividual> p;

	if(nProcessingUnits_ > 0) { // Have we been asked to take into account a possible time-out ?
	   // Calculate how much time has elapsed since the start of the iteration
	   boost::posix_time::time_duration currentElapsed = boost::posix_time::microsec_clock::local_time()-iterationStartTime_;

	   // Make sure (maxAllowedElapsed_ - currentElapsed) is not negative and an item has been retrieved
	   if((maxAllowedElapsed_ < currentElapsed) || !CurrentBufferPort_->pop_back_processed_bool(&p,	maxAllowedElapsed_-currentElapsed)) {
		   p.reset(); // Make sure it is empty if we have encountered a time-out
	   }
	} else {// Wait indefinitely for the next item
		CurrentBufferPort_->pop_back_processed(&p);
	}

	return p;
}

#ifdef GENEVATESTING
/************************************************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GBrokerConnector::modify_GUnitTests() {
	bool result = false;
	return result;
}

/************************************************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GBrokerConnector::specificTestsNoFailureExpected_GUnitTests() {
	/* nothing */
}

/************************************************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GBrokerConnector::specificTestsFailuresExpected_GUnitTests() {
	/* nothing */
}

/************************************************************************************************************/

#endif /* GENEVATESTING */

} /* namespace Geneva */
} /* namespace Gem */
