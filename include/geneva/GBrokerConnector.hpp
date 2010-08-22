/**
 * @file GBrokerConnector.hpp
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

#ifndef GBROKERCONNECTOR_HPP_
#define GBROKERCONNECTOR_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif

// Geneva headers go here
#include "common/GExceptions.hpp"
#include "common/GPODExpectationChecksT.hpp"
#include "courtier/GBufferPortT.hpp"
#include "GOptimizationEnums.hpp"
#include "GIndividual.hpp"

namespace Gem {
namespace Geneva {

/**********************************************************************************/
/**
 * This class centralizes some functionality and data that is needed to connect
 * optimization algorithms to networked execution through Geneva's broker. This
 * class helps to avoid duplication of code in GBrokerSwarm and GBrokerEA (as well
 * as other optimization algorithms that may be implemeted later).
 */
class GBrokerConnector
{
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template<typename Archive>
    void serialize(Archive & ar, const unsigned int){
      using boost::serialization::make_nvp;

      ar & BOOST_SERIALIZATION_NVP(waitFactor_)
		 & BOOST_SERIALIZATION_NVP(firstTimeOut_)
		 & BOOST_SERIALIZATION_NVP(doLogging_);
    }
    ///////////////////////////////////////////////////////////////////////

public:
    typedef boost::shared_ptr<Gem::Courtier::GBufferPortT<boost::shared_ptr<Gem::Geneva::GIndividual> > > GBufferPortT_ptr;

	/** @brief The default constructor */
	GBrokerConnector();
	/** @brief The copy constructor */
	GBrokerConnector(const GBrokerConnector&);
	/** @brief The destructor */
	virtual ~GBrokerConnector();

	/** @brief The assginment operator */
	const GBrokerConnector& operator=(const GBrokerConnector& cp);

    void load(GBrokerConnector const * const);

	/** @brief Checks the relationship between this object and another object */
	boost::optional<std::string> checkRelationshipWith(
			const GBrokerConnector&
		  , const Gem::Common::expectation&
		  , const double&
		  , const std::string&
		  , const std::string&
		  , const bool&
	) const;

	/** @brief Intentionally left undefined */
	bool operator==(const GBrokerConnector&) const;
	/** @brief Intentionally left undefined */
	bool operator!=(const GBrokerConnector&) const;

    /** @brief Sets the wait factor */
    void setWaitFactor(const boost::uint32_t&);
    /** @brief Retrieves the wait factor */
    boost::uint32_t getWaitFactor() const;

    /** @brief Sets the first timeout factor */
    void setFirstTimeOut(const boost::posix_time::time_duration&);
    /** @brief Retrieves the first timeout factor */
    boost::posix_time::time_duration getFirstTimeOut() const;

    /** @brief Sets the loop time */
    void setLoopTime(const boost::posix_time::time_duration&);
    /** @brief Retrieves the second part of the loop time */
    boost::posix_time::time_duration getLoopTime() const;

	/** @brief Allows to specify whether logging of arrival times of individuals should be done */
	void doLogging(const bool& dl=true);
	/** @brief Allows to determine whether logging of arrival times has been activated */
	bool loggingActivated() const;
	/** @brief Instructs GBrokerConnector to performing logging activities */
	void log();
	/** @brief Allows to retrieve the logging results */
	std::vector<std::vector<boost::uint32_t> > getLoggingResults() const;

protected:
	/** @brief Performs necessary initialization work after an optimization run */
	void init();
	/** @brief Performs necessary finalization work after an optimization run */
	void finalize();

	/** @brief Allows to perform any work necessary to be repeated in each new iteration */
	void markNewIteration();

	/** @brief Allows to submit GIndividual-derivatives */
	void submit(boost::shared_ptr<GIndividual>);

	/******************************************************************************/
	/**
	 * Retrieval of the first individual and conversion to target type. Note that
	 * this function will throw if the maximum allowed time for the retrieval of
	 * the first item has been surpassed (if set).
	 *
	 * @return The retrieved item, converted into the desired target type
	 */
	template <typename ind_type>
	boost::shared_ptr<ind_type> retrieveFirstItem() {
		// Holds the retrieved item
		boost::shared_ptr<GIndividual> p;

		if(firstTimeOut_.total_microseconds()) { // Wait for a given maximum amount of time
			// pop_back_processed_bool will return false if we have reached the timeout
			// We cannot continue in this case. It is recommended to set firstTimeOut_
			// to a rather high value or to alternatively disable it completely by setting it
			// to EMPTYDURATION.
			if(!CurrentBufferPort_->pop_back_processed_bool(&p, firstTimeOut_)) {
				std::ostringstream error;
				error << "In GBrokerConnector::retrieveFirstItem(): Error!" << std::endl
					  << "Timeout for first item reached." << std::endl
					  << "Current timeout setting in microseconds is " << firstTimeOut_.total_microseconds() << std::endl
					  << "You can change this value with the setFirstTimeOut() function." << std::endl;
				throw Gem::Common::gemfony_error_condition(error.str());
			}
		}
		else { // Wait indefinitely for the first item to return
			CurrentBufferPort_->pop_back_processed(&p);
		}

		// At this point we have received the first individual of the current generation back.
		// Record the elapsed time and calculated the time until which other individuals are
		// allowed to return
		totalElapsedFirst_ = boost::posix_time::microsec_clock::local_time()-iterationStartTime_;
		maxAllowedElapsed_ = totalElapsedFirst_ * waitFactor_;

		// Convert the item to the target type. Note that there is a specialization
		// of this function in case ind_type == GIndividual
#ifdef DEBUG
		// Check that p actually points somewhere
		if(!p) {
			std::ostringstream error;
			error << "In GBrokerConnector::retrieveFirstItem<ind_type>(): Empty pointer found" << std::endl;
			throw Gem::Common::gemfony_error_condition(error.str());
		}

		boost::shared_ptr<ind_type> p_converted = boost::dynamic_pointer_cast<ind_type>(p);

		if(p_converted) return p_converted;
		else {
			std::ostringstream error;
			error << "In GBrokerConnector::retrieveFirstItem<ind_type>(): Conversion error" << std::endl;
			throw Gem::Common::gemfony_error_condition(error.str());
		}
#else
		return boost::static_pointer_cast<ind_type>(p);
#endif /* DEBUG */
	}

	/******************************************************************************/
	/**
	 * Retrieval of individuals and conversion to target type. This function will
	 * return individuals as long as the elapsed time hasn't surpassed the allotted
	 * time-frame. Once this has happened, it will return an empty pointer. Note that
	 * there is a specialization of this function in case ind_type == GIndividual.
	 *
	 * @return An individual from the processed queue, converted to the desired target type
	 */
	template <typename ind_type>
	boost::shared_ptr<ind_type> retrieveItem() {
		// Will hold retrieved items
		boost::shared_ptr<GIndividual> p;
		// Will hold converted items
		boost::shared_ptr<ind_type> p_converted;

		if(waitFactor_) { // Have we been asked to consider a possible time-out ?
			if(!CurrentBufferPort_->pop_back_processed_bool(&p,	maxAllowedElapsed_-(boost::posix_time::microsec_clock::local_time()-iterationStartTime_))) {
				return p_converted; // will be empty
			}
		} else {// Wait indefinitely for the next item
			CurrentBufferPort_->pop_back_processed(&p);
		}

		// If we have reached this point, we should have a valid p. Convert to the target type.
#ifdef DEBUG
		// Check that p actually points somewhere
		if(!p) {
			std::ostringstream error;
			error << "In GBrokerConnector::retrieveItem<ind_type>(): Empty pointer found" << std::endl;
			throw Gem::Common::gemfony_error_condition(error.str());
		}

		p_converted = boost::dynamic_pointer_cast<ind_type>(p);

		if(p_converted) return p_converted;
		else {
			std::ostringstream error;
			error << "In GBrokerConnector::retrieveItem<ind_type>(): Conversion error" << std::endl;
			throw Gem::Common::gemfony_error_condition(error.str());
		}
#else
		return boost::static_pointer_cast<ind_type>(p);
#endif /* DEBUG */
	}

	/******************************************************************************/
private:
	boost::uint32_t waitFactor_; ///< Affects the timeout for returning individuals

    boost::posix_time::time_duration firstTimeOut_; ///< Maximum time frame for first individual
    boost::posix_time::ptime iterationStartTime_; ///< Temporary that holds the start time for the retrieval of items in a given iteration
    boost::posix_time::time_duration totalElapsedFirst_; ///< Temporary that holds the total elapsed time needed for retrieval of the first individual
    boost::posix_time::time_duration maxAllowedElapsed_; ///< Temporary that holds the maximum allowed elapsed time for all other individuals (as a function of totalElapsedFirst_
    boost::posix_time::time_duration totalElapsed_; ///< Temporary that holds the total elapsed time since the start of the retrieval procedure

    bool doLogging_; ///< Specifies whether arrival times of individuals should be logged
    std::vector<std::vector<boost::uint32_t> >  arrivalTimes_; ///< Holds the actual arrival times. Note: Neither serialized nor copied

    GBufferPortT_ptr CurrentBufferPort_; ///< Holds a GBufferPortT object during the optimization cycle. Note: Neither serialized nor copied

#ifdef GENEVATESTING
public:
	/** @brief Applies modifications to this object. This is needed for testing purposes */
	virtual bool modify_GUnitTests();
	/** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
	virtual void specificTestsNoFailureExpected_GUnitTests();
	/** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
	virtual void specificTestsFailuresExpected_GUnitTests();
#endif /* GENEVATESTING */
};

/**********************************************************************************/
// Specializations for ind_type == GIndividual
template <> boost::shared_ptr<GIndividual> GBrokerConnector::retrieveFirstItem<GIndividual>();
template <> boost::shared_ptr<GIndividual> GBrokerConnector::retrieveItem<GIndividual>();

/**********************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

#endif /* GBROKERCONNECTOR_HPP_ */
