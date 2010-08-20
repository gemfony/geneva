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
		 & BOOST_SERIALIZATION_NVP(maxWaitFactor_)
		 & BOOST_SERIALIZATION_NVP(firstTimeOut_)
		 & BOOST_SERIALIZATION_NVP(loopTime_)
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
    /** @brief Sets the wait factor, including automatic adaption of the factor */
    void setWaitFactor(const boost::uint32_t&, const boost::uint32_t&);
    /** @brief Retrieves the wait factor */
    boost::uint32_t getWaitFactor() const;
    /** @brief Retrieves the maximum wait factor used in automatic adaption of the wait factor */
    boost::uint32_t getMaxWaitFactor() const;

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
	/** @brief Allows to retrieve the logging results */
	std::vector<std::vector<boost::uint32_t> > getLoggingResults() const;

protected:
	boost::uint32_t waitFactor_; ///< Affects the timeout for returning individuals
	boost::uint32_t maxWaitFactor_; ///< Determines the maximum allowed wait factor during automatic adaption of the waitFactor_ variable

    boost::posix_time::time_duration firstTimeOut_; ///< Maximum time frame for first individual
    boost::posix_time::time_duration loopTime_; ///< Timeout for retrieval of items from the GBufferPortT queue

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

} /* namespace Geneva */
} /* namespace Gem */

#endif /* GBROKERCONNECTOR_HPP_ */
