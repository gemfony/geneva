/**
 * @file GBrokerSwarm.hpp
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
#include <boost/serialization/nvp.hpp>
#include <boost/date_time.hpp>
#include <boost/date_time/gregorian/greg_serialize.hpp>
#include <boost/date_time/posix_time/time_serialize.hpp>

#ifndef GBROKERSWARM_HPP_
#define GBROKERSWARM_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif

// Geneva headers go here
#include "common/GExceptions.hpp"
#include "courtier/GBufferPortT.hpp"
#include "GSwarmPersonalityTraits.hpp"
#include "GSwarm.hpp"
#include "GBrokerConnector.hpp"

namespace Gem
{
namespace Geneva
{

/**********************************************************************************/
/**
 * This class implements a swarm algorithm with the ability to delegate certain
 * tasks to remote clients, using Geneva's broker infrastructure.
 */
class GBrokerSwarm
  : public GSwarm
  , public GBrokerConnector
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int){
		using boost::serialization::make_nvp;

		ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(GSwarm)
		   & BOOST_SERIALIZATION_BASE_OBJECT_NVP(GBrokerConnector);
	}
	///////////////////////////////////////////////////////////////////////

public:
	/** @brief The standard constructor */
    GBrokerSwarm(const std::size_t&, const std::size_t&);
    /** @brief A standard copy constructor */
    GBrokerSwarm(const GBrokerSwarm&);
    /** @brief The standard destructor */
    virtual ~GBrokerSwarm();

    /** @brief A standard assignment operator */
    const GBrokerSwarm& operator=(const GBrokerSwarm&);

	/** @brief Checks for equality with another GBrokerSwarm object */
	bool operator==(const GBrokerSwarm&) const;
	/** @brief Checks for inequality with another GBrokerSwarm object */
	bool operator!=(const GBrokerSwarm&) const;

	/** @brief Checks whether this object fulfills a given expectation in relation to another object */
	virtual boost::optional<std::string> checkRelationshipWith(const GObject&, const Gem::Common::expectation&, const double&, const std::string&, const std::string&, const bool&) const;

	/** @brief The actual business logic to be performed during each iteration. Returns the best achieved fitness */
	virtual double cycleLogic();
	/** @brief Performs any necessary initialization work before the start of the optimization cycle */
	virtual void init();
	/** @brief Performs any necessary finalization work after the end of the optimization cycle */
	virtual void finalize();

protected:
    /** @brief Loads the data of another GTransfer Population */
    virtual void load_(const GObject *);
    /** @brief Creates a deep copy of this object */
    virtual GObject *clone_() const;

	/** @brief The default constructor. Intentionally empty, as it is only needed for de-serialization purposes. */
	GBrokerSwarm(){}

    /*********************************************************************************/

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

#endif /* GBROKERSWARM_HPP_ */
