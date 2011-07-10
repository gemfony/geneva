/**
 * @file GBrokerConnectorT.hpp
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

#include <sstream>
#include <algorithm>
#include <vector>

// Includes check for correct Boost version(s)
#include "common/GGlobalDefines.hpp"

// Boost headers go here
#include <boost/cstdint.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/tracking.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/date_time.hpp>
#include <boost/date_time/gregorian/greg_serialize.hpp>
#include <boost/date_time/posix_time/time_serialize.hpp>
#include <boost/utility/enable_if.hpp>

#ifndef GBROKERCONNECTORT_HPP_
#define GBROKERCONNECTORT_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif

// Geneva headers go here
#include "common/GExceptions.hpp"
#include "common/GPODExpectationChecksT.hpp"
#include "courtier/GBufferPortT.hpp"
#include "courtier/GBrokerT.hpp"
#include "courtier/GCourtierEnums.hpp"

namespace Gem {
namespace Courtier {

/************************************************************************************************************/
/**
 * This class centralizes some functionality and data that is needed to connect
 * to networked execution through Geneva's broker. This class helps to avoid
 * duplication of code.
 */
template <typename T>
class GBrokerConnectorT
{
    ///////////////////////////////////////////////////////////////////////
	// NOTE: In order to actually (de-)serialize this class, you will have
	// to make it known to the Boost.Serialization library, once you have
	// specified T. You can do so with the BOOST_CLASS_EXPORT_KEY /
	// BOOST_CLASS_EXPORT_IMPLEMENT combo.
    friend class boost::serialization::access;

    template<typename Archive>
    void serialize(Archive & ar, const unsigned int){
      using boost::serialization::make_nvp;

      ar & BOOST_SERIALIZATION_NVP(waitFactor_)
      	 & BOOST_SERIALIZATION_NVP(minWaitFactor_)
      	 & BOOST_SERIALIZATION_NVP(maxWaitFactor_)
    	 & BOOST_SERIALIZATION_NVP(waitFactorIncrement_)
    	 & BOOST_SERIALIZATION_NVP(boundlessWait_)
		 & BOOST_SERIALIZATION_NVP(allItemsReturned_)
		 & BOOST_SERIALIZATION_NVP(percentOfTimeoutNeeded_)
		 & BOOST_SERIALIZATION_NVP(firstTimeOut_)
		 & BOOST_SERIALIZATION_NVP(doLogging_);
    }
    ///////////////////////////////////////////////////////////////////////

public:
    typedef boost::shared_ptr<Gem::Courtier::GBufferPortT<boost::shared_ptr<T> > > GBufferPortT_ptr;

    /**********************************************************************************/
    /**
     * The default constructor
     */
    GBrokerConnectorT()
        : waitFactor_(DEFAULTBROKERWAITFACTOR)
		, minWaitFactor_(DEFAULTMINBROKERWAITFACTOR)
		, maxWaitFactor_(DEFAULTMAXBROKERWAITFACTOR)
		, waitFactorIncrement_(DEFAULTBROKERWAITFACTORINCREMENT)
    	, boundlessWait_(false)
        , allItemsReturned_(true)
    	, percentOfTimeoutNeeded_(0.)
        , firstTimeOut_(boost::posix_time::duration_from_string(DEFAULTBROKERFIRSTTIMEOUT))
        , doLogging_(false)
    { /* nothing */ }

    /**********************************************************************************/
    /**
     * The standard copy constructor
     *
     * @param cp A copy of another GBrokerConnector object
     */
    GBrokerConnectorT(const GBrokerConnectorT<T>& cp)
    	: waitFactor_(cp.waitFactor_)
		, minWaitFactor_(cp.minWaitFactor_)
		, maxWaitFactor_(cp.maxWaitFactor_)
		, waitFactorIncrement_(cp.waitFactorIncrement_)
    	, boundlessWait_(cp.boundlessWait_)
    	, allItemsReturned_(true)
		, percentOfTimeoutNeeded_(0.)
    	, firstTimeOut_(cp.firstTimeOut_)
    	, doLogging_(cp.doLogging_)
    { /* nothing */ }

    /**********************************************************************************/
    /**
     * The standard destructor. We have no object-wide dynamically allocated data, hence
     * this function is empty.
     */
    virtual ~GBrokerConnectorT()
    { /* nothing */}

    /**********************************************************************************/
    /**
     * A standard assignment operator for GBrokerConnectorT<T> objects,
     *
     * @param cp A copy of another GBrokerConnectorT<T> object
     * @return A constant reference to this object
     */
    const GBrokerConnectorT<T>& operator=(const GBrokerConnectorT<T>& cp) {
    	GBrokerConnectorT<T>::load(&cp);
    	return *this;
    }

    /**********************************************************************************/
    /**
     * Loads the data of another GBrokerConnector object
     *
     * @param cp A constant pointer to another GBrokerConnector object
     */
    void load(GBrokerConnectorT<T> const * const cp) {
    	waitFactor_ = cp->waitFactor_;
		minWaitFactor_ = cp->minWaitFactor_;
		maxWaitFactor_ = cp->maxWaitFactor_;
		waitFactorIncrement_ = cp->waitFactorIncrement_;
		boundlessWait_ = cp->boundlessWait_;
    	allItemsReturned_ = cp->allItemsReturned_;
		percentOfTimeoutNeeded_ = cp->percentOfTimeoutNeeded_;
    	firstTimeOut_ = cp->firstTimeOut_;
    	doLogging_ = cp->doLogging_;
    }

    /**********************************************************************************/
    /**
     * Checks for equality with another GBrokerConnectorT<T> object
     *
     * @param  cp A constant reference to another GBrokerConnectorT<T> object
     * @return A boolean indicating whether both objects are equal
     */
    bool operator==(const GBrokerConnectorT<T>& cp) const {
    	using namespace Gem::Common;
    	// Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
    	return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GBrokerConnectorT<T>::operator==","cp", CE_SILENT);
    }

    /**********************************************************************************/
    /**
     * Checks for inequality with another GBrokerConnectorT<T> object
     *
     * @param  cp A constant reference to another GBrokerConnectorT<T> object
     * @return A boolean indicating whether both objects are inequal
     */
    bool operator!=(const GBrokerConnectorT<T>& cp) const {
    	using namespace Gem::Common;
    	// Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
    	return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GBrokerConnectorT<T>::operator!=","cp", CE_SILENT);
    }

    /**********************************************************************************/
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
    boost::optional<std::string> checkRelationshipWith(
    		const GBrokerConnectorT<T>& cp
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
    	deviations.push_back(checkExpectation(withMessages, "GBrokerConnectorT<T>", waitFactor_, cp.waitFactor_, "waitFactor_", "cp.waitFactor_", e , limit));
    	deviations.push_back(checkExpectation(withMessages, "GBrokerConnectorT<T>", minWaitFactor_, cp.minWaitFactor_, "minWaitFactor_", "cp.minWaitFactor_", e , limit));
    	deviations.push_back(checkExpectation(withMessages, "GBrokerConnectorT<T>", maxWaitFactor_, cp.maxWaitFactor_, "maxWaitFactor_", "cp.maxWaitFactor_", e , limit));
    	deviations.push_back(checkExpectation(withMessages, "GBrokerConnectorT<T>", waitFactorIncrement_, cp.waitFactorIncrement_, "waitFactorIncrement_", "cp.waitFactorIncrement_", e , limit));
    	deviations.push_back(checkExpectation(withMessages, "GBrokerConnectorT<T>", boundlessWait_, cp.boundlessWait_, "boundlessWait_", "cp.boundlessWait_", e , limit));
    	deviations.push_back(checkExpectation(withMessages, "GBrokerConnectorT<T>", allItemsReturned_, cp.allItemsReturned_, "allItemsReturned_", "cp.allItemsReturned_", e , limit));
    	deviations.push_back(checkExpectation(withMessages, "GBrokerConnectorT<T>", percentOfTimeoutNeeded_, cp.percentOfTimeoutNeeded_, "percentOfTimeoutNeeded_", "cp.percentOfTimeoutNeeded_", e , limit));
    	deviations.push_back(checkExpectation(withMessages, "GBrokerConnectorT<T>", firstTimeOut_, cp.firstTimeOut_, "firstTimeOut_", "cp.firstTimeOut_", e , limit));
    	deviations.push_back(checkExpectation(withMessages, "GBrokerConnectorT<T>", doLogging_, cp.doLogging_, "doLogging_", "cp.doLogging_", e , limit));

    	return evaluateDiscrepancies("GBrokerConnectorT<T>", caller, deviations, e);
    }

    /**********************************************************************************/
    /**
     * Sets the maximum turn-around time for the first individual. When this time
     * has passed, an exception should be thrown. Set the time out value to 0 if you
     * do not want the first individual to time out.
     *
     * @param firstTimeOut The maximum allowed time until the first individual returns
     */
    void setFirstTimeOut(const boost::posix_time::time_duration& firstTimeOut) {
    	firstTimeOut_ = firstTimeOut;
    }

    /**********************************************************************************/
    /**
     * Retrieves the value of the firstTimeOut_ variable.
     *
     * @return The value of firstTimeOut_ variable
     */
    boost::posix_time::time_duration getFirstTimeOut() const {
    	return firstTimeOut_;
    }

    /**********************************************************************************/
    /**
     * Allows to set the extremes that the waitFactor_ variable may assume
     *
     * @param minWaitFactor The minimum wait factor
     * @param maxWaitFactor The maximum wait factor
     */
    void setWaitFactorExtremes(const double& minWaitFactor, const double& maxWaitFactor) {
    	// Do some error checking
    	if(minWaitFactor < 0. || minWaitFactor >= maxWaitFactor) {
    		raiseException(
    				"In GBrokerConnectorT<T>::setWaitFactorExtremes(): Error!" << std::endl
    				<< "Got invalid extreme values: " << minWaitFactor << " / " << maxWaitFactor << std::endl
    		);
    	}

    	minWaitFactor_ = minWaitFactor;
    	maxWaitFactor_ = maxWaitFactor;
    }

    /**********************************************************************************/
    /**
     * Returns the current value of the minWaitFactor_ variable
     *
     * @return The current value of the minWaitFactor_ variable
     */
    double getMinWaitFactor_() const {
    	return minWaitFactor_;
    }

    /**********************************************************************************/
    /**
     * Returns the current value of the maxWaitFactor_ variable
     *
     * @return The current value of the maxWaitFactor_ variable
     */
    double getMaxWaitFactor_() const {
    	return maxWaitFactor_;
    }

    /**********************************************************************************/
    /**
     * Allows to specify whether logging of arrival times of processed items should be
     * done. Note that only arrival times of items of the current iteration are logged.
     * This also allows to find out how many items did not return before the deadline.
     *
     * @param dl A boolean whether logging of arrival times of items should be done
     */
    void doLogging(const bool& dl = true) {
    	doLogging_ = dl;
    }

    /**********************************************************************************/
    /**
     * Allows to determine whether logging of arrival times has been activated.
     *
     * @return A boolean indicating whether logging of arrival times has been activated
     */
    bool loggingActivated() const {
    	return doLogging_;
    }

    /**********************************************************************************/
    /**
     * Allows to retrieve the logging results. Note that this function also resets
     * the local arrivalTimes_ vector.
     *
     * @param arrivalTimes A vector containing the logging results
     */
    void getLoggingResults(std::vector<std::vector<boost::uint32_t> >& arrivalTimes) const {
    	arrivalTimes = arrivalTimes_;
    	arrivalTimes_.clear();
    }

    /**********************************************************************************/
    /**
     * Specifies whether item retrievals should wait for an unlimited amount of time
     * for processed items.
     *
     * @param boundlessWait Indicates whether the waiting time for processed items should be unlimited
     */
    void setBoundlessWait(const bool& boundlessWait) {
    	boundlessWait_ = boundlessWait;
    }

    /**********************************************************************************/
    /**
     * Checks whether item retrievals should wait for an unlimited amount of time
     * for processed items.
     *
     * @return A boolean indicating whether item retrievals should wait for an unlimited amount of time for processed items.
     */
    bool getBoundlessWait() const {
    	return boundlessWait_;
    }

    /**********************************************************************************/
    /**
     * Allows to check whether all items have returned before the timeout of an iteration
     *
     * @return A boolean indicating whether all items have returned before the timeout of an iteration
     */
    bool getAllItemsReturned() const {
    	return allItemsReturned_;
    }

    /**********************************************************************************/
    /**
     * Performs necessary initialization work after a new compute run. In particular it
     * creates a buffer port and registers it with the broker, so communication with
     * external consumers can occur.
     */
    void init() {
    	CurrentBufferPort_ = GBufferPortT_ptr(new Gem::Courtier::GBufferPortT<boost::shared_ptr<T> >());
    	GBROKER(T)->enrol(CurrentBufferPort_);
    }

    /**********************************************************************************/
    /**
     * Performs necessary finalization work after an optimization run
     */
    void finalize() {
    	// Remove the GBufferPortT object. The broker only holds shared_ptr's to the
    	// two objects contained therein, which are not invalidated, but become unique.
    	// This is a selection criterion which lets the broker remove surplus buffer
    	// twins.
    	CurrentBufferPort_.reset();
    }

    /**********************************************************************************/
    /**
     * Allows to perform any work necessary to be repeated in each new iteration. In
     * particular, this function adjusts the waitFactor_ variable, so that it fits
     * the current load pattern of the computing resources behind the broker.
 	 */
    void markNewIteration() {
    	// If logging is enabled, add a std::vector<boost::uint32_t> for the current iteration to arrivalTimes_
    	if(doLogging_) arrivalTimes_.push_back(std::vector<boost::uint32_t>());

    	// Adapting the wait factor only makes sense if we haven't been ordered to wait endlessly anyway
    	if(!boundlessWait_) {
			// Increment or decrement the waitFactor_ variable, based on the current load of the system
			// Over the course of a few iterations, waitFactor_ should adjust itself into the correct range
			if(!allItemsReturned_) {
				if(waitFactor_ < maxWaitFactor_) {
					waitFactor_ += waitFactorIncrement_;

					// Make sure we do not accidently slip above the maximum allowed wait factor
					if(waitFactor_ > maxWaitFactor_) {
						waitFactor_ = maxWaitFactor_;
					}
				}
			} else if(percentOfTimeoutNeeded_ < DEFAULTMINPERCENTAGEOFTIMEOUT) {
				if(waitFactor_ > minWaitFactor_) {
					waitFactor_ -= waitFactorIncrement_;

					// Make sure we do not accidently slip below the minimum wait factor
					if(waitFactor_ < minWaitFactor_) {
						waitFactor_ = minWaitFactor_;
					}
				}
			}
    	}

		// Reset the allItemsReturned_ variable. We assume that all items
    	// will return before the timeout in the next iteration
		allItemsReturned_ = true;

    	// Set the start time of the new iteration so we calculate a correct
    	// Return time for the first individual, regardless of whether older
    	// individuals have returned first.
    	iterationStartTime_ = boost::posix_time::microsec_clock::local_time();
    }

    /**********************************************************************************/
    /**
     * Prolongs the timeout. This is useful when there is a need for re-submission of
     * individuals, such as in gradient descents.
     */
    void prolongTimeout() {
    	maxAllowedElapsed_ += totalElapsedFirst_ * (waitFactor_ + 1);
    }

    /**********************************************************************************/
    /**
     * Allows to submit work items to the broker
     *
     * @param gi A boost::shared_ptr to a work item
     */
    void submit(boost::shared_ptr<T> gi) {
    	CurrentBufferPort_->push_front_orig(gi);
    }

    /**********************************************************************************/
    /**
     * Retrieves the current waitFactor_ variable.
     *
     * @return The value of the waitFactor_ variable
     */
    boost::uint32_t getWaitFactor() const  {
    	return waitFactor_;
    }

    /**********************************************************************************/
    /**
     * Retrieval of the first work item. This function simply returns a boost::shared_ptr<T>
     * with the work item. Note that this function will throw if the maximum allowed time for
     * the retrieval of the first item has been surpassed (if set).
	 *
	 * @return The retrieved item
     */
	boost::shared_ptr<T> retrieveFirstItem() {
		// Holds the retrieved item
		boost::shared_ptr<T> p;

		if(firstTimeOut_.total_microseconds()) { // Wait for a given maximum amount of time
			// pop_back_processed_bool will return false if we have reached the timeout
			// We cannot continue in this case. It is recommended to set firstTimeOut_
			// to a rather high value or to alternatively disable it completely by setting it
			// to EMPTYDURATION.
			if(!CurrentBufferPort_->pop_back_processed_bool(&p, firstTimeOut_)) {
				raiseException(
						"In GBrokerConnectorT<T>::retrieveFirstItem():" << std::endl
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
		// Record the elapsed time and calculate the time for which other individuals are
		// allowed to return
		totalElapsedFirst_ = boost::posix_time::microsec_clock::local_time()-iterationStartTime_;
		maxAllowedElapsed_ = totalElapsedFirst_ * (waitFactor_ + 1);

    	// Log arrival times if requested by the user
    	if(doLogging_) {
    		arrivalTimes_.back().push_back(totalElapsedFirst_.total_milliseconds());
    	}

		return p;
	}

    /**********************************************************************************/
	/**
	 * Retrieval of the first work item and conversion to a target type. Note that
	 * this function will throw if the maximum allowed time for the retrieval of
	 * the first item has been surpassed (if set).
	 *
	 * @return The retrieved item, converted into the desired target type
	 */
	template <typename target_type>
	boost::shared_ptr<target_type> retrieveFirstItem() {
		// Holds the retrieved item
		boost::shared_ptr<T> p;

		if(firstTimeOut_.total_microseconds()) { // Wait for a given maximum amount of time
			// pop_back_processed_bool will return false if we have reached the timeout
			// We cannot continue in this case. It is recommended to set firstTimeOut_
			// to a rather high value or to alternatively disable it completely by setting it
			// to EMPTYDURATION.
			if(!CurrentBufferPort_->pop_back_processed_bool(&p, firstTimeOut_)) {
				raiseException(
						"In GBrokerConnectorT<T>::retrieveFirstItem<target_type>():" << std::endl
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
		// Record the elapsed time and calculate the time for which other individuals are
		// allowed to return
		totalElapsedFirst_ = boost::posix_time::microsec_clock::local_time()-iterationStartTime_;
		maxAllowedElapsed_ = totalElapsedFirst_ * (waitFactor_ + 1);

    	// Log arrival times if requested by the user
    	if(doLogging_) {
    		arrivalTimes_.back().push_back(totalElapsedFirst_.total_milliseconds());
    	}

		// Convert the item to the target type. Note that there is a specialization
		// of this function in case target_type == T
#ifdef DEBUG
		// Check that p actually points somewhere
		if(!p) {
			raiseException("In GBrokerConnectorT<T>::retrieveFirstItem<target_type>(): Empty pointer found");
		}

		boost::shared_ptr<target_type> p_converted = boost::dynamic_pointer_cast<target_type>(p);

		if(p_converted) return p_converted;
		else {
			raiseException("In GBrokerConnectorT<T>::retrieveFirstItem<target_type>(): Conversion error");
		}
#else
		return boost::static_pointer_cast<target_type>(p);
#endif /* DEBUG */
	}

    /**********************************************************************************/
	/**
	 * Retrieval of a work item. This function will return items as long as the elapsed
	 * time hasn't surpassed the allotted time-frame. Once this has happened, it will
	 * return an empty pointer.
	 *
	 * @return A work item from the processed queue, converted to the desired target type
	 */
	boost::shared_ptr<T> retrieveItem() {
		// Will hold retrieved items
		boost::shared_ptr<T> p;

		// Will hold the elapsed time since the start of this iteration
		boost::posix_time::time_duration currentElapsed;

		if(boundlessWait_) { // Wait indefinitely for the next item
			CurrentBufferPort_->pop_back_processed(&p);
		} else { // Observe a timeout
			// Calculate how much time has elapsed since the start of the iteration
			currentElapsed = boost::posix_time::microsec_clock::local_time()-iterationStartTime_;

			// Have we already exceeded the timeout ? Or was it impossible to retrieve an item in the remaining time before the timeout ?
			if((currentElapsed > maxAllowedElapsed_) || !CurrentBufferPort_->pop_back_processed_bool(&p, maxAllowedElapsed_-currentElapsed)) {
				// We ran into a timeout before the start of a new iteration was signaled.
				// This is interpreted as a situation where not all items have returned.
				allItemsReturned_ = false;
				p.reset();
				return p; // will be empty
			} else { // o.k., p now holds a valid pointer
#ifdef DEBUG
				if(maxAllowedElapsed_.total_microseconds() == 0) {
					raiseException(
							"In GBrokerConnectorT<T>::retrieveItem(): Error!" << std::endl
							<< "maxAllowedElapsed_ is 0" << std::endl
					);
				}
#endif

				// Make a note of the current percentage of the maximum timeout (needed for the waitFactor calculation).
				// This variable will be updated for every call to retrieveItem<>. When the start of a new iteration is
				// signaled by "markNewIteration()", it will hold the latest percentage.
				percentOfTimeoutNeeded_ = double(currentElapsed.total_microseconds()) / double(maxAllowedElapsed_.total_microseconds());
#ifdef DEBUG
				if(percentOfTimeoutNeeded_ > 1. || percentOfTimeoutNeeded_ < 0) {
					raiseException(
							"In GBrokerConnectorT<T>::retrieveItem(): Error!" << std::endl
							<< "Invalid percentage of time out: " << percentOfTimeoutNeeded_ << std::endl
					);
				}
#endif /* DEBUG */
			}
		}

    	// Log arrival times if requested by the user
    	if(doLogging_) {
    		arrivalTimes_.back().push_back(currentElapsed.total_milliseconds());
    	}

		return p;
	}

    /**********************************************************************************/
	/**
	 * Retrieval of a work item and conversion to a target type. This function will
	 * return items as long as the elapsed time hasn't surpassed the allotted
	 * time-frame. Once this has happened, it will return an empty pointer. Note that
	 * there is a specialization of this function in case target_type == T.
	 *
	 * @return A work item from the processed queue, converted to the desired target type
	 */
	template <typename target_type>
	boost::shared_ptr<target_type> retrieveItem() {
		// Will hold retrieved items
		boost::shared_ptr<T> p;
		// Will hold converted items
		boost::shared_ptr<target_type> p_converted;
		// Will hold the elapsed time since the start of this iteration
		boost::posix_time::time_duration currentElapsed;

		if(boundlessWait_) { // Wait indefinitely for the next item
			CurrentBufferPort_->pop_back_processed(&p);
		} else { // Observe a timeout
			// Calculate how much time has elapsed since the start of the iteration
			currentElapsed = boost::posix_time::microsec_clock::local_time()-iterationStartTime_;

			// Have we already exceeded the timeout ? Or was it impossible to retrieve an item in the remaining time before the timeout ?
			if((currentElapsed > maxAllowedElapsed_) || !CurrentBufferPort_->pop_back_processed_bool(&p, maxAllowedElapsed_-currentElapsed)) {
				// We ran into a timeout before the start of a new iteration was signaled.
				// This is interpreted as a situation where not all items have returned.
				allItemsReturned_ = false;
				return p_converted; // will be empty
			} else { // o.k., p now holds a valid pointer
#ifdef DEBUG
				if(maxAllowedElapsed_.total_microseconds() == 0) {
					raiseException(
							"In GBrokerConnectorT<T>::retrieveItem<target_type>(): Error!" << std::endl
							<< "maxAllowedElapsed_ is 0" << std::endl
					);
				}
#endif

				// Make a note of the current percentage of the maximum timeout (needed for the waitFactor calculation).
				// This variable will be updated for every call to retrieveItem<>. When the start of a new iteration is
				// signaled by "markNewIteration()", it will hold the latest percentage.
				percentOfTimeoutNeeded_ = double(currentElapsed.total_microseconds()) / double(maxAllowedElapsed_.total_microseconds());
#ifdef DEBUG
				if(percentOfTimeoutNeeded_ > 1. || percentOfTimeoutNeeded_ < 0) {
					raiseException(
							"In GBrokerConnectorT<T>::retrieveItem(): Error!" << std::endl
							<< "Invalid percentage of time out: " << percentOfTimeoutNeeded_ << std::endl
					);
				}
#endif /* DEBUG */
			}
		}

    	// Log arrival times if requested by the user
    	if(doLogging_) {
    		arrivalTimes_.back().push_back(currentElapsed.total_milliseconds());
    	}

		// If we have reached this point, we should have a valid p. Convert to the target type.
#ifdef DEBUG
		// Check that p actually points somewhere
		if(!p) {
			raiseException(
					"In GBrokerConnectorT<T>::retrieveItem<target_type>(): Empty pointer found"
			);
		}

		p_converted = boost::dynamic_pointer_cast<target_type>(p);

		if(p_converted) return p_converted;
		else {
			raiseException(
					"In GBrokerConnectorT<T>::retrieveItem<target_type>(): Conversion error"
			);
		}
#else
		return boost::static_pointer_cast<target_type>(p);
#endif /* DEBUG */
	}

    /**********************************************************************************/
private:
	double waitFactor_; ///< Affects the timeout for returning individuals
	double minWaitFactor_; ///< The minimum allowed wait factor
	double maxWaitFactor_; ///< The maximum allowed wait factor
	double waitFactorIncrement_; ///< The amount by which the waitFactor_ may be incremented or decremented
	bool boundlessWait_; ///< Indicates whether the retrieveItem call should wait for an unlimited amount of time

	bool allItemsReturned_; ///< Indicates whether all items have returned before the timeout
	double percentOfTimeoutNeeded_; ///< Makes a note what percentage of the maximum timeout was needed in one iteration

    boost::posix_time::time_duration firstTimeOut_; ///< Maximum time frame for first individual
    boost::posix_time::ptime iterationStartTime_; ///< Temporary that holds the start time for the retrieval of items in a given iteration
    boost::posix_time::time_duration totalElapsedFirst_; ///< Temporary that holds the total elapsed time needed for retrieval of the first individual
    boost::posix_time::time_duration maxAllowedElapsed_; ///< Temporary that holds the maximum allowed elapsed time for all other individuals (as a function of totalElapsedFirst_
    boost::posix_time::time_duration totalElapsed_; ///< Temporary that holds the total elapsed time since the start of the retrieval procedure

    bool doLogging_; ///< Specifies whether arrival times of individuals should be logged
    std::vector<std::vector<boost::uint32_t> >  arrivalTimes_; ///< Holds the actual arrival times. Note: Neither serialized nor copied

    GBufferPortT_ptr CurrentBufferPort_; ///< Holds a GBufferPortT object during the calculation. Note: It is neither serialized nor copied

#ifdef GENEVATESTING
public:
    /**********************************************************************************/
    /**
     * Applies modifications to this object. This is needed for testing purposes
     *
     * @return A boolean which indicates whether modifications were made
     */
    bool modify_GUnitTests() {
    	bool result = false;
    	return result;
    }

    /**********************************************************************************/
    /**
     * Performs self tests that are expected to succeed. This is needed for testing purposes
     */
    void specificTestsNoFailureExpected_GUnitTests() {
    	/* nothing */
    }

    /**********************************************************************************/
    /**
     * Performs self tests that are expected to fail. This is needed for testing purposes
     */
    void specificTestsFailuresExpected_GUnitTests() {
    	/* nothing */
    }

    /**********************************************************************************/

#endif /* GENEVATESTING */
};

/************************************************************************************************************/

} /* Courtier */
} /* Gem */

#endif /* GBROKERCONNECTORT_HPP_ */
