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
#include <utility>

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
#include "common/GHelperFunctionsT.hpp"
#include "common/GParserBuilder.hpp"
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
    	 & BOOST_SERIALIZATION_NVP(maxResubmissions_)
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
    	, maxResubmissions_(DEFAULTMAXRESUBMISSIONS)
        , allItemsReturned_(true)
    	, percentOfTimeoutNeeded_(0.)
    	, submission_counter_(0)
        , firstTimeOut_(boost::posix_time::duration_from_string(DEFAULTBROKERFIRSTTIMEOUT))
        , doLogging_(false)
    	, CurrentBufferPort_(new Gem::Courtier::GBufferPortT<boost::shared_ptr<T> >())
    {
    	GBROKER(T)->enrol(CurrentBufferPort_);
    }

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
    	, maxResubmissions_(cp.maxResubmissions_)
    	, allItemsReturned_(true)
		, percentOfTimeoutNeeded_(0.)
    	, submission_counter_(0) // start new
    	, firstTimeOut_(cp.firstTimeOut_)
    	, doLogging_(cp.doLogging_)
    	, CurrentBufferPort_(new Gem::Courtier::GBufferPortT<boost::shared_ptr<T> >())
    {
    	GBROKER(T)->enrol(CurrentBufferPort_);
    }

    /**********************************************************************************/
    /**
     * The standard destructor. We have no object-wide dynamically allocated data, hence
     * this function is empty.
     */
    virtual ~GBrokerConnectorT()
    {
    	CurrentBufferPort_.reset();
    }

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
		maxResubmissions_ = cp->maxResubmissions_;
    	allItemsReturned_ = cp->allItemsReturned_;
		percentOfTimeoutNeeded_ = cp->percentOfTimeoutNeeded_;
		// We do not copy the submission_counter_
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
    	deviations.push_back(checkExpectation(withMessages, "GBrokerConnectorT<T>", maxResubmissions_, cp.maxResubmissions_, "maxResubmissions_", "cp.maxResubmissions_", e , limit));
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
    void setFirstTimeOut(boost::posix_time::time_duration firstTimeOut) {
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
    void setWaitFactorExtremes(double minWaitFactor, double maxWaitFactor) {
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
     * done. Note that only arrival times of items of the current submission are logged.
     * This also allows to find out how many items did not return before the deadline.
     *
     * @param dl A boolean whether logging of arrival times of items should be done
     */
    void doLogging(bool dl = true) {
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
    void getLoggingResults(std::vector<std::vector<boost::uint32_t> >& arrivalTimes) {
    	arrivalTimes = arrivalTimes_;
    	arrivalTimes_.clear(); // Cannot declare this function as "const" because of this call
    }

    /**********************************************************************************/
    /**
     * Specifies whether item retrievals should wait for an unlimited amount of time
     * for processed items.
     *
     * @param boundlessWait Indicates whether the waiting time for processed items should be unlimited
     */
    void setBoundlessWait(bool boundlessWait) {
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
     * Specifies how often work items should be resubmitted in the case a full return
     * of work items is expected.
     *
     * @param maxResubmissions The maximum number of allowed resubmissions
     */
    void setMaxResubmissions(std::size_t maxResubmissions) {
    	maxResubmissions_ = maxResubmissions;
    }

    /**********************************************************************************/
    /**
     * Returns the maximum number of allowed resubmissions
     *
     * @return The maximum number of allowed resubmissions
     */
    std::size_t getMaxResubmissions() const {
    	return maxResubmissions_;
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
     * Retrieves the current waitFactor_ variable.
     *
     * @return The value of the waitFactor_ variable
     */
    boost::uint32_t getWaitFactor() const  {
    	return waitFactor_;
    }

    /**********************************************************************************/
    /**
     * Allows to set the amount by which the waitFactor is incremented or decremented
     * during automatic adaption
     *
     * @param wfi The desired amount by which the wait factor gets incremented or decremented
     */
    void setWaitFactorIncrement(double wfi) {
    	if(wfi <= 0.) {
    		raiseException(
    			"In GBrokerConnectorT<T>::setWaitFactorIncrement(): Error!" << std::endl
    			<< "Received invalid wait factor increment: " << wfi << std::endl
    		);
    	}

    	waitFactorIncrement_ = wfi;
    }

    /**********************************************************************************/
    /**
     * Allows to retrieve the amount by which the waitFactor is incremented or decremented
     * during automatic adaption
     *
     * @return The amount by which the wait factor gets incremented or decremented
     */
    double getWaitFactorIncrement() const {
    	return waitFactorIncrement_;
    }

    /**********************************************************************************/
    /**
     * Submits and retrieves a set of work items. After the work has been performed,
     * the items contained in the workItems vector may have been changed. Note that,
     * depending on the submission mode, it is not guaranteed by this function that all
     * submitted items are still contained in the vector after the call. It is also possible
     * that returned items do not belong to the current submission cycle. You might thus have
     * to post-process the vector. The parameter "srm" of this function specifies whether
     * we expect all items to return (value EXPECTFULLRETURN), whether we do not expeect
     * all items to return but we do not accept items from older iterations (value
     * REJECTOLDERITEMS) or whether we accept an incomplete return but also accept
     * items from older iterations (value ACCEPTOLDERITEMS). Note that it is impossible to
     * submit items that are not derived from T.
     *
     * @param workItems A vector with work items to be evaluated beyond the broker
     * @param start The id of first item to be worked on in the vector
     * @param end The id beyond the last item to be worked on in the vector
     * @param srm An enum indicating whether all submitted items may return, or some may be missing
     * @return A boolean indicating whether all expected items have returned
     */
    template <typename work_item>
    bool workOn(
    	std::vector<boost::shared_ptr<work_item> >& workItems
    	, const std::size_t& start
    	, const std::size_t& end
    	, const submissionReturnMode& srm = ACCEPTOLDERITEMS
    	, typename boost::enable_if<boost::is_base_of<T, work_item> >::type* dummy = 0
    ) {
    	switch(srm) {
    	case ACCEPTOLDERITEMS:
    		return workOnIncompleteReturnAllowed(
    		    workItems
    		    , start
    		    , end
    		    , true // means: accept items from older iterations
    		);
    		break;

    	case REJECTOLDERITEMS:
    		return workOnIncompleteReturnAllowed(
    		    workItems
    		    , start
    		    , end
    		    , false // means: do not accept items from older iterations
    		);
    		break;

    	case EXPECTFULLRETURN:
    		return workOnFullReturnExpected(
    		    workItems
    		    , start
    		    , end
    		);
    		break;

    	default:
			{
				raiseException(
					"In GBrokerConnectorT<>::workOn(): Received invalid submissionReturnMode: " << srm << std::endl
				);
			}
			return false; // Make the compiler happy
			break;
    	}
    }

	/*********************************************************************************/
	/**
	 * Adds local configuration options to a GParserBuilder object
	 *
	 * @param gpb The GParserBuilder object to which configuration options should be added
	 * @param showOrigin Makes the function indicate the origin of parameters in comments
	 */
	void addConfigurationOptions (
		Gem::Common::GParserBuilder& gpb
		, const bool& showOrigin
	) {
		std::string comment;
		std::string comment1;
		std::string comment2;

		// no parent class

		// add local data
		comment = ""; // Reset the comment string
		comment += "The timeout for the retrieval of an;";
		comment += "iteration's first timeout;";
		if(showOrigin) comment += "[GBrokerConnectorT<T>]";
		gpb.registerFileParameter<boost::posix_time::time_duration>(
			"firstTimeOut" // The name of the variable
			, boost::posix_time::duration_from_string(DEFAULTBROKERFIRSTTIMEOUT) // The default value
			, boost::bind(
				&GBrokerConnectorT<T>::setFirstTimeOut
				, this
				, _1
			  )
			, Gem::Common::VAR_IS_SECONDARY // Alternative: VAR_IS_ESSENTIAL
			, comment
		);

		comment1 = ""; // Reset the first comment string
		comment1 += "The lower boundary for the adaption;";
		comment1 += "of the waitFactor variable;";
		comment2 = ""; // Reset the second comment string
		comment2 += "The upper boundary for the adaption;";
		comment2 += "of the waitFactor variable;";
		if(showOrigin) comment += "[GBrokerConnectorT<T>]";
		gpb.registerFileParameter<double, double>(
			"min" // The name of the first variable
			, "max" // The name of the second variable
			, DEFAULTMINBROKERWAITFACTOR // The first default value
			, DEFAULTMAXBROKERWAITFACTOR // The second default value
			, boost::bind(
				&GBrokerConnectorT<T>::setWaitFactorExtremes
				, this
				, _1
				, _2
			  )
			, "waitFactorExtremes"
			, Gem::Common::VAR_IS_ESSENTIAL // Alternative: VAR_IS_SECONDARY
			, comment1
			, comment2
		);

		comment =  ""; // 	Reset the comment string
		comment += "Specifies the amount by which the wait factor gets;";
		comment += "incremented or decremented during automatic adaption;";
		if(showOrigin) comment += "[GBrokerConnectorT<T>]";
		gpb.registerFileParameter<double>(
			"waitFactorIncrement" // The name of the variable
			, DEFAULTBROKERWAITFACTORINCREMENT // The default value
			, boost::bind (
				&GBrokerConnectorT<T>::setWaitFactorIncrement
				, this
				, _1
			  )
			, Gem::Common::VAR_IS_SECONDARY // Alternative:VAR_IS_ESSENTIAL
			, comment
		);

		comment = ""; // Reset the comment string
		comment += "Indicates that the broker connector should wait endlessly;";
		comment += "for further arrivals of individuals in an iteration;";
		if(showOrigin) comment += "[GBrokerConnectorT<T>]";
		gpb.registerFileParameter<bool>(
			"boundlessWait" // The name of the variable
			, false // The default value
			, boost::bind(
				&GBrokerConnectorT<T>::setBoundlessWait
				, this
				, _1
			  )
			, Gem::Common::VAR_IS_ESSENTIAL // Alternative: VAR_IS_SECONDARY
			, comment
		);

		comment = ""; // Reset the comment string
		comment += "The amount of resubmissions allowed if a full return of work;";
		comment += "items was expected but only a subset has returned;";
		if(showOrigin) comment += "[GBrokerConnectorT<T>]";
		gpb.registerFileParameter<bool>(
			"maxResubmissions" // The name of the variable
			, DEFAULTMAXRESUBMISSIONS // The default value
			, boost::bind(
				&GBrokerConnectorT<T>::setMaxResubmissions
				, this
				, _1
			  )
			, Gem::Common::VAR_IS_ESSENTIAL // Alternative: VAR_IS_SECONDARY
			, comment
		);

		comment = ""; // Reset the comment string
		comment += "Activates (1) or de-activates (0) logging;";
		comment += "iteration's first timeout;";
		if(showOrigin) comment += "[GBrokerConnectorT<T>]";
		gpb.registerFileParameter<bool>(
			"doLogging" // The name of the variable
			, false // The default value
			, boost::bind(
				&GBrokerConnectorT<T>::doLogging
				, this
				, _1
			  )
			, Gem::Common::VAR_IS_SECONDARY // Alternative: VAR_IS_ESSENTIAL
			, comment
		);
	}

private:
    /**********************************************************************************/
    /**
     * Submits and retrieves a set of work items. After the work has been performed,
     * the items contained in the workItems vector may have been changed. Note that
     * it is not guaranteed by this function that all submitted items are still contained
     * in the vector after the call. It is also possible that returned items do not
     * belong to the current submission cycle. You will thus have to post-process the vector.
     * Note that it is impossible to submit items that are not derived from T.
     *
     * @param workItems A vector with work items to be evaluated beyond the broker
     * @param start The id of first item to be worked on in the vector
     * @param end The id beyond the last item to be worked on in the vector
     * @param acceptOlderItems A boolean indicating whether older items should be accepted
     * @return A boolean indicating whether all expected items have returned
     */
    template <typename work_item>
    bool workOnIncompleteReturnAllowed(
    	std::vector<boost::shared_ptr<work_item> >& workItems
    	, const std::size_t& start
    	, const std::size_t& end
    	, const bool& acceptOlderItems = true
    	, typename boost::enable_if<boost::is_base_of<T, work_item> >::type* dummy = 0
    ) {
    	std::size_t expectedNumber = end - start; // The expected number of work items from the current iteration
    	std::size_t nReceivedCurrent = 0; // The number of items of this iteration received so far
    	std::size_t nReceivedOlder = 0; // The number of items from older iterations received so far

    	// Signal a new job submission
    	markNewSubmission();

#ifdef DEBUG
    	// Do some error checking
    	if(workItems.empty()) {
    		raiseException(
    				"In GBrokerConnectorT<T>::workOnIncompleteReturnAllowed(): Error!" << std::endl
    				<< "workItems_ vector is empty." << std::endl
    		);
    	}

    	if(end <= start) {
    		raiseException(
    				"In GBrokerConnectorT<T>::workOnIncompleteReturnAllowed(): Error!" << std::endl
    				<< "Invalid start or end-values: " << start << " / " << end << std::endl
    		);
    	}

    	if(end > workItems.size()) {
    		raiseException(
    				"In GBrokerConnectorT<T>::workOnIncompleteReturnAllowed(): Error!" << std::endl
    				<< "Last id " << end << " exceeds size of vector " << workItems.size() << std::endl
    		);
    	}
#endif /* DEBUG */

    	//-------------------------------------------------------------------------------
    	// First submit all items
    	typename std::vector<boost::shared_ptr<work_item> >::iterator it;
    	POSITIONTYPE pos_cnt = start;
    	for(it=workItems.begin()+start; it!=workItems.begin()+end; ++it) {
    		(*it)->setCourtierId(boost::make_tuple<SUBMISSIONCOUNTERTYPE, POSITIONTYPE>(submission_counter_, pos_cnt));
    		pos_cnt++;
    		this->submit(*it);
    	}

    	// Remove the submitted items, we do not need them anymore
    	workItems.erase(workItems.begin()+start, workItems.begin()+end);

    	//-------------------------------------------------------------------------------
    	// Now wait for the first item of the current iteration to return from its journey

    	boost::shared_ptr<work_item> p; // Will hold returned items

    	// First wait for the first work item of the current iteration to return.
    	// Items from older iterations will also be accepted in this loop. Their
    	// arrival will not terminate this loop, though.
    	while(true) {
    		using namespace boost;

    		// Note: the following call will throw if a timeout has been reached.
    		p = retrieveFirstItem<work_item>();

    		// If it is from the current iteration, break the loop, otherwise continue,
			// until the first item of the current iteration has been received.
    		if(submission_counter_ == (p->getCourtierId()).get<0>()) {
    			// Add the item to the workItems vector at the start of the range
    			workItems.insert(workItems.begin()+start, p);
    			// Update the counter
    			nReceivedCurrent++;
    			break;
    		} else {
    			if(acceptOlderItems) {
    				// Add the item to the workItems vector at the start of the range
    				workItems.insert(workItems.begin()+start, p);
    			} else {
    				p.reset();
    			}

    			// Update the counter
    			nReceivedOlder++;
    		}
    	}

    	// Now wait for further arrivals for a predefined amount of time.
    	// retrieveItem will return an empty pointer, if the timeout has been reached
    	while(nReceivedCurrent!=expectedNumber && (p=retrieveItem<work_item>())) {
    		using namespace boost;

			// Update the counters and insert items
    		if(submission_counter_ == (p->getCourtierId()).get<0>()) {
    			// Add the item to the workItems vector at the start of the range
    			workItems.insert(workItems.begin()+start, p);
    			// Update the counter
    			nReceivedCurrent++;
    		} else {
    			if(acceptOlderItems) {
    				// Add the item to the workItems vector at the start of the range
    				workItems.insert(workItems.begin()+start, p);
    			} else {
    				p.reset();
    			}

    			// Update the counter
    			nReceivedOlder++;
    		}
    	}

    	// Determine whether we have received a complete set of work items
    	bool complete = false;
    	if(nReceivedCurrent == expectedNumber) {
    		complete = true;
    	}

    	//-------------------------------------------------------------------------------
    	// Emit some information in DEBUG mode
#if DEBUG
    	std::ostringstream information;
    	if(!complete) {
    		information	<< std::endl
    			<< "Incomplete submission " << submission_counter_ << ":" << std::endl;
    	} else {
    		std::ostringstream information;
    		information	<< std::endl
    			<< "Complete submission_ " << submission_counter_ << ":" << std::endl;
    	}
    	information
			<< "nReceivedCurrent = " << nReceivedCurrent << std::endl
			<< "expectedNumber   = " << expectedNumber << std::endl
			<< "nReceivedOlder   = " << nReceivedOlder << std::endl
			<< "waitFactor = " << waitFactor_ << std::endl;
    	std::cout << information.str();
#endif /* DEBUG */
    	//-------------------------------------------------------------------------------

    	// Update the submission counter
    	submission_counter_++;

    	return complete;
    }

    /**********************************************************************************/
    /**
     * Submits and retrieves a set of work items. If some items didn't return inside
     * of the allowed time frame, the function will resubmit them up to a configurable
     * number of times. Items from older iterations will be discarded. After the work
     * has been performed, the items contained in the workItems vector may have been
     * changed. The workItems vector will remain unchanged if we didn't receive all items
     * back. Note that it is impossible to submit items that are not derived from T.
     *
     * @param workItems A vector with work items to be evaluated beyond the broker
     * @param start The id of first item to be worked on in the vector
     * @param end The id beyond the last item to be worked on in the vector
     * @param maxResubmissions The maximum allowed number of re-submissions of missing items
     * @return A boolean indicating whether all expected items have returned
     */
    template <typename work_item>
    bool workOnFullReturnExpected(
		std::vector<boost::shared_ptr<work_item> >& workItems
		, const std::size_t& start
		, const std::size_t& end
		, typename boost::enable_if<boost::template is_base_of<T, work_item> >::type* dummy = 0
    ) {
    	std::size_t expectedNumber = end - start; // The expected number of work items from the current iteration
    	std::size_t nReceivedCurrent = 0; // The number of items of this iteration received so far
    	std::size_t nReceivedOlder = 0; // The number of items from older iterations received so far

    	std::vector<boost::shared_ptr<work_item> > returnedItems; // Holds work items that have returned from processing

    	// Signal a new submission
    	markNewSubmission();

#ifdef DEBUG
    	// Do some error checking
    	if(workItems.empty()) {
    		raiseException(
    				"In GBrokerConnectorT<T>::workOnFullReturnExpected(): Error!" << std::endl
    				<< "workItems_ vector is empty." << std::endl
    		);
    	}

    	if(end <= start) {
    		raiseException(
    				"In GBrokerConnectorT<T>::workOnFullReturnExpected(): Error!" << std::endl
    				<< "Invalid start or end-values: " << start << " / " << end << std::endl
    		);
    	}

    	if(end > workItems.size()) {
    		raiseException(
    				"In GBrokerConnectorT<T>::workOnFullReturnExpected(): Error!" << std::endl
    				<< "Last id " << end << " exceeds size of vector " << workItems.size() << std::endl
    		);
    	}
#endif /* DEBUG */

    	//-------------------------------------------------------------------------------
    	// First submit all items
    	typename std::vector<boost::shared_ptr<work_item> >::iterator it;
    	POSITIONTYPE pos_cnt = start;
    	for(it=workItems.begin()+start; it!=workItems.begin()+end; ++it) {
    		(*it)->setCourtierId(boost::make_tuple<SUBMISSIONCOUNTERTYPE,POSITIONTYPE>(submission_counter_, pos_cnt));
    		pos_cnt++;
    		this->submit(*it);
    	}

    	// Create a vector of size workItems.size() with flags indicating whether
    	// items have returned or whether this is a position that is not intended for submission
    	std::vector<std::size_t> returnedItemPos(workItems.size());
    	// Initialize with 2s
    	Gem::Common::assignVecConst(returnedItemPos, std::size_t(2));
    	// Initialize positions of items that have been submitted with 0s
    	for(std::size_t i=0; i<expectedNumber; i++) {
    		returnedItemPos[start+i] = std::size_t(0);
    	}

       	//-------------------------------------------------------------------------------
    	// Now wait for the first item of the current iteration to return from its journey

    	boost::shared_ptr<work_item> p; // Will hold returned items
       	bool complete = false; // Indicates whether we have received all items back
    	// First wait for the first work item of the current iteration to return.
    	// Items from older iterations will also be accepted in this loop. Their
    	// arrival will not terminate this loop, though.
    	while(true) {
    		using namespace boost;

    		// Note: the following call will throw if a timeout has been reached.
    		p = retrieveFirstItem<work_item>();

    		// If it is from the current iteration, break the loop, otherwise continue,
    		// until the first item of the current iteration has been received.
    		if(submission_counter_ == (p->getCourtierId()).get<0>()) {
    			// Make a note about this items return in the returnedItemPos vector
    			returnedItemPos[(p->getCourtierId()).get<1>()] = 1;

    			// Add the item to the list of returned objects
    			returnedItems.push_back(p);

    			// Update the counter and check whether we have received all items back
    			if(expectedNumber == ++nReceivedCurrent) {
    				complete = true;
    			}

    			break;
    		} else {
    			// Reject items from previous iterations
    			p.reset();

    			// Update the counter
    			nReceivedOlder++;
    		}
    	}

    	//-------------------------------------------------------------------------------
    	// Wait for further arrivals. Resubmit items, when we run into a timeout
    	std::size_t retry_counter = 0;
    	while(!complete && retry_counter < maxResubmissions_) {
    		p = retrieveItem<work_item>();
    		if (p) { // Did we receive a valid item ?
    			using namespace boost;

    			// Check whether the received item hasn't been added already or comes from an older submission
    			if(1 == returnedItemPos[(p->getCourtierId()).get<1>()] || submission_counter_ != (p->getCourtierId()).get<0>()) {
    				p.reset();
    			} else {
        			// Make a note about this items return in the returnedItemPos vector
        			returnedItemPos[(p->getCourtierId()).get<1>()] = 1;

        			// Add the item to the list of returned objects
        			returnedItems.push_back(p);


        			// Update the counter and check whether we have received all items back
        			if(expectedNumber == ++nReceivedCurrent) {
        				complete = true;
        			}
    			}
    		} else { // O.k., so we ran into a timeout
    			// Resubmit all items which have not been marked as "returned"
    			for(std::size_t i=0; i<returnedItemPos.size(); i++) {
    				if(0 == returnedItemPos[i]) {
    					this->submit(workItems[i]);
    				}
    			}

    			// Make sure we do not immediately run into a timeout
    			prolongTimeout();

    			// Make it known that we have done a re-submission
    			retry_counter++;
    		}
    	}

    	// Sort received items according to their position
    	if(complete) {
#ifdef DEBUG
    		if(returnedItems.size() != expectedNumber) {
    			raiseException(
    					"In GBrokerConnectorT<T>::workOnFullReturnExpected(): Error!" << std::endl
    					<< "Expected " << expectedNumber << " items to have returned" << std::endl
    					<< "but received " << returnedItems.size() << std::endl
    			);
    		}
#endif /* DEBUG */

    		std::sort(returnedItems.begin(), returnedItems.end(), courtierPosComp<work_item>());

#ifdef DEBUG
    		for(std::size_t i=0; i<returnedItems.size(); i++) {
    			using namespace boost;

    			if((returnedItems[i]->getCourtierId()).get<1>() != start+i){
    				raiseException(
    						"In GBrokerConnectorT<T>::workOnFullReturnExpected(): Error!" << std::endl
							<< "Expected item with position id " << (returnedItems[i]->getCourtierId()).get<1>() << std::endl
							<< "to have id " << start+i << " instead." << std::endl
    				);
    			}
    		}
#endif /* DEBUG */

        	// Insert returned items back into the workItems vector. As this only
    		// happens when all items have been received back, the workItems vector
    		// will remain untouched in case of a problem.
        	for(std::size_t i=0; i<returnedItems.size(); i++) {
        		workItems[start+i] = returnedItems[i];
        	}
    	}

    	// Get rid of the returnItems items
    	returnedItems.clear();

    	// Update the submission counter
    	submission_counter_++;

    	// Let the audience know whether we were able to retrieve all items back
    	// with the number of allowed re-submissions
    	return complete;
    }

	/**********************************************************************************/
    /**
     * Allows to perform any work necessary to be repeated for each new submission. In
     * particular, this function adjusts the waitFactor_ variable, so that it fits
     * the current load pattern of the computing resources behind the broker.
 	 */
    void markNewSubmission() {
    	// If logging is enabled, add a std::vector<boost::uint32_t> for the current submission to arrivalTimes_
    	if(doLogging_) arrivalTimes_.push_back(std::vector<boost::uint32_t>());

    	// Adapting the wait factor only makes sense if we haven't been ordered to wait endlessly anyway
    	if(!boundlessWait_ && submission_counter_ > 0) {
			// Increment or decrement the waitFactor_ variable, based on the current load of the system
			// Over the course of a few submission, waitFactor_ should adjust itself into the correct range
			if(!allItemsReturned_) {
				if(waitFactor_ < maxWaitFactor_) {
					waitFactor_ += waitFactorIncrement_;

					// std::cout << "Increased wait factor to " << waitFactor_ << std::endl;

					// Make sure we do not accidently slip above the maximum allowed wait factor
					if(waitFactor_ > maxWaitFactor_) {
						waitFactor_ = maxWaitFactor_;
					}
				}
			} else if(percentOfTimeoutNeeded_ < DEFAULTMINPERCENTAGEOFTIMEOUT) {
				if(waitFactor_ > minWaitFactor_) {
					waitFactor_ -= waitFactorIncrement_;

					// std::cout << "Decreased wait factor to " << waitFactor_ << std::endl;

					// Make sure we do not accidently slip below the minimum wait factor
					if(waitFactor_ < minWaitFactor_) {
						waitFactor_ = minWaitFactor_;
					}
				}
			}
    	}

		// Reset the allItemsReturned_ variable. We assume that all items
    	// will return before the timeout in the next iteration. This assumption
    	// may be falsified later if we run into the timeout.
		allItemsReturned_ = true;

    	// Set the start time of the new iteration so we calculate a correct
    	// return time for the first individual, regardless of whether older
    	// individuals have returned first.
    	iterationStartTime_ = boost::posix_time::microsec_clock::local_time();
    }

    /**********************************************************************************/
    /**
     * Prolongs the timeout. This is useful when there is a need for re-submission of
     * individuals, such as in gradient descents.
     */
    void prolongTimeout() {
    	// Update the maximum threshold
    	maxAllowedElapsed_ += boost::posix_time::microseconds(boost::numeric_cast<long>(double(totalElapsedFirst_.total_microseconds()) * (waitFactor_ + 1.)));
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
			if(!CurrentBufferPort_->pop_back_processed_bool(p, firstTimeOut_)) {
				raiseException(
						"In GBrokerConnectorT<T>::retrieveFirstItem():" << std::endl
						<< "Timeout for first item reached." << std::endl
						<< "Current timeout setting in microseconds is " << firstTimeOut_.total_microseconds() << std::endl
						<< "You can change this value with the setFirstTimeOut() function."
				);
			}
		}
		else { // Wait indefinitely for the first item to return
			CurrentBufferPort_->pop_back_processed(p);
		}

		// At this point we have received the first individual of the current iteration back.
		// Record the elapsed time and calculate the time for which other individuals are
		// allowed to return
		totalElapsedFirst_ = boost::posix_time::microsec_clock::local_time()-iterationStartTime_;
		maxAllowedElapsed_ = boost::posix_time::microseconds(boost::numeric_cast<long>(double(totalElapsedFirst_.total_microseconds()) * (waitFactor_ + 1.)));

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
			if(!CurrentBufferPort_->pop_back_processed_bool(p, firstTimeOut_)) {
				raiseException(
						"In GBrokerConnectorT<T>::retrieveFirstItem<target_type>():" << std::endl
						<< "Timeout for first item reached." << std::endl
						<< "Current timeout setting in microseconds is " << firstTimeOut_.total_microseconds() << std::endl
						<< "You can change this value with the setFirstTimeOut() function."
				);
			}
		}
		else { // Wait indefinitely for the first item to return
			CurrentBufferPort_->pop_back_processed(p);
		}

		// At this point we have received the first individual of the current generation back.
		// Record the elapsed time and calculate the time for which other individuals are
		// allowed to return
		totalElapsedFirst_ = boost::posix_time::microsec_clock::local_time()-iterationStartTime_;
		maxAllowedElapsed_ = boost::posix_time::microseconds(boost::numeric_cast<long>(double(totalElapsedFirst_.total_microseconds()) * (waitFactor_ + 1.)));

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
			CurrentBufferPort_->pop_back_processed(p);
		} else { // Observe a timeout
			// Calculate how much time has elapsed since the start of the iteration
			currentElapsed = boost::posix_time::microsec_clock::local_time()-iterationStartTime_;

			// Have we already exceeded the timeout ? Or was it impossible to retrieve an item in the remaining time before the timeout ?
			if((currentElapsed > maxAllowedElapsed_) || !CurrentBufferPort_->pop_back_processed_bool(p, maxAllowedElapsed_-currentElapsed)) {
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
				// signaled by "markNewSubmission()", it will hold the latest percentage.
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
			CurrentBufferPort_->pop_back_processed(p);
		} else { // Observe a timeout
			// Calculate how much time has elapsed since the start of the iteration
			currentElapsed = boost::posix_time::microsec_clock::local_time()-iterationStartTime_;

			// Have we already exceeded the timeout ? Or was it impossible to retrieve an item in the remaining time before the timeout ?
			if((currentElapsed > maxAllowedElapsed_) || !CurrentBufferPort_->pop_back_processed_bool(p, maxAllowedElapsed_-currentElapsed)) {
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
				// signaled by "markNewSubmission()", it will hold the latest percentage.
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

	/*********************************************************************************/
    /**
     * A simple comparison operator that helps to sort individuals according to their
     * position in the population. Smaller position numbers will end up in front.
     */
	template <typename work_item>
    struct courtierPosComp {
    	bool operator()(boost::shared_ptr<work_item> x, boost::shared_ptr<work_item> y) {
    		using namespace boost;
    		return (x->getCourtierId()).get<1>() < (y->getCourtierId()).get<1>();
    	}
    };

    /**********************************************************************************/

	double waitFactor_; ///< Affects the timeout for returning individuals
	double minWaitFactor_; ///< The minimum allowed wait factor
	double maxWaitFactor_; ///< The maximum allowed wait factor
	double waitFactorIncrement_; ///< The amount by which the waitFactor_ may be incremented or decremented
	bool boundlessWait_; ///< Indicates whether the retrieveItem call should wait for an unlimited amount of time

    std::size_t maxResubmissions_; ///< The maximum number of resubmissions allowed if a full return of submitted items is expected

	bool allItemsReturned_; ///< Indicates whether all items have returned before the timeout
	double percentOfTimeoutNeeded_; ///< Makes a note what percentage of the maximum timeout was needed in one iteration

	SUBMISSIONCOUNTERTYPE submission_counter_; ///< Counts the number of submissions initiated by this object. NOTE: not serialized!

    boost::posix_time::time_duration firstTimeOut_; ///< Maximum time frame for first individual
    boost::posix_time::ptime iterationStartTime_; ///< Temporary that holds the start time for the retrieval of items in a given iteration
    boost::posix_time::time_duration totalElapsedFirst_; ///< Temporary that holds the total elapsed time needed for retrieval of the first individual
    boost::posix_time::time_duration maxAllowedElapsed_; ///< Temporary that holds the maximum allowed elapsed time for all other individuals (as a function of totalElapsedFirst_
    boost::posix_time::time_duration totalElapsed_; ///< Temporary that holds the total elapsed time since the start of the retrieval procedure

    bool doLogging_; ///< Specifies whether arrival times of individuals should be logged
    std::vector<std::vector<boost::uint32_t> >  arrivalTimes_; ///< Holds the actual arrival times. Note: Neither serialized nor copied

    GBufferPortT_ptr CurrentBufferPort_; ///< Holds a GBufferPortT object during the calculation. Note: It is neither serialized nor copied

#ifdef GEM_TESTING
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

#endif /* GEM_TESTING */
};

/************************************************************************************************************/

} /* Courtier */
} /* Gem */

#endif /* GBROKERCONNECTORT_HPP_ */
