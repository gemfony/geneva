/**
 * @file GMultiThreadedSwarm.hpp
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

// Includes check for correct Boost version(s)
#include "common/GGlobalDefines.hpp"

// Boost headers go here

#include <boost/cast.hpp>
#include <boost/cstdint.hpp>
#include <boost/bind.hpp>
#include <boost/utility.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>
#include <boost/thread/xtime.hpp>
#include <common/thirdparty/boost/threadpool.hpp>

#ifndef GMULTITHREADEDSWARM_HPP_
#define GMULTITHREADEDSWARM_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif


// Geneva headers go here
#include "common/GExceptions.hpp"
#include "GSwarm.hpp"
#include "GIndividual.hpp"
#include "GObject.hpp"

namespace Gem {
namespace Geneva {

/** @brief The default number of threads for parallelization with boost */
const boost::uint16_t DEFAULTBOOSTTHREADSSWARM = 2;

/********************************************************************/
/**
 * A multi-threaded swarm based on GSwarm. This version uses the
 * Boost.Threads library and a thread-pool library from http://threadpool.sf.net .
 */
class GMultiThreadedSwarm
	: public GSwarm
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int) {
		using boost::serialization::make_nvp;

		ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(GSwarm)
		   & BOOST_SERIALIZATION_NVP(nThreads_);
	}
	///////////////////////////////////////////////////////////////////////

public:
	/** @brief The default constructor */
	GMultiThreadedSwarm(const std::size_t&, const std::size_t&);
	/** @brief A standard copy constructor */
	GMultiThreadedSwarm(const GMultiThreadedSwarm&);
	/** @brief The destructor */
	virtual ~GMultiThreadedSwarm();

	/** @brief A standard assignment operator */
	const GMultiThreadedSwarm& operator=(const GMultiThreadedSwarm&);

	/** @brief Checks for equality with another GMultiThreadedSwarm object */
	bool operator==(const GMultiThreadedSwarm&) const;
	/** @brief Checks for inequality with another GMultiThreadedSwarm object */
	bool operator!=(const GMultiThreadedSwarm&) const;

	/** @brief Checks whether this object fulfills a given expectation in relation to another object */
	virtual boost::optional<std::string> checkRelationshipWith(const GObject&, const Gem::Common::expectation&, const double&, const std::string&, const std::string&, const bool&) const;

	/** @brief Loads a checkpoint from disk */
	virtual void loadCheckpoint(const std::string&);

	/** @brief Sets the maximum number of threads */
	void setNThreads(const boost::uint8_t&);
	/** @brief Retrieves the maximum number of threads */
	uint8_t getNThreads() const ;

protected:
	/** @brief Loads the data of another population */
	virtual void load_(const GObject *);
	/** @brief Creates a deep clone of this object */
	virtual GObject *clone_() const;

	/** @brief Does some preparatory work before the optimization starts */
	virtual void init();
	/** @brief Does any necessary finalization work */
	virtual void finalize();

	/** @brief Saves the state of the class to disc. */
	virtual void saveCheckpoint() const;

	/** @brief Updates the fitness of all individuals */
	virtual void updatePositionsAndFitness();

	/**************************************************************************************************/
private:
	/** @brief The default constructor. Intentionally empty, as it is only needed for de-serialization purposes */
	GMultiThreadedSwarm(){}

	boost::uint8_t nThreads_; ///< The number of threads
	boost::threadpool::pool tp_; ///< A thread pool

	std::vector<bool> le_value_; ///< Internal storage for lazy-evaluation settings

#ifdef GENEVATESTING
public:
	/**************************************************************************************************/
	/** @brief Applies modifications to this object. This is needed for testing purposes */
	virtual bool modify_GUnitTests();
	/** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
	virtual void specificTestsNoFailureExpected_GUnitTests();
	/** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
	virtual void specificTestsFailuresExpected_GUnitTests();
#endif /* GENEVATESTING */
};

} /* namespace Geneva */
} /* namespace Gem */

#endif /* GMULTITHREADEDSWARM_HPP_ */
