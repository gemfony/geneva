/**
 * @file GBrokerConnector.cpp
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

#include "geneva/GBrokerConnector.hpp"

/**
 * Included here so no conflicts occur. See explanation at
 * http://www.boost.org/libs/serialization/doc/special.html#derivedpointers
 */
#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT(Gem::Geneva::GBrokerConnector)

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
    , maxWaitFactor_(DEFAULTBROKERMAXWAITFACTOR)
    , firstTimeOut_(boost::posix_time::duration_from_string(DEFAULTBROKERFIRSTTIMEOUT))
    , loopTime_(boost::posix_time::milliseconds(DEFAULTBROKERLOOPMSEC))
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
	maxWaitFactor_ = cp->maxWaitFactor_;
	firstTimeOut_ = cp->firstTimeOut_;
	loopTime_ = cp->loopTime_;
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
	deviations.push_back(checkExpectation(withMessages, "GBrokerConnector", maxWaitFactor_, cp.maxWaitFactor_, "maxWaitFactor_", "cp.maxWaitFactor_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GBrokerConnector", firstTimeOut_, cp.firstTimeOut_, "firstTimeOut_", "cp.firstTimeOut_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GBrokerConnector", loopTime_, cp.loopTime_, "loopTime_", "cp.loopTime_", e , limit));
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
 * Sets the waitFactor_ and the maximumWaitFactor_ variable. If the latter is
 * != 0, the waitFactor_ will be automatically adapted, based on the number of
 * older individuals received.
 *
 * @param waitFactor The desired new value for waitFactor_ .
 */
void GBrokerConnector::setWaitFactor(const boost::uint32_t& waitFactor, const boost::uint32_t& maxWaitFactor)  {
	// Do error checks
	if(maxWaitFactor && maxWaitFactor < waitFactor) {
		std::ostringstream error;
		error << "In GBrokerConnector::setWaitFactor(uint32_t, uint32_t) : Error!" << std::endl
			  << "invalid maximum wait factor: " << maxWaitFactor << " < " << waitFactor << std::endl;
		throw Gem::Common::gemfony_error_condition(error.str());
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
boost::uint32_t GBrokerConnector::getWaitFactor() const  {
	return waitFactor_;
}

/************************************************************************************************************/
/**
 * Retrieves the maxWaitFactor_ variable.
 *
 * @return The value of the maxWaitFactor_ variable
 */
boost::uint32_t GBrokerConnector::getMaxWaitFactor() const  {
	return maxWaitFactor_;
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
 * When retrieving items from the GBoundedBufferT queue, a time-out factor can be set with this function. The
 * default values is DEFAULTBROKERLOOPMSEC. A minimum value of 1 micro second is required.
 *
 * @param loopTime Timeout until an item was retrieved from the GBoundedBufferT
 */
void GBrokerConnector::setLoopTime(const boost::posix_time::time_duration& loopTime) {
	// Only allow "real" values
	if(loopTime.is_special() || loopTime.is_negative() || loopTime.total_microseconds()==0) {
		std::ostringstream error;
		error << "In GBrokerConnector::setLoopTime() : Error!" << std::endl
			  << "loopTime is invalid" << std::endl;

		throw Gem::Common::gemfony_error_condition(error.str());
	}

	loopTime_ = loopTime;
}

/************************************************************************************************************/
/**
 * Retrieves the value of the loopTime_ variable
 *
 * @return The value of the loopTime_ variable
 */
boost::posix_time::time_duration GBrokerConnector::getLoopTime() const {
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
 * Allows to retrieve the logging results
 *
 * @return A vector containing the logging results
 */
std::vector<std::vector<boost::uint32_t> > GBrokerConnector::getLoggingResults() const {
	return arrivalTimes_;
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
